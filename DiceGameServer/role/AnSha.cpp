#include "AnSha.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool AnSha::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case SHUI_YING:
			//tryNotify��������Ϸ���̴߳���Ϣ��ֻ��id���ڵ�ǰ�ȴ�id������state���ڵ�ǰstate������step���ڵ�ǰstep����Ϸ���̲߳Ż����
			session->tryNotify(id, STATE_TIMELINE_3, SHUI_YING, respond);
			return true;
		case QIAN_XING:
			//tryNotify��������Ϸ���̴߳���Ϣ��ֻ��id���ڵ�ǰ�ȴ�id������state���ڵ�ǰstate������step���ڵ�ǰstep����Ϸ���̲߳Ż����
			session->tryNotify(id, STATE_BOOT, QIAN_XING, respond);
			return true;
		}
	}
	//ûƥ���򷵻�false
	return false;
}

int AnSha::p_boot(int &step, int currentPlayerID)
{
	if (currentPlayerID != id || gem == 0)
		return GE_SUCCESS;
	step = QIAN_XING;
	int ret = QianXingBoot();
	if(toNextStep(ret) || ret == GE_URGENT){
		//ȫ����������step���STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int AnSha::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	if (engine->getCurrentPlayerID() == id && con->attack.isActive)
	{
		// Ǳ�в���Ӧս
		return QianXingNoReattack(con);
	}
	return GE_SUCCESS;
}

// Ǳ�������˺�
int AnSha::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con)
{
	if (engine->getCurrentPlayerID() == id && con->harm.srcID == id)
	{
		// Ǳ�������˺�
		return QianXingDamage(con);
	}
	return GE_SUCCESS;
}

// ˮӰ
int AnSha::p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con)
{
	if (con->dstID == id)
	{
		// ˮӰ
		step = SHUI_YING;
		int ret = ShuiYing(con);
		if(toNextStep(ret) || ret == GE_URGENT){
			//ȫ����������step���STEP_DONE
			step = STEP_DONE;
		}
		return ret;
	}
	return GE_SUCCESS;
}

int AnSha::p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con)
{
	if (con->dstID != id || con->harm.type != HARM_ATTACK)
	{
		return GE_SUCCESS;
	}
	int ret = FanShi(con);
	if(toNextStep(ret) || ret == GE_URGENT){
		//ȫ����������step���STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int AnSha::v_attacked()
{
	if (tap == false)
		return GE_SUCCESS;
	else
		return GE_INVALID_PLAYERID;
}

int AnSha::p_before_action(int &step, int currentPlayerID)
{
	if (currentPlayerID != id || !tap)
		return GE_SUCCESS;

	int ret = QianXingReset();
	if(toNextStep(ret) || ret == GE_URGENT){
		//ȫ����������step���STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int AnSha::FanShi(CONTEXT_TIMELINE_6_DRAWN *con)
{
	
	vector<int> cards;
	HARM harm;
	harm.srcID = id;
	harm.type = HARM_NONE;
	harm.point = 1;
	harm.cause = FAN_SHI;
	engine->setStateMoveCardsToHand(-1, DECK_PILE, con->harm.srcID, DECK_HAND, 1, cards, harm, false);

	SkillMsg skill_msg;
	Coder::skillNotice(id, con->harm.srcID, FAN_SHI, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	return GE_URGENT;
}

int AnSha::ShuiYing(CONTEXT_TIMELINE_3 *con)
{
	CommandRequest cmd_req;
	int ret;
	Coder::askForSkill(id, SHUI_YING, cmd_req);
	//���޵ȴ�����UserTask����tryNotify����
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

				if (getCardByID(card_id)->getElement() == ELEMENT_WATER && checkOneHandCard(card_id) == GE_SUCCESS)
					cards.push_back(card_id);
			}
			if (cards.size() > 0)
			{
				//չʾ����������
				CardMsg show_card;
				Coder::showCardNotice(id, cards.size(), cards, show_card);
				engine->sendMessage(-1, MSG_CARD, show_card);
				engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cards.size(), cards, id, SHUI_YING, true);
				
				SkillMsg skill_msg;
				Coder::skillNotice(id, id, SHUI_YING, skill_msg);
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
		//��ʱɶ��������
		return GE_TIMEOUT;
	}
}

int AnSha::QianXingBoot()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, QIAN_XING, cmd_req);
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
				Coder::skillNotice(id, id, QIAN_XING, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				tap = true;
				gem -= 1;
				GameInfo game_info;
				Coder::tapNotice(id, true, game_info);
				Coder::energyNotice(id, gem, crystal, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				CONTEXT_TAP *con = new CONTEXT_TAP; con->id = id; con->tap = tap; engine->pushGameState(new StateTap(con));
				engine->setStateChangeMaxHand(id, false, false, 6, -1);
				return GE_URGENT;
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

int AnSha::QianXingNoReattack(CONTEXT_TIMELINE_1 *con)
{
	if (tap)
	{
		con->hitRate = RATE_NOREATTACK;
	}
	return GE_SUCCESS;
}

int AnSha::QianXingDamage(CONTEXT_TIMELINE_2_HIT *con)
{
	if (tap)
	{
		con->harm.point += crystal + gem;
	}
	return GE_SUCCESS;
}

int AnSha::QianXingReset()
{
	tap = false;
	GameInfo game_info;
	Coder::tapNotice(id, false, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
	engine->setStateChangeMaxHand(id, false, false, 6, 1);	
	return GE_URGENT;
}
