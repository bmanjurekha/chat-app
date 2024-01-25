#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <string>
#include <vector>
#include <thread>
#pragma comment(lib, "ws2_32.lib")

using std::string;
using std::vector;

void receiveMessages(int clientSocket) {
    char buffer[1024];
    while (true) {
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            // Handle server disconnect or error
            std::cerr << "Server disconnected\n";
            break;
        }

        buffer[bytesRead] = '\0'; // Null-terminate the received data
        std::cout << "Received message: " << buffer << std::endl;
    }
}
int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock\n";
        return 1;
    }

    // Create a socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket\n";
        WSACleanup();
        return 1;
    }

    // Set up server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345); // Change to the port your server is using
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change to your server's IP address

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to server\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
     // Start a thread to receive messages
    std::thread receiveThread(receiveMessages, clientSocket);
     // Wait for the receive thread to finish (e.g., when the server disconnects)
    receiveThread.join();
    // Cleanup
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
