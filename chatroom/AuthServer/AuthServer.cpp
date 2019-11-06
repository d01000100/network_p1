#include "AuthServer.h"

#define DEFAULT_BUFLEN 512
#define AUTH_SERVER_PORT "5160"
#define ESCAPE 27
#define RETURN 8

AuthServer::AuthServer() {
	listenSocket = INVALID_SOCKET;
	acceptSocket = INVALID_SOCKET;
}

bool AuthServer::init() {
	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		// Something went wrong, tell the user the error id
		printf("WSAStartup failed with error: %d\n", iResult);
		return false;
	}

	// #1 Socket
	struct addrinfo* addrResult = NULL;
	struct addrinfo hints;

	// Define our connection address info 
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, AUTH_SERVER_PORT, &hints, &addrResult);
	if (iResult != 0)
	{
		printf("getaddrinfo() failed with error %d\n", iResult);
		WSACleanup();
		return false;
	}

	// Create a SOCKET for connecting to the server
	listenSocket = socket(
		addrResult->ai_family,
		addrResult->ai_socktype,
		addrResult->ai_protocol
	);
	if (listenSocket == INVALID_SOCKET)
	{
		// https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
		printf("socket() failed with error %d\n", WSAGetLastError());
		freeaddrinfo(addrResult);
		closeServer();
		return false;
	}

	// #2 Bind - Setup the TCP listening socket
	iResult = bind(
		listenSocket,
		addrResult->ai_addr,
		(int)addrResult->ai_addrlen
	);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(addrResult);
		closeServer();
		return false;
	}

	// We don't need this anymore
	freeaddrinfo(addrResult);

	// #3 Listen 
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen() failed with error %d\n", WSAGetLastError());
		closeServer();
		return false;
	}

	// #4 Accept		(Blocking call)
	printf("Waiting for Chat Server to connect...\n");
	acceptSocket = accept(listenSocket, NULL, NULL);
	if (acceptSocket == INVALID_SOCKET)
	{
		printf("accept() failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("Accepted Chat Server on socket %d\n", acceptSocket);
	}

	return true;
}

int AuthServer::recieveMessage(char *recvbuf) 
{
	if (acceptSocket) {
		int recvbuflen = DEFAULT_BUFLEN;

		int iResult = recv(acceptSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Recibi %d bytes del Chat Server!!\n", iResult);
		}
		if (iResult < 0)
		{
			printf("recv failed with error: %d\n", WSAGetLastError());
			closeServer();
		}
		else if( iResult == 0 )
		{
			printf("Connection closing...\n");
			closeServer();
		}

		return iResult;
	}

	return -1;
}

void AuthServer::closeServer() {

	if (listenSocket != INVALID_SOCKET) {
		closesocket(listenSocket);
		if (listenSocket == SOCKET_ERROR) {
			printf("error: %d while closing listenSocket\n",
				WSAGetLastError());
		}
	}
	if (acceptSocket != INVALID_SOCKET) {
		closesocket(acceptSocket);
		if (acceptSocket == SOCKET_ERROR) {
			printf("error: %d while closing acceptSocket\n",
				WSAGetLastError());
		}
	}
	WSACleanup();
}
