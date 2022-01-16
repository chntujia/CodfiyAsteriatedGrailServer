#include "ZhongCai.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

ZhongCai::ZhongCai(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color)
{
	setCrystal(2);
	GameInfo game_info;
	Coder::energyNotice(id, gem, crystal, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
	tokenMax[0] = 4;
}

bool ZhongCai::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case YI_SHI_ZHONG_DUAN:
			//tryNotify��������Ϸ���̴߳���Ϣ��ֻ��id���ڵ�ǰ�ȴ�id������state���ڵ�ǰstate������step���ڵ�ǰstep����Ϸ���̲߳Ż����
			session->tryNotify(id, STATE_BOOT, YI_SHI_ZHONG_DUAN, respond);
			return true;
		case ZHONG_CAI_YI_SHI:
			//tryNotify��������Ϸ���̴߳���Ϣ��ֻ��id���ڵ�ǰ�ȴ�id������state���ڵ�ǰstate������step���ڵ�ǰstep����Ϸ���̲߳Ż����
			session->tryNotify(id, STATE_BOOT, ZHONG_CAI_YI_SHI, respond);
			return true;
		}
	}
	//ûƥ���򷵻�false
	return false;
}

int ZhongCai::p_turn_begin(int &step, int currentPlayerID)
{
	int ret = GE_SUCCESS;
	if (currentPlayerID == id && tap)
	{
		ret = ZhongCaiYiShiAddToken();
	}
	step = STEP_DONE;
	return ret;
}

int ZhongCai::p_boot(int &step, int currentPlayerID)
{
	if (currentPlayerID != id)
		return GE_SUCCESS;
	int ret;
	if (tap)
	{
		step = YI_SHI_ZHONG_DUAN;
		ret = YiShiZhongDuan();
		if(toNextStep(ret) || ret == GE_URGENT){
			//ȫ����������step���STEP_DONE
			step = STEP_DONE;
		}
		return ret;
	}
	else if (gem > 0)
	{
		step = ZHONG_CAI_YI_SHI;
		ret = ZhongCaiYiShi();
		if(toNextStep(ret) || ret == GE_URGENT){
			//ȫ����������step���STEP_DONE
			step = STEP_DONE;
		}
		return ret;
	}
	return GE_SUCCESS;
}

int ZhongCai::p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con)
{
	if (con->dstID != id || con->harm.type == HARM_NONE)
		return GE_SUCCESS;
	int ret = ShenPanLangChao();
	step = STEP_DONE;
	return ret;
}

int ZhongCai::v_allow_action(Action* action, int allow, bool canGiveUp)
{
	if (token[0] < 4)
		return PlayerEntity::v_allow_action(action, allow, canGiveUp);

	if(canGiveUp && action->action_type() == ACTION_NONE)
		return GE_SUCCESS;

	if (action->action_type() != ACTION_MAGIC_SKILL || action->action_id() != MO_RI_SHEN_PAN)
		return GE_INVALID_ACTION;

	return GE_SUCCESS;
}

int ZhongCai::v_magic_skill(Action* action)
{
	int actionID = action->action_id();
	int playerID = action->src_id();

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}

	switch(actionID)
	{
	case MO_RI_SHEN_PAN:
		//  û������
		if (token[0] == 0){
			return GE_INVALID_ACTION;
		}
		break;
	case PAN_JUE_TIAN_PING:
		//  ������        || ��©�˲���               || û������
		if (token[0] == 4 || action->args_size() == 0 || crystal + gem == 0){
			return GE_INVALID_ACTION;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int ZhongCai::p_magic_skill(int &step, Action *action)
{
	int actionID = action->action_id();
	int ret;

	switch(actionID)
	{
	case MO_RI_SHEN_PAN:
		ret = MoRiShenPan(action);
		if (ret == GE_URGENT)
			step = STEP_DONE;
		break;
	case PAN_JUE_TIAN_PING:
		ret = PanJueTianPing(action);
		if (ret == GE_URGENT)
			step = STEP_DONE;
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}


int ZhongCai::YiShiZhongDuan()
{
	CommandRequest cmd_req;
	int ret;
	Coder::askForSkill(id, YI_SHI_ZHONG_DUAN, cmd_req);
	Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size()-1));
	//���޵ȴ�����UserTask����tryNotify����
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			if (respond->args_size()>0 && respond->args(0) == 1)
			{
				SkillMsg skill_msg;
				Coder::skillNotice(id, id, YI_SHI_ZHONG_DUAN, skill_msg);
				engine->sendMessage(-1, MSG_SKILL, skill_msg);

				TeamArea* team_area = engine->getTeamArea();
				team_area->setGem(color, team_area->getGem(color)+1);
				tap = false;

				GameInfo game_info;
				Coder::stoneNotice(color, team_area->getGem(color), team_area->getCrystal(color), game_info);
				Coder::tapNotice(id, tap, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				
				engine->setStateChangeMaxHand(id, true, false, 6, 0);
				return GE_URGENT;
			}
		}
	}
	else{
		return GE_TIMEOUT;
	}
	return GE_SUCCESS;
}

int ZhongCai::ZhongCaiYiShi()
{
	CommandRequest cmd_req;
	int ret;
	Coder::askForSkill(id, ZHONG_CAI_YI_SHI, cmd_req);
	Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size()-1));
	//���޵ȴ�����UserTask����tryNotify����
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			if (respond->args_size()>0 && respond->args(0) == 1)
			{
				SkillMsg skill_msg;
				Coder::skillNotice(id, id, ZHONG_CAI_YI_SHI, skill_msg);
				engine->sendMessage(-1, MSG_SKILL, skill_msg);

				setGem(gem-1);
				tap = true;

				GameInfo game_info;
				Coder::tapNotice(id, tap, game_info);
				Coder::energyNotice(id, gem, crystal, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				CONTEXT_TAP *con = new CONTEXT_TAP; con->id = id; con->tap = tap; engine->pushGameState(new StateTap(con));
				engine->setStateChangeMaxHand(id, true, true, 5, 0);
				return GE_URGENT;
			}
		}
	}
	else{
		return GE_TIMEOUT;
	}
	return GE_SUCCESS;
}
	
int ZhongCai::MoRiShenPan(Action* action)
{
	SkillMsg skill_msg;
	Coder::skillNotice(id, id, MO_RI_SHEN_PAN, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	HARM shenpan;
	shenpan.cause = MO_RI_SHEN_PAN;
	shenpan.point = token[0];
	shenpan.srcID = id;
	shenpan.type = HARM_MAGIC;
	engine->setStateTimeline3(action->dst_ids(0), shenpan);

	setToken(0, 0);

	GameInfo game_info;
	Coder::tokenNotice(id, 0, token[0], game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
	return GE_URGENT;
}
	
int ZhongCai::ShenPanLangChao()
{
	SkillMsg skill_msg;
	Coder::skillNotice(id, id, SHEN_PAN_LANG_CHAO, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	
	setToken(0, token[0]+1);
	
	GameInfo game_info;
	Coder::tokenNotice(id, 0, token[0], game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);

	return GE_SUCCESS;
}

int ZhongCai::PanJueTianPing(Action* action)
{
	// ��������
	if (crystal > 0)
		setCrystal(crystal-1);
	else
		setGem(gem-1);
	
	// +1����
	setToken(0, token[0]+1);
	SkillMsg skill_msg;
	Coder::skillNotice(id, id, PAN_JUE_TIAN_PING, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	GameInfo game_info;
	Coder::energyNotice(id, gem, crystal, game_info);
	Coder::tokenNotice(id, 0, token[0], game_info);

	if (action->args(0) == 0)
	{
		vector<int> hands;
		list<int>::iterator hand_card_it;
		for (hand_card_it=handCards.begin(); hand_card_it!=handCards.end(); ++hand_card_it)
			hands.push_back(*hand_card_it);
		// ��������
		engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, handCards.size(), hands, id, PAN_JUE_TIAN_PING, false);
	}
	else
	{
		// ��������
		if (handCardsMax > handCards.size())
		{
			vector<int> cards;
			HARM tianping;
			tianping.cause = PAN_JUE_TIAN_PING;
			tianping.point = handCardsMax-handCards.size();
			tianping.srcID = id;
			tianping.type = HARM_NONE;
			engine->setStateMoveCardsToHand(-1, DECK_PILE, id, DECK_HAND, handCardsMax-handCards.size(), cards, tianping, false);
		}
		// ս����+1��ʯ
		TeamArea* team_area = engine->getTeamArea();
		team_area->setGem(color, team_area->getGem(color)+1);

		Coder::stoneNotice(color, team_area->getGem(color), team_area->getCrystal(color), game_info);
	}
	
	engine->sendMessage(-1, MSG_GAME, game_info);
	return GE_URGENT;
}

int ZhongCai::ZhongCaiYiShiAddToken()
{
	setToken(0, token[0]+1);
	
	GameInfo game_info;
	Coder::tokenNotice(id, 0, token[0], game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
	return GE_SUCCESS;
}