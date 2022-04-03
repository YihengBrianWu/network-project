// Created by Yiheng Wu on 4/2/22.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

#define HOST_NAME "127.0.0.1"
#define TCP_PORT_A "25959"

using namespace std;

char buffer_clientA[1024];

int main(int argc, char* argv[]) {

    // check argc number
    if (argc != 2 && argc != 3 && argc != 4) {

        printf("Wrong commands, can't match argc amount.");
        printf("\n");
        return -1;

    }

    // create socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock == -1) {

        perror("Can't create socket");
        return -1;

    }

    // create server info addrinfo struct
    struct addrinfo *info_server = new addrinfo;
    info_server->ai_family = AF_INET;
    info_server->ai_socktype = SOCK_STREAM;
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

    // if there is 1 argument
    if (argc == 2) {

        // TXLIST situation
        if (strcmp(argv[1], "TXLIST") == 0) {
            sprintf(buffer_clientA, "TXLIST");
            int send_success = send(sock, buffer_clientA, strlen(buffer_clientA), 0);
            if (send_success <= 0) {
                perror("Error occurred in send phase.");
                return -1;
            }
            printf("clientA sent a sorted list request to the main server.");
            printf("\n");
        }

        // END command, not in project, just test
        else if (strcmp(argv[1], "END") == 0) {
            sprintf(buffer_clientA, "END");
            int send_success = send(sock, buffer_clientA, strlen(buffer_clientA), 0);
            if (send_success <= 0) {
                perror("Error occurred in send phase.");
                return -1;
            }
            printf("clientA sent a END request to the main server.");
            printf("\n");
        }

        // otherwise it is a check wallet command
        else {
            sprintf(buffer_clientA, "%s", argv[1]);
            int send_success = send(sock, buffer_clientA, strlen(buffer_clientA), 0);
            if (send_success <= 0) {
                perror("Error occurred in send phase.");
                return -1;
            }
            printf("clientA sent a check wallet request to the main server.");
            printf("\n");
        }

    }

    // if there are 2 arguments <username> stats
    else if (argc == 3) {
        // TODO transaction stats
    }

    // if there are 3 arguments -> user1 user2 amount -> user1 send money(amount) to user2
    else{
        sprintf(buffer_clientA, "TRANSFER,%s,%s,%s", argv[1], argv[2], argv[3]);
        int send_success = send(sock, buffer_clientA, strlen(buffer_clientA), 0);
        if (send_success <= 0) {
            perror("Error occurred in send phase.");
            return -1;
        }
        printf("%s has requested to transfer %s coins to %s.", argv[1], argv[3], argv[2]);
        printf("\n");
    }

    close(sock);

}
