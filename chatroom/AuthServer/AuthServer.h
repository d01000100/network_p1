#pragma once

#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <conio.h>
#include <iostream>

#pragma comment (lib, "Ws2_32.lib")

#include "AuthProtocol.h"

#define DEFAULT_BUFLEN 16

struct ClientInfo {
	SOCKET socket;

	// Buffer information (this is basically you buffer class)
	WSABUF dataBuf;
	char buffer[DEFAULT_BUFLEN];
	int bytesRECV;
};

class AuthServer
{
private:
	SOCKET listenSocket, acceptSocket;
	DWORD NonBlock, Flags;
	FD_SET ReadSet;
	ClientInfo* ClientArray[FD_SETSIZE];
	int TotalClients, total;
	void RemoveClient(int index);
	void processMessage(google::protobuf::Message*);
public:
	bool is_on;
	AuthServer();
	bool init();
	google::protobuf::Message* recieveMessage();
	bool sendMessage(std::string serializedMessage);
	void closeServer();
};

