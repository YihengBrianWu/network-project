// Created by Yiheng Wu on 4/2/22.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define HOST_NAME "127.0.0.1"
#define TCP_PORT_A "25959"

char buffer_clientA[1024];

int main(int argc, char* argv[]) {

    // create socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock == -1) {

        perror("Can't create socket");
        return -1;

    }

    // create server info addrinfo struct
    struct addrinfo *info_server;
    info_server -> ai_family = AF_INET;
    info_server -> ai_socktype = SOCK_STREAM;
    getaddrinfo(HOST_NAME, TCP_PORT_A, 0, &info_server);

    // boost up message
    printf("The client A is up and running.");
    printf("\n");

    // connect part
    if (connect(sock, info_server -> ai_addr, info_server -> ai_addrlen) != 0) {

        perror("Can't connect");
        close(sock);
        return -1;

    }

    // read and write
    sprintf(buffer_clientA, "hello from client A");
    if (send(sock, buffer_clientA, strlen(buffer_clientA), 0) <= 0) {

        perror("Can't send to server");
        close(sock);
        return -1;

    }
    close(sock);


    // create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock == -1) {

        perror("Can't create socket");
        return -1;

    }

    // boost up message
    printf("The client A is up and running.");
    printf("\n");

    // connect part
    if (connect(sock, info_server -> ai_addr, info_server -> ai_addrlen) != 0) {

        perror("Can't connect");
        close(sock);
        return -1;

    }

    memset(buffer_clientA,0,sizeof(buffer_clientA));
    sprintf(buffer_clientA, "END");
    if (send(sock, buffer_clientA, strlen(buffer_clientA), 0) <= 0) {

        perror("Can't send to server");
        close(sock);
        return -1;

    }

    close(sock);
    return 0;
}
