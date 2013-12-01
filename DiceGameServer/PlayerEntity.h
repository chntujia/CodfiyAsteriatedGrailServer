#ifndef PLAYERENTITY_H
#define PLAYERENTITY_H
#include "CardEntity.h"
#include "GameGrailCommon.h"
#include "GrailState.h"
#include <list>
#include <vector>

using namespace std;

class TeamArea;
class GameGrail;
class UserTask;
//玩家类

class PlayerEntity
{
public:
    PlayerEntity(GameGrail *engine, int id, int color);
	//在该玩家前增加基础效果
    int addBasicEffect(int effectCard, int srcUserID=-1);
    //移除基础效果
    int removeBasicEffect(int card);
	//检查基础效果
	int checkBasicEffectByCard(int card);
	//检查指定名称的基础效果牌是否存在,cardID和src保存找到的牌ID和施放者
	int checkBasicEffectByName(int name, int* cardID = NULL, int* src = NULL);
	//检查指定独有技的基础效果牌是否存在,cardID和src保存找到的牌ID和施放者
	int checkBasicEffectBySpeciality(int speciality, int* cardID = NULL, int* src = NULL);
	//专属
	int checkExclusiveEffect(int exclusive);
	void addExclusiveEffect(int exclusive);
    void removeExclusiveEffect(int exclusive);
	bool* getExclusiveEffect() { return exclusiveEffects; }
    //增加手牌操作
    int addHandCards(int howMany, vector< int > newCard);
    //移除手牌操作
    int removeHandCards(int howMany, vector<int> oldCard);
	//检查手牌
	int checkHandCards(int howMany, vector<int> cards);
	int checkOneHandCard(int cardID);
	//盖牌
	int addCoverCards(int howMany, vector< int > cards);
	int removeCoverCards(int howMany, vector< int > cards);
    int checkCoverCards(int howMany, vector<int> cards);
	int checkOneCoverCard(int cardID);
    //设定手牌上限是否锁定
    void setHandCardsMaxFixed(bool fixed, int howmany=6);
    //设置手牌变化
    void addHandCardsRange(int howMany);
    //设置治疗
    void addCrossNum(int howMany, int atMost=-1);
    void subCrossNum(int howMany);

    void setGem(int howMany);
    void setCrystal(int howMany);
    //设置当前回合是否为该玩家回合
    void setYourTurn(bool yes);
    void setSeatNum(int num){this->seatNum = num;}
    int getSeatNum(){return this->seatNum;}
    //设置横置
    void setTap(bool tap){this->tap = tap;}
    void setToken(int id,int howMany){if(howMany<0)howMany=0;token[id]=howMany<=tokenMax[id]?howMany:tokenMax[id];}
    //设置下一个玩家
    void setPost(PlayerEntity* nextPlayer){this->postPlayer = nextPlayer;}
	void setPre(PlayerEntity* pre){ prePlayer = pre; }
    int getID();
    string getName();
    int getHandCardMax();
    int getHandCardNum();
    int getCoverCardMax() { return coverCardsMax; }
    int getCoverCardNum() { return coverCards.size(); }
    int getCrossNum();
    int getCrossMax();
    int getGem();
    int getCrystal();
    int getEnergy();
	int getEnergyMax() { return energyMax; }
    int getColor();
    list< int >  getCoverCards(){return this->coverCards;}
    int getToken(int id){return token[id];}
    //获取该玩家面前的基础效果牌
    list< struct BasicEffect > getBasicEffect(){return this->basicEffects;}

    PlayerEntity* getPost(){ return this->postPlayer;}
	PlayerEntity* getPre(){ return prePlayer; }
    list< int > getHandCards(){return this->handCards;}
    int getRoleID(){return characterID;}
    bool tapped(){return this->tap;}
    bool isHandCardsMaxFixed(){return this->handCardsMaxFixed;}
    bool getYourturn();
	bool containsAction(int cause);
	bool hasAdditionalAction() {return !additionalActions.empty();}
	list<ACTION_QUOTA> getAdditionalAction() { return additionalActions; }
	void clearAdditionalAction() { additionalActions.clear(); }
	void addAction(int allowAction, int cause) {
		ACTION_QUOTA quota;
		quota.allowAction = allowAction;
		quota.cause = cause;
		additionalActions.push_back(quota);
	}
	
	bool toNextStep(int ret) {	return GE_SUCCESS == ret || GE_TIMEOUT == ret; }
	//解析角色相关的命令
	//return true 表示处理了
	virtual bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto){ return false; }

	//回合限定等统一在这里初始化
	virtual int p_before_turn_begin(int &step, int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_turn_begin(int &step, int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_before_action(int &step, int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_boot(int &step, int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_before_attack(int &step, int dstID, int srcID) { return GE_SUCCESS; }
	virtual int p_after_attack(int &step, int playerID) { return GE_SUCCESS; }
	virtual int p_before_magic(int &step, int srcID) { return GE_SUCCESS; }
	virtual int p_after_magic(int &step, int srcID) { return GE_SUCCESS; }
	virtual int p_before_special(int &step, int srcID) { return GE_SUCCESS; }
	virtual int p_after_special(int &step, int srcID) { return GE_SUCCESS; }
	virtual int p_turn_end(int &step, int playerID) { return GE_SUCCESS; }
	virtual int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con) { return GE_SUCCESS; }
	virtual int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con) { return GE_SUCCESS; }
	virtual int p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con) { return GE_SUCCESS; }
	virtual int p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con) { return GE_SUCCESS; }
	virtual int p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con) { return GE_SUCCESS; }
	virtual int p_timeline_5(int &step, CONTEXT_TIMELINE_5 *con) { return GE_SUCCESS; }
	virtual int p_timeline_6(int &step, CONTEXT_TIMELINE_6 *con) { return GE_SUCCESS; }
	virtual int p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con) { return GE_SUCCESS; }
	virtual int p_before_lose_morale(int &step, CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_lose_morale(int &step, CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_fix_morale(int &step, CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_true_lose_morale(int &step, CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_hand_change(int &step, int playerID) { return GE_SUCCESS; }
	virtual int p_basic_effect_change(int &step, int dstID, int card, int doerID, int cause)  { return GE_SUCCESS; }
    virtual int p_cover_change(int &step, int dstID, int howMany, vector<int> cards, int doerID, int cause)  { return GE_SUCCESS; }
	virtual int p_show_hand(int &step, int playerID, int howMany, vector<int> cards, HARM harm) { return GE_SUCCESS; }
	virtual int p_additional_action(int chosen);
	virtual int p_attack_skill(int &step, Action *action) { return GE_EMPTY_HANDLE; }
	virtual int p_magic_skill(int &step, Action *action) { return GE_EMPTY_HANDLE; }
	virtual int p_special_skill(int &step, Action *action) { return GE_EMPTY_HANDLE; }
	virtual int p_request_hand_give_up(int &step, int targetID, int cause) { return GE_SUCCESS; }
    virtual int p_request_cover_give_up(int &step, int targetID, int cause) { return GE_SUCCESS; }

	virtual int v_allow_action(int claim, int allow, bool canGiveUp);
	virtual int v_attack(int cardID, int dstID, bool realCard = true);
	virtual int v_attacked() { return GE_SUCCESS; }
	virtual int v_reattack(int cardID, int orignCardID, int dstID, int orignID, int rate, bool realCard = true);
	virtual int v_missile(int cardID, int dstID, bool realCard = true);
	virtual int v_remissile(int cardID, bool realCard = true);
	virtual int v_block(int cardID);
	virtual int v_shield(int cardID, PlayerEntity* dst);
	virtual int v_weaken(int cardID, PlayerEntity* dst);
	virtual int v_buy(Action *action);
	virtual int v_synthesize(Action *action, TeamArea* team);
	virtual int v_extract(Action *action, TeamArea* team);
	virtual int v_additional_action(int chosen);
	virtual int v_attack_skill(Action *action) { return GE_EMPTY_HANDLE; }
	virtual int v_magic_skill(Action *action) { return GE_EMPTY_HANDLE; }
	virtual int v_special_skill(Action *action) { return GE_EMPTY_HANDLE; }
	virtual int v_request_hand(int howMany, vector<int> cards, HARM harm) { return GE_SUCCESS; }
    virtual int v_request_cover(int howMany, vector<int> cards, HARM harm) { return GE_SUCCESS; }
protected:
    int id;//玩家id
    int characterID;
    string name;
    int handCardsMax;
    int handCardsRange;
    int handCardsMin;//蝶舞生命之火使用
    int coverCardsMax;
    int crossNum;
    int crossMax;
    int gem;
    int crystal;
    int energyMax;
    int color;
    float star;
    bool tap;//横置状态
    bool handCardsMaxFixed;//是否锁定手牌上限
    bool yourTurn;
    int seatNum;
    int token[3];
    int tokenMax[3];
	PlayerEntity* prePlayer;//上家
    PlayerEntity* postPlayer;//下家
    TeamArea* teamArea;
    list< int > handCards;//手牌
    list< BasicEffect > basicEffects;//基础效果牌
    bool exclusiveEffects[EXCLUSIVE_NUM];//专属效果
    list< int > coverCards;//盖牌区
	list< ACTION_QUOTA > additionalActions;
	GameGrail *engine;
};

#endif // PLAYERENTITY_H
