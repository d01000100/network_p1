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

class AuthServer
{
private:
	SOCKET listenSocket, acceptSocket;
public:
	AuthServer();
	bool init();
	int recieveMessage(char*);
	void closeServer();
};

