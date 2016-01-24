// DiceGameServer.cpp : 定义控制台应用程序的入口点。
//
#include <fstream>
#include "stdafx.h"
#include "DiceGameServer.h"
#include "zLogger.h"
#include "MessageQueue.h"
#include "Config.h"
#include "zTCPServer.h"
#include "GameManager.h"
#include "GameGrailCommon.h"
#include "DBServices.h"
DiceGameServer::~DiceGameServer()
{
	ztLoggerWrite(ZONE, e_Information, "DiceGameServer is stopped!");
	UserSessionManager::delInstance();
	ServerConfig::delInstance();
	DBServices::delInstance();
}

bool DiceGameServer::serverInit()
{
	ServerConfig::newInstance();
	DBServices::newInstance();
	if (ServerConfig::getInstance().Load() != 0)
	{
		return false;
	}

	//设置日志级别
	ELogLevel level = getDebugLevel(ServerConfig::getInstance().m_strDebugLevel.c_str());
	//ztLoggerInit("ChatServer.log",level);
	char buffer[100];
	sprintf(buffer, "%d.txt", time(NULL));
	CZTLoger::Instance().SetFile(buffer,level);
	
	uint16_t usPort = ServerConfig::getInstance().m_sPort;
	std::string dbHostname = ServerConfig::getInstance().m_db_hostname;
	std::string dbUsername = ServerConfig::getInstance().m_db_username;
	std::string dbPassword = ServerConfig::getInstance().m_db_password;

	DBInstance.init(dbHostname, dbUsername, dbPassword);
	zNetService::init(usPort);

	UserSessionManager::newInstance();
	GameManager::newInstance();
    grailInit();
	return true;
}

void DiceGameServer::reload()
{
	ztLoggerWrite(ZONE, e_Information, "DiceGameServer Reload Config ");
	ServerConfig::getInstance().Reload();
	ELogLevel level = getDebugLevel(ServerConfig::getInstance().m_strDebugLevel.c_str());
	ztLoggerSetLevel(level);
}

bool DiceGameServer::grailInit()
{
	ifstream cardDB("cardDB.txt");
	if (!cardDB.is_open()){
		ztLoggerWrite(ZONE, e_Error, "Cannot Open cardDB.txt");
		return 1;
	}
	string line;
	int i = 0;
	while (i<CARDSUM)
    {
		getline(cardDB,line);
		cardList[i++] = new CardEntity(line);
	}
	cardDB.close();
}