#pragma once
#include "DBConnection.h"

class BaseDAO
{
public:
	BaseDAO(DBConnection* conn): connection(conn) {}

protected:
	DBConnection* connection;
};

