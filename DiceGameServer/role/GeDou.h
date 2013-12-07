#pragma once
#include "..\PlayerEntity.h"

class GeDou : public PlayerEntity
{
public:
	GeDou(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){xuLiUsed  = cangYanUsed = baiShiUsed = false; baiShiTarget = -1; token[0]=0; tokenMax[0]=6;}
	//找出并实现所有需要的p_xxxx虚函数，这些p_xxxx函数会在对应的StateXXXXX里被调用
	//每个p_xxxx里可能有不只一个技能，step就是用来区分这些的
	int p_boot(int &step, int currentPlayerID);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int p_timeline_2_miss(int &setp, CONTEXT_TIMELINE_2_MISS *con);
	int p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con);
	int p_after_attack(int &step, int currentPlayerID);
	int p_before_magic(int &step, int currentPlayerID);
	int p_after_magic(int &step, int currentPlayerID);
	int p_before_special(int &step, int currentPlayerID);
	//角色的消息处理，将由UserTask调用，只要使用了waitForXXX，基本都要覆写这个函数
	//return true 表示处理了
	bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto);
private:
	//具体技能
	int NianQiLiChang(CONTEXT_TIMELINE_3 *con);
	int XuLiCangYan(CONTEXT_TIMELINE_1 *con);
	int XuLiMiss(CONTEXT_TIMELINE_2_MISS *con);
	int NianDan(int step, int playerID);
	int CangYanZiShang(int playerID);
	int BaiShiDouShen(int playerID);
	int BaiShiHarm(CONTEXT_TIMELINE_1 *con);
	int BaiShiQuitMagic(int playerID);
	int BaiShiQuitSpecial(int playerID);
	int baiShiTarget;
	bool xuLiUsed;
	bool cangYanUsed;
	bool baiShiUsed;
};
