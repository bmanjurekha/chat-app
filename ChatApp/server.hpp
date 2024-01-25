#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <cstring>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

class Channel {
public:
    Channel(const std::string& name) : name(name) {}

    std::string getName() const {
        return name;
    }

    std::vector<std::string> getHistory() const {
        return history;
    }

    void addMessage(const std::string& message) {
        history.push_back(message);
    }

private:
    std::string name;
    std::vector<std::string> history;
};

class ChatServer {
public:

    ChatServer(int port) : port(port) {
        // Initialize Winsock
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Failed to initialize Winsock\n";
            exit(EXIT_FAILURE);
        }
    }

    ~ChatServer() {
        WSACleanup();
    }
    void startServer();
    //Handle Client to process incoming messages
    void handleClient(int clientSocket);
    void processMessage(int clientSocket, const std::string& message);
    //CRUD Operations
    void createChannel(int clientSocket, const std::string& channelName);
    void joinChannel(int clientSocket, const std::string& channelName);
    void sendMessage(int clientSocket, const std::string& channelName, const std::string& content);
    void listChannels(int clientSocket);
    void receiveMessages(int clientSocket);
    //Parse HTTP request & get Json Value
    bool parseHttpRequest(const std::string& httpRequest, std::string& method, std::string& path, std::string& body);
    std::string getJsonValue(const std::string& json, const std::string& key);

    private:
    int port;
    std::map<std::string, Channel> channels;
    std::map<int, std::string> clientChannels;
    std::mutex channelsMutex;
    WSADATA wsaData;
 
};