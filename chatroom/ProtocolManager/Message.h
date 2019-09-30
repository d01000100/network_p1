#pragma once
#include <string>

enum MessageType {
	LOGIN, JOIN, LEAVE, SEND, RECIEVE, ERROR
};

std::string messageTypeString(MessageType t);

// It's a class because we need inheritance
// But it's basically a struct
class Message {
public:
	MessageType type;
};

class LoginMessage : public Message {
public:
	std::string client_name;
	LoginMessage();
};

class JoinMessage : public Message {
public:
	std::string room_name;
	JoinMessage();
};

class LeaveMessage : public Message {
public:
	std::string room_name;
	LeaveMessage();
};

class SendMessage : public Message {
public:
	std::string room_name;
	std::string message;
	SendMessage();
};

class RecieveMessage : public Message {
public:
	std::string sender_name;
	std::string room_name;
	std::string message;
	RecieveMessage();
};