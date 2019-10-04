#pragma once
#include <string>
#include <ProtocolManager.h>
#include <vector>
#include <WinSock2.h>

// Client structure
struct ClientInfo {
	SOCKET socket = INVALID_SOCKET;

	// Buffer information (this is basically you buffer class)
	WSABUF dataBuf;
	RecieveBuffer recvBuf;
	std::string name;
};

struct Room {
	std::vector<ClientInfo*> *members = new std::vector<ClientInfo*>;
};