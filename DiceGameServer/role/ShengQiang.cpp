#include "ShengQiang.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"



bool ShengQiang::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case TIAN_QIANG:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_1, TIAN_QIANG, respond);
			return true;
		case DI_QIANG:
			session->tryNotify(id, STATE_TIMELINE_2_HIT, DI_QIANG, respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

int ShengQiang::p_before_turn_begin(int &step, int currentPlayerID)
{
	used_ShengGuangQiYu = false;
	used_TianQiang = false;
	used_DiQiang = false;
	return GE_SUCCESS;
}

int ShengQiang::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	//天枪主动攻击可发动
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	if(step == STEP_INIT)
	{
		step = TIAN_QIANG;
	}
	if(step == TIAN_QIANG)
	{
		ret = TianQiang(con);
		if(toNextStep(ret)){
			step = STEP_DONE;
		}
	}
	return ret;
}
int ShengQiang::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con)
{
	int ret = GE_INVALID_STEP;
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	if(step == STEP_INIT)
	{
		step = DI_QIANG;
	}
	if(step == DI_QIANG)
	{
		ret = DiQiang(con);
		if(toNextStep(ret)){
			step = SHENG_JI;
		}
	}
	if(step == SHENG_JI)
	{
		ret = ShengJi(con);
		if(toNextStep(ret)){
			step = STEP_DONE;
		}
	}
	return ret;
}
int ShengQiang::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int cardID;
	int playerID = action->src_id();
	int dstID;
	CardEntity* card;
	PlayerEntity* dst;
	

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}
	switch(actionID)
	{
	case HUI_YAO:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		//不是自己的手牌                          || 不是对应的法术牌                
		if(GE_SUCCESS != checkOneHandCard(cardID) || ELEMENT_WATER != card->getElement()){
			return GE_INVALID_ACTION;
		}
		
		break;
	case CHENG_JIE:
		dstID = action->dst_ids(0);
		dst = engine->getPlayerEntity(dstID);
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		//engine->getPlayerEntity(dstID)->getCrossNum() != 2
		if(GE_SUCCESS != checkOneHandCard(cardID) || TYPE_MAGIC != card->getType() || 1 != action->dst_ids_size() || 0 == dst->getCrossNum() ){
			return GE_INVALID_ACTION;
		}

		
		break;
	case SHENG_GUANG_QI_YU:
		if(getGem() < 1){
			return GE_INVALID_ACTION;
		}
		
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}
int ShengQiang::p_magic_skill(int &step, Action* action)
{
	int ret;
	switch(action->action_id())
	{
	case HUI_YAO:
		ret = HuiYao(step, action);
		if(GE_URGENT == ret){
			step = HUI_YAO;
		}
		else if (GE_SUCCESS == ret){
			step = STEP_DONE;
		}
		
		break;
	case CHENG_JIE:
		ret = ChengJie(step, action);
		if(GE_URGENT == ret){
			step = CHENG_JIE;
		}
		else if (GE_SUCCESS == ret){
			step = STEP_DONE;
		}
		
		break;
	case SHENG_GUANG_QI_YU:
		ret = ShengGuangQiYu(action);
		if (GE_SUCCESS == ret){
			step = STEP_DONE;
		}
		
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}
int ShengQiang::HuiYao(int &step, Action* action)
{
	list<int> dstIDs;
	int cardID = action->card_ids(0);
	PlayerEntity * dstPlayer = engine->getPlayerEntity(id);
	dstPlayer = dstPlayer->getPost();
	while(dstPlayer->getID() != id)
	{
		dstIDs.push_back(dstPlayer->getID());
		dstPlayer = dstPlayer->getPost();	
	}
	dstIDs.push_back(id);
	
	if(step != HUI_YAO)
	{
		SkillMsg skill_msg;
		Coder::skillNotice(id, dstIDs, HUI_YAO, skill_msg);
		engine->sendMessage(-1, MSG_SKILL, skill_msg);
		CardMsg show_card;
		Coder::showCardNotice(id, 1, cardID, show_card);
		engine->sendMessage(-1, MSG_CARD, show_card);
		engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, HUI_YAO, true);
		//插入了新状态，请return GE_URGENT
		
		return GE_URGENT;
	}
	else
	{
		GameInfo update_info;
		list<int>::iterator it;
		for (it = dstIDs.begin(); it != dstIDs.end(); it++)
		{
			dstPlayer = engine->getPlayerEntity(*it);
			dstPlayer->addCrossNum(1);
			Coder::crossNotice(*it, dstPlayer->getCrossNum(), update_info);			
		}
		engine->sendMessage(-1, MSG_GAME, update_info);
		addAction(ACTION_ATTACK, HUI_YAO);

		return GE_SUCCESS;
	}
}
int ShengQiang::ChengJie(int &step, Action* action)
{
	int dstID = action->dst_ids(0);
	int cardID = action->card_ids(0);
	PlayerEntity * srcPlayer = engine->getPlayerEntity(id);
	PlayerEntity * dstPlayer = engine->getPlayerEntity(dstID);
	if(step != CHENG_JIE)
	{
		SkillMsg skill_msg;
		Coder::skillNotice(id, dstID, CHENG_JIE, skill_msg);
		engine->sendMessage(-1, MSG_SKILL, skill_msg);
		CardMsg show_card;
		Coder::showCardNotice(id, 1, cardID, show_card);
		engine->sendMessage(-1, MSG_CARD, show_card);
		engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, CHENG_JIE, true);
		//插入了新状态，请return GE_URGENT
		return GE_URGENT;
	}
	else
	{
		dstPlayer->subCrossNum(1);
		GameInfo update_info;
		Coder::crossNotice(dstID, dstPlayer->getCrossNum(), update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
		srcPlayer->addCrossNum(1);
		Coder::crossNotice(id, srcPlayer->getCrossNum(), update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
		addAction(ACTION_ATTACK, CHENG_JIE);
		return GE_SUCCESS;
	}
}
int ShengQiang::ShengGuangQiYu(Action* action)
{
	SkillMsg skill_msg;
	PlayerEntity * srcPlayer = engine->getPlayerEntity(id);
	Coder::skillNotice(id, id, SHENG_GUANG_QI_YU, skill_msg);
	
	//更新能量
	GameInfo update_info;
	setGem(--gem);
	Coder::energyNotice(id, gem, crystal, update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	srcPlayer->addCrossNum(2,5);
	Coder::crossNotice(id, srcPlayer->getCrossNum(), update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);

	addAction(ACTION_ATTACK, SHENG_GUANG_QI_YU);
	used_ShengGuangQiYu = true;
	return GE_SUCCESS;
}
int ShengQiang::TianQiang(CONTEXT_TIMELINE_1 *con)
{
	int ret;
	int srcID = con->attack.srcID;
	PlayerEntity * srcPlayer = engine->getPlayerEntity(srcID);
	int dstID = con->attack.dstID;
	int cardID = con->attack.cardID;
	CardEntity* card = getCardByID(cardID);
	if(srcID != id || !con->attack.isActive){
		return GE_SUCCESS;
	}
	if(engine->getPlayerEntity(id)->getCrossNum() < 2|| used_ShengGuangQiYu){
		return GE_SUCCESS;
	}
	//满足发动条件，询问客户端是否发动
	CommandRequest cmd_req;
	Coder::askForSkill(id, TIAN_QIANG, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(srcID, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0) == 1){
				srcPlayer->subCrossNum(2);
				GameInfo update_info;
				Coder::crossNotice(srcID, srcPlayer->getCrossNum(), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
				network::SkillMsg skill;
				Coder::skillNotice(id, dstID, TIAN_QIANG, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				con->hitRate = RATE_NOREATTACK;
				used_TianQiang = true;
			}
			return GE_SUCCESS;
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}
int ShengQiang::DiQiang(CONTEXT_TIMELINE_2_HIT *con)
{
	int ret;
	int srcID = con->attack.srcID;
	PlayerEntity * srcPlayer = engine->getPlayerEntity(srcID);
	int dstID = con->attack.dstID;
	int cardID = con->attack.cardID;
	CardEntity* card = getCardByID(cardID);
	if(srcID != id || !con->attack.isActive){
		return GE_SUCCESS;
	}
	if(engine->getPlayerEntity(id)->getCrossNum() < 1){
		return GE_SUCCESS;
	}
	//满足发动条件，询问客户端是否发动
	CommandRequest cmd_req;
	Coder::askForSkill(id, DI_QIANG, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(srcID, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(0 != respond->args(0) && respond->args(0) < 5 ){
				int crossUsed = respond->args(0);
				srcPlayer->subCrossNum(crossUsed);
				GameInfo update_info;
				Coder::crossNotice(srcID, srcPlayer->getCrossNum(), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
				network::SkillMsg skill;
				Coder::skillNotice(id, dstID, DI_QIANG, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				con->harm.point += crossUsed;
				used_DiQiang = true;
			}
			return GE_SUCCESS;
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int ShengQiang::ShengJi(CONTEXT_TIMELINE_2_HIT *con)
{
	int ret;
	int srcID = con->attack.srcID;
	PlayerEntity * srcPlayer = engine->getPlayerEntity(srcID);
	int dstID = con->attack.dstID;
	int cardID = con->attack.cardID;
	CardEntity* card = getCardByID(cardID);
	if(srcID != id){
		return GE_SUCCESS;
	}
	if(engine->getPlayerEntity(id)->getCrossNum() >= 3|| used_TianQiang || used_DiQiang){
		return GE_SUCCESS;
	}
	//满足发动条件，询问客户端是否发动
	CommandRequest cmd_req;
	Coder::askForSkill(id, TIAN_QIANG, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	SkillMsg skill;
	Coder::skillNotice(id, con->attack.dstID, SHENG_JI, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	srcPlayer->addCrossNum(1);
	GameInfo update_info;
	Coder::crossNotice(id, srcPlayer->getCrossNum(), update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	return GE_SUCCESS;
}