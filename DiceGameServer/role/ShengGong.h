#pragma once
#include "..\PlayerEntity.h"

class ShengGong : public PlayerEntity
{
public:
	ShengGong(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color)
	{
		tokenMax[0] = 10;  //ÐÅÑö
		crossMax = 3;
		used_HuiGuangPao = false;
		useShengXie = 0;
		noTianChong = false;
	}
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int p_before_turn_begin(int &step, int currentPlayerID);
	int p_turn_end(int &step, int currentPlayerID);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con);
	int p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con);
	int v_magic_skill(Action *action);
	int p_magic_skill(int &step, Action* action);
	int p_before_special(int &step, int currentPlayerID);
private:
	int TianZhiGong(CONTEXT_TIMELINE_1 *con);
	int ShengXieJuBao(int &step, Action* action);

	int ShengXieMiss(CONTEXT_TIMELINE_2_MISS *con);

	int ShengHuangMoShi(int &step, Action* action);
	int ShengGuangBaoLie(int &step, Action* action);
	int ShengLiuXingDan(int currentPlayerID);
	int ShengLiuXingHit(CONTEXT_TIMELINE_2_HIT *con);
	int ShengLiuXingAtk(CONTEXT_TIMELINE_1 *con);
	int ZiDongTianChong();
	int HuiGuangPao(int &step, Action* action);

	int useShengXie;
	bool noTianChong;
	bool used_HuiGuangPao;
};