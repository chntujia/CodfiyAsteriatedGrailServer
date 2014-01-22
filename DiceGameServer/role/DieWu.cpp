#include "DieWu.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool DieWu::cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case DU_FEN:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id,STATE_TIMELINE_6,DU_FEN, respond);
			return true;
		case CHAO_SHENG:
			if(!used_DiaoLing)
			  session->tryNotify(id,STATE_TIMELINE_6,CHAO_SHENG, respond);
			else
              session->tryNotify(id,STATE_TIMELINE_6,DIAO_LING, respond);
			return true;
		case JING_HUA_SHUI_YUE:
			session->tryNotify(id,STATE_TIMELINE_6,JING_HUA_SHUI_YUE, respond);
			return true;
		case DIAO_LING:
			session->tryNotify(id,STATE_COVER_CHANGE,DIAO_LING,respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

int DieWu::p_before_turn_begin(int &step, int currentPlayerID) 
{
	used_DiaoLing=false;

	return GE_SUCCESS; 
}

int DieWu::v_magic_skill(Action *action)
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
	case WU_DONG:
		return GE_SUCCESS;
		break;
	case YONG_HUA:
		  if(gem==0) 
			return GE_INVALID_ACTION;
		break;
	case DAO_NI_ZHI_DIE:
		if(getEnergy()==0)
			return GE_INVALID_ACTION;
        break;
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int DieWu::p_magic_skill(int &step, Action* action)
{
	//p_magic_skill不同于别的触发点，有且只有一个匹配，因此每一个技能完成时，务必把step设为STEP_DONE
	int ret;
	switch(action->action_id())
	{
	case WU_DONG:
		ret = WuDong(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case YONG_HUA:
		ret = YongHua(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case DAO_NI_ZHI_DIE:
		ret = DaoNiZhiDie(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}


int DieWu::p_timeline_6(int &step, CONTEXT_TIMELINE_6 *con) 
{   
	int ret = GE_INVALID_STEP;
	bool flag=false;

	list<int>::iterator it,it1;
    
    for (it = coverCards.begin(); it != coverCards.end(); ++it)
  {   
	for (it1 = coverCards.begin(); it1 != coverCards.end(); ++it1)
	 if((getCardByID((*it))->getID()!=getCardByID((*it1))->getID())&&(getCardByID((*it))->getElement()==getCardByID((*it1))->getElement()) )
            flag=true;
  }

while(STEP_DONE!=step)
{
       switch(step)
     {
    case STEP_INIT:
		{
	   if(con->harm.type==HARM_MAGIC&&con->harm.point==1&&coverCards.size()>0)
		   step =DU_FEN;
	   else if(con->dstID==id&&coverCards.size()>0)
		   step=CHAO_SHENG;
	   else if(con->harm.type==HARM_MAGIC&&con->harm.point==2&&flag)
		   step=JING_HUA_SHUI_YUE;
	   else
		   step=STEP_DONE;
		}
	   break;
     case DU_FEN:
		 {
		  ret =DuFeng(con);
		  if(toNextStep(ret))
		  {
			  if(con->dstID == id&&coverCards.size()>0)
				  step=CHAO_SHENG;
			    else if(con->harm.type==HARM_MAGIC&&con->harm.point==2&&flag)
		            step=JING_HUA_SHUI_YUE;
	          else
		        step=STEP_DONE;
			
		  }	
		 }
	   break;
	 case CHAO_SHENG:
		  ret =ChaoSheng(con);
	      if(toNextStep(ret) || ret == GE_URGENT){
			 if(con->harm.type==HARM_MAGIC&&con->harm.point==2&&flag)
		                step=JING_HUA_SHUI_YUE;
			  else
		           step = STEP_DONE;
	            }
	   break;
	 case JING_HUA_SHUI_YUE:
		  ret =JingHuaShuiYue(con);
	      if(toNextStep(ret) || ret == GE_URGENT){
		           step = STEP_DONE;
	            }
	   break;
  default:
			return GE_INVALID_STEP;
     }

  }
   return ret;
}

int DieWu::p_cover_change(int &step, int dstID, int direction, int howMany, vector<int> cards, int doerID, int cause)  
{ 
	int ret = GE_INVALID_STEP;
	if(doerID != id || direction != CHANGE_REMOVE){
		return GE_SUCCESS;
	}

	step=DIAO_LING;
    if (getCardByID(cards[0])->getType()==TYPE_MAGIC)
	    Diao_Ling(cards);

	
    return GE_SUCCESS; 
}

int DieWu::p_lose_morale(int &step, CONTEXT_LOSE_MORALE *con)
{
	int ret = GE_SUCCESS;
	if(used_DiaoLing)
	{
		ret=DiaoLing_Effect(con);
	}
	return ret;
}
//【舞动】
int DieWu::WuDong(Action *action)
{   
	int ret;
	int flag=action->args(0);
	int cardID;
	vector<int> cards;
	//宣告技能
	network::SkillMsg skill;
	Coder::skillNotice(id,id, WU_DONG, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);

	//将牌库顶一张牌移到盖牌区
	    HARM harm1;
	    harm1.srcID = id;
	    harm1.type = HARM_NONE;
	    harm1.point = 1;
	    harm1.cause = WU_DONG;
	    engine->setStateMoveCardsToHand(-1, DECK_PILE,id, DECK_COVER, 1, cards, harm1,false);
      
	 if(flag==1)
	{
		HARM harm;
	    harm.srcID = id;
	    harm.type = HARM_NONE;
	    harm.point = 1;
	    harm.cause = WU_DONG;
	    engine->setStateMoveCardsToHand(-1,DECK_PILE,id,DECK_HAND,1, cards,harm, false);
	}
	else
	{
		cardID=action->card_ids(0);
		engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, WU_DONG, true);
	}
	 //插入了新状态，请return GE_URGENT
	return GE_URGENT;
}
//【毒粉】
int DieWu::DuFeng(CONTEXT_TIMELINE_6 *con)
{
    CommandRequest cmd_req;
	Coder::askForSkill(id,DU_FEN, cmd_req);
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
				network::SkillMsg skill;
				Coder::skillNotice(id, id,DU_FEN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				int cardID;
				cardID=respond->card_ids(0);
				//移除盖牌
				engine->setStateMoveOneCardNotToHand(id,DECK_COVER, -1, DECK_DISCARD, cardID, id,DU_FEN, true);
				
				con->harm.point=con->harm.point+1;  //伤害加1
		   }
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}
//【朝圣】   考虑【灵魂链接】
int DieWu::ChaoSheng(CONTEXT_TIMELINE_6 *con)
{
	CommandRequest cmd_req;
	Coder::askForSkill(id,CHAO_SHENG, cmd_req);
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
		//		int temp=respond->args(1); //
			    network::SkillMsg skill;
				Coder::skillNotice(id, id,CHAO_SHENG, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				int cardID;
			//	cardID=respond->card_ids(0);
				cardID=respond->args(1);
				//移除盖牌
				engine->setStateMoveOneCardNotToHand(id,DECK_COVER, -1, DECK_DISCARD, cardID, id,CHAO_SHENG, true);
				con->harm.point=con->harm.point-1;  //抵御一点伤害来源

		   }
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}

}
//【镜花水月】
int DieWu::JingHuaShuiYue(CONTEXT_TIMELINE_6 *con)
{  
    CommandRequest cmd_req;
	Coder::askForSkill(id,JING_HUA_SHUI_YUE, cmd_req);
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
		//		int temp=respond->args(1); //
			    network::SkillMsg skill;
				Coder::skillNotice(id, id,JING_HUA_SHUI_YUE, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				
			vector<int> cards;
			int card_id;
			for (int i = 0; i < respond->card_ids_size(); ++i)
			{
				card_id = respond->card_ids(i);

				if (checkOneCoverCard(card_id) == GE_SUCCESS)
					cards.push_back(card_id);
			}

			if (cards.size() > 0)
			{
				ret = engine->setStateMoveCardsNotToHand(id, DECK_COVER, -1, DECK_DISCARD, cards.size(), cards, id,JING_HUA_SHUI_YUE, true);
				
			}
        //展示
		CardMsg show_card;
		Coder::showCardNotice(id,2,cards, show_card);
		engine->sendMessage(-1, MSG_CARD, show_card);

	           HARM harm;
	           harm.srcID = id;
	           harm.point = 1;
	           harm.type = HARM_MAGIC;
	           harm.cause =JING_HUA_SHUI_YUE;
	           engine->setStateTimeline3(con->dstID , harm);
			   engine->setStateTimeline3(con->dstID , harm);
			   con->harm.point=0;   //抵御该次伤害

		   }
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}
//【蛹化】
int DieWu::YongHua(Action *action)
{  
	int num;
	vector<int> cards;
	//【蛹化】
   network::SkillMsg skill;
   Coder::skillNotice(id, id,YONG_HUA, skill);
   engine->sendMessage(-1, MSG_SKILL, skill);

   //+1 蛹
   token[0]+=1;
   gem--;
   GameInfo game_info;
   Coder::tokenNotice(id, 0, token[0], game_info);
   Coder::energyNotice(id, gem, crystal, game_info);
   engine->sendMessage(-1, MSG_GAME, game_info);

   if(coverCards.size()<8)
   {
     //将牌库顶4张牌移到盖牌区
   HARM harm;
   harm.srcID = id;
   harm.type = HARM_NONE;
   harm.point = 4;
   harm.cause = WU_DONG;
   engine->setStateMoveCardsToHand(-1,DECK_PILE,id,DECK_COVER,4,cards,harm,false);
   }
   num=(6-token[0]>3)?(6-token[0]):3;
   engine->setStateChangeMaxHand(id, true, true,num);

   return GE_URGENT;
}
//【倒逆之蝶】
int DieWu::DaoNiZhiDie(Action *action)
{
	      //弃两张牌
           vector<int> cards;
		   vector<int> covercards;
		   int card_id;
		   int covercard_id[2];
		   int dstID;

			

			if(action->args(0)==1 )
			{  
			   dstID=action->dst_ids(0);
			   HARM harm;
	           harm.srcID =id;
	           harm.point = 1;
	           harm.type = HARM_MAGIC;
	           harm.cause =DAO_NI_ZHI_DIE;
	        //   engine->setStateTimeline3(dstID, harm);
               engine->setStateTimeline6(dstID,harm);
 
			 }
	else 
	  {   
		 setToken(0,token[0]+1);
		 if(action->args(0)==2)
		    {
				for (int j = 0; j <2; ++j)
				{
					covercard_id[j]=action->args(j+1);
				if (checkOneCoverCard(covercard_id[j]) == GE_SUCCESS)
					covercards.push_back(covercard_id[j]);

				}

			  if (covercards.size()>0)
			   {
				engine->setStateMoveCardsNotToHand(id, DECK_COVER, -1, DECK_DISCARD,covercards.size(),covercards, id,DAO_NI_ZHI_DIE,true);
				
			    }
			   
			}
			else
			{
			    
			   HARM harm;
	           harm.srcID =id;
	           harm.point =4;
	           harm.type = HARM_MAGIC;
	           harm.cause =DAO_NI_ZHI_DIE;
	           engine->setStateTimeline3(id, harm);
			}

	    }  
       //弃两张牌
       for (int i = 0; i < action->card_ids_size(); ++i)
			{
				card_id = action->card_ids(i);

				if (checkOneHandCard(card_id) == GE_SUCCESS)
					cards.push_back(card_id);
			}

			if (cards.size() > 0)
			{
				engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cards.size(),cards, id,DAO_NI_ZHI_DIE,true);
				
			}


		 if(getCrystal()>0)
			crystal--;
		  else
			gem--;

        GameInfo game_info;
        Coder::tokenNotice(id, 0, token[0], game_info);
        Coder::energyNotice(id, gem, crystal, game_info);
        engine->sendMessage(-1, MSG_GAME, game_info);
        return GE_URGENT;
}
//【凋零】
int DieWu::Diao_Ling(vector<int> cards)
{
  int dstID;
  CommandRequest cmd_req;
  Coder::askForSkill(id,DIAO_LING, cmd_req);
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
				network::SkillMsg skill;
				Coder::skillNotice(id, id,DIAO_LING, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				dstID=respond->dst_ids(0);

               HARM harm;
	           harm.srcID =id;
	           harm.point = 2;
	           harm.type = HARM_MAGIC;
	           harm.cause =DIAO_LING;
	           engine->setStateTimeline3(id, harm);

			   HARM harm1;
	           harm1.srcID =id;
	           harm1.point = 1;
	           harm1.type = HARM_MAGIC;
	           harm1.cause =DIAO_LING;
	           engine->setStateTimeline3(dstID, harm1);

			  CardMsg show_card;
	          Coder::showCardNotice(id, cards.size(),cards, show_card);
	          engine->sendMessage(-1, MSG_CARD, show_card);
           	   used_DiaoLing=true;
		   }
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}


int DieWu::DiaoLing_Effect(CONTEXT_LOSE_MORALE *con)
{

int	dst_color=engine->getPlayerEntity(con->dstID)->getColor();
int	current_color=engine->getPlayerEntity(id)->getColor();
     //对方士气强制最少为1
  if(dst_color!=current_color)
 {   
	 TeamArea *m_teamArea = engine->getTeamArea(); 
	 int morale =m_teamArea->getMorale(dst_color);
	  con->howMany=(morale-con->howMany>0)?con->howMany:(morale-1);
	 }
  return  GE_SUCCESS;
}