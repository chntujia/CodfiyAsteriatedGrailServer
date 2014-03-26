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
#define SIT_TABLE_GUEST        3

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
	int deleteGame(int gameType, int tableId);
	int enterRoom(int gameType, string userId, void* req);
	int getGame(int gameType, int tableId, Game **game);
	int getGameList(int gameType, void* req, void* res);
	int setPlayerReady(int gameType, int roomId, int playerId, void* req);

private:
	boost::mutex m_mutex_for_enter;
	boost::mutex m_mutex_for_create;
};

void gameThread(Game* game);

#endif