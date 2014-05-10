#pragma once
#include "..\PlayerEntity.h"
#include <list>

class XianZhe : public PlayerEntity
{
public:
	XianZhe(GameGrail *engine, int id, int color);
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int v_magic_skill(Action* action);
	int p_magic_skill(int &step, Action *action);
	int p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con);
private:
	int ZhiHuiFaDian();
	int MoDaoFaDian(Action *action);
	int ShengJieFaDian_show(Action *action);
	int ShengJieFaDian_effect(Action *action);
	int FaShuFanTan();
	int elementCheck(vector<int> cards);
};