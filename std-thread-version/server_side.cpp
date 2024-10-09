#include <iostream>
#include <cstring>

#include <unistd.h> // for sleep() usleep()

#include <thread>
#include <arpa/inet.h> // for socket
#include <errno.h> // for socket tracebility

#include "functions.hpp"

#define PORT 8080 // Port
#define MAX_CONNECTIONS 9 // Max 9 connection
#define TIME_OUT 10 // second

//
// Server side
//

void handleClient(int clientSocket) {
    // Respond to ping test
    if ( false == pingResponse(clientSocket) ) {
        return;
    }

    // Handle upload from client 
    if ( false == recvFile(clientSocket) ) {
        return;
    }
    
    // Proceed download to client
    double downloadSpeed = 0.0;
    if ( false == sendFile(clientSocket, &downloadSpeed) ) {
        return;
    }
    
    // Send download speed
    send(clientSocket, &downloadSpeed, 8, 0); // size 8 because double is 8 bytes

    return;
}

void createServer() {
    int serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error: Server side could not create socket");
        return;
    }

    // Initialize server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Force to bind via setting (ref: https://stackoverflow.com/questions/24194961/how-do-i-use-setsockoptso-reuseaddr)
    int reuse = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");
#ifdef SO_REUSEPORT
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
        perror("setsockopt(SO_REUSEPORT) failed");
#endif
    
    // Bind socket to address
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error: Server side bind failed");
        return;
    }

    // Listen for incoming connections
    if (listen(serverSocket, MAX_CONNECTIONS) < 0) {
        perror("Error: Server side listen failed");
        return;
    }

    std::cout << "Server side listening on port " << PORT << std::endl << std::endl;

    int que = 0;
    int clientSocket[MAX_CONNECTIONS];
    while (1) {
        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = TIME_OUT;
        timeout.tv_usec = 0;
        setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        // Accept incoming connections and create a thread for each client
        clientSocket[que] = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket[que] < 0) {
            perror("Error: Server side accept failed");
            break;
        }

        // Retrieve and print the client's IP address
        //char client_ip[INET_ADDRSTRLEN];
        //inet_ntop(AF_INET, &clientAddr.sin_addr, client_ip, sizeof(client_ip));
        //std::cout << "Client connected: << client_ip << ":" << ntohs(clientAddr.sin_port) << std::endl << std::endl;
        
        // Create thread for client
        std::thread tid(handleClient, clientSocket[que]);
        if ( false == tid.joinable() ) {
            perror("Error: Server side could not create thread for client");
            continue;
        }
        tid.detach();

        // To next accept
        if (++que == MAX_CONNECTIONS) {
            que = 0;
        }
    }

    // Close server socket
    close(serverSocket);

    return;
}

//
// Main
//

int main()
{
    // Create server thread
    std::thread tid(createServer);
    if ( false == tid.joinable() ) {
        perror("Error: Server side could not create thread for client");
        exit(EXIT_FAILURE);
    }
    
    // Join thread
    tid.join();
    
   return EXIT_SUCCESS;
}