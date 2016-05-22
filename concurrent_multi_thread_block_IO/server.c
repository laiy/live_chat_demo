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
static void *boardcast(void *);

int main() {
    int listenfd, i;
    long connfd;
    pthread_t tid;
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
    for (;;) {
        clilen = sizeof(client_address);
        connfd = accept(listenfd, (struct sockaddr *)&client_address, &clilen);
        for (i = 0; i < MAX_CONNECTIONS; i++)
            if (client[i] < 0) {
                client[i] = connfd;
                break;
            }
        if (i == MAX_CONNECTIONS)
            error("too many clients");
        pthread_create(&tid, NULL, &boardcast, (void *)connfd);
    }
    return 0;
}

static void *boardcast(void *arg) {
    ssize_t n;
    char buf[MAXLINE];
    int j, tmpfd;
    pthread_detach(pthread_self());
    while ((n = read((long)arg, buf, MAXLINE)) > 0) {
        for (j = 0; j < MAX_CONNECTIONS; j++) {
            if ((tmpfd = client[j]) < 0 || (long)arg == tmpfd)
                continue;
            write(tmpfd, buf, n);
        }
    }
    close((long)arg);
    return NULL;
}

