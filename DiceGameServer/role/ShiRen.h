#pragma once
#include "..\PlayerEntity.h"

class ShiRen :
	public PlayerEntity
{
public:
	ShiRen(GameGrail *engine, int id, int color);
	static bool ShiRenParse(UserTask* session, int playerID, ::google::protobuf::Message *proto);
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int p_before_turn_begin(int &step, int currentPlayerID);
	int p_turn_begin_shiren(int &step, int currentPlayerID);
	int p_boot(int &step, int currentPlayerID);
	int p_turn_end(int &step, int currentPlayerID);
	int v_magic_skill(Action *action);
	int p_magic_skill(int &step, Action* action);
	int p_harm_end(int &step, CONTEXT_HARM_END *con);
private:
	int ChenLunXieZouQu(CONTEXT_HARM_END *con);
	int BuXieZhiXian(Action* action);
	int GeYongTianFu();
	int JiAngKuangXiangQu2(int &step, int currentPlayerID);
	int ShengLiJiaoXiangShiStone(int &step, int currentPlayerID);
	int XiWangFuGeQu();

	int YueZhangDst;
	bool useYueZhang;
	bool ChenLunNum[6];
	bool ChenLunUsed;
};