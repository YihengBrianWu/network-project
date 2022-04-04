// Created by Yiheng Wu on 4/4/22.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

#define UDP_PORT_A "21959"
#define UDP_PORT_M "24959"
#define HOST_NAME "127.0.0.1"

char buffer[1024];

int main(int argc, char* argv[]) {

    // create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {

        perror("Can't create UDP socket.");
        return -1;

    }

    // create serverA address info
    struct addrinfo *info_server = new addrinfo;
    info_server->ai_family = AF_INET;
    info_server->ai_socktype = SOCK_DGRAM;
    getaddrinfo(HOST_NAME, UDP_PORT_A, 0, &info_server);

    // bind socket
    if (bind(sock, info_server -> ai_addr, info_server -> ai_addrlen) != 0) {
        close(sock);
        perror("Can't bind UDP socket with the port.");
        return -1;
    }

    // get serverM address info
    struct addrinfo *info_server_m = new addrinfo;
    info_server_m -> ai_family = AF_INET;
    info_server_m -> ai_socktype = SOCK_DGRAM;
    getaddrinfo(HOST_NAME, UDP_PORT_M, 0, &info_server_m);


    // boot up message
    printf("The ServerA is up and running using UDP on port %d.",atoi(UDP_PORT_A));
    printf("\n");

    // read and write
    while (true) { // because the server A need to be always online
        // every time before receiving message, we need to clear buffer
        memset(buffer, 0, sizeof(buffer));
        int receive_len = recvfrom(sock, buffer, sizeof(buffer), 0, info_server_m -> ai_addr, &(info_server_m -> ai_addrlen));
        if (receive_len <= 0) {
            perror("Can't receive message from serverM, something wrong here!");
            continue;
        }
        printf("The ServerA received a request from the Main Server.\n");
        std::string message(buffer);
        printf("%s", buffer);
        printf("\n");
    }

}
