#pragma once

#include <iostream>
#include <list>
#include <stack>
#include "Game.h"
#include "GameGrailPlayerContext.h"
#include "PlayerEntity.h"
#include "GameGrailCommon.h"
#include <boost/random.hpp>
#include <boost/thread/condition_variable.hpp>
#include "codec.h"
#include "StatisticData.h"
using namespace std;

#define GUEST 9


class GrailState;
class PlayerEntity;
class Deck
{
public:
	Deck(int s){
		size = s;
		items = new int[size];
		iterator = -1;
		rng.seed((const boost::random::rand48::result_type)time(0));
	}
	~Deck(){
		delete[] items;
	}
	int init(int min, int max){
		if(size < max - min + 1)
			return GE_DECK_OVERFLOW;
		for(int i = 0; i <= max - min; i++)
			items[i] = i + min;
		iterator = max - min;
		return GE_SUCCESS;
	}
	int pop(int howMany, int* out){
		if (iterator + 1 < howMany)
			return GE_DECK_OVERFLOW;
		memcpy(out, items + iterator - howMany + 1, howMany*sizeof(int));
		iterator -= howMany;
		return GE_SUCCESS;
	}
	int push(int howMany, const int* in){
		if (size - iterator - 1 < howMany)
			return GE_DECK_OVERFLOW;
		int begin = iterator+1;
		memcpy(items + begin, in, howMany*sizeof(int));
		iterator += howMany;
		return GE_SUCCESS;
	}
	int popAll(int *out){
		int howMany = iterator+1;
		memcpy(out, items, howMany*sizeof(int));
		iterator = -1;
		return howMany;
	}
	void randomize(){
		int chosen;
		int temp;
		
		for(int i = 0; i <= iterator; i++){
			chosen = rng() % (iterator + 1);
			temp = items[i];
			items[i] = items[chosen];
			items[chosen] = temp;
		}
	}
	int get_size() {
		return iterator+1;
	}
	bool deleteOne(int id){
		int pos = -1;
		for(int i = iterator;i >=0;i--)
		{
			if (items[i]==id)
			{
				pos = i;
				break;
			}
		}
		if(pos != -1)
		{
			for(int i=pos;i < iterator;i++)
			{
				items[i] = items[i+1];
			}
			iterator--;
			return true;
		}
		else
			return false;
	}
private:
	boost::random::rand48 rng;
	int *items;
	int iterator;
	int size;
};

class TeamArea
{
public:
    TeamArea(){this->initialTeam();}
    void initialTeam();
    void setMorale(int color,int value);
    void setGem(int color,int value);
    void setCrystal(int color,int value);
    void setCup(int color,int value);
    int getMorale(int color){return (color == RED)?moraleRED:moraleBLUE;}
    int getGem(int color){return (color == RED)?gemRED:gemBLUE;}
    int getCrystal(int color){return (color == RED)?crystalRED:crystalBLUE;}
    int getCup(int color){return (color == RED)?cupRED:cupBLUE;}
	int getEnergy(int color) { return (color == RED) ? gemRED+crystalRED : gemBLUE+crystalBLUE; }
private:

    int moraleRED,moraleBLUE;
    int gemRED,gemBLUE;
    int crystalRED,crystalBLUE;
    int cupRED,cupBLUE;
};

class GameGrailConfig : public GameConfig
{
public:
	GameGrailConfig(int maxPlayers, int roleStrategy, bool firstExtension, bool secondExtension, bool spMoDao, int seatMode, bool silence): 
	  maxPlayers(maxPlayers), roleStrategy(roleStrategy), firstExtension(firstExtension), secondExtension(secondExtension),spMoDao(spMoDao), seatMode(seatMode), silence(silence) 
	{}
	~GameGrailConfig() {}
	int maxPlayers;
	int roleStrategy;
	bool spMoDao;
	bool firstExtension;
	bool secondExtension;
	int seatMode;
	bool silence;
};

class GameGrail : public Game
{
public:
	bool playing;
	bool dead;
	bool roleInited;

	int m_roleStrategy;	
	int m_seatMode;
	bool m_silence;
	int m_maxAttempts;
	int m_firstPlayerID;
	int m_currentPlayerID;
	bool m_spMoDao;
	bool m_firstExtension;
	bool m_secondExtension;
	GameInfo room_info;
	list< int > teamA, teamB;
	tableLogData m_tableLog;

protected:
	int m_roundId;
	int m_maxPlayers;
	int m_token;
	int m_responseTime;	

	boost::mutex m_mutex_for_wait;
	boost::condition_variable m_condition_for_wait;	
	PlayerContextList m_playerContexts;
	
	list< string > m_guestList;
	typedef map< int, PlayerEntity* > PlayerEntityList;
    PlayerEntityList m_playerEntities;
	typedef stack< GrailState* > StateStack;
	StateStack  m_states;
	TeamArea* m_teamArea;
	Deck *pile, *discard;
	bool m_ready[MAXPLAYER];

	
public:
	GameGrail(GameGrailConfig *config);
	~GameGrail();
	void sendMessage(int id, uint16_t proto_type, google::protobuf::Message& proto);
	void sendMessageExcept(int id, uint16_t proto_type, google::protobuf::Message& proto);
	int playerEnterIntoTable(string userId, string nickname, int& playerId);
	int guestEnterIntoTable(string userId);
	std::string getUserId(int n){ 
		PlayerContextList::iterator it = m_playerContexts.find(n);
		return it->second->getUserId();
	}
	GrailState* topGameState() { return m_states.empty()? throw GE_NO_STATE : m_states.top(); }
	void pushGameState(GrailState* state) { m_states.push(state); }
	void popGameState() { 
		 SAFE_DELETE(m_states.top()); 
		 m_states.pop(); 
	}
	int popGameState_if(int state);	

	int getGameMaxPlayers() const { return m_maxPlayers; }
	int getGameNowPlayers() { 
		int count = 0;
		for(PlayerContextList::iterator it = m_playerContexts.begin(); it != m_playerContexts.end(); it++){
			if(it->second->isConnected())
				count++;
		}
		return count;
	}
	int getCurrentPlayerID() const { return m_currentPlayerID; }
	PlayerEntity* getPlayerEntity(int id);
	PlayerEntity* getNextPlayerEntity(PlayerEntity* current, int iterator, int step);
	SinglePlayerInfo* getPlayerInfo(int id);
	GameGrailPlayerContext* getPlayerContext(int id);

	TeamArea* getTeamArea() { return m_teamArea; }

	void resetReady(int id = -1){		
		if(id<-1 || id>m_maxPlayers){
			return;
		}
		if(id == -1){
			memset(m_ready, 0, sizeof(m_ready));
		}
		else{
			memset(m_ready,1,sizeof(m_ready));
			m_ready[id] = 0;
		}
	}	
	void setStartReady(int id, bool ready){	
		if(id<0 || id>=m_maxPlayers){
			return;
		}
		PlayerContextList::iterator it = m_playerContexts.find(id);
		if(it != m_playerContexts.end()){
			it->second->setReady(ready);
			GameInfo update;
			SinglePlayerInfo *player = update.add_player_infos();
			player->set_id(id);
			player->set_ready(ready);
			sendMessage(-1, MSG_GAME, update);
		}
	}
	void setTeam(int id, int team){
		if(id<-1 || id>m_maxPlayers){
			return;
		}
		teamA.remove(id);
		teamB.remove(id);
		GameInfo update;
		SinglePlayerInfo *player = update.add_player_infos();
		player->set_id(id);
		if(team == JoinTeamRequest_Team_TEAM_A && teamA.size() < m_maxPlayers/2){
			teamA.push_back(id);
			player->set_team(team);
			sendMessage(-1, MSG_GAME, update);
		}
		else if(team == JoinTeamRequest_Team_TEAM_B && teamB.size() < m_maxPlayers/2){
			teamB.push_back(id);
			player->set_team(team);
			sendMessage(-1, MSG_GAME, update);
		}
		else if(team == JoinTeamRequest_Team_TEAM_RANDOM){
			player->set_team(team);
			sendMessage(-1, MSG_GAME, update);
		}		
	}

	bool isAllStartReady(){
		for(PlayerContextList::iterator it = m_playerContexts.begin(); it != m_playerContexts.end(); it++)
		{
			if(!it->second->isReady())
				return false;
		}
		return true;
	}

	bool isReady(int id);
	bool waitForOne(int id, uint16_t proto_type, google::protobuf::Message& proto, int timeout, bool resetReady = true);
	bool waitForOne(int id, uint16_t proto_type, google::protobuf::Message& proto, bool toResetReady = true) { return waitForOne(id, proto_type, proto, m_responseTime, toResetReady); }
	bool waitForAll(uint16_t proto_type, void** proto_ptrs, int timeout, bool toResetReady = true);
	bool waitForAll(uint16_t proto_type, void** proto_ptrs, bool toResetReady = true) { return waitForAll(proto_type, proto_ptrs, m_responseTime, toResetReady); }
	bool falseNotify(int id);
	bool tryNotify(int id, int state, int step = 0, void* reply = NULL);
	int getReply(int id, void* &reply);
	
	int drawCardsFromPile(int howMany, vector< int > &cards);
	//setState前缀的函数涉及插入状态
	//底层API原则上不直接调用
	int setStateMoveCards(int srcOwner, int srcArea, int dstOwner, int dstArea, int howMany, vector< int > cards, HARM harm, bool isShown);
	//移牌至手上，需提供HARM，若从摸牌堆上移出，cards不用赋值
	int setStateMoveCardsToHand(int srcOwner, int srcArea, int dstOwner, int dstArea, int howMany, vector< int > cards, HARM harm, bool isShown);
	//上面的简化，移1牌至手上，需提供HARM，若从摸牌堆上移出，cards不用赋值
	int setStateMoveOneCardToHand(int srcOwner, int srcArea, int dstOwner, int dstArea, int cardID, HARM harm, bool isShown);
	//移牌至手上以外的地方，需提供原因，若从摸牌堆上移出，cards不用赋值
	int setStateMoveCardsNotToHand(int srcOwner, int srcArea, int dstOwner, int dstArea, int howMany, vector< int > cards, int doerID, int cause, bool isShown);
	//上面的简化，移1牌至手上以外的地方，需提供原因，若从摸牌堆上移出，cards不用赋值
	int setStateMoveOneCardNotToHand(int srcOwner, int srcArea, int dstOwner, int dstArea, int cardID, int doerID, int cause, bool isShown);

	int setStateUseCard(int cardID, int dstID, int srcID, bool stay = false, bool realCard = true);
	int setStateChangeMaxHand(int dstID, bool using_fixed, bool fixed = false, int howmany = 6, int handCardsRange = 0);
	int setStateHandOverLoad(int dstID, HARM harm);
	int setStateCoverOverLoad(int dstID);
	int setStateCheckBasicEffect();
	int setStateAttackAction(int cardID, int dstID, int srcID, bool realCard = true);
	int setStateReattack(int attackFromCard, int attackToCard, int attackFrom, int attacked , int attackTo, bool isActive, bool realCard = true);
	int setStateAttackGiveUp(int cardID, int dstID, int srcID, HARM harm, bool isActive, bool checkSheild = true);
	int setStateMissileGiveUp(int dstID, int srcID, int harmPoint);
	int setStateTimeline1(int cardID, int dstID, int srcID, bool isActive);
	int setStateTimeline2Miss(int cardID, int dstID, int srcID, bool isActive);
	int setStateTimeline2Hit(int cardID, int dstID, int srcID, HARM harm, bool isActive);
	int setStateTimeline3(int dstID, HARM harm);
	int setStateTimeline6(int dstID, HARM harm); //added by Tony
	int setStateStartLoseMorale(int howMany, int dstID, HARM harm);
    int setStateRoleStrategy();
	int setStateCurrentPlayer(int playerID);
	int setStateCheckTurnEnd();

	Deck* initRoles();
	PlayerEntity* createRole(int playerID, int roleID, int color);
	void initPlayerEntities();
	void initDecks(){
		pile = new Deck(CARDSUM);
		pile->init(0,CARDSUM-1);
		pile->randomize();
		discard = new Deck(CARDSUM);
	}
	
	void onPlayerEnter(int playerID);
	void onGuestEnter(string userID);
	void onUserLeave(string userID);
	void toProtoAs(int playerId, GameInfo& game_info);
	bool isTableFull() { return getGameNowPlayers() >= m_maxPlayers; }	
	tableLogData getTableLog(){ return m_tableLog; }
protected:	
	void GameRun();  
};
