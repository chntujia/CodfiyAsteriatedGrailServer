#include "Honglian.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"


int HongLian::p_before_turn_begin(int &step, int currentPlayerID)
{
	used_XingHongShengYue = false;
	return GE_SUCCESS;
}

int HongLian::p_boot(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	step = XUE_XING_DAO_YAN;
	//非红莲回合||没治疗
	if (currentPlayerID != id || crossNum == 0)
		return GE_SUCCESS;
	ret = XueXingDaoYan();
	step = STEP_DONE;
	return ret;
}

int HongLian::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	//主动攻击发动猩红圣约，回合限定
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	if(step == STEP_INIT)
	{
		step = XING_HONG_SHENG_YUE;
	}
	if(step == XING_HONG_SHENG_YUE)
	{
		ret = XingHongShengYue(con);
		if(toNextStep(ret)){
			step = STEP_DONE;
		}
	}
	return ret;
}

int HongLian::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con)
{
	int ret = GE_INVALID_STEP;
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	if(step == STEP_INIT){
		step = SHA_LU_SHENG_YAN;
	}
	if(step == SHA_LU_SHENG_YAN)
	{
		ret = ShaLuShengYan(con);
		if(toNextStep(ret) || ret == GE_URGENT){
			step = STEP_DONE;
		}
	}
	return ret;
}

int HongLian::p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con)
{
	int ret = GE_INVALID_STEP;
	step = XING_HONG_XIN_YANG;
	int srcID = con->dstID;
	//伤害目标为红莲||伤害来源为红莲||有治疗
	if (con->dstID == id && con->harm.srcID != id && con->crossAvailable>0)
	{
		ret = XingHongXinYang(con);
	}
	else
	{
		ret = GE_SUCCESS;
	}
	step = STEP_DONE;
	return ret;
}

int HongLian::p_after_attack(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	if(playerID != id || !tap || getEnergy() < 1 ){
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	step = JIE_JIAO_JIE_ZAO;
	ret = JieJiaoJieZao(playerID);
	if(toNextStep(ret)|| ret == GE_URGENT){
		step = STEP_DONE;;
	}			
	return ret;
}

int HongLian::p_after_magic(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	if(playerID != id || !tap || getEnergy() < 1 ){
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	step = JIE_JIAO_JIE_ZAO;
	ret = JieJiaoJieZao(playerID);
	if(toNextStep(ret)|| ret == GE_URGENT){
		step = STEP_DONE;
	}			
	return ret;
}

int HongLian::p_magic_skill(int &step, Action* action)
{
	int ret;
	switch(action->action_id())
	{
	case XING_HONG_SHI_ZI:
		ret = XingHongShiZi(step, action);
		if(toNextStep(ret) || GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;

	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int HongLian::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int cardID;
	int playerID = action->src_id();
	//int dstID;
	CardEntity* card;
	//PlayerEntity* dst;
	

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}
	switch(actionID)
	{
	case XING_HONG_SHI_ZI:
		//未选中两张||没能量||没血印
		if(action->card_ids_size() != 2 || getEnergy() < 1 || 0 == token[0])
		{
			return GE_INVALID_ACTION;
		}
		//非法术牌
		for(int i = 0;i<action->card_ids_size();i++)
		{
			cardID = action->card_ids(i);
			card = getCardByID(cardID);
			if(TYPE_MAGIC != card->getType() || GE_SUCCESS != checkOneHandCard(cardID))
			{
				return GE_INVALID_ACTION;
			}
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

bool HongLian::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case XING_HONG_SHENG_YUE:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_1, XING_HONG_SHENG_YUE, respond);
			return true;
		case XUE_XING_DAO_YAN:
			session->tryNotify(id, STATE_BOOT, XUE_XING_DAO_YAN, respond);
			return true;
		case SHA_LU_SHENG_YAN:
			session->tryNotify(id, STATE_TIMELINE_2_HIT, SHA_LU_SHENG_YAN, respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}
int HongLian::p_additional_action(int chosen)
{
	if(chosen == JIE_JIAO_JIE_ZAO)
	{
		network::SkillMsg skill;
		Coder::skillNotice(id, id, JIE_JIAO_JIE_ZAO, skill);
		engine->sendMessage(-1, MSG_SKILL, skill);
		if(crystal>0){
			setCrystal(--crystal);
		}
		else{
			setGem(--gem);
		}
		GameInfo update_info;
		Coder::energyNotice(id, gem, crystal, update_info);
		tap = false;
		Coder::tapNotice(id, false, update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
	}
	return PlayerEntity::p_additional_action(chosen);
}
int HongLian::p_before_lose_morale(int &step, CONTEXT_LOSE_MORALE *con)
{
	//不在热血沸腾||摸牌导致掉士气
	if ( !tap || HARM_NONE == con->harm.type || con->howMany <= 0 )
		return GE_SUCCESS;

	int ret = ReXueFeiTengLoseNoMorale(con);
	if(toNextStep(ret) || ret == GE_URGENT)
	{
		step = STEP_DONE;
	}
	return ret;
	
}

int HongLian::p_true_lose_morale(int &step, CONTEXT_LOSE_MORALE *con)
{
	//伤害目标非红莲||热血沸腾（应该不可能）||摸牌导致掉士气
	if (con->dstID != id || tap || HARM_NONE == con->harm.type)
		return GE_SUCCESS;

	step = RE_XUE_FEI_TENG;

	int ret = ReXueFeiTeng(con);
	if(toNextStep(ret)||ret == GE_URGENT)
	{
		step = STEP_DONE;
	}
	return ret;
}

int HongLian::p_turn_end(int &step, int playerID)
{
	if (playerID != id || !tap)
		return GE_SUCCESS;

	int ret = ReXueFeiTengReset();
	if(toNextStep(ret) || ret == GE_URGENT){
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int HongLian::XingHongShengYue(CONTEXT_TIMELINE_1 *con)
{
	int ret;
	int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	PlayerEntity * srcPlayer = engine->getPlayerEntity(srcID);
	//非红莲攻击||非主动攻击||满治疗||回合限定发动过
	if(srcID != id || !con->attack.isActive || engine->getPlayerEntity(id)->getCrossNum() >=4 || used_XingHongShengYue)
	{
		return GE_SUCCESS;
	}

	CommandRequest cmd_req;
	Coder::askForSkill(id, XING_HONG_SHENG_YUE, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(srcID, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0) == 1){
				engine->getPlayerEntity(id)->addCrossNum(1);
				GameInfo update_info;
				Coder::crossNotice(srcID, srcPlayer->getCrossNum(), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
				network::SkillMsg skill;
				Coder::skillNotice(id, dstID, XING_HONG_SHENG_YUE, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				used_XingHongShengYue = true;
			}
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int HongLian::XingHongXinYang(CONTEXT_TIMELINE_4 *con)
{
	if (con->dstID == id && con->harm.srcID != id && 0 != con->crossAvailable)
	{
		con->crossAvailable = 0;
		network::SkillMsg skill;
		Coder::skillNotice(id, id, XING_HONG_XIN_YANG, skill);
		engine->sendMessage(-1, MSG_SKILL, skill);
	}
	return GE_SUCCESS;
}

int HongLian::XueXingDaoYan()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, XUE_XING_DAO_YAN, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			if(respond->args(0) == 0){
				return ret;
			}
			//发动
			list<int> dst;
			//list<int> cross;
			int crossSumToSend = 0;
			for(int i = 0; i < respond->dst_ids_size(); i++)
			{
				dst.push_back(respond->dst_ids(i));
				//cross.push_back(respond->args(i));
				crossSumToSend += respond->args(i);
			}
			SkillMsg skill_msg;
			Coder::skillNotice(id, dst, XUE_XING_DAO_YAN, skill_msg);
			engine->sendMessage(-1, MSG_SKILL, skill_msg);

			GameInfo update_info;
			if(crossNum < crossSumToSend)
				return GE_INVALID_ARGUMENT;
			if(crossNum > 0 && crossSumToSend > 0)
			{
				//heal minus
				engine->getPlayerEntity(id)->subCrossNum(crossSumToSend);
				Coder::crossNotice(id, engine->getPlayerEntity(id)->getCrossNum(), update_info);
				//engine->sendMessage(-1, MSG_GAME, update_info);
				//heal plus
				for(int i = 0; i < respond->dst_ids_size(); i++)
				{
					int dstID = respond->dst_ids(i);
					int crossToSend = respond->args(i);
			
					engine->getPlayerEntity(dstID)->addCrossNum(crossToSend);
					Coder::crossNotice(dstID, engine->getPlayerEntity(dstID)->getCrossNum(), update_info);
					//engine->sendMessage(-1, MSG_GAME, update_info);
				}
				//token notify
				if(token[0]<2){
					setToken(0,token[0] + 1);
				}
				Coder::tokenNotice(id, 0, getToken(0), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);

				HARM selfHarm;
				selfHarm.cause = XUE_XING_DAO_YAN;
				selfHarm.point = crossSumToSend;
				selfHarm.srcID = id;
				selfHarm.type = HARM_MAGIC;


				engine->setStateTimeline3(id, selfHarm);
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

int HongLian::ShaLuShengYan(CONTEXT_TIMELINE_2_HIT *con)
{
	int ret = GE_SUCCESS;
	int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	GameInfo update_info;
	if(srcID != id || !con->attack.isActive || 0 == token[0]){
		return GE_SUCCESS;
	}
	//满足发动条件，询问客户端是否发动
	CommandRequest cmd_req;
	Coder::askForSkill(id, SHA_LU_SHENG_YAN, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(srcID, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0) == 1){
				network::SkillMsg skill;
				Coder::skillNotice(id, dstID, SHA_LU_SHENG_YAN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				con->harm.point += 2;
				setToken(0,--token[0]);
				Coder::tokenNotice(id, 0, getToken(0), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);

				HARM selfHarm;
				selfHarm.cause = SHA_LU_SHENG_YAN;
				selfHarm.point = 4;
				selfHarm.srcID = id;
				selfHarm.type = HARM_MAGIC;


				engine->setStateTimeline3(id, selfHarm);
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

int HongLian::ReXueFeiTeng(CONTEXT_LOSE_MORALE *con)
{
	if(con->dstID == id && !tap && con->howMany > 0 && con->harm.type!=HARM_NONE)
	{
		tap = true;
		GameInfo game_info;
		Coder::tapNotice(id, true, game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);
	}
	return GE_SUCCESS;
}

int HongLian::ReXueFeiTengLoseNoMorale(CONTEXT_LOSE_MORALE *con)
{
	if(con->dstID == id && tap && con->howMany > 0 && con->harm.type!=HARM_NONE)
	{
		con->howMany = 0;
		GameInfo game_info;
	}
	return GE_SUCCESS;
}

int HongLian::ReXueFeiTengReset()
{
	GameInfo game_info;
	tap = false;
	Coder::tapNotice(id, false, game_info);
	//engine->sendMessage(-1, MSG_GAME, game_info);
	addCrossNum(2);
	Coder::crossNotice(id,getCrossNum(),game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
	return GE_SUCCESS;
}

int HongLian::JieJiaoJieZao(int playerID)
{
	if(playerID != id || 0 == getEnergy() || !tap){
		return GE_SUCCESS;
	}
	addAction(ACTION_ATTACK_MAGIC, JIE_JIAO_JIE_ZAO);
	return GE_SUCCESS;
}

int HongLian::XingHongShiZi(int &step, Action* action)
{
	GameInfo game_info;
	vector<int> cards;
	int i;

	for (i = 0; i < action->card_ids_size(); ++i)
		cards.push_back(action->card_ids(i));
	
		SkillMsg skill_msg;
		Coder::skillNotice(id, action->dst_ids(0), XING_HONG_SHI_ZI, skill_msg);
		engine->sendMessage(-1, MSG_SKILL, skill_msg);
		
		CardMsg show_card;
		Coder::showCardNotice(id, 2, cards, show_card);
		engine->sendMessage(-1, MSG_CARD, show_card);

		if(crystal>0)
		{
			setCrystal(--crystal);
		}
		else
		{
			setGem(--gem);
		}
		Coder::energyNotice(id, gem, crystal, game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);
		setToken(0,--token[0]);
		Coder::tokenNotice(id, 0, getToken(0), game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);
		
		HARM selfHarm;
		selfHarm.cause = XING_HONG_SHI_ZI;
		selfHarm.point = 4;
		selfHarm.srcID = id;
		selfHarm.type = HARM_MAGIC;

		HARM harm;
		harm.cause = XING_HONG_SHI_ZI;
		harm.point = 3;
		harm.srcID = id;
		harm.type = HARM_MAGIC;


		engine->setStateTimeline3(action->dst_ids(0), harm);
		engine->setStateTimeline3(id, selfHarm);
		engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cards.size(), cards, id, XING_HONG_SHI_ZI, true);
		return GE_URGENT;
}