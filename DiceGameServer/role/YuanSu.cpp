#include "YuanSu.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

YuanSu::YuanSu(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color)
{
	tokenMax[0] = 3;
}

int YuanSu::v_magic_skill(Action* action)
{
	int actionID = action->action_id();
	int cardID, cardID2;
	int playerID = action->src_id();
	CardEntity* card, *card2;
	PlayerEntity* dst;

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}

	switch (actionID)
	{
	case FENG_REN:
	case BING_DONG:
	case HUO_QIU:
	case YUN_SHI:
	case LEI_JI:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		dst = engine->getPlayerEntity(action->dst_ids(0));
		//不是自己的手牌                          || 不是对应的元素牌
		if(GE_SUCCESS != checkOneHandCard(cardID) || !card->checkSpeciality(actionID)){
			return GE_INVALID_ACTION;
		}
		if (action->card_ids_size() > 1)
		{
			cardID2 = action->card_ids(1);
			card2 = getCardByID(cardID2);
			if (card->getElement() != card2->getElement() || GE_SUCCESS != checkOneHandCard(cardID))
				return GE_INVALID_ACTION;
		}
		if (actionID == BING_DONG && action->dst_ids_size() < 2)
			// 冰冻需要2个目标
			return GE_INVALID_ACTION;

		break;
	case YUAN_SU_DIAN_RAN:
		dst = engine->getPlayerEntity(action->dst_ids(0));
		if (token[0] != tokenMax[0])
			return GE_INVALID_ACTION;
		break;
	case YUE_GUANG:
		dst = engine->getPlayerEntity(action->dst_ids(0));
		if (gem == 0)
			return GE_INVALID_ACTION;
		break;
	default:
		return GE_INVALID_ACTION;
	}

	return GE_SUCCESS;
}

int YuanSu::p_magic_skill(int &step, Action *action)
{
	int ret;
	switch(action->action_id())
	{
	case FENG_REN:
		ret = FengRen(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case BING_DONG:
		ret = BingDong(step, action);
		if (GE_URGENT == ret){
			// 冰冻处理分2断，伤害和加治疗
			step = BING_DONG;
		}
		else if (GE_SUCCESS == ret){
			step = STEP_DONE;
		}
		break;
	case HUO_QIU:
		ret = HuoQiu(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case YUN_SHI:
		ret = YunShi(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case LEI_JI:
		ret = LeiJi(step, action);
		if (GE_URGENT == ret){
			// 累计处理分2断，伤害和加宝石
			step = LEI_JI;
		}
		else if (GE_SUCCESS == ret){
			step = STEP_DONE;
		}
		break;
	case YUAN_SU_DIAN_RAN:
		ret = YuanSuDianRan(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case YUE_GUANG:
		ret = YueGuang(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}

	return ret;
}

// 元素吸收
int YuanSu::p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con)
{
	//  自己造成的伤害        && 魔法伤害                     && 不是元素点燃造成的伤害
	if (con->harm.srcID == id && con->harm.type == HARM_MAGIC && con->harm.cause != YUAN_SU_DIAN_RAN)
	{
		SkillMsg skill_msg;
		Coder::skillNotice(id, id, YUAN_SU_XI_SHOU, skill_msg);
		engine->sendMessage(-1, MSG_SKILL, skill_msg);

		setToken(0, getToken(0)+1);
		GameInfo game_info;
		Coder::tokenNotice(id, 0, getToken(0), game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);
	}
	return GE_SUCCESS;
}

int YuanSu::YunShi(Action *action)
{
	// 加一魔法行动，放在伤害前和后都一样，为了方便，所以放在伤害前了
	addAction(ACTION_MAGIC, YUN_SHI);
	return YuanSuDamage(action);
}

int YuanSu::FengRen(Action *action)
{
	// 加一攻击行动，放在伤害前和后都一样，为了方便，所以放在伤害前了
	addAction(ACTION_ATTACK, FENG_REN);
	return YuanSuDamage(action);
}

int YuanSu::HuoQiu(Action *action)
{
	return YuanSuDamage(action);
}

int YuanSu::LeiJi(int step, Action *action)
{
	if (step != LEI_JI)
	{
		return YuanSuDamage(action);
	}
	else
	{
		int dstID = action->dst_ids(0);
		GameInfo update_info;
		TeamArea* team_area = engine->getTeamArea();
		team_area->setGem(color, team_area->getGem(color)+1);
		if (color == RED)
			update_info.set_red_gem(team_area->getGem(color));
		else
			update_info.set_blue_gem(team_area->getGem(color));
		engine->sendMessage(-1, MSG_GAME, update_info);

		return GE_SUCCESS;
	}
}

int YuanSu::BingDong(int step, Action *action)
{
	if (step != BING_DONG)
	{
		return YuanSuDamage(action);
	}
	else
	{
		int dstID = action->dst_ids(0);
		PlayerEntity* dst = engine->getPlayerEntity(action->dst_ids(1));
		dst->addCrossNum(1);
		GameInfo game_info;
		Coder::crossNotice(dst->getID(), dst->getCrossNum(), game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);

		return GE_SUCCESS;
	}
}

int YuanSu::YuanSuDianRan(Action *action)
{
	// 加一魔法行动，放在伤害前和后都一样，为了方便，所以放在伤害前了
	addAction(ACTION_MAGIC, YUAN_SU_DIAN_RAN);

	int dstID = action->dst_ids(0);
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, YUAN_SU_DIAN_RAN, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	setToken(0, 0);
	GameInfo game_info;
	Coder::tokenNotice(id, 0, getToken(0), game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);

	HARM dianran;
	dianran.cause = YUAN_SU_DIAN_RAN;
	dianran.point = 2;
	dianran.srcID = id;
	dianran.type = HARM_MAGIC;
	engine->setStateTimeline3(dstID, dianran);

	return GE_URGENT;
}

int YuanSu::YueGuang(Action *action)
{
	int dstID = action->dst_ids(0);
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, YUE_GUANG, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	--gem;
	GameInfo game_info;
	Coder::energyNotice(id, gem, crystal, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);

	HARM yueguang;
	yueguang.cause = YUE_GUANG;
	yueguang.point = gem+crystal+1;
	yueguang.srcID = id;
	yueguang.type = HARM_MAGIC;
	engine->setStateTimeline3(dstID, yueguang);

	return GE_URGENT;
}

int YuanSu::YuanSuDamage(Action *action)
{
	int dstID = action->dst_ids(0);
	int magic_id = action->action_id();
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, magic_id, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	
	// 计算使用的手牌
	vector<int> cards;
	cards.push_back(action->card_ids(0));
	if (action->card_ids_size() > 1)
		cards.push_back(action->card_ids(1));

	// 制造伤害
	HARM harm;
	harm.cause = magic_id;
	harm.point = cards.size();
	if (magic_id == HUO_QIU)
		++harm.point;
	harm.srcID = id;
	harm.type = HARM_MAGIC;
	engine->setStateTimeline3(dstID, harm);

	// 丢弃手牌
	CardMsg show_card;
	Coder::showCardNotice(id, cards.size(), cards, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cards.size(), cards, id, magic_id, true);

	return GE_URGENT;
}