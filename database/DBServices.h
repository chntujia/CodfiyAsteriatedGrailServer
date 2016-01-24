#pragma once
#include <string>
#include "zMisc.h"
#include "DBConnection.h"
#include "UserAccountDAO.h"

#define DBInstance 	DBServices::getInstance()

class DBServices: public SingletonBase<DBServices>{
public:
	void init(std::string hostname, std::string username, std::string password){
		connection = new DBConnection(hostname, username, password);
		userAccountDAO = new UserAccountDAO(connection);
	}
	~DBServices(){
		delete userAccountDAO;
		delete connection;
	}
	UserAccountDAO* userAccountDAO;
private:
	DBConnection* connection;
};