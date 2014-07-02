
#ifndef UDP_STREAM_SENDER_H
#define UDP_STREAM_SENDER_H




#include"DropingUDPSender.h"
#include"Window.h"
#include"../../UDPReciver/UDPReciver/UDPReciver.h""
#include<math.h>
#include<atltime.h>
#include<thread>
#include <chrono>
#include<mutex>
#include <condition_variable>
#include<iostream>
#include <fstream>
using namespace std;

class UDPStreamSender
{
public:
	const size_t FILE_SIZE_LEN = 4;
	const size_t FILE_NAME_LEN = 100;
	const size_t SEQ_NUM_LEN = 2;
	const size_t SEG_NUM_LEN = 2;
	const size_t MAX_SEGMENT_LENGTH = pow(2, 8 * SEG_NUM_LEN);
	const size_t MAX_FILE_SIZE = pow(2, 8 * FILE_SIZE_LEN);
	const size_t HEADER_LENGTH = FILE_SIZE_LEN + FILE_NAME_LEN + SEG_NUM_LEN;
	const size_t ACK_LEN = 5;

	const unsigned short START_SEQ_NUM = 255;
	const unsigned short BUFFER_FULL_WAIT = 50;

	UDPStreamSender(int, unsigned int, unsigned int, int, DWORD, float);
	~UDPStreamSender();

	void SendStream(const char*, size_t);
	void SendStream(const char*, size_t, bool);

	int getSendPortNumber()
	{ return sendPortNumber; }

	int getAckPortNumber()
	{ return ackPortNumber; }

	bool IsRunning()
	{ return _isRunning; }

private:
	unsigned int segLength;
	unsigned int msgLength; // long stram length
	unsigned int msgSegLen; // length of data at a segment
	unsigned int segmentsNumber;
	unsigned short lastSeqNum; // last sent seqNum

	float _alpha;
	DWORD timeOutTicks; // timeout inerval
	DWORD timeoutThreadSleep;
	bool _isRunning;
	bool _allRecived;

	size_t bufferLength;
	Window* sendBuffer;

	std::mutex bufLock; // lock for thead safety
	condition_variable stopIfEmpty, stopifFull;

	DropingUDPSender* streamSender;
	int sendPortNumber;

	UDPReciver* ackReciver;
	int ackPortNumber;


	void InitializeNewSendSession()
	{
		sendBuffer = new Window(bufferLength);

		_isRunning = false;
		_allRecived = false;
	}

	int SendSegment(const Segment*);
	void SendBuffer();
	void DoSegmentation(const char*, unsigned int);
	void AckRecive();
	void ManageTimeout();
};

UDPStreamSender::UDPStreamSender(int portNo, unsigned int segLen, unsigned int buffLen, int ackPortNo, DWORD toTicks, float alpha)
{
	lastSeqNum = START_SEQ_NUM;
	sendPortNumber = portNo;
	ackPortNumber = ackPortNo;
	segLength = segLen;
	timeOutTicks = toTicks;
	timeoutThreadSleep = timeOutTicks * 0.5;
	bufferLength = buffLen;

	streamSender = new DropingUDPSender(sendPortNumber, alpha);
	ackReciver = new UDPReciver(ackPortNumber, ACK_LEN);
	InitializeNewSendSession();
}

UDPStreamSender::~UDPStreamSender()
{
	delete sendBuffer;
	delete streamSender;
	delete ackReciver;
}

void UDPStreamSender::SendStream(const char* sendMsg, size_t size)
{
	InitializeNewSendSession();
	cout << "---- Send Stream starts... ----" << endl;

	char* longMsg = new char[size];
	memcpy(longMsg, sendMsg, size);
	
	std::thread segThread(&UDPStreamSender::DoSegmentation, this, longMsg, size);
	_isRunning = true;
	std::thread sendThrad(&UDPStreamSender::SendBuffer, this);
	std::thread ackThrad(&UDPStreamSender::AckRecive, this);
	std::thread timeOutThread(&UDPStreamSender::ManageTimeout, this);
	
	segThread.join();
	sendThrad.join();
	_isRunning = false;
	ackThrad.join();
	timeOutThread.join();

	delete[] longMsg;

	cout << "---- Send Stream done! ----" << endl;
}

void UDPStreamSender::SendStream(const char* sendMsg, size_t size, bool newStream)
{
	lastSeqNum = START_SEQ_NUM;
	SendStream(sendMsg, size);
}

void UDPStreamSender::DoSegmentation(const char* longMsg, unsigned int size)
{
	Segment* cSeg = new Segment();
	cSeg->seqNum = lastSeqNum;

	msgLength = size;
	msgSegLen = segLength - SEQ_NUM_LEN;

	segmentsNumber = (unsigned int)ceil((float)msgLength / (msgSegLen));

	for (size_t i = 0; i < segmentsNumber; i++)
	{

		cSeg->seqNum++;
		lastSeqNum = cSeg->seqNum;

		cSeg->data = new char[msgSegLen];

		int sendMsgSegLen = msgSegLen;
		if (i == segmentsNumber - 1)
			sendMsgSegLen = msgLength - i*msgSegLen;

		memcpy(cSeg->data, longMsg + i*msgSegLen, sendMsgSegLen);

		// Add to send buffer
		std::unique_lock<std::mutex> mtx_lock(bufLock);
		/*bool isFull = sendBuffer->isFull();
		bufLock.unlock();*/
		while (sendBuffer->isFull())
		{
			stopifFull.wait(mtx_lock);
			////cv.wait(bufLock);
			//this_thread::sleep_for(std::chrono::milliseconds(100));
			//bufLock.lock();
			////isFull = sendBuffer->isFull();
			//bufLock.unlock();
		}
		cSeg->isSent = false;

		//bufLock.lock();
		sendBuffer->Add(cSeg);
		mtx_lock.unlock();
		stopIfEmpty.notify_all();
	}
	
}

int UDPStreamSender::SendSegment(const Segment* seg)
{
	char* sMsg = new char[segLength];
	sMsg[0] = NULL;

	memcpy(sMsg, (char*)&seg->seqNum, SEQ_NUM_LEN);
	memcpy(sMsg + SEQ_NUM_LEN, seg->data, msgSegLen);

	int st = streamSender->Send(sMsg, segLength);

	delete sMsg;
	return st;
}

void UDPStreamSender::SendBuffer()
{
	
	bufLock.lock();
	bool hasElms = sendBuffer->hasElems();
	bufLock.unlock();

	while (_isRunning || hasElms)
	{
		std::unique_lock<std::mutex> mtx_lock(bufLock);
		size_t firstUnsent = sendBuffer->firstUnsent();
		size_t buffCount = sendBuffer->count();

		while (!sendBuffer->hasElems())
		{
			stopIfEmpty.wait_for(mtx_lock, std::chrono::milliseconds(100));
			if (_allRecived)
				return;
		}

		//mtx_lock.unlock();

		//if (firstUnsent == buffCount)
		//{
		//	this_thread::sleep_for(std::chrono::milliseconds(200));
		//	/*continue;*/
		//}
		for (size_t i = firstUnsent; i < buffCount; i++)
		{
			/*bufLock.lock();*/
			Segment sSeg = sendBuffer->at(i);
			int st = SendSegment(&sSeg);
			if (st >= segLength)
			{
				sSeg.isSent = true;
				DWORD j = GetTickCount();
				sSeg.timeOutTick = j + timeOutTicks;
				sendBuffer->Replace(i, &sSeg);
				sendBuffer->sendSeg();
				cout << "SEG sent: SeqNum=" << sSeg.seqNum << endl;
			}
			/*bufLock.unlock();*/
		}
		//bufLock.lock();
		hasElms = sendBuffer->hasElems();
		//bufLock.unlock();
		mtx_lock.unlock();
	}
}

void UDPStreamSender::AckRecive()
{
	char* ackMessage;
	while (!_allRecived)
	{
		
		/*while (!sendBuffer->hasElems())
		{
			stopIfEmpty.wait(mtx_lock);
		}*/

		ackMessage = ackReciver->Recive();
		
		string ackMsg(ackMessage);
		if (ackMsg.substr(0, 3).compare("ACK") != 0)
			continue;

		unsigned short ackSeqNum = 0;
		memcpy(&ackSeqNum, ackMessage + 3, 2);

		cout << "ACK recieved: seqNum=" << ackSeqNum << endl;

		/*bufLock.lock();*/
		std::unique_lock<mutex> mtx_lock(bufLock);
		sendBuffer->Ack(ackSeqNum);
		_allRecived = !sendBuffer->hasElems() || !_isRunning;
		mtx_lock.unlock();
		stopifFull.notify_one();
		//cv.notify_all();
	}
}

void UDPStreamSender::ManageTimeout()
{
	bufLock.lock();
	bool hasElms = sendBuffer->hasElems();
	bufLock.unlock();

	while (!_allRecived)
	{
		//bufLock.lock();
		std::unique_lock<mutex> mtx_lock(bufLock);
		while (!sendBuffer->hasElems())
		{
			stopIfEmpty.wait_for(mtx_lock, std::chrono::milliseconds(100));
			if (_allRecived)
				return;
		}

		DWORD segTick = sendBuffer->first().timeOutTick;
		DWORD cTick = GetTickCount();
		if (sendBuffer->hasElems() && cTick > segTick)
		{ // timeout, resend all buffer
			sendBuffer->resendAll();
			cout << "Timeout: seqNum=" << sendBuffer->first().seqNum << endl;
		}
		/*bufLock.unlock();*/

		this_thread::sleep_for(std::chrono::milliseconds(timeoutThreadSleep));

		/*bufLock.lock();*/
		hasElms = sendBuffer->hasElems();
		//bufLock.unlock();
		mtx_lock.unlock();
	}
}


#endif	