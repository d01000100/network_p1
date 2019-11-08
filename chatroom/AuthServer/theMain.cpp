#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <ctime> 
#include <time.h>

#include "AuthServer.h"
#include "SQLFunctions.h"

#define ESCAPE 27
#define RETURN 8
#define DEFAULT_BUFLEN 512

std::string server = "127.0.0.1:3306";
std::string username = "root";
std::string password = "My51l4dm1n";
std::string schema = "casado_zuniga_project2";

bool _should_close = false;

AuthServer authServer;

sql::Driver* driver;
sql::Connection* con;
sql::Statement* stmt;
sql::PreparedStatement* pstmt;
sql::ResultSet* rs;
SHA256_CTX ctx;

void key_listen() {
	if (_kbhit())
	{
		char ch = _getch();
		printf("%d\n", ch);
		if (ch == ESCAPE)
		{
			authServer.is_on = false;
		}
	}
}

int main(int argc, char** argv)
{
	SHA256_Init(&ctx);

	try
	{
		driver = get_driver_instance();
		con = driver->connect(server, username, password);
		printf("Successfully connected to our database!\n");

		con->setSchema(schema);
		printf("Successfully set our schema!\n");
	}
	catch (sql::SQLException & exception)
	{
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << exception.what();
		std::cout << " (MySQL error code: " << exception.getErrorCode();
		std::cout << ", SQLState: " << exception.getSQLState() << ")" << std::endl;
		system("Pause");
		return 1;
	}

	if (!authServer.init()) { return 1; }

	while (authServer.is_on) {

		key_listen();

		google::protobuf::Message* recievedMessage = authServer.recieveMessage();

		if (recievedMessage) {

			std::string messageType = recievedMessage->GetTypeName();

			// Dile que OK a todo
			if (messageType == "auth_protocol.Request") {
				auth_protocol::Request* message = (auth_protocol::Request*)recievedMessage;

				auth_protocol::ResponseOK ok;
				ok.set_action(message->action());
				ok.set_username(message->username());

				printf("Recibi %d para %s\n", message->action(), message->username().c_str());

				authServer.sendMessage(ok.SerializeAsString());
			}
		}
	}

	authServer.closeServer();

	return 0;
}
