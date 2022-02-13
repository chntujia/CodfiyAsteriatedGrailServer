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
//�����

class PlayerEntity
{
public:
    PlayerEntity(GameGrail *engine, int id, int color);
	const char* type() const { return typeid(*this).name(); }
	//�ڸ����ǰ���ӻ���Ч��
    int addBasicEffect(int effectCard, int srcUserID=-1);
    //�Ƴ�����Ч��
    int removeBasicEffect(int card);
	//������Ч��
	int checkBasicEffectByCard(int card);
	//���ָ�����ƵĻ���Ч�����Ƿ����,cardID��src�����ҵ�����ID��ʩ����
	int checkBasicEffectByName(int name, int* cardID = NULL, int* src = NULL);
	//ר��
	int checkExclusiveEffect(int exclusive);
	void addExclusiveEffect(int exclusive);
    void removeExclusiveEffect(int exclusive);
	bool* getExclusiveEffect() { return exclusiveEffects; }
    //�������Ʋ���
    int addHandCards(int howMany, vector< int > newCard);
    //�Ƴ����Ʋ���
    int removeHandCards(int howMany, vector<int> oldCard);
	//�������
	int checkHandCards(int howMany, vector<int> cards);
	int checkOneHandCard(int cardID);
	//����
	int addCoverCards(int howMany, vector< int > cards);
	int removeCoverCards(int howMany, vector< int > cards);
    int checkCoverCards(int howMany, vector<int> cards);
	int checkOneCoverCard(int cardID);
    //�趨���������Ƿ�����
    void setHandCardsMaxFixed(bool fixed, int howmany=6);
    //�������Ʊ仯
    void addHandCardsRange(int howMany);
    //��������
    void addCrossNum(int howMany, int atMost=-1);
    void subCrossNum(int howMany);

    void setGem(int howMany);
    void setCrystal(int howMany);
	void setRoleID(int id) { roleID = id; }
    //���ú���
    void setTap(bool tap){this->tap = tap;}
    void setToken(int id,int howMany){
		if(howMany<0)howMany=0;
		token[id]= howMany<=tokenMax[id] ? howMany: (token[id] >tokenMax[id]? token[id]: tokenMax[id]);}
    //������һ�����
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
    //��ȡ�������ǰ�Ļ���Ч����
    list< struct BasicEffect > getBasicEffect(){return this->basicEffects;}

    PlayerEntity* getPost(){ return this->postPlayer;}
	PlayerEntity* getPre(){ return prePlayer; }
    list< int > getHandCards(){return this->handCards;}
    int getRoleID(){return roleID;}
	//�ı��������
	virtual int getCardElement(int cardID) {
		CardEntity* card = getCardByID(cardID);
		return card->getElement();
	}
    bool tapped(){return this->tap;}
    bool isHandCardsMaxFixed(){return this->handCardsMaxFixed;}
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
	void toProto(SinglePlayerInfo *playerInfo);
	bool virtual toNextStep(int ret) {	return GE_SUCCESS == ret || GE_TIMEOUT == ret; }
	//������ɫ��ص�����
	//return true ��ʾ������
	virtual bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto){ return false; }

	//�غ��޶���ͳһ�������ʼ��
	virtual int p_before_turn_begin(int &step, int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_turn_begin(int &step, int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_turn_begin_shiren(int &step, int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_between_weak_and_action(int &step, int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_before_action(int &step, int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_boot(int &step, int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_before_attack(int &step, int dstID, int srcID) { return GE_SUCCESS; }
	virtual int p_after_attack(int &step, int playerID) { return GE_SUCCESS; }
	virtual int p_before_magic(int &step, int srcID) { return GE_SUCCESS; }
	virtual int p_after_magic(int &step, int srcID) { return GE_SUCCESS; }
	virtual int p_before_special(int &step, int srcID) { return GE_SUCCESS; }
	virtual int p_after_special(int &step, int srcID) { return GE_SUCCESS; }
	virtual int p_turn_end(int &step, int playerID) { return GE_SUCCESS; }
	virtual int p_after_turn_end(int &step, int playerID) { return GE_SUCCESS; }
	virtual int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con) { return GE_SUCCESS; }
	virtual int p_attacked(int &step, CONTEXT_TIMELINE_1 *con) { return GE_SUCCESS; }
	virtual int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con) { return GE_SUCCESS; }
	virtual int p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con) { return GE_SUCCESS; }
	virtual int p_harm_end(int &step, CONTEXT_HARM_END *con) { return GE_SUCCESS; }
	virtual int p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con) { return GE_SUCCESS; }
	virtual int p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con) { return GE_SUCCESS; }
	virtual int p_timeline_5(int &step, CONTEXT_TIMELINE_5 *con) { return GE_SUCCESS; }
	virtual int p_timeline_6(int &step, CONTEXT_TIMELINE_6 *con) { return GE_SUCCESS; }
	virtual int p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con) { return GE_SUCCESS; }
	virtual int p_before_lose_morale(int &step, CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_lose_morale(int &step, CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_xin_yue(int &step, CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_fix_morale(int &step, CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_true_lose_morale(int &step, CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_hand_change(int &step, int playerID) { return GE_SUCCESS; }
	virtual int p_basic_effect_change(int &step, int dstID, int card, int doerID, int cause)  { return GE_SUCCESS; }
    virtual int p_cover_change(int &step, int dstID, int direction, int howMany, vector<int> cards, int doerID, int cause)  { return GE_SUCCESS; }
	virtual int p_show_hand(int &step, int playerID, int howMany, vector<int> cards, HARM harm) { return GE_SUCCESS; }
	virtual int p_additional_action(int chosen);
	virtual int p_attack_skill(int &step, Action *action) { return GE_EMPTY_HANDLE; }
	virtual int p_magic_skill(int &step, Action *action) { return GE_EMPTY_HANDLE; }
	virtual int p_special_skill(int &step, Action *action) { return GE_EMPTY_HANDLE; }
	virtual int p_request_hand_give_up(int &step, int targetID, int cause) { return GE_SUCCESS; }
    virtual int p_request_cover_give_up(int &step, int targetID, int cause) { return GE_SUCCESS; }
	virtual int p_reattack(int &step, int &cardID, int doerID, int targetID, bool &realCard) { return GE_SUCCESS; }
	virtual int p_tap(int &step, CONTEXT_TAP *con) { return GE_SUCCESS; }

	virtual int v_allow_action(Action* action, int allow, bool canGiveUp);
	virtual int v_unactional() { return GE_SUCCESS; }
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
	virtual int v_request_hand(int cardSrc, int howMany, vector<int> cards, HARM harm) { return GE_SUCCESS; }
    virtual int v_request_cover(int howMany, vector<int> cards, HARM harm) { return GE_SUCCESS; }
protected:
    int id;//���id
    int roleID;
    string name;
    int handCardsMax;
    int handCardsRange;
    int handCardsMin;//��������֮��ʹ��
    int coverCardsMax;
    int crossNum;
    int crossMax;
    int gem;
    int crystal;
    int energyMax;
    int color;
    float star;
    bool tap;//����״̬
    bool handCardsMaxFixed;//�Ƿ�������������
    bool yourTurn;
    int seatNum;
    int token[2];
    int tokenMax[2];
	PlayerEntity* prePlayer;//�ϼ�
    PlayerEntity* postPlayer;//�¼�
    TeamArea* teamArea;
    list< int > handCards;//����
    list< BasicEffect > basicEffects;//����Ч����
    bool exclusiveEffects[EXCLUSIVE_NUM];//ר��Ч��
    list< int > coverCards;//������
	list< ACTION_QUOTA > additionalActions;
	GameGrail *engine;
};

#endif // PLAYERENTITY_H
