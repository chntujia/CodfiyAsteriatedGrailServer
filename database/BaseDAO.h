#pragma once
#include "DBConnection.h"

class BaseDAO
{
public:
	BaseDAO(DBConnection* conn): connection(conn) {}
	~BaseDAO(){
		delete connection;
	}
protected:
	DBConnection* connection;
};

