#include <iostream>
#include "socket.hpp"

using std::cerr;
using std::endl;
using std::string;


void exitEnv(int status, string err) {
    cerr << err << endl;
    WSACleanup();
    exit(status);
}

void exitEnv(int status) {
    exitEnv(status, "Unexpected error");
}


Socket::Socket() {
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
        exitEnv(10, "Error on connecting to socket service");
    }

    mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(mSocket == INVALID_SOCKET) {
        exitEnv(11, "Error caused by invalid socket");
    }
}

Socket::~Socket() {
    closesocket(mSocket);
}

NetSocket::NetSocket(string netAddr, int port) : Socket() {
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(netAddr.c_str());
    addr.sin_port = htons(port);
}

ServerSocket::ServerSocket(string netAddr, int port): NetSocket(netAddr, port) {
    if(bind(getSocket(), (SOCKADDR*) &addr, sizeof(addr)) == SOCKET_ERROR) {
        exitEnv(12, "Error ccaused by failure in binding socket instance");
    }

    if(listen(getSocket(), 1) == SOCKET_ERROR) {
        exitEnv(13, "Error caused by failure in listening to the socket instance");
    }
    bufferSize = 256;
}

ServerSocket::~ServerSocket() {
    while(!clients.empty()) { closeConnection(clients.back()); }
}

bool ServerSocket::awaitClient(SOCKET &client) {
    client = accept(getSocket(), NULL, NULL);
    bool isSuccessful = client != SOCKET_ERROR;
    
    clients.push_back(client);

    return isSuccessful;
}

int ServerSocket::receive(SOCKET &client) {
    char package[bufferSize];

    int size = recv(client, package, bufferSize, 0);
    package[size] = '\0';

    message.append(package);

    return size;
}

int ServerSocket::getMessageSize() {
    return message.length();
}

string ServerSocket::retrieveData(SOCKET &client) {
    int size = -1;
    string data;

    do {
        size = receive(client);
    } while(size == bufferSize);

    data = message.substr(0);
    message.clear();

    return data;
}

void ServerSocket::closeConnection(SOCKET &client) {
    int index = 0;
    bool found;

    while(index < clients.size() && !found) {
        if(clients.at(index) == client) {
            found = true;
        } else {
            index++;
        }
    }

    if(found) {
        closesocket(clients.at(index));
        clients.erase(clients.begin() + index);
    }
}


