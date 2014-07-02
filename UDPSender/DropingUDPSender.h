
#ifndef DROPPING_UDP_SENDER_H
#define DROPPING_UDP_SENDER_H

#include"UDPSender.h"

class DropingUDPSender : public UDPSender
{
public:
	DropingUDPSender(int, float);
	~DropingUDPSender();

	int Send(char*, unsigned int);

private:
	float _alpha;
};

DropingUDPSender::DropingUDPSender(int portNo, float alpha)
:UDPSender(portNo)
{
	_alpha = alpha;
}

DropingUDPSender::~DropingUDPSender()
{
}

int DropingUDPSender::Send(char* msg, size_t size)
{
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

	if (r < _alpha)
	{
		cout << "\nDropping seqNum= ";
		return INT_MIN;
	}

	return UDPSender::Send(msg, size);
}

#endif