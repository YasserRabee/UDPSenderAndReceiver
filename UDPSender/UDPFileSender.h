
#ifndef UDP_FILE_SENDER
#define UDP_FILE_SENDER

#include"UDPStreamSender.h"
#include<math.h>

class UDPFileSender
{
public:

	

	UDPFileSender();
	~UDPFileSender();


	int SendFile(char*, int, unsigned int, unsigned int, unsigned short, int, DWORD, float);

	char* getFileName()
	{
		return fileName;
	}

	size_t getFileSize()
	{
		return fileSize;
	}


private:

	UDPStreamSender* sender;

	
	char* fileContentBuffer;
	char* fileName;
	size_t fileSize;
	DWORD delayTicks;

	char* extractFileName(const char* filePath)
	{
		char* fName = new char[sender->FILE_NAME_LEN];
		char* fExt = new char[10];
		_splitpath_s(filePath, NULL, 0, NULL, 0, fName, sender->FILE_NAME_LEN, fExt, 10);
		strcat(fName, fExt);
		return fName;
	}

};

UDPFileSender::UDPFileSender()
{
}

UDPFileSender::~UDPFileSender()
{
}


int UDPFileSender::SendFile(char* filePath, int portNo, unsigned int segLen, unsigned int buffLen, unsigned short cacheMultiplier, int ackPortNo, DWORD toTicks, float alpha)
{
	FILE* sendFile;
	sendFile = fopen(filePath, "rb");
	if (sendFile == NULL)
	{
		fputs("File error", stderr);
		exit(1);
	}

	fseek(sendFile, 0, SEEK_END);
	fileSize = ftell(sendFile);
	rewind(sendFile);


	/*ifstream file(filePath, ios::in | ios::binary | ios::ate);
	if (file.is_open())
	{*/
	sender = new UDPStreamSender(portNo, segLen, buffLen, ackPortNo, toTicks, alpha);
	//fileSize = file.tellg();


	size_t fileBufferSize = cacheMultiplier*(segLen - sender->SEQ_NUM_LEN);
	int noOfIterations;
	size_t d = fileSize + sender->HEADER_LENGTH;
	noOfIterations = ceil((double)d / (double)fileBufferSize);
	size_t firstIterationSize = fileBufferSize;
	size_t firstIterationFileSize;

	size_t lastIterationSize = (fileSize + sender->HEADER_LENGTH) - ((noOfIterations - 1)*fileBufferSize);

	if (noOfIterations == 1)
	{
		firstIterationSize = lastIterationSize;
	}
	
	//if (noOfIterations != 1)
	//	firstIterationSize = fileBufferSize - sender->HEADER_LENGTH;

	firstIterationFileSize = firstIterationSize - sender->HEADER_LENGTH;
	char* fileContent = new char[firstIterationFileSize];
	
	/*file.seekg(0, ios::beg);
	file.read(fileContent, firstIterationFileSize);*/

	size_t result = fread(fileContent, 1, firstIterationFileSize, sendFile);
	if (result != firstIterationFileSize)
	{
		fputs("Reading error", stderr);
		exit(3);
	}


	fileName = extractFileName(filePath);

	fileContentBuffer = new char[firstIterationSize];
	memcpy(fileContentBuffer, fileName, sender->FILE_NAME_LEN);
	memcpy(fileContentBuffer + sender->FILE_NAME_LEN, (char*)&fileSize, sender->FILE_SIZE_LEN);
	memcpy(fileContentBuffer + sender->FILE_NAME_LEN + sender->FILE_SIZE_LEN, &segLen, sender->SEG_NUM_LEN);
	memcpy(fileContentBuffer + sender->HEADER_LENGTH, fileContent, firstIterationFileSize);

	delete[] fileContent;

	sender->SendStream(fileContentBuffer, firstIterationSize);

	delete[] fileContentBuffer;


	size_t sendBufferSize = fileBufferSize;
	for (size_t i = 1; i < noOfIterations; i++)
	{
		if (i == noOfIterations - 1)
			sendBufferSize = lastIterationSize;

		fileContentBuffer = new char[sendBufferSize];
		/*file.seekg(i * fileBufferSize, ios::beg);
		file.read(fileContentBuffer, sendBufferSize);*/

		result = fread(fileContentBuffer, 1, sendBufferSize, sendFile);
		if (result != sendBufferSize)
		{
			fputs("Reading error", stderr);
			exit(3);
		}

		sender->SendStream(fileContentBuffer, sendBufferSize);
		delete[] fileContentBuffer;
	}



	/*	file.close();

	}
	else
	{
	cout << "Unable to open file" << endl;
	return -1;
	}*/



	sender->~UDPStreamSender();
}


#endif