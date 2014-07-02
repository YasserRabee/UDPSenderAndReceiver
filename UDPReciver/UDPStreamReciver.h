
#ifndef UDP_STREAM_RECIVER_H
#define UDP_STREAM_RECIVER_H

#include "UDPReciver.h"
#include "../..//UDPSender/UDPSender/DropingUDPSender.h"
#include "../..//UDPSender/UDPSender/Segment.h"
#include<thread>
#include<fstream>

class UDPStreamReciver
{
public:
	const size_t FILE_SIZE_LEN = 4;
	const size_t FILE_NAME_LEN = 100;
	const size_t SEQ_NUM_LEN = 2;
	const size_t SEG_NUM_LEN = 2;
	const size_t MAX_SEGMENT_LENGTH = (size_t)pow(2, 8 * SEG_NUM_LEN);
	const size_t MAX_FILE_SIZE = (size_t)pow(2, 8 * FILE_SIZE_LEN);
	const size_t HEADER_LENGTH = FILE_SIZE_LEN + FILE_NAME_LEN + SEG_NUM_LEN;

	const unsigned short FIRST_SEQ_NUM = 256;
	
	UDPStreamReciver(int, int, char*, float);
	~UDPStreamReciver();

	int ReciveStream();
	
	/*char* getRecievedStream()
	{ return recStream; }*/
	
	char* getRecievedHeader()
	{ return recHeader; }

	int getRecivePortNumber()
	{ return recPortNum; }

	int getAckPortNumber()
	{ return ackPortNum; }

	size_t getSegmentLength()
	{ return segLength; }

	size_t getRecievedSegmentsNumber()
	{ return recSegmentsNumber; }

	size_t getStreamSize()
	{ return streamSize; }

	char* getFileName()
	{
		return fileName;
	}

private:
	int recPortNum;
	int ackPortNum;

	size_t streamSize; // without header nor seqNumber
	size_t segLength;
	size_t msgSegLength;
	size_t recSegmentsNumber;
	unsigned short expectedSeqNum;
	
	char* recHeader;
	char* fileName;
	char* sPath;
	/*char* recStream;*/
	size_t insertIndex;

	UDPReciver* reciver;
	DropingUDPSender* ackSender;


	ofstream* file;

	Segment Reconstruct(char*); // reconstruct the received msg to Segment
	void DeliverStream(char*, size_t); // write segment msg to file
	void SendAck(unsigned short);
};

UDPStreamReciver::UDPStreamReciver(int recPort, int ackPort, char* savePath, float alpha)
{
	recPortNum = recPort;
	ackPortNum = ackPort;
	sPath = savePath;
	streamSize = 0;
	segLength = 0;
	recSegmentsNumber = 0;
	insertIndex = 0;
	expectedSeqNum = FIRST_SEQ_NUM;
	reciver = new UDPReciver(recPortNum, MAX_SEGMENT_LENGTH);
	ackSender = new DropingUDPSender(ackPortNum, alpha);
}

UDPStreamReciver::~UDPStreamReciver()
{
	delete reciver, ackSender;
}

int UDPStreamReciver::ReciveStream()
{
	cout << "---- Recieve Stream starts... ----" << endl;

	Segment recSeg;
	char* recm;
	do
	{
		recm = reciver->Recive();
		recSeg = Reconstruct(recm);
	} while (recSeg.seqNum != expectedSeqNum);
	
	SendAck(recSeg.seqNum);

	recSegmentsNumber = 1; // first segment contains header + data
	expectedSeqNum++;

	cout << "Recived SEG: seqNum=" << recSeg.seqNum << endl;
	cout << "Recived HEADER MSG:\n";

	for (size_t i = 0; i < HEADER_LENGTH; i++)
	{
		cout << recm[i];
	}
	cout << endl;

	// First segment recived !
	recHeader = new char[HEADER_LENGTH];
	memcpy(recHeader, recSeg.data, HEADER_LENGTH);
	

	for (size_t i = 0; i < HEADER_LENGTH; i++)
	{
		cout << recHeader[i];
	}
	cout << endl;



	size_t fileNameLen = strlen(recHeader);
	size_t savePathLen = strlen(sPath);

	fileName = new char[fileNameLen];
	strcpy(fileName, recHeader);

	char* filePath = new char[savePathLen + fileNameLen];
	memcpy(filePath, sPath, savePathLen);
	memcpy(filePath + savePathLen, fileName, fileNameLen);
	filePath[savePathLen + fileNameLen] = NULL;

	file = new ofstream(filePath, ios::out | ios::binary);

	if (!file->is_open())
	{
		cout << "Error Open file on disk";
		exit(-1);
	}

	memcpy((char*)&streamSize, recHeader + FILE_NAME_LEN, FILE_SIZE_LEN);
	memcpy((char*)&segLength, recHeader + FILE_NAME_LEN + FILE_SIZE_LEN, SEG_NUM_LEN);
	msgSegLength = segLength - SEQ_NUM_LEN;

	//recStream = new char[streamSize];
	size_t firstSegStreamLen = 0;

	if (segLength > MAX_SEGMENT_LENGTH)
	{
		cout << "Error: segLength > MAX_SEGMENT_LENGTH\n";
		return (int)(MAX_SEGMENT_LENGTH);
	}
	else if (segLength < MAX_SEGMENT_LENGTH)
	{ // first segment length is less than MAX_SEGMENT_LENGTH
		reciver->changeMsgLen(segLength);
		firstSegStreamLen = msgSegLength - HEADER_LENGTH;
		DeliverStream(recSeg.data + HEADER_LENGTH, firstSegStreamLen);
	}

	//delete[] recSeg.data;
	size_t streamMsgLen = streamSize + HEADER_LENGTH;
	size_t segmentsNumber = (unsigned int)ceil((float)(streamMsgLen) / (msgSegLength));
	size_t lastSegmentLength = streamMsgLen - (segmentsNumber - 1) * (msgSegLength);
	

	while (recSegmentsNumber < segmentsNumber)
	{
		recSeg = Reconstruct(reciver->Recive());
		if (recSeg.seqNum == expectedSeqNum)
		{ // recived the expected segment
			cout << "Recieved seqNum= " << recSeg.seqNum << endl;
			/*cout << "Recieved data: " << endl;
			for (size_t i = 0; i < msgSegLength; i++)
			{
				cout << recSeg.data[i];
			}
			cout << endl;*/

			SendAck(recSeg.seqNum);

			recSegmentsNumber++;
			if (recSegmentsNumber == segmentsNumber)
				DeliverStream(recSeg.data, lastSegmentLength);
			else
				DeliverStream(recSeg.data, msgSegLength);

			

			expectedSeqNum++;
		}
		else
		{
			SendAck(expectedSeqNum - 1);
		}
		//delete[] recSeg.data;
	}
	file->close();
	return 0;
}

Segment UDPStreamReciver::Reconstruct(char* segMsg)
{
	Segment res;

	memcpy(&res.seqNum, segMsg, SEQ_NUM_LEN);
	res.data = segMsg + SEQ_NUM_LEN;
	return res;
}

void UDPStreamReciver::DeliverStream(char* segData, size_t len)
{
	file->write(segData, len);
	/*free(segData);*/
	/*memcpy(recStream + insertIndex, segData, len);
	insertIndex += len;*/
}

void UDPStreamReciver::SendAck(unsigned short seqNum)
{
	this_thread::sleep_for(chrono::milliseconds(200)); // simulate a propagation delay at the network
	char* ackWord = "ACK";
	char* ackMsg = new char[SEQ_NUM_LEN + strlen(ackWord)];
	memcpy(ackMsg, ackWord, strlen(ackWord));
	memcpy(ackMsg + strlen(ackWord), (char*)&seqNum, SEQ_NUM_LEN);
	ackSender->Send(ackMsg, strlen(ackWord) + SEQ_NUM_LEN);
	delete[] ackMsg;
}

#endif