#pragma once
#include "..\PlayerEntity.h"

class QiDao : public PlayerEntity
{
public:
	
	static int GetXunJieEffectCard(GameGrail* engine,int id);//返回迅捷赐福的cardID，在PlayerEntity.cpp中调用
	static void WeiLiCiFuParse(UserTask* session, int playerID, ::google::protobuf::Message *proto);
	QiDao(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}
	int p_before_turn_begin(int &step, int currentPlayerID) ;
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1* con);
	int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con);
	int p_magic_skill(int &step, Action* action);
	int v_magic_skill(Action *action);
	bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto);
	int p_boot(int &step, int currentPlayerID);
	int p_after_attack(int &step, int playerID) ;
	int p_after_magic(int &step, int srcID);
	int p_additional_action(int chosen);
	int v_additional_action(int chosen);
private:
	//具体技能
	int GuangHuiXinYang(Action* action);//[法术]光辉信仰：祈祷形态下发动，移除1点祈祷符文，弃两张牌，我方战绩区+1宝石，目标队友+1治疗
	int QiHeiXinYang(Action* action);//[法术]漆黑信仰：祈祷形态下发动，移除1点祈祷符文，对目标角色和自己各造成2点伤害
	int WeiLiCiFu(Action* action);//[法术]威力赐福：将威力赐福放置于队友面前
	int XunJieCiFu(Action* action);//[法术]迅捷赐福：将迅捷赐福放置于队友面前
	int XunJieCiFuEffect(int playerID); //判断赐福效果
	int WeiLiCiFuEffect(CONTEXT_TIMELINE_2_HIT * con); //判断赐福效果
	int FaLiChaoXi(int playerID);
	bool CheckCiFuTarget(Action* action,int CiFu);//判断目标是否已有同类赐福
	bool used_FaLiChaoXi;
	int QiDong();//[启动]祈祷
};
