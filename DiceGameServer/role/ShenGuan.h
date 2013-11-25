#pragma once
#include "..\PlayerEntity.h"

class ShenGuan : public PlayerEntity
{
public:
	ShenGuan(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){crossMax = 6;}
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int p_after_special(int &step, int srcID);
	int p_boot(int &step, int currentPlayerID);
	int p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con);
	int v_magic_skill(Action *action);
	int p_magic_skill(int &step, Action* action);
private:
	int ShenShengQiShi(int &step, int srcID);
	int ShenShengQiFu(int &step, Action* action);
	int ShuiZhiShenLi(int &step, Action* action);
	int ShengShiShouHu(CONTEXT_TIMELINE_4 *con);
	int ShenShengLingYu(int &step, Action* action);
	int ShenShengQiYue();
};
