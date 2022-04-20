// Created by Yiheng Wu on 4/2/22.
#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <vector>
#include <climits>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>

#define TCP_PORT_A "25959" // server's port for TCP connection to client A
#define TCP_PORT_B "26959" // server's port for TCP connection to client B
#define UDP_PORT "24959" // serverM's UDP port
#define UDP_PORT_A  "21959" // serverA's UDP port
#define UDP_PORT_B "22959" // serverB's UDP port
#define UDP_PORT_C "23959" // serverC's UDP port
#define HOST_NAME "127.0.0.1"
#define OUTPUT_FILE "alichain.txt"

char send_buffer[1024];
char recv_buffer[1024];
int sockA;
int sockB;
int sock_UDP;
int childSocket_A;
int childSocket_B;
struct addrinfo *info_A;
struct addrinfo *info_B;
struct addrinfo *info_UDP;
struct addrinfo *info_UDP_A;
struct addrinfo *info_UDP_B;
struct addrinfo *info_UDP_C;
struct sockaddr_in clientA_address;
struct sockaddr_in clientB_address;

// data structure for transaction
struct transaction {
    int serial;
    std::string sender;
    std::string receiver;
    int amount;
};

// data structure for statistics
struct statistics {
    int transactions;
    int amount;
};

// data structure for statistics with name
struct statistics_main {
    int transactions;
    int amount;
    std::string name;
};

/**
 *
 * @param user user name
 * @param from_client if it's a check wallet command from client or it's used by serverM to get the balance
 * @return the balance of user, INT_MIN means this user doesn't exist
 */
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
        // this function is called by clientA, not for sever M to check balance
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
                printf("The main server received transactions from Server A using UDP over port %s.", UDP_PORT);
                printf("\n");
            }
            std::string recv_message(recv_buffer);
            if (recv_message != "NOTEXIST") {
                exist = true;
                balance += std::stoi(recv_message);
            }
        }
    }

    // send to serverB
    len_send = sendto(sock_UDP, send_buffer, strlen(send_buffer), 0, info_UDP_B -> ai_addr, info_UDP_B -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send check wallet command to server B.");
    }
    else {
        // this function is called by clientA, not for sever M to check balance
        if(from_client) {
            printf("The main server sent a request to server B.");
            printf("\n");
        }
        // clear recv buffer
        memset(recv_buffer, 0, sizeof(recv_buffer));
        len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_B -> ai_addr, &(info_UDP_B -> ai_addrlen));
        if (len_recv <= 0) {
            perror("Can't receive the result of check wallet command from server B.");
        }
        else {
            if (from_client) {
                printf("The main server received transactions from Server B using UDP over port %s.", UDP_PORT_B);
                printf("\n");
            }
            std::string recv_message(recv_buffer);
            if (recv_message != "NOTEXIST") {
                exist = true;
                balance += std::stoi(recv_message);
            }
        }
    }

    // send to serverC
    len_send = sendto(sock_UDP, send_buffer, strlen(send_buffer), 0, info_UDP_C -> ai_addr, info_UDP_C -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send check wallet command to server C.");
    }
    else {
        // this function is called by clientA, not for sever M to check balance
        if(from_client) {
            printf("The main server sent a request to server C.");
            printf("\n");
        }
        // clear recv buffer
        memset(recv_buffer, 0, sizeof(recv_buffer));
        len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_C -> ai_addr, &(info_UDP_C -> ai_addrlen));
        if (len_recv <= 0) {
            perror("Can't receive the result of check wallet command from server C.");
        }
        else {
            if (from_client) {
                printf("The main server received transactions from Server C using UDP over port %s.", UDP_PORT_C);
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

/**
 *
 * @param sender sender name
 * @param receiver receiver name
 * @param amount transfer amount
 * @return a message to indicate if transfer can happen.
 */
std::string transfer_money (std::string sender, std::string receiver, int amount) {

    printf("The main server sent a request to server A.");
    printf("\n");

    // first let's check the balance of sender and receiver
    int sender_balance = check_wallet(sender, false);
    int receiver_balance = check_wallet(receiver, false);

    // after querying both sender and receiver's feedback from three end servers.
    printf("The main server received the feedback from server A using UDP over port %s.", UDP_PORT);
    printf("\n");
    printf("The main server received the feedback from server B using UDP over port %s.", UDP_PORT);
    printf("\n");
    printf("The main server received the feedback from server C using UDP over port %s.", UDP_PORT);
    printf("\n");

    // all failed cases
    if (sender_balance == INT_MIN && receiver_balance == INT_MIN) {
        return "BNEXIST"; // both not exist in the network
    }
    if (sender_balance == INT_MIN) {
        return "SNEXIST"; // sender not exist in the network
    }
    if (receiver_balance == INT_MIN) {
        return "RNEXIST"; // receiver not exist in the network
    }
    if (sender_balance < amount) {
        return "NOTENOUGH" + std::to_string(sender_balance); // balance is not enough to complete this transfer
    }

    // the transfer can be done when program arrive this position and we need to request the serial number;
    int serial_number = 0;
    int len_send;
    int len_recv;
    sprintf(send_buffer, "SERIAL");

    // request the serial number from server A
    len_send = sendto(sock_UDP, send_buffer, strlen(send_buffer), 0, info_UDP_A -> ai_addr, info_UDP_A -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send SERIAL command to server A.");
    }
    else {
        // clear recv buffer
        memset(recv_buffer, 0, sizeof(recv_buffer));
        len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_A -> ai_addr, &(info_UDP_A -> ai_addrlen));
        if (len_recv <= 0) {
            perror("Can't receive the result of TXCOINS command from server A.");
        }
        else {
            serial_number = std::max(serial_number, atoi(recv_buffer));
        }
    }

    // request the serial number from server B
    len_send = sendto(sock_UDP, send_buffer, strlen(send_buffer), 0, info_UDP_B -> ai_addr, info_UDP_B -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send SERIAL command to server B.");
    }
    else {
        // clear recv buffer
        memset(recv_buffer, 0, sizeof(recv_buffer));
        len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_B -> ai_addr, &(info_UDP_B -> ai_addrlen));
        if (len_recv <= 0) {
            perror("Can't receive the result of TXCOINS command from server B.");
        }
        else {
            serial_number = std::max(serial_number, atoi(recv_buffer));
        }
    }

    serial_number += 1;

    // after get the serial number, write it to the random backend server
    // get random number from range 0 - 2
    int random = rand() % 3;
    // write to server A
    if (random == 0) {
        sprintf(send_buffer, "SAVE,%d,%s,%s,%d", serial_number, sender.c_str(), receiver.c_str(), amount);
        len_send = sendto(sock_UDP, send_buffer, strlen(send_buffer), 0, info_UDP_A -> ai_addr, info_UDP_A -> ai_addrlen);
        if (len_send <= 0) {
            perror("Can't send transfer data to Server A.");
        }
    }
    // write to server B
    else if (random == 1) {
        sprintf(send_buffer, "SAVE,%d,%s,%s,%d", serial_number, sender.c_str(), receiver.c_str(), amount);
        len_send = sendto(sock_UDP, send_buffer, strlen(send_buffer), 0, info_UDP_B -> ai_addr, info_UDP_B -> ai_addrlen);
        if (len_send <= 0) {
            perror("Can't send transfer data to Server A.");
        }
    }
    // write to server C
    else {
        sprintf(send_buffer, "SAVE,%d,%s,%s,%d", serial_number, sender.c_str(), receiver.c_str(), amount);
        len_send = sendto(sock_UDP, send_buffer, strlen(send_buffer), 0, info_UDP_C -> ai_addr, info_UDP_C -> ai_addrlen);
        if (len_send <= 0) {
            perror("Can't send transfer data to Server A.");
        }
    }

    return std::to_string(sender_balance - amount);

}

// compare method for struct transaction, DESC by serial number
bool record_compare(transaction a, transaction b) {
    return a.serial < b.serial;
}

// compare method for struct statistics_main, DESC by number of transactions
bool stats_compare(statistics_main a, statistics_main b) {
    return a.transactions > b.transactions;
}

/**
 * get all records from all data server, for sorted lists command
 * @param records vector to store records
 */
void get_record_from_all_server(std::vector<transaction>& records) {

    // send length and receive length
    int len_send;
    int len_recv;

    // get records from server A
    sprintf(send_buffer, "GET");
    len_send = sendto(sock_UDP, send_buffer, strlen(send_buffer), 0, info_UDP_A -> ai_addr, info_UDP_A -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send GET command to server A.");
    }
    else {
        // clear buffer
        memset(recv_buffer, 0, sizeof(recv_buffer));
        // get length of record from server A
        len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_A -> ai_addr, &(info_UDP_A -> ai_addrlen));
        if (len_recv <= 0) {
            perror("Can't get length of record from server A.");
        }
        else {
            int length = atoi(recv_buffer);
            for (int i = 0; i < length; i++) {
                // clear buffer
                memset(recv_buffer, 0, sizeof(recv_buffer));
                len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_A -> ai_addr, &(info_UDP_A -> ai_addrlen));
                if (len_recv <= 0) {
                    perror("Can't get record detail from server A.");
                }
                else {
                    // split record by ' '
                    std::string detail(recv_buffer);
                    std::vector<std::string> split;
                    std::stringstream stream(detail);
                    while(stream.good()) {
                        std::string substring;
                        std::getline(stream, substring, ' ');
                        split.push_back(substring);
                    }
                    transaction t;
                    t.serial = std::stoi(split.at(0));
                    t.sender = split.at(1);
                    t.receiver = split.at(2);
                    t.amount = std::stoi(split.at(3));
                    records.push_back(t);
                }
            }
        }
    }

    // get records from server B
    sprintf(send_buffer, "GET");
    len_send = sendto(sock_UDP, send_buffer, strlen(send_buffer), 0, info_UDP_B -> ai_addr, info_UDP_B -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send GET command to server B.");
    }
    else {
        // clear buffer
        memset(recv_buffer, 0, sizeof(recv_buffer));
        // get length of record from server A
        len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_B -> ai_addr, &(info_UDP_B -> ai_addrlen));
        if (len_recv <= 0) {
            perror("Can't get length of record from server B.");
        }
        else {
            int length = atoi(recv_buffer);
            for (int i = 0; i < length; i++) {
                // clear buffer
                memset(recv_buffer, 0, sizeof(recv_buffer));
                len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_B -> ai_addr, &(info_UDP_B -> ai_addrlen));
                if (len_recv <= 0) {
                    perror("Can't get record detail from server B.");
                }
                else {
                    // split record by ' '
                    std::string detail(recv_buffer);
                    std::vector<std::string> split;
                    std::stringstream stream(detail);
                    while(stream.good()) {
                        std::string substring;
                        std::getline(stream, substring, ' ');
                        split.push_back(substring);
                    }
                    transaction t;
                    t.serial = std::stoi(split.at(0));
                    t.sender = split.at(1);
                    t.receiver = split.at(2);
                    t.amount = std::stoi(split.at(3));
                    records.push_back(t);
                }
            }
        }
    }

    // get records from server C
    sprintf(send_buffer, "GET");
    len_send = sendto(sock_UDP, send_buffer, strlen(send_buffer), 0, info_UDP_C -> ai_addr, info_UDP_C -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send GET command to server C.");
    }
    else {
        // clear buffer
        memset(recv_buffer, 0, sizeof(recv_buffer));
        // get length of record from server A
        len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_C -> ai_addr, &(info_UDP_C -> ai_addrlen));
        if (len_recv <= 0) {
            perror("Can't get length of record from server C.");
        }
        else {
            int length = atoi(recv_buffer);
            for (int i = 0; i < length; i++) {
                // clear buffer
                memset(recv_buffer, 0, sizeof(recv_buffer));
                len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_C -> ai_addr, &(info_UDP_C -> ai_addrlen));
                if (len_recv <= 0) {
                    perror("Can't get record detail from server A.");
                }
                else {
                    // split record by ' '
                    std::string detail(recv_buffer);
                    std::vector<std::string> split;
                    std::stringstream stream(detail);
                    while(stream.good()) {
                        std::string substring;
                        std::getline(stream, substring, ' ');
                        split.push_back(substring);
                    }
                    transaction t;
                    t.serial = std::stoi(split.at(0));
                    t.sender = split.at(1);
                    t.receiver = split.at(2);
                    t.amount = std::stoi(split.at(3));
                    records.push_back(t);
                }
            }
        }
    }

}

/**
 * get distinct user's records from all server
 * @param records vector to store records
 * @param name user name
 */
void get_distinct_record_from_all_server(std::vector<transaction>& records, std::string name) {

    // send length and receive length
    int len_send;
    int len_recv;

    // get records from server A
    sprintf(send_buffer, "GETDISTINCT,%s", name.c_str());
    len_send = sendto(sock_UDP, send_buffer, strlen(send_buffer), 0, info_UDP_A -> ai_addr, info_UDP_A -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send GET command to server A.");
    }
    else {
        // clear buffer
        memset(recv_buffer, 0, sizeof(recv_buffer));
        // get length of record from server A
        len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_A -> ai_addr, &(info_UDP_A -> ai_addrlen));
        if (len_recv <= 0) {
            perror("Can't get length of record from server A.");
        }
        else {
            int length = atoi(recv_buffer);
            for (int i = 0; i < length; i++) {
                // clear buffer
                memset(recv_buffer, 0, sizeof(recv_buffer));
                len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_A -> ai_addr, &(info_UDP_A -> ai_addrlen));
                if (len_recv <= 0) {
                    perror("Can't get record detail from server A.");
                }
                else {
                    // split record by ' '
                    std::string detail(recv_buffer);
                    std::vector<std::string> split;
                    std::stringstream stream(detail);
                    while(stream.good()) {
                        std::string substring;
                        std::getline(stream, substring, ' ');
                        split.push_back(substring);
                    }
                    transaction t;
                    t.serial = std::stoi(split.at(0));
                    t.sender = split.at(1);
                    t.receiver = split.at(2);
                    t.amount = std::stoi(split.at(3));
                    records.push_back(t);
                }
            }
        }
    }

    // get records from server B
    sprintf(send_buffer, "GETDISTINCT,%s", name.c_str());
    len_send = sendto(sock_UDP, send_buffer, strlen(send_buffer), 0, info_UDP_B -> ai_addr, info_UDP_B -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send GET command to server A.");
    }
    else {
        // clear buffer
        memset(recv_buffer, 0, sizeof(recv_buffer));
        // get length of record from server A
        len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_B -> ai_addr, &(info_UDP_B -> ai_addrlen));
        if (len_recv <= 0) {
            perror("Can't get length of record from server B.");
        }
        else {
            int length = atoi(recv_buffer);
            for (int i = 0; i < length; i++) {
                // clear buffer
                memset(recv_buffer, 0, sizeof(recv_buffer));
                len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_B -> ai_addr, &(info_UDP_B -> ai_addrlen));
                if (len_recv <= 0) {
                    perror("Can't get record detail from server B.");
                }
                else {
                    // split record by ' '
                    std::string detail(recv_buffer);
                    std::vector<std::string> split;
                    std::stringstream stream(detail);
                    while(stream.good()) {
                        std::string substring;
                        std::getline(stream, substring, ' ');
                        split.push_back(substring);
                    }
                    transaction t;
                    t.serial = std::stoi(split.at(0));
                    t.sender = split.at(1);
                    t.receiver = split.at(2);
                    t.amount = std::stoi(split.at(3));
                    records.push_back(t);
                }
            }
        }
    }

    // get records from server C
    sprintf(send_buffer, "GETDISTINCT,%s", name.c_str());
    len_send = sendto(sock_UDP, send_buffer, strlen(send_buffer), 0, info_UDP_C -> ai_addr, info_UDP_C -> ai_addrlen);
    if (len_send <= 0) {
        perror("Can't send GET command to server C.");
    }
    else {
        // clear buffer
        memset(recv_buffer, 0, sizeof(recv_buffer));
        // get length of record from server A
        len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_C -> ai_addr, &(info_UDP_C -> ai_addrlen));
        if (len_recv <= 0) {
            perror("Can't get length of record from server C.");
        }
        else {
            int length = atoi(recv_buffer);
            for (int i = 0; i < length; i++) {
                // clear buffer
                memset(recv_buffer, 0, sizeof(recv_buffer));
                len_recv = recvfrom(sock_UDP, recv_buffer, sizeof(recv_buffer), 0, info_UDP_C -> ai_addr, &(info_UDP_C -> ai_addrlen));
                if (len_recv <= 0) {
                    perror("Can't get record detail from server C.");
                }
                else {
                    // split record by ' '
                    std::string detail(recv_buffer);
                    std::vector<std::string> split;
                    std::stringstream stream(detail);
                    while(stream.good()) {
                        std::string substring;
                        std::getline(stream, substring, ' ');
                        split.push_back(substring);
                    }
                    transaction t;
                    t.serial = std::stoi(split.at(0));
                    t.sender = split.at(1);
                    t.receiver = split.at(2);
                    t.amount = std::stoi(split.at(3));
                    records.push_back(t);
                }
            }
        }
    }

}

/**
 * to sort records and write the sorted records to alichain.txt
 */
void sorted_list() {

    std::vector<transaction> records;

    get_record_from_all_server(records);

    // sort record
    std::sort(records.begin(), records.end(), record_compare);
    std::ofstream output;
    output.open(OUTPUT_FILE, std::ios::out);
    for (int i = 0; i < records.size(); i++) {
        transaction t = records.at(i);
        output << t.serial << " " << t.sender << " " << t.receiver << " " << t.amount << std::endl;
    }

    output.close();
}

/**
 * method to handle statistics operation
 * @param name user name
 * @param client indicate which client need this statistics: 1 is client A, 2 is client B
 */
void statistics(std::string name, int client) {

    // length send
    int len_send;

    int exist = check_wallet(name, false);
    if (exist == INT_MIN) {
        // return error message
        sprintf(send_buffer, "NOTEXIST");
        if (client == 1) {
            len_send = send(childSocket_A, send_buffer, strlen(send_buffer), 0);
            if (len_send <= 0) {
                perror("Can't send statistics result to client A");
            }
        }
        else if (client == 2) {
            len_send = send(childSocket_B, send_buffer, strlen(send_buffer), 0);
            if (len_send <= 0) {
                perror("Can't send statistics result to client B");
            }
        }
        return;
    }

    // if existed, get record from every backend server
    std::vector<transaction> records;

    // get all transactions related to this name from all server
    get_distinct_record_from_all_server(records, name);

    // deal with these records
    // create a map, key = username(have transaction with name), value = statistics struct
    std::map<std::string, struct statistics> map;
    for (int i = 0; i < records.size(); i++) {

        struct transaction trans = records.at(i);
        // if user is sender
        if (trans.sender == name) {
            if (map.count(trans.receiver)) {
                map[trans.receiver].amount -= trans.amount;
                map[trans.receiver].transactions++;
            }
            else {
                struct statistics stat;
                stat.transactions = 1;
                stat.amount = -trans.amount;
                map.insert(std::pair<std::string, struct statistics>(trans.receiver, stat));
            }
        }

        // if user is receiver
        if (trans.receiver == name) {
            if (map.count(trans.sender)) {
                map[trans.sender].amount += trans.amount;
                map[trans.sender].transactions++;
            }
            else {
                struct statistics stat;
                stat.transactions = 1;
                stat.amount = trans.amount;
                map.insert(std::pair<std::string, struct statistics>(trans.sender, stat));
            }
        }

    }

    // store stats data
    std::vector<statistics_main> stats;

    // iterate map
    std::map<std::string, struct statistics>::iterator iter;
    for (iter = map.begin(); iter != map.end(); iter++) {

        statistics_main statisticsMain;
        statisticsMain.name = iter->first;
        statisticsMain.amount = iter->second.amount;
        statisticsMain.transactions = iter->second.transactions;
        stats.push_back(statisticsMain);

    }

    // sort by transaction number
    std::sort(stats.begin(), stats.end(), stats_compare);

    // send messages to client A
    if (client == 1) {
        for (int i = 0; i < stats.size(); i++) {
            statistics_main statisticsMain = stats.at(i);
            if (i == stats.size() - 1) {
                sprintf(send_buffer, "%s %d %d", statisticsMain.name.c_str(), statisticsMain.transactions, statisticsMain.amount);
            }
            else {
                sprintf(send_buffer, "%s %d %d,", statisticsMain.name.c_str(), statisticsMain.transactions, statisticsMain.amount);
            }
            len_send = send(childSocket_A, send_buffer, strlen(send_buffer), 0);
            if (len_send <= 0) {
                perror("Can't send statistics result to client A");
            }
        }
    }
    // send messages to client A
    else if (client == 2) {
        for (int i = 0; i < stats.size(); i++) {
            statistics_main statisticsMain = stats.at(i);
            if (i == stats.size() - 1) {
                sprintf(send_buffer, "%s %d %d", statisticsMain.name.c_str(), statisticsMain.transactions, statisticsMain.amount);
            }
            else {
                sprintf(send_buffer, "%s %d %d,", statisticsMain.name.c_str(), statisticsMain.transactions, statisticsMain.amount);
            }
            len_send = send(childSocket_B, send_buffer, strlen(send_buffer), 0);
            if (len_send <= 0) {
                perror("Can't send statistics result to client B");
            }
        }
    }


}

int main(int argc, char* argv[]) {

    // create sockets for client A
    sockA = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sockA == -1) {
        perror("Can't create socket that connected to client A");
        return -1;
    }

    // create sockets for client B
    sockB = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sockB == -1) {
        perror("Can't create socket that connected to client B");
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

    // bind address and port for client A
    info_B = new addrinfo;
    info_B -> ai_family = AF_INET;
    info_B -> ai_socktype = SOCK_STREAM;
    getaddrinfo(HOST_NAME, TCP_PORT_B, 0, &info_B);

    if (bind(sockB, info_B -> ai_addr, info_B -> ai_addrlen) != 0) {
        perror("Can't bind sockB to the port");
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

    // get serverB address
    info_UDP_B = new addrinfo;
    info_UDP_B -> ai_family = AF_INET;
    info_UDP_B -> ai_socktype = SOCK_DGRAM;
    getaddrinfo(HOST_NAME, UDP_PORT_B, 0, &info_UDP_B);

    // get serverC address
    info_UDP_C = new addrinfo;
    info_UDP_C -> ai_family = AF_INET;
    info_UDP_C  -> ai_socktype = SOCK_DGRAM;
    getaddrinfo(HOST_NAME, UDP_PORT_C, 0, &info_UDP_C);


    // listen to sockA
    if (listen(sockA, 10) != 0) {
        perror("Can't listen sockA!");
        close(sockA);
        return -1;
    }

    // listen to sockB
    if (listen(sockB, 10) != 0) {
        perror("Can't listen sockB!");
        close(sockB);
        return -1;
    }

    // boost up message
    printf("The main server is up and running.");
    printf("\n");

    // using select to deal with clientA and clientB, reference code address: https://www.geeksforgeeks.org/tcp-and-udp-server-using-select/
    // learn from it and modify it.
    fd_set rset;
    FD_ZERO(&rset);
    // clear the descriptor set
    FD_ZERO(&rset);

    // get maxfd
    int maxfdp1 = std::max(sockA, sockB) + 1;

    // read and write message
    while(1) {

        // set listenfd and udpfd in readset
        FD_SET(sockA, &rset);
        FD_SET(sockB, &rset);

        // select the ready descriptor
        int nready = select(maxfdp1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(sockA, &rset)) {
            socklen_t client_A_len = sizeof(clientA_address);
            childSocket_A = accept(sockA, (struct sockaddr*)&clientA_address, &client_A_len);
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
                        printf("A TXLIST request has been received.");
                        printf("\n");
                        sorted_list();
                        printf("The sorted file is up and ready.");
                        printf("\n");
                    }
                        // TXCOINS transfer money command
                    else if (split.at(0) == "TRANSFER") {
                        printf("The main server received from %s to transfer %s coins to %s using TCP over port %d.", split.at(1).c_str(), split.at(3).c_str(), split.at(2).c_str(), clientA_address.sin_port);
                        printf("\n");

                        std::string result = transfer_money(split.at(1), split.at(2), std::stoi(split.at(3)));
                        sprintf(send_buffer, "%s", result.c_str());
                        // send the result to the client A
                        printf("The main server sent the result of the transaction to client A.");
                        printf("\n");
                        int len_send = send(childSocket_A, send_buffer, strlen(send_buffer), 0);
                        if (len_send <= 0) {
                            perror("Can't send the result of check wallet command to client A.");
                        }
                    }
                        // statistics
                    else if (split.at(0) == "STAT") {
                        printf("A STATS request has been received.");
                        printf("\n");
                        statistics(split.at(1), 1);
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

        if (FD_ISSET(sockB, &rset)) {
            socklen_t client_B_len = sizeof(clientB_address);
            childSocket_B = accept(sockB, (struct sockaddr*)&clientB_address, &client_B_len);
            if (childSocket_B == -1) {
                perror("Can't accept sockB");
            }
            else {
                memset(recv_buffer, 0, sizeof(recv_buffer));
                if (recv(childSocket_B, recv_buffer, sizeof(recv_buffer), 0) <= 0) {
                    perror("Can't receive sockB");
                }
                else {
                    if (strcmp(recv_buffer, "END") == 0) {
                        printf("main server down");
                        printf("\n");
                        close(childSocket_B);
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
                        printf("A TXLIST request has been received.");
                        printf("\n");
                        sorted_list();
                        printf("The sorted file is up and ready.");
                        printf("\n");
                    }
                    // TXCOINS transfer money command
                    else if (split.at(0) == "TRANSFER") {
                        printf("The main server received from %s to transfer %s coins to %s using TCP over port %d.", split.at(1).c_str(), split.at(3).c_str(), split.at(2).c_str(), clientB_address.sin_port);
                        printf("\n");

                        std::string result = transfer_money(split.at(1), split.at(2), std::stoi(split.at(3)));
                        sprintf(send_buffer, "%s", result.c_str());
                        // send the result to the client A
                        printf("The main server sent the result of the transaction to client B.");
                        printf("\n");
                        int len_send = send(childSocket_B, send_buffer, strlen(send_buffer), 0);
                        if (len_send <= 0) {
                            perror("Can't send the result of check wallet command to client B.");
                        }
                    }
                    // statistics
                    else if (split.at(0) == "STAT") {
                        printf("A STATS request has been received.");
                        printf("\n");
                        statistics(split.at(1), 2);
                    }
                    // check wallet command
                    else {
                        printf("The main server received input=\"%s\" from the client using TCP over port %d.", split.at(0).c_str(), clientB_address.sin_port);
                        printf("\n");
                        int result = check_wallet(message, true);
                        if (result == INT_MIN) {
                            printf("Username was not found on database.");
                            printf("\n");
                        }
                        else {
                            printf("The main server sent the current balance to client B.");
                            printf("\n");
                        }
                        sprintf(send_buffer, "%d", result);
                        int len_send = send(childSocket_B, send_buffer, strlen(send_buffer), 0);
                        if (len_send <= 0) {
                            perror("Can't send the result of check wallet command to client B.");
                        }
                    }
//                printf("%s\n", buffer);
//                int len_send = sendto(sock_UDP, buffer, strlen(buffer), 0, info_UDP_A -> ai_addr, info_UDP -> ai_addrlen);
//                if (len_send <= 0) {
//                    perror("Can't send message to serverA");
//                }
                    close(childSocket_B);
                }
            }
        }

    }

    // close socket
    close(sockA);
    close(sockB);
    close(sock_UDP);
    return 0;

}