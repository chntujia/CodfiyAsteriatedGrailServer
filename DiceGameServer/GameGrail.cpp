#include "stdafx.h"

#include <boost/random.hpp>
#include <boost/bind.hpp>
#include "GameGrail.h"
#include "GrailState.h"
#include "zLogger.h"
#include "zCommonDefine.h"
#include "UserSessionManager.h"
#include "role\JianSheng.h"
#include "role\AnSha.h"
#include "role\KuangZhan.h"
#include "role\FengYin.h"
#include "role\MoDao.h"
#include "role\YuanSu.h"
#include "role\ShengNv.h"
#include "role\SiLing.h"
#include "role\ShengQiang.h"
#include "role\MaoXian.h"
#include "role\GongNv.h"
#include "role\ShenGuan.h"
#include "role\MoJian.h"
#include "role\LingHun.h"
#include "role\TianShi.h"
#include "role\XianZhe.h"
#include "role\WuNv.h"
#include "role\LingFu.h"
#include "role\GeDou.h"
#include "role\QiDao.h"
#include "role\ZhongCai.h"
#include "role\HongLian.h"
#include "role\MoGong.h"
#include "role\JianDi.h"
#include "role\YongZhe.h"
#include "role\MoQiang.h"
#include "role\DieWu.h"
#include "role\LingHun.h"
using namespace boost;

PlayerEntity* GameGrail::createRole(int id, int roleID, int color)
{
	switch(roleID)
	{
	case 1:
		return new JianSheng(this,id,color);
		break;
	case 2:
		return new KuangZhan(this,id,color);
		break;
	case 3:
		return new GongNv(this,id,color);
		break;
	case 4:
		return new FengYin(this,id,color);
		break;
	case 5:
		return new AnSha(this,id,color);
		break;
	case 6:
		return new ShengNv(this,id,color);
		break;
	case 7:
		return new TianShi(this,id,color);
		break;
	case 8:
		return new MoDao(this,id,color);
		break;
	case 9:
		return new MoJian(this,id,color);
		break;
	case 10:
		return new ShengQiang(this, id, color);
		break;
	case 11:
		return new YuanSu(this,id,color);
		break;
	case 12:
		return new MaoXian(this,id,color);
		break;
	case 13:
		return new SiLing(this,id,color);
		break;
	case 14:
		return new ZhongCai(this,id,color);
		break;
	case 15:
		return new ShenGuan(this,id,color);
		break;
	case 16:
		return new QiDao(this,id,color);
		break;
	case 17:
		return new XianZhe(this,id,color);
		break;
	case 18:
		return new LingFu(this,id,color);
		break;
	case 19:
		return new JianDi(this,id,color);
		break;
	case 20:
		return new GeDou(this,id,color);
		break;

	case 21:
		return new YongZhe(this,id,color);
		break;
	case 22:
		return new LingHun(this,id,color);
		break;
	case 23:
		return new WuNv(this,id,color);
		break;
	case 24:
		return new DieWu(this,id,color);
		break;
	case 26:
		return new MoGong(this,id,color);
		break;
	case 28:
		return new HongLian(this,id,color);
		break;
	case 29:
		return new MoQiang(this,id,color);
		break;
	}
	throw GE_INVALID_ROLEID;
}

void TeamArea::initialTeam()
{
    this->moraleBLUE = 15;
    this->moraleRED = 15;
    this->gemBLUE = 0;
    this->gemRED = 0;
    this->crystalBLUE = 0;
    this->crystalRED = 0;
    this->cupBLUE = 0;
    this->cupRED = 0;
}

void TeamArea::setCrystal(int color, int value)
{
    if(color == RED && value+gemRED<=5)
        this->crystalRED = value;
    else if(color == BLUE && value+gemBLUE<=5)
        this->crystalBLUE = value;
}

void TeamArea::setCup(int color, int value)
{
    if(color == RED)
        this->cupRED = value;
    else if(color == BLUE)
        this->cupBLUE = value;
}

void TeamArea::setGem(int color, int value)
{
    if(color == RED && value+crystalRED<=5)
        this->gemRED = value;
    else if(color == BLUE && value+crystalBLUE<=5)
        this->gemBLUE = value;
}

void TeamArea::setMorale(int color, int value)
{
    if(value<0)
        value=0;
    if(color == RED)
        this->moraleRED = value;
    else if(color == BLUE)
        this->moraleBLUE = value;
}

GameGrail::GameGrail(GameGrailConfig *config) : playing(false), processing(true), roleInited(false)
{
	m_gameId = config->getTableId();
	m_gameName = config->getTableName();
	m_gameType = GAME_TYPE_GRAIL;
	m_roundId = 0;
	m_maxPlayers = config->maxPlayers;
	m_roleStrategy = config->roleStrategy;
	m_seatMode = 0;
	m_responseTime = 20;
	m_maxAttempts = 2;
	pushGameState(new StateWaitForEnter);
}

GameGrail::~GameGrail()
{
	processing = false;
	while(!m_states.empty()){
		popGameState();
	}
	for(int i=0; i<m_maxPlayers; i++){
		SAFE_DELETE(m_playerContexts[i]);
	}
	m_playerContexts.clear();
	for(int i=0; i<m_maxPlayers; i++){
		SAFE_DELETE(m_playerEntities[i]);
	}
	m_playerEntities.clear();
	m_guestList.clear();
	SAFE_DELETE(m_teamArea);
	SAFE_DELETE(pile);
	SAFE_DELETE(discard);
}

int GameGrail::popGameState_if(int state)
{
	if(topGameState()->state == state){
		SAFE_DELETE(m_states.top()); 
		m_states.pop();
		return GE_SUCCESS;
	}
	ztLoggerWrite(ZONE, e_Warning, "[Table %d] Attempt to pop state: %d, but current state: %d"
		, m_gameId, state, topGameState()->state);
	return GE_INVALID_PLAYERID;
}

void GameGrail::sendMessage(int id, uint16_t proto_type, google::protobuf::Message& proto)
{
#ifdef Debug
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] send to %d, type:%d, string:\n%s \nsize: %d", m_gameId, id, proto_type, proto.DebugString().c_str(), proto.ByteSize());
#endif
	UserTask *ref;

	if(id == -1){
		for(PlayerContextList::iterator it = m_playerContexts.begin(); it != m_playerContexts.end(); it++){
			ref = UserSessionManager::getInstance().getUser(it->second->getUserId());
			if(ref){
				ref->sendProto(proto_type, proto);
			}
			else{
				ztLoggerWrite(ZONE, e_Debug, "[Table %d] Cannot find UserSession, PlayerID: %d, UserID: %s", m_gameId, id, it->second->getUserId().c_str());
			}
		}
		for(list<string>::iterator it = m_guestList.begin(); it != m_guestList.end(); it++){
			ref = UserSessionManager::getInstance().getUser(*it);
			if(ref){
				ref->sendProto(proto_type, proto);
			}
			else{
				ztLoggerWrite(ZONE, e_Debug, "[Table %d] Cannot find UserSession, GuestID: %d, UserID: %s", m_gameId, id, *it);
			}
		}
	}
	else{
		PlayerContextList::iterator it = m_playerContexts.find(id);
		if(it != m_playerContexts.end()){
			ref = UserSessionManager::getInstance().getUser(it->second->getUserId());
			if(ref){
				ref->sendProto(proto_type, proto);
			}
			else{
				ztLoggerWrite(ZONE, e_Debug, "[Table %d] Cannot find UserSession, PlayerID: %d, UserID: %s", m_gameId, id, it->second->getUserId().c_str());
			}
		}
	}
}

void GameGrail::sendMessageExcept(int id, uint16_t proto_type, google::protobuf::Message& proto)
{
#ifdef Debug
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] send to %d, type:%d, string:\n%s \nsize: %d", m_gameId, id, proto_type, proto.DebugString().c_str(), proto.ByteSize());
#endif
	UserTask *ref;

	for(PlayerContextList::iterator it = m_playerContexts.begin(); it != m_playerContexts.end(); it++){
		if(it->first != id){
			ref = UserSessionManager::getInstance().getUser(it->second->getUserId());
			if(ref){
				ref->sendProto(proto_type, proto);
			}
			else{
				ztLoggerWrite(ZONE, e_Debug, "[Table %d] Cannot find UserSession, PlayerID: %d, UserID: %s", m_gameId, id, it->second->getUserId().c_str());
			}
		}
	}
	for(list<string>::iterator it = m_guestList.begin(); it != m_guestList.end(); it++){
		ref = UserSessionManager::getInstance().getUser(*it);
		if(ref){
			ref->sendProto(proto_type, proto);
		}
		else{
			ztLoggerWrite(ZONE, e_Debug, "[Table %d] Cannot find UserSession, GuestID: %d, UserID: %s", m_gameId, id, *it);
		}
	}

}

bool GameGrail::isReady(int id)
{
	if(id == -1){
		for(int i = 0; i < m_maxPlayers; i++){
			if(!m_ready[i]){
				return false;
			}
		}
		return true;
	}
	else if(id >= 0 && id < m_maxPlayers){
		return m_ready[id];
	}
	ztLoggerWrite(ZONE, e_Error, "invalid player id: %d", id);
	return false;
} 

bool GameGrail::waitForOne(int id, uint16_t proto_type, google::protobuf::Message& proto, int sec, bool toResetReady)
{
	if(id < 0 || id >= m_maxPlayers){
		ztLoggerWrite(ZONE, e_Error, "unvaliad player id: %d", id);
		return false;
	}
	m_token = id;
	if(toResetReady){
		resetReady(id);
	}	
	
	int attempts = 0;
	boost::mutex::scoped_lock lock(m_mutex_for_wait);
	while(attempts < m_maxAttempts)
	{
		sendMessage(-1, proto_type, proto);
		boost::system_time const timeout=boost::get_system_time()+ boost::posix_time::milliseconds(sec*1000);
		if(m_condition_for_wait.timed_wait(lock,timeout,boost::bind( &GameGrail::isReady, this, id )))
			return true;
		attempts++;
	}
	return false;
}

bool GameGrail::waitForAll(uint16_t proto_types, void** proto_ptrs, int sec, bool toResetReady)
{
	m_token = -1;
	int attempts = 0;
	boost::mutex::scoped_lock lock(m_mutex_for_wait);
	if(toResetReady){
		resetReady();
	}
	while(attempts < m_maxAttempts)
	{
		for(int i = 0; i < m_maxPlayers; i++){
			if(!m_ready[i]){
				google::protobuf::Message* proto = (google::protobuf::Message*)proto_ptrs[i];
				sendMessage(i, proto_types, *proto);
			}
		}
		boost::system_time const timeout=boost::get_system_time()+ boost::posix_time::milliseconds(sec*1000);
		if(m_condition_for_wait.timed_wait(lock,timeout,boost::bind( &GameGrail::isReady, this, -1 ))){
			return true;
		}
		attempts++;
	}
	return false;
}

bool GameGrail::tryNotify(int id, int state, int step, void* reply)
{
	boost::mutex::scoped_lock lock(m_mutex_for_notify);
	if(id == m_token && state == topGameState()->state && step == topGameState()->step) {
		m_ready[id] = true;
		if(reply){
			m_playerContexts[id]->setBuf(reply);
		}
		m_condition_for_wait.notify_one();
		return true;
	}
	else if(m_token == -1 && state == topGameState()->state && step == topGameState()->step){
		m_ready[id] = true;
		if(reply){
			m_playerContexts[id]->setBuf(reply);
		}
		if(isReady(-1)){
			m_condition_for_wait.notify_one();
		}
	    return true;
	}
	ztLoggerWrite(ZONE, e_Warning, "[Table %d] Unauthorized notify detected. Player id: %d, current token: %d; claimed state: %d, current state: %d; claim step: %d, current step: %d", m_gameId,
		id, m_token, state, topGameState()->state, step, topGameState()->step);
	return false;
}

int GameGrail::getReply(int id, void* &reply)
{
	std::map<int, GameGrailPlayerContext*>::iterator iter;
	if((iter=m_playerContexts.find(id)) == m_playerContexts.end()){
		return GE_INVALID_PLAYERID;
	}
	reply = iter->second->getBuf();
	if(!reply){
		return GE_NO_REPLY;
	}	
	return GE_SUCCESS;
}

//FIXME: 现阶段只支持也只需支持以下情况：
//从牌堆、基础状态、别人手牌移入手牌
//手牌移出至弃牌堆、基础状态、别人手里、盖牌
int GameGrail::setStateMoveCards(int srcOwner, int srcArea, int dstOwner, int dstArea, int howMany, vector< int > cards, HARM harm, bool isShown)
{
	PlayerEntity *src;
	int ret;
	bool updated = false;
	//check whether exists

	GameInfo update_info;

	switch(srcArea)
	{
	case DECK_PILE:
		drawCardsFromPile(howMany, cards);
		update_info.set_pile(pile->get_size());
		updated = true;
		break;
	case DECK_HAND:
		src = getPlayerEntity(srcOwner);
		if(GE_SUCCESS != (ret = src->checkHandCards(howMany, cards))){
			throw ret;
		}
		break;
	case DECK_BASIC_EFFECT:
		src = getPlayerEntity(srcOwner);
		if(GE_SUCCESS != (ret = src->checkBasicEffectByCard(cards[0]))){
			return ret;
		}
		break;
	case DECK_COVER:
		src = getPlayerEntity(srcOwner);
		if(GE_SUCCESS != (ret = src->checkCoverCards(howMany, cards))){
			throw ret;
		}
		break;
	default:
		return GE_NOT_SUPPORTED;
	}
	//src hand change->show hand->dst hand change->dst hand overload, but stack is LIFO
	switch(dstArea)
	{
	case DECK_DISCARD:
		ret = discard->push(howMany, &cards[0]);
		update_info.set_discard(discard->get_size());
		updated = true;
		break;
	case DECK_HAND:
		pushGameState(new StateHandChange(dstOwner, CHANGE_ADD, howMany, cards, harm));							
		break;
	case DECK_BASIC_EFFECT:
		pushGameState(new StateBasicEffectChange(dstOwner, CHANGE_ADD, cards[0], harm.srcID, harm.cause));	
		break;
	case DECK_COVER:
		pushGameState(new StateCoverChange(dstOwner, CHANGE_ADD, howMany, cards, harm));	
		break;
	default:
		return GE_NOT_SUPPORTED;
	}
	switch(srcArea)
	{
	case DECK_PILE:
		break;
	case DECK_HAND:
		if(isShown){
			pushGameState(new StateShowHand(srcOwner, howMany, cards, harm));
		}
		pushGameState(new StateHandChange(srcOwner, CHANGE_REMOVE, howMany, cards, harm));	
		break;
	case DECK_BASIC_EFFECT:
		pushGameState(new StateBasicEffectChange(srcOwner, CHANGE_REMOVE, cards[0], harm.srcID, harm.cause));
		break;
	case DECK_COVER:
		pushGameState(new StateCoverChange(srcOwner, CHANGE_REMOVE, howMany, cards, harm));	
		break;
	default:
		return GE_NOT_SUPPORTED;
	}
	if(updated){
		sendMessage(-1, MSG_GAME, update_info);
	}

	return GE_SUCCESS;
}

int GameGrail::setStateMoveCardsToHand(int srcOwner, int srcArea, int dstOwner, int dstArea, int howMany, vector< int > cards, HARM harm, bool isShown)
{
	return setStateMoveCards(srcOwner, srcArea, dstOwner, dstArea, howMany, cards, harm, isShown);
}

int GameGrail::setStateMoveOneCardToHand(int srcOwner, int srcArea, int dstOwner, int dstArea, int cardID, HARM harm, bool isShown)
{
	vector< int > wrapper(1);
	wrapper[0] = cardID;
	return setStateMoveCards(srcOwner, srcArea, dstOwner, dstArea, 1, wrapper, harm, isShown);
}

int GameGrail::setStateMoveCardsNotToHand(int srcOwner, int srcArea, int dstOwner, int dstArea, int howMany, vector< int > cards, int doerID, int cause, bool isShown)
{
	HARM harm;
	harm.type = HARM_NONE;
	harm.point = howMany;
	harm.srcID = doerID;
	harm.cause = cause;
	return setStateMoveCards(srcOwner, srcArea, dstOwner, dstArea, howMany, cards, harm, isShown);
}

int GameGrail::setStateMoveOneCardNotToHand(int srcOwner, int srcArea, int dstOwner, int dstArea, int cardID, int doerID, int cause, bool isShown)
{
	vector< int > wrapper(1);
	wrapper[0] = cardID;
	HARM harm;
	harm.type = HARM_NONE;
	harm.point = 1;
	harm.srcID = doerID;
	harm.cause = cause;
	return setStateMoveCards(srcOwner, srcArea, dstOwner, dstArea, 1, wrapper, harm, isShown);
}

int GameGrail::setStateChangeMaxHand(int dstID, bool using_fixed, bool fixed, int howmany, int handCardsRange)
{
	PlayerEntity *dst = getPlayerEntity(dstID);

	if (using_fixed)
		dst->setHandCardsMaxFixed(fixed, howmany);
	else
		dst->addHandCardsRange(handCardsRange);
	GameInfo game_info;
	Coder::handcardMaxNotice(dstID, dst->getHandCardMax(), game_info);
	sendMessage(-1, MSG_GAME, game_info);

	HARM harm;
	harm.cause = 0;
	harm.point = 0;
	harm.srcID = dstID;
	harm.type = HARM_NONE;
	return setStateHandOverLoad(dstID, harm);
}

int GameGrail::drawCardsFromPile(int howMany, vector< int > &cards)
{	
	int out[CARDBUF];
	//牌堆耗完
	if(GE_SUCCESS != pile->pop(howMany,out)){
		int temp[CARDSUM];
		int outPtr;
		int pilePtr;
		outPtr = pile->popAll(out);
		pilePtr = discard->popAll(temp);
		pile->push(pilePtr, temp);
		pile->randomize();
		if(!pile->pop(howMany-outPtr,out+outPtr)){
			ztLoggerWrite(ZONE, e_Error, "[Table %d] Running out of cards.", m_gameId);
			throw GE_CARD_NOT_ENOUGH;
		}
	}
	cards = vector< int >(out, out+howMany);
	return GE_SUCCESS;
}

int GameGrail::setStateHandOverLoad(int dstID, HARM harm)
{
	PlayerEntity *dst = getPlayerEntity(dstID);
	if(!dst){
		return GE_INVALID_PLAYERID;
	}
	int overNum = dst->getHandCardNum() - dst->getHandCardMax();
	if (overNum <= 0){
		return GE_SUCCESS;
	}
	setStateStartLoseMorale(overNum, dstID, harm);
	harm.point = overNum;
	harm.cause = CAUSE_OVERLOAD;
	pushGameState(new StateRequestHand(dstID, harm));
	return GE_URGENT;
}

int GameGrail::setStateCoverOverLoad(int dstID)
{
	PlayerEntity *dst = getPlayerEntity(dstID);
	if(!dst){
		return GE_INVALID_PLAYERID;
	}
	int overNum = dst->getCoverCardNum() - dst->getCoverCardMax();
	if (overNum <= 0){
		return GE_SUCCESS;
	}
	HARM move;
	move.type = HARM_NONE;
	move.cause = CAUSE_OVERLOAD;
	move.point = overNum;
	move.srcID = dstID;
	pushGameState(new StateRequestCover(dstID, move));
	return GE_URGENT;
}

int GameGrail::setStateUseCard(int cardID, int dstID, int srcID, bool stay, bool realCard)
{
	network::CardMsg use_card;
	Coder::useCardNotice(cardID, dstID, srcID, use_card, realCard);
	sendMessage(-1, MSG_CARD, use_card);
	if(realCard){	
		return stay ? setStateMoveOneCardNotToHand(srcID, DECK_HAND, dstID, DECK_BASIC_EFFECT, cardID, srcID, CAUSE_USE, true)
			        : setStateMoveOneCardNotToHand(srcID, DECK_HAND, -1, DECK_DISCARD, cardID, srcID, CAUSE_USE, true);			        
	}
	else{
		return GE_SUCCESS;
	}
}

int GameGrail::setStateCheckBasicEffect()
{
	PlayerEntity *player = getPlayerEntity(m_currentPlayerID);
	int weakenCardID = -1;
	int weakenSrcID = -1;
	int howMany = FengYin::WuXiShuFu_Effect(this);
	if( player->checkBasicEffectByName(NAME_WEAKEN, &weakenCardID, &weakenSrcID) == GE_SUCCESS ){
		howMany += 3;				
	}
	if(howMany > 0){
		pushGameState(new StateWeaken(weakenSrcID, howMany));
		setStateMoveOneCardNotToHand(m_currentPlayerID, DECK_BASIC_EFFECT, -1, DECK_DISCARD, weakenCardID, m_currentPlayerID, CAUSE_DEFAULT, true);
	}
	else
	{
		pushGameState(new StateBeforeAction);
	}
	//中毒 push timeline3 states here based on basicEffect
	list<BasicEffect> basicEffects = player->getBasicEffect();
	for(list<BasicEffect>::iterator it = basicEffects.begin(); it!=basicEffects.end(); it++)
	{
		if(getCardByID(it->card)->getName() == NAME_POISON)
		{
			HARM poisonHarm;
			poisonHarm.type = HARM_MAGIC;
			poisonHarm.point = 1;
			poisonHarm.srcID = it->srcUser;
			poisonHarm.cause = CAUSE_POISON;

			setStateTimeline3(m_currentPlayerID, poisonHarm);
			setStateMoveOneCardNotToHand(m_currentPlayerID, DECK_BASIC_EFFECT, -1, DECK_DISCARD, it->card, m_currentPlayerID, CAUSE_DEFAULT, true);
		}
	}
	return GE_SUCCESS;
}

int GameGrail::setStateAttackAction(int cardID, int dstID, int srcID, bool realCard)
{
	pushGameState(new StateAfterAttack(srcID));	
	setStateTimeline1(cardID, dstID, srcID, true);
	pushGameState(new StateBeforeAttack(dstID, srcID));	
	return setStateUseCard(cardID, dstID, srcID, false, realCard);
}

int GameGrail::setStateReattack(int attackFromCard, int attackToCard, int attackFrom, int attacked , int attackTo, bool isActive, bool realCard)
{
	setStateTimeline1(attackToCard, attackTo, attacked, false);
	setStateTimeline2Miss(attackFromCard, attacked, attackFrom, isActive);	
	return setStateUseCard(attackToCard, attackTo, attacked, false, realCard);
}

int GameGrail::setStateAttackGiveUp(int cardID, int dstID, int srcID, HARM harm, bool isActive, bool checkSheild)
{
	PlayerEntity *player = getPlayerEntity(dstID);
	int shieldCardID = -1;
	if(checkSheild && (GE_SUCCESS == player->checkBasicEffectByName(NAME_SHIELD, &shieldCardID)  || player->checkBasicEffectByName(TIAN_SHI_ZHI_QIANG, &shieldCardID) == GE_SUCCESS)){
		setStateTimeline2Miss(cardID, dstID, srcID, isActive);	
		return setStateMoveOneCardNotToHand(dstID, DECK_BASIC_EFFECT, -1, DECK_DISCARD, shieldCardID, dstID, CAUSE_DEFAULT, true);	
	}
	
	else{
		return setStateTimeline2Hit(cardID, dstID, srcID, harm, isActive);
	}
}

int GameGrail::setStateMissileGiveUp(int dstID, int srcID, int harmPoint)
{
	PlayerEntity *player = getPlayerEntity(dstID);
	int shieldCardID = -1;
	if(player->checkBasicEffectByName(NAME_SHIELD, &shieldCardID) == GE_SUCCESS || player->checkBasicEffectByName(TIAN_SHI_ZHI_QIANG, &shieldCardID) == GE_SUCCESS){	
		return setStateMoveOneCardNotToHand(dstID, DECK_BASIC_EFFECT, -1, DECK_DISCARD, shieldCardID, dstID, CAUSE_DEFAULT, true);	
	}	
	else{
		HARM harm;
		harm.type = HARM_MAGIC;
		harm.point = harmPoint;
		harm.srcID = srcID;
		harm.cause = CAUSE_MISSILE;
		return setStateTimeline3(dstID, harm);
	}
}

int GameGrail::setStateTimeline1(int cardID, int dstID, int srcID, bool isActive)
{
	CONTEXT_TIMELINE_1* con = new CONTEXT_TIMELINE_1;
	con->attack.cardID = cardID;
	con->attack.srcID = srcID;
	con->attack.dstID = dstID;
	con->attack.isActive = isActive;
	con->harm.srcID = srcID;
	con->harm.point = 2;
	con->harm.type = HARM_ATTACK;
	con->harm.cause = CAUSE_ATTACK;
	con->hitRate = RATE_NORMAL;
	con->checkShield = true;

	// 暗灭
	if(getCardByID(cardID)->getElement() == ELEMENT_DARKNESS){
		con->hitRate = RATE_NOREATTACK;
	}

	pushGameState(new StateTimeline1(con));
	return GE_SUCCESS;
}

int GameGrail::setStateTimeline2Miss(int cardID, int dstID, int srcID, bool isActive)
{
	CONTEXT_TIMELINE_2_MISS* con = new CONTEXT_TIMELINE_2_MISS;
	con->cardID = cardID;
	con->srcID = srcID;
	con->dstID = dstID;
	con->isActive = isActive;
	pushGameState(new StateTimeline2Miss(con));
	return GE_SUCCESS;
}

int GameGrail::setStateTimeline2Hit(int cardID, int dstID, int srcID, HARM harm, bool isActive)
{
	HitMsg hit_msg;
	if (isActive)
		hit_msg.set_cmd_id(ACTION_ATTACK);
	else
		hit_msg.set_cmd_id(RESPOND_REPLY_ATTACK);
	hit_msg.set_hit(1);
	hit_msg.set_src_id(srcID);
	hit_msg.set_dst_id(dstID);
	sendMessage(-1, MSG_HIT, hit_msg);
	int color = getPlayerEntity(srcID)->getColor();
	GameInfo update_info;
	if (isActive)
	{
		m_teamArea->setGem(color, m_teamArea->getGem(color)+1);
		if (color == RED)
			update_info.set_red_gem(m_teamArea->getGem(color));
		else
			update_info.set_blue_gem(m_teamArea->getGem(color));
	}
	else
	{
		m_teamArea->setCrystal(color, m_teamArea->getCrystal(color)+1);
		if (color == RED)
			update_info.set_red_crystal(m_teamArea->getCrystal(color));
		else
			update_info.set_blue_crystal(m_teamArea->getCrystal(color));
	}
	sendMessage(-1, MSG_GAME, update_info);
	CONTEXT_TIMELINE_2_HIT *con = new CONTEXT_TIMELINE_2_HIT;
	con->attack.cardID = cardID;
	con->attack.dstID = dstID;
	con->attack.srcID = srcID;
	con->attack.isActive = isActive;
	con->harm = harm;
	pushGameState(new StateTimeline2Hit(con));
	return GE_SUCCESS;
}

int GameGrail::setStateTimeline3(int dstID, HARM harm)
{
	CONTEXT_TIMELINE_3 *con = new CONTEXT_TIMELINE_3;
	con->harm = harm;
	con->dstID = dstID;
	
	pushGameState(new StateTimeline3(con));
	return GE_SUCCESS;
}
 //灵魂术士-->【灵魂链接】  added by Tony
int GameGrail::setStateTimeline6(int dstID, HARM harm)
{
	CONTEXT_TIMELINE_6 *con = new CONTEXT_TIMELINE_6;
	con->harm = harm;
	con->dstID = dstID;
	
	pushGameState(new StateTimeline6(con));
	return GE_SUCCESS;
}

int GameGrail::setStateStartLoseMorale(int howMany, int dstID, HARM harm)
{
	if(howMany>0){
		CONTEXT_LOSE_MORALE *con = new CONTEXT_LOSE_MORALE;
		con->dstID = dstID;
		con->harm = harm;
		con->howMany = howMany;
		pushGameState(new StateBeforeLoseMorale(con));
	}
	return GE_SUCCESS;
}

int GameGrail::setStateCheckTurnEnd()
{
	PlayerEntity *player = getPlayerEntity(m_currentPlayerID);

	if(player->hasAdditionalAction())
	{
		pushGameState(new StateAdditionalAction(m_currentPlayerID));
	}
	else
	{
		pushGameState(new StateTurnEnd);
	}
	return GE_SUCCESS;
}

PlayerEntity* GameGrail::getPlayerEntity(int playerID)
{
	if(playerID<0 || playerID>m_maxPlayers){
		ztLoggerWrite(ZONE, e_Error, "[Table %d] Invalid PlayerID: %d", 
					m_gameId, playerID);
		throw GE_INVALID_PLAYERID;
	}
	return m_playerEntities[playerID]; 
}

void GameGrail::GameRun()
{
	ztLoggerWrite(ZONE, e_Information, "GameGrail::GameRun() GameGrail [%d] %s create!!", 
					m_gameId, m_gameName.c_str());
	int ret;
	GrailState* currentState;
	while(processing)
	{
		try{
			ret = GE_NO_STATE;
			currentState = topGameState();
			if(currentState){
				ret = currentState->handle(this);
			}
			if(ret != GE_SUCCESS && ret != GE_TIMEOUT && ret != GE_URGENT){
				ztLoggerWrite(ZONE, e_Error, "[Table %d] Handle returns error: %d. Current state: %d", 
					m_gameId, ret, currentState->state);
			}
		}
		catch(GrailError error)	{
			ztLoggerWrite(ZONE, e_Error, "[Table %d] Handle throws error: %d. Current state: %d", 
				m_gameId, error, topGameState()->state);
		}
		catch(std::exception const& e) {
			ztLoggerWrite(ZONE, e_Error, "[Table %d] Handle throws error: %s. Current state: %d", 
				m_gameId, e.what(), topGameState()->state);
		}
	}
	ztLoggerWrite(ZONE, e_Information, "GameGrail::GameRun() GameGrail [%d] %s end!!", 
					m_gameId, m_gameName.c_str());
	delete this;
}

int GameGrail::playerEnterIntoTable(string userId, int &playerId)
{
	playerId = GUEST;
	for(PlayerContextList::iterator it = m_playerContexts.begin(); it != m_playerContexts.end(); it++)
	{
		if(it->second->getUserId() == userId)
		{
			playerId = it->first;
			return SIT_TABLE_SUCCESS;
		}
	}
	if(!isTableFull())
	{
		int availableID;
		for(availableID = 0; availableID<m_maxPlayers; availableID++)
		{
			if(m_playerContexts.find(availableID) == m_playerContexts.end())
			{
				GameGrailPlayerContext *player = new GameGrailPlayerContext(userId);
				UserTask* session = UserSessionManager::getInstance().getUser(userId);
				player->setName(session->m_nickname);
				m_playerContexts.insert(PlayerContextList::value_type(availableID, player));
				playerId = availableID;
				return SIT_TABLE_SUCCESS;
			}
		}
	}
	//FIXME: limit guset number
	m_guestList.push_back(userId);
	return SIT_TABLE_GUEST;
}

int GameGrail::setStateRoleStrategy()
{
	switch (m_roleStrategy)
	{
	case ROLE_STRATEGY_RANDOM:	
		pushGameState(new StateRoleStrategyRandom);
		break;
	case ROLE_STRATEGY_31:
		pushGameState(new StateRoleStrategy31);
		break;
	case ROLE_STRATEGY_BP:
		//pushGameState(new GrailState(STATE_ROLE_STRATEGY_BP));
		break;
	default:
		return GE_INVALID_ARGUMENT;
	}
	return GE_SUCCESS;
}

int GameGrail::setStateCurrentPlayer(int playerID)
{
	if(playerID<0 || playerID>m_maxPlayers){
		return GE_INVALID_ARGUMENT;
	}
	PlayerEntity *player = getPlayerEntity(playerID);
	if(!player){
		return GE_INVALID_ARGUMENT;
	}
	m_currentPlayerID = playerID;
	if(m_currentPlayerID == m_firstPlayerID)
		m_roundId++;
	player->clearAdditionalAction();

	network::TurnBegin turn_begin;
	turn_begin.set_id(m_currentPlayerID);
	turn_begin.set_round(m_roundId);

	sendMessage(-1, network::MSG_TURN_BEGIN, turn_begin);
	pushGameState(new StateBeforeTurnBegin);
	return GE_SUCCESS;
}

Deck* GameGrail::initRoles()
{
	Deck *roles;
	roles = new Deck(30);
	roles->push(sizeof(SUMMON)/sizeof(int), SUMMON);
	//FIXME: disable random for debug
	roles->randomize();
	return roles;
}

void GameGrail::initPlayerEntities()
{
	int pre;
	int id;
	int post;
	int color;
	int roleID;

	SinglePlayerInfo* player_it;
	int position2id[8];

	for(int i = 0; i < m_maxPlayers; i++){
		player_it = (SinglePlayerInfo*)&(room_info.player_infos().Get(i));
		id = player_it->id();
		roleID = player_it->role_id();
		color = player_it->team();
		//FIXME: 全封印时代
		m_playerEntities[id] = createRole(id, roleID, color);
		m_playerEntities[id]->setRoleID(roleID);
		position2id[i] = id;
	}
	for(int i = 1; i < m_maxPlayers-1; i++){
		id = position2id[i];
		post = position2id[i+1];
		pre = position2id[i-1];
		m_playerEntities[id]->setPost(m_playerEntities[post]);
		m_playerEntities[id]->setPre(m_playerEntities[pre]);
	}
	post = position2id[0];
	id = position2id[m_maxPlayers-1];
	pre = position2id[m_maxPlayers-2];
	m_playerEntities[id]->setPost(m_playerEntities[post]);
	m_playerEntities[id]->setPre(m_playerEntities[pre]);

	post = position2id[1];
	id = position2id[0];
	pre = position2id[m_maxPlayers-1];
	m_playerEntities[id]->setPost(m_playerEntities[post]);
	m_playerEntities[id]->setPre(m_playerEntities[pre]);
	m_teamArea = new TeamArea;

	roleInited = true;
}

void GameGrail::onPlayerEnter(int playerID)
{
	if(!roleInited)
	{
		GameInfo room_info;
		Coder::roomInfo(m_playerContexts, room_info);
		sendMessageExcept(playerID, MSG_GAME, room_info);

		room_info.set_room_id(m_gameId);
		room_info.set_player_id(playerID);
		sendMessage(playerID, MSG_GAME, room_info);
	}
	else{
		//FIXME: reconnect notice?
		GameInfo game_info;
		toProto(game_info);
		game_info.set_player_id(playerID);
		sendMessage(playerID, MSG_GAME, game_info);
	}
}

void GameGrail::onGuestEnter(string userID)
{
	UserTask* session = UserSessionManager::getInstance().getUser(userID);
	if(!session)
		return;
	if(!roleInited)
	{
		GameInfo room_info;
		Coder::roomInfo(m_playerContexts, room_info);
		room_info.set_room_id(m_gameId);
		room_info.set_player_id(GUEST);
		session->sendProto(MSG_GAME, room_info);
	}
	else{
		GameInfo game_info;
		toProto(game_info);
		game_info.set_player_id(GUEST);
		session->sendProto(MSG_GAME, game_info);
	}
}

void GameGrail::onUserLeave(string userID)
{
	m_guestList.remove(userID);
	if(!playing)
	{
		for(PlayerContextList::iterator it = m_playerContexts.begin(); it != m_playerContexts.end(); it++)
		{
			if(it->second->getUserId() == userID)
			{
				delete it->second;
				m_playerContexts.erase(it);
				GameInfo game_info;
				Coder::roomInfo(m_playerContexts, game_info);
				sendMessage(-1, MSG_GAME, game_info);	
				break;
			}
		}
	}
	else{
		for(PlayerContextList::iterator it = m_playerContexts.begin(); it != m_playerContexts.end(); it++)
		{
			if(it->second->getUserId() == userID)
			{
				it->second->setConnect(false);
				Error error;
				Coder::errorMsg(GE_DISCONNECTED, it->first, error);
				sendMessage(-1, MSG_ERROR, error);	
				break;
			}
		}
		for(PlayerContextList::iterator it = m_playerContexts.begin(); it != m_playerContexts.end(); it++)
		{
			if(it->second->isConnected())
				return;
		}
		GameManager::getInstance().deleteGame(GAME_TYPE_GRAIL, m_gameId);
	}
}

void GameGrail::toProto(GameInfo& game_info)
{
	game_info.set_room_id(m_gameId);
	game_info.set_blue_crystal(m_teamArea->getCrystal(0));
	game_info.set_blue_gem(m_teamArea->getGem(0));
	game_info.set_blue_grail(m_teamArea->getCup(0));
	game_info.set_blue_morale(m_teamArea->getMorale(0));

	game_info.set_red_crystal(m_teamArea->getCrystal(1));
	game_info.set_red_gem(m_teamArea->getGem(1));
	game_info.set_red_grail(m_teamArea->getCup(1));
	game_info.set_red_morale(m_teamArea->getMorale(1));

	game_info.set_pile(pile->get_size());
	game_info.set_discard(discard->get_size());
	game_info.set_is_started(true);

	for(int i = 0; i < m_maxPlayers; i++)
	{
		SinglePlayerInfo* player_info = game_info.add_player_infos();
		//按座次顺序
		int playerId = room_info.player_infos(i).id();
		PlayerEntity* player = getPlayerEntity(playerId);
		player->toProto(player_info);
		player_info->set_nickname(m_playerContexts[playerId]->getName());
	}

}

