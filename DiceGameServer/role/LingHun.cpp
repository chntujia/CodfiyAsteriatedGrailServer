#include "LingHun.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

LingHun::LingHun(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color)
{
	tokenMax[0] = 6;
	tokenMax[1] = 6;
	used_LING_HUN_ZENG_FU = false;
	used_LING_HUN_LIAN_JIE = false;
	using_LING_HUN_LIAN_JIE = false;
	connectID = -1;
}


bool LingHun::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case LING_HUN_ZHUAN_HUAN:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id,STATE_TIMELINE_1,LING_HUN_ZHUAN_HUAN, respond);
			return true;

		case LING_HUN_LIAN_JIE:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id,STATE_BOOT,LING_HUN_LIAN_JIE, respond);  //用什么状态？？？
			return true;

		case LING_HUN_ZENG_FU:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_BOOT, LING_HUN_ZENG_FU, respond);  //用什么状态？？？
			return true;
		case LING_HUN_LIAN_JIE_REACT:
			session->tryNotify(id, STATE_TIMELINE_6, STEP_INIT, respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

//统一在p_before_turn_begin 初始化各种回合变量
int LingHun::p_before_turn_begin(int &step, int currentPlayerID) 
{
	used_LING_HUN_ZENG_FU = false;
	return GE_SUCCESS; 
}

int LingHun::p_boot(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	if (currentPlayerID != id ){
		return GE_SUCCESS;
	}
	if(step == STEP_INIT)
	{
		step = LING_HUN_ZENG_FU;
	}
	if(step == LING_HUN_ZENG_FU)
	{
		ret = LingHunZengFu();
		if(toNextStep(ret)){
			if(token[0] > 0 && token[1] > 0 && !used_LING_HUN_LIAN_JIE && !used_LING_HUN_ZENG_FU)
				step = LING_HUN_LIAN_JIE;
			else
				step = STEP_DONE;
		}
	}
	if(step == LING_HUN_LIAN_JIE)
	{
		ret = LingHunLianJie();
		if(toNextStep(ret)){
			step = STEP_DONE;
		}
	}
	return ret;
}

int LingHun::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	if(step == STEP_INIT)
	{
		step = LING_HUN_ZHUAN_HUAN;
	}
	if(step == LING_HUN_ZHUAN_HUAN)
	{
		ret = LingHunZhuanHuan(con);
		if(toNextStep(ret) || ret == GE_URGENT){
			step = STEP_DONE;
		}
	}
	return ret;
}


int LingHun::p_timeline_6(int &step, CONTEXT_TIMELINE_6 *con) 
{    
	//【灵魂链接】已使用          被攻击对象为灵魂或链接对象                                                         
	if(used_LING_HUN_LIAN_JIE && (con->dstID == id || con->dstID == connectID))
	{
		int ret = GE_INVALID_STEP;
		//初始化step
		if(step == STEP_INIT){
			ret = LingHunLianJieReact(con);
			if(ret == GE_URGENT){
				step = LING_HUN_LIAN_JIE_REACT;
			}
		}
		else if(step == LING_HUN_LIAN_JIE_REACT){
			using_LING_HUN_LIAN_JIE = false;
			step = STEP_DONE;
			ret = GE_SUCCESS;
		}
		return ret;
	}
	else{
		return GE_SUCCESS;
	}
}

//【灵魂术士】：普通技：被动 【灵魂吞噬】：（我方每有1点士气下降）你+1【黄色灵魂】

int LingHun::p_true_lose_morale(int &step, CONTEXT_LOSE_MORALE *con) 
{ 
	//id-->new_color-->if(new_color=color) 
	int dst_color;
	int current_color;
	int harmPoint=con->howMany;
	dst_color=engine->getPlayerEntity(con->dstID)->getColor();
	current_color=this->getColor();

	if(dst_color==current_color)
	{
		setToken(0,token[0]+harmPoint);  //+1【黄魂】
		network::GameInfo update;
		Coder::tokenNotice(id,0,token[0],update);
		engine->sendMessage(-1, MSG_GAME, update);
	}   
	return  GE_SUCCESS;
}

int LingHun::v_magic_skill(Action *action)
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
	case  LING_HUN_CI_YU:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		//不是自己的手牌                          || 不是对应的法术牌                  ||蓝魂数量小于3  
		if(GE_SUCCESS != checkOneHandCard(cardID) || !card->checkSpeciality(actionID)||getToken(1)<3){
			return GE_INVALID_ACTION;
		}
		break;
	case LING_HUN_ZHEN_BAO:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		//不是自己的手牌                          || 不是对应的法术牌                  ||黄魂数量小于3  
		if(GE_SUCCESS != checkOneHandCard(cardID) || !card->checkSpeciality(actionID)||getToken(0)<3){
			return GE_INVALID_ACTION;
		}
		break;

	case LING_HUN_JING_XIANG:
		//黄色灵魂---个数不够
		if(getToken(0)<2){
			return GE_INVALID_ACTION;
		}
		break;

	case LING_HUN_ZHAO_HUAN:
		return GE_SUCCESS;
		break;

	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int LingHun::p_magic_skill(int &step, Action* action)
{
	//p_magic_skill不同于别的触发点，有且只有一个匹配，因此每一个技能完成时，务必把step设为STEP_DONE
	int ret;
	switch(action->action_id())
	{
	case  LING_HUN_CI_YU:
		ret = LingHunCiYu(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case LING_HUN_ZHEN_BAO:
		ret = LingHunZhenBao(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}	
		break;
	case LING_HUN_JING_XIANG:
		ret = LingHunJingXiang(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}	
		break;
	case LING_HUN_ZHAO_HUAN:
		//灵魂召唤
		ret = LingHunZhaoHuan(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}	
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

//--------------【灵魂增幅】-------------------------
int LingHun::LingHunZengFu()
{
	if(getGem() <=0)
		return GE_SUCCESS;
	CommandRequest cmd_req;
	Coder::askForSkill(id, LING_HUN_ZENG_FU, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{			
			Respond* respond = (Respond*) reply;
			//发动
			if (respond->args(0) == 1)  //自己定义
			{
				if (gem > 0)
				{
					used_LING_HUN_ZENG_FU = true;
					network::SkillMsg skill_msg;
					Coder::skillNotice(id, id, LING_HUN_ZENG_FU, skill_msg);
					engine->sendMessage(-1, MSG_SKILL, skill_msg);
					setGem(--gem);
					setToken(0,token[0]+2); //黄色灵魂+2
					setToken(1,token[1]+2); //蓝色灵魂+2
					GameInfo game_info;
					Coder::tokenNotice(id,0,token[0],game_info);
					Coder::tokenNotice(id,1,token[1], game_info);
					Coder::energyNotice(id, gem, crystal, game_info);
					engine->sendMessage(-1, MSG_GAME, game_info);
					return GE_SUCCESS;
				}
				else
					return GE_INVALID_ACTION;
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
//-----------【灵魂赐予】--------------
int LingHun::LingHunCiYu(Action* action)
{
	int dstID = action->dst_ids(0);
	int cardID = action->card_ids(0);
	PlayerEntity * dstPlayer = engine->getPlayerEntity(dstID);
	CardEntity* card = getCardByID(cardID);
	//检查牌是否为对应技能牌
	if( !card->checkSpeciality(LING_HUN_CI_YU)){
		return GE_SUCCESS;
	}


	//宣告技能
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, LING_HUN_CI_YU, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);	

	setToken(1,token[1]-3);              //蓝色灵魂-3	
	dstPlayer->setGem(dstPlayer->getGem()+2);
	network::GameInfo update;
	Coder::tokenNotice(id,1,token[1],update);
	Coder::energyNotice(dstID, dstPlayer->getGem(), dstPlayer->getCrystal(), update);
	engine->sendMessage(-1, MSG_GAME, update);

	//展示手牌
	CardMsg show_card;
	Coder::showCardNotice(id, 1, cardID, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id,LING_HUN_CI_YU, true);
	return GE_URGENT;
}
//-------------【灵魂震爆】--------------
int LingHun::LingHunZhenBao(Action *action)
{
	int dstID = action->dst_ids(0);
	int magic_id = action->action_id();
	int cardID = action->card_ids(0);
	PlayerEntity * dstPlayer = engine->getPlayerEntity(dstID);

	CardEntity* card = getCardByID(cardID);
	//检查牌是否为对应技能牌
	if( !card->checkSpeciality(LING_HUN_ZHEN_BAO)){
		return GE_SUCCESS;
	}

	//宣告技能
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, magic_id, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	//展示手牌
	CardMsg show_card;
	Coder::showCardNotice(id, 1, cardID, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);

	setToken(0,token[0]-3); //黄色灵魂-3	
	network::GameInfo update;
	Coder::tokenNotice(id,0,token[0],update);
	engine->sendMessage(-1, MSG_GAME, update);

	// 制造伤害
	HARM harm;
	harm.cause = magic_id;
	if(dstPlayer->getHandCardNum()<3&&dstPlayer->getHandCardMax()>5)
		harm.point = 5;
	else 
		harm.point=3;
	harm.srcID = id;
	harm.type = HARM_MAGIC;
	engine->setStateTimeline3(dstID, harm);

	// 丢弃手牌
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, LING_HUN_ZHEN_BAO, true);

	return GE_URGENT;
}

int LingHun::LingHunJingXiang(Action* action)
{
	vector<int> cardIDs;
	vector<int> cards;
	int cardNum= action->card_ids_size();
	int  dstID = action->dst_ids(0);
	PlayerEntity *dstPlayer = engine->getPlayerEntity(dstID);
	int max=dstPlayer->getHandCardMax();
	int currentHandCards=dstPlayer->getHandCardNum();

	setToken(0,token[0]-2); //黄色灵魂-2	
	network::GameInfo update;
	Coder::tokenNotice(id,0,token[0],update);

	engine->sendMessage(-1, MSG_GAME, update);

	if(cardNum>3)      cardNum=3;

	for(int i = 0; i < cardNum;i ++)
	{
		cardIDs.push_back(action->card_ids(i));
	}
	//宣告技能
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, LING_HUN_JING_XIANG, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

    int drawNum = (currentHandCards+3)<max?3:(max-currentHandCards);
	if(drawNum>0){
		HARM  jingXiang;
		jingXiang.type = HARM_NONE;
		jingXiang.point = drawNum; 
		jingXiang.srcID = id;
		jingXiang.cause = LING_HUN_JING_XIANG;
		engine->setStateMoveCardsToHand(-1, DECK_PILE, dstID, DECK_HAND, jingXiang.point, vector< int >(), jingXiang, false);
	}
	if(cardNum>0){
		engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs, id, LING_HUN_JING_XIANG, false);//弃牌，伤害
	}

	//插入了新状态，请return GE_URGENT
	return GE_URGENT;
}

int LingHun::LingHunZhaoHuan(Action* action)
{
	vector<int> cardIDs;
	vector<int> cards;

	int cardNum= action->card_ids_size();
	setToken(1,token[1]+cardNum+1);  //蓝色灵魂+（X+1)	
	network::GameInfo update;
	Coder::tokenNotice(id,1,token[1], update);
	engine->sendMessage(-1, MSG_GAME, update);

	for(int i = 0; i < cardNum;i ++)
	{
		cardIDs.push_back(action->card_ids(i));
	}
	//宣告技能
	SkillMsg skill_msg;
	Coder::skillNotice(id, id,LING_HUN_ZHAO_HUAN, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	//弃X张牌
	CardMsg show_card;
	Coder::showCardNotice(id, cardNum, cardIDs, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs, id, LING_HUN_ZHAO_HUAN, true);//弃牌，伤害

	//插入了新状态，请return GE_URGENT
	return GE_URGENT;

}

int LingHun::LingHunZhuanHuan(CONTEXT_TIMELINE_1 *con)
{
	//黄魂转换为蓝魂
	//蓝魂转换为黄魂
	if(!con->attack.isActive || (token[0]<=0 && token[1]<=0))
		return GE_SUCCESS;
	int ret;
	//满足发动条件，询问客户端是否发动
	CommandRequest cmd_req;
	Coder::askForSkill(id, LING_HUN_ZHUAN_HUAN, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动 
			if(respond->args(0) == 0||respond->args(0) == 1){
				if(respond->args(0) == 0)  //黄魂 转换为 蓝魂(如果蓝魂已满，是否要做判断，进行处理？？？)
				{
					setToken(0,token[0]-1);
					setToken(1,token[1]+1);
				}
				else
				{
					setToken(0,token[0]+1);
					setToken(1,token[1]-1);	   
				}
				network::GameInfo update_info;
				Coder::tokenNotice(id,0,token[0], update_info);
				Coder::tokenNotice(id,1,token[1], update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);

				network::SkillMsg skill;
				Coder::skillNotice(id, id, LING_HUN_ZHUAN_HUAN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
			}
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}


//【灵魂链接】
int LingHun::LingHunLianJie(){

	int ret;
	int dstID;
	//满足发动条件，询问客户端是否发动
	CommandRequest cmd_req;
	Coder::askForSkill(id, LING_HUN_LIAN_JIE, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动 
			if(respond->args(0) == 1){
				//待续
				dstID=respond->dst_ids(0);
				PlayerEntity* dst = engine->getPlayerEntity(dstID);
				dst->addExclusiveEffect(EX_LING_HUN_LIAN_JIE);
				addExclusiveEffect(EX_LING_HUN_LIAN_JIE);

				setToken(0,token[0]-1);  //黄色灵魂-1	
				setToken(1,token[1]-1);  //蓝色灵魂-1	

				network::GameInfo update;
				Coder::exclusiveNotice(dstID, dst->getExclusiveEffect(), update);
				Coder::exclusiveNotice(id, this->getExclusiveEffect(), update);
				Coder::tokenNotice(id,0,token[0], update);
				Coder::tokenNotice(id,1,token[1], update);
				engine->sendMessage(-1, MSG_GAME, update);

				connectID=dstID;
				used_LING_HUN_LIAN_JIE=true;
			}
		}
		return ret;
	}

	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int LingHun::LingHunLianJieReact(CONTEXT_TIMELINE_6 *con)
{
	if(token[1] < 1 || using_LING_HUN_LIAN_JIE  || con->harm.point < 1){
		return GE_SUCCESS;
	}
	int  ret;
	//满足发动条件，询问客户端是否发动
	CommandRequest cmd_req;
	Coder::askForSkill(id,LING_HUN_LIAN_JIE_REACT, cmd_req);
	Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size()-1));
	cmd->add_args(con->harm.point);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动 
			int howMany = respond->args(0);
			if(howMany > 0){
				if(howMany > token[1] || howMany < 0){
					return GE_INVALID_ARGUMENT;
				}
				//技能宣告
				network::SkillMsg skill;
				Coder::skillNotice(id, connectID, LING_HUN_LIAN_JIE_REACT, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				// 制造伤害
				HARM harm;
				harm.cause = LING_HUN_LIAN_JIE;
				harm.point = howMany;
				harm.srcID = id;
				harm.type = HARM_MAGIC;
				con->harm.point -= howMany;

				setToken(1, token[1] - howMany);
				network::GameInfo update_info;
				Coder::tokenNotice(id,1,token[1], update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
				using_LING_HUN_LIAN_JIE = true;

				int dstID = con->dstID;
				if(dstID == id){   
					engine->setStateTimeline6(connectID, harm);
				}
				else{
					engine->setStateTimeline6(id, harm);   
				}	
				return GE_URGENT;
			}
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}
