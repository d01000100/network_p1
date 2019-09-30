
#include "Message.h"

std::string messageTypeString(MessageType t) {
	switch (t) {
	case LOGIN:
		return "LOGIN";
		break;
	case JOIN:
		return "JOIN";
		break;
	case LEAVE:
		return "LEAVE";
		break;
	case SEND:
		return "SEND";
		break;
	case RECIEVE:
		return "RECIEVE";
		break;
	case ERROR:
		return "ERROR";
		break;
	}
}

LoginMessage::LoginMessage() {
	type = LOGIN;
}

JoinMessage::JoinMessage() {
	type = JOIN;
}

LeaveMessage::LeaveMessage() {
	type = LEAVE;
}

SendMessage::SendMessage() {
	type = SEND;
}

RecieveMessage::RecieveMessage() {
	type = RECIEVE;
}