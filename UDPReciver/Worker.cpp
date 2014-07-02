

#include"UDPFileReciver.h"
#include<iostream>
using namespace std;


int main()
{
	char c;
	do
	{
		cout << "---- File reciver Starts ----\n";

		const size_t savePathLen = 2048;
		char* savePath = new char[savePathLen];
		float alpha;
		int recPort, ackPort;

		cout << "Enter save path: ";
		cin.getline(savePath, savePathLen);
		savePath[savePathLen] = NULL;

		cout << "Enter ack send droping parameter: ";
		cin >> alpha;


		cout << "Enter recieve port number: ";
		cin >> recPort;


		cout << "Enter ack port number: ";
		cin >> ackPort;


		/*UDPFileReciver reciver;

		reciver.RecieveFile(recPort,ackPort, savePath, alpha);*/

		UDPStreamReciver reciver(recPort, ackPort, savePath, alpha);
		
		if (reciver.ReciveStream() != 0)
		{
			cout << "Error!!!!!!!";
			exit(-1);
		}


		cout << reciver.getFileName() << " recieve success!\n-----------\n\n";

		cout << "Recieve another file?: [Y] yes, [N] no ";
		cin >> c;
	} while (c == 'Y' || c == 'y');
	
	system("pause");


	return 0;

	


	cout << endl << "\n#########" << endl << "File Recived succeffuly..!\n\n";

	

	return 0;
}