#include <mysql_driver.h>
#include "DBConnection.h"
#include "zMisc.h"
#include "zLogger.h"

using namespace sql;

DBConnection::DBConnection(std::string hostname, std::string username, std::string password)
{
	this->hostname = hostname;
	this->username = username;
	this->password = password;
	this->con = NULL;
	connect();
}

DBConnection::~DBConnection()
{
	ztLoggerWrite(ZONE, e_Information, "DBConnection::~DBConnection(): con: %p", con);
	SAFE_DELETE(con);
}

void DBConnection::connect()
{
	ztLoggerWrite(ZONE, e_Information, "DBConnection::connect()");
	SAFE_DELETE(con);
	try{
		mysql::MySQL_Driver* driver = mysql::get_mysql_driver_instance();
		con = driver->connect(hostname, username, password);
		con->setSchema("codify");
	}catch(sql::SQLException &e){
		logSQLException(e);
	}

}

PreparedStatement* DBConnection::prepare(char* preparedStatement)
{
	if(con->isClosed()){
		connect();
	}
	return con->prepareStatement(preparedStatement);
}

void DBConnection::executeUpdate(PreparedStatement* preparedStatement)
{
	try{
		preparedStatement->executeUpdate();
		SAFE_DELETE(preparedStatement);
	}catch(sql::SQLException &e){
		logSQLException(e);
		SAFE_DELETE(preparedStatement);
	}
}

ResultSet* DBConnection::executeQuery(PreparedStatement* preparedStatement)
{
	try{
		ResultSet* res = preparedStatement->executeQuery();
		SAFE_DELETE(preparedStatement);
		return res;
	}catch(sql::SQLException &e){
		logSQLException(e);
		SAFE_DELETE(preparedStatement);
	}
}

void DBConnection::logSQLException(sql::SQLException &e)
{
	ztLoggerWrite(ZONE, e_Error, "SQL Error: %s, (MySQL error code: %d,  SQLState: %s)", e.what(), e.getErrorCode(), e.getSQLState());
}

