#include "stdafx.h"
#include "UserTask.h"
#include "UserSessionManager.h"
#include "GameManager.h"
#include "GameGrail.h"

#include <boost/algorithm/string.hpp>

uint32_t UserTask::m_sIDSeq = 10000;
using namespace network;

void UserTask::Start()
{
	zTCPTask::Start();
	m_iTmpId = ++m_sIDSeq;

	UserSessionManager::getInstance().AddUserById(m_iTmpId, this);
}

void UserTask::OnQuit()
{
	zTCPTask::OnQuit();
	ztLoggerWrite(ZONE, e_Debug, "UserTask::OnQuit [%s] ",m_userId.c_str());

	UserSessionManager::getInstance().RemoveUser(m_userId, m_iTmpId);
}

void UserTask::OnCheck()
{
	/*if (!m_bAuthen) 
	{
		ztLoggerWrite(ZONE,e_Debug, "OnCheck[%s]: don't authen, kicked off ", m_userId.c_str());
		SetQuit();
		return;
	}*/

	time_t tmNow  = time(NULL);
	int temp = ServerConfig::getInstance().m_iCheckTime;
	if (tmNow - m_activeTime > ServerConfig::getInstance().m_iCheckTime)
	{
		ztLoggerWrite(ZONE,e_Debug, "OnCheck[%s]: heartbeat timeout,be kicked off ", m_userId.c_str());
		SetQuit();
		return;
	}
}

GameGrail* UserTask::getGame()
{
	Game *game;
	if(0 != GameManager::getInstance().getGame(GAME_TYPE_GRAIL, m_tableId, &game)){
		ztLoggerWrite(ZONE,e_Error, "UserTask::getGame() failed. TableId: %d", m_tableId);
		return NULL;
	}
	return (GameGrail*)game;
}

bool UserTask::tryNotify(int id, int state, int step, void* reply)
{
	GameGrail*game = getGame();
	if(!game){
		if (reply != NULL)
			delete reply;  // 不需要的消息就释放掉，如果被需要，会暂时保存，直到下一个成功set进去的Reply
		return false;
	}
	bool try_notify_success = game->tryNotify(id, state, step, reply);
	if (!try_notify_success)
		if (reply != NULL)
			delete reply;  // 不需要的消息就释放掉，如果被需要，会暂时保存，直到下一个成功set进去的Reply
	return try_notify_success;
}

// 消息处理中心
bool UserTask::cmdMsgParse(const char *pstrMsg, const uint32_t nCmdLen)
{
	try
	{
		uint16_t type;
		::google::protobuf::Message *proto = (::google::protobuf::Message*) proto_decoder(pstrMsg, type);
		uint16_t* size = (uint16_t*)pstrMsg;
#ifdef Debug
		ztLoggerWrite(ZONE, e_Debug, "[%s]Receive: %s,\n size:%d, type:%d,\n To proto: %s", m_userId.c_str(), pstrMsg, *size, type, proto->DebugString().c_str());
#endif

		GameGrailConfig*config;
		int ret;
		int tableID;
		int actionFlag;
		m_activeTime = time(NULL);
		EnterRoom* enter_room;
		Action *action;
		Respond *respond;

		switch(type)
		{

		case MSG_ENTER_ROOM:
			enter_room = (EnterRoom*)proto;
			/*		if (enter_room->room_id() == 0){
			//创建房间
			config = new GameGrailConfig(2,0);
			GameManager::getInstance().createGame(GAME_TYPE_GRAIL, config);
			delete config;
			}
			else
			*/{
				//进入房间
				//FIXME temporarily disable login
				m_bAuthen = true;
				// TODO : check username & password here
				m_userId = TOQSTR(m_iTmpId);
				UserSessionManager::getInstance().AddUser(m_userId, this);
				m_gameType = GAME_TYPE_GRAIL;
				tableID = enter_room->room_id();
				ret = GameManager::getInstance().sitIntoTable(m_userId, GAME_TYPE_GRAIL, tableID);
				if (ret == SIT_TABLE_SUCCESS){
					m_tableId = tableID;
				}
				else{
					ztLoggerWrite(ZONE, e_Error, "UserTask::cmdMsgParse() userId [%s] cannot enter Table %d. Ret: %d", 
						m_userId.c_str(), tableID, ret);
				}
			}

			delete proto;    // 如果不需要tryNotify或者tryNotify不带reply的话，释放message对象，这一步相当重要
			break;	
		case MSG_START_REP:
			//clicked start
			tryNotify(m_playerId, STATE_SEAT_ARRANGE);

			delete proto;    // 如果不需要tryNotify或者tryNotify不带reply的话，释放message对象，这一步相当重要
			break;
		case MSG_ACTION:
			action = (Action*)proto;
			// 行动
			tryNotify(m_playerId, STATE_ACTION_PHASE, 0, action);
			break;
		case MSG_RESPOND:
			respond = (Respond*)proto;
			switch(respond->respond_id())
			{
			case RESPOND_REPLY_ATTACK:
				tryNotify(m_playerId, STATE_ATTACKED, 0, respond);
				break;
			case RESPOND_DISCARD:
				tryNotify(m_playerId, STATE_REQUEST_HAND, 0, respond);
				break;
			case RESPOND_BULLET:
				tryNotify(m_playerId, STATE_MISSILED, 0, respond);
				break;
			case RESPOND_WEAKEN:
				tryNotify(m_playerId, STATE_WEAKEN, 0, respond);
				break;
			case RESPOND_ADDITIONAL_ACTION:
				tryNotify(m_playerId, STATE_ADDITIONAL_ACTION, 0, respond);
				break;			
			default:
				//尝试从角色的cmdMsgParse里找匹配
				if(getGame()->getPlayerEntity(m_playerId)->cmdMsgParse(this, type, proto) == false){
					ztLoggerWrite(ZONE, e_Error, "[%s]Received undefine MSG_RESPOND: %s,\n size:%d, type:%d,\n To proto: %s", m_userId.c_str(), pstrMsg, *size, type, proto->DebugString().c_str());
					delete proto;
				}
			}		
			break;
		default:
			ztLoggerWrite(ZONE, e_Error, "[%s]Received undefine MSG_TYPE: %s,\n size:%d, type:%d,\n To proto: %s", m_userId.c_str(), pstrMsg, *size, type, proto->DebugString().c_str());
			delete proto;
		}
		return true;
	}catch(GrailError e){
		ztLoggerWrite(ZONE, e_Error, "[%s]UserTask throws error: %d, Received Message: %s", m_userId.c_str(), e, pstrMsg);
		return false;
	}
}

bool UserTask::msgParse(const void *pstrMsg, const uint32_t nCmdLen)
{
	return MessageQueue::msgParse((const char *)pstrMsg, nCmdLen);
}

/*
bool UserTask::handleUserLogin(const char *pstrCmd, const uint32_t nCmdLen)
{

	LobbyUserLoginReq msgUserLogin;
	msgUserLogin.ParseFromArray(pstrCmd, nCmdLen);

	m_userId = msgUserLogin.userid();
	string password = msgUserLogin.password();

//  UserTask_Ptr sess(shared_from_this());
//	UserSessionManager::getInstance().AddUser(m_userId,sess);
	if(m_userId == "" || password == "")
	{
		m_bAuthen = false;
	}
	else
	{
		m_bAuthen = true;

		// TODO : check username & password here
		UserSessionManager::getInstance().AddUser(m_userId, this);

		// TODO : delete password in log
		ztLoggerWrite(ZONE,e_Debug, "handleUserLogin[%s]:: %s", m_userId.c_str(), password.c_str());
	}
	
	//return msg
	DiceMessage retMsg;
	
	// all login success now
	LobbyUserLoginRet proLoginRetMsg;
	proLoginRetMsg.set_userid(m_userId);
	if(m_bAuthen == true)
	{
		proLoginRetMsg.set_result(proLoginRetMsg.LOGIN_RESULT_SUCCESS);
	}
	else
	{
		proLoginRetMsg.set_result(proLoginRetMsg.LOGIN_RESULT_FAIL);
	}

	PacketCmd(MG_LobbyUserLoginRet, proLoginRetMsg, retMsg);
	SendCmd(retMsg.SerializeAsString().c_str(), retMsg.ByteSize());
	return m_bAuthen;
}

bool UserTask::handleGetGameList(const char *pstrCmd, const uint32_t nCmdLen)
{
	LobbyEnterGame msgEnterGame;
	msgEnterGame.ParseFromArray(pstrCmd, nCmdLen);

	string userId = msgEnterGame.userid();
	int gameType = msgEnterGame.type();

	// check userId = m_userId or not 
	if(userId != m_userId)
	{
		ztLoggerWrite(ZONE,e_Debug, "handleGetGameList[%s] is not %s", m_userId.c_str(), userId.c_str());
		// TODO : error 

		return false;
	}
	DiceMessage retMsg = GameManager::getInstance().getGameList(gameType);
	SendCmd(retMsg.SerializeAsString().c_str(), retMsg.ByteSize());
	m_gameType = gameType;
	return true;
}

bool UserTask::handleSicBoSitIntoTable(const char *pstrCmd, const uint32_t nCmdLen)
{
	SicBoSitIntoTableReq msgSitIntoTable;
	msgSitIntoTable.ParseFromArray(pstrCmd, nCmdLen);

	string userId = msgSitIntoTable.userid();
	int tableId = msgSitIntoTable.tableid();

	bool ret = true;
	DiceMessage retMsg;
	// check userId = m_userId or not 
	if(userId != m_userId)
	{
		ztLoggerWrite(ZONE,e_Debug, "handleSicBoSitIntoTable[%s] is not %s", m_userId.c_str(), userId.c_str());
		SicBoSitIntoTableRes proSitIntoTableRes;
		proSitIntoTableRes.set_userid(m_userId);Z
		proSitIntoTableRes.set_result(SicBoSitIntoTableRes::SICBO_SIT_TABLE_OTHER);

		PacketCmd(MG_LobbyUserLoginRet, proSitIntoTableRes, retMsg);
		ret = false;
	}
	else
	{
		retMsg = GameManager::getInstance().sitIntoTable(userId, GAME_TYPE_SICBO, tableId);
	}
	SendCmd(retMsg.SerializeAsString().c_str(), retMsg.ByteSize());
	return ret;
}

bool UserTask::handleSicBoGameBetAction(const char *pstrCmd, const uint32_t nCmdLen)
{
	SicBoBetAction msgSitBet;
	msgSitBet.ParseFromArray(pstrCmd, nCmdLen);

	string userId = msgSitBet.userid();
	int tableId = msgSitBet.tableid();
	int position = msgSitBet.position();
	int betNum = msgSitBet.betnumber();

	bool ret = true;
	DiceMessage retMsg;
	if(userId != m_userId)
	{
		ztLoggerWrite(ZONE,e_Debug, "handleSicBoGameBetAction[%s] is not %s", m_userId.c_str(), userId.c_str());
		SicBoBetResult proSicBetRes;
		proSicBetRes.set_userid(m_userId);
		proSicBetRes.set_result(SicBoBetResult::SICBO_BET_NOT_IN_THIS_TABLE);

		PacketCmd(MG_LobbyUserLoginRet, proSicBetRes, retMsg);
		ret = false;
	}
	else
	{
		Game *game;
		if(GameManager::getInstance().getGame(GAME_TYPE_SICBO, tableId, game) == 0)
		{
			retMsg = ((GameSicBo*)game)->handlePlayerBetAction(userId, position, betNum);
		}
		else
		{
			ztLoggerWrite(ZONE,e_Debug, "handleSicBoGameBetAction[%s] no this table[%d]", m_userId.c_str(), tableId);
			SicBoBetResult proSicBetRes;
			proSicBetRes.set_userid(m_userId);
			proSicBetRes.set_result(SicBoBetResult::SICBO_BET_NOT_IN_THIS_TABLE);

			PacketCmd(MG_LobbyUserLoginRet, proSicBetRes, retMsg);
			ret = false;
		}
	}

	SendCmd(retMsg.SerializeAsString().c_str(), retMsg.ByteSize());
	return ret;
}

bool UserTask::handleSicBoGameRefresh(const char *pstrCmd, const uint32_t nCmdLen)
{
	SicBoTableRefresh msgSicRefresh;
	msgSicRefresh.ParseFromArray(pstrCmd, nCmdLen);

	string userId = msgSicRefresh.userid();
	int tableId = msgSicRefresh.tableid();

	bool ret = true;
	DiceMessage retMsg;
	if(userId != m_userId)
	{
		ztLoggerWrite(ZONE,e_Debug, "handleSicBoGameRefresh[%s] is not %s", m_userId.c_str(), userId.c_str());
		ret = false;
	}
	else
	{
		Game *game;
		if(GameManager::getInstance().getGame(GAME_TYPE_SICBO, tableId, game) == 0)
		{
			retMsg = ((GameSicBo*)game)->handlePlayerRefreshTable(userId);
		}
		else
		{
			ret = false;
		}
	}

	SendCmd(retMsg.SerializeAsString().c_str(), retMsg.ByteSize());
	return ret;
}
*/