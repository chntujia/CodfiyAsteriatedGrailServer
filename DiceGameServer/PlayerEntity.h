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
//玩家类


class PlayerEntity
{
public:
    PlayerEntity(GameGrail *engine, int id, int color);
	//在该玩家前增加基础效果
    int addBasicEffect(int effectCard,int srcUserID=-1);
    //移除基础效果
    int removeBasicEffect(int card);
	//检查基础效果
	int checkBasicEffect(int card);
	//检查指定名称的基础效果牌是否存在,cardID和src保存找到的牌ID和施放者
	int checkBasicEffectName(int name,int* cardID, int* src = NULL);
    //增加手牌操作
    int addHandCards(int howMany, vector< int > newCard);
    //移除手牌操作
    int removeHandCards(int howMany, vector<int> oldCard);
	//检查手牌
	int checkHandCards(int howMany, vector<int> oldCard);
	int checkOneHandCard(int cardID);
	//盖牌
	int addCoverCards(int howMany, vector< int > cards);
	int removeCoverCards(int howMany, vector< int > cards);
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
    int getCrossNum();
    int getCrossMax();
    int getGem();
    int getCrystal();
    int getEnergy();
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
	bool hasAdditionalAction() {return !additionalActions.empty();}
	void clearAdditionalAction() { additionalActions.clear(); }
	virtual int p_before_turn_begin(int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_turn_begin(int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_before_action(int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_boot(int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_before_attack(int dstID, int srcID) { return GE_SUCCESS; }
	virtual int p_after_attack(int playerID) { return GE_SUCCESS; }
	virtual int p_before_magic(int srcID) { return GE_SUCCESS; }
	virtual int p_after_magic(int srcID) { return GE_SUCCESS; }
	virtual int p_turn_end(int playerID) { return GE_SUCCESS; }
	virtual int p_timeline_1(CONTEXT_TIMELINE_1 *con) { return GE_SUCCESS; }
	virtual int p_timeline_2_hit(CONTEXT_TIMELINE_2_HIT *con) { return GE_SUCCESS; }
	virtual int p_timeline_2_miss(CONTEXT_TIMELINE_2_MISS *con) { return GE_SUCCESS; }
	virtual int p_timeline_3(CONTEXT_TIMELINE_3 *con) { return GE_SUCCESS; }
	virtual int p_timeline_4(CONTEXT_TIMELINE_4 *con) { return GE_SUCCESS; }
	virtual int p_timeline_5(CONTEXT_TIMELINE_5 *con) { return GE_SUCCESS; }
	virtual int p_timeline_6(CONTEXT_TIMELINE_6 *con) { return GE_SUCCESS; }
	virtual int p_timeline_6_drawn(CONTEXT_TIMELINE_6_DRAWN *con) { return GE_SUCCESS; }
	virtual int p_before_lose_morale(CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_lose_morale(CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_fix_morale(CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_true_lose_morale(CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_hand_change(int playerID) { return GE_SUCCESS; }
	virtual int p_basic_effect_change(int dstID, int card, int doerID, int cause)  { return GE_SUCCESS; }
	virtual int p_show_hand(int playerID, int howMany, vector<int> cards) { return GE_SUCCESS; }

	static bool is_allow_action(int claim, int allow, bool canGiveUp);
	virtual int v_attack(int cardID, int dstID, bool realCard = true);
	virtual int v_reattack(int cardID, int orignCardID, int dstID, int orignID, bool realCard = true);
	virtual int v_missile(int cardID, int dstID, bool realCard = true);
	virtual int v_remissile(int cardID, bool realCard = true);
	virtual int v_block(int cardID);
protected:
    int id;//玩家id
    int characterID;
    string name;
    int handCardsMax;
    int handCardsRange;
    int handCardsMin;//蝶舞生命之火使用
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
    list< int > exclusiveEffect;//专属效果
    list< int > coverCards;//盖牌区
	list< ACTION_QUOTA > additionalActions;
	GameGrail *engine;
};

#endif // PLAYERENTITY_H
