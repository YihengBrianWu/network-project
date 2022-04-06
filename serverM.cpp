// Created by Yiheng Wu on 4/2/22.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <vector>
#include <sstream>

#define TCP_PORT_A "25959" // server's port for TCP connection to client A
#define TCP_PORT_B "26959" // server's port for TCP connection to client B
#define UDP_PORT "24959" // serverM's UDP port
#define UDP_PORT_A  "21959" // serverA's UDP port
#define HOST_NAME "127.0.0.1"

char send_buffer[1024];
char recv_buffer[1024];
int sockA;
int sock_UDP;
struct addrinfo *info_A;
struct addrinfo *info_UDP;
struct addrinfo *info_UDP_A;
struct sockaddr_in clientA_address;

int check_wallet(std::string user, bool from_client) {

    // check if the user exists in all blocks
    bool exist = false;

    // initial balance for all users are 1000
    int balance = 1000;

    sprintf(send_buffer, "%s", user.c_str());
    int len_send;
    int len_recv;

    // send to serverA
    len_send = sendto(sock_UDP, send_buffer, strlen(send_buffer), 0, info_UDP_A -> ai_addr, info_UDP_A -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send check wallet command to server A.");
    }
    else {
        if(from_client) {
            printf("The main server sent a request to server A.");
            printf("\n");
        }
        // clear recv buffer
        memset(recv_buffer, 0, sizeof(recv_buffer));
        len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_A -> ai_addr, &(info_UDP_A -> ai_addrlen));
        if (len_recv <= 0) {
            perror("Can't receive the result of check wallet command from server A.");
        }
        else {
            if (from_client) {
                printf("The main server received transactions from Server A using UDP over port %s.", UDP_PORT_A);
                printf("\n");
            }
            std::string recv_message(recv_buffer);
            if (recv_message != "NOTEXIST") {
                exist = true;
                balance += std::stoi(recv_message);
            }
        }
    }

    // user not exist
    if (!exist) {
        return INT_MIN;
    }

    return balance;

}

int main(int argc, char* argv[]) {

    // create sockets for client A
    sockA = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sockA == -1) {
        perror("Can't create socket that connected to client A");
        return -1;
    }

    // create sockets for UDP
    sock_UDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock_UDP == -1) {

        perror("Can't create UDP socket.");
        return -1;

    }

    // bind address and port for client A
    info_A = new addrinfo;
    info_A -> ai_family = AF_INET;
    info_A -> ai_socktype = SOCK_STREAM;
    getaddrinfo(HOST_NAME, TCP_PORT_A, 0, &info_A);

    if (bind(sockA, info_A -> ai_addr, info_A -> ai_addrlen) != 0) {
        perror("Can't bind sockA to the port");
        return -1;
    }

    // bind UDP socket
    info_UDP = new addrinfo;
    info_UDP -> ai_family = AF_INET;
    info_UDP -> ai_socktype = SOCK_DGRAM;
    getaddrinfo(HOST_NAME, UDP_PORT, 0, &info_UDP);

    if (bind(sock_UDP, info_UDP -> ai_addr, info_UDP -> ai_addrlen) != 0) {

        close(sockA);
        close(sock_UDP);
        perror("Can't bind UDP port.");
        return -1;

    }

    // get serverA address
    info_UDP_A = new addrinfo;
    info_UDP_A -> ai_family = AF_INET;
    info_UDP_A -> ai_socktype = SOCK_DGRAM;
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

        socklen_t client_A_len = sizeof(clientA_address);
        int childSocket_A = accept(sockA, (struct sockaddr*)&clientA_address, &client_A_len);
        if (childSocket_A == -1) {
            perror("Can't accept sockA");
        }
        else {
            memset(recv_buffer, 0, sizeof(recv_buffer));
            if (recv(childSocket_A, recv_buffer, sizeof(recv_buffer), 0) <= 0) {
                perror("Can't receive sockA");
            }
            else {
                if (strcmp(recv_buffer, "END") == 0) {
                    printf("main server down");
                    printf("\n");
                    close(childSocket_A);
                    break;
                }

                // convert char[] recv_buffer to string
                std::string message(recv_buffer);
                // split incoming message
                std::vector<std::string> split;
                std::stringstream stream(message);
                while(stream.good()) {
                    std::string substring;
                    std::getline(stream, substring, ',');
                    split.push_back(substring);
                }
                // TXLIST command
                if (split.at(0) == "TXLIST") {

                }
                // check wallet command
                else {
                    printf("The main server received input=\"%s\" from the client using TCP over port %d.", split.at(0).c_str(), clientA_address.sin_port);
                    printf("\n");
                    int result = check_wallet(message, true);
                    if (result == INT_MIN) {
                        printf("Username was not found on database.");
                        printf("\n");
                    }
                    else {
                        printf("The main server sent the current balance to client A.");
                        printf("\n");
                    }
                    sprintf(send_buffer, "%d", result);
                    int len_send = send(childSocket_A, send_buffer, strlen(send_buffer), 0);
                    if (len_send <= 0) {
                        perror("Can't send the result of check wallet command to client A.");
                    }
                }
//                printf("%s\n", buffer);
//                int len_send = sendto(sock_UDP, buffer, strlen(buffer), 0, info_UDP_A -> ai_addr, info_UDP -> ai_addrlen);
//                if (len_send <= 0) {
//                    perror("Can't send message to serverA");
//                }
                close(childSocket_A);
            }
        }
    }

    // close socket
    close(sockA);
    close(sock_UDP);
    return 0;

}