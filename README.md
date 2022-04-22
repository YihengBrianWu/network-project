# EE450 Project - Socket Programming
Yiheng Wu\
USC ID: 6851878959\
Section: 30560
## 1. How to start
This project contains six cpp files, one make file, one README.

This project completes all functionality include extra points functionality (stats).

In terminal, type make all command. Then using ./serverM to start server M, ./serverA to start server A, ./serverB to start server B, ./serverC to start serverC. Then three servers are all running, please run server M first then server A, B, C.

For client. If you want to use client A then type ./clientA. If you want to use client A then type ./clientB.

Please make sure below port numbers are not in used when running this project:
1. server M TCP port with client A: 25959
2. server M TCP port with client B: 26959
3. server M UDP port: 24959
4. server A UDP port: 21959
5. server B UDP port: 22959
6. server C UDP port: 23959

Client offer 4 different operations:
1. ./clientX username -> to check the balance of designated user.
2. ./clientX username1 username2 amount -> Transfer amount of coins from user1 to user2.
3. ./clientX TXLIST -> let the server M create or update it's alichain.txt which contains all transaction in three servers.
4. ./clientX username stats -> view the statistics of designated user.

## 2. How every functionality works
### 2.1 check wallet
Client will send the username to server M, server M will then ask a, b, and c if this user exist in their record. Backend server will traverse every record line in their records to see if this user exist. If not exist, the backend server will send INT_MIN to server M to represent user does not exist. Otherwise, the backend server will send the balance of this user in it record.

Server M will repeat this process for A, B, and C. If all three backends send INT_MIN, then this user is not exist in this network and server M will send this message to client. Otherwise, the server M will accumulate the balance from all three backends and add this value with initial balance 1000, the result is the current balance of this user. Server M will send this result to client.

### 2.2 transfer coins
Basically, when server M receive the message from client, server M first will split the arguments out (user1, user2, amount). Then server M will do check wallet function for each user to see if both user exist and their balance, typically user1's balance.

When server M get the balance of both user, it will check if someone is not in the network or both is not in the network.

There are 4 failed cases:
1. user1 not exists
2. user2 not exists
3. both user1 and user2 not exists
4. both exists but user1 doesn't have enough balance -> user1's balance < amount

For each case, server M will send different failed message to client to show different message to let user know why this transaction failed.

If it's a legal transaction, then server M will send a new request to each backend server which is get serial number request. For each backend server, when it gets this command, backend will traverse the records to get the largest serial number and return this number to server M. server M send the request to all 3 backends, and then get the max number among three result from backends. The result + 1 is the newest serial number server M wants.

Then server M will randomly choose a backend and combine all information (serial number, user1, user2, amount), send this to the chosen server and the chosen server will save this transaction to its local file.

Finally server M will send the success message to client.

### 2.3 TXLIST
When server M get this command, it will send a new request to each backend server which is let backend server sends it all records to server M. When backends receive this request, it will just simply send all lines in its local file to server M.

Server M will repeat this process for all 3 backends. When server M finish requesting, it will do sort to all transactions based on serial number from low to high. 

When sort is done, the server M will write the sorted transaction list to a file named alichian.txt.

### 2.4 statistics
Client will send username to server M with the request statistics. After server M receive the request, it will do check wallet function to this user first. If the user doesn't exist, then it will just send a don't exist message to client.

If user exists, then server M will send a request which to send all transactions include this user, whether is user or receiver, to backends. Backends will call a function to traverse all records include this user and send these records to the server M.

Server M will repeat this process for all 3 backends. When server M finish requesting, it will do all statistics thing.

When statistics is done, it will send the result to client and show to the user.

## 3. Message exchange

## 4. Reference
In the server M, I use select() function to handle two connections to the server M. This part of code, the select part, is modified from https://www.geeksforgeeks.org/tcp-and-udp-server-using-select/ this website. Shout out to geeksforgeeks, help me a lot in this project.

