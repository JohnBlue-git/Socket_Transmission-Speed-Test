#include <iostream>
#include <cstring>

#include <unistd.h> // for sleep() usleep()

#include <thread>
#include <arpa/inet.h> // for socket
#include <errno.h> // for socket tracebility

#include "functions.hpp"

#define PORT 8080 // Port

//
// Client side
//

void requestServer(int clientSocket) {
    // Proceed ping test
    double averageLatency;
    if ( false == pingRequest(clientSocket, &averageLatency) ) {
        return;
    }

    // Proceed upload to server
    double uploadSpeed = 0.0;
    if ( false == sendFile(clientSocket, &uploadSpeed) ) {
        return;
    }
        
    // Handle download from server
    if ( false == recvFile(clientSocket) ) {
        return;
    }
    
    // Receive download speed
    double downloadSpeed = 0.0;
    if ( recv(clientSocket, &downloadSpeed, 8, 0) < 0 ) {
        return;
    }

    // Close socket
    close(clientSocket);
    
    // Show transmission speed
    std::cout << "Average latency (micro second): " << averageLatency << std::endl;
    std::cout << "Upload Speed (Mbps): " << uploadSpeed << std::endl;
    std::cout << "Download Speed (Mbps): " << downloadSpeed << std::endl
    << std::endl;

    return;
}

void createClient() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error: Client side could not create socket");
        return;
    }

    // Initialize server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address
    serverAddr.sin_port = htons(PORT);

    // Connect to server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error: Client side connection failed");
        return;
    }

    // Create thread to send message
    std::thread tid(requestServer, clientSocket);
    if ( false == tid.joinable() ) {
        perror("Error: Client side could not create thread");
        return;
    }

    // Wait for thread to complete
    tid.join();

    return;
}

//
// Main
//

int main()
{
    // Create client thread
    std::thread clt(createClient);
    if ( false == clt.joinable() ) {
        perror("Error: Could not create Client side");
        exit(EXIT_FAILURE);
    }
    
    // Join thread
    clt.join();
    
   return EXIT_SUCCESS;
}