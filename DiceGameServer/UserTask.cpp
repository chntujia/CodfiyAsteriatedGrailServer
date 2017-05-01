#include "stdafx.h"
#include "UserTask.h"
#include "GameManager.h"
#include "Communication.h"
#include "role\QiDao.h"
#include "role\ShiRen.h"
#include <boost/algorithm/string.hpp>
#include <boost/interprocess/detail/atomic.hpp>
#include "DBServices.h"

boost::uint32_t UserTask::m_sIDSeq = 10000;
using namespace network;

UserTask::~UserTask()
{
	ztLoggerWrite(ZONE, e_Information, "~UserTask[%s] deleted", m_userId.c_str() );
	UserSessionManager::getInstance().RemoveUser(m_userId);
	UserSessionManager::getInstance().RemoveUserById(m_iTmpId);
	GameGrail* game = getGame();
	if(game){
		game->onUserLeave(m_userId);
	}
}

void UserTask::Start()
{
	zTCPTask::Start();	
	m_iTmpId = boost::interprocess::ipcdetail::atomic_inc32(&m_sIDSeq);

	UserSessionManager::getInstance().AddUserById(m_iTmpId, this);
}

void UserTask::OnCheck()
{
	time_t tmNow  = time(NULL);
	if (tmNow - m_activeTime > m_iCheckTime)
	{	
		ztLoggerWrite(ZONE, e_Information, "OnCheck[%s]: heartbeat timeout,be kicked off ", m_userId.c_str());
		SetQuit();
		return;
	}
}

GameGrail* UserTask::getGame()
{
	GameGrail *game = NULL;
	if(m_tableId < 0){
		return NULL;
	}
	if(0 != GameManager::getInstance().getGame(m_tableId, &game)){
		ztLoggerWrite(ZONE,e_Warning, "UserTask::getGame() failed. TableId: %d", m_tableId);
		return NULL;
	}
	return game;
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
		if(!proto){
			ztLoggerWrite(ZONE, e_Error, "[%s]Receive: Unkown type: %d", m_userId.c_str(), type);
			return false;
		}
		
		m_activeTime = time(NULL);


		ztLoggerWrite(ZONE, e_Information, "[%s]Receive: type: %d,\n%s", m_userId.c_str(), type, proto->DebugString().c_str());
		
		
		switch (type)
		{
		case MSG_HEARTBEAT:
		{
			HeartBeat heartbeat;
			sendProto(MSG_HEARTBEAT, heartbeat);
			break;
		}
		case MSG_LOGIN_REQ:
		{
			handleLogIn((LoginRequest*)proto);
			delete proto;
			break;
		}
		//创建房间
		case MSG_CREATE_ROOM_REQ:
		{
			handleCreateRoom((CreateRoomRequest*)proto);
			delete proto;
			break;
		}
		//进入房间
		case MSG_ENTER_ROOM_REQ:
		{
			handleEnterRoom((EnterRoomRequest*)proto, false);
			delete proto;    // 如果不需要tryNotify或者tryNotify不带reply的话，释放message对象，这一步相当重要
			break;
		}
		case MSG_LEAVE_ROOM_REQ:
		{
			handleLeaveRoom((LeaveRoomRequest*)proto);
			delete proto;
			break;
		}
		case MSG_ROOMLIST_REQ:
		{
			handleRoomList((RoomListRequest*)proto);
			delete proto;    // 如果不需要tryNotify或者tryNotify不带reply的话，释放message对象，这一步相当重要
			break;
		}
		case MSG_JOIN_TEAM_REQ:
		{
			handleJoinTeam((JoinTeamRequest*)proto);
			delete proto;
			break;
		}
		case MSG_BECOME_LEADER_REP:
		{
			handleBecomeLeader((BecomeLeaderResponse*)proto);
			break;
		}
		case MSG_READY_GAME_REQ:
		{
			handleReadyGame((ReadyForGameRequest*)proto);
			delete proto;    // 如果不需要tryNotify或者tryNotify不带reply的话，释放message对象，这一步相当重要
			break;
		}
		case MSG_PICK_BAN:
		{
			PickBan* pick = (PickBan*)proto;
			GameGrail* game = getGame();
			if (!game) {
				delete proto;
				break;
			}
			if (game->m_roleStrategy == ROLE_STRATEGY_31 && pick->is_pick()) {
				tryNotify(m_playerId, STATE_ROLE_STRATEGY_31, 0, pick);
			}
			else if (game->m_roleStrategy == ROLE_STRATEGY_ANY && pick->is_pick()) {
				tryNotify(m_playerId, STATE_ROLE_STRATEGY_ANY, 0, pick);
			}
			else if (game->m_roleStrategy == ROLE_STRATEGY_BP) {
				tryNotify(m_playerId, STATE_ROLE_STRATEGY_BP, 0, pick);
			}
			else if (game->m_roleStrategy == ROLE_STRATEGY_CM) {
				tryNotify(m_playerId, STATE_ROLE_STRATEGY_CM, 0, pick);
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
			switch (respond->respond_id())
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
			case JI_ANG_KUANG_XIANG_QU:
			case JI_ANG_KUANG_XIANG_QU_2:
			case SHENG_LI_JIAO_XIANG_SHI:
			case SHENG_LI_JIAO_XIANG_SHI_2:
				ShiRen::ShiRenParse(this, m_playerId, proto);
				break;
			default:
				//尝试从角色的cmdMsgParse里找匹配
				GameGrail* game = getGame();
				if (!game || game->getPlayerEntity(m_playerId)->cmdMsgParse(this, type, proto) == false) {
					ztLoggerWrite(ZONE, e_Error, "[%s]Received undefine MSG_RESPOND:\n%s", m_userId.c_str(), proto->DebugString().c_str());
					delete proto;
				}
			}
			break;
		}
		case MSG_TALK:
		{
			Talk* talk = (Talk*)proto;
			player_talk(getGame(), m_playerId, talk);
			delete proto;
			break;
		}
		case MSG_POLLING_REP:
		{
			GameGrail*game = getGame();
			if (game) {
				tryNotify(m_playerId, game->gameover ? STATE_POLLING_GAMEOVER : STATE_POLLING_DISCONNECTED, 0, proto);
			}
			break;
		}
		default:
			ztLoggerWrite(ZONE, e_Error, "[%s]Received undefine MSG_TYPE: type:%d,\n To proto: %s", m_userId.c_str(), type, proto->DebugString().c_str());
			delete proto;
		}
		return true;
	}catch(GrailError e){
		ztLoggerWrite(ZONE, e_Error, "[%s]UserTask throws error: %d, Received Message: %s", m_userId.c_str(), e, pstrMsg);
		return false;
	}catch(std::exception const& e) {
		ztLoggerWrite(ZONE, e_Error, "[%s]UserTask throws error: %s, Received Message: %s",	m_userId.c_str(), e.what(), pstrMsg);
	}
}

void UserTask::handleLogIn(LoginRequest* req)
{
	LoginResponse response;
	if(req->version() < ServerConfig::getInstance().m_version){
		ztLoggerWrite(ZONE, e_Error, "%d < %d", req->version() < ServerConfig::getInstance().m_version);
		Coder::logInResponse(STATUS_OUTDATE, "", response);
		sendProto(MSG_LOGIN_REP, response);
	}
	else if(req->asguest()){
		m_userType = STATUS_GUEST;
		m_bAuthen = true;
		m_userId = TOQSTR(m_iTmpId);
		m_nickname = m_userId;		
	}
	else{
		struct UserAccount account = DBInstance.userAccountDAO->query(req->user_id(), req->user_password());
		m_userType = account.status;
		if(m_userType == STATUS_NORMAL || m_userType == STATUS_VIP || m_userType == STATUS_ADMIN){
			m_bAuthen = true;
			m_userId = account.username;
			m_nickname = account.nickname;
		}
	}
	if(m_bAuthen){
		UserSessionManager::getInstance().AddUser(m_userId, this);		
	}
	Coder::logInResponse(m_userType, m_nickname, response);
	sendProto(MSG_LOGIN_REP, response);
}

void UserTask::handleCreateRoom(CreateRoomRequest* req)
{
	if (logLevel != e_Debug) {
		switch (m_userType) {
		case STATUS_GUEST:
		case STATUS_NORMAL:
			if (req->sp_mo_dao() || req->role_strategy() == ROLE_STRATEGY_BP || req->role_strategy() == ROLE_STRATEGY_CM) {
				Error error;
				Coder::errorMsg(GE_VIP_ONLY, -1, error);
				sendProto(MSG_ERROR, error);
				return;
			}
			break;
		}
	}
	int tableId = GameManager::getInstance().createGame(req);
	EnterRoomRequest enter_room;
	enter_room.set_room_id(tableId);
	handleEnterRoom(&enter_room, true);
}

void UserTask::handleEnterRoom(EnterRoomRequest* req, bool bypassed)
{
	ztLoggerWrite(ZONE, e_Information, "UserTask::handleEnterRoom() userId [%s] enter Table %d.", 
			m_userId.c_str(), m_tableId);
	int playerId = GUEST;
	int ret;
	if(m_tableId != req->room_id()){
		GameGrail* game = getGame();
		if(game){
			game->onUserLeave(m_userId);
		}
	}
	if(bypassed){
		ret = GameManager::getInstance().enterRoom(m_userId, m_nickname, req, playerId);
	}
	else{
		ret = GameManager::getInstance().tryEnterRoom(m_userId, m_nickname, req, playerId, m_userType);
	}
	if(ret == GE_SUCCESS){
		m_tableId = req->room_id();
		m_playerId = playerId;
	}
	else{	
		Error error;
		Coder::errorMsg(ret, playerId, error);
		sendProto(MSG_ERROR, error);
	}
}

void UserTask::handleLeaveRoom(LeaveRoomRequest* request)
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

void UserTask::handleRoomList(RoomListRequest* req)
{
	RoomListResponse response;
	int ret = GameManager::getInstance().getGameList(req, &response);
	if (ret == GE_SUCCESS){
		sendProto(MSG_ROOMLIST_REP, response);
	}
	else{
		ztLoggerWrite(ZONE, e_Error, "UserTask::cmdMsgParse() userId [%s] cannot retrieve TableList. Ret: %d", 
			m_userId.c_str(), ret);
	}
}

void UserTask::handleReadyGame(ReadyForGameRequest* req)
{
	switch (req->type())
	{
	case ReadyForGameRequest_Type_START_READY:
	case ReadyForGameRequest_Type_CANCEL_START_REDAY:
	{
		int ret = GameManager::getInstance().setPlayerReady(m_tableId, m_playerId, req);
		if (ret != GE_SUCCESS) {
			ztLoggerWrite(ZONE, e_Error, "UserTask::cmdMsgParse() userId [%s] cannot get ready. Table %d. Ret: %d",
				m_userId.c_str(), m_tableId, ret);
		}
	}
	break;
	}
}

void UserTask::handleJoinTeam(JoinTeamRequest* req)
{
	GameGrail* game = getGame();
	if(game){
		game->setTeam(m_playerId, req->team());
	}
	else
		ztLoggerWrite(ZONE, e_Warning, "UserTask::cmdMsgParse() userId [%s] cannot join team. Table %d.", 
					m_userId.c_str(), m_tableId);
}

void UserTask::handleBecomeLeader(BecomeLeaderResponse* res)
{
	GameGrail* game = getGame();
	if(game){
		tryNotify(m_playerId, STATE_LEADER_ELECTION, 0, res);
	}
	else
		ztLoggerWrite(ZONE, e_Warning, "UserTask::cmdMsgParse() userId [%s] cannot become leader. Table %d.", 
					m_userId.c_str(), m_tableId);
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
