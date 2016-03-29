#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include "awesome_handler.h"

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in server_address;
    if (argc != 2)
        error("usage: ./client <IP_address>");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("socket error");
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0)
        error("inet_pton error for %s", argv[1]);
    if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        error("connect error");
    chat_cli(stdin, sockfd);
    exit(0);
}

