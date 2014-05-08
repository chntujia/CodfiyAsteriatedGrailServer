#include "ShenGuan.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool ShenGuan::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case SHEN_SHENG_QI_YUE:
			session->tryNotify(id, STATE_BOOT, SHEN_SHENG_QI_YUE, respond);
			return true;
		case SHUI_ZHI_SHEN_LI_GIVE:
			session->tryNotify(id, STATE_MAGIC_SKILL, SHUI_ZHI_SHEN_LI_GIVE, respond);
			return true;
		case SHEN_SHENG_QI_SHI:
			session->tryNotify(id, STATE_AFTER_SPECIAL, SHEN_SHENG_QI_SHI, respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

int ShenGuan::p_boot(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	step = SHEN_SHENG_QI_YUE;
	if (currentPlayerID != id || getEnergy() == 0 || crossNum == 0)
		return GE_SUCCESS;
	ret = ShenShengQiYue();
	if(toNextStep(ret)){
		step = STEP_DONE;
	}
	return ret;
}

int ShenGuan::p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con)
{
	int ret = GE_INVALID_STEP;
	step = SHENG_SHI_SHOU_HU;
	ret = ShengShiShouHu(con);
	if(toNextStep(ret))	{
		step = STEP_DONE;
	}
	return ret;
}

int ShenGuan::p_after_special(int &step, int srcID)
{
	int ret = GE_INVALID_STEP;
	step = SHEN_SHENG_QI_SHI;
	ret = ShenShengQiShi(step, srcID);
	if(toNextStep(ret))	{
		step = STEP_DONE;
	}
	return ret;
}

int ShenGuan::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int cardID;
	int playerID = action->src_id();
	CardEntity* card;
	PlayerEntity* dst;

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}
	switch(actionID)
	{
	case SHEN_SHENG_QI_FU:
		if(action->card_ids_size() != 2)
		{
			return GE_INVALID_ACTION;
		}
		for(int i =0;i<action->card_ids_size();i++)
		{
			cardID = action->card_ids(i);
			card = getCardByID(cardID);
			if(TYPE_MAGIC != card->getType() || GE_SUCCESS != checkOneHandCard(cardID))
			{
				return GE_INVALID_ACTION;
			}
		}
		break;
	case SHUI_ZHI_SHEN_LI:
		
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		dst = engine->getPlayerEntity(action->dst_ids(0));
		if(GE_SUCCESS != checkOneHandCard(cardID) || card->getElement() != ELEMENT_WATER || color != dst->getColor() || action->dst_ids_size() != 1){
			return GE_INVALID_ACTION;
		}
		break;
	case SHEN_SHENG_LING_YU:
		dst = engine->getPlayerEntity(action->dst_ids(0));
		if(this->getHandCardNum()>2){
			if (action->card_ids_size()!=2){
				return GE_INVALID_ACTION;
			}
		}
		else if( getHandCardNum() != action->card_ids_size()){
			return GE_INVALID_ACTION;
		}
		if(action->args(0) == 1){
			if (getEnergy() < 1 || crossNum < 1){
				return GE_INVALID_ACTION;
			}
		}
		else if(getEnergy() < 1 || dst->getColor() != color){
			return GE_INVALID_ACTION;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int ShenGuan::p_magic_skill(int &step, Action* action)
{
	//p_magic_skill不同于别的触发点，有且只有一个匹配，因此每一个技能完成时，务必把step设为STEP_DONE
	int ret;
	switch(action->action_id())
	{
	case SHEN_SHENG_QI_FU:
		ret = ShenShengQiFu(step, action);
		break;
	case SHUI_ZHI_SHEN_LI:
		if( step != SHUI_ZHI_SHEN_LI_GIVE && step !=SHUI_ZHI_SHEN_LI_CROSS)
		{
			step = SHUI_ZHI_SHEN_LI;
		}
		ret = ShuiZhiShenLi(step, action);
		break;
	case SHEN_SHENG_LING_YU:
		ret = ShenShengLingYu(step, action);
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}
int ShenGuan::ShenShengQiYue()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, SHEN_SHENG_QI_YUE, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动			
			int crossNum = respond->args(0);			
			if (crossNum > 0)
			{
				int dstID = respond->dst_ids(0);
				PlayerEntity *dstPlayer = engine->getPlayerEntity(dstID);
				network::SkillMsg skill_msg;
				Coder::skillNotice(id, dstID, SHEN_SHENG_QI_YUE, skill_msg);
				engine->sendMessage(-1, MSG_SKILL, skill_msg);
				if(crystal>0){
					setCrystal(--crystal);
				}
				else{
					setGem(--gem);
				}
				subCrossNum(crossNum);
				dstPlayer->addCrossNum(crossNum, 4);
				GameInfo game_info;
				Coder::energyNotice(id, gem, crystal, game_info);
				Coder::crossNotice(id, getCrossNum(), game_info);
				Coder::crossNotice(dstID, dstPlayer->getCrossNum(), game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				return GE_SUCCESS;
			}
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int ShenGuan::ShenShengQiFu(int &step, Action* action)
{
	if(step != SHEN_SHENG_QI_FU)
	{
		vector<int> cardIDs;
		cardIDs.push_back(action->card_ids(0));
		cardIDs.push_back(action->card_ids(1));
		network::SkillMsg skill;
		Coder::skillNotice(id, id, SHEN_SHENG_QI_FU, skill);
		engine->sendMessage(-1, MSG_SKILL, skill);
		CardMsg show_card;
		Coder::showCardNotice(id, 2, cardIDs, show_card);
		engine->sendMessage(-1, MSG_CARD, show_card);
		engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, 2, cardIDs, id, SHEN_SHENG_QI_FU, true);
		//插入了新状态，请return GE_URGENT
		step = SHEN_SHENG_QI_FU;
		return GE_URGENT;
	}
	else
	{
		addCrossNum(2);
		GameInfo update_info;
		Coder::crossNotice(id, getCrossNum(), update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
		step = STEP_DONE;
		return GE_SUCCESS;
	}
}

int ShenGuan::ShuiZhiShenLi(int &step, Action* action)
{
	int cardID = action->card_ids(0);
	int dstID = action->dst_ids(0);
	PlayerEntity *dstPlayer = engine->getPlayerEntity(dstID);
	if(step == SHUI_ZHI_SHEN_LI)
	{
		SkillMsg skill_msg;
		Coder::skillNotice(id, dstID, SHUI_ZHI_SHEN_LI, skill_msg);
		engine->sendMessage(-1, MSG_SKILL, skill_msg);
		CardMsg show_card;
		Coder::showCardNotice(id, 1, cardID, show_card);
		engine->sendMessage(-1, MSG_CARD, show_card);
		engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, SHUI_ZHI_SHEN_LI, true);
		step = SHUI_ZHI_SHEN_LI_GIVE;
		return GE_URGENT;
	}
	if(step == SHUI_ZHI_SHEN_LI_GIVE)
	{
		if(this->getHandCardNum() == 0)
		{
			step = SHUI_ZHI_SHEN_LI_CROSS;
			return GE_SUCCESS;
		}
		else
		{
			HARM shuishen;
			shuishen.cause = SHUI_ZHI_SHEN_LI;
			shuishen.point = 1;
			shuishen.srcID = id;
			shuishen.type = HARM_NONE;
			engine->pushGameState(new StateRequestHand(id, shuishen, dstID, DECK_HAND));
			step = SHUI_ZHI_SHEN_LI_CROSS;
			return GE_URGENT;
		}
	}
	if(step == SHUI_ZHI_SHEN_LI_CROSS)
	{
		addCrossNum(1);
		dstPlayer->addCrossNum(1);
		GameInfo update_info;
		Coder::crossNotice(id, getCrossNum(), update_info);
		Coder::crossNotice(dstID, dstPlayer->getCrossNum(), update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
		step = STEP_DONE;
		return GE_SUCCESS;
	}
}

int ShenGuan::ShenShengLingYu(int &step, Action *action)
{
	int dstID = action->dst_ids(0);
	vector<int> cardIDs;
	PlayerEntity *dstPlayer = engine->getPlayerEntity(dstID);
	if(step != SHEN_SHENG_LING_YU)
	{
		SkillMsg skill_msg;
		Coder::skillNotice(id, dstID, SHEN_SHENG_LING_YU, skill_msg);
		engine->sendMessage(-1, MSG_SKILL, skill_msg);
		if(crystal>0){
			setCrystal(--crystal);
		}
		else{
			setGem(--gem);
		}
		GameInfo game_info;
		Coder::energyNotice(id, gem, crystal, game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);
		int cardNum = (getHandCardNum()>2)? 2:getHandCardNum();
		for(int i = 0; i < cardNum; i ++)
		{
			cardIDs.push_back(action->card_ids(i));
		}
		if(cardNum > 0)
		{
			engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs, id, SHEN_SHENG_LING_YU, false);
			step = SHEN_SHENG_LING_YU;
			return GE_URGENT;
		}
		else
		{
			step = SHEN_SHENG_LING_YU;
			return GE_SUCCESS;
		}
	}
	else
	{
		if(action->args(0) == 1)
		{
			subCrossNum(1);
			GameInfo update_info;
			Coder::crossNotice(id, getCrossNum(), update_info);
			engine->sendMessage(-1, MSG_GAME, update_info);
			HARM harm;
			harm.point = 2;
			harm.srcID = id;
			harm.type = HARM_MAGIC;
			harm.cause = SHEN_SHENG_LING_YU;
			engine->setStateTimeline3(dstID, harm);
			step = STEP_DONE;
			return GE_URGENT;
		}
		else
		{
			addCrossNum(2);
			dstPlayer->addCrossNum(1);
			GameInfo update_info;
			Coder::crossNotice(id, getCrossNum(), update_info);
			Coder::crossNotice(dstID, dstPlayer->getCrossNum(), update_info);
			engine->sendMessage(-1, MSG_GAME, update_info);
			step = STEP_DONE;
			return GE_SUCCESS;
		}
	}
}

int ShenGuan::ShengShiShouHu(CONTEXT_TIMELINE_4 *con)
{
	if (con->dstID == id && con->crossAvailable>0)
	{
		con->crossAvailable = 1;
		network::SkillMsg skill;
		Coder::skillNotice(id, id, SHENG_SHI_SHOU_HU, skill);
		engine->sendMessage(-1, MSG_SKILL, skill);
	}
	return GE_SUCCESS;
}

int ShenGuan::ShenShengQiShi(int &step, int srcID)
{
	if(srcID != id){
		return GE_SUCCESS;
	}
	CommandRequest cmd_req;
	Coder::askForSkill(id, SHEN_SHENG_QI_SHI, cmd_req);
	int ret;
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
				Coder::skillNotice(id, id, SHEN_SHENG_QI_SHI, skill);
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