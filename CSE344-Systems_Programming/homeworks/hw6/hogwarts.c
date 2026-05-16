/*
 * hogwarts.c - Magic Academy Server
 * CSE344 HW6 - Single-threaded TCP server using select()
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <fcntl.h>

#define MAX_LINE      512
#define MAX_NAME      32
#define MAX_TYPE      16
#define MAX_INGR_NAME 17
#define MAX_INGREDIENTS 64

/* ---------- data structures ---------- */

typedef struct {
    char name[MAX_INGR_NAME];
    int  quantity;
} ingredient_t;

typedef struct {
    char name[MAX_INGR_NAME];
    int  quantity;
} spellbook_entry_t;

typedef struct {
    int    fd;
    char   username[MAX_NAME];
    char   type[MAX_TYPE];        /* WIZARD or PROFESSOR */
    char   line_buf[MAX_LINE + 1];
    int    buf_len;
    time_t last_active;
    int    enrolled;
    spellbook_entry_t spellbook[MAX_INGREDIENTS];
    int    spellbook_count;
} client_t;

/* ---------- globals ---------- */

static volatile sig_atomic_t shutdown_flag = 0;

static ingredient_t ingredients[MAX_INGREDIENTS];
static int           num_ingredients = 0;

static client_t *clients = NULL;
static int        max_clients = 0;
static int        num_connected = 0;

static int    timeout_sec = 30;
static FILE  *log_fp = NULL;
static int    listen_fd = -1;

/* ---------- signal handler ---------- */

static void sigint_handler(int sig) {
    (void)sig;
    shutdown_flag = 1;
}

/* ---------- logging ---------- */

static void server_log(const char *fmt, ...) {
    char timebuf[64];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);

    va_list ap;
    char msg[1024];

    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    printf("[SERVER] %s\n", msg);
    fflush(stdout);

    if (log_fp) {
        fprintf(log_fp, "[%s] %s\n", timebuf, msg);
        fflush(log_fp);
    }
}

/* ---------- safe write (partial write handling) ---------- */

static int send_to_client(int fd, const char *data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = write(fd, data + sent, len - sent);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
}

static int send_line(int fd, const char *line) {
    char buf[MAX_LINE + 2];
    int len = snprintf(buf, sizeof(buf), "%s\n", line);
    return send_to_client(fd, buf, (size_t)len);
}

/* ---------- ingredient helpers ---------- */

static int load_ingredients(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) { perror("fopen ingredients"); return -1; }

    char line[256];
    while (fgets(line, sizeof(line), fp) && num_ingredients < MAX_INGREDIENTS) {
        char name[MAX_INGR_NAME];
        int qty;
        if (sscanf(line, "%16s %d", name, &qty) == 2 && qty > 0) {
            /* validate: uppercase alnum + underscore */
            int valid = 1;
            for (int i = 0; name[i]; i++) {
                char c = name[i];
                if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'))
                    { valid = 0; break; }
            }
            if (valid) {
                strncpy(ingredients[num_ingredients].name, name, MAX_INGR_NAME - 1);
                ingredients[num_ingredients].name[MAX_INGR_NAME - 1] = '\0';
                ingredients[num_ingredients].quantity = qty;
                num_ingredients++;
            }
        }
    }
    fclose(fp);
    return 0;
}

static int find_ingredient(const char *name) {
    for (int i = 0; i < num_ingredients; i++)
        if (strcmp(ingredients[i].name, name) == 0) return i;
    return -1;
}

/* ---------- client helpers ---------- */

static void init_client(client_t *c) {
    c->fd = -1;
    c->username[0] = '\0';
    c->type[0] = '\0';
    c->line_buf[0] = '\0';
    c->buf_len = 0;
    c->last_active = 0;
    c->enrolled = 0;
    c->spellbook_count = 0;
    memset(c->spellbook, 0, sizeof(c->spellbook));
}

static int find_empty_slot(void) {
    for (int i = 0; i < max_clients; i++)
        if (clients[i].fd == -1) return i;
    return -1;
}

static int username_exists(const char *name) {
    for (int i = 0; i < max_clients; i++)
        if (clients[i].fd != -1 && clients[i].enrolled &&
            strcmp(clients[i].username, name) == 0) return 1;
    return 0;
}

static void disconnect_client(int idx, const char *reason) {
    client_t *c = &clients[idx];
    if (c->fd == -1) return;
    server_log("CLIENT_DISCONNECTED username=%s reason=%s",
               c->enrolled ? c->username : "(unknown)", reason);
    close(c->fd);
    init_client(c);
    num_connected--;
}

/* ---------- spellbook helpers ---------- */

static int spellbook_find(client_t *c, const char *name) {
    for (int i = 0; i < c->spellbook_count; i++)
        if (strcmp(c->spellbook[i].name, name) == 0) return i;
    return -1;
}

static void spellbook_add(client_t *c, const char *name, int qty) {
    int idx = spellbook_find(c, name);
    if (idx >= 0) {
        c->spellbook[idx].quantity += qty;
    } else if (c->spellbook_count < MAX_INGREDIENTS) {
        strncpy(c->spellbook[c->spellbook_count].name, name, MAX_INGR_NAME - 1);
        c->spellbook[c->spellbook_count].name[MAX_INGR_NAME - 1] = '\0';
        c->spellbook[c->spellbook_count].quantity = qty;
        c->spellbook_count++;
    }
}

/* ---------- command processing ---------- */

static void process_command(int idx, char *line) {
    client_t *c = &clients[idx];
    c->last_active = time(NULL);

    /* trim trailing whitespace */
    size_t len = strlen(line);
    while (len > 0 && (line[len-1] == '\r' || line[len-1] == '\n' || line[len-1] == ' '))
        line[--len] = '\0';

    if (len == 0) return;

    /* parse command */
    char cmd[64] = {0};
    sscanf(line, "%63s", cmd);

    /* ---------- ENROLL ---------- */
    if (strcmp(cmd, "ENROLL") == 0) {
        char ctype[MAX_TYPE] = {0}, uname[MAX_NAME] = {0};
        if (sscanf(line, "ENROLL %15s %31s", ctype, uname) != 2) {
            send_line(c->fd, "ERR UNKNOWN ENROLL");
            return;
        }
        if (c->enrolled) {
            send_line(c->fd, "ERR UNKNOWN ENROLL");
            return;
        }
        if (strcmp(ctype, "WIZARD") != 0 && strcmp(ctype, "PROFESSOR") != 0) {
            send_line(c->fd, "ERR UNKNOWN ENROLL");
            return;
        }
        if (username_exists(uname)) {
            send_line(c->fd, "ERR ENROLL name_taken");
            return;
        }
        strncpy(c->username, uname, MAX_NAME - 1);
        c->username[MAX_NAME - 1] = '\0';
        strncpy(c->type, ctype, MAX_TYPE - 1);
        c->type[MAX_TYPE - 1] = '\0';
        c->enrolled = 1;

        char resp[MAX_LINE];
        snprintf(resp, sizeof(resp), "OK ENROLL %s", uname);
        send_line(c->fd, resp);
        server_log("ENROLL username=%s type=%s fd=%d", uname, ctype, c->fd);
        return;
    }

    /* ---------- APPARATE (works even before enrollment) ---------- */
    if (strcmp(cmd, "APPARATE") == 0) {
        send_line(c->fd, "OK APPARATE");
        disconnect_client(idx, "APPARATE");
        return;
    }

    /* check enrollment for all other commands */
    if (!c->enrolled) {
        send_line(c->fd, "ERR NOT_ENROLLED");
        return;
    }

    int is_wizard    = (strcmp(c->type, "WIZARD") == 0);
    int is_professor = (strcmp(c->type, "PROFESSOR") == 0);


    /* ---------- BREW ---------- */
    if (strcmp(cmd, "BREW") == 0) {
        if (!is_wizard) { send_line(c->fd, "ERR UNAUTHORIZED"); return; }

        char ingr[MAX_INGR_NAME] = {0};
        int qty = 0;
        if (sscanf(line, "BREW %16s %d", ingr, &qty) != 2 || qty <= 0) {
            send_line(c->fd, "ERR UNKNOWN BREW");
            return;
        }
        int ii = find_ingredient(ingr);
        if (ii < 0) { send_line(c->fd, "ERR UNKNOWN_INGREDIENT"); return; }

        int old_qty = ingredients[ii].quantity;
        ingredients[ii].quantity += qty;
        int new_qty = ingredients[ii].quantity;
        spellbook_add(c, ingr, qty);

        char resp[MAX_LINE];
        snprintf(resp, sizeof(resp), "OK BREW %s %d %d", ingr, qty, new_qty);
        send_line(c->fd, resp);
        server_log("BREW wizard=%s ingredient=%s qty=%d old_qty=%d new_qty=%d",
                   c->username, ingr, qty, old_qty, new_qty);
        return;
    }

    /* ---------- CONSUME ---------- */
    if (strcmp(cmd, "CONSUME") == 0) {
        if (!is_wizard) { send_line(c->fd, "ERR UNAUTHORIZED"); return; }

        char ingr[MAX_INGR_NAME] = {0};
        int qty = 0;
        if (sscanf(line, "CONSUME %16s %d", ingr, &qty) != 2 || qty <= 0) {
            send_line(c->fd, "ERR UNKNOWN CONSUME");
            return;
        }
        int ii = find_ingredient(ingr);
        if (ii < 0) { send_line(c->fd, "ERR UNKNOWN_INGREDIENT"); return; }

        /* check wizard spellbook */
        int si = spellbook_find(c, ingr);
        if (si < 0 || c->spellbook[si].quantity < qty) {
            send_line(c->fd, "ERR INSUFFICIENT_INGREDIENTS");
            return;
        }
        /* check global */
        if (ingredients[ii].quantity < qty) {
            send_line(c->fd, "ERR INSUFFICIENT_INGREDIENTS");
            return;
        }

        int old_qty = ingredients[ii].quantity;
        ingredients[ii].quantity -= qty;
        int new_qty = ingredients[ii].quantity;
        c->spellbook[si].quantity -= qty;

        char resp[MAX_LINE];
        snprintf(resp, sizeof(resp), "OK CONSUME %s %d %d", ingr, qty, new_qty);
        send_line(c->fd, resp);
        server_log("CONSUME wizard=%s ingredient=%s qty=%d old_qty=%d new_qty=%d",
                   c->username, ingr, qty, old_qty, new_qty);
        return;
    }

    /* ---------- SPELLBOOK ---------- */
    if (strcmp(cmd, "SPELLBOOK") == 0) {
        if (!is_wizard) { send_line(c->fd, "ERR UNAUTHORIZED"); return; }

        /* build spellbook string - only entries with qty > 0 */
        char resp[MAX_LINE];
        int off = snprintf(resp, sizeof(resp), "OK SPELLBOOK ");
        int count = 0;
        for (int i = 0; i < c->spellbook_count; i++) {
            if (c->spellbook[i].quantity > 0) {
                if (count > 0)
                    off += snprintf(resp + off, sizeof(resp) - (size_t)off, ",");
                off += snprintf(resp + off, sizeof(resp) - (size_t)off, "%s:%d",
                                c->spellbook[i].name, c->spellbook[i].quantity);
                count++;
            }
        }
        if (count == 0)
            snprintf(resp, sizeof(resp), "OK SPELLBOOK EMPTY");
        send_line(c->fd, resp);
        return;
    }

    /* ---------- INSPECT ---------- */
    if (strcmp(cmd, "INSPECT") == 0) {
        if (!is_professor) { send_line(c->fd, "ERR UNAUTHORIZED"); return; }

        char ingr[MAX_INGR_NAME] = {0};
        if (sscanf(line, "INSPECT %16s", ingr) != 1) {
            send_line(c->fd, "ERR UNKNOWN INSPECT");
            return;
        }
        int ii = find_ingredient(ingr);
        if (ii < 0) { send_line(c->fd, "ERR UNKNOWN_INGREDIENT"); return; }

        char resp[MAX_LINE];
        snprintf(resp, sizeof(resp), "OK INSPECT %s %d", ingr, ingredients[ii].quantity);
        send_line(c->fd, resp);
        server_log("INSPECT professor=%s ingredient=%s qty=%d",
                   c->username, ingr, ingredients[ii].quantity);
        return;
    }

    /* ---------- SCROLL ---------- */
    if (strcmp(cmd, "SCROLL") == 0) {
        if (!is_professor) { send_line(c->fd, "ERR UNAUTHORIZED"); return; }

        char resp[MAX_LINE];
        int off = snprintf(resp, sizeof(resp), "OK SCROLL ");
        for (int i = 0; i < num_ingredients; i++) {
            if (i > 0)
                off += snprintf(resp + off, sizeof(resp) - (size_t)off, ",");
            off += snprintf(resp + off, sizeof(resp) - (size_t)off, "%s:%d",
                            ingredients[i].name, ingredients[i].quantity);
        }
        send_line(c->fd, resp);
        server_log("SCROLL professor=%s ingredients=%d",
                   c->username, num_ingredients);
        return;
    }

    /* ---------- ROSTER ---------- */
    if (strcmp(cmd, "ROSTER") == 0) {
        if (!is_professor) { send_line(c->fd, "ERR UNAUTHORIZED"); return; }

        char resp[MAX_LINE];
        int off = snprintf(resp, sizeof(resp), "OK ROSTER ");
        int count = 0;
        for (int i = 0; i < max_clients; i++) {
            if (clients[i].fd != -1 && clients[i].enrolled) {
                if (count > 0)
                    off += snprintf(resp + off, sizeof(resp) - (size_t)off, ",");
                off += snprintf(resp + off, sizeof(resp) - (size_t)off, "%s",
                                clients[i].username);
                count++;
            }
        }
        send_line(c->fd, resp);
        server_log("ROSTER professor=%s clients=%d", c->username, count);
        return;
    }

    /* ---------- UNKNOWN ---------- */
    {
        char resp[MAX_LINE];
        snprintf(resp, sizeof(resp), "ERR UNKNOWN %s", cmd);
        send_line(c->fd, resp);
    }
    (void)is_professor;
}

/* ---------- handle incoming data for a client ---------- */

static void handle_client_data(int idx) {
    client_t *c = &clients[idx];
    char tmpbuf[MAX_LINE];
    ssize_t n = read(c->fd, tmpbuf, sizeof(tmpbuf) - 1);

    if (n <= 0) {
        /* client hung up or error */
        disconnect_client(idx, "hangup");
        return;
    }

    /* append to line buffer */
    for (ssize_t i = 0; i < n; i++) {
        if (c->buf_len >= MAX_LINE) {
            /* line too long */
            send_line(c->fd, "ERR TOOLONG");
            c->buf_len = 0;
            c->line_buf[0] = '\0';
            continue;
        }
        c->line_buf[c->buf_len++] = tmpbuf[i];

        if (tmpbuf[i] == '\n') {
            c->line_buf[c->buf_len] = '\0';
            process_command(idx, c->line_buf);
            c->buf_len = 0;
            c->line_buf[0] = '\0';
            /* client might have been disconnected by APPARATE */
            if (c->fd == -1) return;
        }
    }
}

/* ---------- main ---------- */

int main(int argc, char *argv[]) {
    int tcp_port = -1;
    char *ingr_file = NULL;
    char *log_file = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "p:s:l:n:t:")) != -1) {
        switch (opt) {
        case 'p': tcp_port    = atoi(optarg); break;
        case 's': ingr_file   = optarg;       break;
        case 'l': log_file    = optarg;       break;
        case 'n': max_clients = atoi(optarg); break;
        case 't': timeout_sec = atoi(optarg); break;
        default:
            fprintf(stderr, "Usage: %s -p <port> -s <file> -l <log> -n <max> -t <timeout>\n",
                    argv[0]);
            return 1;
        }
    }

    if (tcp_port < 1024 || !ingr_file || !log_file || max_clients < 1 || timeout_sec < 1) {
        fprintf(stderr, "Usage: %s -p <port>=1024> -s <file> -l <log> -n <max>=1> -t <timeout>=1>\n",
                argv[0]);
        return 1;
    }

    /* load ingredients */
    if (load_ingredients(ingr_file) < 0) return 1;
    if (num_ingredients == 0) {
        fprintf(stderr, "No valid ingredients loaded.\n");
        return 1;
    }

    /* open log file */
    log_fp = fopen(log_file, "a");
    if (!log_fp) { perror("fopen log"); return 1; }

    /* allocate client array */
    clients = calloc((size_t)max_clients, sizeof(client_t));
    if (!clients) { perror("calloc"); return 1; }
    for (int i = 0; i < max_clients; i++) init_client(&clients[i]);

    /* create TCP listener */
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return 1; }

    int reuse = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family      = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port        = htons((uint16_t)tcp_port);

    if (bind(listen_fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("bind"); close(listen_fd); return 1;
    }
    if (listen(listen_fd, 16) < 0) {
        perror("listen"); close(listen_fd); return 1;
    }

    /* ignore SIGPIPE — prevents crash when writing to closed client socket */
    signal(SIGPIPE, SIG_IGN);

    /* install signal handler */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    server_log("SERVER_STARTED port=%d max_clients=%d timeout=%d ingredients=%d",
               tcp_port, max_clients, timeout_sec, num_ingredients);
    printf("Hogwarts is ready. Port: %d | Max Clients: %d | Timeout: %ds\n",
           tcp_port, max_clients, timeout_sec);
    fflush(stdout);

    /* ---------- main select loop ---------- */
    while (!shutdown_flag) {
        time_t now = time(NULL);

        /* check for timed-out clients */
        for (int i = 0; i < max_clients; i++) {
            if (clients[i].fd == -1) continue;
            int elapsed = (int)(now - clients[i].last_active);
            if (elapsed >= timeout_sec) {
                send_line(clients[i].fd, "TIMEOUT DISCONNECT");
                server_log("TIMEOUT username=%s fd=%d elapsed=%ds",
                           clients[i].enrolled ? clients[i].username : "(unknown)",
                           clients[i].fd, elapsed);
                disconnect_client(i, "timeout");
            }
        }

        /* calculate select timeout = minimum remaining time */
        struct timeval tv;
        tv.tv_sec = timeout_sec; /* default */
        tv.tv_usec = 0;
        now = time(NULL);
        for (int i = 0; i < max_clients; i++) {
            if (clients[i].fd == -1) continue;
            int remaining = timeout_sec - (int)(now - clients[i].last_active);
            if (remaining < 1) remaining = 1;
            if (remaining < (int)tv.tv_sec) tv.tv_sec = remaining;
        }

        /* build fd_set */
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listen_fd, &readfds);
        int maxfd = listen_fd;

        for (int i = 0; i < max_clients; i++) {
            if (clients[i].fd != -1) {
                FD_SET(clients[i].fd, &readfds);
                if (clients[i].fd > maxfd) maxfd = clients[i].fd;
            }
        }

        int ret = select(maxfd + 1, &readfds, NULL, NULL, &tv);
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        if (shutdown_flag) break;

        /* new connection? */
        if (FD_ISSET(listen_fd, &readfds)) {
            struct sockaddr_in caddr;
            socklen_t clen = sizeof(caddr);
            int cfd = accept(listen_fd, (struct sockaddr *)&caddr, &clen);
            if (cfd >= 0) {
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &caddr.sin_addr, ip, sizeof(ip));

                if (num_connected >= max_clients) {
                    /* server full */
                    send_line(cfd, "ERR HOGWARTS_FULL");
                    server_log("REJECTED fd=%d ip=%s reason=HOGWARTS_FULL", cfd, ip);
                    close(cfd);
                } else {
                    int slot = find_empty_slot();
                    if (slot >= 0) {
                        init_client(&clients[slot]);
                        clients[slot].fd = cfd;
                        clients[slot].last_active = time(NULL);
                        num_connected++;
                        server_log("CLIENT_CONNECTED fd=%d ip=%s", cfd, ip);
                    } else {
                        send_line(cfd, "ERR HOGWARTS_FULL");
                        server_log("REJECTED fd=%d ip=%s reason=HOGWARTS_FULL", cfd, ip);
                        close(cfd);
                    }
                }
            }
        }

        /* check client fds */
        for (int i = 0; i < max_clients; i++) {
            if (clients[i].fd != -1 && FD_ISSET(clients[i].fd, &readfds)) {
                handle_client_data(i);
            }
        }
    }

    /* ---------- shutdown ---------- */
    server_log("SHUTDOWN signal=SIGINT");

    for (int i = 0; i < max_clients; i++) {
        if (clients[i].fd != -1) {
            send_line(clients[i].fd, "SERVER SHUTDOWN");
            close(clients[i].fd);
            clients[i].fd = -1;
            num_connected--;
        }
    }

    server_log("CLEANUP_DONE clients=0");

    close(listen_fd);
    free(clients);
    if (log_fp) fclose(log_fp);

    return 0;
}
