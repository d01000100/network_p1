#include "ProtocolManager.h"

SendBuffer writeMessage(LoginMessage* message) {
	//      int            int       int      string
	//[packet_length][message_type][length][client_name]
	unsigned int name_length = message->client_name.length();
	unsigned int packet_length = 3 * sizeof(int) + name_length;
	
	SendBuffer buffer;
	buffer.writeInt(packet_length);
	buffer.writeInt(LOGIN);
	buffer.writeInt(name_length);
	buffer.writeString(message->client_name);
	return buffer;
}

SendBuffer writeMessage(JoinMessage* message) {
	//      int            int       int     string
	//[packet_length][message_type][length][room_name]
	unsigned int name_length = message->room_name.length();
	unsigned int packet_length = 3 * sizeof(int) + name_length;

	SendBuffer buffer;
	buffer.writeInt(packet_length);
	buffer.writeInt(JOIN);
	buffer.writeInt(name_length);
	buffer.writeString(message->room_name);
	return buffer;
}

SendBuffer writeMessage(LeaveMessage* message) {
	//      int            int       int     string
	//[packet_length][message_type][length][room_name]
	unsigned int name_length = message->room_name.length();
	unsigned int packet_length = 3 * sizeof(int) + name_length;

	SendBuffer buffer;
	buffer.writeInt(packet_length);
	buffer.writeInt(LEAVE);
	buffer.writeInt(name_length);
	buffer.writeString(message->room_name);
	return buffer;
}

SendBuffer writeMessage(SendMessage* message) {
	//      int            int       int      string      int     string
	//[packet_length][message_type][length] [room_name] [length] [message]
	unsigned int room_length = message->room_name.length();
	unsigned int message_length = message->message.length();
	unsigned int packet_length = 4 * sizeof(int) + room_length + message_length;

	SendBuffer buffer;
	buffer.writeInt(packet_length);
	buffer.writeInt(SEND);
	buffer.writeInt(room_length);
	buffer.writeString(message->room_name);
	buffer.writeInt(message_length);
	buffer.writeString(message->message);
	return buffer;
}

SendBuffer writeMessage(RecieveMessage* message) {
	//      int            int       int        string       int     string      int    string
	//[packet_length][message_type][length] [sender_name] [length] [room_name] [length] [message]
	unsigned int room_length = message->room_name.length();
	unsigned int message_length = message->message.length();
	unsigned int sender_length = message->sender_name.length();
	unsigned int packet_length = 5 * sizeof(int) + room_length
		+ message_length + sender_length;

	SendBuffer buffer;
	buffer.writeInt(packet_length);
	buffer.writeInt(RECIEVE);
	buffer.writeInt(sender_length);
	buffer.writeString(message->sender_name);
	buffer.writeInt(room_length);
	buffer.writeString(message->room_name);
	buffer.writeInt(message_length);
	buffer.writeString(message->message);
	return buffer;
}

Message* readMessage(RecieveBuffer buffer) {
	// ====== Header ========
	//     int             int
	//[packet_length][message_type]
	int packet_length = buffer.readInt();
	int message_type = buffer.readInt();

	switch (message_type) {
		case LOGIN: {
			//  int      string
			//[length][client_name]
			LoginMessage* loginM = new LoginMessage();
			int client_length = buffer.readInt();
			loginM->client_name = buffer.readString(client_length);
			return loginM;
			}
		case JOIN: {
			//  int     string
			//[length][room_name]
			JoinMessage* joinM = new JoinMessage();
			int room_length = buffer.readInt();
			joinM->room_name = buffer.readString(room_length);
			return joinM;
			break;
		}
		case LEAVE: {
			//  int     string
			//[length][room_name]
			LeaveMessage* leaveM = new LeaveMessage();
			int room_length = buffer.readInt();
			leaveM->room_name = buffer.readString(room_length);
			return leaveM;
			break;
		}
		case SEND: {
			//  int      string      int     string
			//[length] [room_name] [length] [message]
			SendMessage* sendM = new SendMessage();
			int room_length = buffer.readInt();
			sendM->room_name = buffer.readString(room_length);
			int message_length = buffer.readInt();
			sendM->message = buffer.readString(message_length);
			return sendM;
			break;
		}
		case RECIEVE: {
			//   int        string       int     string      int    string
			//[length] [sender_name] [length] [room_name] [length] [message]
			RecieveMessage* recieveM = new RecieveMessage();
			int name_length = buffer.readInt();
			recieveM->sender_name = buffer.readString(name_length);
			int room_length = buffer.readInt();
			recieveM->room_name = buffer.readString(room_length);
			int message_length = buffer.readInt();
			recieveM->message = buffer.readString(message_length);
			return recieveM;
			break;
		}
		default: {
			Message* errorM = new Message();
			errorM->type = ERROR;
			return errorM;
			break;
		}
	}
}