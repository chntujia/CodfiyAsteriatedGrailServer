#pragma once
#include "..\PlayerEntity.h"

class YuanSu : public PlayerEntity
{
public:
	YuanSu(GameGrail *engine, int id, int color);
	int v_magic_skill(Action* action);
	int p_magic_skill(int &step, Action *action);
	int p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con);
private:
	int YunShi(Action *action);
	int FengRen(Action *action);
	int HuoQiu(Action *action);
	int LeiJi(int step, Action *action);
	int BingDong(int step, Action *action);
	int YuanSuDianRan(Action *action);
	int YueGuang(Action *action);
	int YuanSuDamage(Action *action);
};
