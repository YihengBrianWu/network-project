// Created by Yiheng Wu on 4/2/22.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define TCP_PORT_A "25959" // server's port for TCP connection to client A
#define TCP_PORT_B "26959" // server's port for TCP connection to client B
#define UDP_PORT "24959" // serverM's UDP port
#define UDP_PORT_A  "21959" // serverA's UDP port
#define HOST_NAME "127.0.0.1"

char buffer[1024];

int main(int argc, char* argv[]) {

    // create sockets for client A
    int sockA = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sockA == -1) {
        perror("Can't create socket that connected to client A");
        return -1;
    }

    // create sockets for UDP
    int sock_UDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock_UDP == -1) {

        perror("Can't create UDP socket.");
        return -1;

    }

    // bind address and port for client A
    struct addrinfo *info_A = new addrinfo;
    info_A -> ai_family = AF_INET;
    info_A -> ai_socktype = SOCK_STREAM;
    getaddrinfo(HOST_NAME, TCP_PORT_A, 0, &info_A);

    if (bind(sockA, info_A -> ai_addr, info_A -> ai_addrlen) != 0) {
        perror("Can't bind sockA to the port");
        return -1;
    }

    // bind UDP socket
    struct addrinfo *info_UDP = new addrinfo;
    info_UDP -> ai_family = AF_INET;
    info_UDP -> ai_socktype = SOCK_STREAM;
    getaddrinfo(HOST_NAME, UDP_PORT, 0, &info_UDP);

    if (bind(sock_UDP, info_UDP -> ai_addr, info_UDP -> ai_addrlen) != 0) {

        close(sockA);
        close(sock_UDP);
        perror("Can't bind UDP port.");
        return -1;

    }

    // get serverA address
    struct addrinfo *info_UDP_A = new addrinfo;
    info_UDP -> ai_family = AF_INET;
    info_UDP -> ai_socktype = SOCK_STREAM;
    getaddrinfo(HOST_NAME, UDP_PORT_A, 0, &info_UDP_A);


    // listen to sockA
    if (listen(sockA, 10) != 0) {
        perror("Can't listen sockA!");
        close(sockA);
        return -1;
    }

    // boost up message
    printf("The main server is up and running.");
    printf("\n");

    // read and write message
    while(1) {
        struct sockaddr_in clientA_address;
        socklen_t client_A_len = sizeof(clientA_address);
        int childSocket_A = accept(sockA, (struct sockaddr*)&clientA_address, &client_A_len);
        if (childSocket_A == -1) {
            perror("Can't accept sockA");
        }
        else {
            memset(buffer, 0, sizeof(buffer));
            if (recv(childSocket_A, buffer, sizeof(buffer), 0) <= 0) {
                perror("Can't receive sockA");
            }
            else {
                if (strcmp(buffer, "END") == 0) {
                    printf("main server down");
                    printf("\n");
                    close(childSocket_A);
                    break;
                }
                printf("%s\n", buffer);
                int len_send = sendto(sock_UDP, buffer, strlen(buffer), 0, info_UDP_A -> ai_addr, info_UDP -> ai_addrlen);
                if (len_send <= 0) {
                    perror("Can't send message to serverA");
                }
                close(childSocket_A);
            }
        }
    }

    // close socket
    close(sockA);
    close(sock_UDP);
    return 0;

}