#pragma once
#include "..\PlayerEntity.h"

class SiLing : public PlayerEntity
{
public:
	SiLing(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){crossMax = 5;used_SiWangZhiChu = false;}
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int p_after_magic(int &step, int playerID);
	int p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con);
	int v_magic_skill(Action *action);
	int p_magic_skill(int &step, Action* action);
private:
	int BuXiu(int playerID);
	int SiWangZhiChuReset(int playerID);
	int ShengDu(CONTEXT_TIMELINE_4 *con);
	int WenYi(Action* action);
	int SiWangZhiChu(Action* action);
	int MuBeiYunLuo(int &step, Action* action);

	bool used_SiWangZhiChu;
};
