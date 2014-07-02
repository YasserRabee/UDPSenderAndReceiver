

#include"UDPFileSender.h"

#include<iostream>
using namespace std;


int main()
{
	char c;
	do
	{
		cout << "---- File sender Starts ----\n";

		const size_t filePathLen = 2048;
		char* filePath = new char[filePathLen];
		unsigned short segLen;
		size_t bufferLen;
		unsigned short cacheMultiplier;
		size_t timeOut;
		float alpha;
		int sendPort, ackPort;

		cout << "Enter file path: ";
		cin.getline(filePath, filePathLen);
		filePath[filePathLen] = NULL;

		/*char* recfilePath = new char[filePathLen];
		cout << "Enter file path: ";
		cin.getline(recfilePath, filePathLen);
		recfilePath[filePathLen] = NULL;*/


		cout << "Enter segment length: ";
		cin >> segLen;

		cout << "Enter buffer length: ";
		cin >> bufferLen;

		cout << "Enter cache multiplier: ";
		cin >> cacheMultiplier;

		cout << "Enter timeout [milliseconds]: ";
		cin >> timeOut;

		cout << "Enter send segment droping parameter: ";
		cin >> alpha;


		cout << "Enter send port number: ";
		cin >> sendPort;


		cout << "Enter ack port number: ";
		cin >> ackPort;

		
		
		/*FILE* sendFile, *recFile;
		sendFile = fopen(filePath, "rb");
		recFile = fopen(recfilePath, "rb");
		if (sendFile == NULL || recFile==NULL)
		{
			fputs("File error", stderr);
			exit(1);
		}

		fseek(sendFile, 0, SEEK_END);
		size_t	fileSize = ftell(sendFile);
		rewind(sendFile);

		int Len = 4 * 1024 * 1024;
		int noOfSegs = (int)ceil((float)fileSize / (float)Len);
		int slen = Len;

		for (size_t i = 0; i < noOfSegs; i++)
		{
			if (i == noOfSegs - 1)
			{
				slen = fileSize - (noOfSegs - 1)*Len;
			}

			char* fileContent = new char[slen];
			char* recfileContent = new char[slen];

			size_t result = fread(fileContent, 1, slen, sendFile);
			size_t recresult = fread(recfileContent, 1, slen, recFile);
			if (result != slen||recresult !=slen)
			{
				fputs("Reading error", stderr);
				exit(3);
			}

		}

		return 0;
*/
		

		



		UDPFileSender sender;
		if (sender.SendFile(filePath, sendPort, segLen, bufferLen,cacheMultiplier, ackPort, timeOut, alpha) != 0)
			return 0;


		cout << sender.getFileName() << " sent success!\n-----------\n\n";
		sender.~UDPFileSender();

		cout << "Send another file?: [Y] yes, [N] no ";
		cin >> c;
	} while (c == 'Y' || c == 'y');




	system("pause");


	return 0;
}