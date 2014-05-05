#include "QiDao.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

//祈祷师 的赐福效果处理涉及 UserTask::cmdMsgParse 和 PlayerEntity::p_additional_action 函数，查找请使用关键词[QiDao]
void QiDao::WeiLiCiFuParse(UserTask* session, int playerId, ::google::protobuf::Message *proto)
{
	Respond* respond = (Respond*)proto;
	session->tryNotify(playerId, STATE_TIMELINE_2_HIT,  WEI_LI_CI_FU, respond);
}

bool QiDao::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
			case QI_DAO:
				session->tryNotify(id, STATE_BOOT, QI_DAO, respond);
				return true;
			case FA_LI_CHAO_XI:
				session->tryNotify(id,STATE_AFTER_MAGIC,FA_LI_CHAO_XI,respond);
				return true;
		}
	}
	//没匹配则返回false
	return false;
}


//返回迅捷赐福效果的CardID，用于之后的移除
int QiDao::GetXunJieEffectCard(GameGrail* engine,int id){
	int targetCard = 0;
		PlayerEntity *target = engine->getPlayerEntity(id);
		list<BasicEffect> effects = target->getBasicEffect();
		for(list<BasicEffect>::iterator it = effects.begin(); it!=effects.end(); it++)
		{
			CardEntity* effectCard = getCardByID(it->card);
				//记录迅捷赐福效果的id
			if(effectCard->checkSpeciality(XUN_JIE_CI_FU)){
				targetCard = it->card;
			}	
		}
		return targetCard;
}

int QiDao::p_before_turn_begin(int &step, int currentPlayerID) 
{
	used_FaLiChaoXi = false;
	return GE_SUCCESS; 
}

int QiDao::p_boot(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	step = QI_DAO;
	if (currentPlayerID != id || getGem() == 0 || tap){
		return GE_SUCCESS;
	}
	ret = QiDong();
	if(toNextStep(ret))
	{
		step = STEP_DONE;
	}
	return ret;
}
//启动：祈祷
int QiDao::QiDong()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, QI_DAO, cmd_req);
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
				Coder::skillNotice(id, id, QI_DAO, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				tap = true;
				gem -= 1;
				GameInfo game_info;
				Coder::tapNotice(id, true, game_info);
				Coder::energyNotice(id, gem, crystal, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				return ret;
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

int QiDao::p_after_attack(int &step, int playerID){
	XunJieCiFuEffect(playerID);
	return GE_SUCCESS;
}
int QiDao::p_after_magic(int &step, int playerID){
	XunJieCiFuEffect(playerID);
	if(playerID != id){return GE_SUCCESS;}
	int ret = GE_INVALID_STEP;
	if(step == STEP_INIT)
	{
		step = XUN_JIE_CI_FU;
	}
	if(step = XUN_JIE_CI_FU)
	{
		ret = XunJieCiFuEffect(playerID);
		step = FA_LI_CHAO_XI;
	}
	if(step == FA_LI_CHAO_XI)
	{
		if(playerID != id){
			step = STEP_DONE;
			return GE_SUCCESS;
		}
		ret = FaLiChaoXi(playerID);
		if(toNextStep(ret)){
			//全部走完后，请把step设成STEP_DONE
			step = STEP_DONE;
		}
	}
	return ret;
}

//启动后，攻击+2符文
int QiDao::p_timeline_1(int &step, CONTEXT_TIMELINE_1* con){
	step = STEP_DONE;
	if(id != con->attack.srcID || con->attack.isActive == false){
		return GE_SUCCESS;
	}
	if(tap){
		setToken(0,(token[0]+2));
			//向客户端发送更新祈祷符文信息
		GameInfo update_info;
		Coder::tokenNotice(id, 0, token[0], update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
	}
	return GE_SUCCESS;
}
//法力潮汐：法术行动后可以消耗1水晶加1法术行动【回合限定】
int QiDao::FaLiChaoXi(int playerID){
	
	if(playerID != id){
		return GE_SUCCESS;
	}
	if(token[0] == 0 && handCards.empty()){
		return GE_SUCCESS;
	}
	if(getEnergy()>0 && used_FaLiChaoXi == false){
		addAction(ACTION_MAGIC, FA_LI_CHAO_XI);
	}
	return GE_SUCCESS;
}

int QiDao::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con)
{
	step = WEI_LI_CI_FU;
	int ret =WeiLiCiFuEffect(con);//可能返回GE_URGENT
	step = STEP_DONE;
	return ret;
}
//威力赐福效果：  队友有威力赐福，攻击命中后选择是否发动
int QiDao::WeiLiCiFuEffect( CONTEXT_TIMELINE_2_HIT * con){
	int ret = GE_INVALID_ACTION;
	int playerID = con->attack.srcID;

	PlayerEntity *target = engine->getPlayerEntity(playerID);
	list<BasicEffect> effects = target->getBasicEffect();
	bool pushed = false;
	for(list<BasicEffect>::iterator it = effects.begin(); it!=effects.end(); it++)
	{
		CardEntity* effectCard = getCardByID(it->card);
			
		//处理威力赐福
		if(effectCard->checkSpeciality(WEI_LI_CI_FU))
		{
						//等待玩家响应
			CommandRequest cmd_req;
			Coder::askForSkill(playerID, WEI_LI_CI_FU, cmd_req);
			if(engine->waitForOne(playerID, network::MSG_CMD_REQ, cmd_req))
			{
				void* reply;
				if (GE_SUCCESS == (ret = engine->getReply(playerID, reply)))
				{
					Respond* respond = (Respond*) reply;
					
					if(respond->args(0) == 1){
							//确认发动，宣告技能
						SkillMsg skill;
						Coder::skillNotice(playerID, playerID, WEI_LI_CI_FU, skill);
						engine->sendMessage(-1, MSG_SKILL, skill);

						con->harm.point += 2;

						engine->setStateMoveOneCardNotToHand(playerID, DECK_BASIC_EFFECT, -1, DECK_DISCARD, it->card, playerID, WEI_LI_CI_FU, true);	
						return GE_URGENT;
					}
					return GE_SUCCESS;
				}
					
			}else{
				return GE_TIMEOUT;
			}
		}
		
		
	}
	return GE_SUCCESS;
}

//迅捷赐福效果： 队友有迅捷赐福效果，可以发动额外行动，addAction
int QiDao::XunJieCiFuEffect(int playerID){
	int ret = GE_INVALID_ACTION;
	PlayerEntity *target = engine->getPlayerEntity(playerID);
	list<BasicEffect> effects = target->getBasicEffect();

	if(target->getHandCards().empty()){return GE_SUCCESS;}
	

	for(list<BasicEffect>::iterator it = effects.begin(); it!=effects.end(); it++)
	{
			CardEntity* effectCard = getCardByID(it->card);
			
		//处理迅捷赐福
		if(effectCard->checkSpeciality(XUN_JIE_CI_FU))
		{
		
			list<ACTION_QUOTA> additionalActions = target->getAdditionalAction();
			list<ACTION_QUOTA>::iterator it;
			for(it = additionalActions.begin(); it != additionalActions.end(); it++){
					if(XUN_JIE_CI_FU == it->cause){
						return GE_SUCCESS;
					}
			}
			target->addAction(ACTION_ATTACK,XUN_JIE_CI_FU);
		}
		
		
	}
	return GE_SUCCESS;
}
int QiDao::p_additional_action(int chosen){
	int playerID = engine->getCurrentPlayerID();
	SkillMsg skill_msg;

	if(chosen == XUN_JIE_CI_FU)// [Qidao]目前机制下，迅捷赐福判断不会经过祈祷的p_additional_action，因此移除效果在PlayerEntity::p_additional_action中完成
	{}
	else if(chosen ==FA_LI_CHAO_XI){
			//宣告技能
			Coder::skillNotice(id, id, FA_LI_CHAO_XI, skill_msg);
			engine->sendMessage(-1, MSG_SKILL, skill_msg);

			used_FaLiChaoXi = true;
			if(crystal>0){
				setCrystal(--crystal);
			}
			else{
				setGem(--gem);
			}
			GameInfo update_info;
			Coder::energyNotice(id, gem, crystal, update_info);
			engine->sendMessage(-1, MSG_GAME, update_info);
	
	}

	//做完角色相关的操作，扣除额外行动交给底层
	return PlayerEntity::p_additional_action(chosen);
}

int QiDao::v_additional_action(int chosen){

	int playerID = engine->getCurrentPlayerID();
	PlayerEntity *target = engine->getPlayerEntity(playerID);
	list<BasicEffect> effects = target->getBasicEffect();
	bool flag = false;
	CardEntity* effectCard = NULL;

	switch(chosen)
	{
	case XUN_JIE_CI_FU:
		
		for(list<BasicEffect>::iterator it = effects.begin(); it!=effects.end(); it++)
		{
				effectCard = getCardByID(it->card);
			
				//处理迅捷赐福
				if(effectCard->checkSpeciality(XUN_JIE_CI_FU))
				{
					flag = true;
				}
		
				
		}
		if(!flag){return GE_INVALID_ACTION;}
		break;
		
	case FA_LI_CHAO_XI:
		//回合限定       || 能量
		if(used_FaLiChaoXi || getEnergy() <= 0){
			return GE_INVALID_ACTION;
		}
		break;
	}
	return GE_SUCCESS;
}
int QiDao::GuangHuiXinYang(Action* action){//[法术]光辉信仰：祈祷形态下发动，移除1点祈祷符文，弃两张牌，我方战绩区+1宝石，目标队友+1治疗
	
	int dstID = action->dst_ids(0);
	//宣告技能
	SkillMsg skill_msg;
	Coder::skillNotice(dstID, id,GUANG_HUI_XIN_YANG, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	//移除符文
	token[0] -= 1;
	GameInfo update_info;
	Coder::tokenNotice(id, 0, token[0], update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	//团队+1宝石
	TeamArea* team = engine->getTeamArea();
	team->setGem(color, team->getGem(color)+1);
	Coder::stoneNotice(color, team->getGem(color), team->getCrystal(color), update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	vector<int> cards;
	int card_id;

	//队友+1治疗
	PlayerEntity *dstPlayer = engine->getPlayerEntity(dstID);
	dstPlayer->addCrossNum(1);
	Coder::crossNotice(dstID, dstPlayer->getCrossNum(), update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	//移除手牌
	for (int i = 0; i < action->card_ids_size(); ++i)
	{
		card_id = action->card_ids(i);

		cards.push_back(card_id);
	}
	if (cards.size() > 0)
	{
		int ret = engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cards.size(), cards, id, GUANG_HUI_XIN_YANG, true);
				
		SkillMsg skill_msg;
		Coder::skillNotice(id, id, GUANG_HUI_XIN_YANG, skill_msg);
		engine->sendMessage(-1, MSG_SKILL, skill_msg);
		return GE_URGENT;
	}


	return GE_SUCCESS;
	
}
int QiDao::QiHeiXinYang(Action* action){//[法术]漆黑信仰：祈祷形态下发动，移除1点祈祷符文，对目标角色和自己各造成2点伤害
	
	int dstID = action->dst_ids(0);
	//宣告技能
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID,QI_HEI_XIN_YANG, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	//移除祈祷符文
	token[0] -= 1;
	GameInfo update_info;
	Coder::tokenNotice(id, 0, token[0], update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);

	//制造伤害
	HARM harm;
	harm.cause = QI_HEI_XIN_YANG;
	harm.point = 2;
	harm.srcID = id;
	harm.type = HARM_MAGIC;
	//先结算对手，再结算自己，故自己先进状态栈
	engine->setStateTimeline3(id, harm);
	engine->setStateTimeline3(dstID, harm);
	return GE_URGENT;
	

}

bool QiDao::CheckCiFuTarget(Action* action, int CiFu){
	int dstID = action->dst_ids(0);
	PlayerEntity *target = engine->getPlayerEntity(dstID);
	list<BasicEffect> effects = target->getBasicEffect();
	bool pushed = false;
	for(list<BasicEffect>::iterator it = effects.begin(); it!=effects.end(); it++)
	{
		CardEntity* effectCard = getCardByID(it->card);
			
			//处理迅捷赐福
		if(effectCard->checkSpeciality(CiFu))
		{
			return false;	
		}
		
				
	}	
	return true;
}
int QiDao::XunJieCiFu(Action* action){
	int cardID = action->card_ids(0);
	int dstID = action->dst_ids(0);
	//宣告技能
	SkillMsg skill;
	Coder::skillNotice(id, dstID, XUN_JIE_CI_FU, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
    //所有移牌操作都要用setStateMoveXXXX，ToHand的话要填好HARM，就算不是伤害
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, dstID, DECK_BASIC_EFFECT, cardID, id, XUN_JIE_CI_FU, true);
	return GE_URGENT;
}

int QiDao::WeiLiCiFu(Action* action){
	int cardID = action->card_ids(0);
	int dstID = action->dst_ids(0);
	//宣告技能
	SkillMsg skill;
	Coder::skillNotice(id, dstID, WEI_LI_CI_FU, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);

    //所有移牌操作都要用setStateMoveXXXX，ToHand的话要填好HARM，就算不是伤害
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, dstID, DECK_BASIC_EFFECT, cardID, id, WEI_LI_CI_FU, true);
	return GE_URGENT;
}
//P_MAGIC_SKILL只有发起者执行，故不需要判断发起者，法术技能验证均在v_MAGIC_SKILL中
int QiDao::p_magic_skill(int &step, Action *action)
{
	int ret;
	int actionID = action->action_id();
	switch(actionID)
	{
		case GUANG_HUI_XIN_YANG:
			ret = GuangHuiXinYang(action);
			step = STEP_DONE;
			
			break;
		case QI_HEI_XIN_YANG:
			ret = QiHeiXinYang(action);
			step = STEP_DONE;
			
			break;
		case XUN_JIE_CI_FU:
			ret = XunJieCiFu(action);
			step = STEP_DONE;
			
			break;
		case WEI_LI_CI_FU:
			ret = WeiLiCiFu(action);
			step = STEP_DONE;
			
			break;
		default:
			return GE_INVALID_ACTION;
	}
	return ret;
}
//客户端按钮是否允许发动的检测由客户端完成。
int QiDao::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int playerID = action->src_id();
	int cardID = 0;
	CardEntity * card = NULL ;
	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}
	switch(actionID)
	{
		case GUANG_HUI_XIN_YANG:
			if(tap == true && token[0]>0){ //祈祷形态&符文数目>0
				return GE_SUCCESS;
			}
			
			break;
		case QI_HEI_XIN_YANG:
			if(tap == true && token[0]>0){
				return GE_SUCCESS;
			}
			
			break;
		case XUN_JIE_CI_FU:
		case WEI_LI_CI_FU:
			if( !CheckCiFuTarget(action,actionID)){
				return GE_INVALID_ACTION;
			}
			cardID = action->card_ids(0);
		    card = getCardByID(cardID);
			if(GE_SUCCESS != checkOneHandCard(cardID) || !card->checkSpeciality(actionID)){
				return GE_INVALID_ACTION;
			}
			return  GE_SUCCESS;;
			
			break;
		
		default:
			return GE_INVALID_ACTION;
	}
	return  GE_INVALID_ACTION;
}
