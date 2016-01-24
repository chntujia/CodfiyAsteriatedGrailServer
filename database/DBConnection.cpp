#include "DBConnection.h"
#include <mysql_driver.h>
#include <cppconn/exception.h>
#include <iostream>
using namespace std;
DBConnection::DBConnection(std::string hostname, std::string username, std::string password)
{
	try{
		mysql::MySQL_Driver* driver = mysql::get_mysql_driver_instance();
		con = driver->connect(hostname, username, password);
		con->setSchema("codify");
	}catch(sql::SQLException &e){
		cout << "# SQL ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}

DBConnection::~DBConnection()
{
	delete con;
}

PreparedStatement* DBConnection::prepare(char* preparedStatement)
{
	return con->prepareStatement(preparedStatement);
}

void DBConnection::executeUpdate(PreparedStatement* preparedStatement)
{
	preparedStatement->executeUpdate();
	delete preparedStatement;
}

ResultSet* DBConnection::executeQuery(PreparedStatement* preparedStatement)
{
	ResultSet* res = preparedStatement->executeQuery();
	delete preparedStatement;
	return res;
}

