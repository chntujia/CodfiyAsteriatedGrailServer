#ifndef __DICE_GAME_SERVER_H__
#define __DICE_GAME_SERVER_H__

#include "zMisc.h"
#include "zNetService.h"
#include "UserSessionManager.h"
#include "UserTask.h"
#include "boost/thread.hpp"
#include "GameGrailCommon.h"

class DiceGameServer : public zNetService, public SingletonBase<DiceGameServer>
{
public:
	friend class SingletonBase<DiceGameServer>;
	
	virtual zTCPTask* CreateTask(uint16_t usPort) 
	{
		return new UserTask(getIOSerivce());
	}

	virtual zTCPTask* newTCPTask(const uint16_t usPort)
	{
		return new UserTask(getIOSerivce());
	}

	virtual bool serviceCallback()
	{
		Check();
		zNetService::serviceCallback();
		return true;
	}

	virtual void Check()
	{
		UserSessionManager::getInstance().doCmd();
		// GMTaskManager::getInstance().doCmd();
	}

private:
	DiceGameServer():zNetService("DiceGameServer"){ }
	~DiceGameServer();
	bool grailInit();
	void loadAllGameTable();

public:
	bool serverInit();
	void reload();
};

#endif