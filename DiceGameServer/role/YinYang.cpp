#include "YinYang.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool YinYang::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
			case SHI_SHEN_ZHOU_SHU:
				session->tryNotify(id, STATE_ATTACKED, SHI_SHEN_ZHOU_SHU, respond);
				return true;
			case YIN_YANG_ZHUAN_HUAN:
				session->tryNotify(id, STATE_ATTACKED, YIN_YANG_ZHUAN_HUAN, respond);
				return true;
			case HEI_AN_JI_LI:
				session->tryNotify(id,STATE_TURN_END, HEI_AN_JI_LI,respond);
				return true;
		}
	}
	//没匹配则返回false
	return false;
}

int YinYang::p_turn_end(int &step, int playerID)
{
	if (playerID != id || token[0] < 3)
		return GE_SUCCESS;
	step = HEI_AN_JI_LI;
	int ret = HeiAnJiLi();
	if (toNextStep(ret) || ret == GE_URGENT) {
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int YinYang::ShiShenJiangLin(Action* action){

	if (action->card_ids().size() != 2 ||
		getCardByID(action->card_ids(0))->getProperty() != getCardByID(action->card_ids(1))->getProperty()
		)
		return GE_INVALID_CARDID;
	//宣告技能
	SkillMsg skill_msg;
	Coder::skillNotice(id, id,SHI_SHEN_JIANG_LIN, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	//+1鬼火
	setToken(0, token[0]+1);
	GameInfo update_info1;
	Coder::tokenNotice(id, 0, token[0], update_info1);
	engine->sendMessage(-1, MSG_GAME, update_info1);
	//橫置
	tap = true;
	GameInfo update_info2;
	Coder::tapNotice(id, tap, update_info2);
	engine->sendMessage(-1, MSG_GAME, update_info2);
	CONTEXT_TAP *con = new CONTEXT_TAP; con->id = id; con->tap = tap; engine->pushGameState(new StateTap(con));
	//+1攻击行动
	addAction(ACTION_ATTACK, SHI_SHEN_JIANG_LIN);
	//移除手牌
	vector<int> cards;
	cards.push_back(action->card_ids(0));
	cards.push_back(action->card_ids(1));
	CardMsg show_card;
	Coder::showCardNotice(id, 2, cards, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cards.size(), cards, id, SHI_SHEN_JIANG_LIN, true);
	return GE_URGENT;
}

int YinYang::v_reattack(int cardID, int orignCardID, int dstID, int orignID, int rate, bool realCard)
{
	if (rate != RATE_NORMAL) {
		return GE_INVALID_ACTION;
	}
	if (realCard) {
		int ret;
		if (GE_SUCCESS != (ret = checkOneHandCard(cardID))) {
			return ret;
		}
	}
	CardEntity* orignCard = getCardByID(orignCardID);
	if (orignCard->getElement() == ELEMENT_DARKNESS) {
		return GE_INVALID_CARDID;
	}
	CardEntity* card = getCardByID(cardID);
	if (card->getType() != TYPE_ATTACK) {
		return GE_INVALID_CARDID;
	}
	if (card->getElement() != ELEMENT_DARKNESS && card->getElement() != orignCard->getElement()
		&& !(tap && card->getProperty() == orignCard->getProperty())
		)
		return GE_INVALID_CARDID;
	PlayerEntity *dst = engine->getPlayerEntity(dstID);
	if (dstID == orignID || dst->getColor() == color) {
		return GE_INVALID_PLAYERID;
	}
	return GE_SUCCESS;
}


int YinYang::p_attacked(int &step, CONTEXT_TIMELINE_1 * con)
{
		int srcID = con->attack.srcID;
		int dstID = con->attack.dstID;
		int dstCorlor = engine->getPlayerEntity(dstID)->getColor();
		int selfColor = this->getColor();
		int ret = GE_INVALID_ACTION;
		TeamArea* team = engine->getTeamArea();
		//式神咒束
		if (dstID != id && con->attack.isActive && dstCorlor == selfColor && tap && con->hitRate == RATE_NORMAL && team->getEnergy(selfColor)>1 && team->getGem(selfColor) > 0 && getHandCardNum()>0 )
		{
			step = SHI_SHEN_ZHOU_SHU;
			ret = ShiShenZhouShu(con);
			return ret; 
		}
		//阴阳转换
		if (dstID == id && con->hitRate== RATE_NORMAL) {
			step = YIN_YANG_ZHUAN_HUAN;
			ret = YinYangZhuanHuan(con);
			return ret;
		}
		//伤害变化
		if (srcID == id && GuiHuoAtk) 
		{			
			con->harm.point = token[0];
			GuiHuoAtk = false;
		}
		return GE_SUCCESS;
}

int YinYang::HeiAnJiLi()
{
	GameInfo game_info;
	token[0] = 0;
	GameInfo update_info1;
	Coder::tokenNotice(id, 0, token[0], update_info1);
	engine->sendMessage(-1, MSG_GAME, update_info1);
	int dstID;
	int ret;
	bool reset=false;
	CommandRequest cmd_req;
	Coder::askForSkill(id, HEI_AN_JI_LI, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			dstID = respond->dst_ids(0);
			if (tap && respond->args(0) == 2)reset = true;
		}
	}
	else {
		dstID = id;
	}
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, HEI_AN_JI_LI, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	HARM harm;
	harm.cause = HEI_AN_JI_LI;
	harm.point = 2;
	harm.srcID = id;
	harm.type = HARM_MAGIC;
	engine->setStateTimeline3(dstID, harm);
	if (reset)
	{
		tap = false;
		GameInfo update_info2;
		Coder::tapNotice(id, tap, update_info2);
		engine->sendMessage(-1, MSG_GAME, update_info2);
	}

	return GE_URGENT;
}

int YinYang::ShiShenZhouShu(CONTEXT_TIMELINE_1 * con)
{
	int ret;
	CommandRequest cmd_req;
	Coder::askForSkill(id, SHI_SHEN_ZHOU_SHU, cmd_req);
	Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size() - 1));
	cmd->add_args(con->attack.srcID);
	cmd->add_args(con->attack.cardID);
	SkillMsg skill_msg1;
	SkillMsg skill_msg2;
	GameInfo update_info1;
	GameInfo update_info2;
	GameInfo update_info3;
	int cardID,srcID;
	bool isActive;
	TeamArea* team = engine->getTeamArea();
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			switch (respond->args(0))
			{
			case 0:
				break;
			case 3://1.式神咒束+阴阳转换+1鬼火
				Coder::skillNotice(id, respond->dst_ids(0), SHI_SHEN_ZHOU_SHU, skill_msg1);
				engine->sendMessage(-1, MSG_SKILL, skill_msg1);
				Coder::skillNotice(id, respond->dst_ids(0), YIN_YANG_ZHUAN_HUAN, skill_msg2);
				engine->sendMessage(-1, MSG_SKILL, skill_msg2);
				cardID = con->attack.cardID;
				srcID = con->attack.srcID;
				isActive = con->attack.isActive;
				engine->popGameState();				
				engine->setStateReattack(cardID, respond->card_ids(0), srcID, id, respond->dst_ids(0), isActive);
				setToken(0, token[0]+1);				
				Coder::tokenNotice(id, 0, token[0], update_info1);
				engine->sendMessage(-1, MSG_GAME, update_info1);

				if (team->getCrystal(color) > 0)
				{
					team->setCrystal(color, team->getCrystal(color) - 1);
					team->setGem(color, team->getGem(color) - 1);
				}
				else
				{
					team->setGem(color, team->getGem(color) - 2);
				}
				Coder::stoneNotice(color, team->getGem(color), team->getCrystal(color), update_info2);
				engine->sendMessage(-1, MSG_GAME, update_info2);

				GuiHuoAtk = true;
				tap = false;
				Coder::tapNotice(id, tap, update_info3);
				engine->sendMessage(-1, MSG_GAME, update_info3);
				return GE_URGENT;
			case 2://2.式神咒束 + 阴阳转换
				Coder::skillNotice(id, respond->dst_ids(0), SHI_SHEN_ZHOU_SHU, skill_msg1);
				engine->sendMessage(-1, MSG_SKILL, skill_msg1);
				Coder::skillNotice(id, respond->dst_ids(0), YIN_YANG_ZHUAN_HUAN, skill_msg2);
				engine->sendMessage(-1, MSG_SKILL, skill_msg2);

				if (team->getCrystal(color) > 0)
				{
					team->setCrystal(color, team->getCrystal(color) - 1);
					team->setGem(color, team->getGem(color) - 1);
				}
				else
				{
					team->setGem(color, team->getGem(color) - 2);
				}
				Coder::stoneNotice(color, team->getGem(color), team->getCrystal(color), update_info2);
				engine->sendMessage(-1, MSG_GAME, update_info2);
				cardID = con->attack.cardID;
				srcID = con->attack.srcID;
				isActive = con->attack.isActive;
				engine->popGameState();
				engine->setStateReattack(cardID, respond->card_ids(0), srcID, id, respond->dst_ids(0), isActive);
				GuiHuoAtk = true;
				tap = false;
				Coder::tapNotice(id, tap, update_info3);
				engine->sendMessage(-1, MSG_GAME, update_info3);
				return GE_URGENT;
			case 1:	//3.式神咒束（正常应战）
				Coder::skillNotice(id, respond->dst_ids(0), SHI_SHEN_ZHOU_SHU, skill_msg1);
				engine->sendMessage(-1, MSG_SKILL, skill_msg1);

				if (team->getCrystal(color) > 0)
				{
					team->setCrystal(color, team->getCrystal(color) - 1);
					team->setGem(color, team->getGem(color) - 1);
				}
				else
				{
					team->setGem(color, team->getGem(color) - 2);
				}
				Coder::stoneNotice(color, team->getGem(color), team->getCrystal(color), update_info2);
				engine->sendMessage(-1, MSG_GAME, update_info2);
				cardID = con->attack.cardID;
				srcID = con->attack.srcID;
				isActive = con->attack.isActive;
				engine->popGameState();
				engine->setStateReattack(cardID, respond->card_ids(0), srcID, id, respond->dst_ids(0), isActive);
				return GE_URGENT;
			}
			return GE_SUCCESS;
		}
	}
	return GE_TIMEOUT;
}

int YinYang::YinYangZhuanHuan(CONTEXT_TIMELINE_1 * con)
{
	int ret;
	CommandRequest cmd_req;
	Coder::askForSkill(id, YIN_YANG_ZHUAN_HUAN, cmd_req);
	Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size() - 1));
	cmd->add_args(con->attack.srcID);
	cmd->add_args(con->attack.cardID);
	int cardID, srcID;
	bool isActive;
	HARM harm;
	bool checkShield;
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			cardID = con->attack.cardID;
			srcID = con->attack.srcID;
			isActive = con->attack.isActive;
			harm = con->harm;
			checkShield = con->checkShield;
			engine->popGameState();
			if (respond->args(0) == 0)
			{
				engine->setStateAttackGiveUp(cardID, id, srcID, harm, isActive, checkShield);
			}
			else
			{
				if (getCardByID(respond->card_ids(0))->getElement() == ELEMENT_LIGHT)
				{
					engine->setStateTimeline2Miss(cardID, id, srcID, isActive);
					engine->setStateUseCard(respond->card_ids(0), srcID, id);
				}
				else {
					engine->setStateReattack(cardID, respond->card_ids(0), srcID, id, respond->dst_ids(0), isActive);
					if (respond->args(1) > 1)//阴阳转换
					{
						if (tap)
						{
							tap = false;
							GuiHuoAtk = true;
							GameInfo update_info3;
							Coder::tapNotice(id, tap, update_info3);
							engine->sendMessage(-1, MSG_GAME, update_info3);
						}

						SkillMsg skill_msg;
						Coder::skillNotice(id, id, YIN_YANG_ZHUAN_HUAN, skill_msg);
						engine->sendMessage(-1, MSG_SKILL, skill_msg);
						if (respond->args(1) > 2)//式神转换
						{
							setToken(0, token[0] + 1);
							GameInfo update_info1;
							Coder::tokenNotice(id, 0, token[0], update_info1);
							engine->sendMessage(-1, MSG_GAME, update_info1);
						}
					}
					return GE_URGENT;
				}
				
			}
			return GE_URGENT;
		}
	}
	return GE_TIMEOUT;
}

int YinYang::ShengMingJieJie(Action* action) {

	if (getEnergy()==0)
		return GE_INVALID_ACTION;
	if (crystal > 0)
		setCrystal(--crystal);
	else if (gem > 0)
		setGem(--gem);

	int dstID = action->dst_ids(0);
	//宣告技能
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, SHENG_MING_JIE_JIE, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	GameInfo game_info1;
	Coder::energyNotice(id, gem, crystal, game_info1);
	engine->sendMessage(-1, MSG_GAME, game_info1);
	//+1鬼火
	setToken(0, token[0]+1);
	GameInfo game_info2;
	Coder::tokenNotice(id, 0, token[0], game_info2);
	engine->sendMessage(-1, MSG_GAME, game_info2);
	//队友+1宝石
	PlayerEntity * dstPlayer = engine->getPlayerEntity(dstID);
	dstPlayer->setGem(dstPlayer->getGem() + 1);
	GameInfo game_info3;
	Coder::energyNotice(dstID, dstPlayer->getGem(), dstPlayer->getCrystal(), game_info3);
	engine->sendMessage(-1, MSG_GAME, game_info3);
	//+1治疗
	dstPlayer->addCrossNum(1);
	GameInfo game_info4;
	Coder::crossNotice(dstID, dstPlayer->getCrossNum(), game_info4);
	engine->sendMessage(-1, MSG_GAME, game_info4);

	HARM harm;
	harm.cause = SHENG_MING_JIE_JIE;
	harm.point = token[0];
	harm.srcID = id;
	harm.type = HARM_MAGIC;
	engine->setStateTimeline3(id, harm);

	return GE_URGENT;
}

//P_MAGIC_SKILL只有发起者执行，故不需要判断发起者，法术技能验证均在v_MAGIC_SKILL中
int YinYang::p_magic_skill(int &step, Action *action)
{
	int ret;
	int actionID = action->action_id();
	switch (actionID)
	{
	case SHI_SHEN_JIANG_LIN:
		ret = ShiShenJiangLin(action);
		step = STEP_DONE;

		break;
	case SHENG_MING_JIE_JIE:
		ret = ShengMingJieJie(action);
		step = STEP_DONE;

		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int YinYang::p_before_lose_morale(int &step, CONTEXT_LOSE_MORALE * con)
{
	if (con->harm.cause == SHENG_MING_JIE_JIE && token[0] == 3)
	{ 
		con->howMany = 0;
	}
	return GE_SUCCESS;
}

int YinYang::v_magic_skill(Action *action)
{
	return GE_SUCCESS;
}