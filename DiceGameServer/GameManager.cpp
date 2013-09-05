#include "stdafx.h"
#include "GameManager.h"
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
	GameGrail *game;
	switch(gameType)
	{
		case GAME_TYPE_GRAIL:
			config->setTableId(m_gameGrailMap.size());
			game = new GameGrail((GameGrailConfig*)config);
			m_gameGrailMap.insert(GameMapType::value_type(config->getTableId(), game));
			thread(gameThread, game);
			break;
			
		default:
			break;
	}
	return 0;
}

int GameManager::sitIntoTable(string userId, int gameType, int tableId)
{
	GameMapType::iterator iter;	
	int sitRes;
	GameGrail *game;
	switch(gameType)
	{
		case GAME_TYPE_GRAIL:
			iter = m_gameGrailMap.find(tableId);
			if(iter == m_gameGrailMap.end())
			{
				sitRes = SIT_TABLE_NO_TABLE;
			}
			else
			{
				game = (GameGrail*)(iter->second);
				if(game->isCanSitIntoTable())
				{
					GameGrailPlayerContext *player = new GameGrailPlayerContext(userId);
					int result = game->playerEnterIntoTable(player);
					switch(result)
					{
						case 0:
							sitRes = SIT_TABLE_SUCCESS;
							break;
						case 1:
							sitRes = SIT_TABLE_FULL;
							break;
						default:
							break;
					}
				}
				else
				{
					sitRes = SIT_TABLE_FULL;
				}
			}
			break;
		default:
			sitRes = SIT_TABLE_NO_TABLE;
			break;
	}
	return sitRes;
}

int GameManager::getGame(int gameType, int tableId, Game** game)
{
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

void gameThread(Game* game)
{
	game->exec();
}
