
#ifndef UDP_FILE_REVIEVER
#define UDP_FILE_REVIEVER

#include"UDPStreamReciver.h"
#include<fstream>
using namespace std;

class UDPFileReciver
{
public:
	const size_t FILE_NAME_LEN = 100;

	UDPFileReciver();
	~UDPFileReciver();

	int RecieveFile(int, int, char*, float);

	char* getFileName()
	{
		return fileName;
	}

	size_t getFileSize()
	{
		return fileSize;
	}

private:

	UDPStreamReciver* reciever;
	char* fileName;
	size_t fileSize;
};

UDPFileReciver::UDPFileReciver()
{

}

UDPFileReciver::~UDPFileReciver()
{
}

int UDPFileReciver::RecieveFile(int recPort, int ackPort, char* savePath, float alpha)
{
	reciever = new UDPStreamReciver(recPort, ackPort, savePath, alpha);

	int ret = reciever->ReciveStream();
	if (ret != 0)
	{
		cout << "Error Streaming!" << endl;
		return -1;
	}

	size_t size;
	char* fileData;
	char* headerData;

	//fileData = reciever->getRecievedStream();
	size = reciever->getStreamSize();
	headerData = reciever->getRecievedHeader();

	size_t fileNameLen = strlen(headerData);
	size_t savePathLen = strlen(savePath);

	fileName = new char[fileNameLen];
	strcpy(fileName, headerData);

	char* filePath = new char[savePathLen + fileNameLen];
	memcpy(filePath, savePath, savePathLen);
	memcpy(filePath + savePathLen, fileName, fileNameLen);
	filePath[savePathLen + fileNameLen] = NULL;

	ofstream file(filePath, ios::out | ios::binary);

	bool f = file.is_open();
	//file.write(fileData, size);
	file.close();

	cout << "The File Written successfully" << endl;

	reciever->~UDPStreamReciver();

	return 0;
}

#endif