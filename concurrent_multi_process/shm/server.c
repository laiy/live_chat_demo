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
#include <sys/ipc.h>
#include <sys/shm.h>

#define LISTENQ 5
#define MAX_CONNECTIONS 1024

int *client;

void boardcast(void *, int shmid);

int main() {
    int listenfd, i, shmid, pid;
    long connfd;
    socklen_t clilen;
    struct sockaddr_in client_address, server_address;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);
    bind(listenfd, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(listenfd, LISTENQ);
    if ((shmid = shmget(IPC_PRIVATE, MAX_CONNECTIONS * sizeof(int), IPC_CREAT | IPC_EXCL)) < 0)
        error("shm error");
    client = (int*)shmat(shmid, 0, 0);
    for (i = 0; i < MAX_CONNECTIONS; i++)
        client[i] = -1;
    for (;;) {
        clilen = sizeof(client_address);
        connfd = accept(listenfd, (struct sockaddr *)&client_address, &clilen);
        if (shmctl(shmid, SHM_LOCK, 0) < 0)
            error("shmctl error");
        for (i = 0; i < MAX_CONNECTIONS; i++)
            if (client[i] < 0) {
                client[i] = connfd;
                break;
            }
        if (shmctl(shmid, SHM_UNLOCK, 0) < 0)
            error("shmctl error");
        if (i == MAX_CONNECTIONS)
            error("too many clients");
        if ((pid = fork()) == 0) {
            close(listenfd);
            boardcast((void *)connfd, shmid);
            exit(0);
        } else
            continue;
    }
    if (shmctl(shmid, IPC_RMID, NULL) < 0)
        error("shmdt error");
    return 0;
}

void boardcast(void *arg, int shmid) {
    ssize_t n;
    char buf[MAXLINE];
    int j, tmpfd;
    pthread_detach(pthread_self());
    while ((n = read((long)arg, buf, MAXLINE)) > 0) {
        if (shmctl(shmid, SHM_LOCK, 0) < 0)
            error("shmctl error");
        for (j = 0; j < MAX_CONNECTIONS; j++) {
            if ((tmpfd = client[j]) < 0 || (long)arg == tmpfd)
                continue;
            printf("%d b\n", tmpfd);
            write(tmpfd, buf, n);
        }
        if (shmctl(shmid, SHM_UNLOCK, 0) < 0)
            error("shmctl error");
    }
    if (shmdt(client) < 0)
        error("shmdt error");
    close((long)arg);
}

