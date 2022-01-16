#include "JingLingSheShou.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool JingLingSheShou::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch (type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch (respond->respond_id())
		{
		case YUAN_SU_SHE_JI:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_1, YUAN_SU_SHE_JI, respond);
			return true;
			break;
		case YUAN_SU_SHE_JI_HIT:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_2_HIT, YUAN_SU_SHE_JI_HIT, respond);
			return true;
			break;
		case DONG_WU_HUO_BAN:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_6_DRAWN, DONG_WU_HUO_BAN, respond);
			return true;
			break;
		case JING_LING_MI_YI:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TURN_END, JING_LING_MI_YI, respond);
			return true;
			break;          
		case JING_LING_MI_YI_BOOT:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_BOOT, JING_LING_MI_YI_BOOT, respond);
			return true;
			break;
		}
	}
	//没匹配则返回false
	return false;
}

int JingLingSheShou::p_before_turn_begin(int &step, int currentPlayerID)
{
	used_YUAN_SU_SHE_JI = false;

	return GE_SUCCESS;
}

int JingLingSheShou::p_boot(int &step, int currentPlayerID)
{
	if (currentPlayerID != id || gem == 0 || tap == true)
		return GE_SUCCESS;
	step = JING_LING_MI_YI_BOOT;
	int ret = JingLingMiYiBoot();
	if (toNextStep(ret) || ret == GE_URGENT) {
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int JingLingSheShou::p_turn_end(int &step, int playerID)
{
	if (playerID != id || !tap || coverCards.size() > 0)
		return GE_SUCCESS;
	step = JING_LING_MI_YI;
	int ret = JingLingMiYiReset();
	if (toNextStep(ret) || ret == GE_URGENT) {
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}



int JingLingSheShou::JingLingMiYiReset()
{
	GameInfo game_info;
	tap = false;
	int dstID;
	int ret;
	Coder::tapNotice(id, false, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
	CommandRequest cmd_req;
	Coder::askForSkill(id, JING_LING_MI_YI, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			dstID = respond->dst_ids(0);
		}
	}
	else {
		dstID = id;
	}
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, JING_LING_MI_YI, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	HARM harm;
	harm.cause = JING_LING_MI_YI;
	harm.point = 2;
	harm.srcID = id;
	harm.type = HARM_MAGIC;
	engine->setStateTimeline3(dstID, harm);
	return GE_URGENT;
}

int JingLingSheShou::JingLingMiYiBoot()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, JING_LING_MI_YI_BOOT, cmd_req);
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
				Coder::skillNotice(id, id, JING_LING_MI_YI_BOOT, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				tap = true;
				gem -= 1;
				GameInfo game_info;
				Coder::tapNotice(id, true, game_info);
				CONTEXT_TAP *con = new CONTEXT_TAP; con->id = id; con->tap = tap; engine->pushGameState(new StateTap(con));
				Coder::energyNotice(id, gem, crystal, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				engine->setStateMoveCardsNotToHand(-1, DECK_PILE, id, DECK_COVER, 3, cards, id, JING_LING_MI_YI_BOOT, false);
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
// 动物伙伴
int JingLingSheShou::p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con)
{
	//  自己造成的伤害
	if (con->harm.srcID == id && engine->getCurrentPlayerID()==id)
	{
		// 动物伙伴
		step = DONG_WU_HUO_BAN;
		int ret = DongWuHuoBan(con);
		if (toNextStep(ret) || ret == GE_URGENT) {
			//全部走完后，请把step设成STEP_DONE
			step = STEP_DONE;
		}
		return ret;
	}
	return GE_SUCCESS;
}

int JingLingSheShou::DongWuHuoBan(CONTEXT_TIMELINE_6_DRAWN *con)
{
	{
		vector<int> cards;
		int ret;
				{
			CommandRequest cmd_req;
			Coder::askForSkill(id, DONG_WU_HUO_BAN, cmd_req);
			if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
			{
				void* reply;
				if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
				{
					Respond* respond = (Respond*)reply;
					if (respond->args(0) == 1) {
						if (respond->dst_ids(0) != id)//指定自己以外的角色将消耗能量
						{
							if (crystal > 0)
								setCrystal(--crystal);
							else if (gem > 0)
								setGem(--gem);
							GameInfo game_info;
							Coder::energyNotice(id, gem, crystal, game_info);
							engine->sendMessage(-1, MSG_GAME, game_info);
						}
						//弃一张手牌
						HARM qipai;
						qipai.cause = DONG_WU_HUO_BAN;
						qipai.point = 1;
						qipai.srcID = id;
						qipai.type = HARM_NONE;
						engine->pushGameState(new StateRequestHand(respond->dst_ids(0), qipai, -1, DECK_DISCARD, false, false));
						//摸一张手牌
						HARM  harm;
						harm.srcID = id;
						harm.type = HARM_NONE;
						harm.point = 1;
						harm.cause = DONG_WU_HUO_BAN;
						engine->setStateMoveCardsToHand(-1, DECK_PILE, respond->dst_ids(0), DECK_HAND, 1, cards, harm, false);
						return GE_URGENT;
					}
				}
			}
			else {
				return GE_TIMEOUT;
			}
		}
		return GE_SUCCESS;
	}
}

int JingLingSheShou::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int srcID = con->attack.srcID;
	int ret = GE_INVALID_STEP;
	if (srcID != id || !con->attack.isActive || used_YUAN_SU_SHE_JI || getCardByID(con->attack.cardID)->getElement()==ELEMENT_DARKNESS  ) {
		return GE_SUCCESS;
	}
	if (step == STEP_INIT || step == YUAN_SU_SHE_JI) {
		step = YUAN_SU_SHE_JI;
		ret = YuanSuSheJi(con);
		if (toNextStep(ret) || ret == GE_URGENT)
		{
			step = STEP_DONE;
		}
	}
	return ret;
}

int JingLingSheShou::YuanSuSheJi(CONTEXT_TIMELINE_1 *con)
{
	int ret;
	int cardID;
	int element = getCardByID(con->attack.cardID)->getElement();
	// 等待玩家响应
	CommandRequest cmd_req;
	Coder::askForSkill(id, YUAN_SU_SHE_JI, cmd_req);
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			if ((respond->args(0) == 1) || (respond->args(0) == 2))
			{
				cardID = respond->card_ids(0);
				//确认发动，宣告技能
				SkillMsg skill;
				Coder::skillNotice(id, id, YUAN_SU_SHE_JI, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				used_YUAN_SU_SHE_JI = true;
				GameInfo game_info;
				switch (element)
				{
				case ELEMENT_FIRE:
					con->harm.point = con->harm.point + 1;
					break;
				case ELEMENT_EARTH:
					dizhishi = true;
					break;
				case ELEMENT_THUNDER:
					con->hitRate = RATE_NOREATTACK;
					break;
				case ELEMENT_WATER:
					shuizhishi = true;
					break;
				case ELEMENT_WIND:
					addAction(ACTION_ATTACK, YUAN_SU_SHE_JI);
					break;
				default:
					return GE_SUCCESS;
				}
				//展示弃牌
				CardMsg show_card;
				Coder::showCardNotice(id, 1, cardID, show_card);
				engine->sendMessage(-1, MSG_CARD, show_card);

				//1：使用法术牌；2：使用祝福
				if (respond->args(0) == 1)
					engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, YUAN_SU_SHE_JI, true);
				else
					engine->setStateMoveOneCardNotToHand(id, DECK_COVER, -1, DECK_DISCARD, cardID, id, YUAN_SU_SHE_JI, false);
				//返回URGENT，终止目前状态，执行状态栈顶状态
				return GE_URGENT;
			}
		}
		return ret;
	}
	else {
		return GE_TIMEOUT;
	}
	return GE_SUCCESS;
}

int JingLingSheShou::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con)
{
	int srcID = con->attack.srcID;
	int ret = GE_INVALID_STEP;
	if (srcID != id || !con->attack.isActive || !(dizhishi || shuizhishi) ) {
		return GE_SUCCESS;
	}
	if (step == STEP_INIT || step == YUAN_SU_SHE_JI_HIT) {
		step = YUAN_SU_SHE_JI_HIT;
		ret = YuanSuSheJiHit(con);
		if (toNextStep(ret) || ret == GE_URGENT)
		{
			step = STEP_DONE;
		}
	}
	return ret;
}

int JingLingSheShou::YuanSuSheJiHit(CONTEXT_TIMELINE_2_HIT *con)
{
	int ret;
	GameInfo game_info;
	if (shuizhishi)
	{
		shuizhishi = false;
		CommandRequest cmd_req;
		Coder::askForSkill(id, YUAN_SU_SHE_JI_HIT, cmd_req);
		Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size() - 1));
		cmd->add_args(1);
		if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
		{
			void* reply;
			if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
			{
				Respond* respond = (Respond*)reply;
				SkillMsg skill;
				Coder::skillNotice(id, respond->dst_ids(0), YUAN_SU_SHE_JI_HIT, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				engine->getPlayerEntity(respond->dst_ids(0))->addCrossNum(1);
				Coder::crossNotice(respond->dst_ids(0), engine->getPlayerEntity(respond->dst_ids(0))->getCrossNum(), game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				return GE_SUCCESS;
			}
			return ret;
		}
		return GE_TIMEOUT;
	}
	else if (dizhishi)
	{
		dizhishi = false;
		CommandRequest cmd_req;
		Coder::askForSkill(id, YUAN_SU_SHE_JI_HIT, cmd_req);
		Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size() - 1));
		cmd->add_args(2);
		if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
		{
			void* reply;
			if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
			{
				Respond* respond = (Respond*)reply;
				SkillMsg skill;
				Coder::skillNotice(id, respond->dst_ids(0), YUAN_SU_SHE_JI_HIT, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				HARM harm;
				harm.cause = YUAN_SU_SHE_JI_HIT;
				harm.point = 1;
				harm.srcID = id;
				harm.type = HARM_MAGIC;
				engine->setStateTimeline3(respond->dst_ids(0), harm);
				return GE_URGENT;
			}
			return ret;
		}
		return GE_TIMEOUT;
	}
	return GE_SUCCESS;
}
	

int JingLingSheShou::p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con)
{
	shuizhishi = false;
	dizhishi = false;
	return GE_SUCCESS;
}