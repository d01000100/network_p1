#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls

// Include our serializer and protocol
#include <ProtocolManager.h>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <string>
#include <vector>


// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5150"

#define ESCAPE 27
#define RETURN 13
#define BACKSPACE 8

std::vector<std::string> receivedMessages;
RecieveBuffer rcvBuffer;

SendBuffer determineMsgType(std::string msgType, std::string message, std::string roomName = "")
{
	std::string debug = "type: " + msgType + ", room: " + roomName + ", message: " + message + "\n";
	receivedMessages.push_back(debug);
	SendBuffer newBuff;

	if (".send" == msgType)
	{
		UserMessage MSG;
		MSG.room_name = roomName;
		MSG.message = message;
		newBuff = writeMessage(&MSG);
	}
	if (".join" == msgType)
	{
		JoinMessage MSG;
		MSG.room_name = roomName;
		newBuff = writeMessage(&MSG);
	}
	if (".leave" == msgType)
	{
		LeaveMessage MSG;
		MSG.room_name = roomName;
		newBuff = writeMessage(&MSG);
	}
	return newBuff;
}

SendBuffer checkMessage(std::string message)
{
	//printf("checking this message...\n");
	// Look for protocol
	std::string delimiter = " ";

	size_t pos = 0;
	std::string msgType,roomName;

	pos = message.find(delimiter);
	msgType = message.substr(0, pos);
	//std::cout << msgType << std::endl;
	message.erase(0, pos + delimiter.length());

	pos = message.find(delimiter);
	roomName = message.substr(0, pos);
	//std::cout << roomName << std::endl;
	message.erase(0, pos + delimiter.length());

	return determineMsgType(msgType,message,roomName);
}

int main(int argc, char** argv)
{
	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		// Something went wrong, tell the user the error id
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}
	else
	{
		printf("WSAStartup() was successful!\n");
	}

	// #1 socket
	SOCKET connectSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo* ptr = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// resolve the server address and port
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo() failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("getaddrinfo() successful!\n");
	}

	// #2 connect
	// Attempt to connect to the server until a socket succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connecting to the server
		connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (connectSocket == INVALID_SOCKET)
		{
			printf("socket() failed with error code %d\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return 1;
		}

		// Attempt to connect to the server
		iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			printf("connect() failed with error code %d\n", WSAGetLastError());
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(result);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to the server!\n");
		WSACleanup();
		return 1;
	}
	printf("Successfully connected to the server on socket %d!\n", (int)connectSocket);

	// Make user login
	std::string client_name;
	std::cout << "Input username:\t";
	std::getline(std::cin, client_name);

	LoginMessage loginMessage;
	loginMessage.client_name = client_name;
	SendBuffer theBuffer;
	theBuffer = writeMessage(&loginMessage);

	printf("Logging in to the server...\n");
	iResult = send(connectSocket, (char*)theBuffer.getBuffer(), theBuffer.getDataLength(), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send() failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}
	// printf("Bytes sent: %d\n", iResult);

	system("cls");
	std::cout << "Welcome " << client_name << "!!\n" << std::endl;
	std::cout << "======================================\n" << std::endl;
	std::cout << "Received Messages: \n\n\n\n" << std::endl;
	printf("Your Message: ");

	// #3 write & read
	bool should_exit = false;
	std::string* message = new std::string();
	DWORD NonBlock = 1;

	while (!should_exit)
	{
		if (_kbhit())
		{
			char ch = _getch();
			if (ch == ESCAPE)
			{
				should_exit = true;
			}
			if (ch == BACKSPACE)
			{
				message->pop_back();
			}
			if (ch == RETURN)
			{

				SendBuffer theSecondBuffer = checkMessage(*message);
				message->clear();
				if (theSecondBuffer.getDataLength()==0) { continue; }

				iResult = send(connectSocket, (char*)theSecondBuffer.getBuffer(), theSecondBuffer.getDataLength(), 0);
				if (iResult == SOCKET_ERROR)
				{
					printf("send() failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					std::cout << theSecondBuffer.getDataLength() << std::endl;
					system("pause");
					return 1;
				}
			}
			else
			{
				message->push_back(ch);
				system("cls");
				std::cout << "Welcome " << client_name << "!!\n" << std::endl;
				std::cout << "======================================\n" << std::endl;
				std::cout << "Received Messages: \n" << std::endl;
				for (int i = 0; i < receivedMessages.size(); i++)
				{
					printf("$ %s\n", receivedMessages[i].c_str());
				}
				std::cout << "\n\n\n" << std::endl;
				printf("Your Message: %s", message->c_str());
			}
		}

		// Receive messages from the server
		//char recvbuf[DEFAULT_BUFLEN];
		//int recvbuflen = DEFAULT_BUFLEN;
		iResult = ioctlsocket(connectSocket, FIONBIO, &NonBlock);
		if (iResult == SOCKET_ERROR)
		{
			printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			system("pause");
			return 1;
		}
		//printf("Waiting to receive data from the server...\n");
		iResult = recv(connectSocket, (char*)rcvBuffer.getBuffer(), DEFAULT_BUFLEN, 0);
		if (iResult > 0)
		{
			printf("Bytes received: %d\n", iResult);
			rcvBuffer.setDataRecieved(iResult);
			Message* MSG = readMessage(rcvBuffer);
			if (MSG->type == RECIEVE)
			{
				RecieveMessage* rcvMSG = (RecieveMessage*)MSG;
				std::string tempString;
				tempString += (rcvMSG->room_name + "." + rcvMSG->sender_name + ":\t" + rcvMSG->message);
				receivedMessages.push_back(tempString);
				if(receivedMessages.size() > 3)
				{ // erase the 1st element
					receivedMessages.erase(receivedMessages.begin());
				}
			}
		}
		else if (iResult == 0)
		{
			printf("Connection closed\n");
		}
		else
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK) // currently no data available
			{  
				continue;
			}
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		}

	} // while (!should_exit)

	// #4 close
	closesocket(connectSocket);
	WSACleanup();

	system("pause");

	return 0;
}