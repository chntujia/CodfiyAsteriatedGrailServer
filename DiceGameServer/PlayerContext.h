#ifndef __PLAYER_CONTEXT_H__
#define __PLAYER_CONTEXT_H__

#include <iostream>
using namespace std;

class PlayerContext
{
protected:
	string m_userId;
	int m_chips;
	bool m_isConnected;
	int m_tableId;

public:
	PlayerContext() {}
	PlayerContext(string userId);
	~PlayerContext() {}

public:
	string getUserId() const { return m_userId; }
	bool isConnected() const { return m_isConnected; }
	void setConnect(bool value) { m_isConnected = value; }
	void setTableId(int value) { m_tableId = value; }
	int getTableId() const { return m_tableId; }
	int getChips() const { return m_chips; }
	void changeChips(int value) { m_chips += value; }
};

#endif
