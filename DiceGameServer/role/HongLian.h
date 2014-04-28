#pragma once
#include "..\PlayerEntity.h"

class HongLian : public PlayerEntity
{
public:
	HongLian(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){crossMax = 4;tokenMax[0] = 2;used_XingHongShengYue = false;}
	int p_before_turn_begin(int &step, int currentPlayerID);
	int p_boot(int &step, int currentPlayerID);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con);
	int p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con);
	int p_after_attack(int &step, int playerID);
	int p_after_magic(int &step, int playerID);
	int v_magic_skill(Action *action);
	int p_magic_skill(int &step, Action* action);
	bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto);
	int p_additional_action(int chosen);
	int p_before_lose_morale(int &step, CONTEXT_LOSE_MORALE *con);
	int p_true_lose_morale(int &step, CONTEXT_LOSE_MORALE *con);
	int p_turn_end(int &step, int playerID);
private:
	//具体技能
	int XingHongShengYue(CONTEXT_TIMELINE_1 *con);
	int XingHongXinYang(CONTEXT_TIMELINE_4 *con);
	int XueXingDaoYan();
	int ShaLuShengYan(CONTEXT_TIMELINE_2_HIT *con);
	int ReXueFeiTeng(CONTEXT_LOSE_MORALE *con);
	int ReXueFeiTengLoseNoMorale(CONTEXT_LOSE_MORALE *con);
	int ReXueFeiTengReset();
	int JieJiaoJieZao(int playerID);
	//int JieJiaoJieZaoAfterMagic(int playerID);
	int XingHongShiZi(int &step, Action* action);

	bool used_XingHongShengYue;
	bool used_Magic;
};
