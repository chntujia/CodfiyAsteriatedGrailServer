#pragma once
#include "..\PlayerEntity.h"

class WuNv : public PlayerEntity
{
public:
	WuNv(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){tap =false; tongShengID = -1;}
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int p_turn_begin(int &step, int currentPlayerID);
	int p_boot(int &step, int currentPlayerID);
	int p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con);
	int p_hand_change(int &step, int playerID);
	int p_true_lose_morale(int &step, CONTEXT_LOSE_MORALE *con);
	int v_magic_skill(Action *action);
	int p_magic_skill(int &step, Action* action);
private:
	int TongShengQiangSi(int &step, Action* action);
	int XueZhiAiShang(int &step, int srcID);
	int XueZhiAiShangJudge(CONTEXT_TIMELINE_4 *con);
	int NiLiu(int &step, Action* action);
	int XueZhiBeiMing(Action* action);
	int XueZhiZuZhou(int &step, Action* action);
	int ToLiuXueXingTai(CONTEXT_LOSE_MORALE *context);
	int ToPuTongXingTai(int playerID);
	int LiuXue(int playerID);

	int tongShengID;
	int aiShangChoose=0;
	int aiShangDst;
};
