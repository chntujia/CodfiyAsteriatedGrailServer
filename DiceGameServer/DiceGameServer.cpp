// DiceGameServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "DiceGameServer.h"
#include "zLogger.h"
#include "MessageQueue.h"
#include "Config.h"
#include "zTCPServer.h"
#include "GameManager.h"
#include "GameGrailCommon.h"
#include <fstream>
DiceGameServer::~DiceGameServer()
{
	ztLoggerWrite(ZONE, e_Information, "DiceGameServer is stopped!");
	UserSessionManager::delInstance();
	ServerConfig::delInstance();
}

bool DiceGameServer::serverInit()
{
	ServerConfig::newInstance();
	if (ServerConfig::getInstance().Load() != 0)
	{
		return false;
	}

	//设置日志级别
	ELogLevel level = getDebugLevel(ServerConfig::getInstance().m_strDebugLevel.c_str());
	//ztLoggerInit("ChatServer.log",level);
	ztLoggerInit(NULL,level);

	std::string strIP = ServerConfig::getInstance().m_strIP;
	uint16_t usPort = ServerConfig::getInstance().m_sPort;

	zNetService::init(usPort);

	UserSessionManager::newInstance();
	GameManager::newInstance();
    grailInit();

			GameGrailConfig*	config = new GameGrailConfig(2,0);
			GameManager::getInstance().createGame(GAME_TYPE_GRAIL, config);
	// create all the game table here
	//loadAllGameTable();
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
}