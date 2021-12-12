#include <iostream>
#include <fstream>
#include <memory>
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <string>

#pragma warning(disable: 4996)

SOCKET Connection;

enum Packet
{
    ChatMessage,
    Test,
    File,
};

bool ProcessPacket(Packet packettype)
{
    switch (packettype)
    {
    case ChatMessage:
    {
        int msg_size;
        recv(Connection, (char*)&msg_size, sizeof(int), NULL);
        char* msg = new char[msg_size + 1];
        msg[msg_size] = '\0';
        recv(Connection, msg, msg_size, NULL);
        std::cout << msg << std::endl;
        break;
    }
    case Test:
        std::cout << "Test packet\n";
        break;
    case File:
        break;
    default:
        std::cout << "Unrecognized packet: " << packettype << std::endl;
        return false;
    }
    return true;
}

void ClientHandler()
{
    Packet packettype;
    while (true)
    {
        recv(Connection, (char*)&packettype, sizeof(Packet), NULL);
        if (!ProcessPacket(packettype))
        {
            break;
        }
    }
    closesocket(Connection);
}

std::ofstream out("E:/Programming/C++/MyProjects/Network/out.exe", std::ios::binary);
void FileReceive(char* recvbuf, int recvbuflen)
{
    if (out.is_open())
    {
        out.write(recvbuf, recvbuflen);
        memset(&recvbuf, (int)recvbuf, recvbuflen);
}
}

int main(int argc, char* argv[])
{
    //WSAStartup
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        std::cout << "ERROR" << std::endl;
        exit(1);
    }
    SOCKADDR_IN addr;
    int size = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(1111);
    addr.sin_family = AF_INET;

    Connection = socket(AF_INET, SOCK_STREAM, NULL);
    if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
        std::cout << "Error: failed connect to server" << std::endl;
        return 1;
    }
    std::cout << "Conncted!\n";

    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

    std::string send_msg;
    while (true)
    {
        std::getline(std::cin, send_msg);
        int msg_size = send_msg.size();
        Packet packettype = ChatMessage;
        send(Connection, (char*)&packettype, sizeof(Packet), NULL);
        send(Connection, (char*)&msg_size, sizeof(int), NULL);
        send(Connection, send_msg.c_str(), msg_size, NULL);
        Sleep(10);
    }
    WSACleanup();
    system("pause");
    return 0;
}