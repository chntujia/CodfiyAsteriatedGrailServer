#include "ShengGong.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"



bool ShengGong::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case SHENG_XIE_MISS:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_2_MISS, SHENG_XIE_MISS, respond);
			return true;
		case SHENG_LIU_XING_ATK:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_1, SHENG_LIU_XING_ATK, respond);
			return true;
		case ZI_DONG_TIAN_CHONG:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TURN_END, ZI_DONG_TIAN_CHONG, respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

int ShengGong::p_before_turn_begin(int &step, int currentPlayerID)
{
	noTianChong = false;
	return GE_SUCCESS;
}

int ShengGong::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	//天枪主动攻击可发动
	if(con->attack.srcID != id || !con->attack.isActive){
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	if(step == STEP_INIT)
	{
		step = TIAN_ZHI_GONG;
	}
	if(step == TIAN_ZHI_GONG)
	{
		ret = TianZhiGong(con);
		if(toNextStep(ret)){
			step = SHENG_LIU_XING_ATK;
		}
	}
	if(step == SHENG_LIU_XING_ATK)
	{
		ret = ShengLiuXingAtk(con);
		if (toNextStep(ret)) {
			step = STEP_DONE;
		}
	}
	return ret;
}

int ShengGong::TianZhiGong(CONTEXT_TIMELINE_1 *con)
{
	int pro = getCardByID(con->attack.cardID)->getProperty();
	if (pro != PROPERTY_HOLY)
		con->harm.point--;
	return GE_SUCCESS;
}

int ShengGong::ShengLiuXingAtk(CONTEXT_TIMELINE_1 *con)
{

	if (!tap || ((crossNum == 0) && (token[0] == 0)))
		return GE_SUCCESS;
	int ret;
	CommandRequest cmd_req;
	Coder::askForSkill(id, SHENG_LIU_XING_ATK, cmd_req);
	GameInfo update_info;
	//有限等待，由UserTask调用tryNotify唤醒
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			//发动
			if(respond->args(0)>0)
			{
				
			if (respond->args(0) == 1) {

				subCrossNum(1);
				Coder::crossNotice(id, getCrossNum(), update_info);
				
			}
			else if (respond->args(0) == 2) {
				setToken(0,token[0]-1);
				Coder::tokenNotice(id, 0, token[0], update_info);
			}
			int dstID = respond->dst_ids(0);
			PlayerEntity* player = engine->getPlayerEntity(dstID);
			player->addCrossNum(1);
			Coder::crossNotice(dstID, player->getCrossNum(), update_info);
			engine->sendMessage(-1, MSG_GAME, update_info);

			network::SkillMsg skill;
			Coder::skillNotice(id, dstID, SHENG_LIU_XING_ATK, skill);
			engine->sendMessage(-1, MSG_SKILL, skill);
			}
			return GE_SUCCESS;
		}
		return ret;
	}
	return GE_SUCCESS;
}

int ShengGong::p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con)
{
	int ret = GE_INVALID_STEP;
	if ( con->srcID != id ) {
		return GE_SUCCESS;
	}
	if (step == STEP_INIT)
	{
		step = SHENG_XIE_MISS;
	}
	if (step == SHENG_XIE_MISS)
	{
		ret = ShengXieMiss(con);
		if (toNextStep(ret) || ret == GE_URGENT ) {
			step = STEP_DONE;
		}
	}
	return ret;
}

int ShengGong::ShengXieMiss(CONTEXT_TIMELINE_2_MISS *con)
{
	int ret;
	vector<int> cards;
	CommandRequest cmd_req;
	Coder::askForSkill(id, SHENG_XIE_MISS, cmd_req);
	GameInfo update_info;
	//有限等待，由UserTask调用tryNotify唤醒
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;
			//发动
			if (respond->args(0) == 1) {
				subCrossNum(respond->args(1));
				Coder::crossNotice(id, getCrossNum(), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
				HARM qipai;
				qipai.cause = SHENG_XIE_MISS;
				qipai.point = respond->args(1);
				qipai.srcID = id;
				qipai.type = HARM_NONE;
				engine->pushGameState(new StateRequestHand(respond->dst_ids(0), qipai, -1, DECK_DISCARD, false, false));
				network::SkillMsg skill;
				Coder::skillNotice(id, respond->dst_ids(0), SHENG_XIE_MISS, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				return GE_URGENT;
			}
			else if (respond->args(0) == 2) {
				subCrossNum(respond->args(1));
				Coder::crossNotice(id, getCrossNum(), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
				HARM  harm;
				harm.srcID = id;
				harm.type = HARM_NONE;
				harm.point = respond->args(1);  //摸牌数量
				harm.cause = SHENG_XIE_MISS;
				engine->setStateMoveCardsToHand(-1, DECK_PILE, respond->dst_ids(0), DECK_HAND, respond->args(1), cards, harm, false);

				network::SkillMsg skill;
				Coder::skillNotice(id, respond->dst_ids(0), SHENG_XIE_MISS, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				return GE_URGENT;

			}
			return GE_SUCCESS;
		}
		return ret;
	}
	return GE_SUCCESS;
}

int ShengGong::ShengHuangMoShi(int &step, Action* action)
{

	//宣告技能
	network::SkillMsg skill;
	Coder::skillNotice(id, id, SHENG_HUANG_MO_SHI, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	GameInfo update_info;
	if (action->args(0) == 1)
	{
		subCrossNum(2);
		Coder::crossNotice(id, getCrossNum(), update_info);
			}
	else
	{
		setToken(0, token[0] - 2);
		Coder::tokenNotice(id, 0, token[0], update_info);
	}
	tap = true;
	Coder::tapNotice(id, true, update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	addAction(ACTION_MAGIC, SHENG_HUANG_MO_SHI);

	return GE_SUCCESS;;
}

int ShengGong::ShengGuangBaoLie(int &step, Action* action)
{

	//宣告技能
	network::SkillMsg skill;
	Coder::skillNotice(id, id, SHENG_GUANG_BAO_LIE, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	
	int howmany = action->args(1);
	setToken(0, token[0] + 1);
	subCrossNum(howmany);

	GameInfo update_info;
	Coder::tokenNotice(id, 0, token[0], update_info);
	Coder::crossNotice(id, getCrossNum() ,  update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);

	HARM BaoLie;
	BaoLie.cause = SHENG_GUANG_BAO_LIE;
	BaoLie.point = 2;
	BaoLie.srcID = id;
	BaoLie.type = HARM_ATTACK;
	
	PlayerEntity* dst;
	for (int i = 0; i < action->dst_ids_size(); ++i)
	{
		dst = engine->getPlayerEntity(action->dst_ids(i));
		if (dst->getCrossNum() > 0)
			BaoLie.point++;
	}
	PlayerEntity* start = getPre();
	PlayerEntity* it = start;
	do {
		for (int i = 0; i < action->dst_ids_size(); ++i)
		{
			if (it->getID()== action->dst_ids(i))
				engine->setStateTimeline3(it->getID(), BaoLie);
		}
		it = it->getPre();
	} while (it != start);
	vector<int> cards;
	if (action->args(0) == 1) {
		
		int i;
		for (i = 0; i < howmany; ++i)
			cards.push_back(action->card_ids(i));
		engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, howmany, cards, id, SHENG_GUANG_BAO_LIE, false);
	}
	else {
		HARM  harm;
		harm.srcID = id;
		harm.type = HARM_NONE;
		harm.point = howmany;  //摸牌数量
		harm.cause = SHENG_GUANG_BAO_LIE;
		engine->setStateMoveCardsToHand(-1, DECK_PILE, id, DECK_HAND, howmany, cards, harm, false);
	}
	return GE_URGENT;;
}


int ShengGong::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT *con)
{
	int ret = GE_INVALID_STEP;
	if (con->attack.srcID != id || !con->attack.isActive ) {
		return GE_SUCCESS;
	}
	if(step == STEP_INIT)
	{
		step = SHENG_LIU_XING_HIT;
	}
	if(step == SHENG_LIU_XING_HIT)
	{
		ret = ShengLiuXingHit(con);
		if(toNextStep(ret)){
			step = SHENG_JI;
		}
	}
	return ret;
}

int ShengGong::ShengLiuXingHit(CONTEXT_TIMELINE_2_HIT *con)
{
	int pro = getCardByID(con->attack.cardID)->getProperty();
	if (pro == PROPERTY_HOLY){
		
		network::SkillMsg skill;
		Coder::skillNotice(id, id, SHENG_LIU_XING_HIT, skill);
		engine->sendMessage(-1, MSG_SKILL, skill);

		setToken(0, token[0] + 1);
		GameInfo update_info;
		Coder::tokenNotice(id, 0, token[0], update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
		
	}
	return GE_SUCCESS;
}
int ShengGong::v_magic_skill(Action *action)
{
	return GE_SUCCESS;
}

int ShengGong::p_magic_skill(int &step, Action* action)
{
	int ret;
	switch(action->action_id())
	{
	case SHENG_XIE_JU_BAO:
		ret = ShengXieJuBao(step, action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case SHENG_HUANG_MO_SHI:
		ret = ShengHuangMoShi(step, action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}		
		break;
	case SHENG_GUANG_BAO_LIE:
		ret = ShengGuangBaoLie(step, action);
		if (GE_URGENT == ret){
			step = STEP_DONE;
		}		
		break;
	case HUI_GUANG_PAO:
		ret = HuiGuangPao(step, action);
		if (GE_URGENT == ret) {
			step = STEP_DONE;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int ShengGong::ShengXieJuBao(int &step, Action* action)
{
	int actionID = action->action_id();
	int virtualCardID = action->card_ids(2);
	int dstID = action->dst_ids(0);

	//宣告技能
	network::SkillMsg skill;
	Coder::skillNotice(id, dstID, actionID, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	vector<int> cardIDs(2);
	cardIDs[0] = action->card_ids(0);
	cardIDs[1] = action->card_ids(1);
	engine->setStateTimeline1(virtualCardID, dstID, id, true);
	engine->setStateUseCard(virtualCardID, dstID, id, false, false);
	//所有移牌操作都要用setStateMoveXXXX，ToHand的话要填好HARM，就算不是伤害
	CardMsg show_card;
	Coder::showCardNotice(id, 2, cardIDs, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD,2, cardIDs, id, actionID, true);
	//插入了新状态，请return GE_URGENT
	return GE_URGENT;
}

int ShengGong::p_before_special(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	if (currentPlayerID != id) {
		return GE_SUCCESS;
	}
	noTianChong = true;
	step = SHENG_LIU_XING_DAN;
	ret = ShengLiuXingDan(currentPlayerID);
	if (toNextStep(ret)) {
		step = STEP_DONE;
	}
	return ret;
}

int ShengGong::ShengLiuXingDan(int playerID)
{
	if (!tap || id != playerID) {
		return GE_SUCCESS;
	}
	tap = false;
	addCrossNum(1);
	GameInfo game_info;
	Coder::tapNotice(id, tap, game_info);
	Coder::crossNotice(id, getCrossNum(), game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
	return GE_SUCCESS;
}
int ShengGong::p_turn_end(int &step, int playerID)
{
	if (playerID != id || noTianChong || getEnergy()<1)
		return GE_SUCCESS;
	step = ZI_DONG_TIAN_CHONG;
	int ret = ZiDongTianChong();
	if (toNextStep(ret) || ret == GE_URGENT) {
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}
int ShengGong::ZiDongTianChong()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, ZI_DONG_TIAN_CHONG, cmd_req);
	GameInfo update_info;
	int ret;
	//有限等待，由UserTask调用tryNotify唤醒
	if (engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*)reply;

			network::SkillMsg skill;
			Coder::skillNotice(id, id, ZI_DONG_TIAN_CHONG, skill);
			engine->sendMessage(-1, MSG_SKILL, skill);
			//发动
				if (respond->args(0) == 1) {
					setGem(--gem);
					setCrystal(++crystal);
					setToken(0, token[0] + 2);
					Coder::tokenNotice(id, 0, token[0], update_info);
					Coder::energyNotice(id, gem, crystal, update_info);
					engine->sendMessage(-1, MSG_GAME, update_info);
				}
				else if (respond->args(0) == 2) {
					setGem(--gem);
					setCrystal(++crystal);
					addCrossNum(2);
					Coder::crossNotice(id, getCrossNum(), update_info);
					Coder::energyNotice(id, gem, crystal, update_info);
					engine->sendMessage(-1, MSG_GAME, update_info);
				}
				else if (respond->args(0) == 3) {
					if (crystal > 0)
						setCrystal(--crystal);
					else
						setGem(--gem);
					setToken(0, token[0] + 1);
					Coder::energyNotice(id, gem, crystal, update_info);
					Coder::tokenNotice(id, 0, token[0], update_info);
					engine->sendMessage(-1, MSG_GAME, update_info);
				}
				else if (respond->args(0) == 4) {
					if (crystal > 0)
						setCrystal(--crystal);
					else
						setGem(--gem);
					addCrossNum(1);
					Coder::energyNotice(id, gem, crystal, update_info);
					Coder::crossNotice(id, getCrossNum(), update_info);
					engine->sendMessage(-1, MSG_GAME, update_info);
				}			
			return GE_SUCCESS;
		}
		return ret;
	}
	else
	{
		if (crystal > 0) {
			setCrystal(--crystal);
			setToken(0, token[0] + 1);
			Coder::energyNotice(id, gem, crystal, update_info);
			Coder::tokenNotice(id, 0, token[0], update_info);
			engine->sendMessage(-1, MSG_GAME, update_info);
		}
		else {
			setGem(--gem);
			setCrystal(++crystal);
			setToken(0, token[0] + 2);
			Coder::tokenNotice(id, 0, token[0], update_info);
			Coder::energyNotice(id, gem, crystal, update_info);
			engine->sendMessage(-1, MSG_GAME, update_info);
		}
	}
	return GE_SUCCESS;
}
int ShengGong::HuiGuangPao(int &step, Action* action)
{
	network::SkillMsg skill;
	Coder::skillNotice(id, id, HUI_GUANG_PAO, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	list<int> dstIDs;
	PlayerEntity * dstPlayer = engine->getPlayerEntity(id);
	do
	{
		dstIDs.push_back(dstPlayer->getID());
		dstPlayer = dstPlayer->getPost();
	} while (dstPlayer->getID() != id);
	list<int>::iterator it;
	dstIDs.reverse();
	for (it = dstIDs.begin(); it != dstIDs.end(); it++)
	{
		PlayerEntity * dst = engine->getPlayerEntity(*it);
		HARM tiaozheng;
		tiaozheng.cause = HUI_GUANG_PAO;
		tiaozheng.srcID = id;
		tiaozheng.type = HARM_NONE;
		int num = dst->getHandCardNum() - 4;
		int howmany = abs(num);
		tiaozheng.point = howmany;
		//补手牌，待补充
		if (num > 0)
		{			
			engine->pushGameState(new StateRequestHand(*it, tiaozheng, -1, DECK_DISCARD, false, false));
		}
		else if (num < 0)
		{
			vector<int> cards;
			engine->setStateMoveCardsToHand(-1, DECK_PILE, *it, DECK_HAND, -num, cards, tiaozheng, false);
		}
	}
	TeamArea *m_teamArea = engine->getTeamArea();
	int myMorale = m_teamArea->getMorale(color);
	int enColor = color == RED ? BLUE : RED;
	int enMorale = m_teamArea->getMorale(enColor);
	int howmany = abs(myMorale - enMorale);
	m_teamArea->setCup(color, m_teamArea->getCup(color) + 1);
	GameInfo cup_info;
	GameInfo game_info;
	if(action->args(0)==1)
	{
		m_teamArea->setMorale(color, enMorale);
		if (color == RED)
		{
			game_info.set_red_morale(enMorale);
			cup_info.set_red_grail(m_teamArea->getCup(color));
		}
		else
		{
			game_info.set_blue_morale(enMorale);
			cup_info.set_blue_grail(m_teamArea->getCup(color));
		}
	}
	else
	{
		m_teamArea->setMorale(enColor, myMorale);
		if (color == RED)
		{
			game_info.set_blue_morale(myMorale);
			cup_info.set_red_grail(m_teamArea->getCup(color));
		}
		else
		{
			game_info.set_red_morale(myMorale);
			cup_info.set_blue_grail(m_teamArea->getCup(color));
		}		
	}
	engine->sendMessage(-1, MSG_GAME, game_info);
	engine->sendMessage(-1, MSG_GAME, cup_info);

	GameInfo update_info;
	setToken(0, token[0] - howmany - 5);
	Coder::tokenNotice(id, 0, token[0], update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
	return GE_URGENT;
}