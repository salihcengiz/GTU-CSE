#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <limits.h>

typedef struct {
    char *filename_pattern;
    off_t file_size;
    char file_type;
    char permissions[10];
    int link_count;
    int use_name;
    int use_size;
    int use_type;
    int use_perm;
    int use_links;
} SearchCriteria;

typedef struct TreeNode {
    char *name;
    int depth;
    int is_dir;
    struct TreeNode *next;
} TreeNode;

static volatile sig_atomic_t g_interrupted = 0;
static TreeNode *g_tree_head = NULL;
static TreeNode *g_tree_tail = NULL;

static void sigint_handler(int sig) {
    (void)sig;
    g_interrupted = 1;
}

static void free_tree_list(void) {
    TreeNode *cur = g_tree_head;
    while (cur) {
        TreeNode *tmp = cur;
        cur = cur->next;
        free(tmp->name);
        free(tmp);
    }
    g_tree_head = NULL;
    g_tree_tail = NULL;
}

static TreeNode *add_tree_node(const char *name, int depth, int is_dir) {
    TreeNode *node = malloc(sizeof(TreeNode));
    if (!node) {
        perror("malloc");
        return NULL;
    }

    node->name = strdup(name);
    if (!node->name) {
        perror("strdup");
        free(node);
        return NULL;
    }

    node->depth = depth;
    node->is_dir = is_dir;
    node->next = NULL;

    if (!g_tree_head) {
        g_tree_head = node;
        g_tree_tail = node;
    } else {
        g_tree_tail->next = node;
        g_tree_tail = node;
    }
    return node;
}

static void print_usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s -w <path> [-f <pattern>] [-b <size>] "
            "[-t <type>] [-p <perms>] [-l <links>]\n"
            "  -w <path>    : Directory to search (required)\n"
            "  -f <pattern> : Filename pattern (case-insensitive, '+' = one or more)\n"
            "  -b <size>    : File size in bytes (exact match)\n"
            "  -t <type>    : File type (d/s/b/c/f/p/l)\n"
            "  -p <perms>   : Permissions string (e.g. rwxr-xr--)\n"
            "  -l <links>   : Number of hard links\n",
            prog);
}

static int match_pattern(const char *pattern, const char *str) {
    int pi = 0, si = 0;
    int plen = (int)strlen(pattern);
    int slen = (int)strlen(str);

    while (pi < plen) {
        if (pi + 1 < plen && pattern[pi + 1] == '+') {
            char pc = (char)tolower((unsigned char)pattern[pi]);
            if (si >= slen || tolower((unsigned char)str[si]) != pc)
                return 0;

            while (si < slen && tolower((unsigned char)str[si]) == pc)
                si++;

            pi += 2;
        } else {
            if (si >= slen)
                return 0;
            if (tolower((unsigned char)pattern[pi]) != tolower((unsigned char)str[si]))
                return 0;
            pi++;
            si++;
        }
    }

    return si == slen;
}

static int match_name(const SearchCriteria *c, const char *name) {
    return match_pattern(c->filename_pattern, name);
}

static int match_size(const SearchCriteria *c, const struct stat *st) {
    return st->st_size == c->file_size;
}

static int match_type(const SearchCriteria *c, const struct stat *st) {
    mode_t m = st->st_mode;
    switch (c->file_type) {
        case 'd': return S_ISDIR(m);
        case 'f': return S_ISREG(m);
        case 'l': return S_ISLNK(m);
        case 'b': return S_ISBLK(m);
        case 'c': return S_ISCHR(m);
        case 'p': return S_ISFIFO(m);
        case 's': return S_ISSOCK(m);
        default:  return 0;
    }
}

static mode_t parse_permissions(const char *pstr) {
    mode_t mode = 0;
    if (strlen(pstr) != 9)
        return (mode_t)-1;

    if (pstr[0] == 'r')
        mode |= S_IRUSR;
    else if (pstr[0] != '-')
        return (mode_t)-1;

    if (pstr[1] == 'w')
        mode |= S_IWUSR;
    else if (pstr[1] != '-')
        return (mode_t)-1;

    if (pstr[2] == 'x')
        mode |= S_IXUSR;
    else if (pstr[2] != '-')
        return (mode_t)-1;

    if (pstr[3] == 'r')
        mode |= S_IRGRP;
    else if (pstr[3] != '-')
        return (mode_t)-1;

    if (pstr[4] == 'w')
        mode |= S_IWGRP;
    else if (pstr[4] != '-')
        return (mode_t)-1;

    if (pstr[5] == 'x')
        mode |= S_IXGRP;
    else if (pstr[5] != '-')
        return (mode_t)-1;

    if (pstr[6] == 'r')
        mode |= S_IROTH;
    else if (pstr[6] != '-')
        return (mode_t)-1;

    if (pstr[7] == 'w')
        mode |= S_IWOTH;
    else if (pstr[7] != '-')
        return (mode_t)-1;

    if (pstr[8] == 'x')
        mode |= S_IXOTH;
    else if (pstr[8] != '-')
        return (mode_t)-1;

    return mode;
}

static int match_perm(const SearchCriteria *c, const struct stat *st) {
    mode_t target = parse_permissions(c->permissions);
    if (target == (mode_t)-1)
        return 0;
    return (st->st_mode & 0777) == target;
}

static int match_links(const SearchCriteria *c, const struct stat *st) {
    return (int)st->st_nlink == c->link_count;
}

static int matches_criteria(const SearchCriteria *c, const char *name, const struct stat *st) {
    if (c->use_name && !match_name(c, name))
        return 0;
    if (c->use_size && !match_size(c, st))
        return 0;
    if (c->use_type && !match_type(c, st))
        return 0;
    if (c->use_perm && !match_perm(c, st))
        return 0;
    if (c->use_links && !match_links(c, st))
        return 0;
    return 1;
}

static void print_tree_node(const char *name, int depth) {
    int i;
    if (depth == 0) {
        printf("%s\n", name);
        return;
    }
    printf("|");
    for (i = 0; i < depth * 4 - 2; i++)
        printf("-");
    printf("%s\n", name);
}

static void mark_ancestors(TreeNode *list, TreeNode *target) {
    int target_depth = target->depth;
    TreeNode *cur;
    int d;

    for (d = target_depth - 1; d >= 0; d--) {
        cur = list;
        TreeNode *last_at_depth = NULL;
        while (cur && cur != target) {
            if (cur->depth == d && cur->is_dir)
                last_at_depth = cur;
            cur = cur->next;
        }
        if (last_at_depth)
            last_at_depth->is_dir = 2;
    }
}

static int search_recursive(const char *dirpath, int depth, const SearchCriteria *criteria, int *found) {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char fullpath[PATH_MAX];
    TreeNode *dir_node;

    if (g_interrupted)
        return -1;

    dir = opendir(dirpath);
    if (!dir) {
        fprintf(stderr, "Cannot open directory '%s': ", dirpath);
        perror("");
        return -1;
    }

    dir_node = add_tree_node(dirpath, depth, 1);
    if (!dir_node) {
        closedir(dir);
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (g_interrupted)
            break;

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);

        if (lstat(fullpath, &st) == -1) {
            fprintf(stderr, "Cannot stat '%s': ", fullpath);
            perror("");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            search_recursive(fullpath, depth + 1, criteria, found);

            if (matches_criteria(criteria, entry->d_name, &st)) {
                TreeNode *cur = g_tree_head;
                while (cur) {
                    if (cur->depth == depth + 1 && cur->is_dir == 1 &&
                        strcmp(cur->name, fullpath) == 0) {
                        cur->is_dir = 2;
                        mark_ancestors(g_tree_head, cur);
                        *found = 1;
                        break;
                    }
                    cur = cur->next;
                }
            }
        } else {
            if (matches_criteria(criteria, entry->d_name, &st)) {
                TreeNode *file_node = add_tree_node(entry->d_name, depth + 1, 0);
                if (file_node) {
                    mark_ancestors(g_tree_head, file_node);
                    *found = 1;
                }
            }
        }
    }

    if (closedir(dir) == -1) {
        fprintf(stderr, "Cannot close directory '%s': ", dirpath);
        perror("");
    }
    return 0;
}

static const char *basename_of(const char *path) {
    const char *last = strrchr(path, '/');
    return last ? last + 1 : path;
}

static void print_results(void) {
    TreeNode *cur = g_tree_head;
    while (cur) {
        if (cur->is_dir == 2) {
            if (cur->depth == 0)
                print_tree_node(cur->name, 0);
            else
                print_tree_node(basename_of(cur->name), cur->depth);
        } else if (!cur->is_dir) {
            print_tree_node(cur->name, cur->depth);
        }
        cur = cur->next;
    }
}

int main(int argc, char *argv[]) {
    int opt;
    char *search_path = NULL;
    SearchCriteria criteria;
    int found = 0;
    int has_filter = 0;
    struct sigaction sa;

    memset(&criteria, 0, sizeof(criteria));

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    while ((opt = getopt(argc, argv, "w:f:b:t:p:l:")) != -1) {
        switch (opt) {
            case 'w':
                search_path = optarg;
                break;
            case 'f':
                criteria.filename_pattern = optarg;
                criteria.use_name = 1;
                has_filter = 1;
                break;
            case 'b':
                criteria.file_size = atol(optarg);
                criteria.use_size = 1;
                has_filter = 1;
                break;
            case 't':
                criteria.file_type = optarg[0];
                criteria.use_type = 1;
                has_filter = 1;
                break;
            case 'p':
                if (strlen(optarg) != 9) {
                    fprintf(stderr, "Error: permissions must be 9 characters (e.g. rwxr-xr--)\n");
                    return EXIT_FAILURE;
                }
                strncpy(criteria.permissions, optarg, 9);
                criteria.permissions[9] = '\0';
                criteria.use_perm = 1;
                has_filter = 1;
                break;
            case 'l':
                criteria.link_count = atoi(optarg);
                criteria.use_links = 1;
                has_filter = 1;
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (!search_path) {
        fprintf(stderr, "Error: -w <path> is required.\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (!has_filter) {
        fprintf(stderr, "Error: at least one filter (-f, -b, -t, -p, -l) is required.\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    search_recursive(search_path, 0, &criteria, &found);

    if (g_interrupted) {
        fprintf(stderr, "\nCaught SIGINT, exiting...\n");
        free_tree_list();
        return EXIT_FAILURE;
    }

    if (!found) {
        printf("No file found\n");
    } else {
        print_results();
    }

    free_tree_list();
    return EXIT_SUCCESS;
}
