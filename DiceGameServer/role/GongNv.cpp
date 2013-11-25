#include "GongNv.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"


bool GongNv::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case  JING_ZHUN_SHE_JI:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_1,  JING_ZHUN_SHE_JI, respond);
			return true;
		case GUAN_CHUAN_SHE_JI:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_2_MISS, GUAN_CHUAN_SHE_JI, respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

int GongNv::p_before_turn_begin(int &step, int currentPlayerID) 
{
	used_JingZhunSheJi = false;
	step = STEP_DONE;
	return GE_SUCCESS; 
}

//闪电箭：雷系攻击或应战无法被应战
int GongNv::ShanDianJian(CONTEXT_TIMELINE_1 *con)
{
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	if(getCardByID(con->attack.cardID)->getElement() == ELEMENT_THUNDER){
		con->hitRate = RATE_NOREATTACK;
		
		//没有技能宣告
		
	}
	return GE_SUCCESS;
}

//贯穿射击：主动攻击未命中，弃1法术牌造成2法术伤害
int GongNv::GuanChuanSheJi(CONTEXT_TIMELINE_2_MISS *con)
{
	int ret;
	int srcID = con->srcID;
	int dstID = con->dstID;
	if(srcID != id || con->isActive == false){
		return GE_SUCCESS;
	}

	
	// 等待玩家响应
	CommandRequest cmd_req;
	Coder::askForSkill(id, GUAN_CHUAN_SHE_JI, cmd_req);
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(srcID, reply)))
		{
			Respond* respond = (Respond*) reply;
			if(respond->args(0) == 1){
				
				//检查客户端返回的是否为魔法牌
				int cardID = respond->card_ids(0);
				CardEntity* card = getCardByID(cardID);
				if( card->getType() != TYPE_MAGIC ){
					return GE_SUCCESS;
				}
				
				//确认发动，宣告技能
				SkillMsg skill;
				Coder::skillNotice(id, dstID, GUAN_CHUAN_SHE_JI, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				//制造伤害
				HARM harm;
				harm.cause = GUAN_CHUAN_SHE_JI;
				harm.point = 2;
				harm.srcID = id;
				harm.type = HARM_MAGIC;
				engine->setStateTimeline3(dstID, harm);
				
				//丢弃魔法牌
				CardMsg show_card;
				Coder::showCardNotice(id, 1, cardID, show_card);
				engine->sendMessage(-1, MSG_CARD, show_card);
				engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, GUAN_CHUAN_SHE_JI, true);
				//返回URGENT，终止目前状态，执行状态栈顶状态
				return GE_URGENT;

			}		
		}
		return ret;
	}else{
		return GE_TIMEOUT;
	}

	return GE_SUCCESS;
}

//闪光陷阱：牌技能：2点法术伤害
int GongNv::ShanGuangXianJing(Action* action)
{

	
	int cardID = action->card_ids(0);
	int dstID = action->dst_ids(0);
	CardEntity* card = getCardByID(cardID);
	//检查牌是否为对应技能牌
	if( !card->checkSpeciality(SHAN_GUANG_XIAN_JIN)  ){
		return GE_SUCCESS;
	}
	
	//宣告技能
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, SHAN_GUANG_XIAN_JIN, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	
	//展示手牌
	CardMsg show_card;
	Coder::showCardNotice(id, 1, cardID, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);

	//制造伤害
	HARM harm;
	harm.cause = SHAN_GUANG_XIAN_JIN;
	harm.point = 2;
	harm.srcID = id;
	harm.type = HARM_MAGIC;
	engine->setStateTimeline3(dstID, harm);

	//移除闪光陷阱牌
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, GUAN_CHUAN_SHE_JI, true);
	//插入了新状态，返回URGENT，终止目前状态，执行状态栈顶状态
	return GE_URGENT;
}

//精准射击：牌技能：攻击强制命中，1点伤害，可选触发
int GongNv::JingZhunSheJiAttack(CONTEXT_TIMELINE_1 *con)
{

	int ret;
	int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	int cardID = con->attack.cardID;
	//  精准射击应战也可发动
	if(srcID != id ){
		return GE_SUCCESS;
	}
	
	CardEntity* card = getCardByID(cardID);
	if( !card->checkSpeciality(JING_ZHUN_SHE_JI)  ){
		return GE_SUCCESS;
	}

	//等待玩家响应
	CommandRequest cmd_req;
	Coder::askForSkill(id, JING_ZHUN_SHE_JI, cmd_req);
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(srcID, reply)))
		{
			Respond* respond = (Respond*) reply;
			if(respond->args(0) == 1){
				//确认发动，宣告技能
				SkillMsg skill;
				Coder::skillNotice(id, dstID, JING_ZHUN_SHE_JI, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				used_JingZhunSheJi = true;
				con->hitRate = RATE_NOMISS;
			}
		}
		return ret;
	}else{
		return GE_TIMEOUT;
	}

}

//精准射击：牌技能：攻击强制命中，1点伤害，可选触发
int GongNv::JingZhunSheJiDamage(CONTEXT_TIMELINE_2_HIT *con)
{
	
	if(used_JingZhunSheJi){
		con->harm.point = 1;
	}
	
	return GE_SUCCESS;

}

//狙击：1水晶，目标手牌补到5张，+1攻击行动
int GongNv::JuJi(Action *action)
{ 
	int dstID = action->dst_ids(0);
	
	//宣告技能
	SkillMsg skill;
	Coder::skillNotice(id, dstID, JU_JI, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);

	//扣除水晶,此处优先扣除水晶 //TO VERIFY 客户端是否能够选择扣除水晶还是宝石 args[0]
	GameInfo update_info;
	if(crystal>0){
			setCrystal(--crystal);
	}
	else{
		setGem(--gem);
	}

	Coder::energyNotice(id, gem, crystal, update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	

	//额外攻击行动
	addAction(ACTION_ATTACK, JU_JI);

	//补牌，若手牌已有五张或以上不进行操作
	HARM move;
	move.cause = JU_JI;
	move.point = 5 -  (engine->getPlayerEntity(dstID))->getHandCardNum();
	if(move.point>0){
		move.srcID = id;
		move.type = HARM_NONE;
		int ret = engine->setStateMoveCardsToHand(-1, DECK_PILE, dstID, DECK_HAND, move.point, vector< int >(), move,false);
	//插入了新状态，返回URGENT，终止目前状态，执行状态栈顶状态
		return GE_URGENT;
	}

	
	return GE_SUCCESS;
}

int GongNv::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	while(STEP_DONE != step)
	{
		switch(step)
		{
		case STEP_INIT:
			step = SHAN_DIAN_JIAN;

		case SHAN_DIAN_JIAN:
				// 检测是否雷系攻击
			ret = ShanDianJian(con);
			step = JING_ZHUN_SHE_JI;
			break;

		case JING_ZHUN_SHE_JI:

			ret = JingZhunSheJiAttack(con);
			if(toNextStep(ret)){
				step = STEP_DONE;
			}
				break;

		default:
			return GE_INVALID_STEP;
		}
	}

	return ret;
}

int GongNv::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con)
{
	int ret = GE_INVALID_STEP;
	if(con->attack.srcID != id){//精准射击应战也可发动，只需检查发起者
		return GE_SUCCESS;
	}
	while(STEP_DONE != step)
	{
		switch(step)
		{
		case STEP_INIT:
			step = JING_ZHUN_SHE_JI;
			break;

		case JING_ZHUN_SHE_JI:
			ret = JingZhunSheJiDamage(con);
			step =  STEP_DONE; 
			break;

		default:
			return GE_INVALID_STEP;
		}
	}
	return ret;
}

int GongNv::p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con){

	int ret = GE_INVALID_STEP;
	int srcID = con->srcID;
	int dstID = con->dstID;
	bool isActive = con->isActive;
	if(srcID != id || isActive == false){//贯穿射击只在主动攻击时发动
		return GE_SUCCESS;
	}

	while(STEP_DONE != step){
		switch(step)
		{
		case STEP_INIT:
			step = GUAN_CHUAN_SHE_JI;
			break;

		case GUAN_CHUAN_SHE_JI:
			ret = GuanChuanSheJi(con);
			if(toNextStep(ret)|| ret == GE_URGENT){
				step = STEP_DONE;
			}
			break;
		default:
			return GE_INVALID_STEP;
		}
	}
	return ret;
	
}

int GongNv::p_after_attack(int &step, int playerID)
{
	//攻击过程完毕重置精准射击flag
	if(id == playerID){
		used_JingZhunSheJi = false;
	}
	return GE_SUCCESS;
}

//P_MAGIC_SKILL只有发起者执行，故不需要判断发起者，法术技能验证均在v_MAGIC_SKILL中
int GongNv::p_magic_skill(int &step, Action *action)
{

	int ret;
	int actionID = action->action_id();
	switch(actionID)
	{
		case SHAN_GUANG_XIAN_JIN:
			ret = ShanGuangXianJing(action);
			step = STEP_DONE;
			
			break;
		case JU_JI:
			ret = JuJi(action);
			step = STEP_DONE;
			
			break;
		default:
			return GE_INVALID_ACTION;
	}
	return ret;
}

int GongNv::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int playerID = action->src_id();
	int cardID = 0;
	CardEntity * card = NULL ;
	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}
	switch(actionID)
	{
		case SHAN_GUANG_XIAN_JIN:
			cardID = action->card_ids(0);
		    card = getCardByID(cardID);
			if(GE_SUCCESS != checkOneHandCard(cardID) || !card->checkSpeciality(actionID)){
				return GE_INVALID_ACTION;
			}
			return  GE_SUCCESS;

		case JU_JI:
			if(crystal == 0 && gem == 0){
				return  GE_INVALID_ACTION;
			}
			return GE_SUCCESS;
	}
	return  GE_INVALID_ACTION;
}
