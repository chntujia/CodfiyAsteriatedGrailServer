#pragma once
#include <string>

enum ACCOUNT_STATUS{
	STATUS_NORMAL = 0,
	STATUS_LOGIN_FAILED = 1,
	STATUS_FORBIDDEN = 2,
	STATUS_OUTDATE = 3
};

struct UserAccount{
	std::string username;
	std::string nickname;
	int status;
};
