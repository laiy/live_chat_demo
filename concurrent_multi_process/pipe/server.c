#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "awesome_handler.h"

#define LISTENQ 5
#define MAX_CONNECTIONS 1024

int client[MAX_CONNECTIONS];
int pipes[MAX_CONNECTIONS][2];

static void boardcast(long connfd, int i);

int main() {
    int listenfd, i, pid, nready, maxfd, maxi, sockfd, j, tmpfd, n;
    fd_set rset, allset;
    long connfd;
    char buf[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in client_address, server_address;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);
    bind(listenfd, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(listenfd, LISTENQ);
    for (i = 0; i < MAX_CONNECTIONS; i++)
        client[i] = -1;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    maxfd = listenfd;
    maxi = -1;
    for (;;) {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(listenfd, &rset)) {
            clilen = sizeof(client_address);
            connfd = accept(listenfd, (struct sockaddr *)&client_address, &clilen);
            for (i = 0; i < MAX_CONNECTIONS; i++)
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            if (i == MAX_CONNECTIONS)
                error("too many clients");
            if (i > maxi)
                maxi = i;
            pipe(pipes[i]);
            if ((pid = fork()) == 0) {
                close(listenfd);
                close(pipes[i][0]);
                boardcast(connfd, i);
                exit(0);
            } else {
                close(pipes[i][1]);
                FD_SET(pipes[i][0], &allset);
                if (pipes[i][0] > maxfd)
                    maxfd = pipes[i][0];
                continue;
            }
        }
        for (i = 0; i <= maxi; i++) {
            if ((sockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(pipes[i][0], &rset)) {
                if ((n = read(pipes[i][0], buf, MAXLINE)) == 0) {
                    close(pipes[i][0]);
                    close(client[i]);
                    FD_CLR(pipes[i][0], &allset);
                    client[i] = -1;
                } else {
                    for (j = 0; j <= maxi; j++) {
                        if ((tmpfd = client[j]) < 0 || sockfd == tmpfd)
                            continue;
                        write(tmpfd, buf, n);
                    }
                }
            }
        }
    }
    return 0;
}

void boardcast(long connfd, int i) {
    ssize_t n;
    char buf[MAXLINE];
    while ((n = read(connfd, buf, MAXLINE)) > 0)
        write(pipes[i][1], buf, n);
    close(connfd);
}

