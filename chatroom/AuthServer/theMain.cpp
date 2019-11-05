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

int main(int argc, char** argv)
{
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

	CreateUser("robin", "robin");

	system("Pause");
	return 0;
}
