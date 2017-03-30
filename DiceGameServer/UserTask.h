#ifndef __USER_TASK_H__
#define __USER_TASK_H__

#include <time.h>
#include "Config.h"
#include "zMisc.h"
#include "zTCPTask.h"
#include "MessageQueue.h"
#include "GameGrail.h"
#include "UserSessionManager.h"
#include "UserAccount.h"

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
	ACCOUNT_STATUS    m_userType;
	static boost::uint32_t m_sIDSeq;
public:
	string m_nickname;
	UserTask( boost::asio::io_service& service): zTCPTask(service) 
	{
		m_userId = "";
		m_playerId = -1;
		m_tableId = -1;
		m_bAuthen = false;
		m_userType = STATUS_LOGIN_FAILED;
		m_activeTime = time(NULL);
		m_iCheckTime = ServerConfig::getInstance().m_iCheckTime;
	}

	~UserTask();
	string getUserID(){  return m_userId;  }
	uint32_t getUserTempId(){ return m_iTmpId; }
	bool msgParse(const void *pstrMsg, const uint32_t nCmdLen);
	bool cmdMsgParse(const char *pstrMsg, const uint32_t nCmdLen);
	GameGrail* getGame();
	bool tryNotify(int id, int state, int step = 0, void* reply = NULL);
	void Start();
	void OnCheck();
	void sendProto(uint16_t proto_type, google::protobuf::Message& proto);
private:
	void handleLogIn(LoginRequest* request);
	void handleCreateRoom(CreateRoomRequest* request);
	void handleEnterRoom(EnterRoomRequest* request, bool bypassed);
	void handleLeaveRoom(LeaveRoomRequest* request);
	void handleRoomList(RoomListRequest* request);
	void handleJoinTeam(JoinTeamRequest* request);
	void handleReadyGame(ReadyForGameRequest* request);
	void handleBecomeLeader(BecomeLeaderResponse* request);
};

#endif