#include "SiLing.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool SiLing::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case BU_XIU:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_AFTER_MAGIC, BU_XIU, respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

int SiLing::p_after_magic(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	
	if(playerID != id){
		return GE_SUCCESS;
	}
	step = BU_XIU;
	ret = BuXiu(playerID);
	if(toNextStep(ret)){
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int SiLing::p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con)
{
	int ret = GE_SUCCESS;
	step = SHENG_DU;
	int srcID = con->dstID;
	if (con->dstID == id && con->harm.type == HARM_ATTACK && con->crossAvailable>0)
	{
		// 圣渎
		ret = ShengDu(con);
	}
	step = STEP_DONE;
	return ret;
}

int SiLing::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int cardID;
	list<int> cardIDs;
	int element;
	int playerID = action->src_id();
	CardEntity* card;

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}
	switch(actionID)
	{
	case WEN_YI:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		//不是自己的手牌                          || 不是地系牌               
		if(GE_SUCCESS != checkOneHandCard(cardID) || card->getElement()!= ELEMENT_EARTH){
			return GE_INVALID_ACTION;
		}
		break;
	case SI_WANG_ZHI_CHU:
		if(action->card_ids_size() < 2 || action->args(0) < 2)
		{
			return GE_INVALID_ACTION;
		}
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		element = card->getElement();
		for(int i =0;i<action->card_ids_size();i++)
		{
			cardID = action->card_ids(i);
			card = getCardByID(cardID);
			if(element != card->getElement() || GE_SUCCESS != checkOneHandCard(cardID))
			{
				return GE_INVALID_ACTION;
			}
		}
		break;
	case MU_BEI_YUN_LUO:
		if(getGem() < 1){
			return GE_INVALID_ACTION;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int SiLing::p_magic_skill(int &step, Action* action)
{
	//p_magic_skill不同于别的触发点，有且只有一个匹配，因此每一个技能完成时，务必把step设为STEP_DONE
	int ret;
	switch(action->action_id())
	{
	case WEN_YI:
		ret = WenYi(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case SI_WANG_ZHI_CHU:
		ret = SiWangZhiChu(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case MU_BEI_YUN_LUO:
		ret = MuBeiYunLuo(step, action);
		if(GE_URGENT == ret){
			step = MU_BEI_YUN_LUO;
		}
		else if(GE_SUCCESS == ret) {
			step = STEP_DONE;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int SiLing::BuXiu(int playerID)
{
	if(playerID != id){
		return GE_SUCCESS;
	}
	if(used_SiWangZhiChu)
	{
		used_SiWangZhiChu = false;
		return GE_SUCCESS;
	}
	CommandRequest cmd_req;
	Coder::askForSkill(id, BU_XIU, cmd_req);
	int ret;
	int srcID = id;
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(srcID, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0) == 1){
				network::SkillMsg skill;
				Coder::skillNotice(id, id, BU_XIU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				int dstID = id;
				PlayerEntity * dstPlayer = engine->getPlayerEntity(dstID);
				dstPlayer->addCrossNum(1);
				GameInfo update_info;
				Coder::crossNotice(dstID, dstPlayer->getCrossNum(), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
			}
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int SiLing::ShengDu(CONTEXT_TIMELINE_4 *con)
{
	if (con->dstID == id && con->harm.type == HARM_ATTACK && con->crossAvailable>0)
	{
		con->crossAvailable = 0;
		network::SkillMsg skill;
		Coder::skillNotice(id, id, SHENG_DU, skill);
		engine->sendMessage(-1, MSG_SKILL, skill);
	}
	return GE_SUCCESS;

}

int SiLing::WenYi(Action *action)
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
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstIDs, WEN_YI, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	HARM harm;
	harm.type = HARM_MAGIC;
	harm.point = 1;
	harm.srcID = id;
	harm.cause = WEN_YI;
	list<int>::iterator it;
	dstIDs.reverse();
	for (it = dstIDs.begin(); it != dstIDs.end(); it++)
	{
		engine->setStateTimeline3(*it, harm);
	}
	CardMsg show_card;
	Coder::showCardNotice(id, 1, cardID, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, WEN_YI, true);
	return GE_URGENT;
}

int SiLing::SiWangZhiChu(Action* action)
{
	vector<int> cardIDs;
	int cardNum= action->card_ids_size();
	int dstID = action->dst_ids(0);
	int crossUsed = action->args(0);
	PlayerEntity *dstPlayer = engine->getPlayerEntity(dstID);
	PlayerEntity *self = engine->getPlayerEntity(id);
	for(int i = 0; i < cardNum;i ++)
	{
		cardIDs.push_back(action->card_ids(i));
	}
	used_SiWangZhiChu = true;
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, SI_WANG_ZHI_CHU, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	self->subCrossNum(crossUsed);
	GameInfo update_info;
	Coder::crossNotice(id, self->getCrossNum(), update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	CardMsg show_card;
	Coder::showCardNotice(id, cardNum, cardIDs, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	HARM chuShou;
	chuShou.type = HARM_MAGIC;
	chuShou.point = cardNum+crossUsed-3;
	chuShou.srcID = id;
	chuShou.cause = SI_WANG_ZHI_CHU;
	engine->setStateTimeline3(dstID, chuShou);
	engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs, id, SI_WANG_ZHI_CHU, true);
	//插入了新状态，请return GE_URGENT
	return GE_URGENT;
}

int SiLing::MuBeiYunLuo(int &step, Action *action)
{
	list<int> dstIDs;
	PlayerEntity * dstPlayer = engine->getPlayerEntity(id);
	if(step != MU_BEI_YUN_LUO)
	{
		dstPlayer = dstPlayer->getPost();
		while(dstPlayer->getID() != id)
		{
			dstIDs.push_back(dstPlayer->getID());
			dstPlayer = dstPlayer->getPost();
		}
		SkillMsg skill_msg;
		Coder::skillNotice(id, dstIDs, MU_BEI_YUN_LUO, skill_msg);
		engine->sendMessage(-1, MSG_SKILL, skill_msg);
		GameInfo gem_info;
		setGem(--gem);
		Coder::energyNotice(id, gem, crystal, gem_info);
		engine->sendMessage(-1, MSG_GAME, gem_info);
		HARM harm;
		harm.type = HARM_MAGIC;
		harm.point = 2;
		harm.srcID = id;
		harm.cause = MU_BEI_YUN_LUO;
		list<int>::iterator it;
		dstIDs.reverse();
		for (it = dstIDs.begin(); it != dstIDs.end(); it++)
		{
			engine->setStateTimeline3(*it, harm);
		}
		return GE_URGENT;
	}
	else
	{
		PlayerEntity *self = engine->getPlayerEntity(id);
		self->addCrossNum(1);
		GameInfo cross_info;
		Coder::crossNotice(id, self->getCrossNum(), cross_info);
		engine->sendMessage(-1, MSG_GAME, cross_info);
		return GE_SUCCESS;
	}
}