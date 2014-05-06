#pragma once
#include "..\PlayerEntity.h"

class YongZhe : public PlayerEntity
{
public:
	YongZhe(GameGrail *engine, int id, int color);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1* con);
	int p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con);
	int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con);
	int p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con);
	int p_magic_skill(int &step, Action* action);
	int v_magic_skill(Action *action);
	bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto);
	int p_before_turn_begin(int &step, int currentPlayerID);
	int p_before_action(int &step, int currentPlayerID);
	int p_turn_end(int &step, int playerID);
private:
	//具体技能
	int TiaoXin(Action* action);//[法术]【挑衅】：（移除1点【怒气】，将【挑衅】放置于目标对手面前）你+1【知性】；该对手在其下个行动阶段必须且只能主动攻击你，否则他跳过该行动阶段，触发后移除此牌。
	int RemoveTiaoXinEffect(int currentPlayer);
	int NuHou(CONTEXT_TIMELINE_1 *con);//[响应]【怒吼】：（主动攻击前发动①，移除1点【怒气】）本次攻击伤害额外+2；若未命中，你+1【知性】。
	int NuHouMiss(CONTEXT_TIMELINE_2_MISS *con);
	int using_NuHou;//是否在怒吼效果中
	int MingJingZhiShui(CONTEXT_TIMELINE_1 *con);//[响应]【明镜止水】：（主动攻击前发动①，移除4点【知性】）本次攻击对方不能应战。
	
	int JinDuanZhiLiHit(CONTEXT_TIMELINE_2_HIT *con);//[响应]A 【禁断之力】：【能量】×1 （主动攻击命中或未命中后发动②）弃掉你所有手牌【展示】，其中每有1张法术牌，你+1点【怒气】；若未命中②，其中每有1张水系牌，你+1点【知性】；若命中②，其中每有1张火系牌，本次攻击伤害额外+1，并对自己造成等同于火系牌数量的法术伤害③
	int JinDuanZhiLiMiss(CONTEXT_TIMELINE_2_MISS *con);

	int JingPiLiJie();//[被动]A 【精疲力竭】：（发动【禁断之力】后强制触发【强制】）【横置】额外+1【攻击行动】；持续到你的下个行动阶段开始，你的手牌上限恒定为4【恒定】。【精疲力竭】的效果结束时【重置】，并对自己造成3点法术伤害③。
	int SiDou(CONTEXT_TIMELINE_6_DRAWN *con);//[响应]D 【死斗】：【宝石】×1 （每当你承受法术伤害时发动⑥）你+3点【怒气】。 　*【怒气】和【知性】为勇者专有指示物，上限各为4。
	bool toRemoveTiaoXin;//挑衅前触发了虚弱跳过
	 
	
	
};
