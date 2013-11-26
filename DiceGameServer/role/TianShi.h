#pragma once
#include "..\PlayerEntity.h"

class TianShi : public PlayerEntity
{
public:
	TianShi(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int p_turn_begin(int &step, int currentPlayerID);
	int v_magic_skill(Action* action);
	int p_magic_skill(int &step, Action *action);
	int p_basic_effect_change(int &step, int dstID, int card, int doerID, int cause);
	int p_before_lose_morale(int &step, CONTEXT_LOSE_MORALE *con);
private:
	int TianShiZhiQiang(Action *action);
	int TianShiZhuFu(int step, Action *action);
	int FengZhiJieJing(Action *action);
	int TianShiJiBan();
	int TianShiZhiGe();
	int ShenZhiBiHu(CONTEXT_LOSE_MORALE *con);
};