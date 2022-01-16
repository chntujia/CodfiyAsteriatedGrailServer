#include "MoJian.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"


bool MoJian::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case AN_YING_NING_JU:
			//tryNotify��������Ϸ���̴߳���Ϣ��ֻ��id���ڵ�ǰ�ȴ�id������state���ڵ�ǰstate������step���ڵ�ǰstep����Ϸ���̲߳Ż����
			session->tryNotify(id,STATE_BOOT,AN_YING_NING_JU, respond);
			return true;

		case HEI_AN_ZHEN_CHAN:
			//tryNotify��������Ϸ���̴߳���Ϣ��ֻ��id���ڵ�ǰ�ȴ�id������state���ڵ�ǰstate������step���ڵ�ǰstep����Ϸ���̲߳Ż����
			session->tryNotify(id,STATE_TIMELINE_1,HEI_AN_ZHEN_CHAN, respond);  //��ʲô״̬������
			return true;
		}
	}
	//ûƥ���򷵻�false
	return false;
}

//ͳһ��p_before_turn_begin ��ʼ�����ֻغϱ���
int  MoJian::p_before_turn_begin(int &step, int currentPlayerID) 
{
	used_XiuLuoLianZhan = false;
	used_HeiAnZhenChan = false;
	using_XiuLuoLianZhan=false;
	using_HeiAnZhenChan = false;

	return GE_SUCCESS; 
}

int MoJian::p_before_action(int &step, int currentPlayerID)
{
	if (currentPlayerID != id || !tap)
		return GE_SUCCESS;

	int ret = AnYingNingJuReset();
	if(toNextStep(ret) || ret == GE_URGENT){
		//ȫ����������step���STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

//������ ����Ӱ���ۡ�
int MoJian::p_boot(int &step,int currentPlayerID)
{
	if (currentPlayerID != id)
		return GE_SUCCESS;
	step =AN_YING_NING_JU;
	int ret = AnYingNingJu();
	if(toNextStep(ret) || ret == GE_URGENT){
		//ȫ����������step���STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

/**
����Ӱ���ǡ�
**/
int MoJian::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int cardID;
	int playerID = action->src_id();
	CardEntity* card;

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}

	if(action->action_id()==AN_YING_LIU_XING&&tap)
	{
		//�ж��Ƿ������ŷ�����
		if(action->card_ids_size()!=2)
		{
			return GE_INVALID_ACTION;
		}

		for(int i =0;i<action->card_ids_size();i++)
		{
			cardID = action->card_ids(i);
			card = getCardByID(cardID);
			if( TYPE_MAGIC != card->getType() || GE_SUCCESS != checkOneHandCard(cardID))
			{
				return GE_INVALID_ACTION;
			}
		}
		return GE_SUCCESS;

	}

	else 
		return GE_INVALID_ACTION;
}

int MoJian::p_magic_skill(int &step, Action* action)
{
	int ret;
	int actionID = action->action_id();
	if(action->action_id()==AN_YING_LIU_XING&&tap)
	{
		ret = AnYingLiuXing(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		return ret;
	}
	else
	{
		return GE_INVALID_ACTION;
	}

}

int MoJian::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con) 
{ 	
	//��ȡ��ǰӢ�۵ı�ʯ�����жϱ�ʯ���Ƿ����1,�ҽ��С��ڰ���������غ��޶ȡ�
	if (con->attack.srcID == id && con->attack.isActive && gem > 0 && !used_HeiAnZhenChan)
	{
		// Ǳ�в���Ӧս
		step =HEI_AN_ZHEN_CHAN;
		int ret =HeiAnZhenChanNoReattack(con);
		if(toNextStep(ret) || ret == GE_URGENT){
			//ȫ����������step���STEP_DONE	
			step = STEP_DONE;
		}
		return  ret;
	}
	else 
		return  GE_SUCCESS;	
}


//����Ӱ֮���������ڰ����������
int MoJian::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con)
{

	int ret = GE_INVALID_STEP;
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	if(step == STEP_INIT || step == AN_YING_ZHI_LI){
		//��ʼ��step
		step = AN_YING_ZHI_LI;
		ret = AnYingZhiLi(con);
		if(toNextStep(ret)){
			if(using_HeiAnZhenChan)
				step =HEI_AN_ZHEN_CHAN_BU_PAI;
			else 
				step=STEP_DONE;
		}			
	}
	if(step == HEI_AN_ZHEN_CHAN_BU_PAI){

		ret =HeiAnZhenChanBuPai(con);  //����������
		if(toNextStep(ret) || ret == GE_URGENT){
			//ȫ����������step���STEP_DONE	
			step = STEP_DONE;
		}
	}
	return ret;
}

int MoJian::p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con)
{
	using_HeiAnZhenChan = false;
	return GE_SUCCESS;
}

//���ж����ж������Ǽ��е�һ���ط�ѯ�ʣ�������ÿ������һ��
int MoJian::p_after_attack(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	if(playerID != id){
		return GE_SUCCESS;
	}
	if(step == STEP_INIT || step == XIU_LUO_LIAN_ZHAN){
		//��ʼ��step
		step = XIU_LUO_LIAN_ZHAN;
		ret =XiuLuoLianZhan(playerID);
		if(toNextStep(ret)){
			step = STEP_DONE;;
		}			
	}
	return ret;
}

/**
��������ն����ֻ���ܻ�ϵ����
**/  

int  MoJian::v_attack(int cardID, int dstID, bool realCard)
{
	if(using_XiuLuoLianZhan){
		CardEntity* card = getCardByID(cardID);
		if(card->getElement() != ELEMENT_FIRE){
			return GE_INVALID_ACTION;
		}
	}
	//ͨ����ɫ��صļ�⣬����������⽻���ײ�
	return PlayerEntity::v_attack(cardID, dstID, realCard);
}

/**
����Ӱ���ܡ�������ɫ�ж��׶η����Ʋ�����
**/
int MoJian::v_additional_action(int chosen)
{
	switch(chosen)
	{
	case XIU_LUO_LIAN_ZHAN:
		//�غ��޶�
		if(used_XiuLuoLianZhan){
			return GE_INVALID_ACTION;
		}
		break;
	}
	//ͨ����ɫ��صļ�⣬������⽻���ײ�
	return PlayerEntity::v_additional_action(chosen);
}

int MoJian::p_additional_action(int chosen)
{
	GameInfo update_info;
	switch(chosen)
	{
	case XIU_LUO_LIAN_ZHAN:
		used_XiuLuoLianZhan = true;
		using_XiuLuoLianZhan = true;
		break;
	default:
		using_XiuLuoLianZhan = false;
	}
	//�����ɫ��صĲ������۳������ж������ײ�
	return PlayerEntity::p_additional_action(chosen);
}

/***
����Ӱ���ܡ��������Ʋ�����
***/

//ħ�����Ǳ���
int MoJian::v_missile(int cardID, int dstID, bool realCard)
{
	if(realCard){
		int ret;
		if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
			return ret;
		}
	}
	return GE_INVALID_CARDID;
}

int MoJian::v_shield(int cardID, PlayerEntity* dst)
{
	int ret;
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
		return ret;
	}
	return GE_INVALID_CARDID;
}

int MoJian::v_weaken(int cardID, PlayerEntity* dst)
{
	int ret;
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
		return ret;
	}
	return GE_INVALID_CARDID;
}

/***
��������ն��
***/
int  MoJian::XiuLuoLianZhan(int playerID)
{
	//�ǲ���ħ��      || ��û������        || �غ��޶�      || �Ѿ����ϡ�������ն����
	if(playerID != id || handCards.empty() || used_XiuLuoLianZhan || containsAction(XIU_LUO_LIAN_ZHAN)){
		return GE_SUCCESS;
	}
	addAction(ACTION_ATTACK, XIU_LUO_LIAN_ZHAN);  //�����ж�
	return GE_SUCCESS;
}
/*
����Ӱ֮����  ���е�ʱ���һ���˺�  
*/
int MoJian::AnYingZhiLi(CONTEXT_TIMELINE_2_HIT *con)
{
	//�ڡ���Ӱ��̬���� ���������й����˺�+1
	if(tap) {	
		int srcID = con->attack.srcID;
		int dstID = con->attack.dstID;
		if(srcID != id){
			return GE_SUCCESS;
		}
		SkillMsg skill;
		Coder::skillNotice(id, con->attack.dstID, AN_YING_ZHI_LI, skill);
		engine->sendMessage(-1, MSG_SKILL, skill);
		con->harm.point += 1;
	}
	return GE_SUCCESS;
}

/*
����Ӱ���ۡ�
*/
int MoJian::AnYingNingJu()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, AN_YING_NING_JU, cmd_req);
	//���޵ȴ�����UserTask����tryNotify����
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//����
			if (respond->args(0) == 1)  //�Լ�����
			{
				network::SkillMsg skill;
				Coder::skillNotice(id, id, AN_YING_NING_JU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				tap = true;
				GameInfo game_info;
				Coder::tapNotice(id, true, game_info);
				CONTEXT_TAP *con = new CONTEXT_TAP; con->id = id; con->tap = tap; engine->pushGameState(new StateTap(con));
				engine->sendMessage(-1, MSG_GAME, game_info);
				HARM harm;
				harm.type=HARM_MAGIC;
				harm.point=1;
				harm.srcID=id;
				harm.cause=AN_YING_NING_JU;				
				engine->setStateTimeline3(id, harm);
				return GE_URGENT;
			}
		}
		return ret;
	}
	else{
		//��ʱɶ��������
		return GE_TIMEOUT;
	}
}

/*
�غϿ�ʼ������Ӱ��̬��--��������״̬��
*/
int MoJian::AnYingNingJuReset()
{
	tap = false;
	//�� ��Ӱ��̬ �ص� ������̬
	GameInfo game_info;
	Coder::tapNotice(id, false, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);

	return GE_SUCCESS;
}

int MoJian::HeiAnZhenChanNoReattack(CONTEXT_TIMELINE_1 *con)
{
	int dstID = con->attack.dstID;
	CommandRequest cmd_req;
	Coder::askForSkill(id, HEI_AN_ZHEN_CHAN, cmd_req);

	//���޵ȴ�����UserTask����tryNotify����
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//����
			if (respond->args(0) == 1)
			{
				network::SkillMsg skill;
				Coder::skillNotice(id, dstID,HEI_AN_ZHEN_CHAN, skill);  //gaidong
				engine->sendMessage(-1, MSG_SKILL, skill);

				//����������Ϣ
				gem -= 1;
				GameInfo game_info;
				Coder::energyNotice(id, gem, crystal, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);

				//��������Ӧս
				used_HeiAnZhenChan=true;
				using_HeiAnZhenChan=true;
				con->hitRate = RATE_NOREATTACK;

			}
		}
		return ret;
	}

	else{
		//��ʱɶ��������
		return GE_TIMEOUT;
	}
	return GE_SUCCESS;
}

//����������
int MoJian::HeiAnZhenChanBuPai(CONTEXT_TIMELINE_2_HIT* con)
{
	SkillMsg skill_msg;
	Coder::skillNotice(id, id, HEI_AN_ZHEN_CHAN, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	using_HeiAnZhenChan = false;
	//��ȡ��Ҫ�������������
	int num=getHandCardNum()-4;
	//�����ƣ�������
	if (num > 0)
	{
		HARM qipai;
		qipai.cause = HEI_AN_ZHEN_CHAN;
		qipai.point = num;
		qipai.srcID = id;
		qipai.type = HARM_NONE;
		engine->pushGameState(new StateRequestHand(id, qipai, -1, DECK_DISCARD, false, false));

		SkillMsg skill_msg;
		Coder::skillNotice(id, id, HEI_AN_ZHEN_CHAN, skill_msg);
		engine->sendMessage(-1, MSG_SKILL, skill_msg);
		using_HeiAnZhenChan = false;  //��ǰ���ڰ���������ڴ������
		return GE_URGENT;
	}
	else if (num < 0)
	{
		vector<int> cards;
		HARM harm;
		harm.srcID = id;
		harm.type = HARM_NONE;
		harm.point = -num;                  //������Ƽ�ȥ��ǰ����
		harm.cause = HEI_AN_ZHEN_CHAN;
		engine->setStateMoveCardsToHand(-1, DECK_PILE, con->harm.srcID, DECK_HAND, -num, cards, harm, false);
	  //��ǰ���ڰ���������ڴ������
		return GE_URGENT;
	}
	else
		return GE_SUCCESS;

}

int MoJian::AnYingLiuXing(Action* action)
{
	vector<int> cardIDs;
	vector<int> cards;
	int cardNum= action->card_ids_size();
	int  dstID = action->dst_ids(0);

	PlayerEntity *dstPlayer = engine->getPlayerEntity(dstID);

	for(int i = 0; i < cardNum;i ++)
	{
		cardIDs.push_back(action->card_ids(i));
	}
	//���漼��
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, AN_YING_LIU_XING, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	HARM  harm;
	harm.type = HARM_MAGIC;
	harm.point = 2;
	harm.srcID = id;
	harm.cause = AN_YING_LIU_XING;
	engine->setStateTimeline3(dstID, harm);
	CardMsg show_card;
	Coder::showCardNotice(id, 2, cardIDs, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs, id, AN_YING_LIU_XING, true);//���ƣ��˺�

	//��������״̬����return GE_URGENT
	return GE_URGENT;
}