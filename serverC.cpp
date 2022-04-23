// Created by Yiheng Wu on 4/19/22.
#include <stdio.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>

#define UDP_PORT_C "23959"
#define UDP_PORT_M "24959"
#define HOST_NAME "127.0.0.1"
#define FILE_NAME "block3.txt"

char send_buffer[20480];
char recv_buffer[20480];
int sock;
struct addrinfo *info_server;
struct addrinfo *info_server_m;

/**
 * to check if this user exist in this server, if exist send balance to server M, if not exist send INT_MIN to serverN
 * @param user user name
 */
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

        // split every line by space
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

    // if user not exist
    if (!exist) {
        sprintf(send_buffer, "NOTEXIST");
    }
        // if user exist
    else {
        sprintf(send_buffer, "%d", result);
    }

    // send to serverM
    int send_len = sendto(sock, send_buffer, strlen(send_buffer), 0, info_server_m -> ai_addr, info_server_m -> ai_addrlen);
    if (send_len <= 0) {
        perror("Can't send the result of check wallet method.");
    }

}

/**
 * to find the largest serial number in this server
 */
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
    sprintf(send_buffer, "%d", n);
    int send_len = sendto(sock, send_buffer, strlen(send_buffer), 0, info_server_m -> ai_addr, info_server_m -> ai_addrlen);
    if (send_len <= 0) {
        perror("Can't send the result of serial number method.");
    }

}

/**
 * save transaction data in this server
 * @param serial_number serial number
 * @param sender sender name
 * @param receiver receiver name
 * @param amount transaction amount
 */
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

    output.close();

}

/**
 * send all transactions in this server to server M
 */
void send_all_transaction() {

    // send length
    int len_send;

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
    sprintf(send_buffer, "%lu", messages.size());
    len_send = sendto(sock, send_buffer, strlen(send_buffer), 0, info_server_m -> ai_addr, info_server_m -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send length of records to server M.");
    }
    else {
        for (int i = 0; i < messages.size(); i++) {
            sprintf(send_buffer, "%s", messages.at(i).c_str());
            len_send = sendto(sock, send_buffer, strlen(send_buffer), 0, info_server_m -> ai_addr, info_server_m -> ai_addrlen);
            if (len_send <= 0) {
                perror("Can't send record detail to server M.");
            }
        }
    }


}

/**
 * send distinct user's transaction in this server to serverM
 * @param name
 */
void send_distinct_transaction(std::string name) {

    // open file
    std::ifstream infile;
    infile.open(FILE_NAME, std::ios::in);

    // send length
    int len_send;

    // vector to store lines get from file
    std::vector<std::string> messages;

    // get every line in the file
    std::string transaction;
    while(std::getline(infile, transaction)) {

        if (transaction.empty()) {
            continue;
        }

        // split by space
        std::vector<std::string> split;
        std::stringstream stream(transaction);
        while(stream.good()) {
            std::string substring;
            std::getline(stream, substring, ' ');
            split.push_back(substring);
        }

        // if name is user or sender, then we store this record
        if (split.at(1) == name || split.at(2) == name) {
            messages.push_back(transaction);
        }

    }

    // send the length of record, then send all transactions to server M
    // send length of messages to server M;
    sprintf(send_buffer, "%lu", messages.size());
    len_send = sendto(sock, send_buffer, strlen(send_buffer), 0, info_server_m -> ai_addr, info_server_m -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send length of records to server M.");
    }
    else {
        for (int i = 0; i < messages.size(); i++) {
            sprintf(send_buffer, "%s", messages.at(i).c_str());
            len_send = sendto(sock, send_buffer, strlen(send_buffer), 0, info_server_m -> ai_addr, info_server_m -> ai_addrlen);
            if (len_send <= 0) {
                perror("Can't send record detail to server M.");
            }
        }
    }

    // close and return the balance
    infile.close();

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
    getaddrinfo(HOST_NAME, UDP_PORT_C, 0, &info_server);

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
    printf("The ServerA is up and running using UDP on port %s.", UDP_PORT_C);
    printf("\n");

    // read and write
    while (true) { // because the server A need to be always online

        // every time before receiving message, we need to clear buffer
        memset(recv_buffer, 0, sizeof(recv_buffer));
        int receive_len = recvfrom(sock, recv_buffer, sizeof(recv_buffer), 0, info_server_m -> ai_addr, &(info_server_m -> ai_addrlen));
        if (receive_len <= 0) {
            perror("Can't receive message from server M, something wrong here!");
            continue;
        }
        printf("The Server C received a request from the Main Server.");
        printf("\n");
        std::string message(recv_buffer);

        // split incoming message
        std::vector<std::string> split;
        std::stringstream stream(message);
        while(stream.good()) {
            std::string substring;
            std::getline(stream, substring, ',');
            split.push_back(substring);
        }

        // get record
        if (split.at(0) == "GET") {
            send_all_transaction();
        }

            // get record by name
        else if (split.at(0) == "GETDISTINCT") {
            send_distinct_transaction(split.at(1));
        }

            // get serial number
        else if (split.at(0) == "SERIAL") {
            largest_serial_number();
        }

            // save log
        else if (split.at(0) == "SAVE"){
            save_data(std::stoi(split.at(1)), split.at(2), split.at(3), std::stoi(split.at(4)));
        }

            // check wallet
        else {
            check_wallet(split.at(0));
        }


//        printf("%s", message.c_str());
//        printf("\n");
        printf("The Server C finished sending the response to the Main Server.");
        printf("\n");
    }

}
