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

const int LeftOthers = 0;
int y = 0;

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
		Form1->CreateLabel(LeftOthers, y, Form1->CharToUString(msg, msg_size));
        y+=23;
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
	recv(Connection, (char*)&packettype, sizeof(Packet), NULL);
	if (!ProcessPacket(packettype))
	{
		closesocket(Connection);
		exit(0);
	}

}

void TForm1::CreateLabel(int x, int y, UnicodeString caption)
{
	//Creating label with mrssage
	TLabel* label = new TLabel(this);
	label->Parent = this;
	label->Caption = caption;
	label->Left = x;
	label->Top = y;
	label->Font->Color = clBlue;
    label->Font->Size = 20;
}

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
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
        exit(1);
    }
    std::cout << "Connected!\n";

    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);
	//Need to create main form
    //ClientWidth - 5 * 5 or ClientWidth - sizeof(symbol) * msg.size();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::BSMsgClick(TObject *Sender)
{
	if(!MsgBox->Text.Length()) return;
	CreateLabel(ClientWidth - 15 * MsgBox->Text.Length(), y, MsgBox->Text);
	y+=23;
	Packet packettype = Message;
	int len = MsgBox->Text.Length();
	send(Connection, (char*)&packettype, sizeof(Packet), NULL);
	send(Connection, (char*)&len, sizeof(int), NULL);
	send(Connection, (char*)MsgBox->Text.c_str(), len, NULL);
	MsgBox->Text = "";
}
//---------------------------------------------------------------------------
UnicodeString TForm1::CharToUString(char* str, int size)
{
	UnicodeString result;
	for(int i = 0; i < size; i++)
	{
		result.Insert(str[i], i);
	}
    return result;
}

