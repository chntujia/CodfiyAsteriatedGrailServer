#pragma once
#include "..\PlayerEntity.h"

class DieWu: public PlayerEntity
{
public:
	DieWu(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){
		tokenMax[0]=99;
	    coverCardsMax = 8;
		tokenMax[2]=8;
		handCardsMin = 3;
	}
	int	p_before_turn_begin(int &step, int currentPlayerID);
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int	v_magic_skill(Action *action);
	int p_magic_skill(int &step, Action* action);
	int p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con);
	int p_timeline_5(int &step, CONTEXT_TIMELINE_5 *con);
	int p_timeline_6(int &step, CONTEXT_TIMELINE_6 *con);
	int p_cover_change(int &step, int dstID, int direction, int howMany, vector<int> cards, int doerID, int cause); 
	int p_fix_morale(int &step, CONTEXT_LOSE_MORALE *con);
private:

	int	DuFen(CONTEXT_TIMELINE_5 *con);
	int WuDong(Action *action);
	int YongHua(Action *action);
	int DaoNiZhiDie(Action *action);
	int DaoNiZhiDie_Yong();
	int Diao_Ling(vector<int> cards);
	int ChaoSheng(CONTEXT_TIMELINE_6 *con);
	int JingHuaShuiYue(CONTEXT_TIMELINE_5 *con);
	int DiaoLing_Effect(CONTEXT_LOSE_MORALE *con);
	int DaoNiCross(CONTEXT_TIMELINE_4 *con);
};
