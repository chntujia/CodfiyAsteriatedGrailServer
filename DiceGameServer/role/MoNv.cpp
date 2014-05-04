#include "MoNv.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool MoNv::cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		  case TI_SHEN_WAN_OU:
			session->tryNotify(id, STATE_TIMELINE_3,TI_SHEN_WAN_OU, respond);
			return true;
			break;
		  case MO_NENG_FAN_ZHUAN:
			session->tryNotify(id,STATE_TIMELINE_3,MO_NENG_FAN_ZHUAN, respond);
			return true;
			break;
		  case MO_NV_ZHI_NU:
			session->tryNotify(id,STATE_BOOT,MO_NV_ZHI_NU, respond);
			return true;
			break;
		}
	}
	//没匹配则返回false
	return false;
}

//统一在p_before_turn_begin 初始化各种回合变量
int MoNv::p_between_weak_and_action(int &step, int currentPlayerID)
{
	if(id != currentPlayerID)
		return GE_SUCCESS;
	int ret = GE_SUCCESS;
	ret = GetAwayFromFire();
	return ret;
}

int MoNv::GetAwayFromFire()
{
	if(!tap)
		return GE_SUCCESS;
	tap = false;
	GameInfo game_info;
	Coder::tapNotice(id, tap, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);

	int ret  = engine->setStateChangeMaxHand(id, false, false, 6, 2-token[0]);
	return ret;
}

int MoNv::getCardElement(int cardID)
{
	CardEntity* card = getCardByID(cardID);
	int element = card->getElement();
	if(tap)
	{
		if(card->getType() ==  TYPE_ATTACK && (element != ELEMENT_WATER && element != ELEMENT_DARKNESS))
			element = ELEMENT_FIRE;
	}
	return element;
}

bool MoNv::IsFired(int cardID)
{
	CardEntity* card = getCardByID(cardID);
	int element = card->getElement();
	if(tap && card->getType() ==  TYPE_ATTACK && (element != ELEMENT_WATER && element != ELEMENT_DARKNESS &&  element != ELEMENT_FIRE))
		return true;
	return false;
}

int MoNv::p_boot(int &step, int currentPlayerID)
{
	PlayerEntity *self = engine->getPlayerEntity(id);
	if (currentPlayerID != id || self->getHandCardNum() > 3)
		return GE_SUCCESS;
	step = MO_NV_ZHI_NU;
	int ret = MoNvZhiNu();
	if(toNextStep(ret) || ret == GE_URGENT){
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int MoNv::MoNvZhiNu()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, MO_NV_ZHI_NU, cmd_req);
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
				int drawNum = respond->args(1);
				if(drawNum > 3)
					return GE_INVALID_ARGUMENT;
				network::SkillMsg skill;
				Coder::skillNotice(id, id, MO_NV_ZHI_NU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				tap = true;

				GameInfo game_info;
				Coder::tapNotice(id, tap, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);

				int ret  = engine->setStateChangeMaxHand(id, false, false, 6, token[0]-2);
				if(drawNum > 0)
				{
					HARM harm;
					harm.srcID = id;
					harm.type = HARM_NONE;
					harm.point = drawNum;
					harm.cause = MO_NV_ZHI_NU;
					vector<int> cards;
					engine->setStateMoveCardsToHand(-1, DECK_PILE, id, DECK_HAND, drawNum, cards, harm, false);
					ret =  GE_URGENT;
				}
				return ret;
			}
			return GE_SUCCESS;
		}
		return ret;
	}
	else{
		return GE_TIMEOUT;
	}
	return GE_SUCCESS;
}

int MoNv::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int playerID = action->src_id();
	int cardID = 0;
	list<int> cardIDs;
	CardEntity * card = NULL ;
	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}
	switch(actionID)
	{
		case CANG_YAN_FA_DIAN:
			cardID = action->card_ids(0);
			if(GE_SUCCESS != checkOneHandCard(cardID) || this->getCardElement(cardID) != ELEMENT_FIRE){
				return GE_INVALID_ACTION;
			}
			return  GE_SUCCESS;

		case TIAN_HUO_DUAN_KONG:
			if(action->card_ids_size() != 2)
			{
				return GE_INVALID_ACTION;
			}
			for(int i =0;i<action->card_ids_size();i++)
			{
				cardID = action->card_ids(i);
				if(ELEMENT_FIRE != this->getCardElement(cardID) || GE_SUCCESS != checkOneHandCard(cardID))
				{
					return GE_INVALID_ACTION;
				}
			}
			if(!tap && token[0] < 1)
			{
				return GE_INVALID_ACTION;
			}
			return GE_SUCCESS;
		case TONG_KU_LIAN_JIE:
			if(getEnergy() < 1)
				return GE_INVALID_ACTION;
			int dstID = action->dst_ids(0);
			if(engine->getPlayerEntity(id)->getColor()==engine->getPlayerEntity(dstID)->getColor())
				return GE_INVALID_ACTION;
			return  GE_SUCCESS;
	}
	return  GE_INVALID_ACTION;
}

int MoNv::p_magic_skill(int &step, Action *action)
{

	int ret;
	int actionID = action->action_id();
	switch(actionID)
	{
		case CANG_YAN_FA_DIAN:
			ret = CangYanFaDian(action);
			step = STEP_DONE;
			break;
		case TIAN_HUO_DUAN_KONG:
			ret = TianHuoDuanKong(action);
			step = STEP_DONE;
			break;
		case TONG_KU_LIAN_JIE:
			if(step == STEP_INIT) {
				ret = TongKuLianJie(action);
				step = TONG_KU_LIAN_JIE_CARD;
			}
			else {
				ret = TongKuLianJieCard(action);
				step = STEP_DONE;
			}
			break;
		default:
			return GE_INVALID_ACTION;
	}
	return ret;
}

int MoNv::CangYanFaDian(Action *action)
{
	int cardID = action->card_ids(0);
	int dstID = action->dst_ids(0);
	CardEntity* card = getCardByID(cardID);

	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, CANG_YAN_FA_DIAN, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	CardMsg show_card;
	Coder::showCardNotice(id, 1, cardID, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);

	HARM selfHarm;
	selfHarm.cause = CANG_YAN_FA_DIAN;
	selfHarm.point = 2;
	selfHarm.srcID = id;
	selfHarm.type = HARM_MAGIC;
	engine->setStateTimeline3(id, selfHarm);

	HARM harm;
	harm.cause = CANG_YAN_FA_DIAN;
	harm.point = 2;
	harm.srcID = id;
	harm.type = HARM_MAGIC;

	engine->setStateTimeline3(dstID, harm);
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, CANG_YAN_FA_DIAN, true);

	return GE_URGENT;
}

int MoNv::TianHuoDuanKong(Action *action)
{
	int dstID = action->dst_ids(0);
	int cardID = 0;
	vector<int> cardIDs;
	cardIDs.push_back(action->card_ids(0));
	cardIDs.push_back(action->card_ids(1));

	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, TIAN_HUO_DUAN_KONG, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	CardMsg show_card;
	Coder::showCardNotice(id, 2, cardIDs, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);

	if(!tap)
	{
		setToken(0, token[0]-1);
		GameInfo game_info;
		Coder::tokenNotice(id, 0, token[0], game_info);
	}
	int harmPoint = 3;
	int dstColor = engine->getPlayerEntity(dstID)->getColor();
	int selfColor = engine->getPlayerEntity(id)->getColor();
	TeamArea *teamArea = engine->getTeamArea();
	if(teamArea->getMorale(dstColor) > teamArea->getMorale(selfColor))
		harmPoint = 4;
	HARM selfHarm;
	selfHarm.cause = TIAN_HUO_DUAN_KONG;
	selfHarm.point = harmPoint;
	selfHarm.srcID = id;
	selfHarm.type = HARM_MAGIC;
	engine->setStateTimeline3(id, selfHarm);

	HARM harm;
	harm.cause = TIAN_HUO_DUAN_KONG;
	harm.point = harmPoint;
	harm.srcID = id;
	harm.type = HARM_MAGIC;

	engine->setStateTimeline3(dstID, harm);
	engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, 2, cardIDs, id, TIAN_HUO_DUAN_KONG, true);

	return GE_URGENT;
}

int MoNv::TongKuLianJie(Action *action)
{
	int dstID = action->dst_ids(0);

	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, TONG_KU_LIAN_JIE, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	if(getCrystal()>0) {
		setCrystal(crystal-1);
	}
	else {
		setGem(gem-1);
	}
	GameInfo game_info;
	Coder::energyNotice(id, gem, crystal, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
	
	HARM selfHarm;
	selfHarm.cause = TONG_KU_LIAN_JIE;
	selfHarm.point = 1;
	selfHarm.srcID = id;
	selfHarm.type = HARM_MAGIC;
	engine->setStateTimeline3(id, selfHarm);

	HARM harm;
	harm.cause = TONG_KU_LIAN_JIE;
	harm.point = 1;
	harm.srcID = id;
	harm.type = HARM_MAGIC;
	engine->setStateTimeline3(dstID, harm);

	return GE_URGENT;
}

int MoNv::TongKuLianJieCard(Action *action)
{
	 if(this->getHandCardNum() < 4)
		 return GE_SUCCESS;
	 int cardNum = this->getHandCardNum() - 3;
	 HARM lianjie;
	 lianjie.cause = NIAN_ZHOU;
	 lianjie.point = cardNum;
	 lianjie.srcID = id;
	 lianjie.type = HARM_NONE;
	 engine->pushGameState(new StateRequestHand(id, lianjie, -1, DECK_DISCARD, false, false));
	 return GE_URGENT;
}

int MoNv::v_attack_skill(Action *action)
{
	int actionID = action->action_id();
	int playerID = action->src_id();
	int cardNum = action->card_ids_size();
	int ret;
	CardEntity* card;
	PlayerEntity* dst;

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}
	if(actionID != MO_NV_ZHI_NU_ATTACK){
		return GE_INVALID_ACTION;
	}
	vector<int> cardIDs(cardNum);
	for(int i = 0; i < cardNum; i++){
		cardIDs[i] = action->card_ids(i);
	}
	if(GE_SUCCESS != (ret = checkHandCards(cardNum, cardIDs))){
		return ret;
	}
	int virtualCardID = 87;
	if(GE_SUCCESS != (ret = v_attack(virtualCardID, action->dst_ids(0), false))){
		return ret;
	}
	return GE_SUCCESS;
}

int MoNv::p_attack_skill(int &step, Action* action)
{
	if(action->action_id() != MO_NV_ZHI_NU_ATTACK){
		return GE_INVALID_ACTION;
	}
	int ret = ToFire(action);
	if(toNextStep(ret)||ret==GE_URGENT){
		step = STEP_DONE;
	}
	return ret;
}

int MoNv::ToFire(Action *action)
{
	int actionID = action->action_id();
	int cardID = action->card_ids(0);
	int virtualCardID = 87;
	int dstID = action->dst_ids(0);

	//宣告技能

	engine->setStateTimeline1(virtualCardID, dstID, id, true);
	engine->setStateUseCard(virtualCardID, dstID, id, false, false);
	//所有移牌操作都要用setStateMoveXXXX，ToHand的话要填好HARM，就算不是伤害
	CardMsg show_card;
	Coder::showCardNotice(id, 1, cardID, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, MO_NV_ZHI_NU, true);
	//插入了新状态，请return GE_URGENT
	return GE_URGENT;
}

int MoNv::v_reattack(int cardID, int orignCardID, int dstID, int orignID, int rate, bool realCard)
{
	if(rate != RATE_NORMAL){
		return GE_INVALID_ACTION;
	}
	if(realCard){
		int ret;
		if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
			return ret;
		}
	}
	CardEntity* orignCard = getCardByID(orignCardID);
	if(orignCard->getElement() == ELEMENT_DARKNESS){
		return GE_INVALID_CARDID; 
	}
	CardEntity* card = getCardByID(cardID);		
	if(card->getType() != TYPE_ATTACK){
		return GE_INVALID_CARDID; 
	}
	if(card->getElement() != ELEMENT_DARKNESS && this->getCardElement(cardID) != orignCard->getElement()){
		return GE_INVALID_CARDID; 
	}

	PlayerEntity *dst = engine->getPlayerEntity(dstID);
	if(dstID == orignID || dst->getColor() == color){
		return GE_INVALID_PLAYERID;
	}
	return GE_SUCCESS;
}

int MoNv::p_reattack(int &step, int &cardID, int doerID, int targetID, bool &realCard)
{
	if(id != doerID || !tap)
		return GE_SUCCESS;
	int ret = GE_SUCCESS;;
	if(IsFired(cardID))
	{
		ret = ToFire(cardID, doerID, targetID, realCard);
		step = STEP_DONE;
	}
	return ret;
}

int MoNv::ToFire(int &cardID, int doerID, int targetID, bool &realCard)
{
	int virtualCardID = 87;
	//宣告技能

	//所有移牌操作都要用setStateMoveXXXX，ToHand的话要填好HARM，就算不是伤害
	cardID = virtualCardID;
	realCard = false;
	//插入了新状态，请return GE_URGENT
	return GE_URGENT;
}

int MoNv::p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con)
{
	int ret = GE_INVALID_STEP;
	if(con->dstID != id){
		return GE_SUCCESS;
	}

	if(step == STEP_INIT){
		step = TI_SHEN_WAN_OU;
	}
	if(con->harm.type == TYPE_ATTACK)
	{
		step = TI_SHEN_WAN_OU;
		ret = TiShenWanOu(con);
		if(toNextStep(ret) || ret == GE_URGENT){
			step = STEP_DONE;
		}
	}
	if(con->harm.type == TYPE_MAGIC)
	{
		step = MO_NENG_FAN_ZHUAN;
		ret = MoNengFanZhuan(con);
		if(toNextStep(ret) || ret == GE_URGENT){
			step = STEP_DONE;
		}
	}
	return ret;
}

int MoNv::TiShenWanOu(CONTEXT_TIMELINE_3 *con)
{
	int ret;
	if(con->dstID != id || con->harm.type != HARM_ATTACK){
		return GE_SUCCESS;
	}

	CommandRequest cmd_req;
	Coder::askForSkill(id, TI_SHEN_WAN_OU, cmd_req);
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			if(respond->args(0) == 1){
				
				int cardID = respond->card_ids(0);
				int mateID = respond->dst_ids(0);
				CardEntity* card = getCardByID(cardID);
				if( card->getType() != TYPE_MAGIC || GE_SUCCESS != checkOneHandCard(cardID)){
					return GE_INVALID_ARGUMENT;
				}

				PlayerEntity* self = engine->getPlayerEntity(id);
				PlayerEntity* mate = engine->getPlayerEntity(mateID);
				if(self->getColor()!=mate->getColor())
					return GE_INVALID_ARGUMENT;

				SkillMsg skill;
				Coder::skillNotice(id, mateID, TI_SHEN_WAN_OU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				vector<int> cards;
				HARM harm;
				harm.srcID = id;
				harm.type = HARM_NONE;
				harm.point = 1;
				harm.cause = TI_SHEN_WAN_OU;
				int ret = engine->setStateMoveCardsToHand(-1, DECK_PILE, mateID, DECK_HAND, 1, cards, harm, false);
				
				CardMsg show_card;
				Coder::showCardNotice(id, 1, cardID, show_card);
				engine->sendMessage(-1, MSG_CARD, show_card);
				engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, TI_SHEN_WAN_OU, true);
				//返回URGENT，终止目前状态，执行状态栈顶状态
				return GE_URGENT;

			}		
		}
		return ret;
	}else{
		return GE_TIMEOUT;
	}

	return GE_SUCCESS;
}

int MoNv::MoNengFanZhuan(CONTEXT_TIMELINE_3 *con)
{
	int ret;
	if(con->dstID != id || con->harm.type != HARM_MAGIC || this->getEnergy() < 1){
		return GE_SUCCESS;
	}

	CommandRequest cmd_req;
	Coder::askForSkill(id, MO_NENG_FAN_ZHUAN, cmd_req);
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			if(respond->args(0) == 1){
				
				int cardNum = respond->card_ids_size();
				vector<int> cardIDs;
				if(cardNum < 2) {
					return GE_INVALID_ARGUMENT;
				}
				for(int i = 0; i < cardNum;i ++)
				{
					CardEntity* card = getCardByID(respond->card_ids(i));
					if( card->getType() != TYPE_MAGIC)
						return GE_INVALID_ARGUMENT;
					cardIDs.push_back(respond->card_ids(i));
				}
				if( GE_SUCCESS != checkHandCards(cardIDs.size(), cardIDs)) {
					return GE_INVALID_CARDID;
				}

				int enermyID = respond->dst_ids(0);


				PlayerEntity* self = engine->getPlayerEntity(id);
				PlayerEntity* mate = engine->getPlayerEntity(enermyID);
				if(self->getColor()==mate->getColor())
					return GE_INVALID_ARGUMENT;

				SkillMsg skill;
				Coder::skillNotice(id, enermyID, MO_NENG_FAN_ZHUAN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				if(getCrystal()>0) {
					setCrystal(crystal-1);
				}
				else {
					setGem(gem-1);
				}
				GameInfo game_info;
				Coder::energyNotice(id, gem, crystal, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);

				vector<int> cards;
				HARM harm;
				harm.srcID = id;
				harm.type = HARM_MAGIC;
				harm.point = cardNum-1;
				harm.cause = MO_NENG_FAN_ZHUAN;
				engine->setStateTimeline3(enermyID, harm);
				
				CardMsg show_card;
				Coder::showCardNotice(id, cardNum, cardIDs, show_card);
				engine->sendMessage(-1, MSG_CARD, show_card);
				engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs, id, MO_NENG_FAN_ZHUAN, true);
				//返回URGENT，终止目前状态，执行状态栈顶状态
				return GE_URGENT;

			}		
		}
		return ret;
	}else{
		return GE_TIMEOUT;
	}

	return GE_SUCCESS;
}

int MoNv::p_true_lose_morale(int &step, CONTEXT_LOSE_MORALE *con)
{
	if(con->dstID != id || con->harm.type != TYPE_MAGIC)
		return GE_SUCCESS;
	step = YONG_SHENG_YIN_SHI_JI;
	int ret = YongShengYinShiJi(con);
	if(toNextStep(ret) || ret == GE_URGENT)
		step = STEP_DONE;
	return ret;
}

int MoNv::YongShengYinShiJi(CONTEXT_LOSE_MORALE *con)
{
	if(con->dstID != id || con->harm.type != TYPE_MAGIC || con->howMany < 1)
		return GE_SUCCESS;

	SkillMsg skill;
	Coder::skillNotice(id, id, YONG_SHENG_YIN_SHI_JI, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	setToken(0, token[0]+1);
	GameInfo game_info;
	Coder::tokenNotice(id, 0, token[0], game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
	if(tap)
	{
		return engine->setStateChangeMaxHand(id, false, false, 6, 1);
	}
	return GE_SUCCESS;
}