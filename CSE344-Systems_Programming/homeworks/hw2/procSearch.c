#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>

#define MAX_WORKERS 8
#define MIN_WORKERS 2
#define MAX_SUBDIRS 1024            
#define MAX_DIRS_PER_WORKER 512 
#define INDENT_STEP 6     
#define MAX_MATCHES 65536
#define TEMP_FILE_PREFIX "/tmp/procSearch_"

volatile sig_atomic_t finished_worker_count = 0;
volatile sig_atomic_t sigint_received = 0;
volatile sig_atomic_t sigterm_received = 0;
volatile sig_atomic_t sigterm_sent = 0;

volatile sig_atomic_t reaped_pids[MAX_WORKERS * 2];
volatile sig_atomic_t reaped_statuses[MAX_WORKERS * 2];
volatile sig_atomic_t reaped_count = 0;

typedef struct {
    pid_t pid;
    char assigned_dirs[MAX_DIRS_PER_WORKER][PATH_MAX];
    int dir_count;
    int match_count;
    int collected;
} WorkerInfo;

typedef struct {
    char path[PATH_MAX];
    off_t size;
    pid_t worker_pid;
} MatchResult;

WorkerInfo workers[MAX_WORKERS];
int total_worker_count = 0;
pid_t parent_pid;

void handle_sigusr1(int sig) {
    (void)sig;
    finished_worker_count++;  
}

void handle_sigint(int sig) {
    (void)sig;
    sigint_received = 1;
}

void handle_sigterm(int sig) {
    (void)sig;
    sigterm_received = 1;
}

void handle_sigchld(int sig) {
    (void)sig;                  
    int status;
    pid_t pid;
    int saved_errno = errno;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (reaped_count < MAX_WORKERS * 2) {
            reaped_pids[reaped_count] = pid;
            reaped_statuses[reaped_count] = status;
            reaped_count++;
        }
    }

    errno = saved_errno;
}

void print_usage(void) {
    fprintf(stderr, "Usage: ./procSearch -d <root_dir> -n <num_workers> "
                    "-f <pattern> [-s <min_size_bytes>]\n");
    fprintf(stderr, "  -d <root_dir>     : Root directory to search (must exist)\n");
    fprintf(stderr, "  -n <num_workers>  : Number of workers (2-8 inclusive)\n");
    fprintf(stderr, "  -f <pattern>      : Filename pattern ('+' operator supported)\n");
    fprintf(stderr, "  -s <min_size>     : Optional, minimum file size in bytes\n");
}

int match_pattern_at(const char *pattern, int pat_len, const char *filename, int fname_len, int start) {
    int p = 0;
    int f = start;

    while (p < pat_len && f < fname_len) {
        char pat_char = pattern[p];

        if (p + 1 < pat_len && pattern[p + 1] == '+') {
            if (tolower((unsigned char)filename[f]) != tolower((unsigned char)pat_char)) {
                return 0;
            }

            int repeat_start = f;
            while (f < fname_len &&
                   tolower((unsigned char)filename[f]) == tolower((unsigned char)pat_char)) {
                f++;
            }
            int repeat_end = f;

            int found = 0;
            int try_pos;
            for (try_pos = repeat_end; try_pos >= repeat_start + 1; try_pos--) {
                if (match_pattern_at(pattern + p + 2, pat_len - p - 2,
                                     filename, fname_len, try_pos)) {
                    found = 1;
                    break;
                }
            }
            return found;
        } else {
            if (tolower((unsigned char)filename[f]) != tolower((unsigned char)pat_char)) {
                return 0;
            }
            p++;
            f++;
        }
    }

    if (p == pat_len) {
        return 1;
    }

    return 0;
}

int match_pattern(const char *pattern, const char *filename) {
    int pat_len = (int)strlen(pattern); 
    int fname_len = (int)strlen(filename); 
    int i;                                 

    for (i = 0; i < fname_len; i++) {
        if (match_pattern_at(pattern, pat_len, filename, fname_len, i)) {
            return 1;
        }
    }

    return 0;
}

void setup_parent_signals(void) {
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigusr1;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction(SIGUSR1) failed");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;   
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction(SIGINT) failed");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigchld; 
    sa.sa_flags = SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction(SIGCHLD) failed");
        exit(EXIT_FAILURE);
    }
}

void setup_worker_signals(void) {
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigterm; 
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction(SIGTERM) failed");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction(SIGINT/IGN) failed");
        exit(EXIT_FAILURE);
    }
}

void search_directory(const char *dir_path, const char *pattern, long min_size, int *match_counter, int *scan_counter, FILE *temp_file, const char *role_label) {
    DIR *dir;               
    struct dirent *entry;
    struct stat file_info;
    char full_path[PATH_MAX];

    if (sigterm_received) {
        return;
    }

    dir = opendir(dir_path);
    if (dir == NULL) {
        fprintf(stderr, "[%s PID:%d] Cannot open directory: %s (%s)\n",
                role_label, getpid(), dir_path, strerror(errno));
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (sigterm_received) {
            break;
        }

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(full_path, PATH_MAX, "%s/%s", dir_path, entry->d_name);

        if (lstat(full_path, &file_info) == -1) {
            fprintf(stderr, "[%s PID:%d] stat failed: %s (%s)\n",
                    role_label, getpid(), full_path, strerror(errno));
            continue;
        }

        if (S_ISDIR(file_info.st_mode)) {
            search_directory(full_path, pattern, min_size, match_counter, scan_counter, temp_file, role_label);
        }
        else if (S_ISREG(file_info.st_mode)) {
            (*scan_counter)++;

            if (match_pattern(pattern, entry->d_name)) {
                if (min_size >= 0 && file_info.st_size < min_size) {
                    continue;
                }

                (*match_counter)++;

                printf("[%s PID:%d] MATCH: %s (%ld bytes)\n",
                       role_label, getpid(), full_path, (long)file_info.st_size);
                fflush(stdout);

                if (temp_file != NULL) {
                    fprintf(temp_file, "%s|%ld|%d\n",
                            full_path, (long)file_info.st_size, getpid());
                    fflush(temp_file);
                }
            }
        }
    }

    closedir(dir);
}

int read_subdirectories(const char *root_dir, char subdirs[][PATH_MAX], int *subdir_count) {
    DIR *dir;               
    struct dirent *entry;
    struct stat file_info;
    char full_path[PATH_MAX];

    *subdir_count = 0;

    dir = opendir(root_dir);
    if (dir == NULL) {
        fprintf(stderr, "Error: Cannot open root directory: %s (%s)\n",
                root_dir, strerror(errno));
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(full_path, PATH_MAX, "%s/%s", root_dir, entry->d_name);

        if (stat(full_path, &file_info) == -1) {
            continue;
        }

        if (S_ISDIR(file_info.st_mode)) {
            if (*subdir_count < MAX_SUBDIRS) {
                strncpy(subdirs[*subdir_count], full_path, PATH_MAX - 1);
                subdirs[*subdir_count][PATH_MAX - 1] = '\0'; 
                (*subdir_count)++; 
            }
        }
    }

    closedir(dir);
    return 0;
}

void read_temp_files(MatchResult *results, int *result_count, int max_results) {
    char temp_path[PATH_MAX];
    char line[PATH_MAX + 64];
    FILE *fp;                   
    int i;                      

    *result_count = 0; 

    for (i = 0; i < total_worker_count; i++) {
        snprintf(temp_path, PATH_MAX, "%s%d.tmp", TEMP_FILE_PREFIX, workers[i].pid);

        fp = fopen(temp_path, "r");
        if (fp == NULL) {
            continue;
        }

        while (fgets(line, sizeof(line), fp) != NULL) {
            if (*result_count >= max_results) {
                break;
            }

            line[strcspn(line, "\n")] = '\0';

            char *path_str = strtok(line, "|");
            char *size_str = strtok(NULL, "|");
            char *pid_str = strtok(NULL, "|");

            if (path_str && size_str && pid_str) {
                strncpy(results[*result_count].path, path_str, PATH_MAX - 1);
                results[*result_count].path[PATH_MAX - 1] = '\0';
                results[*result_count].size = atol(size_str);
                results[*result_count].worker_pid = atoi(pid_str);
                (*result_count)++; 
            }
        }

        fclose(fp);

        unlink(temp_path);
    }
}

void print_tree(const char *root_dir, MatchResult *results, int result_count) {
    if (result_count == 0) {
        printf("No matching files found.\n");
        return;
    }

    printf("%s\n", root_dir);

    int root_len = (int)strlen(root_dir);

    static char printed_dirs[4096][PATH_MAX];
    int printed_dir_count = 0;

    int i;
    for (i = 0; i < result_count; i++) {
        const char *rel_path = results[i].path + root_len;

        if (*rel_path == '/') {
            rel_path++;
        }

        char path_copy[PATH_MAX];
        strncpy(path_copy, rel_path, PATH_MAX - 1);
        path_copy[PATH_MAX - 1] = '\0';

        char *parts[256];
        int part_count = 0;
        char *token = strtok(path_copy, "/");

        while (token != NULL && part_count < 256) {
            parts[part_count++] = token;
            token = strtok(NULL, "/");
        }

        int j;
        for (j = 0; j < part_count; j++) {
            int dash_count = (j + 1) * INDENT_STEP;

            if (j == part_count - 1) {
                printf("|");
                int k;
                for (k = 0; k < dash_count; k++) {
                    printf("-");
                }
                if (total_worker_count == 0) {
                    printf(" %s (%ld bytes) [Parent %d]\n",
                           parts[j], (long)results[i].size, results[i].worker_pid);
                } else {
                    printf(" %s (%ld bytes) [Worker %d]\n",
                           parts[j], (long)results[i].size, results[i].worker_pid);
                }
            } else {
                char dir_path[PATH_MAX];
                dir_path[0] = '\0';
                int m;
                for (m = 0; m <= j; m++) {
                    if (m > 0) strcat(dir_path, "/");
                    strcat(dir_path, parts[m]);
                }

                int already_printed = 0;
                int n;
                for (n = 0; n < printed_dir_count; n++) {
                    if (strcmp(printed_dirs[n], dir_path) == 0) {
                        already_printed = 1;
                        break;
                    }
                }

                if (!already_printed) {
                    printf("|");
                    int k;
                    for (k = 0; k < dash_count; k++) {
                        printf("-");
                    }
                    printf(" %s\n", parts[j]);

                    if (printed_dir_count < 4096) {
                        strncpy(printed_dirs[printed_dir_count],
                                dir_path, PATH_MAX - 1);
                        printed_dirs[printed_dir_count][PATH_MAX - 1] = '\0';
                        printed_dir_count++;
                    }
                }
            }
        }
    }
}

void print_summary(int total_scanned) {
    int total_matches = 0;  
    int i;                  

    for (i = 0; i < total_worker_count; i++) {
        total_matches += workers[i].match_count;
    }

    printf("--- Summary ---\n");
    printf("Total workers used : %d\n", total_worker_count);
    printf("Total files scanned : %d\n", total_scanned);
    printf("Total matches found : %d\n", total_matches);

    for (i = 0; i < total_worker_count; i++) {
        if (workers[i].match_count == 1) {
            printf("Worker PID %d : 1 match\n", workers[i].pid);
        } else {
            printf("Worker PID %d : %d matches\n",
                   workers[i].pid, workers[i].match_count);
        }
    }
}

void parent_direct_search(const char *root_dir, const char *pattern, long min_size) {
    int match_count = 0;
    int scan_count = 0;
    static MatchResult results[MAX_MATCHES];
    int result_count = 0;

    char temp_path[PATH_MAX];
    snprintf(temp_path, PATH_MAX, "%s%d.tmp", TEMP_FILE_PREFIX, getpid());

    FILE *temp_file = fopen(temp_path, "w");

    search_directory(root_dir, pattern, min_size,
                     &match_count, &scan_count, temp_file,
                     "Parent");

    if (temp_file != NULL) {
        fclose(temp_file);
    }

    temp_file = fopen(temp_path, "r");
    if (temp_file != NULL) {
        char line[PATH_MAX + 64];
        while (fgets(line, sizeof(line), temp_file) != NULL && result_count < MAX_MATCHES) {
            line[strcspn(line, "\n")] = '\0';
            char *path_str = strtok(line, "|");
            char *size_str = strtok(NULL, "|");
            char *pid_str = strtok(NULL, "|");
            if (path_str && size_str && pid_str) {
                strncpy(results[result_count].path, path_str, PATH_MAX - 1);
                results[result_count].path[PATH_MAX - 1] = '\0';
                results[result_count].size = atol(size_str);
                results[result_count].worker_pid = atoi(pid_str);
                result_count++;
            }
        }
        fclose(temp_file);
    }

    unlink(temp_path);

    print_tree(root_dir, results, result_count);

    printf("--- Summary ---\n");
    printf("Total workers used : 0\n");
    printf("Total files scanned : %d\n", scan_count);
    printf("Total matches found : %d\n", match_count);
}

void run_worker(int worker_index, const char *pattern, long min_size) {
    int match_count = 0;
    int scan_count = 0;
    char temp_path[PATH_MAX];
    int i;

    setup_worker_signals();

    snprintf(temp_path, PATH_MAX, "%s%d.tmp", TEMP_FILE_PREFIX, getpid());

    FILE *temp_file = fopen(temp_path, "w");
    if (temp_file == NULL) {
        fprintf(stderr, "[Worker PID:%d] Cannot create temp file: %s\n",
                getpid(), temp_path);
    }

    for (i = 0; i < workers[worker_index].dir_count; i++) {
        if (sigterm_received) {
            break;
        }

        search_directory(workers[worker_index].assigned_dirs[i],
                         pattern, min_size,
                         &match_count, &scan_count, temp_file,
                         "Worker");
    }

    if (temp_file != NULL) {
        fclose(temp_file);
    }

    if (sigterm_received) {
        printf("[Worker PID:%d] SIGTERM received. Partial matches: %d. Exiting.\n",
               getpid(), match_count);
        fflush(stdout);
        exit(match_count % 256);
    }

    char info_path[PATH_MAX];
    snprintf(info_path, PATH_MAX, "%s%d_info.tmp", TEMP_FILE_PREFIX, getpid());
    FILE *info_file = fopen(info_path, "w");
    if (info_file != NULL) {
        fprintf(info_file, "%d\n", scan_count);
        fclose(info_file);
    }

    kill(parent_pid, SIGUSR1);

    exit(match_count % 256);
}

int main(int argc, char *argv[]) {
    char *root_dir = NULL;
    int num_workers = 0;
    char *pattern = NULL;
    long min_size = -1;
    int opt;

    while ((opt = getopt(argc, argv, "d:n:f:s:")) != -1) {
        switch (opt) {
            case 'd':
                root_dir = optarg;
                break;
            case 'n':
                num_workers = atoi(optarg);
                break;
            case 'f':
                pattern = optarg;
                break;
            case 's':
                min_size = atol(optarg);
                break;
            default:
                print_usage();
                exit(EXIT_FAILURE);
        }
    }

    if (root_dir == NULL || num_workers == 0 || pattern == NULL) {
        fprintf(stderr, "Error: Missing required arguments.\n");
        print_usage();
        exit(EXIT_FAILURE);
    }

    if (num_workers < MIN_WORKERS || num_workers > MAX_WORKERS) {
        fprintf(stderr, "Error: Number of workers must be between %d and %d.\n",
                MIN_WORKERS, MAX_WORKERS);
        print_usage();
        exit(EXIT_FAILURE);
    }

    struct stat dir_info;
    if (stat(root_dir, &dir_info) == -1 || !S_ISDIR(dir_info.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a valid directory.\n", root_dir);
        exit(EXIT_FAILURE);
    }

    parent_pid = getpid();

    static char subdirs[MAX_SUBDIRS][PATH_MAX];
    int subdir_count = 0;

    if (read_subdirectories(root_dir, subdirs, &subdir_count) == -1) {
        exit(EXIT_FAILURE);
    }

    if (subdir_count == 0) {
        printf("Notice: no subdirectories found; parent will search root directly.\n");
        fflush(stdout);
        parent_direct_search(root_dir, pattern, min_size);
        return 0;
    }

    if (subdir_count < num_workers) {
        printf("Notice: only %d subdirectories found; using %d workers instead of %d.\n",
               subdir_count, subdir_count, num_workers);
        fflush(stdout);
        num_workers = subdir_count;
    }

    total_worker_count = num_workers;

    int i; 
    for (i = 0; i < num_workers; i++) {
        workers[i].pid = 0;
        workers[i].dir_count = 0;
        workers[i].match_count = 0;
        workers[i].collected = 0;
    }

    for (i = 0; i < subdir_count; i++) {
        int target_worker = i % num_workers;
        int dir_idx = workers[target_worker].dir_count;

        strncpy(workers[target_worker].assigned_dirs[dir_idx],
                subdirs[i], PATH_MAX - 1);
        workers[target_worker].assigned_dirs[dir_idx][PATH_MAX - 1] = '\0';
        workers[target_worker].dir_count++;
    }

    setup_parent_signals();

    for (i = 0; i < num_workers; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            fprintf(stderr, "Error: fork() failed for worker %d: %s\n", i, strerror(errno));
            int j;
            for (j = 0; j < i; j++) {
                kill(workers[j].pid, SIGTERM);
            }
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            run_worker(i, pattern, min_size);
            exit(0);
        }

        workers[i].pid = pid;
    }

    while ((int)finished_worker_count < total_worker_count &&
           (int)reaped_count < total_worker_count &&
           !sigint_received) {
        sleep(1);
    }

    if (sigint_received) {
        printf("[Parent] SIGINT received. Terminating all workers...\n");
        fflush(stdout);

        sigterm_sent = 1;

        for (i = 0; i < total_worker_count; i++) {
            kill(workers[i].pid, SIGTERM);
        }

        sleep(3);

        for (i = 0; i < total_worker_count; i++) {
            int status = 0;
            pid_t result = waitpid(workers[i].pid, &status, WNOHANG);
            if (result == 0) {
                kill(workers[i].pid, SIGKILL);
                waitpid(workers[i].pid, &status, 0);
            } else if (result == -1) {
                continue;
            }
            if (WIFEXITED(status)) {
                workers[i].match_count = WEXITSTATUS(status);
            }
        }

        static MatchResult results[MAX_MATCHES];
        int result_count = 0;
        read_temp_files(results, &result_count, MAX_MATCHES);
        print_tree(root_dir, results, result_count);

        int total_scanned = 0;
        print_summary(total_scanned);

        for (i = 0; i < total_worker_count; i++) {
            char info_path[PATH_MAX];
            snprintf(info_path, PATH_MAX, "%s%d_info.tmp",
                     TEMP_FILE_PREFIX, workers[i].pid);
            unlink(info_path);
        }

        return 1;
    }

    for (i = 0; i < (int)reaped_count; i++) {
        pid_t reaped_pid = reaped_pids[i];
        int reaped_status = reaped_statuses[i];

        int j;
        for (j = 0; j < total_worker_count; j++) {
            if (workers[j].pid == reaped_pid) {
                if (WIFEXITED(reaped_status)) {
                    workers[j].match_count = WEXITSTATUS(reaped_status);
                }
                if (WIFSIGNALED(reaped_status) && !sigterm_sent) {
                    fprintf(stderr,
                            "[Parent] Worker PID:%d terminated unexpectedly (exit status: %d).\n",
                            (int)reaped_pid, WTERMSIG(reaped_status));
                }
                workers[j].collected = 1;
                break;
            }
        }
    }

    for (i = 0; i < total_worker_count; i++) {
        if (workers[i].collected) {
            continue;
        }

        int status = 0;
        pid_t result = waitpid(workers[i].pid, &status, WNOHANG);

        if (result == 0) {
            waitpid(workers[i].pid, &status, 0);
            result = workers[i].pid;
        }

        if (result > 0) {
            if (WIFEXITED(status)) {
                workers[i].match_count = WEXITSTATUS(status);
            }
        }
    }

    static MatchResult results[MAX_MATCHES];
    int result_count = 0;

    read_temp_files(results, &result_count, MAX_MATCHES);

    print_tree(root_dir, results, result_count);

    int total_scanned = 0;
    for (i = 0; i < total_worker_count; i++) {
        char info_path[PATH_MAX];
        snprintf(info_path, PATH_MAX, "%s%d_info.tmp",
                 TEMP_FILE_PREFIX, workers[i].pid);
        FILE *info_file = fopen(info_path, "r");
        if (info_file != NULL) {
            int scanned = 0;
            if (fscanf(info_file, "%d", &scanned) == 1) {
                total_scanned += scanned;
            }
            fclose(info_file);
            unlink(info_path);
        }
    }

    print_summary(total_scanned);

    return 0;
}
