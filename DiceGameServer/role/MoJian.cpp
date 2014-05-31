#include "MoJian.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"


bool MoJian::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case AN_YING_NING_JU:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id,STATE_BOOT,AN_YING_NING_JU, respond);
			return true;

		case HEI_AN_ZHEN_CHAN:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id,STATE_TIMELINE_1,HEI_AN_ZHEN_CHAN, respond);  //用什么状态？？？
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

//统一在p_before_turn_begin 初始化各种回合变量
int  MoJian::p_before_turn_begin(int &step, int currentPlayerID) 
{
	used_XiuLuoLianZhan = false;
	used_HeiAnZhenChan = false;
	using_XiuLuoLianZhan=false;
	using_HeiAnZhenChan = false;

	return GE_SUCCESS; 
}

int MoJian::p_before_action(int &step, int currentPlayerID)
{
	if (currentPlayerID != id || !tap)
		return GE_SUCCESS;

	int ret = AnYingNingJuReset();
	if(toNextStep(ret) || ret == GE_URGENT){
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

//启动技 【暗影凝聚】
int MoJian::p_boot(int &step,int currentPlayerID)
{
	if (currentPlayerID != id)
		return GE_SUCCESS;
	step =AN_YING_NING_JU;
	int ret = AnYingNingJu();
	if(toNextStep(ret) || ret == GE_URGENT){
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

/**
【暗影流星】
**/
int MoJian::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int cardID;
	int playerID = action->src_id();
	CardEntity* card;

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}

	if(action->action_id()==AN_YING_LIU_XING&&tap)
	{
		//判断是否有两张法术牌
		if(action->card_ids_size()!=2)
		{
			return GE_INVALID_ACTION;
		}

		for(int i =0;i<action->card_ids_size();i++)
		{
			cardID = action->card_ids(i);
			card = getCardByID(cardID);
			if( TYPE_MAGIC != card->getType() || GE_SUCCESS != checkOneHandCard(cardID))
			{
				return GE_INVALID_ACTION;
			}
		}
		return GE_SUCCESS;

	}

	else 
		return GE_INVALID_ACTION;
}

int MoJian::p_magic_skill(int &step, Action* action)
{
	int ret;
	int actionID = action->action_id();
	if(action->action_id()==AN_YING_LIU_XING&&tap)
	{
		ret = AnYingLiuXing(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		return ret;
	}
	else
	{
		return GE_INVALID_ACTION;
	}

}

int MoJian::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con) 
{ 	
	//获取当前英雄的宝石数，判断宝石数是否大于1,且进行【黑暗震颤】【回合限度】
	if (con->attack.srcID == id && con->attack.isActive && gem > 0 && !used_HeiAnZhenChan)
	{
		// 潜行不能应战
		step =HEI_AN_ZHEN_CHAN;
		int ret =HeiAnZhenChanNoReattack(con);
		if(toNextStep(ret) || ret == GE_URGENT){
			//全部走完后，请把step设成STEP_DONE	
			step = STEP_DONE;
		}
		return  ret;
	}
	else 
		return  GE_SUCCESS;	
}


//【暗影之力】及【黑暗震颤】补拍
int MoJian::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con)
{

	int ret = GE_INVALID_STEP;
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	if(step == STEP_INIT || step == AN_YING_ZHI_LI){
		//初始化step
		step = AN_YING_ZHI_LI;
		ret = AnYingZhiLi(con);
		if(toNextStep(ret)){
			if(using_HeiAnZhenChan)
				step =HEI_AN_ZHEN_CHAN_BU_PAI;
			else 
				step=STEP_DONE;
		}			
	}
	if(step == HEI_AN_ZHEN_CHAN_BU_PAI){

		ret =HeiAnZhenChanBuPai(con);  //命中则补手牌
		if(toNextStep(ret) || ret == GE_URGENT){
			//全部走完后，请把step设成STEP_DONE	
			step = STEP_DONE;
		}
	}
	return ret;
}

int MoJian::p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con)
{
	using_HeiAnZhenChan = false;
	return GE_SUCCESS;
}

//所有额外行动，都是集中到一个地方询问，而不是每个都问一遍
int MoJian::p_after_attack(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	if(playerID != id){
		return GE_SUCCESS;
	}
	if(step == STEP_INIT || step == XIU_LUO_LIAN_ZHAN){
		//初始化step
		step = XIU_LUO_LIAN_ZHAN;
		ret =XiuLuoLianZhan(playerID);
		if(toNextStep(ret)){
			step = STEP_DONE;;
		}			
	}
	return ret;
}

/**
【修罗连斩】：只接受火系攻击
**/  

int  MoJian::v_attack(int cardID, int dstID, bool realCard)
{
	if(using_XiuLuoLianZhan){
		CardEntity* card = getCardByID(cardID);
		if(card->getElement() != ELEMENT_FIRE){
			return GE_INVALID_ACTION;
		}
	}
	//通过角色相关的检测，其他基本检测交给底层
	return PlayerEntity::v_attack(cardID, dstID, realCard);
}

/**
【暗影抗拒】：本角色行动阶段法术牌不可用
**/
int MoJian::v_additional_action(int chosen)
{
	switch(chosen)
	{
	case XIU_LUO_LIAN_ZHAN:
		//回合限定
		if(used_XiuLuoLianZhan){
			return GE_INVALID_ACTION;
		}
		break;
	}
	//通过角色相关的检测，基本检测交给底层
	return PlayerEntity::v_additional_action(chosen);
}

int MoJian::p_additional_action(int chosen)
{
	GameInfo update_info;
	switch(chosen)
	{
	case XIU_LUO_LIAN_ZHAN:
		used_XiuLuoLianZhan = true;
		using_XiuLuoLianZhan = true;
		break;
	default:
		using_XiuLuoLianZhan = false;
	}
	//做完角色相关的操作，扣除额外行动交给底层
	return PlayerEntity::p_additional_action(chosen);
}

/***
【暗影抗拒】：法术牌不可用
***/

//魔弹：非本人
int MoJian::v_missile(int cardID, int dstID, bool realCard)
{
	if(realCard){
		int ret;
		if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
			return ret;
		}
	}
	return GE_INVALID_CARDID;
}

int MoJian::v_shield(int cardID, PlayerEntity* dst)
{
	int ret;
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
		return ret;
	}
	return GE_INVALID_CARDID;
}

int MoJian::v_weaken(int cardID, PlayerEntity* dst)
{
	int ret;
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
		return ret;
	}
	return GE_INVALID_CARDID;
}

/***
【修罗连斩】
***/
int  MoJian::XiuLuoLianZhan(int playerID)
{
	//是不是魔剑      || 有没有手牌        || 回合限定      || 已经算上【修罗连斩】了
	if(playerID != id || handCards.empty() || used_XiuLuoLianZhan || containsAction(XIU_LUO_LIAN_ZHAN)){
		return GE_SUCCESS;
	}
	addAction(ACTION_ATTACK, XIU_LUO_LIAN_ZHAN);  //额外行动
	return GE_SUCCESS;
}
/*
【暗影之力】  命中的时候加一点伤害  
*/
int MoJian::AnYingZhiLi(CONTEXT_TIMELINE_2_HIT *con)
{
	//在【暗影形态】下 发动的所有攻击伤害+1
	if(tap) {	
		int ret;
		int srcID = con->attack.srcID;
		int dstID = con->attack.dstID;
		if(srcID != id){
			return GE_SUCCESS;
		}
		SkillMsg skill;
		Coder::skillNotice(id, con->attack.dstID, AN_YING_ZHI_LI, skill);
		engine->sendMessage(-1, MSG_SKILL, skill);
		con->harm.point += 1;
	}
	return GE_SUCCESS;
}

/*
【暗影凝聚】
*/
int MoJian::AnYingNingJu()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, AN_YING_NING_JU, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if (respond->args(0) == 1)  //自己定义
			{
				network::SkillMsg skill;
				Coder::skillNotice(id, id, AN_YING_NING_JU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				tap = true;
				GameInfo game_info;
				Coder::tapNotice(id, true, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				HARM harm;
				harm.type=HARM_MAGIC;
				harm.point=1;
				harm.srcID=id;
				harm.cause=AN_YING_NING_JU;				
				engine->setStateTimeline3(id, harm);
				return GE_URGENT;
			}
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

/*
回合开始：【暗影形态】--》【正常状态】
*/
int MoJian::AnYingNingJuReset()
{
	tap = false;
	//由 暗影形态 回到 正常形态
	GameInfo game_info;
	Coder::tapNotice(id, false, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);

	return GE_SUCCESS;
}

int MoJian::HeiAnZhenChanNoReattack(CONTEXT_TIMELINE_1 *con)
{
	int dstID = con->attack.dstID;
	CommandRequest cmd_req;
	Coder::askForSkill(id, HEI_AN_ZHEN_CHAN, cmd_req);

	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if (respond->args(0) == 1)
			{
				network::SkillMsg skill;
				Coder::skillNotice(id, dstID,HEI_AN_ZHEN_CHAN, skill);  //gaidong
				engine->sendMessage(-1, MSG_SKILL, skill);

				//更新能量信息
				gem -= 1;
				GameInfo game_info;
				Coder::energyNotice(id, gem, crystal, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);

				//攻击不能应战
				used_HeiAnZhenChan=true;
				using_HeiAnZhenChan=true;
				con->hitRate = RATE_NOREATTACK;

			}
		}
		return ret;
	}

	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
	return GE_SUCCESS;
}

//命中则补手牌
int MoJian::HeiAnZhenChanBuPai(CONTEXT_TIMELINE_2_HIT* con)
{
	//获取需要补充的手牌数量
	int num=getHandCardMax()-getHandCardNum();
	//补手牌，待补充
	vector<int> cards;
	HARM harm;
	harm.srcID = id;
	harm.type = HARM_NONE;
	harm.point =num;                  //最大手牌减去当前手牌
	harm.cause = HEI_AN_ZHEN_CHAN;
	engine->setStateMoveCardsToHand(-1, DECK_PILE, con->harm.srcID, DECK_HAND, num, cards, harm, false);
	SkillMsg skill_msg;
	Coder::skillNotice(id, con->harm.srcID, HEI_AN_ZHEN_CHAN, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	using_HeiAnZhenChan=false;  //当前【黑暗震颤】环节处理完毕
	return GE_URGENT;
}

int MoJian::AnYingLiuXing(Action* action)
{
	vector<int> cardIDs;
	vector<int> cards;
	int cardNum= action->card_ids_size();
	int  dstID = action->dst_ids(0);

	PlayerEntity *dstPlayer = engine->getPlayerEntity(dstID);

	for(int i = 0; i < cardNum;i ++)
	{
		cardIDs.push_back(action->card_ids(i));
	}
	//宣告技能
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, AN_YING_LIU_XING, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	HARM  harm;
	harm.type = HARM_MAGIC;
	harm.point = 2;
	harm.srcID = id;
	harm.cause = AN_YING_LIU_XING;
	engine->setStateTimeline3(dstID, harm);
	CardMsg show_card;
	Coder::showCardNotice(id, 2, cardIDs, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs, id, AN_YING_LIU_XING, true);//弃牌，伤害

	//插入了新状态，请return GE_URGENT
	return GE_URGENT;
}