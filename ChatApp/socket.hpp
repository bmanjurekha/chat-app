#pragma once

#include <winsock2.h>
#include <string>
#include <vector>

using std::vector;
using std::string;

class Socket {

    protected: 
        SOCKET mSocket;
        WSAData wsaData;

    public:
        Socket();
        ~Socket();
};

class NetSocket : Socket {
    protected: 
        sockaddr_in addr;

    public:
        NetSocket(string addr, int port);
        SOCKET getSocket() { return mSocket; };
};

class ServerSocket : NetSocket {
    private:
        string message;
        int bufferSize;
        vector<SOCKET> clients;

    public:
        ServerSocket(string addr, int port);
        ~ServerSocket();
        bool awaitClient(SOCKET &client);
        int receive(SOCKET &client);
        int getMessageSize();
        string retrieveData(SOCKET &client);
        void closeConnection(SOCKET &client);
};