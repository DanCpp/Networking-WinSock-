//---------------------------------------------------------------------------

#include <vcl.h>
#include <iostream>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#pragma hdrstop

#include "main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

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
		//here will be creating label with message
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
        //here will be creating label with message like 'this user sent you a file named: "file.txt"'
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

void TForm1::CreateLabel(int x, int y, UnicodeString caption)
{
    //Do create label
}

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
	//Need to create main form
	CreateLabel(50, 50, "hello");
}
//---------------------------------------------------------------------------
