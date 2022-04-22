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
#include <string>
#include <vector>
#include <sstream>
#include <climits>

#define HOST_NAME "127.0.0.1"
#define TCP_PORT_A "25959"

char send_buffer[10240];
char recv_buffer[10240];

int sock;
struct addrinfo *info_server;

int main(int argc, char* argv[]) {

    // check argc number
    if (argc != 2 && argc != 3 && argc != 4) {

        printf("Wrong commands, can't match argc amount.");
        printf("\n");
        return -1;

    }

    // create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock == -1) {

        perror("Can't create socket");
        return -1;

    }

    // create server info addrinfo struct
    info_server = new addrinfo;
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
            sprintf(send_buffer, "TXLIST");
            int send_success = send(sock, send_buffer, strlen(send_buffer), 0);
            if (send_success <= 0) {
                perror("Error occurred in send phase.");
                return -1;
            }
            printf("ClientA sent a sorted list request to the main server.");
            printf("\n");
        }

        // END command, not in project, just test
        else if (strcmp(argv[1], "END") == 0) {
            sprintf(send_buffer, "END");
            int send_success = send(sock, send_buffer, strlen(send_buffer), 0);
            if (send_success <= 0) {
                perror("Error occurred in send phase.");
                return -1;
            }
            printf("ClientA sent a END request to the main server.");
            printf("\n");
        }

        // otherwise it is a check wallet command
        else {
            sprintf(send_buffer, "%s", argv[1]);
            int send_success = send(sock, send_buffer, strlen(send_buffer), 0);
            if (send_success <= 0) {
                perror("Error occurred in send phase.");
                return -1;
            }
            printf("\"%s\" sent a balance enquiry request to the main server.", argv[1]);
            printf("\n");
            // clear receive buffer
            memset(recv_buffer, 0, sizeof(recv_buffer));
            int len_recv = recv(sock, recv_buffer, sizeof(recv_buffer), 0);
            if (len_recv <= 0) {
                perror("Can't receive the result of check wallet command from server M.");
                return -1;
            }
            int result = atoi(recv_buffer);
            // user doesn't exist
            if (result == INT_MIN) {
                printf("Unable to proceed with the request as %s is not part of the network.", argv[1]);
                printf("\n");
            } else {
                printf("The current balance of \"%s\" is : %d alicoins.\n", argv[1], result);
            }
        }

    }

    // if there are 2 arguments <username> stats
    else if (argc == 3) {
        // send message to main server
        sprintf(send_buffer, "STAT,%s", argv[1]);
        int send_success = send(sock, send_buffer, strlen(send_buffer), 0);
        if (send_success <= 0) {
            perror("Error occurred in send phase.");
            return -1;
        }
        // print message
        printf("\"%s\" sent a statistics enquiry request to the main server.", argv[1]);
        printf("\n");

        // receive messages;
        int len_recv;
        memset(recv_buffer, 0, sizeof(recv_buffer));
        len_recv = recv(sock, recv_buffer, sizeof(recv_buffer), 0);
        if (len_recv <= 0) {
            perror("Can't receive the result length of statistics command from server M.");
            return -1;
        }

        // split incoming message
        std::string message(recv_buffer);

        if (message == "NOTEXIST") {
            printf("Unable to proceed with the STATS command as \"%s\" is not part of the network.", argv[1]);
            printf("\n");
        }
        else {
            std::vector<std::string> split;
            std::stringstream stream(message);
            while(stream.good()) {
                std::string substring;
                std::getline(stream, substring, ',');
                split.push_back(substring);
            }

            printf("%s statistics are the following.:\n",argv[1]);
            printf("Rank - Username - Transactions - Amount\n");

            for (int i = 1; i <= split.size(); i++) {
                printf("%d %s\n", i, split.at(i - 1).c_str());
            }
        }

    }

    // if there are 3 arguments -> user1 user2 amount -> user1 send money(amount) to user2
    else{
        sprintf(send_buffer, "TRANSFER,%s,%s,%s", argv[1], argv[2], argv[3]);
        int send_success = send(sock, send_buffer, strlen(send_buffer), 0);
        if (send_success <= 0) {
            perror("Error occurred in send phase.");
            return -1;
        }
        printf("\"%s\" has requested to transfer %s coins to \"%s\".", argv[1], argv[3], argv[2]);
        printf("\n");
        // receive message back from serverM
        // clear receiver buffer
        memset(recv_buffer, 0, sizeof(recv_buffer));
        int len_recv = recv(sock, recv_buffer, sizeof(recv_buffer), 0);
        if (len_recv <= 0) {
            perror("Can't receive the result of check wallet command from server M.");
            return -1;
        }
        std::string result(recv_buffer);
        // both not exist
        if (result == "BNEXIST") {
            printf("Unable to proceed with the transaction as \"%s\" and \"%s\" are not part of the network.", argv[1], argv[2]);
            printf("\n");
        }
        // sender not exist
        else if (result == "SNEXIST") {
            printf("Unable to proceed with the transaction as \"%s\" is not part of the network.",argv[1]);
            printf("\n");
        }
        // receiver not exist
        else if (result == "RNEXIST") {
            printf("Unable to proceed with the transaction as \"%s\" is not part of the network.",argv[2]);
            printf("\n");
        }
        // balance not enough
        else if (result.substr(0, 9) == "NOTENOUGH") {
            printf("\"%s\" was unable to transfer %s alicoins to \"%s\" because of insufficient balance. ", argv[1], argv[3], argv[2]);
            printf("The current balance of \"%s\" is : %s alicoins.", argv[1], result.substr(9).c_str());
            printf("\n");
        }
        // legal transaction
        else {
            printf("\"%s\" successfully transferred %s alicoins to \"%s\". ", argv[1], argv[3], argv[2]);
            printf("The current balance of \"%s\" is : %d alicoins.", argv[1], std::stoi(result));
            printf("\n");
        }
    }

    close(sock);

}
