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

	//// Medium example: Retrieve the employees table
	//// SELECT * FROM employees;
	//try
	//{
	//	// #1 Prepare our statement
	//	pstmt = con->prepareStatement("select * from user,web_auth where (user.id = web_auth.userId);");
	//	// id last_login creation_date id email salt hashed_password userId
	//	// #2 Execute our statement
	//	rs = pstmt->executeQuery();

	//	// #3 Show the result
	//	if (rs->rowsCount() > 0)
	//	{
	//		printf("%d rows returned!\n", (int)rs->rowsCount());

	//		printf("Employee List:\n");
	//		while (rs->next())
	//		{
	//			std::string email = rs->getString(5);
	//			std::string hashed_password = rs->getString(7);

	//			std::cout << "email: " << email << std::endl;
	//			std::cout << "hashed_password: " << hashed_password << std::endl;
	//		}
	//	}
	//	else
	//	{
	//		printf("No rows returned!\n");
	//	}
	//}
	//catch (sql::SQLException & exception)
	//{
	//	std::cout << "# ERR: SQLException in " << __FILE__;
	//	std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
	//	std::cout << "# ERR: " << exception.what();
	//	std::cout << " (MySQL error code: " << exception.getErrorCode();
	//	std::cout << ", SQLState: " << exception.getSQLState() << ")" << std::endl;
	//	system("Pause");
	//	return 1;
	//}

	std::string user = "robin";
	std::string password = "password";
	std::string result = "";

	//result = createUser("robin", "password");
	result = authenticateUser(user, password);

	std::cout << result << std::endl;

	//system("Pause");
	return 0;
}
