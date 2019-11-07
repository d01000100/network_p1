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
	DWORD Flags, NonBlock;
public:
	AuthClient();
	bool init();
	bool sendMessage(std::string serializedMessage);
	google::protobuf::Message* recieveMessage();
};

