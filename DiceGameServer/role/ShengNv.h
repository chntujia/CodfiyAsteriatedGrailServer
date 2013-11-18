#pragma once
#include "..\PlayerEntity.h"

class ShengNv : public PlayerEntity
{
public:
	ShengNv(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){used_ShengLiao = false;}
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int p_before_turn_begin(int &step, int currentPlayerID);
	int p_boot(int &step, int currentPlayerID);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int v_magic_skill(Action *action);
	int p_magic_skill(int &step, Action* action);
private:
	int ZhiLiaoShu(Action* action);
	int ZhiYuZhiGuang(Action* action);
	int ShengLiao(Action* action);
	int LianMin();
	int BingShuangDaoYan(CONTEXT_TIMELINE_1 *con);

	bool used_ShengLiao;
};
