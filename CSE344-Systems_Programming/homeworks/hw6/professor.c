/*
 * professor.c - Professor Client
 * CSE344 HW6 - Connects to Hogwarts server as a PROFESSOR
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_LINE 512

static volatile sig_atomic_t quit_flag = 0;

static void sigint_handler(int sig) {
    (void)sig;
    quit_flag = 1;
}

/* partial write helper */
static int send_all(int fd, const char *data, size_t len) {
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
    return send_all(fd, buf, (size_t)len);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <tcp_port> <username>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    const char *username = argv[3];

    /* connect to server */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return 1; }

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port   = htons((uint16_t)port);
    if (inet_pton(AF_INET, server_ip, &saddr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address: %s\n", server_ip);
        close(sockfd);
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    printf("[PROFESSOR %s] CONNECTED server=%s:%d\n", username, server_ip, port);
    fflush(stdout);

    /* install signal handler */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    /* auto-send ENROLL */
    char enroll_cmd[MAX_LINE];
    snprintf(enroll_cmd, sizeof(enroll_cmd), "ENROLL PROFESSOR %s", username);
    send_line(sockfd, enroll_cmd);
    printf("[PROFESSOR %s] SENT %s\n", username, enroll_cmd);
    fflush(stdout);

    /* main loop: multiplex stdin + server socket */
    char srv_buf[MAX_LINE + 1];
    int srv_buf_len = 0;
    int running = 1;
    int need_prompt = 1;
    int apparate_sent = 0;

    while (running) {
        if (quit_flag) {
            if (!apparate_sent) {
                send_line(sockfd, "APPARATE");
                printf("[PROFESSOR %s] SENT APPARATE\n", username);
                fflush(stdout);
                apparate_sent = 1;
            }

            char tmp[MAX_LINE];
            ssize_t n = read(sockfd, tmp, sizeof(tmp) - 1);
            if (n > 0) {
                tmp[n] = '\0';
                char *start = tmp;
                for (char *p = tmp; *p; p++) {
                    if (*p == '\n') {
                        *p = '\0';
                        if (strlen(start) > 0)
                            printf("[PROFESSOR %s] RECEIVED %s\n", username, start);
                        start = p + 1;
                    }
                }
                if (strlen(start) > 0)
                    printf("[PROFESSOR %s] RECEIVED %s\n", username, start);
            }
            printf("[PROFESSOR %s] DISCONNECTED reason=APPARATE\n", username);
            fflush(stdout);
            break;
        }

        if (need_prompt) {
            printf("> ");
            fflush(stdout);
            need_prompt = 0;
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sockfd, &readfds);
        int maxfd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ret = select(maxfd + 1, &readfds, NULL, NULL, &tv);
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        /* server data */
        if (FD_ISSET(sockfd, &readfds)) {
            char tmp[MAX_LINE];
            ssize_t n = read(sockfd, tmp, sizeof(tmp) - 1);
            if (n <= 0) {
                printf("[PROFESSOR %s] DISCONNECTED reason=hangup\n", username);
                fflush(stdout);
                break;
            }

            for (ssize_t i = 0; i < n; i++) {
                if (srv_buf_len < MAX_LINE)
                    srv_buf[srv_buf_len++] = tmp[i];

                if (tmp[i] == '\n') {
                    srv_buf[srv_buf_len] = '\0';
                    if (srv_buf_len > 0 && srv_buf[srv_buf_len - 1] == '\n')
                        srv_buf[--srv_buf_len] = '\0';

                    if (srv_buf_len > 0) {
                        printf("[PROFESSOR %s] RECEIVED %s\n", username, srv_buf);
                        fflush(stdout);

                        if (strcmp(srv_buf, "SERVER SHUTDOWN") == 0) {
                            printf("[PROFESSOR %s] DISCONNECTED reason=shutdown\n", username);
                            fflush(stdout);
                            running = 0;
                        } else if (strcmp(srv_buf, "TIMEOUT DISCONNECT") == 0) {
                            printf("[PROFESSOR %s] DISCONNECTED reason=timeout\n", username);
                            fflush(stdout);
                            running = 0;
                        } else if (strcmp(srv_buf, "ERR HOGWARTS_FULL") == 0) {
                            printf("[PROFESSOR %s] DISCONNECTED reason=full\n", username);
                            fflush(stdout);
                            running = 0;
                        }
                    }
                    srv_buf_len = 0;
                    if (running) need_prompt = 1;
                }
            }
            if (!running) break;
        }

        /* stdin data */
        if (running && FD_ISSET(STDIN_FILENO, &readfds)) {
            char line[MAX_LINE];
            if (fgets(line, sizeof(line), stdin) == NULL) {
                if (!apparate_sent) {
                    send_line(sockfd, "APPARATE");
                    printf("[PROFESSOR %s] SENT APPARATE\n", username);
                    printf("[PROFESSOR %s] DISCONNECTED reason=APPARATE\n", username);
                    fflush(stdout);
                }
                break;
            }
            size_t len = strlen(line);
            while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
                line[--len] = '\0';

            if (len > 0) {
                send_line(sockfd, line);
                printf("[PROFESSOR %s] SENT %s\n", username, line);
                fflush(stdout);
                if (strcmp(line, "APPARATE") == 0)
                    apparate_sent = 1;
                need_prompt = 0;
            } else {
                need_prompt = 1;
            }
        }
    }

    close(sockfd);
    return 0;
}
