// class GameManager
//
#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include <map>
#include "Game.h"
#include "zMisc.h"
#include "UserTask.h"


#define GAME_TYPE_LOBBY		9999
#define GAME_TYPE_SICBO		1000
#define GAME_TYPE_GRAIL     2000
#define SIT_TABLE_SUCCESS      0
#define SIT_TABLE_NO_TABLE     1
#define SIT_TABLE_FULL         2

class UserTask;

class GameManager : public SingletonBase<GameManager>
{
protected:
	int m_roundSeed;
	typedef map<int, Game*> GameMapType;
	GameMapType m_gameGrailMap;

public:
	GameManager();
	~GameManager() {}

protected:
	void initRoundSeed();

public:
	int createNewRoundId();
	int createGame(int gameType, GameConfig *config);
	int sitIntoTable(string userId, int gameType, int tableId);
	int getGame(int gameType, int tableId, Game **game);
};

void gameThread(Game* game);

#endif