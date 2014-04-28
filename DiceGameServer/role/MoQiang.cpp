#include "MoQiang.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"


bool MoQiang::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case AN_ZHI_JIE_FANG:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id,STATE_BOOT,AN_ZHI_JIE_FANG, respond);
			return true;

        case HUAN_YING_XING_CHEN:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id,STATE_BOOT,HUAN_YING_XING_CHEN, respond);
			return true;

		case  AN_ZHI_ZHANG_BI:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id,STATE_TIMELINE_3,AN_ZHI_ZHANG_BI, respond);
			return true;

		case QI_HEI_ZHI_QIANG:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id,STATE_TIMELINE_2_HIT,QI_HEI_ZHI_QIANG, respond);  //用什么状态？？？
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

//统一在p_before_turn_begin 初始化各种回合变量
int  MoQiang::p_before_turn_begin(int &step, int currentPlayerID) 
{
	using_AnZhiJieFang=false;
	using_HuanYingXingCeng=false;
	availabel_QiHeiZhiQiang=true;
	availabel_ChongYing=true;
	used_ChongYing=false;
	hurtID=-1;
	return GE_SUCCESS; 
}

int MoQiang::p_boot(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	if( id != currentPlayerID){
		return GE_SUCCESS;
	}

	  while(STEP_DONE != step)
	{
		switch(step)
		{
		case STEP_INIT:
			if(tap==false)
	           step=AN_ZHI_JIE_FANG;
	              
	      else
		      step=HUAN_YING_XING_CHEN;
	          break;	
		case AN_ZHI_JIE_FANG:
		case HUAN_YING_XING_CHEN:
	        ret = AnZhiHuanYing();
	      if(toNextStep(ret) || ret == GE_SUCCESS){
			       step = STEP_DONE;
	       }
			break;	
		default:
			return GE_INVALID_STEP;
		}
	}
	
	return ret;
}


int MoQiang::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con)
{

	int ret = GE_INVALID_STEP;
	if(con->attack.srcID != id||!tap){
		return GE_SUCCESS;
	}

	while(STEP_DONE!= step)
	{
	  switch(step)
		{
		case STEP_INIT:
			//初始化step
			   step = AN_ZHI_JIE_FANG;
			break;

		case AN_ZHI_JIE_FANG:
			if(using_AnZhiJieFang)
			{
			  ret =AnZhiJieFang_Effect(con);
			  if(toNextStep(ret)){
				   step=QI_HEI_ZHI_QIANG;
			   }			
			}
			else
				step=QI_HEI_ZHI_QIANG;
			break;

        case QI_HEI_ZHI_QIANG:
			//初始化step
		{	
			int dstID=con->attack.dstID;
			PlayerEntity* dst=engine->getPlayerEntity(dstID);

			//【能量】         手牌为1&2
 			if(getEnergy()>0&&(dst->getHandCardNum()==1||dst->getHandCardNum()==2))
			{
			   ret =QiHeiZhiQiang(con);
			  if(toNextStep(ret)){
				   step=STEP_DONE;
			   }	
			}
			else
				step=STEP_DONE;
		}
			break;
    
		default:
			return GE_INVALID_STEP;
		}
	}
	     return ret;
}

//【暗之障壁】
int MoQiang::p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con)
{
	if (con->dstID == id)
	{
		// 【暗之障壁】
		//如果有雷系或法术牌，则发动 
		int validCard=0;
		  list<int>::iterator it;
		for (it = handCards.begin(); it != handCards.end(); ++it)
		{
			if(getCardByID((*it))->getElement()==ELEMENT_THUNDER||getCardByID((*it))->getType()==TYPE_MAGIC)
                          validCard++;
		}

		if(validCard>0)
		{
		step =AN_ZHI_ZHANG_BI;
		int ret =AnZhiZhangBi(con);
		if(toNextStep(ret) || ret == GE_URGENT){
			//全部走完后，请把step设成STEP_DONE
			step = STEP_DONE;
		   }
		  return ret;
	    }
	}

	if(con->harm.srcID == id)
	{
	   if(used_ChongYing)
		   con->harm.point+= cardCount;
	}
	return GE_SUCCESS;
}


//幻影星辰
int MoQiang::p_lose_morale(int &step, CONTEXT_LOSE_MORALE *con)
{
	int ret = GE_SUCCESS;
	if(using_HuanYingXingCeng)
	{
		ret=HuanYingXingChen_Effect(con);
	}
	return ret;
}


int MoQiang::v_magic_skill(Action *action)
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
	case CHONG_YING:
		
		cardID = action->card_ids(0);
		 //手牌不存在                                 //不能使用【充盈】
		if(GE_SUCCESS != checkOneHandCard(cardID)||!availabel_ChongYing)
		return GE_INVALID_ACTION;
		break;
	//通过角色相关的检
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int MoQiang::p_magic_skill(int &step, Action* action)
{
	//p_magic_skill不同于别的触发点，有且只有一个匹配，因此每一个技能完成时，务必把step设为STEP_DONE
	int ret;
	int actionID = action->action_id();

	switch(actionID)
	{
	case CHONG_YING:
		ret =ChongYing(action);
		step = STEP_DONE;
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}


//FIXME: 如果以后有人能转换法牌，会无法通过这里
int MoQiang::v_request_hand(int cardSrc, int howMany, vector<int> cards, HARM harm)
{
	if(harm.cause ==CHONG_YING){
		int cardID = cards[0];
		PlayerEntity* dst =engine->getPlayerEntity(cardSrc);
		CardEntity* card = getCardByID(cardID);
		
		if(card->getType() == TYPE_MAGIC||dst->getCardElement(cardID) ==ELEMENT_THUNDER){
			cardCount++;
		}
	}
	return GE_SUCCESS;
}


//【黑暗束缚】 法术牌 不能使用

int MoQiang::v_missile(int cardID, int dstID, bool realCard)
{
	if(realCard){
		int ret;
		if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
			return ret;
		}
	}
	return GE_INVALID_CARDID;
}

int MoQiang::v_shield(int cardID, PlayerEntity* dst)
{
	int ret;
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
		return ret;
	}
	return GE_INVALID_CARDID;
}

int MoQiang::v_weaken(int cardID, PlayerEntity* dst)
{
	int ret;
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
		return ret;
	}
	return GE_INVALID_CARDID;
}

int MoQiang::v_remissile(int cardID, bool realCard)
{
	if(realCard){
		int ret;
		if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
			return ret;
		}
	}
	return GE_INVALID_CARDID;
}

int MoQiang::v_block(int cardID)
{
	int ret;
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
		return ret;
	}
	return GE_INVALID_CARDID;
}

int MoQiang::AnZhiHuanYing()
{
    int ret;
    int skillID;

	if(tap==false){
		skillID =AN_ZHI_JIE_FANG;
	}
	else
	   {
		skillID =HUAN_YING_XING_CHEN;
	}

    CommandRequest cmd_req;
	Coder::askForSkill(id, skillID, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0)==1)     
			{	
				network::SkillMsg skill;
				Coder::skillNotice(id, id, AN_ZHI_JIE_FANG, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				using_AnZhiJieFang=true;
	            availabel_QiHeiZhiQiang=false;
	            availabel_ChongYing=false;

				//进入【幻影形态】
				tap = true;
				GameInfo game_info;
				Coder::tapNotice(id, tap, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
	            //手牌上限设定为5
				 engine->setStateChangeMaxHand(id, true, true, 5);
				return GE_SUCCESS;
			}

			if(respond->args(0)==2)
			{
			    network::SkillMsg skill;
				Coder::skillNotice(id, id,HUAN_YING_XING_CHEN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				
			   hurtID=respond->dst_ids(0);
			//对自己造成2点法术伤害
			   HARM harm;
			   harm.type = HARM_MAGIC;
			   harm.point =2;
			   harm.srcID =id;
			   harm.cause =HUAN_YING_XING_CHEN;
			   engine->setStateTimeline3(id, harm);
			   using_HuanYingXingCeng=true;
				//【幻影形态】
				tap = false;
				GameInfo game_info;
				Coder::tapNotice(id, tap, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);

				 //手牌上限设定为6
				 engine->setStateChangeMaxHand(id, true, true,6);
				 return GE_SUCCESS;
			}
			//没发动技能
			return GE_SUCCESS;
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}


//【充盈】
int MoQiang::ChongYing(Action* action)
{

   //检查客户端返回的是否为魔法牌&&雷系牌
	  int cardID = action->card_ids(0);
	  CardEntity* card = getCardByID(cardID);
		if( card->getType() != TYPE_MAGIC&&card->getElement()!=ELEMENT_THUNDER ){
			return GE_SUCCESS;
		}
		cardCount=0;
		used_ChongYing=true;
	   //丢弃魔法牌&&雷系牌
		CardMsg show_card;
		Coder::showCardNotice(id, 1, cardID, show_card);
		engine->sendMessage(-1, MSG_CARD, show_card);

		//所有移牌操作都要用setStateMoveXXXX，ToHand的话要填好HARM，就算不是伤害
	PlayerEntity* it = this->getPre();
	HARM chongying;
	chongying.cause =CHONG_YING;
	chongying.point = 1;
	chongying.srcID = id;
	chongying.type = HARM_NONE;

	HARM chongying_discard;
	chongying_discard.cause =CHONG_YING_DISCARD;
	chongying_discard.point = 1;
	chongying_discard.srcID = id;
	chongying_discard.type = HARM_NONE;
	
	//先进后出，所以逆出牌顺序压，最后才是魔导自己明弃法牌
	do{
		//bool isShown = false, bool canGiveUp = false
		if(it->getColor()!=color)
		engine->pushGameState(new StateRequestHand(it->getID(),chongying, -1, DECK_DISCARD, true, false)); //不能弃牌
		else
         engine->pushGameState(new StateRequestHand(it->getID(),chongying_discard, -1, DECK_DISCARD, true, true));

		it = it->getPre();
	}while(it != this);
	
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, CHONG_YING, true);

	addAction(ACTION_ATTACK,CHONG_YING);
		return GE_URGENT;


}
//【暗之解放】攻击效果
int MoQiang::AnZhiJieFang_Effect(CONTEXT_TIMELINE_2_HIT *con)
{
   //在【暗影形态】下 发动的所有攻击伤害+1

	SkillMsg skill;
	Coder::skillNotice(id, con->attack.dstID, AN_ZHI_JIE_FANG, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	con->harm.point+= 2;
	return GE_SUCCESS;
}

int MoQiang::HuanYingXingChen_Effect(CONTEXT_LOSE_MORALE *con)
{
    HARM harm;
	harm.type = HARM_MAGIC;
    harm.point =2;
    harm.srcID =id;
    harm.cause =HUAN_YING_XING_CHEN;
    engine->setStateTimeline3(hurtID,harm);

	using_HuanYingXingCeng=false;
	 return GE_SUCCESS;
}

int MoQiang::AnZhiZhangBi(CONTEXT_TIMELINE_3 *con)
{   
	int ret;
    CommandRequest cmd_req;
	Coder::askForSkill(id,AN_ZHI_ZHANG_BI, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0)==1)     
			{	
				network::SkillMsg skill;
				Coder::skillNotice(id, id,AN_ZHI_ZHANG_BI, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				//弃牌
				vector<int> cardIDs;
				int cardNum= respond->card_ids_size();
				int card_id;

			for(int i = 0; i < cardNum; i ++)
				{   
					card_id=respond->card_ids(i);
     if((getCardByID(card_id)->getElement() == ELEMENT_THUNDER||getCardByID(card_id)->getType()==TYPE_MAGIC )&& checkOneHandCard(card_id) == GE_SUCCESS)
					cardIDs.push_back(card_id);
				}
			if(cardNum>0)
			{
		        ret=engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs, id,AN_ZHI_ZHANG_BI, false);
			}
			else
			{
				return GE_SUCCESS;
			}
				
			}
			//没发动技能
			return GE_SUCCESS;
		}
		return ret;
	}

	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

//【漆黑之枪】
int MoQiang::QiHeiZhiQiang(CONTEXT_TIMELINE_2_HIT *con)
{
   int ret;
    CommandRequest cmd_req;
	Coder::askForSkill(id,QI_HEI_ZHI_QIANG, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0)==1)     
			{	
				network::SkillMsg skill;
				Coder::skillNotice(id, id,QI_HEI_ZHI_QIANG, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

			   for(int i=1;i<=respond->args(1);i++)
			   {
			      if(crystal>0) setCrystal(--crystal);
				  else          setGem(--gem);
			   }

			   //更新能量信息
			  network::GameInfo update;
		      Coder::energyNotice(id, gem, crystal, update);
		      engine->sendMessage(-1, MSG_GAME, update);
		      con->harm.point+=(respond->args(1)+2);

			}
			//没发动技能
			return GE_SUCCESS;
		}
		return ret;
	}

	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}