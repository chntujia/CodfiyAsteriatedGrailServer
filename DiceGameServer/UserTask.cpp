#include "stdafx.h"
#include "UserTask.h"
#include "UserSessionManager.h"
#include "GameManager.h"
#include "Communication.h"
#include "role\QiDao.h"
#include <boost/algorithm/string.hpp>

uint32_t UserTask::m_sIDSeq = 10000;
using namespace network;

void UserTask::Start()
{
	zTCPTask::Start();
	m_iTmpId = ++m_sIDSeq;

	UserSessionManager::getInstance().AddUserById(m_iTmpId, this);
	//FIXME temporarily disable login
	m_bAuthen = true;
	// TODO : check username & password here
	m_userId = TOQSTR(m_iTmpId);
	m_nickname = m_userId;
	UserSessionManager::getInstance().AddUser(m_userId, this);
}

void UserTask::OnQuit()
{
	zTCPTask::OnQuit();
	ztLoggerWrite(ZONE, e_Debug, "UserTask::OnQuit [%s] ",m_userId.c_str());
	GameGrail* game = getGame();
	if(game){
		game->onUserLeave(m_userId);
	}
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
		UserSessionManager::getInstance().RemoveUser(m_userId, m_iTmpId);
		SetQuit();
		return;
	}
}

GameGrail* UserTask::getGame()
{
	Game *game;
	if(m_tableId < 0){
		return NULL;
	}
	if(0 != GameManager::getInstance().getGame(GAME_TYPE_GRAIL, m_tableId, &game)){
		ztLoggerWrite(ZONE,e_Warning, "UserTask::getGame() failed. TableId: %d", m_tableId);
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

		int ret;
		int tableID;
		m_activeTime = time(NULL);
		
		switch(type)
		{
		case MSG_LOGIN_REQ:
			{
				//FIXME temporarily disable login
				m_bAuthen = true;
				// TODO : check username & password here
				m_userId = TOQSTR(m_iTmpId);
				UserSessionManager::getInstance().AddUser(m_userId, this);
				delete proto;
				break;
			}
		//创建房间
		case MSG_CREATE_ROOM_REQ:
			{
				handleCreateRoom(GAME_TYPE_GRAIL, proto);
				delete proto;
				break;
			}
		//进入房间
		case MSG_ENTER_ROOM_REQ:
			{					
				handleEnterRoom(GAME_TYPE_GRAIL, proto);
				delete proto;    // 如果不需要tryNotify或者tryNotify不带reply的话，释放message对象，这一步相当重要
				break;	
			}
		case MSG_LEAVE_ROOM_REQ:
			{
				handleLeaveRoom(GAME_TYPE_GRAIL, proto);
				delete proto;
				break;
			}
		case MSG_ROOMLIST_REQ:
			{
				handleRoomList(GAME_TYPE_GRAIL, proto);					
				delete proto;    // 如果不需要tryNotify或者tryNotify不带reply的话，释放message对象，这一步相当重要
				break;	
			}
		case MSG_READY_GAME_REQ:
			{
				handleReadyGame(GAME_TYPE_GRAIL, proto);					
				delete proto;    // 如果不需要tryNotify或者tryNotify不带reply的话，释放message对象，这一步相当重要
				break;	
			}
		case MSG_PICK_BAN:
			{
				PickBan* pick = (PickBan*)proto;
				int strategy = getGame()->m_roleStrategy;
				if(strategy == ROLE_STRATEGY_31 && pick->is_pick()){
					tryNotify(m_playerId, STATE_ROLE_STRATEGY_31, 0, pick);
				}
				else if(strategy == ROLE_STRATEGY_ANY && pick->is_pick()){
					tryNotify(m_playerId, STATE_ROLE_STRATEGY_ANY, 0, pick);
				}
				break;
			}
		case MSG_ACTION:
			{
				Action *action = (Action*)proto;
				// 行动
				tryNotify(m_playerId, STATE_ACTION_PHASE, 0, action);
				break;
			}
		case MSG_RESPOND:
			{
				Respond* respond = (Respond*)proto;
				switch(respond->respond_id())
				{
				case RESPOND_REPLY_ATTACK:
					tryNotify(m_playerId, STATE_ATTACKED, 0, respond);
					break;
				case RESPOND_DISCARD:
					tryNotify(m_playerId, STATE_REQUEST_HAND, 0, respond);
					break;
				case RESPOND_DISCARD_COVER:
					tryNotify(m_playerId, STATE_REQUEST_COVER, 0, respond);
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
				case RESPOND_HEAL:
					tryNotify(m_playerId, STATE_ASK_FOR_CROSS, 0, respond);
					break;			
				case WEI_LI_CI_FU:
					QiDao::WeiLiCiFuParse(this, m_playerId, proto);
					break;
				default:
					//尝试从角色的cmdMsgParse里找匹配
					if(getGame()->getPlayerEntity(m_playerId)->cmdMsgParse(this, type, proto) == false){
						ztLoggerWrite(ZONE, e_Error, "[%s]Received undefine MSG_RESPOND: %s,\n size:%d, type:%d,\n To proto: %s", m_userId.c_str(), pstrMsg, *size, type, proto->DebugString().c_str());
						delete proto;
					}
				}		
				break;
			}
		case MSG_TALK:
			{
				Talk* talk = (Talk*) proto;
				player_talk(getGame(), m_playerId, talk);
				delete proto;
				break;
			}
		default:
			ztLoggerWrite(ZONE, e_Error, "[%s]Received undefine MSG_TYPE: %s,\n size:%d, type:%d,\n To proto: %s", m_userId.c_str(), pstrMsg, *size, type, proto->DebugString().c_str());
			delete proto;
		}
		return true;
	}catch(GrailError e){
		ztLoggerWrite(ZONE, e_Error, "[%s]UserTask throws error: %d, Received Message: %s", m_userId.c_str(), e, pstrMsg);
		return false;
	}catch(std::exception const& e) {
		ztLoggerWrite(ZONE, e_Error, "[%s]UserTask throws error: %d, Received Message: %s",	m_userId.c_str(), e.what(), pstrMsg);
	}
}

void UserTask::handleCreateRoom(int game_type, void* req)
{
	CreateRoomRequest* create_room = (CreateRoomRequest*)req;
	GameConfig *config = new GameGrailConfig(create_room->max_player(), create_room->role_strategy());
	GameManager::getInstance().createGame(GAME_TYPE_GRAIL, config);
	EnterRoomRequest enter_room;
	enter_room.set_room_id(config->getTableId());
	handleEnterRoom(GAME_TYPE_GRAIL, &enter_room);
	delete config;
}

void UserTask::handleEnterRoom(int game_type, void* req)
{
	GameManager::getInstance().enterRoom(GAME_TYPE_GRAIL, m_userId, req);
}

void UserTask::handleLeaveRoom(int game_type, void* request)
{
	GameGrail* game = getGame();
	if(game){
		game->onUserLeave(m_userId);
	}
	else{
		ztLoggerWrite(ZONE, e_Warning, "UserTask::cmdMsgParse() userId [%s] cannot leave Table %d.", 
			m_userId.c_str(), m_tableId);
	}
}

void UserTask::handleRoomList(int game_type, void* req)
{
	RoomListRequest* request = (RoomListRequest*)req;
	RoomListResponse response;
	int ret = GameManager::getInstance().getGameList(GAME_TYPE_GRAIL, req, &response);
	if (ret == GE_SUCCESS){
		sendProto(MSG_ROOMLIST_REP, response);
	}
	else{
		ztLoggerWrite(ZONE, e_Error, "UserTask::cmdMsgParse() userId [%s] cannot retrieve TableList. Ret: %d", 
			m_userId.c_str(), ret);
	}
}

void UserTask::handleReadyGame(int game_type, void* req)
{
	ReadyForGameRequest* request = (ReadyForGameRequest*)req;
	switch(request->type())
	{
	case ReadyForGameRequest_Type_START_READY:
	case ReadyForGameRequest_Type_CANCEL_START_REDAY:
		{
			int ret = GameManager::getInstance().setPlayerReady(GAME_TYPE_GRAIL, m_tableId, m_playerId, req);
			if (ret != GE_SUCCESS){
				ztLoggerWrite(ZONE, e_Error, "UserTask::cmdMsgParse() userId [%s] cannot get ready. Table %d. Ret: %d", 
					m_userId.c_str(), m_tableId, ret);
			}		
		}
		break;
	case ReadyForGameRequest_Type_SEAT_READY:
		tryNotify(m_playerId, STATE_SEAT_ARRANGE);
		break;
	}
}

bool UserTask::msgParse(const void *pstrMsg, const uint32_t nCmdLen)
{
	return MessageQueue::msgParse((const char *)pstrMsg, nCmdLen);
}

void UserTask::sendProto(uint16_t proto_type, google::protobuf::Message& proto)
{
	string msg;
	proto_encoder(proto_type, proto, msg);
	SendCmd(msg.c_str(), msg.size());
}
