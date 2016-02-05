#include "stdafx.h"
#include "GameManager.h"
#include "UserSessionManager.h"
#include <boost/thread.hpp>

using namespace boost;

GameManager::GameManager()
{
	m_next_roomId = 0;
}

int GameManager::createGame(CreateRoomRequest *req)
{	
	int tableId = m_next_roomId++;
	GameGrailConfig *config = new GameGrailConfig(req->max_player(), 
		                                          req->role_strategy(),
												  req->first_extension(),
												  req->second_extension(),
												  req->seat_mode(),
												  req->silence());
	config->setTableName(req->room_name());
	config->setTableId(tableId);
	GameGrail *game = new GameGrail(config);
	m_gameGrailMap.insert(GameMapType::value_type(tableId, game));
	GameContext context;
	context.allowGuest = req->allow_guest();
	context.password = req->password();
	m_gameGrailContextMap.insert(GameContextMapType::value_type(tableId, context));
	thread(gameThread, game);
	delete config;
	return tableId;
}

void GameManager::Check()
{
	for(GameMapType::iterator iter = m_gameGrailMap.begin(); iter != m_gameGrailMap.end(); )
	{
		if(iter->second->dead)
		{
			ztLoggerWrite(ZONE, e_Information, "GameManager::Check: delete game[%d].", iter->first);
			SAFE_DELETE(iter->second);
			m_gameGrailMap.erase(iter++);
			m_gameGrailContextMap.erase(iter->first);
		}
		else{
			iter++;
		}
	}
}

int GameManager::tryEnterRoom(string userId, string nickname, EnterRoomRequest* req, int& playerId, UserType identity)
{
	int roomId = req->room_id();
	GameMapType::iterator iter1 = m_gameGrailMap.find(roomId);
	if(iter1 == m_gameGrailMap.end())
	{
		return GE_INVALID_TABLEID;
	}
	if(!iter1->second->playing)
	{
		GameContextMapType::iterator iter2 = m_gameGrailContextMap.find(roomId);
		if(iter2 != m_gameGrailContextMap.end())
		{
			GameContext context = iter2->second;
			if(!context.allowGuest && identity <= UT_GUEST)
				return GE_NOT_WELCOME;
			if(!context.password.empty() && req->password() != context.password)
				return GE_WRONG_PASSWORD;
		}
	}
	return enterRoom(userId, nickname, req, playerId);
}

int GameManager::enterRoom(string userId, string nickname, EnterRoomRequest* req, int& playerId)
{	
	EnterRoomRequest* request = (EnterRoomRequest*)req;
	int ret;
	int roomId = request->room_id();

	GameMapType::iterator iter = m_gameGrailMap.find(roomId);
	if(iter == m_gameGrailMap.end())
	{
		return GE_INVALID_TABLEID;
	}

	GameGrail *game = iter->second;

	ret = game->playerEnterIntoTable(userId, nickname, playerId);
	if (ret == GE_SUCCESS){
		game->onPlayerEnter(playerId);	
		return GE_SUCCESS;
	}

	ret = game->guestEnterIntoTable(userId);
	if (ret == GE_SUCCESS){
		playerId = GUEST;
		game->onGuestEnter(userId);
		return GE_SUCCESS;
	}

	ztLoggerWrite(ZONE, e_Error, "GameManager::enterRoom() userId [%s] cannot enter Table %d. Ret: %d", 
		userId.c_str(), request->room_id(), ret);

	return ret;
}

int GameManager::getGame(int tableId, GameGrail** game)
{
	if(tableId < 0)
		return -1;
	GameMapType::iterator iter;

	iter = m_gameGrailMap.find(tableId);
	if(iter == m_gameGrailMap.end())
	{
		return -1;
	}
	else
	{
		*game = iter->second;
	}

	return 0;
}

int GameManager::getGameList(RoomListRequest* req, RoomListResponse* res)
{
	int role_strategy = req->role_strategy();

	for(GameMapType::iterator iter1 = m_gameGrailMap.begin(); iter1 != m_gameGrailMap.end(); iter1++)
	{
		RoomListResponse *response = (RoomListResponse*)res;
		RoomListResponse_RoomInfo *room = response->add_rooms();
		int roomId = iter1->first;
		GameGrail* game = iter1->second;
		room->set_room_id(roomId);
		room->set_room_name(game->getGameName());
		room->set_max_player(game->getGameMaxPlayers());
		room->set_now_player(game->getGameNowPlayers());
		room->set_role_strategy((ROLE_STRATEGY)game->m_roleStrategy);
		room->set_seat_mode((SEAT_MODE)game->m_seatMode);
		room->set_first_extension(game->m_firstExtension);
		room->set_second_extension(game->m_secondExtension);
		GameContextMapType::iterator iter2 = m_gameGrailContextMap.find(roomId);
		if(iter2 != m_gameGrailContextMap.end())
		{
			GameContext context = iter2->second;
			room->set_allow_guest(context.allowGuest);
			room->set_has_password(!context.password.empty());
		}
		room->set_silence(game->m_silence);
		room->set_playing(game->playing);
			
	}

	return 0;
}

int GameManager::setPlayerReady(int roomId, int playerId, ReadyForGameRequest* req)
{
	if(playerId == GUEST)
		return 0;
	GameMapType::iterator iter;	
	iter = m_gameGrailMap.find(roomId);
	if(iter != m_gameGrailMap.end())
	{
		GameGrail *game = iter->second;
		if(game->topGameState()->state == STATE_WAIT_FOR_ENTER){

			if(req->type() == ReadyForGameRequest_Type_START_READY){
				game->setStartReady(playerId, true);
			}
			else if(req->type() == ReadyForGameRequest_Type_CANCEL_START_REDAY){
				game->setStartReady(playerId, false);
			}
			return 0;
		}
	}
	return -1;
}

void gameThread(Game* game)
{
	game->exec();
}
