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
#include <vector>
#include <sstream>
#include <fstream>

#define UDP_PORT_A "21959"
#define UDP_PORT_M "24959"
#define HOST_NAME "127.0.0.1"
#define FILE_NAME "block1.txt"

char buffer[1024];
int sock;
struct addrinfo *info_server;
struct addrinfo *info_server_m;


void check_wallet(std::string user) {

    // return value, how many coins should add or delete in this block
    int result = 0;

    // check if the user exist in this block
    bool exist = false;

    // open file
    std::ifstream infile;
    infile.open(FILE_NAME, std::ios::in);

    // get every line in the file
    std::string transaction;
    while(std::getline(infile, transaction)) {

        if (transaction.empty()) {
            continue;
        }

        std::vector<std::string> split;
        std::stringstream stream(transaction);
        while(stream.good()) {
            std::string substring;
            std::getline(stream, substring, ' ');
            split.push_back(substring);
        }

        // user is sender
        if (split.at(1) == user) {
            exist = true;
            result -= std::stoi(split.at(3));
        }
        // user is receiver
        else if (split.at(2) == user) {
            exist = true;
            result += std::stoi(split.at(3));
        }

    }

    // close and return the balance
    infile.close();
//    printf("%d\n", result);

    // if user not exist
    if (!exist) {
        sprintf(buffer, "NOTEXIST");
    }
    // if user exist
    else {
        sprintf(buffer, "%d", result);
    }

    // send to serverM
    int send_len = sendto(sock, buffer, strlen(buffer), 0, info_server_m -> ai_addr, info_server_m -> ai_addrlen);
    if (send_len <= 0) {
        perror("Can't send the result of check wallet method.");
    }

}

void largest_serial_number() {

    // largest serial number in this block
    int n = 0;

    // open file
    std::ifstream infile;
    infile.open(FILE_NAME, std::ios::in);

    // get every line in the file
    std::string transaction;
    while(std::getline(infile, transaction)) {

        // prevent there are more /n lines
        if (transaction.empty()) {
            continue;
        }

        // split the incoming line
        std::vector<std::string> split;
        std::stringstream stream(transaction);
        while(stream.good()) {
            std::string substring;
            std::getline(stream, substring, ' ');
            split.push_back(substring);
        }

        n = std::max(n, std::stoi(split.at(0)));

    }

    // close file
    infile.close();

    // send the largest serial number of this block to server M
    sprintf(buffer, "%d", n);
    int send_len = sendto(sock, buffer, strlen(buffer), 0, info_server_m -> ai_addr, info_server_m -> ai_addrlen);
    if (send_len <= 0) {
        perror("Can't send the result of serial number method.");
    }

}

void save_data(int serial_number, std::string sender, std::string receiver, int amount) {


    // input file
    std::ifstream input;
    input.open(FILE_NAME, std::ios::in);

    // vector to store lines get from file
    std::vector<std::string> messages;

    // get every line in the file
    std::string transaction;
    while(std::getline(input, transaction)) {
        // filter of blank lines
        if (transaction.empty()) {
            continue;
        }
        messages.push_back(transaction);
    }

    input.close();

    std::ofstream output;
    output.open(FILE_NAME, std::ios::out);

    for (int i = 0; i < messages.size(); i++) {

        output << messages.at(i) << std::endl;

    }

    output << serial_number << " " << sender << " " << receiver << " " << amount;

    sprintf(buffer, "Log Saved.");
    int len_send = sendto(sock, buffer, strlen(buffer), 0, info_server_m -> ai_addr, info_server_m -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send log saved successful message back to server M.");
    }

    output.close();

}

void sendAll() {

    // send length and receiver length
    int len_send;
    int len_recv;

    // load all data
    // input file
    std::ifstream input;
    input.open(FILE_NAME, std::ios::in);

    // vector to store lines get from file
    std::vector<std::string> messages;

    // get every line in the file
    std::string transaction;
    while(std::getline(input, transaction)) {
        // filter of blank lines
        if (transaction.empty()) {
            continue;
        }
        messages.push_back(transaction);
    }

    // send length of messages to server M;
    sprintf(buffer, "%lu", messages.size());
    len_send = sendto(sock, buffer, strlen(buffer), 0, info_server_m -> ai_addr, info_server_m -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send length of records to server M.");
    }
    else {
        for (int i = 0; i < messages.size(); i++) {
            sprintf(buffer, "%s", messages.at(i).c_str());
            len_send = sendto(sock, buffer, strlen(buffer), 0, info_server_m -> ai_addr, info_server_m -> ai_addrlen);
            if (len_send <= 0) {
                perror("Can't send record detail to server M.");
            }
        }
    }


}

int main(int argc, char* argv[]) {

    // create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {

        perror("Can't create UDP socket.");
        return -1;

    }

    // create serverA address info
    info_server = new addrinfo;
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
    info_server_m = new addrinfo;
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
        printf("The ServerA received a request from the Main Server.");
        printf("\n");
        std::string message(buffer);

        // split incoming message
        std::vector<std::string> split;
        std::stringstream stream(message);
        while(stream.good()) {
            std::string substring;
            std::getline(stream, substring, ',');
            split.push_back(substring);
        }

        // TXLIST command
        if (split.at(0) == "GET") {
            sendAll();
        }

        else if (split.at(0) == "SERIAL") {
            largest_serial_number();
        }

        else if (split.at(0) == "SAVE"){
            save_data(std::stoi(split.at(1)), split.at(2), split.at(3), std::stoi(split.at(4)));
        }

        // check wallet
        else {
            check_wallet(split.at(0));
        }


//        printf("%s", message.c_str());
//        printf("\n");
        printf("The ServerA finished sending the response to the Main Server.");
        printf("\n");
    }

}
