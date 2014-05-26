#pragma once
#include "..\PlayerEntity.h"

class MoNv: public PlayerEntity
{
public:
	MoNv(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){
		tokenMax[0]=4;
		cardNum = 0;
	}
	int	p_between_weak_and_action(int &step, int currentPlayerID);
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	int p_boot(int &step, int currentPlayerID);
	int p_magic_skill(int &step, Action* action);
	int	v_magic_skill(Action *action);
	int p_attack_skill(int &step, Action* action);
	int v_attack_skill(Action *action);
	int v_reattack(int cardID, int orignCardID, int dstID, int orignID, int rate, bool realCard);
	int p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con);
	int p_true_lose_morale(int &step, CONTEXT_LOSE_MORALE *con);
	int p_reattack(int &step, int &cardID, int doerID, int targetID, bool &realCard);
	int getCardElement(int cardID);
private:
	int CangYanFaDian(Action *action);
	int TianHuoDuanKong(Action *action);
	int MoNvZhiNu();
	int MoNvZhiNuDraw();
	int GetAwayFromFire();
	int TiShenWanOu(CONTEXT_TIMELINE_3 *con);
	int YongShengYinShiJi(CONTEXT_LOSE_MORALE *con);
	int TongKuLianJie(Action *action);
	int TongKuLianJieCard(Action *action);
	int MoNengFanZhuan(CONTEXT_TIMELINE_3 *con);
	int ToFire( Action *action);
	int ToFire(int &cardID, int doerID, int targetID, bool &realCard);
	int ToFire(int howMany, vector<int> &cards);
	bool IsFired(int cardID);

	int cardNum;
};
