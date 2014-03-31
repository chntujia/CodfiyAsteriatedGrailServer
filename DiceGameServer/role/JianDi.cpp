#include "JianDi.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool JianDi::cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		  case JIAN_HUN_SHOU_HU :
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_2_MISS,JIAN_HUN_SHOU_HU, respond);
			return true;
			break;

		  case BU_QU_YI_ZHI :
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id,STATE_AFTER_ATTACK,BU_QU_YI_ZHI, respond);
			return true;
			break;
		 
		  case JIAN_QI_ZHAN :
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id,STATE_TIMELINE_2_HIT,JIAN_QI_ZHAN, respond);
			return true;
			break;

		 case TIAN_SHI_ZHI_HUN:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id,STATE_TIMELINE_1,TIAN_SHI_ZHI_HUN, respond);
			return true;
			break;
		 
		  case E_MO_ZHI_HUN:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id,STATE_TIMELINE_1,E_MO_ZHI_HUN, respond);
			return true;
			break;
		}
	}
	//没匹配则返回false
	return false;
}

//统一在p_before_turn_begin 初始化各种回合变量
int JianDi::p_before_turn_begin(int &step, int currentPlayerID) 
{
	used_JIAN_HUN_SHOU_HU=false;
    used_TIAN_SHI_ZHI_HUN=false;
    used_E_MO_ZHI_HUN=false;
	used_BU_QU_YI_ZHI=false;
	TianShiYuEMo();
	
	return GE_SUCCESS; 
}

//【天使之魂】 与 【恶魔之魂】
int JianDi::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
    int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	int ret = GE_INVALID_STEP;
	if(srcID != id || !con->attack.isActive ){
		return GE_SUCCESS;
	}

	if(step == STEP_INIT) {
    //发动天使之魂
	if(flag==1)
	 {
	   step =TIAN_SHI_ZHI_HUN;
	 }

	else if(flag==2)
	  {
	    step =E_MO_ZHI_HUN;
	  }
	else 
	{
	   step =STEP_DONE;
	}
  }
  
  if(step==TIAN_SHI_ZHI_HUN)
  {
      ret=TianShiZhiHun();
	   if(toNextStep(ret)|| ret == GE_URGENT){
			step = STEP_DONE;
	   }
  }
  
  if(step==E_MO_ZHI_HUN)
  {
      ret=EMoZhiHun();
	   if(toNextStep(ret)|| ret == GE_URGENT){
			step = STEP_DONE;
    }
  }
	  return GE_SUCCESS;
}

//【剑气斩】 【恶魔之魂】
int JianDi::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con)
{
	int ret;
	if(con->attack.srcID != id || !con->attack.isActive){
		return GE_SUCCESS;
	}
	//【剑气】不为0
	if(token[0]>0)
	{
	  if(step == STEP_INIT){
		    step=JIAN_QI_ZHAN;
		}			
	if(step ==JIAN_QI_ZHAN){
		ret = JianQiZhan(con);
		if(toNextStep(ret)){
			//全部走完后，请把step设成STEP_DONE
			   step = STEP_DONE;
		    }
	    }
	}

	if(used_E_MO_ZHI_HUN)
	{
	  con->harm.point+=1;
	  used_E_MO_ZHI_HUN=false;
	}

	if(used_TIAN_SHI_ZHI_HUN)
	{
	    TianShiZhiHun_Effect();
		used_TIAN_SHI_ZHI_HUN=false;
	}
	return GE_SUCCESS;
}

//【佯攻】  【恶魔之魂】
int JianDi::p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con) { 

	//保证是本角色，应战攻击丢失也会调用
	if(con->srcID != id || !con->isActive){
		return GE_SUCCESS;
	}
	int ret;
	//【佯攻】
	YangGon();

	//【剑魂守护】
	if(coverCards.size()<3&&!used_TIAN_SHI_ZHI_HUN&&!used_E_MO_ZHI_HUN)
	{
	  if(step == STEP_INIT){
		    step=JIAN_HUN_SHOU_HU;
		}			
	if(step ==JIAN_HUN_SHOU_HU){
		ret = JianHunShouHu(con);
		if(toNextStep(ret)){
			//全部走完后，请把step设成STEP_DONE
			   step = STEP_DONE;
		    }
	    }
	}

	if(used_E_MO_ZHI_HUN)
	{
	  EMoZhiHun_Effect();
	  used_E_MO_ZHI_HUN=false;
	}
	
	return GE_SUCCESS;
}

int JianDi::p_after_attack(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	//
	if(playerID != id){
		return GE_SUCCESS;
	}

	 TianShiYuEMo();
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	if(step == STEP_INIT){
		if(getEnergy()>0&&!used_BU_QU_YI_ZHI)
		  {
			step =BU_QU_YI_ZHI;
		  }
		else
			step=STEP_DONE;
		}			
	if(step ==BU_QU_YI_ZHI){
		ret = BuQuYiZhi();
		if(toNextStep(ret)){
			//全部走完后，请把step设成STEP_DONE
			   step = STEP_DONE;
		    }
	    }
	return ret;
}



//【天使与魔鬼】
int JianDi::TianShiYuEMo()
{
	if(coverCards.size()==0)         flag=0;
	else if(coverCards.size()%2!=0)  flag=1; //单数为天使之魂
	else                             flag=2; //双数为恶魔之魂

	return GE_SUCCESS;
}
//【佯攻】
int JianDi::YangGon()
{
	setToken(0,token[0]+1);
	network::GameInfo update_info;
	Coder::tokenNotice(id,0,token[0], update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	return GE_SUCCESS;
}
//【剑魂守护】
int JianDi::JianHunShouHu(CONTEXT_TIMELINE_2_MISS* con)
{
    int ret;
	int cardID;
	CommandRequest cmd_req;
	Coder::askForSkill(id,JIAN_HUN_SHOU_HU, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0)==1)         //启动
			{
			    network::SkillMsg skill;
				Coder::skillNotice(id, id,JIAN_HUN_SHOU_HU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				cardID=con->cardID;
				vector<int> cards;
				cards.push_back(cardID);
				addCoverCards(1, cards);  

				//更新盖牌
	            GameInfo game_info;
	            Coder::coverNotice(id,getCoverCards(), game_info);
	            engine->sendMessage(-1, MSG_GAME, game_info);
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
//【不屈意志】
int JianDi::BuQuYiZhi()
{   
	int ret;
	vector<int> cards;
	CommandRequest cmd_req;
	Coder::askForSkill(id,BU_QU_YI_ZHI, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0)==1)         //启动
			{
			    network::SkillMsg skill;
				Coder::skillNotice(id, id,BU_QU_YI_ZHI, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				//更新能量
				if(crystal>0)
				  setCrystal(--crystal);
				else
                  setGem(--gem);

				GameInfo game_info;
				Coder::energyNotice(id, gem, crystal,game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);

				//添加一【剑气】
				setToken(0,token[0]+1);
				network::GameInfo update_info;
				Coder::tokenNotice(id,0,token[0], update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
	           
				//摸一张手牌
				HARM  harm;
	            harm.srcID = id;
	            harm.type = HARM_NONE;
	            harm.point =1;  //摸牌数量
	            harm.cause =BU_QU_YI_ZHI;
	            ret=engine->setStateMoveCardsToHand(-1, DECK_PILE, id, DECK_HAND,1,cards, harm, false);

                addAction(ACTION_ATTACK, BU_QU_YI_ZHI);
				used_BU_QU_YI_ZHI=true;
			    return ret;

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
//【剑气斩】
int JianDi::JianQiZhan(CONTEXT_TIMELINE_2_HIT *con)
{   
	int ret;
	int dstID;
	int JianQiCount;
	CommandRequest cmd_req;
	//LingHun *connectPlayer = m_playerEntities[connectID];

	Coder::askForSkill(id,JIAN_QI_ZHAN, cmd_req);
	Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size()-1));
	cmd->add_args(con->attack.dstID);
		if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动 
			if(respond->args(0)==1){
				//技能宣告
                network::SkillMsg skill;
				Coder::skillNotice(id, id,JIAN_QI_ZHAN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
                
				JianQiCount=respond->args(1);
				dstID=respond->dst_ids(0);
				
				//移除剑气
				setToken(0,token[0]-JianQiCount);
				network::GameInfo update_info;
				Coder::tokenNotice(id,0,token[0], update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);

				HARM  harm;
	            harm.srcID = id;
	            harm.type = HARM_MAGIC;
	            harm.point =JianQiCount;  //摸牌数量
	            harm.cause =JIAN_QI_ZHAN;

				engine->setStateTimeline3(dstID,harm);
				 

	        }
			else
			{
				return GE_SUCCESS;
			}
		}
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}

}
//【天使之魂】
int JianDi::TianShiZhiHun()
{    
	int ret;
	int card_id;
	 CommandRequest cmd_req;
	 Coder::askForSkill(id, TIAN_SHI_ZHI_HUN, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0)==1)         //启动
			{
				//宣告技能
			    network::SkillMsg skill;
				Coder::skillNotice(id, id,TIAN_SHI_ZHI_HUN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				
			   used_TIAN_SHI_ZHI_HUN=true;

           //移除一个【天使之魂】
		      card_id = respond->card_ids(0);
			  engine->setStateMoveOneCardNotToHand(id,DECK_COVER,-1,DECK_DISCARD,card_id,id,TIAN_SHI_ZHI_HUN,false);
			//  TianShiYuEMo();
			}
		
			return GE_SUCCESS ;
		}
		  return ret;
	}

else{
	//超时啥都不用做
	  return GE_TIMEOUT;
}

}

int JianDi::TianShiZhiHun_Effect()
{    

	//+2【治疗】及+1【士气】
	    int color;
		
	    addCrossNum(2);
		GameInfo update_info;
		Coder::crossNotice(id, getCrossNum(), update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);

     //+1【士气】
		color=getColor();
		TeamArea *m_teamArea = engine->getTeamArea();
	    int morale = m_teamArea->getMorale(color);
		if(morale<15)
	    m_teamArea->setMorale(color, morale+1);
		morale = m_teamArea->getMorale(color);

	   GameInfo game_info;	
	if (color == RED)
		game_info.set_red_morale(morale);  //星杯+士气
	else
		game_info.set_blue_morale(morale);
	engine->sendMessage(-1, MSG_GAME, game_info);

	return GE_SUCCESS;
}
//【恶魔之魂】
int JianDi::EMoZhiHun()
{
	int ret;
	int card_id;
	 CommandRequest cmd_req;
	 Coder::askForSkill(id,E_MO_ZHI_HUN, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0)==1)         //启动
			{
				//宣告技能
			    network::SkillMsg skill;
				Coder::skillNotice(id, id,E_MO_ZHI_HUN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				
			   used_E_MO_ZHI_HUN=true;

           //移除一个【恶魔之魂】
		      card_id = respond->card_ids(0);
			  engine->setStateMoveOneCardNotToHand(id, DECK_COVER,-1,DECK_DISCARD,card_id,id,E_MO_ZHI_HUN,false);
			//  TianShiYuEMo();
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

int JianDi::EMoZhiHun_Effect()
{
	//+2 【剑气】
	   setToken(0,token[0]+2);
	   network::GameInfo update_info;
	   Coder::tokenNotice(id,0,token[0], update_info);
	   engine->sendMessage(-1, MSG_GAME, update_info);
	return GE_SUCCESS;
}