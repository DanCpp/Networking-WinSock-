#include <iostream>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <stdio.h>
#include <vector>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable: 4996)

const u_int COUNTCONN = 100;

SOCKET Connections[COUNTCONN];
std::vector<u_int> ids_connections;

enum Packet
{
    Message,
    File,
    EXIT,
};

void FatalError(std::string message)
{
    throw new std::exception(message.c_str());
}

void DisconnectClient(u_int index)
{
    std::cout << "Client " << index << " disconnected!" << std::endl;
    auto it = std::find(ids_connections.begin(), ids_connections.end(), index);
    if (it != ids_connections.end())
        ids_connections.erase(it);
    closesocket(Connections[index]);
    Connections[index] = NULL;
}

std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    std::ifstream::pos_type len = in.tellg();
    in.close();
    return len;
}

void FileSend(char* FilePath, int client, char*format, int formatlen)
{
    std::ifstream in(FilePath, std::ios::binary);
    int sendbuflen = filesize(FilePath);
    char* sendbuf = new char[sendbuflen + 1];
    if (in.is_open())
    {
        in.seekg(0, std::ios::beg);
        in.read(sendbuf, sendbuflen);
        for (auto i = ids_connections.cbegin(); i != ids_connections.cend(); i++)
        {
            if (*i == client) continue;
            Packet packet_file = File;
            send(Connections[*i], (char*)&packet_file, sizeof(Packet), NULL);
            send(Connections[*i], (char*)&formatlen, sizeof(int), NULL);
            send(Connections[*i], format, formatlen, NULL);
            send(Connections[*i], (char*)&sendbuflen, sizeof(int), NULL);
            send(Connections[*i], sendbuf, sendbuflen, NULL);
        }
        in.close();
    }
    delete[] sendbuf;

}

void MessageSend(char* msg, int msg_size, int client)
{
    for (auto i = ids_connections.cbegin(); i != ids_connections.cend(); i++)
    {
        if (*i == client) continue;
        Packet packettype = Message;
        send(Connections[*i], (char*)&packettype, sizeof(Packet), NULL);
        send(Connections[*i], (char*)&msg_size, sizeof(int), NULL);
        send(Connections[*i], msg, msg_size, NULL);
    }
}

bool ProcessPacket(int index, Packet packettype)
{
    switch (packettype)
    {
    case Message:
    {
        try {
            int msg_size;
            recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
            char* msg = new char[msg_size + 1]; msg[msg_size] = '\0';
            recv(Connections[index], msg, msg_size, NULL);
            MessageSend(msg, msg_size, index);
            delete[] msg;
        }
        catch (std::exception& exc)
        {
            return false;
        }
        break;
    }
    case File:
    {
        try {
            int pthlen, sizeformat;
            recv(Connections[index], (char*)&sizeformat, sizeof(int), NULL);
            char* format = new char[sizeformat + 1]; format[sizeformat] = '\0';
            recv(Connections[index], format, sizeformat, NULL);
            recv(Connections[index], (char*)&pthlen, sizeof(int), NULL);
            char* path = new char[pthlen + 1]; path[pthlen] = '\0';
            recv(Connections[index], path, pthlen, NULL);
            FileSend(path, index, format, sizeformat);
            delete[] path, format;
        }
        catch (std::exception& exc)
        {
            return false;
        }
        break;
    }
    default:
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
    DisconnectClient(index);
}

int main(int argc, char* argv[])
{
    //WSAStartup
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        FatalError("can not initialize server");
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
            FatalError("can not accept new client");
        }
        else
        {
            std::cout << "Client Connected!" << std::endl;
            Connections[i] = newConnection;
            ids_connections.push_back(i);
            CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);
        }
    }

    WSACleanup();
    system("pause");
    return 0;
}
