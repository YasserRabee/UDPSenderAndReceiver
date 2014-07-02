

#ifndef UDP_SENDER_H
#define UDP_SENDER_H

#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include<iostream>
using namespace std;

class UDPSender
{
public:
	UDPSender(int);
	~UDPSender();

	virtual int Send(char*, size_t);
	
	int get_portNumber()
	{ return portNumber; }

private:
	int portNumber;
	SOCKET sendSocket;
	sockaddr_in sendAddr;
	int len;

protected:
	virtual int Initialize();
	virtual int Finalize();
};

UDPSender::UDPSender(int portNo)
{
	portNumber = portNo;
	Initialize();
}

UDPSender::~UDPSender()
{
	Finalize();
}

int UDPSender::Initialize()
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

	sendSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sendSocket == INVALID_SOCKET)
	{
		cerr << "Socket Initialization: Error creating socket" << endl;
		system("pause");
		WSACleanup();
		exit(11);
	}

	char opt = 1;
	setsockopt(sendSocket, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(char));

	memset(&sendAddr, 0, sizeof(sendAddr));
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_addr.s_addr = INADDR_BROADCAST;
	sendAddr.sin_port = htons(portNumber);
	len = sizeof(struct sockaddr_in);

	return 0;
}

int UDPSender::Finalize()
{
	return ::closesocket(sendSocket);
}

int UDPSender::Send(char* msg, size_t size)
{
	return sendto(sendSocket, msg, size, 0, (sockaddr*)&sendAddr, len);
	delete[] msg;
}


#endif