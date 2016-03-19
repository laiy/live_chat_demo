#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define MAXLINE 1024
#define SERVER_PORT 8000
#define max(x, y) x > y ? x : y

void *copyto(void *);
static int global_sockfd;
static FILE *global_fp;

void error(const char *fmt, ...) {
    va_list ptr;
    va_start(ptr, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, ptr);
    fprintf(stderr, "\n");
    va_end(ptr);
    exit(1);
}

void chat_cli(FILE *fp_arg, int sockfd_arg) {
    int n;
    char recvline[MAXLINE];
    pthread_t tid;
    global_sockfd = sockfd_arg;
    global_fp = fp_arg;
    pthread_create(&tid, NULL, copyto, NULL);
    while ((n = read(global_sockfd, recvline, MAXLINE)) > 0)
        recvline[n] = '\n', recvline[n + 1] = '\0', write(fileno(stdout), recvline, n + 1);
}

void *copyto(void *arg) {
    int namelen, n;
    char sendline[MAXLINE], name[MAXLINE + MAXLINE];
    fputs("WHO R U: ", stdout);
    scanf("%s", name);
    namelen = strlen(name);
    name[namelen] = ':';
    name[namelen + 1] = '\0';
    while ((n = read(fileno(global_fp), sendline, MAXLINE)) > 0)
        write(global_sockfd, strcat(name, sendline), n + namelen), name[namelen + 1] = '\0';
    shutdown(global_sockfd, SHUT_WR);
    return NULL;
}

