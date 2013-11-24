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
using namespace boost;

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

GameGrail::GameGrail(GameGrailConfig *config)
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
	string msg;
	proto_encoder(proto_type, proto, msg);

	PlayerContextList::iterator it;
	if(id == -1){
		for(it = m_playerContexts.begin(); it !=m_playerContexts.end(); it++){
			ref = UserSessionManager::getInstance().getUser(it->second->getUserId());
			if(ref){
				ref->SendCmd(msg);
			}
			else{
				ztLoggerWrite(ZONE, e_Debug, "[Table %d] Cannot find UserSession, PlayerID: %d, UserID: %s", m_gameId, id, it->second->getUserId().c_str());
			}
		}
	}
	else{
		it = m_playerContexts.find(id);
		if(it != m_playerContexts.end()){
			ref = UserSessionManager::getInstance().getUser(it->second->getUserId());
			if(ref){
				ref->SendCmd(msg);
			}
			else{
				ztLoggerWrite(ZONE, e_Debug, "[Table %d] Cannot find UserSession, PlayerID: %d, UserID: %s", m_gameId, id, it->second->getUserId().c_str());
			}
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
	//check whether exists

	GameInfo update_info;

	switch(srcArea)
	{
	case DECK_PILE:
		drawCardsFromPile(howMany, cards);
		update_info.set_pile(pile->get_size());
		break;
	case DECK_HAND:
		src = getPlayerEntity(srcOwner);
		if(GE_SUCCESS != (ret=src->checkHandCards(howMany,cards))){
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
	default:
		return GE_NOT_SUPPORTED;
	}
	//src hand change->show hand->dst hand change->dst hand overload, but stack is LIFO
	switch(dstArea)
	{
	case DECK_DISCARD:
		ret = discard->push(howMany, &cards[0]);
		update_info.set_discard(discard->get_size());
		break;
	case DECK_HAND:
		pushGameState(new StateHandChange(dstOwner, CHANGE_ADD, howMany, cards, harm));							
		break;
	case DECK_BASIC_EFFECT:
		pushGameState(new StateBasicEffectChange(dstOwner, CHANGE_ADD, cards[0], harm.srcID, harm.cause));	
		break;
	case DECK_COVER:
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
	default:
		return GE_NOT_SUPPORTED;
	}

	sendMessage(-1, MSG_GAME, update_info);

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

//Must ensure it is called through last line and must push the nextState first
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

int GameGrail::setStateUseCard(int cardID, int dstID, int srcID, bool stay, bool realCard)
{
	network::UseCard use_card;
	Coder::useCardNotice(cardID, dstID, srcID, use_card, realCard);
	sendMessage(-1, MSG_USE_CARD, use_card);
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
	//中毒 push timeline3 states here based on basicEffect by Fengyu
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
	// FIX_ME  没有检查天使之墙 check sheild here by Fengyu

	PlayerEntity *player = getPlayerEntity(dstID);
	int shieldCardID = -1;
	if(checkSheild && GE_SUCCESS == player->checkBasicEffectByName(NAME_SHIELD, &shieldCardID)){
		setStateTimeline2Miss(cardID, dstID, srcID, isActive);	
		return setStateMoveOneCardNotToHand(dstID, DECK_BASIC_EFFECT, -1, DECK_DISCARD, shieldCardID, dstID, CAUSE_DEFAULT, true);	
	}
	
	else{
		return setStateTimeline2Hit(cardID, dstID, srcID, harm, isActive);
	}
}

int GameGrail::setStateMissileGiveUp(int dstID, int srcID, int harmPoint)
{
	// FIX_ME  没有检查天使之墙 check sheild here by Fengyu
	PlayerEntity *player = getPlayerEntity(dstID);
	int shieldCardID = -1;
	if(player->checkBasicEffectByName(NAME_SHIELD, &shieldCardID) == GE_SUCCESS){	
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

	//FIXME: 暗灭 by Fengyu 
	if(getCardByID(cardID)->getElement() == ELEMENT_DARKNESS){
		con->hitRate = RATE_NOREATTACK;
	}
	////

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
	network::HurtMsg hurt_msg;
	Coder::hurtNotice(dstID, harm.srcID, harm.type, harm.point, harm.cause, hurt_msg);
	sendMessage(-1, MSG_HURT, hurt_msg);

	CONTEXT_TIMELINE_3 *con = new CONTEXT_TIMELINE_3;
	con->harm = harm;
	con->dstID = dstID;
	
	pushGameState(new StateTimeline3(con));
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
	while(true)
	{
		try{
			ret = GE_NO_STATE;
			currentState = topGameState();
			if(currentState){
				ret = currentState->handle(this);
			}
			if(ret != GE_SUCCESS){
				ztLoggerWrite(ZONE, e_Error, "[Table %d] Handle returns error: %d. Current state: %d", 
					m_gameId, ret, currentState->state);
			}
		}
		catch(GrailError error)	{
			ztLoggerWrite(ZONE, e_Error, "[Table %d] Handle throws error: %d. Current state: %d", 
				m_gameId, error, topGameState()->state);
		}
	}
}

int GameGrail::playerEnterIntoTable(GameGrailPlayerContext *player)
{
	if(isCanSitIntoTable())
	{
		int availableID;
		for(availableID=0;availableID<m_maxPlayers;availableID++)
		{
			if(m_playerContexts.find(availableID) == m_playerContexts.end())
			{
				UserSession *ref = UserSessionManager::getInstance().getUser(player->getUserId());
				ref->setPlayerID(availableID);
				m_playerContexts.insert(PlayerContextList::value_type(availableID, player));
				SingleRoom single_room;
				single_room.set_player_id(availableID);
				sendMessage(availableID, MSG_SINGLE_ROOM, single_room);
				return 0;
			}
		}
	}
	return 1;
}

int GameGrail::setStateRoleStrategy()
{
	switch (m_roleStrategy)
	{
	case ROLE_STRATEGY_RANDOM:	
		pushGameState(new StateRoleStrategyRandom);
		break;
	case ROLE_STRATEGY_31:
		pushGameState(new GrailState(STATE_ROLE_STRATEGY_31));
		break;
	case ROLE_STRATEGY_BP:
		pushGameState(new GrailState(STATE_ROLE_STRATEGY_BP));
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
	player->clearAdditionalAction();

	network::TurnBegin turn_begin;
	turn_begin.set_id(m_currentPlayerID);
	sendMessage(-1, network::MSG_TURN_BEGIN, turn_begin);
	pushGameState(new StateBeforeTurnBegin);
	return GE_SUCCESS;
}

Deck* GameGrail::initRoles()
{
	Deck *roles;
	roles = new Deck(30);
	roles->init(1, 24);
	//int temp[]={26, 28, 29};
	//roles->push(3, temp);
	//FIXME: disable random for debug
	//roles->randomize();
	return roles;
}

void GameGrail::initPlayerEntities()
{
	int pre;
	int id;
	int post;
	int color;
	if(m_maxPlayers<2){
		ztLoggerWrite(ZONE, e_Error, "[Table %d] maxPlayers: %d must be at least 2", 
					m_gameId, m_maxPlayers);
		return;
	}
	//FIXME should init roles instead of playerEntity

	//google::protobuf::RepeatedPtrField<SinglePlayerInfo>::iterator player_it = game_info.player_infos().begin();
	SinglePlayerInfo* player_it;
	int position2id[8];

	for(int i = 0; i < m_maxPlayers; i++){
		player_it = (SinglePlayerInfo*)&(game_info.player_infos().Get(i));
		id = player_it->id();
		color = player_it->team();
		//FIXME: 全封印时代
		m_playerEntities[id] = new MaoXian(this, id, color);
		
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
}
/*
int GameGrail::handleRoleStrategy31(GrailState *state)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter handleRoleStrategy31", m_gameId);
	CONTEXT_BROADCAST* con;
	if(!state->context){
		Deck *roles = initRoles();
		con = new CONTEXT_BROADCAST;
		int out[3];
		for(int i = 0; i < m_maxPlayers; i++){
			if(roles->pop(3, out)){
				con->msgs[i] = Coder::askForRolePick(3, out);
			}
			else{
				return GE_INVALID_ARGUMENT;
			}
		}
		state->context = con;
		delete roles;
	}
	con = (CONTEXT_BROADCAST*)state->context;
	if(waitForAll(con->msgs)){
		//TODO
		;
	}
	else{
		return GE_TIMEOUT;
	}
}

*/

