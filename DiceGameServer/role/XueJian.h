#pragma once
#include "..\PlayerEntity.h"

class XueJian : public PlayerEntity
{
public:
	XueJian(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){ tokenMax[0] = 3; }
	//找出并实现所有需要的p_xxxx虚函数，这些p_xxxx函数会在对应的StateXXXXX里被调用
	//每个p_xxxx里可能有不只一个技能，step就是用来区分这些的
	bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto);
	int p_timeline_2_hit(int &setp, CONTEXT_TIMELINE_2_HIT *con);
	int p_after_attack(int &step, int playerID);
	int p_additional_action(int chosen);
	int v_magic_skill(Action *action);
	int p_magic_skill(int &step, Action* action);
	int p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con);
	int p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con);
	int p_boot(int &step, int currentPlayerID);
	int p_turn_end(int &step, int playerID);
	//角色的消息处理，将由UserTask调用，只要使用了waitForXXX，基本都要覆写这个函数
	//return true 表示处理了

private:
	//具体技能
	int XueSeJingJi(CONTEXT_TIMELINE_2_HIT *con);
	int ChiSeYiShan(int playerID);
	int XueRanQiangWei(int &step, Action* action);
	int XueQiPingZhang(CONTEXT_TIMELINE_3 *con);
	int XueQiangWeiBoot();
};
