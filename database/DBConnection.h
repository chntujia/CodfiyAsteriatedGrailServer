#pragma once

#include<stdio.h>
#include <WinSock.h>
#include "mysql.h"
#include <mysql_connection.h>
#include <prepared_statement.h>
#include <cppconn/exception.h>
#include <string>

class DBConnection
{
public:
	DBConnection(std::string hostname, std::string username, std::string password);
	DBConnection(const DBConnection&);
	~DBConnection();
	DBConnection clone();
	sql::PreparedStatement* prepare(char* preparedStatement);
	void executeUpdate(sql::PreparedStatement* preparedStatement);
	sql::ResultSet* executeQuery(sql::PreparedStatement* preparedStatement);
private:
	void connect();
	void logSQLException(sql::SQLException &e);
	sql::Connection *con;
	std::string hostname;
	std::string username;
	std::string password;
};

