#ifndef __GAME_H__
#define __GAME_H__

#include <iostream>
using namespace std;

class GameConfig
{
private:
	int m_tableId;
	string m_tableName;

public:
	
	GameConfig(int tableId=0)
	{
		m_tableId = tableId;
		m_tableName = "test table";
	}
	~GameConfig(){}
	void setTableId(int id) { m_tableId = id;} 
	void setTableName(string name){m_tableName = name;}
	int getTableId() const { return m_tableId; }
	string getTableName() const { return m_tableName; }
};

class Game
{
protected:
	int m_gameId;						// 游戏桌ID
	int m_gameState;					// 游戏当前状态
	int m_gameType;						// game type
	string m_gameName;					// game name

public:
	Game() {}
	Game(GameConfig *config)	
	{
		m_gameId = config->getTableId();
		m_gameName = config->getTableName();
	}
	~Game(){}

	void exec()
	{
		GameRun();
	}

	int getGameId() const { return m_gameId; }
	string getGameName() const { return m_gameName; }

protected:
	virtual void GameRun() = 0;			// 游戏主逻辑
	void run();
};

#endif