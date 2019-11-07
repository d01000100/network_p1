#include "AuthServer.h"

#define DEFAULT_BUFLEN 512
#define AUTH_SERVER_PORT "5160"
#define ESCAPE 27
#define RETURN 8

AuthServer::AuthServer() {
	listenSocket = INVALID_SOCKET;
	acceptSocket = INVALID_SOCKET;
	NonBlock = 1, Flags = 0;
	is_on = false;
	TotalClients = 0;
}

void AuthServer::RemoveClient(int index)
{
	ClientInfo* client = ClientArray[index];
	closesocket(client->socket);
	printf("Closing socket %d\n", (int)client->socket);

	for (int clientIndex = index; clientIndex < TotalClients; clientIndex++)
	{
		ClientArray[clientIndex] = ClientArray[clientIndex + 1];
	}

	TotalClients--;

	delete client;
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
	printf("ioctlsocket() was successful!\n");

	is_on = true;

	return true;
}

google::protobuf::Message* AuthServer::recieveMessage()
{
	DWORD RecvBytes;
	timeval tv = { 0 };
	tv.tv_sec = 1;
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
	total = select(0, &ReadSet, NULL, NULL, &tv);
	if (total == SOCKET_ERROR)
	{
		printf("select() failed with error: %d\n", WSAGetLastError());
		return NULL;
	}

	// #4 Check for arriving connections on the listening socket
	if (FD_ISSET(listenSocket, &ReadSet))
	{
		total--;
		acceptSocket = accept(listenSocket, NULL, NULL);
		if (acceptSocket == INVALID_SOCKET)
		{
			printf("accept() failed with error %d\n", WSAGetLastError());
			return NULL;
		}
		else
		{
			int iResult = ioctlsocket(acceptSocket, FIONBIO, &NonBlock);
			if (iResult == SOCKET_ERROR)
			{
				printf("ioctsocket() failed with error %d\n", WSAGetLastError());
			}
			else
			{
				ClientInfo* info = new ClientInfo();
				info->socket = acceptSocket;
				info->bytesRECV = 0;
				ClientArray[TotalClients] = info;
				TotalClients++;
				printf("New client connected on socket %d\n", (int)acceptSocket);
			}
		}
	}

	// #5 recv 
	for (int i = 0; i < TotalClients; i++)
	{
		ClientInfo* client = ClientArray[i];

		// If the ReadSet is marked for this socket, then this means data
		// is available to be read on the socket
		if (FD_ISSET(client->socket, &ReadSet))
		{
			total--;
			client->dataBuf.buf = client->buffer;
			client->dataBuf.len = DEFAULT_BUFLEN;

			DWORD Flags = 0;
			int iResult = WSARecv(
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
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					// We can ignore this, it isn't an actual error.
				}
				else
				{
					printf("WSARecv failed on socket %d with error: %d\n", (int)client->socket, WSAGetLastError());
					RemoveClient(i);
				}
			}
			else
			{
				printf("WSARecv() is OK!\n");
				if (RecvBytes == 0)
				{
					RemoveClient(i);
				}
				else if (RecvBytes == SOCKET_ERROR)
				{
					printf("recv: There was an error..%d\n", WSAGetLastError());
					continue;
				}
				else
				{
					std::string recv_message;

					for (int b = 0; b < RecvBytes; b++) {
						recv_message += client->dataBuf.buf[b];
					}

					processMessage(readAuthMessage(recv_message));
				}
			}
		}
	}
	return NULL;
}

void AuthServer::processMessage(google::protobuf::Message* recievedMessage) {
	if (recievedMessage) {

		std::string messageType = recievedMessage->GetTypeName();

		// Dile que OK a todo
		if (messageType == "auth_protocol.Request") {
			auth_protocol::Request* message = (auth_protocol::Request*)recievedMessage;

			printf("Recibi %d para %s\n", message->action(), message->username().c_str());

			sendMessage(writeLoginOK(message->username()));
		}
	}
}

bool AuthServer::sendMessage(std::string serializedMessage)
{
	DWORD nBytes = serializedMessage.size();

	WSABUF buffer;
	buffer.buf = (char*)serializedMessage.c_str();
	buffer.len = serializedMessage.size();

	int iSendResult = WSASend(
		acceptSocket,
		&buffer,
		1,
		&nBytes,
		Flags,
		NULL,
		NULL
	); 
	// send(acceptSocket, serializedMessage.c_str(), serializedMessage.size(), 0);
	if (iSendResult == SOCKET_ERROR)
	{
		printf("send to Chat Server failed with error: %d\n", WSAGetLastError());
		closeServer();
		return false;
	}

	return true;
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

	is_on = false;
}
