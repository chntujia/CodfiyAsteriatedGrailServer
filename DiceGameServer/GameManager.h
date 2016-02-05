// class GameManager
//
#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include <map>
#include "GameGrail.h"
#include "zMisc.h"
#include "UserTask.h"
#include "action_respond.pb.h"
#include "base.pb.h"

class UserTask;

typedef struct{

	bool allowGuest;
	string password;
}GameContext;

class GameManager : public SingletonBase<GameManager>
{
protected:
	typedef map<int, GameGrail*> GameMapType;
	GameMapType m_gameGrailMap;
	typedef map<int, GameContext> GameContextMapType;
	GameContextMapType m_gameGrailContextMap;

public:
	GameManager();
	~GameManager() {}
	void Check();

	int createGame(CreateRoomRequest *req);
	int enterRoom(string userId, string nickname, EnterRoomRequest* req, int& playerId);
	int tryEnterRoom(string userId, string nickname, EnterRoomRequest* req, int& playerId, UserType identity);
	int getGame(int tableId, GameGrail **game);
	int getGameList(RoomListRequest* req, RoomListResponse* res);
	int setPlayerReady(int roomId, int playerId, ReadyForGameRequest* req);

private:
	void leaveLastRoom(string userId, int newTableId);
	int m_next_roomId;
};

void gameThread(Game* game);

#endif