#include "YongZhe.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

YongZhe::YongZhe(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color)
{
	this->roleID = 21;//设置Role ID 为21， 用于在GrailState.cpp进行挑衅的攻击目标判断
	tokenMax[0]=4;//怒气 最大值均为4
	tokenMax[1]=4;//知性
	token[0] = token[1] = 0;
	  //【被动】勇者之心  初始+2 水晶
	setCrystal(2);
	GameInfo game_info;
	Coder::energyNotice(id, gem, crystal, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);

	tap = false;
	using_NuHou = 0;
}

int YongZhe::RemoveTiaoXinEffect(int currentPlayer){
	PlayerEntity* pe = engine->getPlayerEntity(currentPlayer);
	if( GE_SUCCESS == pe->checkExclusiveEffect(EX_TIAO_XIN)){
		pe->removeExclusiveEffect(EX_TIAO_XIN);
		GameInfo update_info;	
		Coder::exclusiveNotice(pe->getID(), pe->getExclusiveEffect(), update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
	}
	return GE_SUCCESS;
}

bool YongZhe::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
			case  JIN_DUAN_ZHI_LI:
				//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
				//禁断之力的两种情形共用id
				if(engine->topGameState()->state == STATE_TIMELINE_2_HIT){
					session->tryNotify(engine->getCurrentPlayerID(), STATE_TIMELINE_2_HIT,  JIN_DUAN_ZHI_LI, respond);
				}else{
					session->tryNotify(engine->getCurrentPlayerID(), STATE_TIMELINE_2_MISS, JIN_DUAN_ZHI_LI, respond);
				}
				return true;

			case NU_HOU:
				session->tryNotify(id, STATE_TIMELINE_1, NU_HOU, respond);
				return true;
			case SI_DOU:
				session->tryNotify(id,STATE_TIMELINE_6_DRAWN,SI_DOU, respond);
				return true;
			case MING_JING_ZHI_SHUI:
				session->tryNotify(id,STATE_TIMELINE_1,MING_JING_ZHI_SHUI, respond);
				return true;
		}
	}
	//没匹配则返回false
	return false;
}

//[响应]【怒吼】：（主动攻击前发动①，移除1点【怒气】）本次攻击伤害额外+2
int YongZhe::NuHou(CONTEXT_TIMELINE_1 *con){
	if (token[0] <=0){
		if(token[0] <0){
			return GE_FATAL_ERROR;
		}
		return GE_SUCCESS;
	}

	int ret = GE_SUCCESS;
		//等待玩家响应
	CommandRequest cmd_req;
	Coder::askForSkill(id, NU_HOU, cmd_req);
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			if(respond->args(0) == 1){
				//确认发动，宣告技能
				SkillMsg skill;
				Coder::skillNotice(id, con->attack.dstID, NU_HOU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				//消耗1怒气
				token[0]--;
				GameInfo update_info;
				Coder::tokenNotice(id, 0, token[0], update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);

				//伤害+2
				con->harm.point += 2;

				//设置怒吼标记
				using_NuHou = 1;
			}
		}
		return ret;
	}else{
		return GE_TIMEOUT;
	}

}

//[响应]【明镜止水】：（主动攻击前发动①，移除4点【知性】）本次攻击对方不能应战。
int YongZhe::MingJingZhiShui(CONTEXT_TIMELINE_1 *con){
	if (token[1] != 4){
		if(token[1] <0){
			return GE_FATAL_ERROR;
		}
		return GE_SUCCESS;
	}

	int ret = GE_SUCCESS;
		//等待玩家响应
	CommandRequest cmd_req;
	Coder::askForSkill(id, MING_JING_ZHI_SHUI, cmd_req);
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			if(respond->args(0) == 1){
				//确认发动，宣告技能
				SkillMsg skill;
				Coder::skillNotice(id, con->attack.dstID, MING_JING_ZHI_SHUI, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				//消耗4知性
				token[1]= 0 ;
				GameInfo update_info;
				Coder::tokenNotice(id, 1, token[1], update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);

				//不能应战
				con->hitRate = RATE_NOREATTACK;
			}
		}
		return ret;
	}else{
		return GE_TIMEOUT;
	}

}

int YongZhe::p_timeline_1(int &step, CONTEXT_TIMELINE_1* con){
	int ret = GE_INVALID_STEP;
	if(con->attack.srcID != id || !con->attack.isActive){ //确认攻击是否由该id发动
		return GE_SUCCESS;
	}
	if(step == STEP_INIT || step == NU_HOU){
		step = NU_HOU;
		ret = NuHou(con);
		if(toNextStep(ret)){
			step = MING_JING_ZHI_SHUI;
		}
	}
	if(step == MING_JING_ZHI_SHUI){				
		ret = MingJingZhiShui(con);
		if(toNextStep(ret)){
			step = STEP_DONE;
		}
	}
	return ret;
}

//若未命中，你+1【知性】。
int YongZhe::NuHouMiss(CONTEXT_TIMELINE_2_MISS *con){
	if(using_NuHou !=0){
		using_NuHou = 0;
		
		//+1【知性】
		setToken(1,token[1]+1);
		GameInfo update_info;
		Coder::tokenNotice(id, 1, token[1], update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
	}
	return GE_SUCCESS;
}

int YongZhe::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con)
{
	if(con->attack.srcID != id || !con->attack.isActive){ //确认攻击是否由该id发动
		return GE_SUCCESS;
	}
	step = JIN_DUAN_ZHI_LI;
	int ret = JinDuanZhiLiHit(con);
	if(toNextStep(ret) || ret == GE_URGENT){
		step = STEP_DONE;
	}
	return ret;
}

int YongZhe::p_before_turn_begin(int &step, int currentPlayerID){
	toRemoveTiaoXin = false;
	step = STEP_DONE;
	return GE_SUCCESS;
}

int YongZhe::p_before_action(int &step, int currentPlayerID) { 
	/*
		挑衅效果判断在 GrailState.cpp中完成，请使用[YongZhe]标记定位
	*/
	PlayerEntity* pe = engine->getPlayerEntity(currentPlayerID);
	if( GE_SUCCESS == pe->checkExclusiveEffect(EX_TIAO_XIN)){
		toRemoveTiaoXin = true;
	}
	
	//解除精疲力竭
	if(currentPlayerID == id){
		if(tap){
			//对自己造成3点法术伤害
			HARM harm;
			harm.cause = JING_PI_LI_JIE;
			harm.point = 3;
			harm.srcID = id;
			harm.type = HARM_MAGIC;
			engine->setStateTimeline3(id, harm);

			GameInfo game_info;
			tap = false;
			Coder::tapNotice(id, false, game_info);
			engine->sendMessage(-1, MSG_GAME, game_info);

			this->setHandCardsMaxFixed(false);
			engine->setStateChangeMaxHand(id, false, false,6 ,0);

			return GE_URGENT;
		}
	}

	return GE_SUCCESS;
}

int YongZhe::p_turn_end(int &step, int playerID)
{
	if(toRemoveTiaoXin){
		int ret = RemoveTiaoXinEffect(playerID);
		if(ret == GE_SUCCESS){
			step = STEP_DONE;
		}
		return ret;
	}
	else 
		return GE_SUCCESS;
}

int YongZhe::JingPiLiJie(){
	tap = true;
	GameInfo game_info;
	Coder::tapNotice(id, true, game_info);
	CONTEXT_TAP *con = new CONTEXT_TAP; con->id = id; con->tap = tap; engine->pushGameState(new StateTap(con));

	addAction(ACTION_ATTACK, JING_PI_LI_JIE);

	this->setHandCardsMaxFixed(true,4);
	Coder::handcardMaxNotice(id, this->getHandCardMax(), game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
	
	return GE_URGENT;
}

int YongZhe::JinDuanZhiLiHit(CONTEXT_TIMELINE_2_HIT *con)//[响应]A 【禁断之力】：【能量】×1 （主动攻击命中或未命中后发动②）弃掉你所有手牌【展示】，其中每有1张法术牌，你+1点【怒气】；若命中②，其中每有1张火系牌，本次攻击伤害额外+1，并对自己造成等同于火系牌数量的法术伤害③
{
	int energy = getEnergy();
	if(energy<=0){
		return GE_SUCCESS;	
	}
	int ret = GE_SUCCESS;

		//等待玩家响应
	CommandRequest cmd_req;
	Coder::askForSkill(id, JIN_DUAN_ZHI_LI, cmd_req);
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			
			Respond* respond = (Respond*) reply;
			if(respond->args(0) == 1){
				//确认发动，宣告技能
				SkillMsg skill;
				Coder::skillNotice(id, con->attack.dstID, JIN_DUAN_ZHI_LI, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				//消耗1能量
				//此处优先扣除水晶
				GameInfo update_info;
				if(crystal>0){
					setCrystal(--crystal);
				}
				else{
					setGem(--gem);
				}
				Coder::energyNotice(id, gem, crystal, update_info);

				//计算特殊牌数目
				vector<int> cards;
				int card_id = 0;
				int magicCardNum = 0;
				int fireCardNum = 0;
				for (int i = 0; i < respond->card_ids_size(); ++i)
				{
					card_id = respond->card_ids(i);
					CardEntity * ce = getCardByID(card_id);
					if(ce->getType() == TYPE_MAGIC){
						magicCardNum ++;
					}
					if(ce->getElement() == ELEMENT_FIRE){
						fireCardNum ++;
					}
					cards.push_back(card_id);
				}

				//每有1张法术牌，你+1点【怒气】
				setToken(0,token[0]+magicCardNum);

				Coder::tokenNotice(id, 0, token[0], update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);

				//每有1张火系牌，本次攻击伤害额外+1
				con->harm.point += fireCardNum;
				//发动精疲力竭
				JingPiLiJie();
				//对自己造成等同于火系牌数量的法术伤害③
				if(fireCardNum>0){
					HARM harm;
					harm.cause = JIN_DUAN_ZHI_LI;
					harm.point = fireCardNum;
					harm.srcID = id;
					harm.type = HARM_MAGIC;
					engine->setStateTimeline3(id, harm);
				}
				//展示并丢弃手牌
				if (cards.size() > 0)
				{
					CardMsg show_card;
					Coder::showCardNotice(id, cards.size(), cards, show_card);
					engine->sendMessage(-1, MSG_CARD, show_card);
					engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cards.size(), cards, id, JIN_DUAN_ZHI_LI, true);
					
				}
				
				return GE_URGENT;

			}
		}
		return ret;
	}else{
		return GE_TIMEOUT;
	}
}

int YongZhe::JinDuanZhiLiMiss(CONTEXT_TIMELINE_2_MISS *con)//[响应]A 【禁断之力】：【能量】×1 （主动攻击命中或未命中后发动②）弃掉你所有手牌【展示】，其中每有1张法术牌，你+1点【怒气】；若未命中②，其中每有1张水系牌，你+1点【知性】；
{
	int energy = getEnergy();
	if(energy<=0){
		if(energy < 0){
			return GE_FATAL_ERROR;
		}
		return GE_SUCCESS;
	
	}
	int ret = GE_SUCCESS;

			//等待玩家响应
	CommandRequest cmd_req;
	Coder::askForSkill(id, JIN_DUAN_ZHI_LI, cmd_req);
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			if(respond->args(0) == 1){
				//确认发动，宣告技能
				SkillMsg skill;
				Coder::skillNotice(id, con->dstID, JIN_DUAN_ZHI_LI, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				//消耗1能量
				//此处优先扣除水晶
				GameInfo update_info;
				if(crystal>0){
					setCrystal(--crystal);
				}
				else{
					setGem(--gem);
				}
				Coder::energyNotice(id, gem, crystal, update_info);

				//计算特殊牌数目
				vector<int> cards;
				int card_id = 0;
				int magicCardNum = 0;
				int waterCardNum = 0;
				for (int i = 0; i < respond->card_ids_size(); ++i)
				{
					card_id = respond->card_ids(i);
					CardEntity * ce = getCardByID(card_id);
					if(ce->getType() == TYPE_MAGIC){
						magicCardNum ++;
					}
					if(ce->getElement() == ELEMENT_WATER){
						waterCardNum ++;
					}
					cards.push_back(card_id);
				}

				//每有1张法术牌，你+1点【怒气】
				setToken(0,token[0]+magicCardNum);

				Coder::tokenNotice(id, 0, token[0], update_info);

				//每有1张水系牌，你+1点【知性】
				setToken(1,token[1]+waterCardNum);

				Coder::tokenNotice(id, 1, token[1], update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);

				//发动精疲力竭  , 插入了状态 因此必须返回urgent
				JingPiLiJie();
				//展示并丢弃手牌
				if (cards.size() > 0)
				{
					CardMsg show_card;
					Coder::showCardNotice(id, cards.size(), cards, show_card);
					engine->sendMessage(-1, MSG_CARD, show_card);
					engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cards.size(), cards, id, JIN_DUAN_ZHI_LI, true);					
				}
				return GE_URGENT;
			}
		}
	}
	return GE_SUCCESS;
}

int YongZhe::p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS * con)
{
	int ret = GE_INVALID_STEP;
	if(con->srcID != id || !con->isActive){ //确认攻击是否由该id发动
		return GE_SUCCESS;
	}
	if(step == STEP_INIT || step == NU_HOU){
		step = NU_HOU;
		ret = NuHouMiss(con);
		step = JIN_DUAN_ZHI_LI;
	}
	if(step == JIN_DUAN_ZHI_LI){
		ret = JinDuanZhiLiMiss(con);
		if(toNextStep(ret) || ret == GE_URGENT){
			step = STEP_DONE;
		}
	}

	return ret;
}

int YongZhe::SiDou(CONTEXT_TIMELINE_6_DRAWN *con){

	//消耗1个宝石
	if(gem <= 0){
		return GE_SUCCESS;
	}
	int ret = GE_SUCCESS;
		//等待玩家响应
	CommandRequest cmd_req;
	Coder::askForSkill(id, SI_DOU, cmd_req);
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			if(respond->args(0) == 1){
				//确认发动，宣告技能
				SkillMsg skill;
				Coder::skillNotice(id, con->dstID, SI_DOU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				//消耗宝石
				setGem(--gem);
				GameInfo update_info;
				Coder::energyNotice(id, gem, crystal, update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);

				//怒气+3
				setToken(0, (token[0] + 3));
				Coder::tokenNotice(id, 0, token[0], update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
			}
		}
		return ret;
	}else{
		return GE_TIMEOUT;
	}

}
int YongZhe::p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con){
	if(con->dstID == id){
		if(con->harm.point > 0 && con->harm.type == HARM_MAGIC){
			if(gem<=0){return GE_SUCCESS;}
			step = SI_DOU;
			int ret = SiDou(con);
			if(toNextStep(ret)){
				step = STEP_DONE;
			}
			return ret;
		}
	}
	return GE_SUCCESS;
	
}
int YongZhe::TiaoXin(Action* action)//[法术]【挑衅】：（移除1点【怒气】，将【挑衅】放置于目标对手面前）你+1【知性】；该对手在其下个行动阶段必须且只能主动攻击你，否则他跳过该行动阶段，触发后移除此牌。
{
	int dstID = action->dst_ids(0);

	//怒气-1
	token[0]--;
	GameInfo update_info;
	Coder::tokenNotice(id, 0, token[0], update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);

	//知性+1
	setToken(1,token[1]+1);
	Coder::tokenNotice(id, 1, token[1], update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);

	//更新专属
	PlayerEntity* dst = engine->getPlayerEntity(dstID);
	dst->addExclusiveEffect(EX_TIAO_XIN);
	Coder::exclusiveNotice(dstID, dst->getExclusiveEffect(), update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	
	//确认发动，宣告技能
	SkillMsg skill;
	Coder::skillNotice(id, dstID, TIAO_XIN, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
				
	return GE_SUCCESS;
}



//P_MAGIC_SKILL只有发起者执行，故不需要判断发起者，法术技能验证均在v_MAGIC_SKILL中
int YongZhe::p_magic_skill(int &step, Action *action)
{
	int ret;
	int actionID = action->action_id();
	switch(actionID)
	{
		case TIAO_XIN:
			ret = TiaoXin(action);
			step = STEP_DONE;
			
			break;
		
		default:
			return GE_INVALID_ACTION;
	}
	return ret;
}
//客户端按钮是否允许发动的检测由客户端完成。
int YongZhe::v_magic_skill(Action *action)
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
		case TIAO_XIN:
			if(token[0]>0){ //怒气>0
				return GE_SUCCESS;
			}
			
			break;
	
		
		default:
			return GE_INVALID_ACTION;
	}
	return  GE_INVALID_ACTION;
}
