#include "AuthClient.h"

#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls

#include "AuthProtocol.h"

#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <conio.h>
#include <iostream>

//Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define AUTH_SERVER_PORT "5160"
#define ESCAPE 27
#define RETURN 8

bool AuthClient::init() {
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
	else
	{
		printf("WSAStartup() was successful!\n");
	}

	// #1 socket
	connectSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo* ptr = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// resolve the server address and port
	iResult = getaddrinfo("127.0.0.1", AUTH_SERVER_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo() failed with error: %d\n", iResult);
		WSACleanup();
		return false;
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
			return false;
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
		return false;
	}
	printf("Successfully connected to the server on socket %d!\n", (int)connectSocket);

	return true;
}

bool AuthClient::sendSignUp(std::string username, std::string password) 
{
	if (connectSocket == INVALID_SOCKET) { return false; }
	std::string signUpRequest = writeSignUpRequest(username, password);
	int iResult = send(connectSocket, signUpRequest.c_str(), signUpRequest.size(), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("sign up request failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return false;
	}

	return true;
}

bool AuthClient::sendLogIn(std::string username, std::string password) 
{
	if (connectSocket == INVALID_SOCKET) { return false; }
	std::string loginRequest = writeLoginRequest(username, password);
	int iResult = send(connectSocket, loginRequest.c_str(), loginRequest.size(), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("login request failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return false;
	}

	return true;
}
