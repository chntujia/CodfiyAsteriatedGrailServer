#pragma once
#include "..\PlayerEntity.h"

class MoGong: public PlayerEntity
{
public:
	MoGong(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){
	    coverCardsMax = 8;
		tokenMax[2]=8;
	}
	//找出并实现所有需要的p_xxxx虚函数，这些p_xxxx函数会在对应的StateXXXXX里被调用
	//每个p_xxxx里可能有不只一个技能，step就是用来区分这些的
	 bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	 int p_before_turn_begin(int &step, int currentPlayerID);
	 int p_boot(int &step, int currentPlayerID);
	 int v_magic_skill(Action *action);
	 int p_magic_skill(int &step, Action* action);

	 int v_attack_skill(Action *action);
	 int p_attack_skill(int &step, Action *action); 

	 int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	 int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con);
	 int p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con);
	 int v_attack(int cardID, int dstID, bool realCard);
	 int p_after_attack(int &step, int playerID);
	 int v_additional_action(int chosen);

	 int p_additional_action(int chosen);
	 int ChongNengMoYan(int PlayerID);
	 int MoYan();
	 int ChongNeng();
	 int ChongNengGaiPai();
//	 int MoYanGaiPai();

	 int MoGuanChongJi(CONTEXT_TIMELINE_1 *con);
	 int MoGuanChongJi_Hit(CONTEXT_TIMELINE_2_HIT *con);
	 int DuoChongSheJi_Effect(CONTEXT_TIMELINE_1 *con);
	 int DuoChongSheJi(int playerID);
	 int LeiGuangSanShe(Action *action);
	 int DuoChongSheJi_QiPai(Action *action);
private:
	int ChongNengNum;
	int lastTarget;   //为【多重射击】记录上次攻击目标
	int bootCount;   //计算回合启动数
	//【充能】及【魔眼】
	bool used_CHONG_NENG;
	bool used_MO_YAN;
	//用于回合【魔贯冲击】与【多重射击】互斥
	bool used_MO_GUAN_CHONG_JI; 
	bool used_DUO_CHONG_SHE_JI;  
	bool using_DUO_CHONG_SHE_JI;
	//用于判别本回合【魔贯冲击】及【雷光闪射】是否可用
	bool available_MO_GUAN_CHONG_JI;
	bool avilable_LEI_GUANG_SAN_SHE;
};
