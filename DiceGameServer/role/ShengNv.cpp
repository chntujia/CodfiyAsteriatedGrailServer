#include "ShengNv.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool ShengNv::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case BING_SHUANG_DAO_YAN:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_1, BING_SHUANG_DAO_YAN, respond);
			return true;
		case LIAN_MIN:
			session->tryNotify(id, STATE_BOOT, LIAN_MIN, respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

int ShengNv::p_before_turn_begin(int &step, int currentPlayerID)
{
	step = SHENG_LIAO;
	used_ShengLiao = false;
	step = STEP_DONE;
	return GE_SUCCESS;
}

int ShengNv::p_boot(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	step = LIAN_MIN;
	if (currentPlayerID != id || getGem() == 0 || tap)
		return GE_SUCCESS;
	ret = LianMin();
	step = STEP_DONE;
	return ret;
}

int ShengNv::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	step = BING_SHUANG_DAO_YAN;
	int srcID = con->attack.srcID;
	int cardID = con->attack.cardID;
	CardEntity* card = getCardByID(cardID);
	if (srcID == id && card->getElement() == ELEMENT_WATER)
	{
		// 冰霜祷言
		ret = BingShuangDaoYan(con);
	}
	step = STEP_DONE;
	return ret;
}

int ShengNv::v_magic_skill(Action *action)
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
	case ZHI_LIAO_SHU:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		//不是自己的手牌                          || 不是对应的法术牌                
		if(GE_SUCCESS != checkOneHandCard(cardID) || !card->checkSpeciality(actionID)){
			return GE_INVALID_ACTION;
		}
		break;
	case ZHI_YU_ZHI_GUANG:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		if(GE_SUCCESS != checkOneHandCard(cardID) || !card->checkSpeciality(actionID) || action->dst_ids_size() < 1 || action->dst_ids_size() > 3){
			return GE_INVALID_ACTION;
		}
		break;
	case SHENG_LIAO:
		if(getEnergy() < 1 || used_ShengLiao){
			return GE_INVALID_ACTION;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int ShengNv::p_magic_skill(int &step, Action* action)
{
	//p_magic_skill不同于别的触发点，有且只有一个匹配，因此每一个技能完成时，务必把step设为STEP_DONE
	int ret;
	switch(action->action_id())
	{
	case ZHI_LIAO_SHU:
		ret = ZhiLiaoShu(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case ZHI_YU_ZHI_GUANG:
		ret = ZhiYuZhiGuang(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case SHENG_LIAO:
		ret = ShengLiao(action);
		if(GE_SUCCESS == ret){
			step = STEP_DONE;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}
int ShengNv::LianMin()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, LIAN_MIN, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if (respond->args(0) == 1)
			{
				network::SkillMsg skill;
				Coder::skillNotice(id, id, LIAN_MIN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				tap = true;
				gem -= 1;
				GameInfo game_info;
				Coder::tapNotice(id, true, game_info);
				Coder::energyNotice(id, gem, crystal, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);

				return engine->setStateChangeMaxHand(id, true, true, 7);
			}
			else
				return ret;
		}
		else
			return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int ShengNv::BingShuangDaoYan(CONTEXT_TIMELINE_1 *con)
{
	CommandRequest cmd_req;
	int ret;
	Coder::askForSkill(id, BING_SHUANG_DAO_YAN, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			int dstID = respond->dst_ids(0);
			PlayerEntity * dstPlayer = engine->getPlayerEntity(dstID);
			SkillMsg skill_msg;
			Coder::skillNotice(id, id, BING_SHUANG_DAO_YAN, skill_msg);
			engine->sendMessage(-1, MSG_SKILL, skill_msg);
			dstPlayer->addCrossNum(1);
			GameInfo update_info;
			Coder::crossNotice(dstID, dstPlayer->getCrossNum(), update_info);
			engine->sendMessage(-1, MSG_GAME, update_info);
		}
		return ret;
	}
	else{
		//超时为自己加治疗
		int dstID = id;
		PlayerEntity * dstPlayer = engine->getPlayerEntity(dstID);
		SkillMsg skill_msg;
		Coder::skillNotice(id, id, BING_SHUANG_DAO_YAN, skill_msg);
		engine->sendMessage(-1, MSG_SKILL, skill_msg);
		dstPlayer->addCrossNum(1);
		GameInfo update_info;
		Coder::crossNotice(dstID, dstPlayer->getCrossNum(), update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
		return GE_TIMEOUT;
	}
}

int ShengNv::ZhiLiaoShu(Action* action)
{
	int dstID = action->dst_ids(0);
	int cardID = action->card_ids(0);
	PlayerEntity * dstPlayer = engine->getPlayerEntity(dstID);
	
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, ZHI_LIAO_SHU, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, ZHI_LIAO_SHU, true);

	dstPlayer->addCrossNum(2);
	GameInfo update_info;
	Coder::crossNotice(dstID, dstPlayer->getCrossNum(), update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	//插入了新状态，请return GE_URGENT
	return GE_URGENT;
}

int ShengNv::ZhiYuZhiGuang(Action* action)
{
	int cardID = action->card_ids(0);
	SkillMsg skill_msg;
	list<int> dsts;
	int dstID;
	PlayerEntity * dstPlayer;
	for(int i=0; i < action->dst_ids_size(); i ++)
	{
		dsts.push_back(action->dst_ids(i));
	}
	Coder::skillNotice(id, dsts, ZHI_YU_ZHI_GUANG, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, ZHI_YU_ZHI_GUANG, true);

	for(int i=0; i < action->dst_ids_size(); i ++)
	{
		dstID = action->dst_ids(i);
		dstPlayer = engine->getPlayerEntity(dstID);
		dstPlayer->addCrossNum(1);
		GameInfo update_info;
		Coder::crossNotice(dstID, dstPlayer->getCrossNum(), update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
	}
	//插入了新状态，请return GE_URGENT
	return GE_URGENT;
}

int ShengNv::ShengLiao(Action* action)
{
	SkillMsg skill_msg;
	list<int> dsts;
	int dstID;
	PlayerEntity * dstPlayer;
	for(int i=0; i < action->dst_ids_size(); i ++)
	{
		dsts.push_back(action->dst_ids(i));
	}
	Coder::skillNotice(id, dsts, SHENG_LIAO, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	//更新能量
	GameInfo update_info;
	if(crystal>0){
		setCrystal(--crystal);
	}
	else{
		setGem(--gem);
	}
	Coder::energyNotice(id, gem, crystal, update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	for(int i=0; i < action->dst_ids_size(); i ++)
	{
		dstID = action->dst_ids(i);
		dstPlayer = engine->getPlayerEntity(dstID);
		dstPlayer->addCrossNum(action->args(i));
		GameInfo update_info;
		Coder::crossNotice(dstID, dstPlayer->getCrossNum(), update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
	}
	used_ShengLiao = true;
	addAction(ACTION_ATTACK_MAGIC, SHENG_LIAO);
	return GE_SUCCESS;
}