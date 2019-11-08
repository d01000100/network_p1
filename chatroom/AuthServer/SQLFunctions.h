#pragma once

#include <string>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <openssl\conf.h>
#include <openssl\evp.h>
#include <openssl\err.h>
#include <openssl\sha.h>

extern sql::Driver* driver;
extern sql::Connection* con;
extern sql::Statement* stmt;
extern sql::PreparedStatement* pstmt;
extern sql::ResultSet* rs;
extern SHA256_CTX ctx;

std::string createUser(std::string email, std::string password);
std::string authenticateUser(std::string email, std::string password);