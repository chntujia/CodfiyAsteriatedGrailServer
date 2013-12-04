#pragma once
#include "..\PlayerEntity.h"

class ZhongCai : public PlayerEntity
{
public:
	ZhongCai(GameGrail *engine, int id, int color);
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int p_turn_begin(int &step, int currentPlayerID);
	int p_boot(int &step, int currentPlayerID);
	int p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con);
	int v_allow_action(Action* action, int allow, bool canGiveUp);
	int v_magic_skill(Action* action);
	int p_magic_skill(int &step, Action *action);
private:
	int YiShiZhongDuan();
	int ZhongCaiYiShi();
	int MoRiShenPan(Action* action);
	int ShenPanLangChao();
	int PanJueTianPing(Action* action);
	int ZhongCaiYiShiAddToken();
};
