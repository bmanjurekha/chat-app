// Server.cpp

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <cstring>
#include <winsock2.h>
#include "server.hpp"
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

    void ChatServer::startServer() {
        // Set up socket
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            std::cerr << "Error creating socket\n";
            exit(EXIT_FAILURE);
        }

        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(port);

        // Bind
        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
            std::cerr << "Error binding socket\n";
            closesocket(serverSocket);
            exit(EXIT_FAILURE);
        }

        // Listen
        if (listen(serverSocket, 10) == -1) {
            std::cerr << "Error listening on socket\n";
            closesocket(serverSocket);
            exit(EXIT_FAILURE);
        }

        std::cout << "Server listening on port " << port << std::endl;

        while (true) {
            int clientSocket = accept(serverSocket, nullptr, nullptr);

            if (clientSocket == -1) {
                std::cerr << "Error accepting connection\n";
                continue;
            }

            std::thread clientThread(&ChatServer::handleClient, this, clientSocket);
            clientThread.detach(); // Detach the thread to allow it to run independently
        }
    }

    void ChatServer::handleClient(int clientSocket) {
    char buffer[1024]; // Buffer to store incoming messages

        while (true) {
            int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

            if (bytesRead <= 0) {
                // Handle client disconnect or error
                std::cerr << "Client disconnected\n";
                closesocket(clientSocket);
                return;
            }
            std::cerr << "Client connected\n";
            buffer[bytesRead] = '\0'; // Null-terminate the received data

            std::string receivedMessage(buffer);

           
            // Process the received message
            processMessage(clientSocket, receivedMessage);
        }
    }

    void ChatServer::processMessage(int clientSocket, const std::string& message) {
    // Extract command and arguments from the message
    std::cerr << message;

    std::string method, path, body;
     if (parseHttpRequest(message, method, path, body)) {
        // Print the parsed components
        std::cout << "HTTP Method: " << method << std::endl;
        std::cout << "Path: " << path << std::endl;
        std::cout << "Body: " << body << std::endl;
    } 
    
    if(path=="/CREATE")
    {
         // Extract channel name from arguments
        std::cerr << "Create Channel\n";
        // Split the JSON-like string
        std::string channelname = getJsonValue(body,"channelname");
        createChannel(clientSocket, channelname);
    }
    else if (path == "/JOIN") {
        // Extract channel name from arguments
        std::cerr << "Joined Channel\n";
        // Split the JSON-like string
        std::string channelname = getJsonValue(body,"channelname");
        std::cerr << "Joined Channel\n"+channelname;
        joinChannel(clientSocket, channelname);
        return ;
    } else if (path == "/SEND") {
        // Extract channel name and content from arguments
        std::cerr << "Message Sent\n";
       // Split the JSON-like string
       std::string channelname = getJsonValue(body,"channelname");
       std::string content = getJsonValue(body,"message");
       sendMessage(clientSocket, channelname, content);
       return ;       
    } else if (path == "/CHANNELLIST") {
        // No additional arguments for LIST_CHANNELS
        std::cerr << "List of Channel\n";
        listChannels(clientSocket);
        return ;
    } else if (path == "/MESSAGES"){
        // No additional arguments for LIST_CHANNELS
        std::cerr << "Show messages\n";
        receiveMessages(clientSocket);
        return ;
    }
}

    void ChatServer::createChannel(int clientSocket, const std::string& channelName) {
        std::lock_guard<std::mutex> lock(channelsMutex);

        
            // Channel does not exist, create it
            auto result = channels.emplace(channelName, channelName);
            if (result.second) {
                // Channel was successfully created
                std::cout << "Channel created: " << channelName << std::endl;
                sendMessage(clientSocket,channelName, "CREATE_CHANNEL_OK "+channelName);
            } else {
                // Channel already exists
                std::cout << "Channel already exists: " << channelName << std::endl;
                sendMessage(clientSocket,channelName, "ERROR Channel already exists");
            }
            closesocket(clientSocket);
    }

    void ChatServer::joinChannel(int clientSocket, const std::string& channelName) {
        std::lock_guard<std::mutex> lock(channelsMutex);

        auto it = channels.find(channelName);
        if (it != channels.end()) {
            // Channel exists
            std::cerr << "Channel exists\n";
            std::cerr << "User joined the channel\n";
            it->second.addMessage("User joined the channel");
            clientChannels[clientSocket] = channelName;
            sendMessage(clientSocket,channelName, "JOIN_OK");
        } else {
            // Channel does not exist
             std::cerr << "Channel does not exist\n";
            sendMessage(clientSocket,channelName, "ERROR Channel does not exist");
        }
        closesocket(clientSocket);
    }

    void ChatServer::sendMessage(int clientSocket, const std::string& channelName, const std::string& content) {
    std::lock_guard<std::mutex> lock(channelsMutex);

    auto it = channels.find(channelName);
    if (it != channels.end()) {
        // Channel exists
        Channel& channel = it->second;
        std::string message = "User: " + content; // You can customize the format
        channel.addMessage(message);
        std::cout << "Message Sent to Channel: " << std::endl;
        // Broadcast the message to all clients in the channel
        for (const auto& entry : clientChannels) {
            if (entry.second == channelName && entry.first != clientSocket) {
                // Send the message to other clients in the channel
                 std::cout << "Message Sent to Channel: "+channelName+"Message :"+message << std::endl;
                sendMessage(entry.first, channelName, message);
            }
        }
    } else {
        // Channel does not exist
        std::cerr << "Channel does not exist\n";
        sendMessage(clientSocket,channelName, "ERROR Channel does not exist");
    }
    closesocket(clientSocket);
}

    void ChatServer::listChannels(int clientSocket) {
        std::lock_guard<std::mutex> lock(channelsMutex);

        std::string channelList = "Available channels:\n";
        std::cout << "Available channels: " << std::endl;
        for (const auto& entry : channels) {
            channelList += entry.first + "\n";
            std::cout << "Available channels: "+channelList << std::endl;
             sendMessage(clientSocket,entry.first, channelList);
        }
    closesocket(clientSocket);
       
    }

    void ChatServer::receiveMessages(int clientSocket){
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
        closesocket(clientSocket);
    }    

    bool ChatServer::parseHttpRequest(const std::string& httpRequest, std::string& method, std::string& path, std::string& body) {
    // Find the end of the request line
    size_t requestLineEnd = httpRequest.find("\r\n");
    if (requestLineEnd == std::string::npos) {
        return false;  // Invalid request
    }

    // Extract the request line
    std::string requestLine = httpRequest.substr(0, requestLineEnd);

    // Extract method and path from the request line
    size_t methodEnd = requestLine.find(' ');
    if (methodEnd == std::string::npos) {
        return false;  // Invalid request
    }
    method = requestLine.substr(0, methodEnd);

    size_t pathStart = methodEnd + 1;
    size_t pathEnd = requestLine.find(' ', pathStart);
    if (pathEnd == std::string::npos) {
        return false;  // Invalid request
    }
    path = requestLine.substr(pathStart, pathEnd - pathStart);

    // Extract body by finding the double newline separator
    size_t bodyStart = httpRequest.find("\r\n\r\n");
    if (bodyStart != std::string::npos) {
        bodyStart += 4;  // Move past the double newline separator
        body = httpRequest.substr(bodyStart);
    }

    return true;
}


    // Function to parse a simple key-value JSON string
    std::string ChatServer::getJsonValue(const std::string& json, const std::string& key) {
        // Find the position of the key
        size_t keyPos = json.find("\"" + key + "\":");
        if (keyPos == std::string::npos) {
            return "";  // Key not found
        }

        // Find the value after the key
        size_t valueStart = json.find("\"", keyPos + key.length() + 2);
        size_t valueEnd = json.find("\"", valueStart + 1);

        if (valueStart == std::string::npos || valueEnd == std::string::npos) {
            return "";  // Invalid JSON
        }

        // Extract the value
        return json.substr(valueStart + 1, valueEnd - valueStart - 1);
    }
int main() {
    ChatServer server(12345); // Replace 12345 with your desired port number
    server.startServer();

    return 0;
}
