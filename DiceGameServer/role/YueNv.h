#pragma once
#include "..\PlayerEntity.h"

class YueNv : public PlayerEntity
{
public:
	YueNv(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){tokenMax[0] = 2; tokenMax[1] = 3; tokenMax[2] = 99; coverCardsMax = 99;}
	bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto);
	int p_xin_yue(int &step, CONTEXT_LOSE_MORALE *con);
	int p_cover_change(int &step, int dstID, int direction, int howMany, vector<int> cards, int doerID, int cause);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int p_turn_end(int &step, int playerID);
	int p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con);
	int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con);
	int v_magic_skill(Action *action);
	int p_magic_skill(int &step, Action* action);
	int p_after_turn_end(int &step, int playerID);
	int p_before_turn_begin(int &step, int currentPlayerID);
	
	int p_additional_action(int chosen);


private:
	//具体技能
	int XinYueBiHu(CONTEXT_LOSE_MORALE *con);
	int AnYueZuZhou();
	int MeiDuSha(CONTEXT_TIMELINE_1 *con);
	int YueZhiLunHui();
	int YueDu(CONTEXT_TIMELINE_6_DRAWN *con);
	int AnYueZhan(CONTEXT_TIMELINE_2_HIT *con);
	int CangBaiZhiYue(int &step, Action* action);
	bool used_YueDu = false;
	bool CangBaiAddTurn = false;
	bool CangBaiAttack = false;
};
