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
#define UDP_PORT "24959" // server's UDP port
#define HOST_NAME "127.0.0.1"

char buffer[1024];

int main(int argc, char* argv[]) {

    // create sockets for client A
    int sockA = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sockA == -1) {
        perror("Can't create socket that connected to client A");
        return -1;
    }

    // bind address and port for client A
    struct addrinfo *info_A;
    info_A -> ai_family = AF_INET;
    info_A -> ai_socktype = SOCK_STREAM;
    getaddrinfo(HOST_NAME, TCP_PORT_A, 0, &info_A);

    if (bind(sockA, info_A -> ai_addr, info_A -> ai_addrlen) != 0) {
        perror("Can't bind sockA to the port");
        return -1;
    }

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
        } else {
            memset(buffer, 0, sizeof(buffer));
            if (recv(childSocket_A, buffer, sizeof(buffer), 0) <= 0) {
                perror("Can't receive sockA");
            } else {
                if (strcmp(buffer, "END") == 0) {
                    printf("main server down");
                    printf("\n");
                    close(childSocket_A);
                    break;
                }
                printf("%s\n", buffer);
                close(childSocket_A);
            }
        }
    }

    // close socket
    close(sockA);
    return 0;

}