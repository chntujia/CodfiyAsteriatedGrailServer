#pragma once
#include "..\PlayerEntity.h"

class YinYang : public PlayerEntity
{
public:
	YinYang(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){
		tokenMax[0]=3;
	}
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int v_reattack(int cardID, int orignCardID, int dstID, int orignID, int rate, bool realCard);
	int p_attacked(int &step, CONTEXT_TIMELINE_1 * con);
	int p_magic_skill(int &step, Action* action);
	int v_magic_skill(Action *action);
	int p_turn_end(int &step, int playerID);
	int zhoushu_cardid;
	int zhoushu_dstid;
	int p_before_lose_morale(int &step, CONTEXT_LOSE_MORALE * con);
private:
	//具体技能
	int HeiAnJiLi();
	int YinYangZhuanHuan(CONTEXT_TIMELINE_1 * con);
	int ShiShenZhouShu(CONTEXT_TIMELINE_1 * con);
	int ShiShenJiangLin(Action* action);
	int ShengMingJieJie(Action* action);
	bool ZhuanHuanAtk = false;
	bool GuiHuoAtk=false;
};
