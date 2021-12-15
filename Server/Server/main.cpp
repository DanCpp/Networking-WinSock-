#include <iostream>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <stdio.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable: 4996)

const int COUNTCONN = 100;

SOCKET Connections[COUNTCONN];
int Counterclients = 0;

enum Packet
{
    Message,
    File,
    EXIT
};

std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    std::ifstream::pos_type len = in.tellg();
    in.close();
    return len;
}

void FileSend(char* FilePath, int index)
{
    std::ifstream in(FilePath, std::ios::binary);
    int sendbuflen = filesize(FilePath);
    char* sendbuf = new char[sendbuflen + 1];
    if (in.is_open())
    {
        in.seekg(0, std::ios::beg);
        in.read(sendbuf, sendbuflen);
        for (int i = 0; i < Counterclients; i++)
        {
            if (i == index) continue;
            Packet packet_file = File;
            send(Connections[i], (char*)&packet_file, sizeof(Packet), NULL);
            send(Connections[i], (char*)&sendbuflen, sizeof(int), NULL);
            send(Connections[i], sendbuf, sendbuflen, NULL);
        }
        in.close();
    }

}

bool ProcessPacket(int index, Packet packettype)
{
    switch (packettype)
    {
    case Message:
    {
        int msg_size;
        recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
        char* msg = new char[msg_size + 1];
        msg[msg_size] = '\0';
        recv(Connections[index], msg, msg_size, NULL);
        for (int i = 0; i < Counterclients; i++)
        {
            if (i == index) continue;
            Packet msg_Packet = Message;
            send(Connections[i], (char*)&msg_Packet, sizeof(Packet), NULL);
            send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
            send(Connections[i], msg, msg_size, NULL);
        }
        delete[] msg;
        break;
    }
    case File:
    {
        int pthlen;
        recv(Connections[index], (char*)&pthlen, sizeof(int), NULL);
        char* path = new char[pthlen + 1]; path[pthlen] = '\0';
        recv(Connections[index], path, pthlen, NULL);
        FileSend(path, index);
        break;
    }
    default:
        std::cout << "Client " << index << " disconnected!" << std::endl;
        closesocket(Connections[index]);
        Counterclients--;
        Connections[index] = NULL;
        return false;
    }
    return true;
}

void ClientHandler(int index)
{
    Packet packettype;
    while (true)
    {
        recv(Connections[index], (char*)&packettype, sizeof(Packet), NULL);
        if (!ProcessPacket(index, packettype))
        {
            break;
        }
    }
    closesocket(Connections[index]);
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

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
    listen(sListen, SOMAXCONN);
    SOCKET newConnection;

    for (int i = 0; i < COUNTCONN; i++)
    {
        newConnection = accept(sListen, (SOCKADDR*)&addr, &size);
        if (newConnection == 0) {
            std::cout << "Error" << std::endl;
        }
        else
        {
            std::cout << "Client Connected!" << std::endl;
            Connections[i] = newConnection;
            Counterclients++;
            CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);
        }
    }

    WSACleanup();
    system("pause");
    return 0;
}
