#include "WuNv.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool WuNv::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case XUE_ZHI_AI_SHANG:
			session->tryNotify(id, STATE_BOOT, XUE_ZHI_AI_SHANG, respond);
			return true;
		case XUE_ZHI_ZU_ZHOU_QI_PAI:
			session->tryNotify(id, STATE_MAGIC_SKILL, XUE_ZHI_ZU_ZHOU, respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

int WuNv::p_turn_begin(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	if (currentPlayerID != id || !tap)
		return GE_SUCCESS;
	ret = LiuXue(currentPlayerID);
	if(toNextStep(ret) || ret == GE_URGENT)
	{
	step = STEP_DONE;
	}
	return ret;
}

int WuNv::p_boot(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	if (currentPlayerID != id || tongShengID == -1)
		return GE_SUCCESS;
	ret = XueZhiAiShang(step, currentPlayerID);
	return ret;
}

int WuNv::p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con)
{
	int ret = GE_INVALID_STEP;
	step = XUE_ZHI_AI_SHANG;
	ret = XueZhiAiShangJudge(con);
	if(toNextStep(ret))
	{
	step = STEP_DONE;
	}
	return ret;
}

int WuNv::p_true_lose_morale(int &step, CONTEXT_LOSE_MORALE *con)
{
	int ret = GE_INVALID_STEP;
	ret = ToLiuXueXingTai(con);
	if(toNextStep(ret)||ret == GE_URGENT)
	{
	step = STEP_DONE;
	}
	return ret;
}

int WuNv::p_hand_change(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	if (playerID != id)
		return GE_SUCCESS;
	ret = ToPuTongXingTai(playerID);
	if(toNextStep(ret)||ret == GE_URGENT)
	{
	step = STEP_DONE;
	}
	return ret;
}

int WuNv::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int playerID = action->src_id();
	CardEntity* card;
	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}
	switch(actionID)
	{
	case TONG_SHENG_GONG_SI:
		if(tongShengID != -1)
		{
			return GE_INVALID_ACTION;
		}
		break;
	case NI_LIU:
		if(!tap || action->card_ids_size()!=2)
			return GE_INVALID_ACTION;
		break;
	case XUE_ZHI_BEI_MING:
		card = getCardByID(action->card_ids(0));
		if(GE_SUCCESS != checkOneHandCard(action->card_ids(0)) || !card->checkSpeciality(actionID) || action->args(0) <1 || action->args(0) >3){
			return GE_INVALID_ACTION;
		}
		break;
	case XUE_ZHI_ZU_ZHOU:
		if(getGem()<1){
			return GE_INVALID_ACTION;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int WuNv::p_magic_skill(int &step, Action* action)
{
	//p_magic_skill不同于别的触发点，有且只有一个匹配，因此每一个技能完成时，务必把step设为STEP_DONE
	int ret;
	switch(action->action_id())
	{
	case TONG_SHENG_GONG_SI:
		ret = TongShengGongSi(step, action);
		if(toNextStep(ret) || GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case NI_LIU:
		ret = NiLiu(step, action);
		break;
	case XUE_ZHI_BEI_MING:
		ret = XueZhiBeiMing(step, action);
		if(toNextStep(ret) || GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case XUE_ZHI_ZU_ZHOU:
		ret = XueZhiZuZhou(step, action);
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int WuNv::TongShengGongSi(int &step, Action *action)
{
	int dstID = action->dst_ids(0);
	network::SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, TONG_SHENG_GONG_SI, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	GameInfo update_info;
	PlayerEntity* dst = engine->getPlayerEntity(dstID);
	dst->addExclusiveEffect(EX_TONG_SHENG_GONG_SI);
	Coder::exclusiveNotice(dstID, dst->getExclusiveEffect(), update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	if(dstID ==id)
	{
		if(tap)
		{
			engine->setStateChangeMaxHand(id, false, false, 6,1);
		}
		else
		{
			engine->setStateChangeMaxHand(id, false, false, 6,-2);
		}
	}
	else
	{
		if(tap)
		{
			engine->setStateChangeMaxHand(dstID, false, false, 6,1);
			engine->setStateChangeMaxHand(id, false, false, 6,1);
		}
		else
		{
			engine->setStateChangeMaxHand(dstID, false, false, 6,-2);
			engine->setStateChangeMaxHand(id, false, false, 6,-2);
		}
	}
	vector<int> cards;
	HARM harm;
	harm.srcID = id;
	harm.type = HARM_NONE;
	harm.point = 2;
	harm.cause = TONG_SHENG_GONG_SI;
	int ret = engine->setStateMoveCardsToHand(-1, DECK_PILE, id, DECK_HAND, 2, cards, harm, false);
	tongShengID = dstID;
	return GE_URGENT;
}

int WuNv::XueZhiAiShang(int &step, int currentPlayerID)
{
	if(step != XUE_ZHI_AI_SHANG)
	{
		step = XUE_ZHI_AI_SHANG;
		CommandRequest cmd_req;
		Coder::askForSkill(id, XUE_ZHI_AI_SHANG, cmd_req);
		//有限等待，由UserTask调用tryNotify唤醒
		if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
		{
			void* reply;
			int ret;
			if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
			{
				Respond* respond = (Respond*) reply;
				//发动
				if (respond->args(0)==0)
				{
					step = STEP_DONE;
					return GE_SUCCESS;
				}
				else
				{
					aiShangChoose = respond->args(0);
					if(aiShangChoose == 1){
						aiShangDst = respond->dst_ids(0);
					}
					HARM harm;
					harm.point = 2;
					harm.srcID = id;
					harm.type = HARM_MAGIC;
					harm.cause = XUE_ZHI_AI_SHANG;
					engine->setStateTimeline3(id, harm);
					return GE_URGENT;
				}
			}
			return ret;
		}
		else{
			//超时啥都不用做
			step = STEP_DONE;
			return GE_TIMEOUT;
		}
	}
	else
	{
		if(aiShangChoose == 1)
		{
			int newDst = aiShangDst;
			network::SkillMsg skill_msg;
			Coder::skillNotice(id, newDst, XUE_ZHI_AI_SHANG, skill_msg);
			engine->sendMessage(-1, MSG_SKILL, skill_msg);
			GameInfo update_info;
			PlayerEntity* dst = engine->getPlayerEntity(tongShengID);
			dst->removeExclusiveEffect(EX_TONG_SHENG_GONG_SI);
			Coder::exclusiveNotice(tongShengID, dst->getExclusiveEffect(), update_info);
			dst = engine->getPlayerEntity(newDst);
			dst->addExclusiveEffect(EX_TONG_SHENG_GONG_SI);
			Coder::exclusiveNotice(newDst, dst->getExclusiveEffect(), update_info);
			engine->sendMessage(-1, MSG_GAME, update_info);
			//逆序压栈，转移后，转移前，伤害
			if(newDst !=id)
			{
				if(tap)
				{
					engine->setStateChangeMaxHand(newDst, false, false, 6,1);
				}
				else
				{
					engine->setStateChangeMaxHand(newDst, false, false, 6,-2);
				}
			}
			if(tongShengID !=id)
			{
				if(tap)
				{
					engine->setStateChangeMaxHand(tongShengID, false, false, 6,-1);
				}
				else
				{
					engine->setStateChangeMaxHand(tongShengID, false, false, 6,2);
				}
			}
			//记录新的同生对象
			tongShengID = newDst;
			step = STEP_DONE;
			return GE_URGENT;
		}
		else if(aiShangChoose==2)//取消同生共死
		{
			network::SkillMsg skill_msg;
			Coder::skillNotice(id, id, XUE_ZHI_AI_SHANG, skill_msg);
			engine->sendMessage(-1, MSG_SKILL, skill_msg);
			GameInfo update_info;
			PlayerEntity* dst = engine->getPlayerEntity(tongShengID);
			dst->removeExclusiveEffect(EX_TONG_SHENG_GONG_SI);
			Coder::exclusiveNotice(tongShengID, dst->getExclusiveEffect(), update_info);
			engine->sendMessage(-1, MSG_GAME, update_info);
			if(tongShengID ==id)
			{
				if(tap)
				{
					engine->setStateChangeMaxHand(id, false, false, 6,-1);
				}
				else
				{
					engine->setStateChangeMaxHand(id, false, false, 6,2);
				}
			}
			else
			{
				if(tap)
				{
					engine->setStateChangeMaxHand(tongShengID, false, false, 6,-1);
					engine->setStateChangeMaxHand(id, false, false, 6,-1);
				}
				else
				{
					engine->setStateChangeMaxHand(tongShengID, false, false, 6,2);
					engine->setStateChangeMaxHand(id, false, false, 6,2);
				}
			}
			//记录新的同生对象
			tongShengID = -1;
			step =STEP_DONE;
			return GE_URGENT;
		}
	}
}

int WuNv::NiLiu(int &step, Action* action)
{
	if(step != NI_LIU)
	{
		vector<int> cardIDs;
		cardIDs.push_back(action->card_ids(0));
		cardIDs.push_back(action->card_ids(1));
		network::SkillMsg skill;
		Coder::skillNotice(id, id, NI_LIU, skill);
		engine->sendMessage(-1, MSG_SKILL, skill);
		engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, 2, cardIDs, NI_LIU, false);
		//插入了新状态，请return GE_URGENT
		step = NI_LIU;
		return GE_URGENT;
	}
	else
	{
		addCrossNum(1);
		GameInfo update_info;
		Coder::crossNotice(id, getCrossNum(), update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
		step = STEP_DONE;
		return GE_SUCCESS;
	}
}

int WuNv::XueZhiZuZhou(int &step, Action* action)
{
	int dstID = action->dst_ids(0);
	PlayerEntity *dstPlayer = engine->getPlayerEntity(dstID);

	if(step != XUE_ZHI_ZU_ZHOU)
	{
		SkillMsg skill_msg;
		Coder::skillNotice(id, dstID, XUE_ZHI_ZU_ZHOU, skill_msg);
		engine->sendMessage(-1, MSG_SKILL, skill_msg);
		setGem(--gem);
		GameInfo game_info;
		Coder::energyNotice(id, gem, crystal, game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);

		HARM harm;
		harm.point = 2;
		harm.srcID = dstID;
		harm.type = HARM_MAGIC;
		harm.cause = XUE_ZHI_ZU_ZHOU;
		engine->setStateTimeline3(dstID, harm);
		
		step = XUE_ZHI_ZU_ZHOU;
		return GE_URGENT;
	}
	else
	{
		if(this->getHandCardNum() == 0)
		{
			step = STEP_DONE;
			return GE_SUCCESS;
		}
		else
		{
			int cardNum = (getHandCardNum()>3)? 3:getHandCardNum();
			HARM qipai;
			qipai.point = cardNum;
			qipai.srcID = id;
			qipai.type = HARM_NONE;
			qipai.cause = XUE_ZHI_ZU_ZHOU;
			engine->pushGameState(new StateRequestHand(id, qipai, -1, DECK_DISCARD, false, false));
			return GE_URGENT;
		}
	}
}

int WuNv::XueZhiBeiMing(int &step, Action *action)
{
	int dstID = action->dst_ids(0);
	int cardID = action->card_ids(0);
	int harmNum = action->args(0);
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, XUE_ZHI_BEI_MING, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
		
	HARM harm;
	harm.point = harmNum;
	harm.srcID = id;
	harm.type = HARM_MAGIC;
	harm.cause = XUE_ZHI_BEI_MING;
	engine->setStateTimeline3(id, harm);
	engine->setStateTimeline3(dstID, harm);
	CardMsg show_card;
	Coder::showCardNotice(id, 1, cardID, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, XUE_ZHI_BEI_MING, true);
	return GE_URGENT;
}

int WuNv::XueZhiAiShangJudge(CONTEXT_TIMELINE_4 *con)
{
	if (con->dstID == id && con->crossAvailable>1)
	{
		PlayerEntity *self = engine->getPlayerEntity(id);
		if(self->getHandCardNum()==0 && getGem()<1)
		{
			con->crossAvailable = 1;
		}
		else
		{
			bool allLight = true;
			list<int>::iterator card = self->getHandCards().begin();
			for(;card != self->getHandCards().end(); card ++)
			{
				if(getCardByID(*card)->getElement() != ELEMENT_LIGHT){
					allLight = false;
				}
			}
			if(allLight){
				con->crossAvailable = 1;
			}
		}
	}
	return GE_SUCCESS;
}

int WuNv::ToLiuXueXingTai(CONTEXT_LOSE_MORALE *con)
{
	if(con->dstID == id && !tap && con->howMany > 0 && con->harm.type!=HARM_NONE)
	{
		tap = true;
		addCrossNum(1);
		GameInfo game_info;
		Coder::tapNotice(id, true, game_info);
		Coder::crossNotice(id, getCrossNum(), game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);

		if(tongShengID != -1)
		{
			if(tongShengID == id)
			{
				engine->setStateChangeMaxHand(id, false, false, 6,3);
			}
			else
			{
				engine->setStateChangeMaxHand(tongShengID, false, false, 6,3);
				engine->setStateChangeMaxHand(id, false, false, 6,3);
			}
			return GE_URGENT;
		}
	}
	return GE_SUCCESS;
}

int WuNv::ToPuTongXingTai(int playerID)
{
	if(id == playerID && tap)
	{
		PlayerEntity *self = engine->getPlayerEntity(id);
		if(self->getHandCardNum()<3)
		{
			tap = false;
			GameInfo game_info;
			Coder::tapNotice(id, false, game_info);
			engine->sendMessage(-1, MSG_GAME, game_info);
			if(tongShengID != -1)
			{
				if(tongShengID == id)
				{
					engine->setStateChangeMaxHand(id, false, false, 6,-3);
				}
				else
				{
					engine->setStateChangeMaxHand(tongShengID, false, false, 6,-3);
					engine->setStateChangeMaxHand(id, false, false, 6,-3);
				}
				return GE_URGENT;
			}
		}
	}
	return GE_SUCCESS;
}

int WuNv::LiuXue(int playerID)
{
	SkillMsg skill_msg;
	Coder::skillNotice(id, id, LIU_XUE, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	HARM harm;
	harm.point = 1;
	harm.srcID = id;
	harm.type = HARM_MAGIC;
	harm.cause = LIU_XUE;
	engine->setStateTimeline3(id, harm);
	return GE_SUCCESS;
}