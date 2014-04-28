#ifndef __USER_TASK_H__
#define __USER_TASK_H__

#include <time.h>
#include "Config.h"
#include "zMisc.h"
#include "zTCPTask.h"
#include "MessageQueue.h"
#include "GameManager.h"
#include "GameGrail.h"

using namespace std;
class GameGrail;
class UserTask : public zTCPTask, public MessageQueue
{
private:      
	string		m_userId;
	int         m_playerId;
	int         m_tableId;
	bool        m_bAuthen;		//是否在指定时间内发送第一个Login包,如果没发踢掉
	time_t      m_activeTime;
	int32_t     m_iCheckTime;
	int32_t		m_gameType;

	uint32_t	m_iTmpId;
	static uint32_t m_sIDSeq;

public:
	string m_nickname;
	UserTask( boost::asio::io_service& service):zTCPTask(service) 
	{
//FIXME used to mimic userId, which should be retrieved from DB
		m_userId = "";
		m_playerId = -1;
		m_tableId = -1;
		m_bAuthen = false;
		m_activeTime = time(NULL);
		m_iCheckTime = ServerConfig::getInstance().m_iCheckTime;
	}

	virtual ~UserTask()
	{
		ztLoggerWrite(ZONE,e_Debug,"~UserTask ,[%s] close",m_userId.c_str() );
	}
	string getUserID(){  return m_userId;  }
	bool msgParse(const void *pstrMsg, const uint32_t nCmdLen);
	bool cmdMsgParse(const char *pstrMsg, const uint32_t nCmdLen);
	void setPlayerID(int id) { m_playerId = id; } 
	void setTableID(int id) { m_tableId = id; }
	GameGrail* getGame();
	bool tryNotify(int id, int state, int step = 0, void* reply = NULL);
	void Start();
	void OnQuit();
	void OnCheck();
	void sendProto(uint16_t proto_type, google::protobuf::Message& proto);
private:
	void handleCreateRoom(int game_type, void* request);
	void handleEnterRoom(int game_type, void* request);
	void handleLeaveRoom(int game_type, void* request);
	void handleRoomList(int game_type, void* request);
	void handleJoinTeam(int game_type, void* request);
	void handleReadyGame(int game_type, void* request);
};

#endif