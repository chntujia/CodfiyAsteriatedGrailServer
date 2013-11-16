#pragma once
#include "..\PlayerEntity.h"

class MoDao : public PlayerEntity
{
public:
	MoDao(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}	
	int p_magic_skill(int &step, Action* action);
	int p_request_hand_give_up(int &step, int targetID, int cause);

	int v_magic_skill(Action *action);
	int v_request_hand(int howMany, vector<int> cards, HARM harm);	
	int v_missile(int cardID, int dstID, bool realCard = true);
	int v_remissile(int cardID, bool realCard = true);
private:
	int MoBaoChongJi(Action* action);
	int MoDanRongHe(Action* action);
	int HuiMieFengBao(Action* action);
	int checkTwoTarget(Action* action);
	bool isHit;

};