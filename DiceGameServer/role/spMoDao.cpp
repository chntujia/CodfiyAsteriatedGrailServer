#include "spMoDao.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"
//FIXME: 如果以后有人能转换法牌，会无法通过这里

//——————————————————————主要增加内容。
bool spMoDao::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)  //萌导也有响应啦。
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case FA_LI_HU_DUN:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_3, FA_LI_HU_DUN, respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}
//法影
int spMoDao::p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con)
{
	if (con->dstID == id)
	{
		// 水影
		step = FA_LI_HU_DUN;
		int ret = FaLiHuDun(con);
		if(toNextStep(ret) || ret == GE_URGENT){
			//全部走完后，请把step设成STEP_DONE
			step = STEP_DONE;
		}
		return ret;
	}
	return GE_SUCCESS;
}

int spMoDao::FaLiHuDun(CONTEXT_TIMELINE_3 *con)
{
	CommandRequest cmd_req;
	int ret;
	Coder::askForSkill(id, FA_LI_HU_DUN, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			vector<int> cards;
			int card_id;
			if(respond->card_ids_size() == 0)
				return GE_SUCCESS;
			for (int i = 0; i < respond->card_ids_size(); ++i)
			{
				card_id = respond->card_ids(i);

				if (getCardByID(card_id)->getType()==TYPE_MAGIC  && checkOneHandCard(card_id) == GE_SUCCESS)
					cards.push_back(card_id);
			}
			if (cards.size() > 0)
			{
				//展示并丢弃手牌
				CardMsg show_card;
				Coder::showCardNotice(id, cards.size(), cards, show_card);
				engine->sendMessage(-1, MSG_CARD, show_card);
				engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cards.size(), cards, id, FA_LI_HU_DUN, true);
				
				SkillMsg skill_msg;
				Coder::skillNotice(id, id, FA_LI_HU_DUN, skill_msg);
				engine->sendMessage(-1, MSG_SKILL, skill_msg);
				return GE_URGENT;
			}
			else
			{
				return GE_INVALID_CARDID;
			}
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}
//---------------------以上是法力护盾，以下是改版魔爆涉及的。

int spMoDao::v_request_hand(int cardSrc, int howMany, vector<int> cards, HARM harm)
{
	if(harm.cause == SP_MO_BAO_CHONG_JI){
		int cardID = cards[0];
		CardEntity* card = getCardByID(cardID);
		if(card->getType() != TYPE_MAGIC){
			return GE_INVALID_CARDID;
		}
	}
	return GE_SUCCESS;
}

//若不弃牌
int spMoDao::p_request_hand_give_up(int &step, int targetID, int cause)
{
	if(cause == SP_MO_BAO_CHONG_JI){
		HARM spmoBao;
		spmoBao.cause = SP_MO_BAO_CHONG_JI;
		spmoBao.point = 2;
		spmoBao.srcID = id;
		spmoBao.type = HARM_MAGIC;
		engine->setStateTimeline3(targetID, spmoBao);
		return GE_URGENT;
	}
	return GE_INVALID_ACTION;
}

int spMoDao::v_magic_skill(Action *action)
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
	case SP_MO_BAO_CHONG_JI:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		//不是自己的手牌                          || 不是法术牌                 
		if(GE_SUCCESS != checkOneHandCard(cardID) || TYPE_MAGIC != card->getType()){
			return GE_INVALID_CARDID;
		}
		//SP魔爆仅指定单体。
		return GE_SUCCESS;
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

int spMoDao::checkTwoTarget(Action* action)
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

int spMoDao::p_magic_skill(int &step, Action* action)
{
	//p_magic_skill不同于别的触发点，有且只有一个匹配，因此每一个技能完成时，务必把step设为STEP_DONE
	int ret;
	switch(action->action_id())
	{
	case SP_MO_BAO_CHONG_JI:
		ret = spMoBaoChongJi(action);
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

int spMoDao::v_missile(int cardID, int dstID, bool realCard)
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

int spMoDao::v_remissile(int cardID, bool realCard)
{
	int ret;
	CardEntity* card = getCardByID(cardID);
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID) || card->getName() != NAME_MISSILE && card->getElement() != ELEMENT_FIRE && card->getElement() != ELEMENT_EARTH)){
		return GE_INVALID_CARDID; 
	}
	return GE_SUCCESS;
}

int spMoDao::spMoBaoChongJi(Action* action)
{
	int cardID = action->card_ids(0);
	int dstID = action->dst_ids(0);
	//宣告技能
	network::SkillMsg skill;
	Coder::skillNotice(id, dstID, SP_MO_BAO_CHONG_JI, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	CardMsg show_card;
	Coder::showCardNotice(id, 1, cardID, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
    //所有移牌操作都要用setStateMoveXXXX，ToHand的话要填好HARM，就算不是伤害
	PlayerEntity* it = this->getPre();
	HARM spmoBao;
	spmoBao.cause = SP_MO_BAO_CHONG_JI;
	spmoBao.point = 1;
	spmoBao.srcID = id;
	spmoBao.type = HARM_NONE;
	//先进后出，所以逆出牌顺序压，最后才是魔导自己明弃法牌
	while(it != this){
		if(it->getID() == dstID){
			engine->pushGameState(new StateRequestHand(it->getID(), spmoBao, -1, DECK_DISCARD, true, true));
		}
		it = it->getPre();
	}
	//直接增加宝石
	TeamArea* m_teamArea = engine->getTeamArea();
	m_teamArea->setGem(color, m_teamArea->getGem(color)+1);
	GameInfo update_info;
	if (color == RED)
		update_info.set_red_gem(m_teamArea->getGem(color));
	else
		update_info.set_blue_gem(m_teamArea->getGem(color));
	engine->sendMessage(-1, MSG_GAME, update_info);
	//
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, SP_MO_BAO_CHONG_JI, true);
	//插入了新状态，请return GE_URGENT
	return GE_URGENT;
}

int spMoDao::MoDanRongHe(Action* action)
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

int spMoDao::HuiMieFengBao(Action* action)
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