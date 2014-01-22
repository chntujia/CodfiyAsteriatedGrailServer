#pragma once
#include "..\PlayerEntity.h"

class DieWu: public PlayerEntity
{
public:
	DieWu(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){
		tokenMax[0]=99;
	    coverCardsMax = 8;
		tokenMax[2]=8;
	}
int	p_before_turn_begin(int &step, int currentPlayerID);
bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
int	v_magic_skill(Action *action);
int p_magic_skill(int &step, Action* action);
int p_timeline_6(int &step, CONTEXT_TIMELINE_6 *con);
int p_cover_change(int &step, int dstID, int direction, int howMany, vector<int> cards, int doerID, int cause); 
int p_lose_morale(int &step, CONTEXT_LOSE_MORALE *con);
private:

int	DuFeng(CONTEXT_TIMELINE_6 *con);
int WuDong(Action *action);
int YongHua(Action *action);
int DaoNiZhiDie(Action *action);
int Diao_Ling(vector<int> cards);
int ChaoSheng(CONTEXT_TIMELINE_6 *con);
int JingHuaShuiYue(CONTEXT_TIMELINE_6 *con);
int DiaoLing_Effect(CONTEXT_LOSE_MORALE *con);
bool used_DiaoLing;

};
