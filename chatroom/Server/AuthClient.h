#pragma once

#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls
#include <Windows.h>
#include <WinSock2.h>
#pragma comment (lib, "Ws2_32.lib")

#include "AuthProtocol.h"

class AuthClient
{
private:
	SOCKET connectSocket;
public:
	bool init();
	bool sendSignUp(std::string username, std::string password);
	bool sendLogIn(std::string username, std::string password);
};

