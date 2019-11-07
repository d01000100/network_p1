#define WIN32_LEAN_AND_MEAN			// Strip rarely used calls

#include "ProtocolManager.h"
#include "RoomManager.h"
#include "AuthClient.h"

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <conio.h>
#include <iostream>
#include <bcrypt.h>

 //Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5150"
#define ESCAPE 27
#define RETURN 8

int TotalClients = 0, clients = 0;
ClientInfo* ClientArray[FD_SETSIZE];
RoomManager _room_m;
AuthClient authClient;

bool _should_close = false;

SOCKET listenSocket = INVALID_SOCKET;
SOCKET acceptSocket = INVALID_SOCKET;

void key_listen() {
	if (_kbhit())
	{
		char ch = _getch();
		printf("%d\n", ch);
		if (ch == ESCAPE)
		{
			_should_close = true;
		}

		if (ch == 9) { // Tab 
			printf("clients: %d, TotalClients: %d\n", clients, TotalClients);
			for (int i = 0; i < TotalClients; i++) {
				printf("Client: %s, socket: %llu\n", ClientArray[i]->name.c_str(), ClientArray[i]->socket);
			}
			_room_m.printRooms();
		}
	}
}

void disconnectClient(int index) {

	ClientInfo* client = ClientArray[index];
	printf("Client %s with socket %ud disconnected\n", client->name.c_str(), client->socket);
	closesocket(client->socket);

	_room_m.deleteMember(client);
	//_room_m.printRooms();

	for (int j = index; j < TotalClients; j++) {
		ClientArray[j] = ClientArray[j + 1];
	}
	TotalClients--;
}

void closeServer() {
	for (int i = 0; i < TotalClients; i++) {
		if (ClientArray[i]->socket != INVALID_SOCKET) {
			closesocket(ClientArray[i]->socket);
			if (ClientArray[i]->socket == SOCKET_ERROR) {
				printf("error: %d while closing socket from %d client\n",
					WSAGetLastError(), i);
			}
		}
	}
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

void processMessages(ClientInfo* client, Message* recievedMessage) {
	switch (recievedMessage->type) {
	case LOGIN: {
		std::string login_info = ((LoginMessage*)recievedMessage)->client_name;

		std::istringstream iss(login_info);

		// During login, the username and password are recieved in a single
		// string separated by spaces
		std::string username, password;
		iss >> username; iss >> password;

		authClient.sendMessage(writeLoginRequest( username, password));

		printf("Requesting login of %s\n", username.c_str());
		break;
	}
	case JOIN: {
		JoinMessage join = *((JoinMessage*)recievedMessage);
		_room_m.addMember(join.room_name, client);
		printf("%s added to %s\n", client->name.c_str(), join.room_name.c_str());

		RecieveMessage* recv = new RecieveMessage();
		recv->sender_name = "server";
		recv->room_name = join.room_name;
		recv->message = client->name + " joined the room.";
		_room_m.broadcastMessage(recv);
		delete recv;
		break;
	}
	case LEAVE: {
		LeaveMessage* leave = (LeaveMessage*)recievedMessage;
		_room_m.removeMember(leave->room_name, client);
		printf("%s left %s\n", client->name.c_str(), leave->room_name.c_str());

		RecieveMessage* recv = new RecieveMessage();
		recv->sender_name = "server";
		recv->room_name = leave->room_name;
		recv->message = client->name + " left the room.";
		_room_m.broadcastMessage(recv);
		delete recv;

		break;
	}
	case SEND: {
		UserMessage* userMessage = (UserMessage*)recievedMessage;

		RecieveMessage* recv = new RecieveMessage();
		recv->sender_name = client->name;
		recv->room_name = userMessage->room_name;
		recv->message = userMessage->message;
		printf("%s sends %s: %s\n", recv->sender_name.c_str(),
			recv->room_name.c_str(),
			recv->message.c_str());

		_room_m.broadcastMessage(recv);
		delete recv;
	}
	}
}

ClientInfo* findClientByUsername(std::string username)
{
	for (int c = 0; c < TotalClients; c++) {
		if (ClientArray[c]->name == username) {
			return ClientArray[c];
		}
	}

	return NULL;
}

void processAuthMessage(google::protobuf::Message* authMessage) 
{
	std::string messageType = authMessage->GetTypeName();
	if (messageType == "auth_protocol.ResponseOK") {
		auth_protocol::ResponseOK* message = (auth_protocol::ResponseOK*)authMessage;

		switch (message->action()) {
		case auth_protocol::SIGN_UP:
			printf("User created with username %s\n",
				message->username().c_str());
			break;
		case auth_protocol::LOGIN:
			printf("User %s logged in\n",
				message->username().c_str());
			ClientInfo* client = findClientByUsername(message->username());
			if (client) {
				client->is_logged_in = true;
			}
			break;
		}
	}
	if (messageType == "auth_protocol.ResponseError") {
		auth_protocol::ResponseError* message = (auth_protocol::ResponseError*)authMessage;
		std::string error;

		switch (message->error()) {
		case auth_protocol::REPEATED_USERNAME:
			error = "The username is already taken";
			break;
		case auth_protocol::INVALID_CREDENTIALS:
			error = "The username or password are not valid";
			break;
		case auth_protocol::INTERNAL_SERVER_ERROR:
			error = "Internal Server Error";
			break;
		}

		switch (message->action()) {
		case auth_protocol::SIGN_UP:
			printf("Request to create a user with email %s failed because %s\n",
				message->username().c_str(), error.c_str());
			break;
		case auth_protocol::LOGIN:
			printf("Request to login a user with email %s failed becasue %s\n",
				message->username().c_str(), error.c_str());
			break;
		}
	}
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
		closeServer();
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
		closeServer();
		return 1;
	}

	// We don't need this anymore
	freeaddrinfo(addrResult);

	// #3 Listen
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen() failed with error %d\n", WSAGetLastError());
		closeServer();
		return 1;
	}

	// Change the socket mode on the listening socket from blocking to
	// non-blocking so the application will not block waiting for requests
	DWORD NonBlock = 1;
	iResult = ioctlsocket(listenSocket, FIONBIO, &NonBlock);
	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
		closeServer();
		return 1;
	}

	FD_SET ReadSet;
	DWORD flags;
	DWORD RecvBytes;
	timeval timeVal;
	timeVal.tv_sec = 1;

	if (!authClient.init()) {
		closeServer();
		return 1;
	}

	printf("Server ready!\n");

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
		clients = select(0, &ReadSet, NULL, NULL, &timeVal);
		if (clients == SOCKET_ERROR)
		{
			printf("select() failed with error: %d\n", WSAGetLastError());
			closeServer();
			return 1;
		}

		// Check for arriving connections on the listening socket
		if (FD_ISSET(listenSocket, &ReadSet))
		{
			clients--;
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
					printf("Client with socket %u detected\n", info->socket);
				}
			}
		}

		for (int i = 0; i < TotalClients; i++)
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
					else {
						continue;
					}
				}
				else if (RecvBytes > 0)
				{
					printf("Recieved %d bytes\n", RecvBytes);
					client->recvBuf.setDataRecieved(RecvBytes);

					Message* recievedMessage = readMessage(client->recvBuf);

					processMessages(client, recievedMessage);
					
					delete recievedMessage;
				}
				else {
					disconnectClient(i);
				}
			}
		} // for listening to clients

		// Listening to responses from Auth Server
		google::protobuf::Message* authMessage = authClient.recieveMessage();
		if (authMessage)
		{
			processAuthMessage(authMessage);
		}
		authMessage = NULL;
	}

	// cleanup
	closeServer();

	system("Pause");
	return 0;
}