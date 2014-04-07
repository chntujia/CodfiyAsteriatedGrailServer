#include "stdafx.h"
#include "GameManager.h"
#include "UserSessionManager.h"
#include "GameGrail.h"
#include <boost/thread.hpp>
using namespace boost;



GameManager::GameManager()
{
	initRoundSeed();
}

void GameManager::initRoundSeed()
{
	// TODO : get the last round seed from db
	m_roundSeed = 1;
}

int GameManager::createNewRoundId()
{
	return m_roundSeed ++;
}

int GameManager::createGame(int gameType, GameConfig *config)
{
	boost::mutex::scoped_lock lock(m_mutex_for_create);
	
	switch(gameType)
	{
		case GAME_TYPE_GRAIL:
			{
				int tableId = m_gameGrailMap.size();
				config->setTableId(tableId);
				GameGrail *game = new GameGrail((GameGrailConfig*)config);
				m_gameGrailMap.insert(GameMapType::value_type(tableId, game));
				thread(gameThread, game);
				break;
			}
			
		default:
			break;
	}
	return 0;
}

int GameManager::deleteGame(int gameType, int tableId)
{
	boost::mutex::scoped_lock lock(m_mutex_for_create);
	switch(gameType)
	{
		case GAME_TYPE_GRAIL:
			{
				GameGrail *game = (GameGrail*)m_gameGrailMap[tableId];
				m_gameGrailMap.erase(tableId);
				ztLoggerWrite(ZONE, e_Information, "GameManager::deleteGame() Table %d.", tableId);
				break;
			}
			
		default:
			break;
	}
	return 0;
}

int GameManager::enterRoom(int gameType, string userId, void* req)
{
	boost::mutex::scoped_lock lock(m_mutex_for_enter);		
	
	EnterRoomRequest* request = (EnterRoomRequest*)req;
	int ret;
	int roomId = request->room_id();

	GameMapType::iterator iter = m_gameGrailMap.find(roomId);
	if(iter == m_gameGrailMap.end())
	{
		ret = SIT_TABLE_NO_TABLE;
	}
	else
	{
		GameGrail *game = (GameGrail*)(iter->second);
		int playerId;
		ret = game->playerEnterIntoTable(userId, playerId);
		if (ret == SIT_TABLE_SUCCESS || ret == SIT_TABLE_GUEST){
			UserTask* session = UserSessionManager::getInstance().getUser(userId);
			GameGrail* lastGame = NULL;
			if(session){
				lastGame = session->getGame();
				session->setTableID(roomId);
				session->setPlayerID(playerId);
			}
			if(lastGame){
				lastGame->onUserLeave(userId);
			}
			if(ret == SIT_TABLE_SUCCESS)
				game->onPlayerEnter(playerId);
			else if(ret == SIT_TABLE_GUEST){
				game->onGuestEnter(userId);
			}
			
		}
		else{
			ztLoggerWrite(ZONE, e_Error, "GameManager::enterRoom() userId [%s] cannot enter Table %d. Ret: %d", 
				userId.c_str(), request->room_id(), ret);
		}		
	}
	return ret;
}

int GameManager::getGame(int gameType, int tableId, Game** game)
{
	if(tableId == -1)
		return -1;
	GameMapType::iterator iter;
	switch(gameType)
	{
		case GAME_TYPE_GRAIL:
			iter = m_gameGrailMap.find(tableId);
			if(iter == m_gameGrailMap.end())
			{
				return -1;
			}
			else
			{
				*game = iter->second;
			}
			break;
			
		default:
			return -1;
	}
	return 0;
}

int GameManager::getGameList(int gameType, void* req, void* res)
{
	int role_strategy = ((RoomListRequest*)req)->role_strategy();
	switch(gameType)
	{
		case GAME_TYPE_GRAIL:
			for(GameMapType::iterator iter = m_gameGrailMap.begin(); iter != m_gameGrailMap.end(); iter++)
			{
				RoomListResponse *response = (RoomListResponse*)res;
				RoomListResponse_RoomInfo *room = response->add_rooms();
				GameGrail* game = (GameGrail*)iter->second;
				room->set_room_id(iter->first);
				room->set_room_name(game->getGameName());
				room->set_max_player(game->getGameMaxPlayers());
				room->set_now_player(game->getGameNowPlayers());
				room->set_role_strategy((ROLE_STRATEGY)game->m_roleStrategy);
			}
			break;
			
		default:
			return -1;
	}
	return 0;
}

int GameManager::setPlayerReady(int gameType, int roomId, int playerId, void* req)
{
	ReadyForGameRequest* request = (ReadyForGameRequest*)req;
	GameMapType::iterator iter;	
	iter = m_gameGrailMap.find(roomId);
	if(iter != m_gameGrailMap.end())
	{
		GameGrail *game = (GameGrail*)(iter->second);
		if(game->topGameState()->state == STATE_WAIT_FOR_ENTER){
			if(request->type() == ReadyForGameRequest_Type_START_READY)
				game->setStartReady(playerId, true);
			else if(request->type() == ReadyForGameRequest_Type_CANCEL_START_REDAY)
				game->setStartReady(playerId, false);
			return 0;
		}
	}
	return -1;
}

void gameThread(Game* game)
{
	game->exec();
}
