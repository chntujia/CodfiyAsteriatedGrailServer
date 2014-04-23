#include "MoGong.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool MoGong::cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case MO_GUAN_CHONG_JI:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id,STATE_TIMELINE_1,MO_GUAN_CHONG_JI, respond);
			return true;
			break;
		case MO_GUAN_CHONG_JI_HIT:
			session->tryNotify(id,STATE_TIMELINE_2_HIT,MO_GUAN_CHONG_JI_HIT, respond);
			return true;
			break;
		case  CHONG_NENG:
			session->tryNotify(id, STATE_BOOT, CHONG_NENG, respond);
			return true;
			break;
		case  MO_YAN:
			session->tryNotify(id, STATE_BOOT, MO_YAN, respond);
			return true;
			break;
		case  CHONG_NENG_MO_YAN:
			session->tryNotify(id, STATE_BOOT, CHONG_NENG_MO_YAN, respond);
			return true;
			break;
		case CHONG_NENG_GAI_PAI:
			session->tryNotify(id, STATE_BOOT,CHONG_NENG_GAI_PAI, respond);
			return true;
			break; 
		}
	}
	//没匹配则返回false
	return false;
}

int MoGong::v_magic_skill(Action *action)
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
	case LEI_GUANG_SAN_SHE:
		// 本回合未使用【魔贯冲击】，或【魔贯冲击】不可用     +存在【风系】充能 才发动！！！
		{
			CardEntity* card;
			list<int>::iterator it;
			for (it = coverCards.begin(); it != coverCards.end(); ++it)
			{
				card = getCardByID(*it);
				if(card->getElement() !=ELEMENT_THUNDER || GE_SUCCESS != checkOneCoverCard(cardID))
					return GE_INVALID_ACTION;
			}
				  //使用了【魔贯冲击】  【雷光散射】不可用
			if(!avilable_LEI_GUANG_SAN_SHE){
				return GE_INVALID_ACTION;
			}
		}
		break;
	
	//通过角色相关的检
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

//必须调用v_attack，否则就没有了潜行、挑衅之类的检测
int MoGong::v_attack_skill(Action *action)
{
	int actionID = action->action_id();
	int cardID=action->card_ids(0);
	int playerID = action->src_id();
	int cardNum = action->card_ids_size();
	int ret;
	CardEntity* card;
	PlayerEntity* dst;

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}

	if(cardNum != 1){
		return GE_INVALID_CARDID;
	}

	if(GE_SUCCESS != (ret =checkOneCoverCard(cardID))){
		return ret;
	}
	//?????
	if(GE_SUCCESS != (ret = v_attack(39, action->dst_ids(0), false))){
		return ret;
	}

	switch(actionID)
	{
	case DUO_CHONG_SHE_JI:
		// 本回合未使用【魔贯冲击】，或【魔贯冲击】不可用     +存在【风系】充能 才发动！！！
	{	 
		if(used_DUO_CHONG_SHE_JI) 
			return GE_SUCCESS;
		
	}
		break;
	//通过角色相关的检
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;

}


int MoGong::p_magic_skill(int &step, Action* action)
{
	//p_magic_skill不同于别的触发点，有且只有一个匹配，因此每一个技能完成时，务必把step设为STEP_DONE
	int ret;
	switch(action->action_id())
	{
	case LEI_GUANG_SAN_SHE:
		ret = LeiGuangSanShe(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int MoGong::p_attack_skill(int &step, Action *action) 
{ 
   int ret;
	switch(action->action_id())
	{
	case DUO_CHONG_SHE_JI:
		ret = DuoChongSheJi_QiPai(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;  //??

}

//统一在p_before_turn_begin 初始化各种回合变量
int MoGong::p_before_turn_begin(int &step, int currentPlayerID) 
{
	used_MO_GUAN_CHONG_JI=false;
	used_DUO_CHONG_SHE_JI=false;
	using_DUO_CHONG_SHE_JI=false;
	available_MO_GUAN_CHONG_JI=true;
	avilable_LEI_GUANG_SAN_SHE=true;
	used_CHONG_NENG=false;
	used_MO_YAN=false;
	ChongNengNum=0;
	lastTarget=-1;    //上次攻击目标不指定
	bootCount=0;
	return GE_SUCCESS; 
}


int MoGong::p_boot(int &step, int currentPlayerID)
{
	
	int ret = GE_INVALID_STEP;
	if( id != currentPlayerID){
		return GE_SUCCESS;
	}
	bootCount++;
	if(getEnergy()>0&&bootCount==1)
		if(getEnergy()>0)
    {   
		
	   if(step == STEP_INIT) {
		step=CHONG_NENG_MO_YAN;
	}	
	   if(step==CHONG_NENG_MO_YAN)
	   {
	      ret = ChongNengMoYan(currentPlayerID);
	    if(toNextStep(ret) || ret == GE_SUCCESS){
		   if(used_CHONG_NENG)  step=CHONG_NENG;
		   else if(used_MO_YAN) step=MO_YAN; 
			 else
				 step = STEP_DONE;
	     }  
	   }
	   if(step==CHONG_NENG)
	   {
	        ret=ChongNeng();
		   if(toNextStep(ret) || ret == GE_SUCCESS){
		//	if(used_CHONG_NENG)  step=CHONG_NENG_GAI_PAI;
		//	else  
			   step= STEP_DONE;
		   }
	   }

	   if(step==MO_YAN)
	   {
	        ret=MoYan();
			if(toNextStep(ret) || ret == GE_SUCCESS){
			    step = STEP_DONE;
			}
	   }     
		
    }

	if(bootCount==2&&used_CHONG_NENG)
	{
	  if(step == STEP_INIT) {
		step=CHONG_NENG_GAI_PAI;
	}	
	  if(step==CHONG_NENG_GAI_PAI)
	  {
	       ret=ChongNengGaiPai();
		   if(toNextStep(ret) || ret == GE_SUCCESS){
			  step = STEP_DONE;
		   }
	  }
	}
if(bootCount==2&&used_MO_YAN)
{
	HARM moyan;
	moyan.cause =MO_YAN;
	moyan.point = 1;
	moyan.srcID = id;
	moyan.type = HARM_NONE;
	engine->pushGameState(new StateRequestHand(id,moyan, id, DECK_COVER, false, true));
}

return GE_SUCCESS;
   

}

int MoGong::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
    int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	int ret = GE_INVALID_STEP;
	if(srcID != id || !con->attack.isActive ){
		return GE_SUCCESS;
	}

	    if(using_DUO_CHONG_SHE_JI)
		{
		   ret =DuoChongSheJi_Effect(con);
		   if(toNextStep(ret)|| ret == GE_URGENT){
			step = STEP_DONE;
		    }

		}

		 int fireCount=0;
		 list<int>::iterator it;
	  for (it = coverCards.begin(); it != coverCards.end(); ++it)
		{
			if(getCardByID((*it))->getElement() == ELEMENT_FIRE)
                          fireCount++;
		}

	PlayerEntity* dst = engine->getPlayerEntity(dstID);
	//手牌达到上限
	if(dst->getHandCardNum() ==dst->getHandCardMax()){
		available_MO_GUAN_CHONG_JI=false;     //不能攻击手牌达到上限的角色
	}

	if(!used_DUO_CHONG_SHE_JI&&available_MO_GUAN_CHONG_JI&&fireCount>0)
	{  
		if(step == STEP_INIT) {
		step =MO_GUAN_CHONG_JI;
	}	
    
	if(step==MO_GUAN_CHONG_JI)
	  {
		   ret=MoGuanChongJi(con);

		   if(toNextStep(ret)){
			  step =STEP_DONE;
		   }
	 }
	}	
		  
	  return GE_SUCCESS;
}

int MoGong::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con)
{
	int ret;
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}

	 int fireCount=0;
		 list<int>::iterator it;
	  for (it = coverCards.begin(); it != coverCards.end(); ++it)
		{
			if(getCardByID((*it))->getElement() == ELEMENT_FIRE)
                          fireCount++;
		}
	if(used_MO_GUAN_CHONG_JI&&fireCount>0)
	{
     if(step == STEP_INIT) {
		step =MO_GUAN_CHONG_JI_HIT;
	}	
	 if(step ==MO_GUAN_CHONG_JI_HIT)
	 {
	  ret = MoGuanChongJi_Hit(con);
	if(toNextStep(ret) || GE_URGENT == ret){
		step = STEP_DONE;
	}
	 }
	   return ret;
	}

	return GE_SUCCESS;
}

int MoGong::p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con) { 

	if(con->srcID != id){
		return GE_SUCCESS;
	}

	if(used_MO_GUAN_CHONG_JI)
	{
	            HARM  harm;
	            harm.cause = MO_GUAN_CHONG_JI;
	            harm.point = 3;
	            harm.srcID = id;
	            harm.type = HARM_MAGIC;

	            engine->setStateTimeline3(con->dstID,harm );
	}
	return GE_SUCCESS;
}

//【多重射击】
int MoGong::v_attack(int cardID, int dstID, bool realCard)
{
	int ret;
	if(GE_SUCCESS != (ret = PlayerEntity::v_attack(cardID,dstID,realCard))){
		return ret;
	}
	
	if(using_DUO_CHONG_SHE_JI)
	{
	    if(lastTarget==dstID)
			return GE_INVALID_PLAYERID;
    }
	lastTarget=dstID;
	return GE_SUCCESS;
}
                
int MoGong::p_after_attack(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	//不是剑圣就不用跑了
	if(playerID != id){
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	if(step == STEP_INIT){
			step = DUO_CHONG_SHE_JI;
		}			
	if(step == DUO_CHONG_SHE_JI){
		ret = DuoChongSheJi(playerID);
		if(toNextStep(ret)){
			//全部走完后，请把step设成STEP_DONE
			   step = STEP_DONE;
		    }
	    }
	return ret;
}

int MoGong::v_additional_action(int chosen)
{
	switch(chosen)
	{
	case DUO_CHONG_SHE_JI:
		// 本回合未使用【魔贯冲击】，或【魔贯冲击】不可用     +有盖牌 才发动！！！

		if((used_MO_GUAN_CHONG_JI &&available_MO_GUAN_CHONG_JI)||this->getCoverCardNum() == 0){
			return GE_INVALID_ACTION;
		}
		break;
	}
	//通过角色相关的检测，基本检测交给底层
	return PlayerEntity::v_additional_action(chosen);
}

int MoGong::p_additional_action(int chosen)
{
	GameInfo update_info;
	switch(chosen)
	{
	//【多重射击】
	case DUO_CHONG_SHE_JI:
			used_DUO_CHONG_SHE_JI=true;
		    available_MO_GUAN_CHONG_JI=false;
		break;
	} 

	//做完角色相关的操作，扣除额外行动交给底层
	return PlayerEntity::p_additional_action(chosen);
}

int MoGong::ChongNengMoYan(int PlayerID)
{
	int ret;
	int skillID;
    int howMany;
	vector<int> cards;

	skillID =CHONG_NENG_MO_YAN;
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
			if(respond->args(0)==1)         //充能
		  {	
	         used_CHONG_NENG=true;  
			   
            }

	        if(respond->args(0)==2)         //魔眼
          {
			  used_MO_YAN=true;
           } //if(respond->args(0)==2)
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

int MoGong::ChongNeng()
{
	int ret;
    int howMany;
	vector<int> cards;
    CommandRequest cmd_req1;
	Coder::askForSkill(id, CHONG_NENG, cmd_req1);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req1))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0)==1)         //启动
			{
			    network::SkillMsg skill;
				Coder::skillNotice(id, id, CHONG_NENG, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				//本回合不能发动 【魔贯冲击】 及 【雷光散射】
				available_MO_GUAN_CHONG_JI=false;
	            avilable_LEI_GUANG_SAN_SHE=false;
				used_CHONG_NENG=true;   //标记当前启动技为【充能】
				ChongNengNum=respond->args(2);  //最多允许充能数
				if(crystal>0)
				  setCrystal(--crystal);
				else
                  setGem(--gem);
				GameInfo game_info;
				Coder::energyNotice(id, gem, crystal,game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);

				engine->pushGameState(new StateBoot());

				HARM  harm;
	            harm.srcID = id;
	            harm.type = HARM_NONE;
	            howMany=harm.point =respond->args(2);  //摸牌数量
	            harm.cause = CHONG_NENG;
	            engine->setStateMoveCardsToHand(-1, DECK_PILE, id, DECK_HAND, howMany, cards, harm, false);

             //弃到四张手牌
			  vector<int> cardIDs;
			//  int cardNum = ((getHandCardNum()-4)>0)? (getHandCardNum()-4):0;
			  int cardNum=respond->args(1);
			    for(int i = 0; i < cardNum; i ++)
				{
					cardIDs.push_back(respond->card_ids(i));
				}

				if(cardNum > 0)
				{
					engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs, id, CHONG_NENG, false);
				}

			}

			if(respond->args(0)==0)  //不启动
			{
			   used_CHONG_NENG=false;
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

int MoGong::MoYan()
{
	int ret;
    int howMany;
	vector<int> cards;
    CommandRequest cmd_req2;
	Coder::askForSkill(id, MO_YAN, cmd_req2);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req2))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0)==1)         //启动
			{
			    network::SkillMsg skill;
				Coder::skillNotice(id, id, MO_YAN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				//更新【能量】
				setGem(--gem);	
				setCrystal(++crystal);

				GameInfo update_info;					
				Coder::energyNotice(id, gem, crystal, update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);

				used_MO_YAN=true;   //标记当前启动技为【魔眼】
				engine->pushGameState(new StateBoot());

			   if(respond->args(1)==0)    //没有选择目标角色
			  {
				   //摸3张牌【强制】
	            HARM harm1;
	            harm1.srcID =id;
	            harm1.type =HARM_NONE;
	            harm1.point =3;
	            harm1.cause =MO_YAN;
	            engine->setStateMoveCardsToHand(-1, DECK_PILE, id, DECK_HAND, 3, cards, harm1, false);
				ChongNengNum=1;   //将自己一张手牌作为充能
			  }
				else                       //目标角色弃一张牌      
				{
				   int dstID=respond->dst_ids(0);
				   PlayerEntity *dstPlayer = engine->getPlayerEntity(dstID);
				    //没有手牌不用弃
				   if(dstPlayer->getHandCardNum() > 0){
					      HARM qipai;
	                      qipai.cause =MO_YAN;
	                      qipai.point = 1;
	                      qipai.srcID = id;
	                      qipai.type = HARM_NONE;
			              engine->pushGameState(new StateRequestHand(dstID, qipai, -1, DECK_DISCARD, false, false));
				
		              } 
			   }

			}//if 发动

			if(respond->args(0)==0)  //不启动
			{
			   used_MO_YAN=false;
			}
			return   GE_SUCCESS ;
		}  //getReply
	   return ret;
	}

 else{
	//超时啥都不用做
	  return GE_TIMEOUT;
     }

}


int MoGong::ChongNengGaiPai()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id,CHONG_NENG_GAI_PAI, cmd_req);

	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动  ---摸牌
			if (respond->args(0) == 1)
			{
				network::SkillMsg skill;
				Coder::skillNotice(id, id,CHONG_NENG_GAI_PAI, skill);  //gaidong
				engine->sendMessage(-1, MSG_SKILL, skill);  
				
				vector<int> cards;
				int cardNum;
				cardNum=respond->card_ids_size();
				 int card_id;
              for (int i = 0; i <cardNum; ++i)
                {
		           card_id = respond->card_ids(i);
	              if (checkOneHandCard(card_id) == GE_SUCCESS)
		             cards.push_back(card_id);
	             }

			  
			   engine->setStateMoveCardsNotToHand(id,DECK_HAND,id, DECK_COVER, cards.size(), cards, id,CHONG_NENG_GAI_PAI,true);	
			   
			}
		}
		return ret;
	}

	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
     return GE_SUCCESS;

}

 //【魔贯冲击】
int MoGong::MoGuanChongJi(CONTEXT_TIMELINE_1 *con)
{
     CommandRequest cmd_req;
	Coder::askForSkill(id, MO_GUAN_CHONG_JI, cmd_req);
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
			  /*  HARM  harm;
	            harm.cause = MO_GUAN_CHONG_JI;
	            harm.point = 1;
	            harm.srcID = id;
	            harm.type = HARM_NONE;
				engine->pushGameState(new StateRequestCover(id, harm, -1, DECK_DISCARD, false, false));*/

				int cardID;
				cardID=respond->card_ids(0);
				//移除盖牌
				engine->setStateMoveOneCardNotToHand(id,DECK_COVER, -1, DECK_DISCARD, cardID, id, MO_GUAN_CHONG_JI, true);
				
			//	engine->setStateMoveCardsNotToHand(id,DECK_COVER, -1, DECK_DISCARD, cards.size(), cards, id,LEI_GUANG_SAN_SHE,true);
				con->harm.point=con->harm.point+1;  //伤害加1
				used_MO_GUAN_CHONG_JI=true;         //使用【魔贯冲击标记】 
		   }
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}

}

int MoGong::MoGuanChongJi_Hit(CONTEXT_TIMELINE_2_HIT *con)
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, MO_GUAN_CHONG_JI_HIT, cmd_req);
 
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
				con->harm.point=con->harm.point+1;  //伤害加1

				int cardID;
				cardID=respond->card_ids(0);
				//移除盖牌
				engine->setStateMoveOneCardNotToHand(id,DECK_COVER, -1, DECK_DISCARD, cardID, id, MO_GUAN_CHONG_JI_HIT, true);
		   }
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}


}

//[多重射击]

int MoGong::DuoChongSheJi_QiPai(Action *action)
{
	
	int virtualCardID =39; //暗灭
	int dstID=action->dst_ids(0);    
	int cardID =action->card_ids(0);

	//移除盖牌
	CardMsg show_card;
	Coder::showCardNotice(id,1,cardID, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	engine->setStateMoveOneCardNotToHand(id, DECK_COVER, -1, DECK_DISCARD,cardID, id,DUO_CHONG_SHE_JI, false);

	//暗系攻击
	engine->setStateTimeline1(virtualCardID, dstID, id, true);
	engine->setStateUseCard(virtualCardID, dstID, id, false, false);
	using_DUO_CHONG_SHE_JI=true;
	return GE_URGENT;
}

int MoGong::DuoChongSheJi_Effect(CONTEXT_TIMELINE_1 *con)
{   

 //    engine->setStateMoveCardsNotToHand(id,DECK_COVER, -1, DECK_DISCARD, cards.size(), cards, id,LEI_GUANG_SAN_SHE,true);  //
	 con->hitRate = RATE_NOREATTACK;       //暗系的主动攻击
	 con->harm.point = con->harm.point-1;  //伤害减1
	 using_DUO_CHONG_SHE_JI=false;
	 return GE_SUCCESS;

}
int MoGong::DuoChongSheJi(int playerID)
{
	//是不是魔弓    || 本回合使用【魔贯冲击】   ||已使【多重射击】
	if(playerID != id ||used_MO_GUAN_CHONG_JI || this->getCoverCardNum() == 0){
		return GE_SUCCESS;
	}
	addAction(ACTION_ATTACK, DUO_CHONG_SHE_JI);
	return GE_SUCCESS;
}

//【雷光闪射】
int MoGong::LeiGuangSanShe(Action *action)
{
	list<int> dstIDs;
	vector<int> cards;
	int cardID =action->card_ids(0);
	int cardNum=action->card_ids_size();
	PlayerEntity * dstPlayer = engine->getPlayerEntity(id);
	int color = dstPlayer->getColor();
	dstPlayer = dstPlayer->getPost();
	while(dstPlayer->getID() != id)
	{
		if(color != dstPlayer->getColor())
			dstIDs.push_back(dstPlayer->getID());
		dstPlayer = dstPlayer->getPost();
	}
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstIDs, LEI_GUANG_SAN_SHE, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	HARM harm;
	harm.type = HARM_MAGIC;
	harm.point = 1;
	harm.srcID = id;
	harm.cause = LEI_GUANG_SAN_SHE;

	list<int>::iterator it;
	dstIDs.reverse();

	if(cardNum==1)
	{
	   for (it = dstIDs.begin(); it != dstIDs.end(); it++)
	  {
		engine->setStateTimeline3(*it, harm);
	  }
	}
	else     //雷系牌多于一张
	{
	   int dstId=action->dst_ids(0);
	    for (it = dstIDs.begin(); it != dstIDs.end(); it++)
	  { 
		  if(*it!=dstId)
		engine->setStateTimeline3(*it, harm);
	  }
		harm.point=cardNum;
		engine->setStateTimeline3(dstId, harm);
	}
	 int card_id;
   for (int i = 0; i <cardNum; ++i)
   {
		card_id = action->card_ids(i);
	 if (getCardByID(card_id)->getElement() == ELEMENT_THUNDER && checkOneCoverCard(card_id) == GE_SUCCESS)
		 cards.push_back(card_id);
	}
    
    engine->setStateMoveCardsNotToHand(id,DECK_COVER, -1, DECK_DISCARD, cards.size(), cards, id,LEI_GUANG_SAN_SHE,true);

	return GE_URGENT;
}