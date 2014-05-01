#include "MoDao.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"
//FIXME: 如果以后有人能转换法牌，会无法通过这里
int MoDao::v_request_hand(int cardSrc, int howMany, vector<int> cards, HARM harm)
{
	if(harm.cause == MO_BAO_CHONG_JI){
		int cardID = cards[0];
		CardEntity* card = getCardByID(cardID);
		if(card->getType() != TYPE_MAGIC){
			return GE_INVALID_CARDID;
		}
	}
	return GE_SUCCESS;
}
//若不弃牌
int MoDao::p_request_hand_give_up(int &step, int targetID, int cause)
{
	if(cause == MO_BAO_CHONG_JI){
		HARM moBao;
		moBao.cause = MO_BAO_CHONG_JI;
		moBao.point = 2;
		moBao.srcID = id;
		moBao.type = HARM_MAGIC;
		engine->setStateTimeline3(targetID, moBao);
		if(!isHit){
			isHit = true;
			TeamArea* m_teamArea = engine->getTeamArea();
			m_teamArea->setGem(color, m_teamArea->getGem(color)+1);
			GameInfo update_info;
			if (color == RED)
				update_info.set_red_gem(m_teamArea->getGem(color));
			else
				update_info.set_blue_gem(m_teamArea->getGem(color));
			engine->sendMessage(-1, MSG_GAME, update_info);
		}
		return GE_URGENT;
	}
	return GE_INVALID_ACTION;
}

int MoDao::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int cardID;
	int playerID = action->src_id();
	CardEntity* card;

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}
	switch(action->action_id())
	{
	case MO_BAO_CHONG_JI:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		//不是自己的手牌                          || 不是法术牌                 
		if(GE_SUCCESS != checkOneHandCard(cardID) || TYPE_MAGIC != card->getType()){
			return GE_INVALID_CARDID;
		}
		return checkTwoTarget(action);
	case MO_DAN_RONG_HE:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		//不是自己的手牌                          || 不是火地系牌                 
		if(GE_SUCCESS != checkOneHandCard(cardID) || ELEMENT_FIRE != card->getElement() && ELEMENT_EARTH != card->getElement()){
			return GE_INVALID_CARDID;
		}
		return GE_SUCCESS;
	case HUI_MIE_FENG_BAO:
		//能量             
		if(gem <= 0){
			return GE_INVALID_ACTION;
		}
		return checkTwoTarget(action);
	default:
		return GE_INVALID_ACTION;
	}
}

int MoDao::checkTwoTarget(Action* action)
{
	int dst1ID;
	int dst2ID;
	PlayerEntity* dst1;
	PlayerEntity* dst2;
	if(action->dst_ids_size()!=2){
		return GE_INVALID_PLAYERID;
	}
	dst1ID = action->dst_ids(0);
	dst2ID = action->dst_ids(1);
	dst1 = engine->getPlayerEntity(dst1ID);
	dst2 = engine->getPlayerEntity(dst2ID);
	//不是对手                   || 不是对手                  || 同一人
	if(dst1->getColor() == color || dst2->getColor() == color || dst1ID == dst2ID){
		return GE_INVALID_PLAYERID;
	}
	return GE_SUCCESS;
}

int MoDao::p_magic_skill(int &step, Action* action)
{
	//p_magic_skill不同于别的触发点，有且只有一个匹配，因此每一个技能完成时，务必把step设为STEP_DONE
	int ret;
	switch(action->action_id())
	{
	case MO_BAO_CHONG_JI:
		ret = MoBaoChongJi(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case MO_DAN_RONG_HE:
		ret = MoDanRongHe(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case HUI_MIE_FENG_BAO:
		ret = HuiMieFengBao(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int MoDao::v_missile(int cardID, int dstID, bool realCard)
{
	int ret;
	CardEntity* card = getCardByID(cardID);
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID) || card->getName() != NAME_MISSILE && card->getElement() != ELEMENT_FIRE && card->getElement() != ELEMENT_EARTH)){
		return GE_INVALID_CARDID; 
	}
	//PlayerEntity *it = this;
	//while((it = it->getPost())->getColor() == color)
	//	;
	//if(dstID == it->getID()){
	//	return GE_SUCCESS;
	//}
	//it = this;
	//while((it = it->getPre())->getColor() == color)
	//	;
	//if(dstID == it->getID()){
	//	return GE_SUCCESS;
	//}
	return GE_SUCCESS;
}

int MoDao::v_remissile(int cardID, bool realCard)
{
	int ret;
	CardEntity* card = getCardByID(cardID);
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID) || card->getName() != NAME_MISSILE && card->getElement() != ELEMENT_FIRE && card->getElement() != ELEMENT_EARTH)){
		return GE_INVALID_CARDID; 
	}
	return GE_SUCCESS;
}

int MoDao::MoBaoChongJi(Action* action)
{
	isHit = false;
	int cardID = action->card_ids(0);
	int dst1ID = action->dst_ids(0);
	int dst2ID = action->dst_ids(1);
	list<int> dstIDs;
	dstIDs.push_back(dst1ID);
	dstIDs.push_back(dst2ID);
	//宣告技能
	network::SkillMsg skill;
	Coder::skillNotice(id, dstIDs, MO_BAO_CHONG_JI, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	CardMsg show_card;
	Coder::showCardNotice(id, 1, cardID, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
    //所有移牌操作都要用setStateMoveXXXX，ToHand的话要填好HARM，就算不是伤害
	PlayerEntity* it = this->getPre();
	HARM moBao;
	moBao.cause = MO_BAO_CHONG_JI;
	moBao.point = 1;
	moBao.srcID = id;
	moBao.type = HARM_NONE;
	//先进后出，所以逆出牌顺序压，最后才是魔导自己明弃法牌
	while(it != this){
		if(it->getID() == dst1ID || it->getID() == dst2ID){
			engine->pushGameState(new StateRequestHand(it->getID(), moBao, -1, DECK_DISCARD, true, true));
		}
		it = it->getPre();
	}
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, MO_BAO_CHONG_JI, true);
	//插入了新状态，请return GE_URGENT
	return GE_URGENT;
}

int MoDao::MoDanRongHe(Action* action)
{
	//宣告技能
	network::SkillMsg skill;
	Coder::skillNotice(id, action->dst_ids(0), MO_DAN_RONG_HE, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	engine->pushGameState(StateMissiled::create(engine, action->card_ids(0), action->dst_ids(0), id));
	engine->setStateUseCard(action->card_ids(0), action->dst_ids(0), id);
	//插入了新状态，请return GE_URGENT
	return GE_URGENT;
}

int MoDao::HuiMieFengBao(Action* action)
{
	int dst1ID = action->dst_ids(0);
	int dst2ID = action->dst_ids(1);
	list<int> dstIDs;
	dstIDs.push_back(dst1ID);
	dstIDs.push_back(dst2ID);
	//宣告技能
	network::SkillMsg skill;
	Coder::skillNotice(id, dstIDs, HUI_MIE_FENG_BAO, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	//扣能量
	network::GameInfo update;
	setGem(--gem);
	Coder::energyNotice(id, gem, crystal, update);
	engine->sendMessage(-1, MSG_GAME, update);
    //填写伤害结构
	PlayerEntity* it = this->getPre();
	HARM huiMie;
	huiMie.cause = HUI_MIE_FENG_BAO;
	huiMie.point = 2;
	huiMie.srcID = id;
	huiMie.type = HARM_MAGIC;
	//先进后出，所以逆出牌顺序压
	while(it != this){
		if(it->getID() == dst1ID || it->getID() == dst2ID){
			engine->setStateTimeline3(it->getID(), huiMie);
		}
		it = it->getPre();
	}
	//插入了新状态，请return GE_URGENT
	return GE_URGENT;
}