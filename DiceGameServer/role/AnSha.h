#pragma once
#include "..\PlayerEntity.h"

class AnSha : public PlayerEntity
{
public:
	AnSha(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int p_boot(int &step, int currentPlayerID);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con);
	int p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con);
	int p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con);
	int v_attacked();
	int p_before_action(int &step, int currentPlayerID);
private:
	int FanShi( CONTEXT_TIMELINE_6_DRAWN *con);
	int ShuiYing(CONTEXT_TIMELINE_3 *con);
	int QianXingBoot();
	int QianXingNoReattack(CONTEXT_TIMELINE_1 *con);
	int QianXingDamage(CONTEXT_TIMELINE_2_HIT *con);
	int QianXingReset();
};
