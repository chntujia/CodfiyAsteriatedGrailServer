#include "ShouLing.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool ShouLing::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case WU_ZHE_CAN_XIN:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_AFTER_ATTACK, WU_ZHE_CAN_XIN, respond);
			return true;
			break;
		case YI_JI_WU_NIAN:
			session->tryNotify(id, STATE_AFTER_ATTACK, YI_JI_WU_NIAN, respond);
			return true;
			break;
		case SHOU_HUN_JING_JIE:
			session->tryNotify(id, STATE_TAP, SHOU_HUN_JING_JIE, respond);
			return true;
			break;
		case SHOU_FAN:
			session->tryNotify(id, STATE_TIMELINE_3, SHOU_FAN, respond);
			return true;
			break;
		case NI_FAN_ZHAN:
			session->tryNotify(id, STATE_TIMELINE_1, NI_FAN_ZHAN, respond);
			return true;
			break;
		case NI_FAN_HIT:
			session->tryNotify(id, STATE_TIMELINE_2_HIT, NI_FAN_HIT, respond);
			return true;
			break;
		case YU_HUN_BOOT:
			session->tryNotify(id, STATE_BOOT, YU_HUN_BOOT, respond);
			return true;
			break;
		}
	}
	//没匹配则返回false
	return false;
}

//在出错的时候，p_xxxx有可能执行不止一次，若每次都重头来过的话。。。所以需要step记录执行到哪里
int ShouLing::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}

		
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	if(step == STEP_INIT){
		step = YI_JI_ATK;
	}

	if(step == YI_JI_ATK )
	{
		ret = YiJiAtk(con);
		if(toNextStep(ret)){
			step = YU_HUN_ATK;
		}
	}
	if (step == YU_HUN_ATK)
	{
		if (tap && engine->getPlayerEntity(con->attack.dstID)->tapped())
			con->harm.point++;
		step = NI_FAN_ZHAN;	
	}
	if (step == NI_FAN_ZHAN)
	{
		ret = NiFanZhan(con);
		if (toNextStep(ret)) {
			step = STEP_DONE;
		}
	}
	return ret;
}

int ShouLing::WuZheCanXin()
{
	if (used_wuzhe)
		return GE_SUCCESS;
	int ret;
	CommandRequest cmd_req;
	Coder::askForSkill(id, WU_ZHE_CAN_XIN, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			//发动
			if (respond->args(0) == 1) {
				used_wuzhe = true;
				setToken(0, token[0] + 1);
				GameInfo gameInfo;
				Coder::tokenNotice(id, 0, token[0], gameInfo);
				engine->sendMessage(-1, MSG_GAME, gameInfo);

				network::SkillMsg skill;
				Coder::skillNotice(id, id, WU_ZHE_CAN_XIN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

			}
			return GE_SUCCESS;

		}
		return ret;
	}
	else {
		//超时啥都不用做
		return GE_TIMEOUT;
	}
	return GE_INVALID_ARGUMENT;
}

int ShouLing::p_before_turn_begin(int &step, int currentPlayerID)
{
	used_wuzhe = false;

	return GE_SUCCESS;
}

int ShouLing::p_after_attack(int &step, int currentPlayerID)
{
	int ret;
	if (currentPlayerID != id)
		return GE_SUCCESS;

	if (step == STEP_INIT ||step == WU_ZHE_CAN_XIN)
	{
		step = WU_ZHE_CAN_XIN;
		ret = WuZheCanXin();
		if (toNextStep(ret)) {
			step = STEP_DONE;
		}
	}

	if (token[0] > 3)
	{
		addAction(ACTION_ATTACK, YI_JI_WU_NIAN);
	}
	return GE_SUCCESS;
}

int ShouLing::p_additional_action(int chosen)
{
	GameInfo update_info;
	switch (chosen)
	{

	case YI_JI_WU_NIAN:
		setToken(0, token[0] - 4);
		Coder::tokenNotice(id, 0, token[0], update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
		yijiatk = true;
		break;
	default:
		yijiatk = false;
	}
	//做完角色相关的操作，扣除额外行动交给底层
	return PlayerEntity::p_additional_action(chosen);
}

int ShouLing::YiJiAtk(CONTEXT_TIMELINE_1 *con)
{
	if (!yijiatk)	return GE_SUCCESS;
	con->checkShield = false;
	con->canLight = false;
	if (getCardByID(con->attack.cardID)->getProperty() == (PROPERTY_MARTIAL) )
		con->hitRate = RATE_NOMISS;

	yijiatk = false;
	return GE_SUCCESS;
}

int ShouLing::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con)
{
	int ret = GE_INVALID_STEP;
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	if(step == STEP_INIT){
		step = SHOU_HUN_YI_NIAN;
	}
	if(step == SHOU_HUN_YI_NIAN)
	{
		if (con->attack.isActive && !tap){
			setToken(1, token[1] + 1);
			GameInfo update_info;
			Coder::tokenNotice(id, 1, token[1], update_info);
			engine->sendMessage(-1, MSG_GAME, update_info);
		}
		ret = GE_SUCCESS;

		if(toNextStep(ret)){
			step = NI_FAN_HIT;
		}			
	}
	if(step == NI_FAN_HIT)
	{
		ret = NiFanHit(con);
		if(toNextStep(ret)){
			step = STEP_DONE;
		}
	}
	return ret;
}

int ShouLing::p_tap(int &step, CONTEXT_TAP *con)
{
	int ret = GE_INVALID_STEP;
	if (con->id == id || con->tap == false || token[1] < 1 || tap)
		return GE_SUCCESS;
	step = SHOU_HUN_JING_JIE;
	ret = ShouHunJingJie();
	if (toNextStep(ret) || step==GE_URGENT) {
		step = STEP_DONE;
	}
	return ret;
}

int ShouLing::ShouHunJingJie()
{
	int ret;
	CommandRequest cmd_req;
	Coder::askForSkill(id, SHOU_HUN_JING_JIE, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			//发动
			if (respond->args(0) == 1) {
				int dstId = respond->dst_ids(0);
				
				tap = true;
				setToken(0, token[0] + 1);
				setToken(1, token[1] - 1);

				GameInfo gameInfo;
				Coder::tokenNotice(id, 0, token[0], gameInfo);
				Coder::tokenNotice(id, 1, token[1], gameInfo);
				Coder::tapNotice(id, true, gameInfo);
				engine->sendMessage(-1, MSG_GAME, gameInfo);

				network::SkillMsg skill;
				Coder::skillNotice(id, dstId, SHOU_HUN_JING_JIE, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);


				HARM shouhunjingjie;
				shouhunjingjie.cause = SHOU_HUN_JING_JIE;
				shouhunjingjie.point = 1;
				shouhunjingjie.srcID = id;
				shouhunjingjie.type = HARM_NONE;
				engine->pushGameState(new StateRequestHand( dstId , shouhunjingjie,-1, DECK_DISCARD, true,false));
				return GE_URGENT;
			}
			return GE_SUCCESS;

		}
		return ret;
	}
	else {
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int ShouLing::p_show_hand(int &step, int playerID, int howMany, vector<int> cards, HARM harm)
{
	if( (harm.cause == SHOU_HUN_JING_JIE || harm.cause == SHOU_FAN)&& getCardByID(cards[0])->getType()==TYPE_MAGIC)
	{
		setToken(1, token[1] + 1);
		GameInfo gameInfo;
		Coder::tokenNotice(id, 1, token[1], gameInfo);
		engine->sendMessage(-1, MSG_GAME, gameInfo);
	}
	return GE_SUCCESS;
}

int ShouLing::p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con)
{
	int ret = GE_INVALID_STEP;
	if (con->dstID == id && con->harm.type == HARM_MAGIC && token[1] > 0) {
		step = SHOU_FAN;
		ret = ShouFan(con);
		if (toNextStep(ret)) {
			step = STEP_DONE;
		}
	}
		return GE_SUCCESS;
}

int ShouLing::p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con)
{
	if (con->harm.srcID == id && con->harm.point > 0)
	{
		tap = false;
		GameInfo gameInfo;
		Coder::tapNotice(id, tap, gameInfo);
		engine->sendMessage(-1, MSG_GAME, gameInfo);
	}
	return GE_SUCCESS;
}

int ShouLing::ShouFan(CONTEXT_TIMELINE_3 *con)
{
	int ret;
	CommandRequest cmd_req;
	Coder::askForSkill(id, SHOU_FAN, cmd_req);
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			//发动
			if (respond->args(0) == 1) {
				int howmany = respond->args(1);
				int dstId = con->harm.srcID;
				setToken(0, token[0] + howmany);
				setToken(1, token[1] - howmany);
				GameInfo gameInfo;
				Coder::tokenNotice(id, 0, token[0], gameInfo);
				Coder::tokenNotice(id, 1, token[1], gameInfo);
				engine->sendMessage(-1, MSG_GAME, gameInfo);

				HARM shoufan;
				shoufan.cause = SHOU_FAN;
				shoufan.point = 1;
				shoufan.srcID = id;
				shoufan.type = HARM_NONE;
				engine->pushGameState(new StateRequestHand(dstId, shoufan, -1, DECK_DISCARD, true, false));

				int cardNum = ((getHandCardNum() - howmany) > 0) ? howmany : getHandCardNum();

				vector<int> cardIDs;
				for (int i = 0; i < cardNum; i++)
				{
					cardIDs.push_back(respond->card_ids(i));
				}
				if (cardNum > 0)
				{
					engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs, id, SHOU_FAN, false);
					
				}


				network::SkillMsg skill;
				Coder::skillNotice(id, dstId, SHOU_FAN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				return GE_URGENT;
			}
			return GE_SUCCESS;

		}
		return ret;
	}
	else {
		//超时啥都不用做
		return GE_TIMEOUT;
	}

	return GE_SUCCESS;
}

int ShouLing::p_turn_end(int &step, int playerID)
{
	if (playerID != id || !tap)
		return GE_SUCCESS;
	if (token[1] > 0)
		setToken(0, token[0] + 1);
	setToken(1, token[1] - 1);
	if (token[1] < 1)
		tap = false;
	GameInfo gameInfo;
	Coder::tokenNotice(id, 0, token[0], gameInfo);
	Coder::tokenNotice(id, 1, token[1], gameInfo);
	Coder::tapNotice(id, tap, gameInfo);
	engine->sendMessage(-1, MSG_GAME, gameInfo);
	return GE_SUCCESS;
}

int ShouLing::NiFanZhan(CONTEXT_TIMELINE_1 *con)
{
	if (engine->getPlayerEntity(con->attack.dstID)->getHandCardNum() > 3 || !tap )
		return GE_SUCCESS;
	int ret;
	CommandRequest cmd_req;
	Coder::askForSkill(id, NI_FAN_ZHAN, cmd_req);
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			//发动
			if (respond->args(0) == 1) {
				nifanatk = true;
				network::SkillMsg skill;
				Coder::skillNotice(id, con->attack.dstID, NI_FAN_ZHAN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
			}
			return GE_SUCCESS;
		}
		return ret;
	}
	else {
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int ShouLing::NiFanHit(CONTEXT_TIMELINE_2_HIT *con)
{
	if (token[1] < 1) return GE_SUCCESS;
	if (nifanatk==false) return GE_SUCCESS;
	int ret=GE_INVALID_ARGUMENT;
	CommandRequest cmd_req;
	Coder::askForSkill(id, NI_FAN_HIT, cmd_req);
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			//发动
			if (respond->args(0) == 1) {
				int howmany = respond->args(1);
				setToken(0, token[0] + howmany);
				
				setToken(1, token[1] - howmany);

				GameInfo gameInfo;
				Coder::tokenNotice(id, 0, token[0], gameInfo);
				Coder::tokenNotice(id, 1, token[1], gameInfo);
				engine->sendMessage(-1, MSG_GAME, gameInfo);

				network::SkillMsg skill;
				Coder::skillNotice(id, con->attack.dstID, NI_FAN_ZHAN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				nifanaddharm = howmany;
			}
			return GE_SUCCESS;
		}
		return ret;
	}
	else {
		//超时啥都不用做
		return GE_TIMEOUT;
	}
	return ret;
}

int ShouLing::p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con)
{
	if (con->srcID == id)
		nifanatk = false;
	return GE_SUCCESS;
}

int ShouLing::p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con)
{

	if (con->harm.srcID == id && nifanatk && con->harm.type== HARM_ATTACK)
	{
		int dstID = con->dstID;
		engine->popGameState();
		nifanatk = false;
		if (engine->getPlayerEntity(dstID)->getHandCardNum() < 2 + nifanaddharm)
		{
			CONTEXT_LOSE_MORALE* morale = new CONTEXT_LOSE_MORALE;
			HARM nifan;
			nifan.cause = NI_FAN_ZHAN;
			nifan.point = 1;
			nifan.srcID = id;
			nifan.type = HARM_NONE;
			morale->dstID = dstID;
			morale->harm = nifan;
			morale->howMany = 1;
			engine->pushGameState(new StateBeforeLoseMorale(morale));
		}
		HARM nifan;
		nifan.cause = NI_FAN_ZHAN;
		nifan.point = 2+nifanaddharm;
		nifan.srcID = id;
		nifan.type = HARM_NONE;
		engine->pushGameState(new StateRequestHand(dstID, nifan, -1, DECK_DISCARD, false, false));
		nifanaddharm = 0;
		return GE_URGENT;
	}
	return GE_SUCCESS;
}

int ShouLing::p_boot(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	step = YU_HUN_BOOT;
	//非红莲回合||没治疗
	if (currentPlayerID != id || getGem() == 0 || tap)
		return GE_SUCCESS;
	ret = YuHunBoot();
	step = STEP_DONE;
	return ret;
}

int ShouLing::YuHunBoot()
{
	int ret;
	GameInfo update_info;
	//满足发动条件，询问客户端是否发动
	CommandRequest cmd_req;
	Coder::askForSkill(id, YU_HUN_BOOT, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			int choose = respond->args(0);
			if(choose > 0){
				int howmany = respond->args(1);
				network::SkillMsg skill;
				Coder::skillNotice(id, id, YU_HUN_BOOT, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				tap = true;
				setGem(gem - 1);
				token[1]++;
				Coder::tokenNotice(id,1,token[1], update_info);
				Coder::tapNotice(id, tap, update_info);
				Coder::energyNotice(id, gem, crystal, update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
				vector<int> cardIDs;
				if (howmany > 0)
				{
					switch (choose)
					{
					case 1:
						for (int i = 0; i < howmany; i++)
						{
							cardIDs.push_back(respond->card_ids(i));
						}
						engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, howmany, cardIDs, id, YU_HUN_BOOT, false);
						break;
					case 2:
						HARM harm;
						harm.srcID = id;
						harm.type = HARM_NONE;
						harm.point = howmany;
						harm.cause = YU_HUN_BOOT;
						engine->setStateMoveCardsToHand(-1, DECK_PILE, id, DECK_HAND, howmany, cardIDs, harm, false);
						break;
					default:
						return GE_INVALID_ARGUMENT;
					}
					return GE_URGENT;
				}
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