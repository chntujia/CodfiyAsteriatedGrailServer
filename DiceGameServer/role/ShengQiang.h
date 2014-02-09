#pragma once
#include "..\PlayerEntity.h"

class ShengQiang : public PlayerEntity
{
public:
	ShengQiang(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){crossMax = 3;used_ShengGuangQiYu = false;used_TianQiang = false; used_DiQiang = false;}
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int p_before_turn_begin(int &step, int currentPlayerID);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con);
	int v_magic_skill(Action *action);
	int p_magic_skill(int &step, Action* action);
private:
	int HuiYao(int &step, Action* action);
	int ChengJie(int &step, Action* action);
	int ShengGuangQiYu(Action* action);
	int TianQiang(CONTEXT_TIMELINE_1 *con);
	int DiQiang(CONTEXT_TIMELINE_2_HIT *con);
	int ShengJi(CONTEXT_TIMELINE_2_HIT *con);
	//int TianQiangUsed(CONTEXT_TIMELINE_2_HIT *con);
	//int DiQiangUsed(CONTEXT_TIMELINE_2_HIT *con);

	bool used_ShengGuangQiYu;
	bool used_TianQiang;
	bool used_DiQiang;
};