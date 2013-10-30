#pragma once
#include "..\PlayerEntity.h"

class JianSheng : public PlayerEntity
{
public:
	JianSheng(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}
	int p_before_turn_begin(int currentPlayerID);
	int p_timeline_1(CONTEXT_TIMELINE_1 *con);
	int p_after_attack(int playerID);

private:
	int LieFengJi(CONTEXT_TIMELINE_1 *con);
	int JiFengJi(CONTEXT_TIMELINE_1 *con);
	int ShengJian(CONTEXT_TIMELINE_1 *con);
	int LianXuJi(int playerID);

	bool has_LianXuJi;
	bool has_JianYing;
	int attackCount;
};
