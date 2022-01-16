#include "YueNv.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool YueNv::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch (type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch (respond->respond_id())
		{
		case XIN_YUE_BI_HU:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_XIN_YUE, XIN_YUE_BI_HU, respond);
			return true;
		case MEI_DU_SHA:
			session->tryNotify(id, STATE_TIMELINE_1, MEI_DU_SHA, respond);
			return true;
		case YUE_ZHI_LUN_HUI:
			session->tryNotify(id, STATE_TURN_END, YUE_ZHI_LUN_HUI, respond);
			return true;
		case YUE_DU:
			session->tryNotify(id, STATE_TIMELINE_6_DRAWN, YUE_DU, respond);
			return true;
		case AN_YUE_ZHAN:
			session->tryNotify(id, STATE_TIMELINE_2_HIT, AN_YUE_ZHAN, respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

int YueNv::p_xin_yue(int &step, CONTEXT_LOSE_MORALE *con)
{
	//未横时己方受到伤害掉士气
	if (tap || HARM_NONE == con->harm.type || con->howMany <= 0 ||engine->getPlayerEntity(con->dstID)->getColor() != color)
		return GE_SUCCESS;
	step = XIN_YUE_BI_HU;
	int ret = XinYueBiHu(con);
	if (toNextStep(ret) || ret == GE_URGENT)
	{
		step = STEP_DONE;
	}
	return ret;
}

int YueNv::XinYueBiHu(CONTEXT_LOSE_MORALE *con)
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, XIN_YUE_BI_HU, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		vector<int> cards;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			//发动
			if (respond->args(0) == 1)
			{
				network::SkillMsg skill;
				Coder::skillNotice(id, con->dstID, XIN_YUE_BI_HU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				
				con->howMany = 0;
				tap = true;
				GameInfo game_info;
				Coder::tapNotice(id, true, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				CONTEXT_TAP *con2 = new CONTEXT_TAP; con2->id = id; con2->tap = tap; engine->pushGameState(new StateTap(con2));
				engine->setStateMoveCardsNotToHand(-1, DECK_DISCARD, id, DECK_COVER, con->toDiscard.size(), con->toDiscard, id, XIN_YUE_BI_HU, false);
				
				return GE_URGENT;
			}
		}
		return ret;
	}
	else {
		//超时啥都不用做
		return GE_TIMEOUT;
	}
	return GE_SUCCESS;
}

int YueNv::p_cover_change(int &step, int dstID, int direction, int howMany, vector<int> cards, int doerID, int cause)
{
	int ret = GE_SUCCESS;
	if (doerID != id || direction != CHANGE_REMOVE ||howMany < 1)
	{
		return GE_SUCCESS;
	}
	step = AN_YUE_ZU_ZHOU;
	ret = AnYueZuZhou();
	if (toNextStep(ret) || ret == GE_URGENT) {
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int YueNv::AnYueZuZhou()
{
	if (getCoverCardNum()==0)
	{
		tap = false;
		GameInfo game_info;
		Coder::tapNotice(id, false, game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);
	}
	CONTEXT_LOSE_MORALE* morale = new CONTEXT_LOSE_MORALE;
	HARM anyue;
	anyue.cause = AN_YUE_ZU_ZHOU;
	anyue.point = 1;
	anyue.srcID = id;
	anyue.type = HARM_NONE;
	morale->dstID = id;
	morale->harm = anyue;
	morale->howMany = 1;
	engine->pushGameState(new StateBeforeLoseMorale(morale));
	return GE_URGENT;
}

int YueNv::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	if (con->attack.srcID == id && CangBaiAttack)
	{
		con->hitRate = RATE_NOREATTACK;
		CangBaiAttack = false;
		return GE_SUCCESS;
	}

	//美杜莎：非队友，有盖牌
	if (engine->getPlayerEntity(con->attack.srcID)->getColor()== color || getCoverCardNum() < 1)  {
		return GE_SUCCESS;
	}
	step = MEI_DU_SHA;
	ret = MeiDuSha(con);
	if (toNextStep(ret) || ret == GE_URGENT) {
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int YueNv::MeiDuSha(CONTEXT_TIMELINE_1 *con)
{
	int ret;
	CommandRequest cmd_req;
	Coder::askForSkill(id, MEI_DU_SHA, cmd_req);
	Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size() - 1));
	cmd->add_args(con->attack.cardID);
	//有限等待，由UserTask调用tryNotify唤醒
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;

			//发动
			if (respond->args(0)>0) {
				network::SkillMsg skill;
				Coder::skillNotice(id, con->attack.srcID, MEI_DU_SHA, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				addCrossNum(1);
				setToken(1, token[1] + 1);
				GameInfo game_info;
				Coder::crossNotice(id, getCrossNum(), game_info);
				Coder::tokenNotice(id, 1, token[1], game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				if (respond->args(0)==2)
				{
					HARM harm;
					harm.cause = MEI_DU_SHA;
					harm.point = 1;
					harm.srcID = id;
					harm.type = HARM_MAGIC;
					engine->setStateTimeline3(respond->dst_ids(0), harm);

					HARM qipai;
					qipai.cause = MEI_DU_SHA;
					qipai.point = 1;
					qipai.srcID = id;
					qipai.type = HARM_NONE;
					engine->pushGameState(new StateRequestHand(id, qipai, -1, DECK_DISCARD, false, false));
					
				}
				engine->setStateMoveOneCardNotToHand(id, DECK_COVER, -1, DECK_DISCARD, respond->card_ids(0), id, MEI_DU_SHA, true);
				CardMsg show_card;
				Coder::showCardNotice(id, 1, respond->card_ids(0), show_card);
				engine->sendMessage(-1, MSG_CARD, show_card);
				return GE_URGENT;
			}
		}
		return ret;
	}
	else {
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int YueNv::p_turn_end(int &step, int playerID)
{
	if ( (playerID != id) ||( getCrossNum() < 1 && getCoverCardNum() < 1) )
		return GE_SUCCESS;
	step = YUE_ZHI_LUN_HUI;
	int ret = YueZhiLunHui();
	if (toNextStep(ret) || ret == GE_URGENT) {
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int YueNv::YueZhiLunHui()
{
	{
		int ret;
		CommandRequest cmd_req;
		Coder::askForSkill(id, YUE_ZHI_LUN_HUI, cmd_req);
		//有限等待，由UserTask调用tryNotify唤醒
		if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
		{
			void* reply;
			if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
			{
				Respond* respond = (Respond*)reply;
				//发动

				if (respond->args(0) == 1 ) {
					network::SkillMsg skill;
					Coder::skillNotice(id, id, YUE_ZHI_LUN_HUI, skill);
					engine->sendMessage(-1, MSG_SKILL, skill);
					PlayerEntity* player = engine->getPlayerEntity(respond->dst_ids(0));
					player->addCrossNum(1);
					GameInfo game_info;
					Coder::crossNotice(respond->dst_ids(0), player->getCrossNum(), game_info);
					engine->sendMessage(-1, MSG_GAME, game_info);
					engine->setStateMoveOneCardNotToHand(id, DECK_COVER, -1, DECK_DISCARD, respond->card_ids(0), id, YUE_ZHI_LUN_HUI, false);
					return GE_URGENT;
				}
				else if (respond->args(0) == 2) {
					network::SkillMsg skill;
					Coder::skillNotice(id, id, YUE_ZHI_LUN_HUI, skill);
					engine->sendMessage(-1, MSG_SKILL, skill);
					subCrossNum(1);
					setToken(0, token[0] + 1);
					GameInfo game_info;
					Coder::crossNotice(id, getCrossNum(), game_info);
					Coder::tokenNotice(id, 0, token[0], game_info);
					engine->sendMessage(-1, MSG_GAME, game_info);
				}
			}
			return ret;
		}
		else {
			//超时啥都不用做
			return GE_TIMEOUT;
		}
	}
}

int YueNv::p_before_turn_begin(int &step, int currentPlayerID)
{
	used_YueDu = false;
	CangBaiAddTurn = false;
	CangBaiAttack = false;
	return GE_SUCCESS;
}



int YueNv::p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con)
{
	int ret = GE_INVALID_STEP;
	//月渎：法伤，有治疗
	if (con->harm.srcID != id || con->harm.type != HARM_MAGIC || getCrossNum() < 1 ||used_YueDu) {
		return GE_SUCCESS;
	}
	step = YUE_DU;
	ret = YueDu(con);
	if (toNextStep(ret) || ret == GE_URGENT) {
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int YueNv::YueDu(CONTEXT_TIMELINE_6_DRAWN *con)
{
	{
		int ret;
		CommandRequest cmd_req;
		Coder::askForSkill(id, YUE_DU, cmd_req);
		//有限等待，由UserTask调用tryNotify唤醒
		if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
		{
			void* reply;
			if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
			{
				Respond* respond = (Respond*)reply;
				//发动
				if (respond->args(0) == 1)
				{
					network::SkillMsg skill;
					Coder::skillNotice(id, respond->dst_ids(0), YUE_DU, skill);
					engine->sendMessage(-1, MSG_SKILL, skill);
					used_YueDu = true;
					subCrossNum(1);
					GameInfo game_info;
					Coder::crossNotice(id, getCrossNum(), game_info);
					engine->sendMessage(-1, MSG_GAME, game_info);

					HARM harm;
					harm.cause = YUE_DU;
					harm.point = 1;
					harm.srcID = id;
					harm.type = HARM_MAGIC;
					engine->setStateTimeline3(respond->dst_ids(0), harm);
					return GE_URGENT;
				}
			}
			return ret;
		}
		else {
			//超时啥都不用做
			return GE_TIMEOUT;
		}
	}
}

int YueNv::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con)
{
	int ret = GE_INVALID_STEP;
	//美杜莎：非队友，有盖牌
	if (con->attack.srcID != id || getEnergy() < 1|| getCoverCardNum()<1||con->attack.isActive==false) {
		return GE_SUCCESS;
	}
	step = AN_YUE_ZHAN;
	ret = AnYueZhan(con);
	if (toNextStep(ret) || ret == GE_URGENT) {
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int YueNv::AnYueZhan(CONTEXT_TIMELINE_2_HIT *con)
{
	int ret = GE_SUCCESS;
	CommandRequest cmd_req;
	Coder::askForSkill(id, AN_YUE_ZHAN, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			//发动
			if (respond->args(0) == 1) {
				network::SkillMsg skill;
				Coder::skillNotice(id, id, AN_YUE_ZHAN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				if (crystal > 0)
					setCrystal(--crystal);
				else if (gem > 0)
					setGem(--gem);
				GameInfo game_info;
				Coder::energyNotice(id, gem, crystal, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);

				vector<int>cards;
				int card_id;
				int howmany = respond->card_ids_size();
				for (int i = 0; i < howmany; ++i)
				{
					card_id = respond->card_ids(i);
					if (checkOneCoverCard(card_id) != GE_SUCCESS) {
						return GE_INVALID_CARDID;
					}
					cards.push_back(card_id);
				}
				con->harm.point += howmany;
				engine->setStateMoveCardsNotToHand(id, DECK_COVER, -1, DECK_DISCARD, howmany, cards, id, AN_YUE_ZHAN, false);

				return GE_URGENT;
			}
		}
		return ret;
	}
	else {
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int YueNv::p_magic_skill(int &step, Action* action)
{
	int ret;
	switch(action->action_id())
	{
	case CANG_BAI_ZHI_YUE:
		step = CANG_BAI_ZHI_YUE;
		ret = CangBaiZhiYue(step, action);
		if(toNextStep(ret) || GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;

	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int YueNv::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	//int cardID;
	int playerID = action->src_id();
	//int dstID;
	//CardEntity* card;
	//PlayerEntity* dst;
	

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}
	switch(actionID)
	{
	case CANG_BAI_ZHI_YUE:
		if( getGem() < 1  || (action->args(0)==1 && token[1] < 3) )
			return GE_INVALID_ACTION;
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int YueNv::CangBaiZhiYue(int &step, Action* action)
{
	GameInfo game_info;
	gem--;
	Coder::energyNotice(id, gem,crystal,game_info);
	network::SkillMsg skill;
	Coder::skillNotice(id, id, CANG_BAI_ZHI_YUE, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	if (action->args(0) == 1)
	{
		token[1] -= 3;
		Coder::tokenNotice(id, 1,token[1], game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);

		addAction(ACTION_ATTACK, CANG_BAI_ZHI_YUE);
		CangBaiAddTurn = true;
	}
	if (action->args(0) == 2)
	{
		int xinyue = action->args(1);
		token[0] -= action->args(1);
		setToken(1, token[1] + 1);
		Coder::tokenNotice(id, 1, token[1], game_info);
		Coder::tokenNotice(id, 0, token[0], game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);

		HARM harm;
		harm.cause = CANG_BAI_ZHI_YUE;
		harm.point = 1+xinyue;
		harm.srcID = id;
		harm.type = HARM_MAGIC;
		engine->setStateTimeline3(action->dst_ids(0), harm);

		HARM qipai;
		qipai.cause = CANG_BAI_ZHI_YUE;
		qipai.point = 1;
		qipai.srcID = id;
		qipai.type = HARM_NONE;
		engine->pushGameState(new StateRequestHand(id, qipai, -1, DECK_DISCARD, false, false));

		return GE_URGENT;
	}
	return GE_SUCCESS;
}

int YueNv::p_additional_action(int chosen)
{
	GameInfo update_info;
	switch (chosen)
	{

	case CANG_BAI_ZHI_YUE:
		CangBaiAttack = true;
		break;
	default:
		CangBaiAttack = false;
	}
	//做完角色相关的操作，扣除额外行动交给底层
	return PlayerEntity::p_additional_action(chosen);
}

int YueNv::p_after_turn_end(int &step, int playerID)
{
	if (CangBaiAddTurn)
	{ 
		CangBaiAddTurn = false;
		engine->popGameState_if(STATE_AFTER_TURN_END);

		clearAdditionalAction();
		network::TurnBegin turn_begin;
		turn_begin.set_id(id);
		turn_begin.set_round(engine->m_roundId);
		engine->sendMessage(-1, network::MSG_TURN_BEGIN, turn_begin);
		engine->pushGameState(new StateBeforeTurnBegin);
		return GE_SUCCESS;
	}
	return GE_SUCCESS;
}