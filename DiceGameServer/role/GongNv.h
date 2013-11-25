#pragma once
#include "..\PlayerEntity.h"

class GongNv : public PlayerEntity
{
public:
	GongNv(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}
	//找出并实现所有需要的p_xxxx虚函数，这些p_xxxx函数会在对应的StateXXXXX里被调用
	//每个p_xxxx里可能有不只一个技能，step就是用来区分这些的
	//int p_before_turn_begin(int &step, int currentPlayerID);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	//int p_timeline_2_hit(int &setp, CONTEXT_TIMELINE_2_HIT *con);
	int p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con);
	//int p_after_attack(int &step, int playerID);
	int p_magic_skill(int &step, Action* action);
	int v_magic_skill(Action *action);
	bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto);

private:
	//具体技能
	int ShanDianJian(CONTEXT_TIMELINE_1 *con);//闪电箭：雷系攻击或应战无法被应战
	int GuanChuanSheJi(CONTEXT_TIMELINE_2_MISS *con);//贯穿射击：主动攻击未命中，弃1法术牌造成2法术伤害
	int ShanGuangXianJing(Action* action);//闪光陷阱：牌技能：2点法术伤害
	int JingZhunSheJi(CONTEXT_TIMELINE_1 *con);//精准射击：牌技能：攻击强制命中，1点伤害，可选触发
	int JuJi(Action *action); //狙击：1水晶，目标手牌补到5张，+1攻击行动
	
};
