#pragma once
#include "..\PlayerEntity.h"

class JingLingSheShou : public PlayerEntity
{
public:
	JingLingSheShou(GameGrail *engine, int id, int color):PlayerEntity(engine, id, color) { coverCardsMax = 3; }
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int p_before_turn_begin(int &step, int currentPlayerID);
	int p_boot(int &step, int currentPlayerID);
	int p_turn_end(int &step, int playerID);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con);
	int p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con);
	int p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con);
private:
	int YuanSuSheJi(CONTEXT_TIMELINE_1 *con);
	int YuanSuSheJiHit(CONTEXT_TIMELINE_2_HIT *con);

	int DongWuHuoBan(CONTEXT_TIMELINE_6_DRAWN *con);
	int JingLingMiYiBoot();
	int JingLingMiYiReset();

	bool shuizhishi = false;;
	bool dizhishi = false;
	bool used_YUAN_SU_SHE_JI = false;
};
