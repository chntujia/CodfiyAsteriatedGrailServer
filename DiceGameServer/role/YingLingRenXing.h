#pragma once
#include "..\PlayerEntity.h"

class YingLingRenXing :
	public PlayerEntity
{
public:
	YingLingRenXing(GameGrail *engine, int id, int color);
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con);
	int p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con);
	int p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con);
	int p_turn_begin(int &step, int currentPlayerID);
	int p_boot(int &step, int currentPlayerID);
	int p_turn_end(int &step, int currentPlayerID);
	int p_before_lose_morale(int &step, CONTEXT_LOSE_MORALE *con);
private:
	int NuHuoMoWen(CONTEXT_TIMELINE_2_MISS *con);
	int ZhanWenSuiJi(CONTEXT_TIMELINE_2_HIT *con);
	int FuWenGaiZao();
	int FuWenGaiZaoToken();
	int FuWenTurnEnd();
	int ShuangChongHuiXiang(CONTEXT_TIMELINE_3 *con);
	int ShuangChongHuiXiangMorale(CONTEXT_LOSE_MORALE *con);
	int YingLingRenXing::elementCheck(vector<int> cards);

	bool shuangChongHuiXiangUsed;
};