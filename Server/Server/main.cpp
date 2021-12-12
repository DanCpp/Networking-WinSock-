#include <iostream>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable: 4996)

const int COUNTCONN = 100;

SOCKET Connections[COUNTCONN];
int Counterclients = 0;

enum Packet
{
	ChatMessage = 0,
    Test,
	File,
};


bool ProcessPacket(int index, Packet packettype)
{
    switch (packettype)
    {
    case ChatMessage:
    {
        int msg_size;
        recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
        char* msg = new char[msg_size + 1];
        msg[msg_size] = '\0';
        recv(Connections[index], msg, msg_size, NULL);
        for (int i = 0; i < Counterclients; i++)
        {
            if (i == index) continue;
            Packet msg_Packet = ChatMessage;
            send(Connections[i], (char*)&msg_Packet, sizeof(Packet), NULL);
            send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
            send(Connections[i], msg, msg_size, NULL);
        }
        delete[] msg;
        break;
    }
    case File:
        break;
    default:
        std::cout << "Unreconigzed packet: " << packettype << std::endl;
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

void FileSend(SOCKET FileSendSocket, char* FilePath, char* sendbuf, int sendbuflen)
{
    std::streampos filesize = 0;
    std::ifstream in(FilePath, std::ios::binary);
    memset(&sendbuf, (int)sendbuf, sendbuflen);
    if (in.is_open())
    {
        while (1)
        {
            in.read(sendbuf, sendbuflen);
            if (in.eof())
            {
                std::cout << "End of File sending from Client" << std::endl;
                in.close();
                break;
            }
            else
            {
                send(FileSendSocket, sendbuf, sendbuflen, 0);
                memset(&sendbuf, (int)sendbuf, sendbuflen);
            }
        }
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

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
    listen(sListen, SOMAXCONN);
    SOCKET newConnection;
    for (int i = 0; i < 100; i++)
    {
        newConnection = accept(sListen, (SOCKADDR*)&addr, &size);
        if (newConnection == 0) {
            std::cout << "Error" << std::endl;
        }
        else
        {
            std::cout << "Client Connected!" << std::endl;
            //std::string msg = "Hello. It's my first network program";
            //int msg_size = msg.size();
            //Packet msg_Packet = ChatMessage;
            //send(newConnection, (char*)&msg_Packet, sizeof(Packet), NULL);
            //send(newConnection, (char*)&msg_size, sizeof(int), NULL);
            //send(newConnection, msg.c_str(), msg_size, NULL);

            Connections[i] = newConnection;
            Counterclients++;
            CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);
            Packet test = Test;
            send(newConnection, (char*)&test, sizeof(Packet), NULL);
        }
    }
    WSACleanup();
    system("pause");
    return 0;
}
