#pragma once

#include<stdio.h>
#include <WinSock.h>
#include "mysql.h"
#include <mysql_connection.h>
#include <prepared_statement.h>
#include <string>
using namespace sql;

class DBConnection
{
public:
	DBConnection(std::string hostname, std::string username, std::string password);
	~DBConnection();
	PreparedStatement* prepare(char* preparedStatement);
	void executeUpdate(PreparedStatement* preparedStatement);
	ResultSet* executeQuery(PreparedStatement* preparedStatement);
private:
	Connection *con;
};

