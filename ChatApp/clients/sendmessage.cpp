#include <iostream>
#include <cstring>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

void sendMessage(int clientSocket, const std::string& channelName, const std::string& content) {
    std::cout << "In sendMessage " ;
    std::string sendCommand = "SEND " + channelName + " " + content;
    if (send(clientSocket, sendCommand.c_str(), sendCommand.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Error sending SEND command to server\n";
    } else {
        std::cout << "Message sent to channel " << channelName << ": " << content << std::endl;
    }
}
void listChannel(int clientSocket) {
    std::cout << "In listChannel " ;
    std::string sendCommand = "LIST_CHANNELS ";
    if (send(clientSocket, sendCommand.c_str(), sendCommand.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Error sending SEND command to server\n";
    } else {
        std::cout << "Message sent to channel \n";
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

    // You can add more logic here to interact with the server, send messages, etc.
    sendMessage(clientSocket, "General", "Hello, everyone!");

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
