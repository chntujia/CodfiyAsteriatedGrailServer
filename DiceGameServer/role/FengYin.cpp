#include "FengYin.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

int FengYin::p_after_magic(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	//不是封印就不用跑了
	if(playerID != id){
		return GE_SUCCESS;
	}
	ret = FaShuJiDang(playerID);
	if(toNextStep(ret)){
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int FengYin::p_show_hand(int &step, int playerID, int howMany, vector<int> cards, HARM harm)
{
	int ret = GE_INVALID_STEP;
	ret = FengYin_Effect(playerID, howMany, cards);
	if(toNextStep(ret) || ret == GE_URGENT){
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int FengYin::v_magic_skill(Action *action)
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
	case FENG_ZHI_FENG_YIN:
	case SHUI_ZHI_FENG_YIN:
	case HUO_ZHI_FENG_YIN:
	case DI_ZHI_FENG_YIN:
	case LEI_ZHI_FENG_YIN:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		dst = engine->getPlayerEntity(action->dst_ids(0));
		//不是自己的手牌                          || 不是对应的封印牌                 || 目标已有该系的封印
		if(GE_SUCCESS != checkOneHandCard(cardID) || !card->checkSpeciality(actionID) || GE_SUCCESS == dst->checkBasicEffectBySpeciality(actionID) 
			                         // 目标不是对手
		                             || engine->getPlayerEntity(action->dst_ids(0))->getColor() == color){
			return GE_INVALID_ACTION;
		}
		break;
	case WU_XI_SHU_FU:
		//能量              || 目标不是对手
		if(getEnergy() <= 0 || engine->getPlayerEntity(action->dst_ids(0))->getColor() == color){
			return GE_INVALID_ACTION;
		}
		break;
	case FENG_YIN_PO_SUI:
		//能量              || 目标基础效果不存在
		if(getEnergy() <= 0 || GE_SUCCESS != engine->getPlayerEntity(action->dst_ids(0))->checkBasicEffectByCard(action->card_ids(0))){
			return GE_INVALID_ACTION;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int FengYin::p_magic_skill(int &step, Action* action)
{
	//p_magic_skill不同于别的触发点，有且只有一个匹配，因此每一个技能完成时，务必把step设为STEP_DONE
	int ret;
	switch(action->action_id())
	{
	case FENG_ZHI_FENG_YIN:
	case SHUI_ZHI_FENG_YIN:
	case HUO_ZHI_FENG_YIN:
	case DI_ZHI_FENG_YIN:
	case LEI_ZHI_FENG_YIN:
		ret = FengYin_Cast(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case WU_XI_SHU_FU:
		ret = WuXiShuFu(action);
		if(GE_SUCCESS == ret){
			step = STEP_DONE;
		}
		break;
	case FENG_YIN_PO_SUI:
		ret = FengYinPoSui(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int FengYin::FaShuJiDang(int playerID)
{
	if(playerID != id){
		return GE_SUCCESS;
	}
	addAction(ACTION_ATTACK, FA_SHU_JI_DONG);
	return GE_SUCCESS;
}

int FengYin::FengYin_Effect(int playerID, int howMany, vector<int> cards)
{
	PlayerEntity *target = engine->getPlayerEntity(playerID);
	list<BasicEffect> effects = target->getBasicEffect();
	bool pushed = false;
	for(list<BasicEffect>::iterator it = effects.begin(); it!=effects.end(); it++)
	{
		CardEntity* feng = getCardByID(it->card);
		//幻系攻击牌 == 封印牌
		if(feng->getType() == TYPE_ATTACK && feng->getProperty() == PROPERTY_PHANTOM)
		{
			for(int i = 0; i < howMany; i++)
			{
				CardEntity* card = getCardByID(cards[i]);
				if(card->getElement() == feng->getElement()){
					pushed = true;
					HARM feng;
					feng.type = HARM_MAGIC;
					feng.point = 3;
					feng.srcID = it->srcUser;
					feng.cause = getMapping(card->getElement());

					engine->setStateTimeline3(playerID, feng);
					engine->setStateMoveOneCardNotToHand(playerID, DECK_BASIC_EFFECT, -1, DECK_DISCARD, it->card, playerID, feng.cause, true);
					break;
				}
			}
			
		}
	}
	return pushed ? GE_URGENT : GE_SUCCESS;
}

int FengYin::FengYin_Cast(Action *action)
{
	int actionID = action->action_id();
	int cardID = action->card_ids(0);
	int dstID = action->dst_ids(0);
	
	//宣告技能
	network::SkillMsg skill;
	Coder::skillNotice(id, dstID, actionID, skill);
    //所有移牌操作都要用setStateMoveXXXX，ToHand的话要填好HARM，就算不是伤害
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, dstID, DECK_BASIC_EFFECT, cardID, id, actionID, true);
	//插入了新状态，请return GE_URGENT
	return GE_URGENT;
}

int FengYin::WuXiShuFu(Action *action)
{
	int dstID = action->dst_ids(0);
	//宣告技能
	network::SkillMsg skill;
	Coder::skillNotice(id, dstID, WU_XI_SHU_FU, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	//更新能量
	GameInfo update_info;
	if(crystal>0){
		setCrystal(--crystal);
	}
	else{
		setGem(--gem);
	}
	Coder::energyNotice(id, gem, crystal, update_info);
	//更新专属
	PlayerEntity* dst = engine->getPlayerEntity(dstID);
	dst->addExclusiveEffect(EX_WU_XI_SHU_FU);
	Coder::exclusiveNotice(dstID, dst->getExclusiveEffect(), update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	return GE_SUCCESS;
}

int FengYin::WuXiShuFu_Effect(GameGrail *engine)
{
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity* target = engine->getPlayerEntity(m_currentPlayerID);
	if(GE_SUCCESS != target->checkExclusiveEffect(EX_WU_XI_SHU_FU)){
		return 0;
	}
	//移除专属
	target->removeExclusiveEffect(EX_WU_XI_SHU_FU);
	GameInfo update_info;
	Coder::exclusiveNotice(m_currentPlayerID, target->getExclusiveEffect(), update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);

	int howMany = 2;
	PlayerEntity* pit = target;
	do{
		list<BasicEffect> effects = pit->getBasicEffect();
		for(list<BasicEffect>::iterator cit = effects.begin(); cit!=effects.end(); cit++)
		{
			CardEntity* feng = getCardByID(cit->card);
			//幻系攻击牌 == 封印牌
			if(feng->getType() == TYPE_ATTACK && feng->getProperty() == PROPERTY_PHANTOM){
				howMany++;
			}
		}
		pit = pit->getPost();
	}while(pit != target && howMany < 4);
	return howMany>4 ? 4 : howMany;
}

int FengYin::FengYinPoSui(Action *action)
{
	int dstID = action->dst_ids(0);
	int cardID = action->card_ids(0);
	//宣告技能
	network::SkillMsg skill;
	Coder::skillNotice(id, dstID, FENG_YIN_PO_SUI, skill);
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
	//所有移牌操作都要用setStateMoveXXXX，ToHand的话要填好HARM，就算不是伤害
	HARM move;
	move.cause = FENG_YIN_PO_SUI;
	move.point = 1;
	move.srcID = id;
	move.type = HARM_NONE;
	engine->setStateMoveOneCardToHand(dstID, DECK_BASIC_EFFECT, id, DECK_HAND, cardID, move, true);
	//插入了新状态，请return GE_URGENT
	return GE_URGENT;
}

int FengYin::getMapping(int element)
{
	switch(element)
	{
	case ELEMENT_WIND:
		return FENG_ZHI_FENG_YIN;
	case ELEMENT_WATER:
		return SHUI_ZHI_FENG_YIN;
	case ELEMENT_FIRE:
		return HUO_ZHI_FENG_YIN;
	case ELEMENT_EARTH:
		return DI_ZHI_FENG_YIN;
	case ELEMENT_THUNDER:
		return LEI_ZHI_FENG_YIN;
	default:
		return 0;
	}
}