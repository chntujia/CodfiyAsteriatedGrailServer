#pragma once
#include "..\PlayerEntity.h"

class NvWuShen :
	public PlayerEntity
{
public:
	NvWuShen(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int p_turn_begin(int &step, int currentPlayerID);
	int p_after_attack(int &step, int playerID);
	int p_after_magic(int &step, int playerID);
	int p_additional_action(int chosen);
	int v_magic_skill(Action *action);
	int p_magic_skill(int &step, Action* action);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con);
private:
	int ShenShengZhuiJiAfterAttack(int playerID);
	int ShenShengZhuiJi(int playerID);
	int ZhiXuZhiYin(int &step, Action* action);
	int HePingXingZhe(CONTEXT_TIMELINE_1 *con);
	int JunShenWeiGuang();
	int YingLingZhaoHuan(CONTEXT_TIMELINE_2_HIT *con);
};

