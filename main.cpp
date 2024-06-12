#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <ws2tcpip.h>
#include <WinSock2.h>
#include <thread>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning(disable:4996) 

int makeServer(std::string ip);
int isClient(std::string ip);


int PORT;

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

    sockaddr_in addr;

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




    // Creating a Socket using socket()
    {
        
        connectSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (connectSocket == INVALID_SOCKET) {
            std::cout << "INVALID SOCKET" << std::endl;
            WSACleanup();
            return 1;
        }



    }
    // Populating addr var of sockaddr_in 
    {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
    }


    // Connecting to the server using connect()
    {
        wsaerr = connect(connectSocket, (sockaddr*) &addr, sizeof(addr));
        if (wsaerr != 0) {
            std::cout << "Cannot connect to the given Address" << std::endl;
            WSACleanup();
            closesocket(connectSocket);
            return 1;
        }
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

    sockaddr_in addr;

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


    // Populating addr var
    {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
    }


    // Creating a Socket using socket()
    {

        lsocket = socket(AF_INET, SOCK_STREAM, 0);

        if (lsocket == INVALID_SOCKET) {
            std::cout << "INVALID SOCKET" << std::endl;
            WSACleanup();
            return 1;
        }
    }


    // Binding the socket using bind()
    {
        wsaerr = bind(lsocket,(sockaddr*)&addr, sizeof(addr));
        if (wsaerr != 0) {
            std::cout << "Bind Failed: " << WSAGetLastError() << std::endl;
            WSACleanup();
            closesocket(lsocket);
            return 1;
        }
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


