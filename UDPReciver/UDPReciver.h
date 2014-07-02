
#ifndef UDP_RECIVER_H
#define UDP_RECIVER_H

#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <iostream>
using namespace std;

class UDPReciver
{
public:
	UDPReciver(int, size_t);
	~UDPReciver();


	char* Recive();

	void changeMsgLen(size_t newMsgLen)
	{ msgLen = newMsgLen; }

private:
	int portNumber;
	char* recMsg; // received message
	size_t msgLen;  // received message length
	int len;

	SOCKET recSocket;
	sockaddr_in recAddress;

	void refineMsgStr(char*); // insert null at the end of msg
	int Initialize();
	int Finalize();
};

UDPReciver::UDPReciver(int portNo, size_t msglen)
{
	portNumber = portNo;
	msgLen = msglen;
	Initialize();
}

UDPReciver::~UDPReciver()
{
	Finalize();
}

int UDPReciver::Initialize()
{
	WSADATA wsaData;

	//create socket
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		cerr << "Socket Initialization: Error with WSAStartup\n";
		system("pause");
		WSACleanup();
		exit(10);
	}

	recSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (recSocket == INVALID_SOCKET)
	{
		cerr << "Socket Initialization: Error creating socket" << endl;
		system("pause");
		WSACleanup();
		exit(11);
	}

	//bind
	memset(&recAddress, 0, sizeof(recAddress));
	recAddress.sin_family = AF_INET;
	recAddress.sin_addr.s_addr = INADDR_ANY;
	recAddress.sin_port = htons(portNumber);
	len = sizeof(struct sockaddr_in);


	if (bind(recSocket, (SOCKADDR*)&recAddress, sizeof(recAddress)) == SOCKET_ERROR)
	{
		cerr << "ServerSocket: Failed to connect\n";
		system("pause");
		WSACleanup();
		exit(14);
	}

	return 0;

}

int UDPReciver::Finalize()
{
	return ::closesocket(recSocket);
}

char* UDPReciver::Recive()
{
	recMsg = new char[msgLen];

	recvfrom(recSocket, recMsg, msgLen, 0, (SOCKADDR*)&recAddress, &len);
	refineMsgStr(recMsg);

	return recMsg;
}

void UDPReciver::refineMsgStr(char* msgStr)
{
	*(msgStr + msgLen) = NULL;
}

#endif