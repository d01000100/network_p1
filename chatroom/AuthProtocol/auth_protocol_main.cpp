#include "AuthProtocol.h"

// The user of the Protocol should do something like this
// to interpret the recieved Message
void printMessage(google::protobuf::Message* recievedMessage) {

	if (recievedMessage == NULL) { return; }

	std::string messageType = recievedMessage->GetTypeName();

	if (messageType == "auth_protocol.Request") {
		auth_protocol::Request *message = (auth_protocol::Request*)recievedMessage;

		switch (message->action()) {
		case auth_protocol::SIGN_UP:
			printf("Request to create a user with email %s and password %s\n",
				message->username().c_str(), message->plaintextpassword().c_str());
			break;
		case auth_protocol::LOGIN:
			printf("Request to login a user with email %s and password %s\n",
				message->username().c_str(), message->plaintextpassword().c_str());
			break;
		}
	}
	if (messageType == "auth_protocol.ResponseOK") {
		auth_protocol::ResponseOK *message = (auth_protocol::ResponseOK*)recievedMessage;

		switch (message->action()) {
		case auth_protocol::SIGN_UP:
			printf("User created with email %s\n",
				message->username().c_str());
			break;
		case auth_protocol::LOGIN:
			printf("User %s logged in\n",
				message->username().c_str());
			break;
		}
	}
	if (messageType == "auth_protocol.ResponseError") {
		auth_protocol::ResponseError *message = (auth_protocol::ResponseError*)recievedMessage;

		switch (message->action()) {
		case auth_protocol::SIGN_UP:
			printf("Request to create a user with email %s failed because %s\n",
				message->username().c_str(), message->error().c_str());
			break;
		case auth_protocol::LOGIN:
			printf("Request to login a user with email %s failed becasue %s\n",
				message->username().c_str(), message->error().c_str());
			break;
		}
	}
}

int main(int argc, char** argv) {

	google::protobuf::Message* recievedMessage;
	std::string serialized_message;

	// sign up
	 serialized_message = writeSignUpRequest("foo@bar.mx", "asdf");
	recievedMessage = readAuthMessage(serialized_message);
	printMessage(recievedMessage);

	// sign up ok
	 serialized_message = writeSignUpOK("foo@bar.mx");
	recievedMessage = readAuthMessage(serialized_message);
	printMessage(recievedMessage);

	// sign up error
	 serialized_message = writeSignUpError("foo@bar.mx", "Nombre de usuario ya existente");
	recievedMessage = readAuthMessage(serialized_message);
	printMessage(recievedMessage);

	// log in
	 serialized_message = writeLoginRequest("foo@bar.mx", "asdf");
	recievedMessage = readAuthMessage(serialized_message);
	printMessage(recievedMessage);

	// log in ok
	 serialized_message = writeLoginOK("foo@bar.mx");
	recievedMessage = readAuthMessage(serialized_message);
	printMessage(recievedMessage);

	// log in error
	 serialized_message = writeLoginError("foo@bar.mx", "Contrasenia incorrecta");
	recievedMessage = readAuthMessage(serialized_message);
	printMessage(recievedMessage);

	return 0;
}