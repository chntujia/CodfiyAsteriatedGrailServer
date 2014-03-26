#ifndef __PLAYER_CONTEXT_H__
#define __PLAYER_CONTEXT_H__

#include <iostream>

class PlayerContext
{
protected:
	std::string m_userId;
	int m_chips;
	bool m_isConnected;
	int m_tableId;

public:
	PlayerContext() : m_isConnected(true), m_tableId(-1){}
	PlayerContext(std::string userId);
	~PlayerContext() {}

public:
	std::string getUserId() const { return m_userId; }
	bool isConnected() const { return m_isConnected; }
	void setConnect(bool value) { m_isConnected = value; }
	void setTableId(int value) { m_tableId = value; }
	int getTableId() const { return m_tableId; }
	int getChips() const { return m_chips; }
	void changeChips(int value) { m_chips += value; }
};

#endif
