#pragma once
#include "..\PlayerEntity.h"

class KuangZhan : public PlayerEntity
{
public:
	KuangZhan(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}
	//找出并实现所有需要的p_xxxx虚函数，这些p_xxxx函数会在对应的StateXXXXX里被调用
	//每个p_xxxx里可能有不只一个技能，step就是用来区分这些的
	int p_before_turn_begin(int &step, int currentPlayerID);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int p_timeline_2_hit(int &setp, CONTEXT_TIMELINE_2_HIT *con);
	//角色的消息处理，将由UserTask调用，只要使用了waitForXXX，基本都要覆写这个函数
	//return true 表示处理了
	bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto);
private:
	//具体技能
	int XueYingKuangDao(CONTEXT_TIMELINE_1 *con);
	int XueXingPaoXiao(CONTEXT_TIMELINE_1 *con);
	int KuangHua(CONTEXT_TIMELINE_2_HIT *con);
	int SiLie(CONTEXT_TIMELINE_2_HIT *con);
	bool used_XueYingKuangDao;
};
