#pragma once
#include "..\PlayerEntity.h"

class  MoJian: public PlayerEntity
{
public:
	 MoJian(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){};
	 bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	 int p_before_turn_begin(int &step, int currentPlayerID);
	 int p_before_action(int &step, int currentPlayerID);
	 int p_boot(int &step, int currentPlayerID);
	 int v_magic_skill(Action *action);
	 int p_magic_skill(int &step, Action* action);
	 int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	 int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con);
	 int p_after_attack(int &step, int playerID);
	 int v_attack(int cardID, int dstID, bool realCard);
	 int v_additional_action(int chosen);
	 int p_additional_action(int chosen);
	 int v_missile(int cardID, int dstID, bool realCard);
	 int v_remissile(int cardID, bool realCard);
	 int v_block(int cardID);
	 int v_shield(int cardID, PlayerEntity* dst);
	 int v_weaken(int cardID, PlayerEntity* dst);

private:
	//具体技能
	int XiuLuoLianZhan(int playerID);
	int AnYingZhiLi(CONTEXT_TIMELINE_2_HIT *con);
	int AnYingNingJu();
	int AnYingNingJuReset();
	int AnYingNingJuSelfHurt();
	int HeiAnZhenChanNoReattack(CONTEXT_TIMELINE_1 *con);
    int HeiAnZhenChanBuPai(CONTEXT_TIMELINE_2_HIT* con);
	int AnYingLiuXing(Action* action);

bool   used_XiuLuoLianZhan;
bool   used_HeiAnZhenChan;
bool   using_XiuLuoLianZhan;
bool   using_HeiAnZhenChan;
};