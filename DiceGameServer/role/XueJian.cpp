#include "XueJian.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool XueJian::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case CHI_SE_YI_SHAN:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_AFTER_ATTACK, CHI_SE_YI_SHAN, respond);
			return true;
			break;
		case XUE_QI_PING_ZHANG:
			session->tryNotify(id, STATE_TIMELINE_3, XUE_QI_PING_ZHANG, respond);
			return true;
			break;
		case XUE_QIANG_WEI_BOOT:
			session->tryNotify(id, STATE_BOOT, XUE_QIANG_WEI_BOOT, respond);
			return true;
			break;
		}
	}
	//没匹配则返回false
	return false;
}

int XueJian::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con)
{
	int ret = GE_INVALID_STEP;
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	ret = XueSeJingJi(con);
	if (toNextStep(ret) || ret == GE_URGENT)
	{
		step = STEP_DONE;
	}
	return ret;
}


int XueJian::XueSeJingJi(CONTEXT_TIMELINE_2_HIT *con)
{
	int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	if(srcID != id){
		return GE_SUCCESS;
	}
	SkillMsg skill;
	Coder::skillNotice(id, id, XUE_SE_JING_JI, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	setToken(0, token[0] + 1);
	GameInfo update_info;
	Coder::tokenNotice(id, 0, getToken(0), update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	return GE_SUCCESS;
}

int XueJian::p_after_attack(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	if (playerID != id || token[0] < 1) {
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	step = CHI_SE_YI_SHAN;
	ret = ChiSeYiShan(playerID);
	if (toNextStep(ret) || ret == GE_URGENT) {
		step = STEP_DONE;;
	}
	return ret;
}

int XueJian::ChiSeYiShan(int playerID)
{
	if (playerID != id || token[0]<1) {
		return GE_SUCCESS;
	}
	addAction(ACTION_ATTACK, CHI_SE_YI_SHAN);
	return GE_SUCCESS;
}

int XueJian::p_additional_action(int chosen)
{
	if (chosen == CHI_SE_YI_SHAN)
	{
		PlayerEntity::p_additional_action(chosen);

		network::SkillMsg skill;
		Coder::skillNotice(id, id, CHI_SE_YI_SHAN, skill);
		engine->sendMessage(-1, MSG_SKILL, skill);
		setToken(0, token[0] - 1);
		GameInfo update_info;
		Coder::tokenNotice(id, 0, getToken(0), update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
		HARM selfHarm;
		selfHarm.cause = CHI_SE_YI_SHAN;
		selfHarm.point = 2;
		selfHarm.srcID = id;
		selfHarm.type = HARM_MAGIC;
		engine->setStateTimeline3(id, selfHarm);
		return GE_URGENT;
	}
	else
	return PlayerEntity::p_additional_action(chosen);
}

int XueJian::p_magic_skill(int &step, Action* action)
{
	int ret;
	switch (action->action_id())
	{
	case XUE_RAN_QIANG_WEI:
		ret = XueRanQiangWei(step, action);
		if (toNextStep(ret) || GE_URGENT == ret) {
			step = STEP_DONE;
		}
		break;

	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int XueJian::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
//	int cardID;
	int playerID = action->src_id();
	//int dstID;
	//CardEntity* card;
	//PlayerEntity* dst;
	if (playerID != id) {
		return GE_INVALID_PLAYERID;
	}
	switch (actionID)
	{
	case XUE_RAN_QIANG_WEI:
		//没指示物
		if (token[0]<2)
		{
			return GE_INVALID_ACTION;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}


int XueJian::XueRanQiangWei(int &step, Action* action)
{

	SkillMsg skill;
	Coder::skillNotice(id, id, XUE_RAN_QIANG_WEI, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);

	setToken(0, token[0] - 2);
	GameInfo update_info;
	Coder::tokenNotice(id, 0, getToken(0), update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	//移除目标2治疗
	int dstID1 = action->dst_ids(0);
	engine->getPlayerEntity(dstID1)->subCrossNum(2);
	GameInfo update_info1;
	Coder::crossNotice(dstID1, engine->getPlayerEntity(dstID1)->getCrossNum(), update_info1);
	engine->sendMessage(-1, MSG_GAME, update_info1);
	//我方1水晶变宝石
	int dstID2 = action->dst_ids(1);
	int dstCrystal = engine->getPlayerEntity(dstID2)->getCrystal();
	int dstGem = engine->getPlayerEntity(dstID2)->getGem();
	if(dstCrystal>0)
	{
		engine->getPlayerEntity(dstID2)->setCrystal(dstCrystal - 1);
		engine->getPlayerEntity(dstID2)->setGem(dstGem + 1);

		GameInfo update_info3;
		Coder::energyNotice(dstID2, engine->getPlayerEntity(dstID2)->getGem(), engine->getPlayerEntity(dstID2)->getCrystal(), update_info3);
		engine->sendMessage(-1, MSG_GAME, update_info3);
	}

	if (tap)
	{
		list<int> dstIDs;
		PlayerEntity * dstPlayer = engine->getPlayerEntity(id);
		do
		{	dstIDs.push_back(dstPlayer->getID());
			dstPlayer = dstPlayer->getPost();
		}while (dstPlayer->getID() != id);
		HARM harm;
		harm.type = HARM_MAGIC;
		harm.point = 1;
		harm.srcID = id;
		harm.cause = XUE_RAN_QIANG_WEI;
		list<int>::iterator it;
		dstIDs.reverse();
		for (it = dstIDs.begin(); it != dstIDs.end(); it++)
		{
			engine->setStateTimeline3(*it, harm);
		}
		return GE_URGENT;
	}
	return GE_SUCCESS;
}

int XueJian::p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con)
{
	if (con->dstID == id && con->harm.type==HARM_MAGIC && token[0]>0)
	{
		// 血气屏障
		step = XUE_QI_PING_ZHANG;
		int ret = XueQiPingZhang(con);
		if (toNextStep(ret) || ret == GE_URGENT) {
			//全部走完后，请把step设成STEP_DONE
			step = STEP_DONE;
		}
		return ret;
	}
	return GE_SUCCESS;
}

int XueJian::XueQiPingZhang(CONTEXT_TIMELINE_3 *con)
{
	CommandRequest cmd_req;
	int ret;
	Coder::askForSkill(id, XUE_QI_PING_ZHANG, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			if(respond->args(0)==1)
			{
				con->harm.point--;

				SkillMsg skill;
				Coder::skillNotice(id, id, XUE_QI_PING_ZHANG, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				setToken(0, token[0] - 1);
				GameInfo update_info;
				Coder::tokenNotice(id, 0, getToken(0), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);

				HARM harm;
				harm.type = HARM_MAGIC;
				harm.point = 1;
				harm.srcID = id;
				harm.cause = XUE_QI_PING_ZHANG;

				engine->setStateTimeline3(respond->dst_ids(0), harm);
				return GE_URGENT;
			}
			return GE_SUCCESS;
		}
		return GE_SUCCESS;
	}
	else {
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int XueJian::p_boot(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	step = XUE_QIANG_WEI_BOOT;
	//非红莲回合||没治疗
	if (currentPlayerID != id || getEnergy()==0)
		return GE_SUCCESS;
	ret = XueQiangWeiBoot();
	step = STEP_DONE;
	return ret;
}

int XueJian::XueQiangWeiBoot()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, XUE_QIANG_WEI_BOOT, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			if (respond->args(0) == 1) //宝石发动
			{
				setGem(gem - 1);
				tap = true;
				token[0] = token[0]>1 ? 4 : token[0] + 2;
				GameInfo game_info;
				Coder::energyNotice(id, gem, crystal, game_info);
				Coder::tapNotice(id, true, game_info);
				Coder::tokenNotice(id, 0,token[0], game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);

				vector<int> cardIDs;
				int cardNum = ((getHandCardNum() - 4) > 0) ? (getHandCardNum() - 4) : 0;
				for (int i = 0; i < cardNum; i++)
				{
					cardIDs.push_back(respond->card_ids(i));
				}
				if (cardNum > 0)
				{
					engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs, id, XUE_QIANG_WEI_BOOT, false);
					return GE_URGENT;
				}
			}
			else if (respond->args(0) == 2)//水晶发动
			{
				if(crystal>0)
				{
					setCrystal(crystal - 1);
				}
				else
				{
					setGem(gem - 1);
				}
				tap = true;
				setToken(0, token[0]+2);
				GameInfo game_info;
				Coder::energyNotice(id, gem,crystal,game_info);
				Coder::tapNotice(id, true, game_info);
				Coder::tokenNotice(id, 0, token[0], game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
			}
		}
		return ret;
	}
	else {
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int XueJian::p_timeline_4(int &step, CONTEXT_TIMELINE_4 *con)
{
	if (tap)
		con->crossAvailable = 0;
	return GE_SUCCESS;
}

int XueJian::p_turn_end(int &step, int playerID)
{
	if (playerID != id || !tap)
		return GE_SUCCESS;
	GameInfo game_info;
	tap = false;
	Coder::tapNotice(id, false, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
	return GE_SUCCESS;
}