#pragma once
#include "..\PlayerEntity.h"

class JianSheng : public PlayerEntity
{
public:
	JianSheng(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}
	//找出并实现所有需要的p_xxxx虚函数，这些p_xxxx函数会在对应的StateXXXXX里被调用
	//每个p_xxxx里可能有不只一个技能，step就是用来区分这些的
	int p_before_turn_begin(int &step, int currentPlayerID);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int p_after_attack(int &step, int playerID);

private:
	//具体技能
	int LieFengJi(CONTEXT_TIMELINE_1 *con);
	int JiFengJi(CONTEXT_TIMELINE_1 *con);
	int ShengJian(CONTEXT_TIMELINE_1 *con);
	int LianXuJi(int playerID);
	int JianYing(int playerID);

	bool used_LianXuJi;
	bool used_JianYing;
	int attackCount;
};
