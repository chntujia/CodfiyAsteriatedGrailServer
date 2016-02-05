#ifndef __PLAYER_CONTEXT_H__
#define __PLAYER_CONTEXT_H__

#include <iostream>

class PlayerContext
{
protected:
	std::string m_userId;
	bool m_isConnected;
	int m_tableId;

public:
	PlayerContext() : m_isConnected(false), m_tableId(-1){}
	~PlayerContext() {}

public:
	void setUserId(std::string userId) { m_userId = userId; }
	std::string getUserId() const { return m_userId; }
	bool isConnected() const { return m_isConnected; }
	void setConnected(bool value) { m_isConnected = value; }
	void setTableId(int value) { m_tableId = value; }
	int getTableId() const { return m_tableId; }
};

#endif
