#pragma once
#include "..\PlayerEntity.h"

class FengYin : public PlayerEntity
{
public:
	FengYin(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}
	int p_after_magic(int &step, int playerID);
	int p_show_hand(int &step, int playerID, int howMany, vector<int> cards);
	int p_magic_skill(int &step, Action* action);
	int v_magic_skill(Action *action);
	static int WuXiShuFu_Effect(GameGrail *engine);
private:
	int FaShuJiDang(int playerID);
	int FengYin_Effect(int playerID, int howMany, vector<int> cards);
	int FengYin_Cast(Action *action);
	int WuXiShuFu(Action *action);
	int FengYinPoSui(Action *action);
	int getMapping(int element);
};