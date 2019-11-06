#pragma warning(disable:4996)

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <ctime> 
#include <time.h>

#include "AuthServer.h"

#define ESCAPE 27
#define RETURN 8
#define DEFAULT_BUFLEN 512

/*
$(SolutionDir)dev\include;
$(SolutionDir)dev\lib\$(Platform)\$(Configuration);
lib files, libcrypto.lib, libss.lib, mysqlcppconn.lib
*/

sql::Driver* driver;
sql::Connection* con;
sql::Statement* stmt;
sql::PreparedStatement* pstmt;
sql::ResultSet* rs;

std::string server = "127.0.0.1:3306";
std::string username = "davezn";
std::string password = "hola";
std::string schema = "casado_zuniga_project2";

bool _should_close = false;


void CreateUser(std::string email, std::string password)
{
	sql::PreparedStatement* addUser;
	int UID = 1000;
	struct tm* ptm;
	time_t now;
	time(&now);

	std::stringstream prep;
	prep << "INSERT INTO user (last_login, creation_date) VALUES (FROM_UNIXTIME(" << now << "), FROM_UNIXTIME(" << now << "));";
	addUser = con->prepareStatement(prep.str());
	try
	{
		int result = addUser->executeUpdate();
		printf("%d row(s) affected.\n", result);
	}
	catch (sql::SQLException & exception)
	{
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << exception.what();
		std::cout << " (MySQL error code: " << exception.getErrorCode();
		std::cout << ", SQLState: " << exception.getSQLState() << ")" << std::endl;
		system("Pause");
	}
	
	prep.str(std::string());
	prep << "select * from user where creation_date = FROM_UNIXTIME(" << now << ");";
	pstmt = con->prepareStatement(prep.str());
	rs = pstmt->executeQuery();
	if (rs->rowsCount() > 0)
	{
		while (rs->next())
		{
			UID = rs->getInt(1);
			std::cout << "UID: " << UID << std::endl;
		}
	}

	prep.str(std::string());
	prep << "INSERT INTO web_auth (email, salt, hashed_password, userId) VALUES ("<< email<<","<<");";
	// `id`,`email`, `salt`, `hashed_password`, `userId`
	addUser = con->prepareStatement(prep.str());
	try
	{
		int result = addUser->executeUpdate();
		printf("%d row(s) affected.\n", result);
	}
	catch (sql::SQLException & exception)
	{
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << exception.what();
		std::cout << " (MySQL error code: " << exception.getErrorCode();
		std::cout << ", SQLState: " << exception.getSQLState() << ")" << std::endl;
		system("Pause");
	}
}

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

int main(int argc, char** argv)
{
	AuthServer authServer;

	if (!authServer.init()) { return 1; }

	while (!_should_close) {

		char buffer[DEFAULT_BUFLEN];

		int iResult = authServer.recieveMessage(buffer);

		if (iResult > 0) {

			std::string recv_message;

			for (int i = 0; i < iResult; i++) {
				recv_message += buffer[i];
			}

			auth_protocol::Request* message = (auth_protocol::Request*)readAuthMessage(recv_message);

			if (message) {
				printf("recibi un %s\n", message->GetTypeName().c_str());

				printf("username: %s, password: %s\n", 
					message->username().c_str(),
					message->plaintextpassword().c_str());
			}

		}
		else if (iResult < 0) {
			_should_close = true;
		}
	}


	authServer.closeServer();

	return 0;
}
