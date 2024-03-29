#include <iostream>
#include <cstring>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

void joinChannel(int clientSocket, const std::string& channelName) {
    std::string joinCommand = "JOIN " + channelName;
    if (send(clientSocket, joinCommand.c_str(), joinCommand.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Error sending JOIN command to server\n";
    } else {
        std::cout << "Joined channel: " << channelName << std::endl;
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

    // Join a channel
    joinChannel(clientSocket, "General");
    // Cleanup
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
