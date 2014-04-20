#pragma once
#include "..\PlayerEntity.h"


class JianDi: public PlayerEntity
{
public:
	JianDi(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){
	    tokenMax[0]=5;  //剑气
		tokenMax[2]=3;  //剑魂
		coverCardsMax =3;
		
	}
 bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
 int p_before_turn_begin(int &step, int currentPlayerID);
 int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
 int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con);
 int p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con);
 int p_after_attack(int &step, int playerID);

 int TianShiYuEMo();
 int YangGon();
 int JianHunShouHu(CONTEXT_TIMELINE_2_MISS* con);
 int BuQuYiZhi();
 int JianQiZhan(CONTEXT_TIMELINE_2_HIT *con);
 int TianShiZhiHun();
 int TianShiZhiHun_EffectHit();
 int TianShiZhiHun_EffectMiss();
 int EMoZhiHun();
 int EMoZhiHun_EffectMiss();

private:
bool used_TIAN_SHI_ZHI_HUN;
bool used_E_MO_ZHI_HUN;
int flag;  //0:非天使非恶魔  1：天使   2：恶魔
};