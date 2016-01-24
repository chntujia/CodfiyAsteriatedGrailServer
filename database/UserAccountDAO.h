#pragma once

#include "BaseDAO.h"
#include "UserAccount.h"

class UserAccountDAO: BaseDAO
{  
public:
	UserAccountDAO(DBConnection* conn): BaseDAO(conn){}
	void insert(std::string username, std::string password, std::string nickname, int status);
	struct UserAccount query(std::string username, std::string password);
};

