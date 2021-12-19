#include <iostream>
#include <fstream>
#include <memory>
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <string>
#include <conio.h>

#pragma warning(disable: 4996)

SOCKET Connection;

enum Packet
{
    Message,
    File,
    EXIT
};

void FatalError(std::string message)
{
    throw new std::exception(message.c_str());
}

void FileReceive(char* recvbuf, int recvbuflen, char* format)
{
    std::ofstream out(format, std::ios::binary);
    if (out.is_open())
    {
        out.write(recvbuf, recvbuflen);
    }
    out.close();
}

void SendFile(std::string msg)
{
    int point = msg.rfind('\\');
    if (point == std::string::npos)
    {
        point = msg.rfind('/');
    }
    msg = msg.substr(point + 1);
    int size = msg.size();
    send(Connection, (char*)&size, sizeof(int), NULL);
    send(Connection, msg.c_str(), size, NULL);
}

Packet choicePacket()
{
    while (true)
    {
        if (kbhit())
        {
            switch (getch())
            {
            case 'm':
                return Message;
            case 'f':
                return File;
            case 'M':
                return Message;
            case 'F':
                return File;
            default:
                return EXIT;
            }
        }
    }
    throw ExceptionContinueSearch;
}

bool ProcessPacket(Packet packettype)
{
    switch (packettype)
    {
    case Message:
    {
        int msg_size;
        recv(Connection, (char*)&msg_size, sizeof(int), NULL);
        char* msg = new char[msg_size + 1];
        msg[msg_size] = '\0';
        recv(Connection, msg, msg_size, NULL);
        std::cout << msg << std::endl;
        delete[] msg;
        break;
    }
    case File:
    {
        int recvbuflen, sizeformat;
        recv(Connection, (char*)&sizeformat, sizeof(int), NULL);
        char* format = new char[sizeformat + 1]; format[sizeformat] = '\0';
        recv(Connection, format, sizeformat, NULL);
        recv(Connection, (char*)&recvbuflen, sizeof(int), NULL);
        char* recvbuf = new char[recvbuflen + 1]; recvbuf[recvbuflen] = '\0';
        recv(Connection, recvbuf, recvbuflen, NULL);
        FileReceive(recvbuf, recvbuflen, format);
        delete[] recvbuf, format;
        break;
    }
    default:
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
    exit(0);
}

int main(int argc, char* argv[])
{
    //WSAStartup
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        FatalError("can not initialize Client");
        exit(1);
    }
    SOCKADDR_IN addr;
    int size = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(1111);
    addr.sin_family = AF_INET;

    Connection = socket(AF_INET, SOCK_STREAM, NULL);
    if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
        FatalError("failed connect to server");
        return 1;
    }
    std::cout << "Connected!\n";

    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

    while (true)
    {
        std::cout << "Set packettype 'f' - File; 'm' - Message\n";
        Packet packettype = choicePacket();
        send(Connection, (char*)&packettype, sizeof(Packet), NULL);
        std::cout << "Your text\n";
        std::string msg;
        std::getline(std::cin, msg);
        int msg_size = msg.size();
        if (packettype == File) {
            SendFile(msg);
        }
        send(Connection, (char*)&msg_size, sizeof(int), NULL);
        send(Connection, msg.c_str(), msg_size, NULL);
    }
    if (Connection) closesocket(Connection);
    WSACleanup();
    system("pause");
    return 0;
}