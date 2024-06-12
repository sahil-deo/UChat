#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef UNICODE
#define UNICODE
#endif

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <ws2tcpip.h>
#include <WinSock2.h>
#include <thread>
#include <windows.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


int makeServer(std::string ip);
int isClient(std::string ip);


std::string PORT;

void line(int n) {
    for (int i = 0; i < n; i++) {
        std::cout << "----------------------------------------------------------------" << std::endl;
    }
}



auto SENDDATA = [](SOCKET socket) {

    std::string sendBuf;


    while (true) {


        std::cin.sync();
        std::cin.clear();

        std::getline(std::cin, sendBuf);

        int byteCount = send(socket, sendBuf.c_str(), 512, 0);
        if (byteCount > 0) {

            if (sendBuf == "\STOP") return;
            line(1);
        }
        else {
            std::cout << "DATA NOT TRANSMITTED" << std::endl;
        }
    }
    };
auto RECVDATA = [](SOCKET socket) {
    while (true) {

        char recvBuf[512];
        int byteCount = recv(socket, recvBuf, 512, 0);
        if (byteCount > 0) {

            if (recvBuf == "\STOP") return;
            
            std::cout << "Received: " << recvBuf << std::endl;
            line(1);
        }
    }
    };


int main() {

    // Defining const PORT
    // ... (same as before)

    std::string ip, serverIP;
    char isServer;
    std::cout << "IsServer? (Y/N): ";
    std::cin >> isServer;

    if (isServer == 'Y' || isServer == 'y') {
        
        // Bind to all interfaces 
        ip = "0.0.0.0";

        std::cout << "PORT: ";
        std::cin >> PORT;
        if (makeServer(ip) == 0) {

        }
        else {
            std::cout << "Error!!!" << std::endl;
        }
    }
    else {

        std::cout << "Enter Server's IP address: " << std::endl;
        std::cin >> serverIP;
        std::cout << "PORT: ";
        std::cin >> PORT;

        if (isClient(serverIP) == 0) {

        }
        else {
            std::cout << "Error!!!" << std::endl;
        }
    }

    char a;
    std::cin >> a;

    return 0;
}


int isClient(std::string ip) {
    WSADATA wsaData;

    SOCKET connectSocket;

    WORD wsaword = MAKEWORD(2, 2);

    int wsaerr;

    struct addrinfo* result = NULL, * ptr = NULL, hints;

    //WSA initialization
    {
        wsaerr = WSAStartup(wsaword, &wsaData);

        if (wsaerr != 0) {
            std::cout << "Error Occured: " << wsaerr << std::endl;
            WSACleanup();
            return 1;

        }
        else {
            std::cout << "WSA Initialized Successfully!" << std::endl;
            std::cout << "Status: " << wsaData.szSystemStatus << std::endl;
        }
    }


    // Populating addrinfo var using getaddrinfo()
    {
        ZeroMemory(&hints, sizeof(hints));

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        wsaerr = getaddrinfo(ip.c_str(), PORT.c_str(), &hints, &result);
        if (wsaerr != 0) {
            std::cout << "GETADDRINFO() failed: " << wsaerr << std::endl;
            WSACleanup();
            return 1;
        }
    }


    // Creating a Socket using socket()
    {
        ptr = result;
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (connectSocket == INVALID_SOCKET) {
            std::cout << "INVALID SOCKET" << std::endl;
            WSACleanup();
            return 1;
        }
    }


    // Connecting to the server using connect()
    {
        wsaerr = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (wsaerr != 0) {
            std::cout << "Cannot connect to the given Address" << std::endl;
            WSACleanup();
            closesocket(connectSocket);
            freeaddrinfo(ptr);
            return 1;
        }
        freeaddrinfo(ptr);
    }

     
    // SEND & RECV DATA
    {

        std::thread sendDataThread(SENDDATA, connectSocket);
        std::thread receiveDataThread(RECVDATA, connectSocket);
        
        sendDataThread.join();

    }



    WSACleanup();
    
    closesocket(connectSocket);
    
    return 0;
}


int makeServer(std::string ip) {

    WSADATA wsaData;

    WORD wsaword = MAKEWORD(2, 2);

    int wsaerr;

    wsaerr = WSAStartup(wsaword, &wsaData);

    struct addrinfo* result = NULL, * ptr = NULL, hints;

    SOCKET lsocket;
    SOCKET acceptSocket;


    // WSA initialization
    {
        if (wsaerr != 0) {
            std::cout << "Error Occured: " << wsaerr << std::endl;
            WSACleanup();
            return 1;

        }
        else {
            std::cout << "WSA Initialized Successfully!" << std::endl;
            std::cout << "Status: " << wsaData.szSystemStatus << std::endl;
        }
    }


    // Populating addrinfo var using getaddrinfo()
    {
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        wsaerr = getaddrinfo(ip.c_str(), PORT.c_str(), &hints, &result);
        if (wsaerr != 0) {
            std::cout << "GETADDRINFO() failed: " << wsaerr << std::endl;
            WSACleanup();
            return 1;
        }
    }


    // Creating a Socket using socket()
    {

        lsocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

        if (lsocket == INVALID_SOCKET) {
            std::cout << "INVALID SOCKET" << std::endl;
            WSACleanup();
            return 1;
        }
    }


    // Binding the socket using bind()
    {
        wsaerr = bind(lsocket, result->ai_addr, (int)result->ai_addrlen);
        if (wsaerr != 0) {
            std::cout << "Bind Failed: " << WSAGetLastError() << std::endl;
            freeaddrinfo(result);
            WSACleanup();
            closesocket(lsocket);
            return 1;
        }
        freeaddrinfo(result);
    }


    // Listening for client socket using listen()
    {

        if (listen(lsocket, 1) == SOCKET_ERROR) {
            std::cout << "SOCKET ERROR: " << WSAGetLastError() << std::endl;
            closesocket(lsocket);
            WSACleanup();
            return 1;
        }
    }


    // Accepting a connection using accept()
    {
        acceptSocket = accept(lsocket, NULL, NULL);
        if (acceptSocket == INVALID_SOCKET) {
            std::cout << "INVALID CLIENT SOCKET " << std::endl;
            WSACleanup();
            closesocket(lsocket);
            return 1;
        }
    }

    
    std::cout << "Status: Connected" << std::endl;
    
    
    
    // SEND & RECV DATA
    {
        std::thread sendDataThread(SENDDATA, acceptSocket);
        std::thread receiveDataThread(RECVDATA, acceptSocket);

        sendDataThread.join();
    }



    std::cout << "Connection Closed" << std::endl;

    WSACleanup();

    closesocket(lsocket);

    return 0;
}


