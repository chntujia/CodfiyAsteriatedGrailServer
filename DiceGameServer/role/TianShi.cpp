#include "TianShi.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool TianShi::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case TIAN_SHI_JI_BAN:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_BASIC_EFFECT_CHANGE, TIAN_SHI_JI_BAN, respond);
			return true;
		case TIAN_SHI_ZHI_GE:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_BEFORE_TURN_BEGIN, TIAN_SHI_ZHI_GE, respond);
			return true;
		case SHEN_ZHI_BI_HU:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_LOSE_MORALE, SHEN_ZHI_BI_HU, respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

int TianShi::p_before_turn_begin(int &step, int currentPlayerID)
{
	if (currentPlayerID != id || gem+crystal == 0)
		return GE_SUCCESS;
	step = TIAN_SHI_ZHI_GE;
	int ret = TianShiZhiGe();
	if(toNextStep(ret) || ret == GE_URGENT){
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int TianShi::v_magic_skill(Action* action)
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
	case TIAN_SHI_ZHI_QIANG:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		dst = engine->getPlayerEntity(action->dst_ids(0));
		//  不是自己的手牌                         || 不是墙                           ||
		if (GE_SUCCESS != checkOneHandCard(cardID) || !card->checkSpeciality(actionID) ||
		//  目标已经有盾                                           || 目标已经有墙
			GE_SUCCESS == dst->checkBasicEffectByName(NAME_SHIELD) || GE_SUCCESS == dst->checkBasicEffectByName(actionID)){
			return GE_INVALID_ACTION;
		}
		break;
	case FENG_ZHI_JIE_JING:
		{
			cardID = action->card_ids(0);
			card = getCardByID(cardID);
			dst = engine->getPlayerEntity(action->dst_ids(0));
			int effect_card_id = action->args(0);
			// 不是自己的手牌                          || 不是风属性                         || 目标的基础效果里面没有指定卡牌
			if (GE_SUCCESS != checkOneHandCard(cardID) || ELEMENT_WIND != card->getElement() || GE_SUCCESS != dst->checkBasicEffectByCard(effect_card_id))
			{
				return GE_INVALID_ACTION;
			}
			break;
		}
	case TIAN_SHI_ZHU_FU:
		{
			cardID = action->card_ids(0);
			card = getCardByID(cardID);
			dst = engine->getPlayerEntity(action->dst_ids(0));
			PlayerEntity* dst2;
			if (action->dst_ids_size() > 1)
				dst2 = engine->getPlayerEntity(action->dst_ids(1));
			// 不是自己的手牌                          || 不是水属性
			if (GE_SUCCESS != checkOneHandCard(cardID) || ELEMENT_WATER != card->getElement())
			{
				return GE_INVALID_ACTION;
			}

			break;
		}
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int TianShi::p_magic_skill(int &step, Action *action)
{
	int actionID = action->action_id();
	int ret;

	switch(actionID)
	{
	case TIAN_SHI_ZHI_QIANG:
		ret = TianShiZhiQiang(action);
		if (ret == GE_SUCCESS)
			step = STEP_DONE;
		break;
	case FENG_ZHI_JIE_JING:
		ret = FengZhiJieJing(action);
		if (ret == GE_SUCCESS)
			step = STEP_DONE;
		break;
	case TIAN_SHI_ZHU_FU:
		ret = TianShiZhuFu(step, action);
		if(GE_URGENT == ret){
			step = TIAN_SHI_ZHU_FU;
		}
		else if (GE_SUCCESS == ret){
			step = STEP_DONE;}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int TianShi::p_basic_effect_change(int &step, int dstID, int card, int doerID, int cause)
{
	if (doerID != id)
		return GE_SUCCESS;
	CardEntity* card_entity = getCardByID(card);

	int ret = GE_SUCCESS;
	//  风之洁净                   || 天使之歌                 ||
	if (cause == FENG_ZHI_JIE_JING || cause == TIAN_SHI_ZHI_GE ||
	//  ((天使之墙                                         || 圣盾                                 ) && 卡牌是被使用使用)
		((card_entity->checkSpeciality(TIAN_SHI_ZHI_QIANG) || card_entity->getName() == NAME_SHIELD) && cause == CAUSE_USE))
	{
		step = TIAN_SHI_JI_BAN;
		ret = TianShiJiBan();
		if(toNextStep(ret) || ret == GE_URGENT){
			//全部走完后，请把step设成STEP_DONE
			step = STEP_DONE;
		}
	}
	return GE_SUCCESS;
}

int TianShi::p_lose_morale(int &step, CONTEXT_LOSE_MORALE *con)
{
	int ret = GE_SUCCESS;
	if (engine->getPlayerEntity(con->dstID)->getColor() == getColor() && con->harm.type == HARM_MAGIC && crystal+gem > 0)
	{
		step = SHEN_ZHI_BI_HU;
		ret = ShenZhiBiHu(con);
		if(toNextStep(ret) || ret == GE_URGENT){
			//全部走完后，请把step设成STEP_DONE
			step = STEP_DONE;
		}
	}
	return ret;
}


int TianShi::TianShiZhiQiang(Action *action)
{
	SkillMsg skill_msg;
	Coder::skillNotice(id, action->dst_ids(0), TIAN_SHI_ZHI_QIANG, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	CardMsg show_card;
	Coder::showCardNotice(id, 1, action->card_ids(0), show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, action->dst_ids(0), DECK_BASIC_EFFECT, action->card_ids(0), id, CAUSE_USE, true);
	return GE_SUCCESS;
}

int TianShi::TianShiZhuFu(int step, Action *action)
{
	int cardID = action->card_ids(0);
	if (action->dst_ids_size() > 1)
	{
		SkillMsg skill_msg;
		list<int> dst_ids;
		dst_ids.push_back(action->dst_ids(0));
		dst_ids.push_back(action->dst_ids(1));

		if(step != TIAN_SHI_ZHU_FU)
		{
			Coder::skillNotice(id, dst_ids, TIAN_SHI_ZHU_FU, skill_msg);
			engine->sendMessage(-1, MSG_SKILL, skill_msg);
			CardMsg show_card;
			Coder::showCardNotice(id, 1, cardID, show_card);
			engine->sendMessage(-1, MSG_CARD, show_card);
			engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, TIAN_SHI_ZHU_FU, true);
			//插入了新状态，请return GE_URGENT
		
			return GE_URGENT;
		}

		HARM zhufu;
		zhufu.cause = TIAN_SHI_ZHU_FU;
		zhufu.point = 1;
		zhufu.srcID = id;
		zhufu.type = HARM_NONE;
		//先进后出，所以逆出牌顺序压，由自己开始弃牌
		PlayerEntity* start = this->getPre();
		PlayerEntity* it = start;
		do{
			//没有手牌就不用弃
			if(it->getHandCardNum() > 0 && (it->getID() == action->dst_ids(0) || it->getID() == action->dst_ids(1))){
				engine->pushGameState(new StateRequestHand(it->getID(), zhufu, id, DECK_HAND, false, false));
			}
			it = it->getPre();
		}while(it != start);
	}
	else
	{
		SkillMsg skill_msg;

		if(step != TIAN_SHI_ZHU_FU)
		{

			Coder::skillNotice(id, action->dst_ids(0), TIAN_SHI_ZHU_FU, skill_msg);
			engine->sendMessage(-1, MSG_SKILL, skill_msg);
			CardMsg show_card;
			Coder::showCardNotice(id, 1, cardID, show_card);
			engine->sendMessage(-1, MSG_CARD, show_card);
			engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, TIAN_SHI_ZHU_FU, true);
			//插入了新状态，请return GE_URGENT
		
			return GE_URGENT;
		}

		HARM zhufu;
		zhufu.cause = TIAN_SHI_ZHU_FU;
		zhufu.point = 2;
		zhufu.srcID = id;
		zhufu.type = HARM_NONE;
		engine->pushGameState(new StateRequestHand(action->dst_ids(0), zhufu, id, DECK_HAND, false, false));
	}
	return GE_SUCCESS;
}

int TianShi::FengZhiJieJing(Action *action)
{
	int dstID = action->dst_ids(0);
	int card_id = action->card_ids(0);
	int effect_card_id = action->args(0);

	SkillMsg skill_msg;
	Coder::skillNotice(id, action->dst_ids(0), FENG_ZHI_JIE_JING, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	engine->pushGameState(new StateBasicEffectChange(dstID, CHANGE_REMOVE, effect_card_id, id, FENG_ZHI_JIE_JING));

	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, card_id, id, FENG_ZHI_JIE_JING, true);
	
	return GE_SUCCESS;
}

int TianShi::TianShiJiBan()
{
	CommandRequest cmd_req;
	int ret;
	int dstID;
	Coder::askForSkill(id, TIAN_SHI_JI_BAN, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			dstID = respond->dst_ids(0);
		}
	}
	else{
		dstID = id;
	}

	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, TIAN_SHI_JI_BAN, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	PlayerEntity* player = engine->getPlayerEntity(dstID);
	player->addCrossNum(1);
	GameInfo game_info;
	Coder::crossNotice(dstID, player->getCrossNum(), game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);

	return GE_SUCCESS;
}

int TianShi::TianShiZhiGe()
{
	PlayerEntity* player = this;
	bool has_basic_effect = false;
	do {
		if (player->getBasicEffect().size() > 0)
		{
			has_basic_effect = true;
			break;
		}
		player = player->getPost();
	}
	while (player != this);

	if (!has_basic_effect)
	{
		return GE_SUCCESS;
	}

	CommandRequest cmd_req;
	int ret;
	Coder::askForSkill(id, TIAN_SHI_ZHI_GE, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;

			if (respond->args_size() < 2)
			{
				return GE_SUCCESS;
			}

			int dstID = respond->dst_ids(0);
			PlayerEntity* dst = engine->getPlayerEntity(dstID);
			int card_id = respond->args(1);
			int useGem = respond->args(0);

			if (GE_SUCCESS != dst->checkBasicEffectByCard(card_id) || (useGem>0 && gem ==0) || (useGem==0 && crystal==0))
				return GE_INVALID_CARDID;

			if (useGem > 0)
				setGem(gem-1);
			else
				setCrystal(crystal-1);

			GameInfo game_info;
			Coder::energyNotice(id, gem, crystal, game_info);
			engine->sendMessage(-1, MSG_GAME, game_info);
			SkillMsg skill_msg;
			Coder::skillNotice(id, dstID, TIAN_SHI_ZHI_GE, skill_msg);
			engine->sendMessage(-1, MSG_SKILL, skill_msg);

			engine->pushGameState(new StateBasicEffectChange(dstID, CHANGE_REMOVE, card_id, id, TIAN_SHI_ZHI_GE));
			return GE_SUCCESS;
		}
	}
	else{
		return GE_TIMEOUT;
	}
}

int TianShi::ShenZhiBiHu(CONTEXT_LOSE_MORALE *con)
{
	CommandRequest cmd_req;
	int ret;
	Coder::askForSkill(id, SHEN_ZHI_BI_HU, cmd_req);
	Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size()-1));
	cmd->add_args(con->howMany);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			int gem_num = respond->args(0);
			int crystal_num = respond->args(1);
			int howmany = gem_num + crystal_num;
			if (howmany > 0 && howmany <= con->howMany && gem_num <= gem && crystal_num <= crystal)
			{
				SkillMsg skill_msg;
				Coder::skillNotice(id, con->dstID, SHEN_ZHI_BI_HU, skill_msg);
				engine->sendMessage(-1, MSG_SKILL, skill_msg);

				con->howMany -= howmany;
				setGem(gem - gem_num);
				setCrystal(crystal - crystal_num);

				GameInfo game_info;
				Coder::energyNotice(id, gem, crystal, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);

				return GE_SUCCESS;
			}
			else
			{
				return GE_SUCCESS;
			}
		}
	}
	else{
		return GE_TIMEOUT;
	}
}


