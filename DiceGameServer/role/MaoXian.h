#pragma once
#include "..\PlayerEntity.h"

class MaoXian : public PlayerEntity
{
public:
	MaoXian(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}
	//找出并实现所有需要的p_xxxx虚函数，这些p_xxxx函数会在对应的StateXXXXX里被调用
	//每个p_xxxx里可能有不只一个技能，step就是用来区分这些的
	int p_before_turn_begin(int &step, int currentPlayerID);
	int p_attack_skill(int &step, Action* action);
	int p_magic_skill(int &step, Action* action);
	int p_special_skill(int &step, Action* action);
	//v_xxxx用来做检测，将由State调用，一般检测已实现，但是像剑圣的连续技必须是风斩这类的特殊检测，只能覆写v_xxxx
	int v_buy(Action *action);
	int v_attack_skill(Action *action);
	int v_magic_skill(Action *action);
	int v_special_skill(Action *action);
private:
	//具体技能
	int QiZha(int step, Action *action);
	int TouTianHuanRi(Action *action);
	int TeShuJiaGong(Action *action);
	int MaoXianZheTianTang(Action *action);
	//响应三版号召，偷天换日跟特殊加工共用一个flag
	bool used_TouTianHuanRi;
};
