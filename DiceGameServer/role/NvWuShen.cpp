#include "NvWuShen.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool NvWuShen::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case JUN_SHEN_WEI_GUANG:
			session->tryNotify(id, STATE_TURN_BEGIN, JUN_SHEN_WEI_GUANG, respond);
			return true;
		case YING_LING_ZHAO_HUAN:
			session->tryNotify(id, STATE_TIMELINE_2_HIT, YING_LING_ZHAO_HUAN, respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

int NvWuShen::p_turn_begin(int &step, int currentPlayerID)
{
	if (currentPlayerID != id || !tap )
		return GE_SUCCESS;
	step = JUN_SHEN_WEI_GUANG;
	int ret = JunShenWeiGuang();
	if(toNextStep(ret) || ret == GE_URGENT){
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int NvWuShen::p_after_attack(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	if(playerID != id || getCrossNum()<1){
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	step = SHEN_SHENG_ZHUI_JI;
	ret = ShenShengZhuiJi(playerID);
	if(toNextStep(ret) || ret == GE_URGENT){
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int NvWuShen::p_after_magic(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	if(playerID != id || getCrossNum()<1){
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	step = SHEN_SHENG_ZHUI_JI;
	ret = ShenShengZhuiJi(playerID);
	if(toNextStep(ret) || ret == GE_URGENT){
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int NvWuShen::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int playerID = action->src_id();


	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}
	switch(actionID)
	{
	case ZHI_XU_ZHI_YIN:
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int NvWuShen::p_magic_skill(int &step, Action* action)
{
	int ret;
	switch(action->action_id())
	{
	case ZHI_XU_ZHI_YIN:
		ret = ZhiXuZhiYin(step, action);
		if(GE_URGENT == ret){
			step = ZHI_XU_ZHI_YIN;
		}
		else if (GE_SUCCESS == ret){
			step = STEP_DONE;
		}		
		break;

	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int NvWuShen::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	step = HE_PING_XING_ZHE;

	ret = HePingXingZhe(con);
	if(toNextStep(ret))
	{
		step = STEP_DONE;
	}
	return ret;
}

int NvWuShen::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con)
{
	int ret = GE_INVALID_STEP;
	step = YING_LING_ZHAO_HUAN;

	ret = YingLingZhaoHuan(con);
	if(toNextStep(ret) || ret == GE_URGENT)
	{
		step = STEP_DONE;
	}
	return ret;
}
int NvWuShen::p_additional_action(int chosen)
{
	int ret = GE_INVALID_STEP;
	ret = PlayerEntity::p_additional_action(chosen);
	if(chosen == SHEN_SHENG_ZHUI_JI)
	{
		network::SkillMsg skill;
		Coder::skillNotice(id, id, SHEN_SHENG_ZHUI_JI, skill);
		engine->sendMessage(-1, MSG_SKILL, skill);
		subCrossNum(1);
		GameInfo update_info;
		Coder::crossNotice(id, getCrossNum(), update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
	}
	return ret;
}

int NvWuShen::ShenShengZhuiJi(int playerID)
{
	if(playerID != id || getCrossNum() < 1){
		return GE_SUCCESS;
	}
	addAction(ACTION_ATTACK, SHEN_SHENG_ZHUI_JI);
	return GE_SUCCESS;
}

int NvWuShen::ZhiXuZhiYin(int &step, Action* action)
{
	GameInfo game_info;

	if(step != ZHI_XU_ZHI_YIN)
	{
		SkillMsg skill_msg;
		Coder::skillNotice(id, id, ZHI_XU_ZHI_YIN, skill_msg);
		engine->sendMessage(-1, MSG_SKILL, skill_msg);
		vector<int> cards;
		HARM harm;
		harm.srcID = id;
		harm.type = HARM_NONE;
		harm.point = 2;
		harm.cause = ZHI_XU_ZHI_YIN;
		engine->setStateMoveCardsToHand(-1, DECK_PILE, id, DECK_HAND, 2, cards, harm, false);

		//插入了新状态，请return GE_URGENT
		return GE_URGENT;
	}
	else
	{
		this->addCrossNum(1);
		this->setCrystal(++crystal);
		Coder::crossNotice(id, this->getCrossNum(), game_info);
		Coder::energyNotice(id, gem, crystal, game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);

		return GE_SUCCESS;
	}
}

int NvWuShen::HePingXingZhe(CONTEXT_TIMELINE_1 *con)
{
	//非主动攻击 没有横置
	if(con->attack.srcID != id || !con->attack.isActive || !this->tap) {
		return GE_SUCCESS;
	}
	SkillMsg skill;
	Coder::skillNotice(id, id, HE_PING_XING_ZHE, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	tap=false;
	GameInfo game_info;
	Coder::tapNotice(id, tap, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
	return GE_SUCCESS;
}

int NvWuShen::JunShenWeiGuang()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, JUN_SHEN_WEI_GUANG, cmd_req);

	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;

			SkillMsg skill_msg;
			Coder::skillNotice(id, id, JUN_SHEN_WEI_GUANG, skill_msg);
			engine->sendMessage(-1, MSG_SKILL, skill_msg);

			GameInfo game_info;
			int choice = respond->args(0);
			if(choice != 1)
			{
				TeamArea *team = engine->getTeamArea();
				int color = this->getColor();
				int gem = respond->args(1);
				int crystal = respond->args(2);
				team->setGem(color, team->getGem(color)-gem);
				team->setCrystal(color, team->getCrystal(color)-crystal);
				if (color == RED){
					game_info.set_red_gem(team->getGem(color));
					game_info.set_red_crystal(team->getCrystal(color));
				}
				else{
					game_info.set_blue_gem(team->getGem(color));
					game_info.set_blue_crystal(team->getCrystal(color));
				}
				this->addCrossNum(2, -2);
				Coder::crossNotice(id, getCrossNum(), game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				return GE_SUCCESS;
			}
			else
			{
				this->addCrossNum(1);
				Coder::crossNotice(id, getCrossNum(), game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				return GE_SUCCESS;
			}
		}
		return ret;
	}
	else{
		//超时执行第一项
		SkillMsg skill_msg;
		Coder::skillNotice(id, id, JUN_SHEN_WEI_GUANG, skill_msg);
		engine->sendMessage(-1, MSG_SKILL, skill_msg);
		this->addCrossNum(1);
		GameInfo game_info;
		Coder::crossNotice(id, getCrossNum(), game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);
		return GE_TIMEOUT;
	}
}

int NvWuShen::YingLingZhaoHuan(CONTEXT_TIMELINE_2_HIT *con)
{
	int ret;
	if(con->attack.srcID != id || getEnergy() == 0) {
		return GE_SUCCESS;
	}
	CommandRequest cmd_req;
	Coder::askForSkill(id, YING_LING_ZHAO_HUAN, cmd_req);

	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			if(respond->args(0) == 1){
				network::SkillMsg skill;
				GameInfo game_info;
				Coder::skillNotice(id, con->attack.dstID, YING_LING_ZHAO_HUAN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				if(crystal >0) {
					setCrystal(--crystal);
				}
				else {
					setGem(--gem);
				}
				Coder::energyNotice(id, gem, crystal, game_info);
				tap = true;
				engine->sendMessage(-1, MSG_GAME, game_info);
				con->harm.point += 1;
				if(respond->args(1)==1){
					int cardID = respond->card_ids(0);
					CardEntity* card = getCardByID(cardID);
					if( card->getType() == TYPE_MAGIC ){
						con->harm.point += 1;
						CardMsg show_card;
						Coder::showCardNotice(id, 1, cardID, show_card);
						engine->sendMessage(-1, MSG_CARD, show_card);
						engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, YING_LING_ZHAO_HUAN, true);
					}
					else {
						return GE_INVALID_CARDID;
					}
				}
				skill.Clear();
				Coder::skillNotice(id, id, HE_PING_XING_ZHE, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				game_info.Clear();
				Coder::tapNotice(id, tap, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				if(respond->args(1)==1)
					return GE_URGENT;
				else
					return GE_SUCCESS;
			}
			//取消直接返回
			return GE_SUCCESS;
		}
		return GE_SUCCESS;
	}
	else {
		return GE_TIMEOUT;
	}
}