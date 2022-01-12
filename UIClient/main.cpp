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

const int LeftOthers = 0;
int y = 0;

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

std::ifstream::pos_type filesize(const std::string filename)
{
	std::ifstream in(filename, std::ios_base::ate | std::ios_base::binary);
    std::ifstream::pos_type len = in.tellg();
    in.close();
    return len;
}

void FileSend(const char* FilePath)
{
ShowMessage(FilePath);
    std::ifstream in(FilePath, std::ios_base::binary);
    int sendbuflen = filesize(FilePath);
    char* sendbuf = new char[sendbuflen + 1];
    if (in.is_open())
    {
        in.seekg(0, std::ios::beg);
        in.read(sendbuf, sendbuflen);
        send(Connection, (char*)&sendbuflen, sizeof(int), NULL); //2
        send(Connection, sendbuf, sendbuflen, NULL);//2
        in.close();
    }
    else throw new std::exception("Cannot find file at your path");
    delete[] sendbuf;

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

void SendFile(std::string& msg)
{
	int point = msg.rfind('\\');
    if (point == std::string::npos)
    {
        point = msg.rfind('/');
	}
	ShowMessage("here");
    FileSend(msg.c_str());
	msg = msg.substr(point + 1);
	int len = msg.size();
	send(Connection, (char*)&len, sizeof(int), NULL);
	send(Connection, msg.c_str(), len, NULL);
    ShowMessage("here");
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
		//std::cout << msg << std::endl;
		Form1->CreateLabel(LeftOthers, y, msg);
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
		Form1->CreateLabel(LeftOthers, y, "you recieved a file");
        y+=23;
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
	char* msg = ToChar(MsgBox->Text);
	send(Connection, (char*)&packettype, sizeof(Packet), NULL);
	send(Connection, (char*)&len, sizeof(int), NULL);
	send(Connection, msg, len, NULL);
	MsgBox->Text = "";
}
//---------------------------------------------------------------------------
char* TForm1::ToChar(UnicodeString str)
{
    int len = str.Length();
	char* res = new char[len + 1];
	res[len] = '\0';
	for(int i = 0; i < len; i++)
	{
		res[i] = str[i + 1];
	}
    return res;
}


void __fastcall TForm1::BSFileClick(TObject *Sender)
{
	if(!OpenDialog1->Execute()) return;
	Packet packettype = File;
	send(Connection, (char*)&packettype, sizeof(Packet), NULL);
	char* filename = ToChar(OpenDialog1->FileName);
	SendFile((std::string&)filename);
	CreateLabel( ClientWidth - 15 * OpenDialog1->FileName.Length(), y, "you sent a file");
    y+=23;
}
//---------------------------------------------------------------------------

