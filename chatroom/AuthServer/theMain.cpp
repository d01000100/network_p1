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
#include <openssl\conf.h>
#include <openssl\evp.h>
#include <openssl\err.h>
#include <openssl\sha.h>

SHA256_CTX ctx;

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

AuthServer authServer;

bool deleteByCreationDateInTableUser(time_t creationDate)
{
	std::stringstream prep;
	prep.str(std::string());
	prep << "delete from user where creation_date = FROM_UNIXTIME(" << creationDate << ");";
	pstmt = con->prepareStatement(prep.str());
	try
	{
		int result = pstmt->executeUpdate();
		printf("%d row(s) affected.\n", result);
	}
	catch (sql::SQLException & exception)
	{
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << exception.what();
		std::cout << " (MySQL error code: " << exception.getErrorCode();
		std::cout << ", SQLState: " << exception.getSQLState() << ")" << std::endl;
		return false;
	}
}

std::string createSalt(int saltSize)
{
	std::string salt;  /* Salt for the crypt() function  */
	const char* salt_chars = "abcdefghijklmnopqrstuvwxyz" /* Range of character supported   */
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"  /* as a value for salt in crypt() */
		"0123456789";
	char password1[BUFSIZ], * buf;

	/* Build salt */
	srand(time(NULL));
	//salt[0] = salt_chars[rand() % 62];
	//salt[1] = salt_chars[rand() % 62];

	for (int i = 0; i < saltSize; i++)
	{
		salt += salt_chars[rand() % 62];
	}

	std::cout << "aqui la sal xdxd:  " << salt << std::endl;

	return salt;
}

std::string to_hex(unsigned char s) {
	std::stringstream ss;
	ss << std::hex << (int)s;
	return ss.str();
}

std::string sha256(std::string line) {
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, line.c_str(), line.length());
	SHA256_Final(hash, &sha256);

	std::string output = "";
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
		output += to_hex(hash[i]);
	}
	return output;
}

std::string createUser(std::string email, std::string password)
{
	sql::PreparedStatement* addUser;
	sql::PreparedStatement* addWebAuth;
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
		//printf("%d row(s) affected.\n", result);
	}
	catch (sql::SQLException & exception)
	{
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << exception.what();
		std::cout << " (MySQL error code: " << exception.getErrorCode();
		std::cout << ", SQLState: " << exception.getSQLState() << ")" << std::endl;
		//system("Pause");
		return exception.what();
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
			//std::cout << "UID: " << UID << std::endl;
		}
	}

	std::string salt = createSalt(password.size());
	std::string addedSalt = password + salt;
	std::string hash = sha256(addedSalt);
	//std::cout << "hash: " << hash << std::endl;

	prep.str(std::string());
	prep << "INSERT INTO web_auth (email, salt, hashed_password, userId) VALUES ('" 
		<< email << "','" << salt << "','" << hash << "','" << UID << "');";
	addWebAuth = con->prepareStatement(prep.str());
	try
	{
		int result = addWebAuth->executeUpdate();
		//printf("%d row(s) affected.\n", result);
	}
	catch (sql::SQLException & exception)
	{
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << exception.what();
		std::cout << " (MySQL error code: " << exception.getErrorCode();
		std::cout << ", SQLState: " << exception.getSQLState() << ")" << std::endl;
		//system("Pause");
		deleteByCreationDateInTableUser(now);
		return exception.what();
	}
	return ("succesful creation of user");
}

std::string authenticateUser(std::string email, std::string password)
{
	sql::PreparedStatement* addUser;
	sql::PreparedStatement* addWebAuth;
	std::stringstream prep;

	// web_auth thingies
	int webID = 1000;
	std::string userMail;
	std::string salt;
	std::string hashedPassword;
	int UID = 1000;
	
	prep.str(std::string());
	prep << "select * from web_auth where email = '" << email << "';";
	pstmt = con->prepareStatement(prep.str());
	rs = pstmt->executeQuery();
	if (rs->rowsCount() > 0)
	{
		while (rs->next())
		{
			webID = rs->getInt(1);
			userMail = rs->getString(2);
			salt = rs->getString(3);
			hashedPassword = rs->getString(4);
			UID = rs->getInt(5);
		}
	}
	else
	{
		return "ERROR: account not found";
	}

	std::string try1 = password + salt;
	std::string hash = sha256(try1);
	// std::cout << "hash: " << hash << std::endl;

	if (hashedPassword != hash)
	{
		return "Authentication failed, wrong password";
	}

	time_t now;
	time(&now);
	prep.str(std::string());
	prep << "UPDATE user SET last_login=FROM_UNIXTIME(" << now << ") WHERE id = "<<UID<<";";
	// "UPDATE MyGuests SET lastname='Doe' WHERE id=2";
	addUser = con->prepareStatement(prep.str());
	try
	{
		int result = addUser->executeUpdate();
		//printf("%d row(s) affected.\n", result);
	}
	catch (sql::SQLException & exception)
	{
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << exception.what();
		std::cout << " (MySQL error code: " << exception.getErrorCode();
		std::cout << ", SQLState: " << exception.getSQLState() << ")" << std::endl;
	}

	return "Succesful login";
}

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
