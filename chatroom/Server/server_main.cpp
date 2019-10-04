#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls

#include "ProtocolManager.h"
#include "RoomManager.h"

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

 //Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5150"
#define ESCAPE 27
#define RETURN 8

int TotalClients = 0;
ClientInfo* ClientArray[FD_SETSIZE];
RoomManager _room_m;

bool _should_close = false;

void key_listen() {
	if (_kbhit())
	{
		char ch = _getch();
		printf("%d\n", ch);
		if (ch == ESCAPE)
		{
			_should_close = true;
		}
	}
}

void disconnectClient(int index) {

	ClientInfo* client = ClientArray[index];
	printf("Client %s with socket %lu disconnected\n", client->name.c_str(), client->socket);
	closesocket(client->socket);

	_room_m.deleteMember(client);
	//_room_m.printRooms();

	for (int j = index; j < TotalClients; j++) {
		ClientArray[j] = ClientArray[j + 1];
	}
	TotalClients--;
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

	// #1 Socket
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET acceptSocket = INVALID_SOCKET;

	struct addrinfo* addrResult = NULL;
	struct addrinfo hints;

	// Define our connection address info 
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addrResult);
	if (iResult != 0)
	{
		printf("getaddrinfo() failed with error %d\n", iResult);
		WSACleanup();
		return 1;
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
		WSACleanup();
		return 1;
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
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// We don't need this anymore
	freeaddrinfo(addrResult);

	// #3 Listen
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen() failed with error %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Change the socket mode on the listening socket from blocking to
	// non-blocking so the application will not block waiting for requests
	DWORD NonBlock = 1;
	iResult = ioctlsocket(listenSocket, FIONBIO, &NonBlock);
	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	FD_SET ReadSet;
	int total;
	DWORD flags;
	DWORD RecvBytes;
	timeval timeVal;
	timeVal.tv_sec = 1;

	while (!_should_close)
	{
		key_listen();
		// Initialize our read set
		FD_ZERO(&ReadSet);

		// Always look for connection attempts
		FD_SET(listenSocket, &ReadSet);

		// Set read notification for each socket.
		for (int i = 0; i < TotalClients; i++)
		{
			FD_SET(ClientArray[i]->socket, &ReadSet);
		}

		// Call our select function to find the sockets that
		// require our attention
		total = select(0, &ReadSet, NULL, NULL, &timeVal);
		if (total == SOCKET_ERROR)
		{
			printf("select() failed with error: %d\n", WSAGetLastError());
			return 1;
		}

		// Check for arriving connections on the listening socket
		if (FD_ISSET(listenSocket, &ReadSet))
		{
			total--;
			acceptSocket = accept(listenSocket, NULL, NULL);

			if (acceptSocket == INVALID_SOCKET)
			{
				printf("accept() failed with error %d\n", WSAGetLastError());
				return 1;
			}
			else
			{
				iResult = ioctlsocket(acceptSocket, FIONBIO, &NonBlock);
				if (iResult == SOCKET_ERROR)
				{
					printf("listen() failed with error %d\n", WSAGetLastError());
					closesocket(listenSocket);
					WSACleanup();
					return 1;
				}
				else {
					ClientInfo* info = new ClientInfo();
					info->socket = acceptSocket;
					ClientArray[TotalClients] = info;
					TotalClients++;
				}
			}
		}

		for (int i = 0; i < total && i < TotalClients; i++)
		{
			ClientInfo* client = ClientArray[i];

			// If the ReadSet is marked for this socket, then this means data
			// is available to be read on the socket
			if (FD_ISSET(client->socket, &ReadSet))
			{
				client->dataBuf.buf = (char*)client->recvBuf.getBuffer();
				client->dataBuf.len = DEFAULT_BUFLEN;

				DWORD Flags = 0;
				iResult = WSARecv(
					client->socket,
					&(client->dataBuf),
					1,
					&RecvBytes,
					&Flags,
					NULL,
					NULL
				);

				if (iResult == SOCKET_ERROR)
				{
					// WSAEWOULDBLOCK means that the client is not sending
					// anything right now, not that an actual error preventing
					// the comunication has happened
					if (WSAGetLastError() != WSAEWOULDBLOCK) {
						printf("WSARecv failed with error: %d\n", WSAGetLastError());
						disconnectClient(i);
					}
				}
				else if (RecvBytes > 0)
				{
					client->recvBuf.setDataRecieved(RecvBytes);
					Message* recievedMessage = readMessage(client->recvBuf);

					switch (recievedMessage->type) {
					case LOGIN: {
							client->name = ((LoginMessage*)recievedMessage)->client_name;
							printf("%s logged in\n", ((LoginMessage*)recievedMessage)->client_name.c_str());
							break;
						}
						case JOIN: {
							JoinMessage join = *((JoinMessage*)recievedMessage);
							_room_m.addMember(join.room_name, client);
							printf("%s added to %s\n", client->name.c_str(), join.room_name.c_str());
							//_room_m.printRooms();
							break;
						}						
						case LEAVE: {
							LeaveMessage* leave = (LeaveMessage*)recievedMessage;
							_room_m.removeMember(leave->room_name, client);
							printf("%s left %s\n", client->name.c_str(), leave->room_name.c_str());
							//_room_m.printRooms();
							break;
						}
						case SEND: {
							UserMessage *userMessage = (UserMessage*)recievedMessage;
							printf("%s sends %s: %s\n", client->name.c_str(), 
								userMessage->room_name.c_str(),
								userMessage->message.c_str());
							std::vector<ClientInfo*> *clients = _room_m.getMembers(userMessage->room_name);
							for (int c = 0; c < clients->size(); c++) {
								ClientInfo* client = clients->at(c);

								RecieveMessage* recv = new RecieveMessage();
								recv->sender_name = client->name;
								recv->room_name = userMessage->room_name;
								recv->message = userMessage->message;
								SendBuffer buffer = writeMessage(recv);

								printf("Echoing message...\n");
								iResult = send(client->socket, (char*)buffer.getBuffer(), buffer.getDataLength(), 0);
								if (iResult == SOCKET_ERROR)
								{
									printf("send() failed with error: %d\n", WSAGetLastError());
								}
								printf("RECIEVE. Bytes sent: %d\n", iResult);
							}
						}
					}
				}
				else {
					disconnectClient(i);
				}
			}
		}
	}

	// #6 close
	iResult = shutdown(acceptSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(acceptSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(acceptSocket);
	WSACleanup();

	return 0;
}