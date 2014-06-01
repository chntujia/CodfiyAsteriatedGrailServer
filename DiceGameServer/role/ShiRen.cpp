#include "ShiRen.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

ShiRen::ShiRen(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color)
{

	tokenMax[0]=4;
	YueZhangDst = id;
	GameInfo update_info;
	this->addExclusiveEffect(EX_YONG_HENG_YUE_ZHANG);
	Coder::exclusiveNotice(id, this->getExclusiveEffect(), update_info);
	engine->sendMessage(-1, MSG_GAME, update_info);
};

bool ShiRen::ShiRenParse(UserTask* session, int playerID, ::google::protobuf::Message *proto)
{
	Respond* respond = (Respond*)proto;
	switch(respond->respond_id())
	{
	case JI_ANG_KUANG_XIANG_QU:
		session->tryNotify(playerID, STATE_TURN_BEGIN_SHIREN, JI_ANG_KUANG_XIANG_QU, respond);
		return true;
		break;
	case JI_ANG_KUANG_XIANG_QU_2:
		session->tryNotify(playerID, STATE_TURN_BEGIN_SHIREN, JI_ANG_KUANG_XIANG_QU_2, respond);
		return true;
		break;
	case SHENG_LI_JIAO_XIANG_SHI:
		session->tryNotify(playerID, STATE_TURN_END, SHENG_LI_JIAO_XIANG_SHI, respond);
		return true;
		break;
	case SHENG_LI_JIAO_XIANG_SHI_2:
		session->tryNotify(playerID, STATE_TURN_END, SHENG_LI_JIAO_XIANG_SHI_2, respond);
		return true;
		break;
	}
	return false;
}

bool ShiRen::cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case CHEN_LUN_XIE_ZOU_QU:
			session->tryNotify(id, STATE_HARM_END, CHEN_LUN_XIE_ZOU_QU, respond);
			return true;
			break;
		case BAO_FENG_QIAN_ZOU_QU:
			session->tryNotify(id, STATE_BEFORE_TURN_BEGIN, BAO_FENG_QIAN_ZOU_QU, respond);
			return true;
			break;
		
		case XI_WANG_FU_GE_QU:
			session->tryNotify(id, STATE_BOOT, XI_WANG_FU_GE_QU, respond);
			return true;
			break;
		
		}
	}
	//没匹配则返回false
	return false;
}

int ShiRen::p_before_turn_begin(int &step, int currentPlayerID)
{
	for(int i=0; i<6; i++)
		ChenLunNum[i] = false;
	ChenLunUsed = false;
	BaoFengUsed = false;
	YueZhangReturn = false;
	if(id != currentPlayerID)
		return GE_SUCCESS;
	step = BAO_FENG_QIAN_ZOU_QU;
	int ret = BaoFengQianZouQu();
	if(toNextStep(ret) || ret == GE_URGENT){
		//全部走完后，请把step设成STEP_DONE
		step = STEP_DONE;
	}
	return ret;
}

int ShiRen::BaoFengQianZouQu()
{
	if(getToken(0) < 1)
		return GE_SUCCESS;
	int ret;
	CommandRequest cmd_req;
	Coder::askForSkill(id, BAO_FENG_QIAN_ZOU_QU, cmd_req);
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			if(respond->args(0) == 1){
				//确认发动，宣告技能
				SkillMsg skill;
				Coder::skillNotice(id, id, BAO_FENG_QIAN_ZOU_QU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				setToken(0,token[0]-1);
				GameInfo game_info;
				Coder::tokenNotice(id, 0, token[0], game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				BaoFengUsed = true;
			}
			return GE_SUCCESS;
		}
		return ret;
	}else{
		return GE_TIMEOUT;
	}
}

int ShiRen::p_turn_begin_shiren(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	if(step == STEP_INIT)
	{
		step = JI_ANG_KUANG_XIANG_QU;
	}
	if(step == JI_ANG_KUANG_XIANG_QU)
	{
		ret = JiAngKuangXiangQu(step, currentPlayerID);
		if(ret == GE_URGENT)
			return ret;
	}
	if(step == JI_ANG_KUANG_XIANG_QU_2)
	{
		ret = JiAngKuangXiangQu2(step, currentPlayerID);
	}
	return ret;
}

int ShiRen::JiAngKuangXiangQu(int &step, int  currentPlayerID)
{
	if(YueZhangDst != currentPlayerID)
		return GE_SUCCESS;
	if(YueZhangDst == id && !BaoFengUsed)
		return GE_SUCCESS;
	int choice = 0;
	PlayerEntity *owner = engine->getPlayerEntity(currentPlayerID);
	TeamArea *team = engine->getTeamArea();
	if(team->getEnergy(owner->getColor()) > 1)
		choice+=1;
	if(YueZhangDst != id)
		choice+=2;
	if(choice <= 0)
		return GE_SUCCESS;
	CommandRequest cmd_req;
	Coder::askForSkill(currentPlayerID, JI_ANG_KUANG_XIANG_QU, cmd_req);
	Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size()-1));
	cmd->add_args(choice);
	if(engine->waitForOne(currentPlayerID, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(currentPlayerID, reply)))
		{
			Respond* respond = (Respond*) reply;

			int choice = respond->args(0);
			if(choice == 1)
			{
				SkillMsg skill;
				Coder::skillNotice(currentPlayerID, currentPlayerID, JI_ANG_KUANG_XIANG_QU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				GeYongTianFu();

				GameInfo game_info;
				PlayerEntity *self = engine->getPlayerEntity(currentPlayerID);
				TeamArea *team = engine->getTeamArea();
				int color = self->getColor();
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
				engine->sendMessage(-1, MSG_GAME, game_info);
				step = JI_ANG_KUANG_XIANG_QU_2;
				return GE_SUCCESS;
			}
			else if(choice == 2)
			{
				if(id == currentPlayerID)
					return GE_INVALID_ARGUMENT;

				SkillMsg skill;
				Coder::skillNotice(currentPlayerID, currentPlayerID, JI_ANG_KUANG_XIANG_QU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				GeYongTianFu();

				GameInfo update_info;
				PlayerEntity* dst = engine->getPlayerEntity(YueZhangDst);
				dst->removeExclusiveEffect(EX_YONG_HENG_YUE_ZHANG);
				Coder::exclusiveNotice(YueZhangDst, dst->getExclusiveEffect(), update_info);
				YueZhangReturn = true;
				this->addExclusiveEffect(EX_YONG_HENG_YUE_ZHANG);
				Coder::exclusiveNotice(id, this->getExclusiveEffect(), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
				step = JI_ANG_KUANG_XIANG_QU_2;
				return GE_SUCCESS;
			}
			else
			{
				step = STEP_DONE;
				return GE_SUCCESS;
			}
		}
		return ret;
	}
	else{
		step = STEP_DONE;
		return GE_TIMEOUT;
	}
}

int ShiRen::JiAngKuangXiangQu2(int &step, int currentPlayerID)
{
	if(YueZhangDst != currentPlayerID)
		return GE_SUCCESS;
	if(YueZhangDst == id && !BaoFengUsed)
		return GE_SUCCESS;

	CommandRequest cmd_req;
	Coder::askForSkill(currentPlayerID, JI_ANG_KUANG_XIANG_QU_2, cmd_req);
	if(engine->waitForOne(currentPlayerID, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(currentPlayerID, reply)))
		{
			Respond* respond = (Respond*) reply;

			int choice = respond->args(0);
			if(choice==1)
			{
				int dst1ID = respond->dst_ids(0);
				int dst2ID = respond->dst_ids(1);
				//填写伤害结构
				HARM harm;
				harm.cause = JI_ANG_KUANG_XIANG_QU;
				harm.point = 1;
				harm.srcID = YueZhangDst;
				harm.type = HARM_MAGIC;
				//先进后出，所以逆出牌顺序压
				PlayerEntity* start = this->getPre();
				PlayerEntity* it = start;
				do{
					if(it->getID() == dst1ID || it->getID() == dst2ID){
						engine->setStateTimeline3(it->getID(), harm);
					}
					it = it->getPre();
				}while(it != start);
				if(YueZhangReturn)
					YueZhangDst = id;
				step = STEP_DONE;
				return GE_URGENT;
			}
			else if(choice == 2)
			{
				PlayerEntity* self = engine->getPlayerEntity(currentPlayerID);
				int cardNum = (self->getHandCardNum()>2)? 2:self->getHandCardNum();
				HARM qipai;
				qipai.point = cardNum;
				qipai.srcID = YueZhangDst;
				qipai.type = HARM_NONE;
				qipai.cause = JI_ANG_KUANG_XIANG_QU;
				engine->pushGameState(new StateRequestHand(YueZhangDst, qipai, -1, DECK_DISCARD, false, false));
				if(YueZhangReturn)
					YueZhangDst = id;
				step = STEP_DONE;
				return GE_URGENT;
			}
			else
				return GE_INVALID_ARGUMENT;
		}
		return ret;
	}
	else{
		int cardNum = (getHandCardNum()>2)? 2:getHandCardNum();
		HARM qipai;
		qipai.point = cardNum;
		qipai.srcID = YueZhangDst;
		qipai.type = HARM_NONE;
		qipai.cause = JI_ANG_KUANG_XIANG_QU;
		if(cardNum != 0)
		{
			engine->pushGameState(new StateRequestHand(YueZhangDst, qipai, -1, DECK_DISCARD, false));
		}
		if(YueZhangReturn)
			YueZhangDst = id;
		step = STEP_DONE;
		return GE_TIMEOUT;
	}
}

int ShiRen::p_boot(int &step, int currentPlayerID)
{
	if(id != currentPlayerID)
		return GE_SUCCESS;
	int ret = GE_INVALID_STEP;
	step = XI_WANG_FU_GE_QU;
	ret = XiWangFuGeQu();
	if(toNextStep(ret) || ret == GE_URGENT)
	{
		step = STEP_DONE;
	}
	return ret;
}

int ShiRen::XiWangFuGeQu()
{
	if(getGem() < 1)
		return GE_SUCCESS;
	if(getHandCardNum() == 0)
		return GE_SUCCESS;
	bool allLight = true;
	list<int> handcards = this->getHandCards();
	list<int>::iterator it = handcards.begin();
	for(; it != handcards.end(); ++it){
		if(getCardByID(*it)->getElement() != ELEMENT_LIGHT)
		{
			allLight = false;
			break;
		}
	}
	if(allLight && getHandCardNum() < 4)
		return GE_SUCCESS;

	CommandRequest cmd_req;
	Coder::askForSkill(id, XI_WANG_FU_GE_QU, cmd_req);
	Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size()-1));
	cmd->add_args(YueZhangDst);
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;

			GameInfo game_info;
			int choice1 = respond->args(0);
			if(respond->args(0) == 1)
			{
				int dstID = respond->dst_ids(0);
				if(engine->getPlayerEntity(dstID)->getColor() != this->getColor())
					return GE_INVALID_ARGUMENT;
				SkillMsg skill;
				Coder::skillNotice(id, dstID, XI_WANG_FU_GE_QU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				GameInfo update_info;
				setGem(gem-1);
				Coder::energyNotice(id, gem, crystal, update_info);
				this->removeExclusiveEffect(EX_YONG_HENG_YUE_ZHANG);
				Coder::exclusiveNotice(YueZhangDst, this->getExclusiveEffect(), update_info);
				YueZhangDst = dstID;
				PlayerEntity* dst = engine->getPlayerEntity(dstID);
				dst->addExclusiveEffect(EX_YONG_HENG_YUE_ZHANG);
				Coder::exclusiveNotice(dstID, dst->getExclusiveEffect(), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
			}
			return GE_SUCCESS;
		}
	}
	else {
		return GE_TIMEOUT;
	}
}

int ShiRen::p_turn_end(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	if(step == STEP_INIT)
	{
		step = SHENG_LI_JIAO_XIANG_SHI;
	}
	if(step == SHENG_LI_JIAO_XIANG_SHI)
	{
		ret = ShengLiJiaoXiangShi(step, currentPlayerID);
		if(ret == GE_URGENT)
			return ret;
	}
	if(step == SHENG_LI_JIAO_XIANG_SHI_2)
	{
		ret = ShengLiJiaoXiangShiStone(step, currentPlayerID);
	}
	return ret;
}

int ShiRen::ShengLiJiaoXiangShi(int &step, int currentPlayerID)
{
	if(YueZhangDst != currentPlayerID)
		return GE_SUCCESS;
	if(YueZhangDst == id && !BaoFengUsed)
		return GE_SUCCESS;
	int choice = 0;
	choice+=1;
	if(YueZhangDst != id)
		choice+=2;
	CommandRequest cmd_req;
	Coder::askForSkill(currentPlayerID, SHENG_LI_JIAO_XIANG_SHI, cmd_req);
	Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size()-1));
	cmd->add_args(choice);
	if(engine->waitForOne(currentPlayerID, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(currentPlayerID, reply)))
		{
			Respond* respond = (Respond*) reply;

			GameInfo game_info;
			int choice = respond->args(0);
			if(choice == 1)
			{
				SkillMsg skill;
				Coder::skillNotice(currentPlayerID, currentPlayerID, SHENG_LI_JIAO_XIANG_SHI, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				GeYongTianFu();

				HARM harm;
				harm.cause = SHENG_LI_JIAO_XIANG_SHI;
				harm.point = 3;
				harm.srcID = YueZhangDst;
				harm.type = HARM_MAGIC;
				engine->setStateTimeline3(id,harm);
				step = SHENG_LI_JIAO_XIANG_SHI_2;
				return GE_URGENT;
			}
			else if(choice == 2)
			{
				if(id == currentPlayerID)
					return GE_SUCCESS;

				SkillMsg skill;
				Coder::skillNotice(currentPlayerID, currentPlayerID, SHENG_LI_JIAO_XIANG_SHI, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				GeYongTianFu();

				GameInfo update_info;
				PlayerEntity* dst = engine->getPlayerEntity(YueZhangDst);
				dst->removeExclusiveEffect(EX_YONG_HENG_YUE_ZHANG);
				Coder::exclusiveNotice(YueZhangDst, dst->getExclusiveEffect(), update_info);
				YueZhangReturn = true;
				this->addExclusiveEffect(EX_YONG_HENG_YUE_ZHANG);
				Coder::exclusiveNotice(id, this->getExclusiveEffect(), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
				step = SHENG_LI_JIAO_XIANG_SHI_2;
				return GE_SUCCESS;
			}
			else
			{
				step = STEP_DONE;
				return GE_SUCCESS;
			}
		}
		return ret;
	}
	else{
		step = STEP_DONE;
		return GE_TIMEOUT;
	}
}

int ShiRen::ShengLiJiaoXiangShiStone(int &step, int currentPlayerID)
{
	if(YueZhangDst != currentPlayerID)
		return GE_SUCCESS;
	if(YueZhangDst == id && !BaoFengUsed)
		return GE_SUCCESS;
	int choice = 2;
	PlayerEntity *owner = engine->getPlayerEntity(currentPlayerID);
	TeamArea *team = engine->getTeamArea();
	if(team->getEnergy(owner->getColor()) > 0)
		choice+=1;
	CommandRequest cmd_req;
	Coder::askForSkill(currentPlayerID, SHENG_LI_JIAO_XIANG_SHI_2, cmd_req);
	Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size()-1));
	cmd->add_args(choice);
	if(engine->waitForOne(currentPlayerID, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(currentPlayerID, reply)))
		{
			Respond* respond = (Respond*) reply;
			int choice = respond->args(0);
			if(choice == 1)
			{
				PlayerEntity *self = engine->getPlayerEntity(currentPlayerID);
				TeamArea *team = engine->getTeamArea();

				int color = self->getColor();

				GameInfo update_info;
				if(respond->args(1)==1){
					self->setCrystal(self->getCrystal()+1);
					team->setCrystal(color, team->getCrystal(color) - 1);
				}
				else if(respond->args(1)==2){
					self->setGem(self->getGem()+1);
					team->setGem(color, team->getGem(color) - 1);
				}
				else
				{
					choice = 2;
				}

				Coder::energyNotice(currentPlayerID, self->getGem(),self->getCrystal(), update_info);
				
				Coder::stoneNotice(color, team->getGem(color), team->getCrystal(color), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
			}
			if(choice == 2)
			{
				GameInfo update_info;
				PlayerEntity *self = engine->getPlayerEntity(currentPlayerID);

				TeamArea* team = engine->getTeamArea();
				team->setGem(color, team->getGem(color) +1);
				Coder::stoneNotice(color, team->getGem(color), team->getCrystal(color), update_info);

				self->addCrossNum(1);
				Coder::crossNotice(YueZhangDst,self->getCrossNum(), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
			}
			if(YueZhangReturn)
				YueZhangDst = id;
			step = STEP_DONE;
			return GE_SUCCESS;
		}
	}
	else
	{
		GameInfo update_info;
		PlayerEntity *self = engine->getPlayerEntity(currentPlayerID);

		TeamArea* team = engine->getTeamArea();
		team->setGem(color, team->getGem(color) +1);
		Coder::stoneNotice(color, team->getGem(color), team->getCrystal(color), update_info);

		self->addCrossNum(1);
		Coder::crossNotice(YueZhangDst,self->getCrossNum(), update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
		step = STEP_DONE;
		return GE_TIMEOUT;
	}
}

int ShiRen::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int playerID = action->src_id();

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}

	switch(actionID)
	{
	case BU_XIE_HE_XIAN:
		if(action->args(0) <=1 || action->args(0) > getToken(0) || action->args(1) >2 || action->args(1) < 1)
			return GE_INVALID_ARGUMENT;
		return GE_SUCCESS;
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int ShiRen::p_magic_skill(int &step, Action* action)
{
	//p_magic_skill不同于别的触发点，有且只有一个匹配，因此每一个技能完成时，务必把step设为STEP_DONE
	int ret;
	switch(action->action_id())
	{
	case BU_XIE_HE_XIAN:
		ret = BuXieZhiXian(action);
		if(toNextStep(ret) || ret == GE_URGENT)
		{
			step = STEP_DONE;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int ShiRen::BuXieZhiXian(Action* action)
{
	int choice = action->args(1);
	int dstID = action->dst_ids(0);
	int num = action->args(0);
	SkillMsg skill;
	Coder::skillNotice(id,dstID, BU_XIE_HE_XIAN, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	
	setToken(0, token[0]-num);

	GameInfo game_info;
	Coder::tokenNotice(id,0,token[0],game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);

	int ret;
	if(choice == 1)
	{
		vector<int> cards;
		HARM harm;
		harm.srcID = id;
		harm.type = HARM_NONE;
		harm.point = num -1;
		harm.cause = BU_XIE_HE_XIAN;
		engine->setStateMoveCardsToHand(-1, DECK_PILE, dstID, DECK_HAND, num -1, cards, harm, false);
		engine->setStateMoveCardsToHand(-1, DECK_PILE, id, DECK_HAND, num -1, cards, harm, false);
	}
	else if(choice == 2)
	{
		HARM qipai;
		qipai.point = num;
		qipai.srcID = id;
		qipai.type = HARM_NONE;
		qipai.cause = BU_XIE_HE_XIAN;
		engine->pushGameState(new StateRequestHand(dstID, qipai, -1, DECK_DISCARD, false, false));
		engine->pushGameState(new StateRequestHand(id, qipai, -1, DECK_DISCARD, false, false));
	}
	return GE_URGENT;
}

int ShiRen::p_harm_end(int &step, CONTEXT_HARM_END *con)
{
	if(con->harm.type != HARM_MAGIC)
		return GE_SUCCESS;
	int ret = GE_INVALID_STEP;
	step = CHEN_LUN_XIE_ZOU_QU;
	ret = ChenLunXieZouQu(con);
	if(toNextStep(ret) || ret == GE_URGENT)
	{
		step = STEP_DONE;
	}
	return ret;
}

int ShiRen::ChenLunXieZouQu(CONTEXT_HARM_END *con)
{
	int dstID = con->dstID;
	if(engine->getPlayerEntity(dstID)->getColor() == this->getColor())
		return GE_SUCCESS;
	int srcID = con->harm.srcID;
	if(engine->getPlayerEntity(srcID)->getColor() != this->getColor())
		return GE_SUCCESS;
	if(ChenLunUsed)
		return GE_SUCCESS;
	ChenLunNum[dstID] = true;
	int ChenLunFlag = 0;
	for(int i=0; i<6; i++)
		if(ChenLunNum[i])
			ChenLunFlag ++;
	if(ChenLunFlag<2)
		return GE_SUCCESS;

	CommandRequest cmd_req;
	Coder::askForSkill(id, CHEN_LUN_XIE_ZOU_QU, cmd_req);
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			if(respond->args(0)==1)
			{
				int dstID = respond->dst_ids(0);
				vector<int> cardIDs;
				PlayerEntity *dstPlayer = engine->getPlayerEntity(dstID);
				for(int i = 0; i < respond->card_ids_size();i ++)
				{
					cardIDs.push_back(respond->card_ids(i));
				}
				if(respond->card_ids_size() != 2 || getCardByID(cardIDs[0])->getElement() != getCardByID(cardIDs[1])->getElement())
					return GE_INVALID_ARGUMENT;
				int cardNum = 2;
				SkillMsg skill;
				Coder::skillNotice(id, dstID, CHEN_LUN_XIE_ZOU_QU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

				ChenLunUsed = true;

				CardMsg show_card;
				Coder::showCardNotice(id, cardNum, cardIDs, show_card);
				engine->sendMessage(-1, MSG_CARD, show_card);

				HARM harm;
				harm.type = HARM_MAGIC;
				harm.point = 1;
				harm.srcID = id;
				harm.cause = CHEN_LUN_XIE_ZOU_QU;
				engine->setStateTimeline3(dstID, harm);
				engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs, id, CHEN_LUN_XIE_ZOU_QU, true);
				return GE_URGENT;
			}
			else
				return GE_SUCCESS;
		}
		return ret;
	}
	else
	{
		return GE_TIMEOUT;
	}
}

int ShiRen::GeYongTianFu()
{
	if(!BaoFengUsed)
	{
		SkillMsg skill;
		Coder::skillNotice(id, id, GE_YONG_TIAN_FU, skill);
		engine->sendMessage(-1, MSG_SKILL, skill);

		setToken(0,token[0]+1);
		GameInfo game_info;
		Coder::tokenNotice(id, 0, token[0], game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);
	}
	return GE_SUCCESS;
}