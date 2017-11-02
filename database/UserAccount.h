#pragma once
#include <string>

enum ACCOUNT_STATUS{
	STATUS_NORMAL = 0,
	STATUS_LOGIN_FAILED = 1,
	STATUS_FORBIDDEN = 2,
	STATUS_OUTDATE = 3,
	STATUS_VIP = 4,
	STATUS_ADMIN = 5,
	STATUS_GUEST = 6
};

struct UserAccount{
	int userId;
	std::string username;
	std::string nickname;
	ACCOUNT_STATUS status;
};
