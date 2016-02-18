#pragma once
#include "..\PlayerEntity.h"

class spMoDao : public PlayerEntity
{
public:
	spMoDao(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}	
	bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	//-------------
	int p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con);
	//-------------
	int p_magic_skill(int &step, Action* action);
	int p_request_hand_give_up(int &step, int targetID, int cause);

	int v_magic_skill(Action *action);
	int v_request_hand(int cardSrc, int howMany, vector<int> cards, HARM harm);	
	int v_missile(int cardID, int dstID, bool realCard = true);
	int v_remissile(int cardID, bool realCard = true);
private:
	//-------
	int FaLiHuDun(CONTEXT_TIMELINE_3 *con);
	int spMoBaoChongJi(Action* action);
	//-------
	int MoDanRongHe(Action* action);
	int HuiMieFengBao(Action* action);
	int checkTwoTarget(Action* action);

};