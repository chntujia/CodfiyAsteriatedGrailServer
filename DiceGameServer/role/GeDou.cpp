#include "GeDou.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool GeDou::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case XU_LI_CANG_YAN:
		case CANG_YAN_ZHI_HUN:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_1, XU_LI_CANG_YAN, respond);
			return true;
			break;
		case NIAN_DAN:
			session->tryNotify(id, STATE_AFTER_MAGIC, NIAN_DAN, respond);
			return true;
			break;
		case DOU_SHEN_TIAN_QU:
		case BAI_SHI_HUAN_LONG_QUAN:
		case BAI_SHI_DOU_SHEN:
			session->tryNotify(id, STATE_BOOT, BAI_SHI_DOU_SHEN, respond);
			return true;
			break;
		}
	}
	//没匹配则返回false
	return false;
}

int GeDou::p_after_attack(int &step, int currentPlayerID) 
{
	int ret = GE_INVALID_STEP;
	if(currentPlayerID != id){
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步

	step = CANG_YAN_ZHI_HUN;
	ret = CangYanZiShang(currentPlayerID);
	if(toNextStep(ret) || ret == GE_URGENT){
		step = STEP_DONE;
	}
	return ret;
}
int GeDou::p_boot(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	if( id != currentPlayerID){
		return GE_SUCCESS;
	}
	step = BAI_SHI_DOU_SHEN;
	ret = BaiShiDouShen(currentPlayerID);
	if(toNextStep(ret) || ret == GE_URGENT){
			step = STEP_DONE;
	}
	return ret;
}
//在出错的时候，p_xxxx有可能执行不止一次，若每次都重头来过的话。。。所以需要step记录执行到哪里
int GeDou::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	if(step == STEP_INIT) {
		step = XU_LI_CANG_YAN;
	}
	if(step == XU_LI_CANG_YAN)
	{
		step = XU_LI_CANG_YAN;
		ret = XuLiCangYan(con);
		if(toNextStep(ret)){
			step = BAI_SHI_HUAN_LONG_QUAN;
		}
	}
	if(step == BAI_SHI_HUAN_LONG_QUAN)
	{
		ret = BaiShiHarm(con);
		if(toNextStep(ret)){
			step = STEP_DONE;
		}
	}
	return ret;
}

int GeDou::p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS * con)
{
	int ret = GE_INVALID_STEP;
	if(con->srcID != id || !con->isActive){
		return GE_SUCCESS;
	}
	step = XU_LI_YI_JI;
	ret = XuLiMiss(con);
	if(toNextStep(ret) || ret == GE_URGENT){
		step = STEP_DONE;
	}
	return ret;
}

int GeDou::p_timeline_3(int &step, CONTEXT_TIMELINE_3* con)
{
	int ret = GE_INVALID_STEP;
	if(con->dstID != id){
		return GE_SUCCESS;
	}
	step = NIAN_QI_LI_CHANG;
	ret = NianQiLiChang(con);
	if(toNextStep(ret)){
		step = STEP_DONE;
	}			
	return ret;
}

int GeDou::p_before_magic(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	if(currentPlayerID != id){
		return GE_SUCCESS;
	}
	step = BAI_SHI_HUAN_LONG_QUAN;
	ret = BaiShiQuitMagic(currentPlayerID);
	if(toNextStep(ret)){
		step = STEP_DONE;
	}
	return ret; 
}

int GeDou::p_after_magic(int &step, int currentPlayerID) 
{
	int ret = GE_INVALID_STEP;
	if(currentPlayerID != id){
		return GE_SUCCESS;
	}
	step = NIAN_DAN;
	ret = NianDan(step, currentPlayerID);
	if(toNextStep(ret) || ret == GE_URGENT){
		step = STEP_DONE;
	}
	return ret; 
}

int GeDou::p_before_special(int &step, int currentPlayerID)
{
	int ret = GE_INVALID_STEP;
	if(currentPlayerID != id){
		return GE_SUCCESS;
	}
	step = BAI_SHI_HUAN_LONG_QUAN;
	ret = BaiShiQuitSpecial(currentPlayerID);
	if(toNextStep(ret)){
		step = STEP_DONE;
	}
	return ret; 
}

int GeDou::NianQiLiChang(CONTEXT_TIMELINE_3 *con)
{
	if(con->dstID != id || con->harm.point <= 4)
	{
		return GE_SUCCESS;
	}
	network::SkillMsg skill;
	Coder::skillNotice(id, id, NIAN_QI_LI_CHANG, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	con->harm.point = 4;
	return GE_SUCCESS;
}

int GeDou::XuLiCangYan(CONTEXT_TIMELINE_1 *con)
{
	int ret;
	int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	if(srcID != id || !con->attack.isActive ){
		return GE_SUCCESS;
	}
	int skillID;
	if(token[0] == tokenMax[0]){
		skillID = CANG_YAN_ZHI_HUN;
	}
	if(token[0] == 0){
		skillID = XU_LI_YI_JI;
	}
	if(token[0] > 0 && token[0] < tokenMax[0]){
		skillID = XU_LI_CANG_YAN;
	}
	//满足发动条件，询问客户端是否发动
	CommandRequest cmd_req;
	Coder::askForSkill(id, skillID, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(srcID, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0)==1)
			{	
				network::SkillMsg skill;
				Coder::skillNotice(id, dstID, XU_LI_YI_JI, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				xuLiUsed = true;
				setToken(0, token[0]+1);
				GameInfo game_info;
				Coder::tokenNotice(id, 0, token[0], game_info);
				//蓄力退出百事
				if(tap)
				{
					tap = false;
					baiShiTarget = -1;
					Coder::tapNotice(id, tap, game_info);
				}
				engine->sendMessage(-1, MSG_GAME, game_info);
				con->harm.point += 1;
				return GE_SUCCESS;
			}
			if(respond->args(0)==2)
			{
				network::SkillMsg skill;
				Coder::skillNotice(id, dstID, CANG_YAN_ZHI_HUN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				cangYanUsed = true;
				setToken(0, token[0]-1);
				GameInfo game_info;
				Coder::tokenNotice(id, 0, token[0], game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				con->hitRate = RATE_NOREATTACK;
				return GE_SUCCESS;
			}
			//没法动技能
			return GE_SUCCESS;
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int GeDou::XuLiMiss(CONTEXT_TIMELINE_2_MISS *con)
{
	if(con->srcID != id || !con->isActive || token[0] == 0 || !xuLiUsed)
	{
		return GE_SUCCESS;
	}
	xuLiUsed = false;
	HARM harm;
	harm.srcID = id;
	harm.point = token[0];
	harm.type = HARM_MAGIC;
	harm.cause = XU_LI_YI_JI;
	engine->setStateTimeline3(id, harm);
	return GE_URGENT;
}

int GeDou::NianDan(int step, int playerID)
{
	int ret;
	if(playerID != id || token[0] == tokenMax[0]){
		return GE_SUCCESS;
	}
	//满足发动条件，询问客户端是否发动
	CommandRequest cmd_req;
	Coder::askForSkill(id, NIAN_DAN, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0)==1)
			{	
				int dstID = respond->dst_ids(0);
				PlayerEntity * dstPlayer = engine->getPlayerEntity(dstID);
				network::SkillMsg skill;
				Coder::skillNotice(id, dstID, NIAN_DAN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				setToken(0, token[0]+1);
				GameInfo game_info;
				Coder::tokenNotice(id, 0, token[0], game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				if(dstPlayer->getCrossNum()==0)
				{
					HARM selfHarm;
					selfHarm.srcID = id;
					selfHarm.point = token[0];
					selfHarm.type = HARM_MAGIC;
					selfHarm.cause = NIAN_DAN;
					engine->setStateTimeline3(id, selfHarm);
				}
				HARM harm;
				harm.srcID = id;
				harm.point = 1;
				harm.type = HARM_MAGIC;
				harm.cause = NIAN_DAN;
				engine->setStateTimeline3(dstID, harm);
				return GE_URGENT;
			}
			//没发动技能
			return GE_SUCCESS;
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int GeDou::CangYanZiShang(int PlayerID)
{
	//把蓄力重置放在其中
	xuLiUsed = false;
	if(PlayerID != id || token[0] == 0 || !cangYanUsed)
	{
		cangYanUsed = false;
		return GE_SUCCESS;
	}
	cangYanUsed = false;
	HARM harm;
	harm.srcID = id;
	harm.point = token[0];
	harm.type = HARM_MAGIC;
	harm.cause = CANG_YAN_ZHI_HUN;
	engine->setStateTimeline3(id, harm);
	return GE_URGENT;
}

int GeDou::BaiShiDouShen(int PlayerID)
{
	int ret;
	int skillID;
	if(getEnergy()<=0 && token[0] < 3)
	{
		return GE_SUCCESS;
	}
	if(getEnergy()>0 && token[0] >= 3){
		skillID = BAI_SHI_DOU_SHEN;
	}
	if(getEnergy()>0 && token[0] < 3){
		skillID = DOU_SHEN_TIAN_QU;
	}
	if(getEnergy()<=0 && token[0] >=3 ){
		skillID = BAI_SHI_HUAN_LONG_QUAN;
	}
	CommandRequest cmd_req;
	Coder::askForSkill(id, skillID, cmd_req);
	//有限等待，由UserTask调用tryNotify唤醒
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//发动
			if(respond->args(0)==1)
			{	
				network::SkillMsg skill;
				Coder::skillNotice(id, id, BAI_SHI_HUAN_LONG_QUAN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				baiShiUsed = true;
				setToken(0, token[0]-3);
				GameInfo game_info;
				Coder::tokenNotice(id, 0, token[0], game_info);
				tap = true;
				Coder::tapNotice(id, tap, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				return GE_SUCCESS;
			}
			if(respond->args(0)==2)
			{
				network::SkillMsg skill;
				Coder::skillNotice(id, id, DOU_SHEN_TIAN_QU, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				GameInfo update_info;
				if(crystal>0){
					setCrystal(--crystal);
				}
				else{
					setGem(--gem);
				}
				Coder::energyNotice(id, gem, crystal, update_info);
				addCrossNum(2);
				Coder::crossNotice(id, crossNum, update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
				vector<int> cardIDs;
				int cardNum = ((getHandCardNum()-3)>0)? (getHandCardNum()-3):0;
				for(int i = 0; i < cardNum; i ++)
				{
					cardIDs.push_back(respond->card_ids(i));
				}
				if(cardNum > 0)
				{
					engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs, id, DOU_SHEN_TIAN_QU, false);
					return GE_URGENT;
				}
				return GE_SUCCESS;
			}
			//没发动技能
			return GE_SUCCESS;
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}

}

int GeDou::BaiShiHarm(CONTEXT_TIMELINE_1 *con)
{
	int ret;
	int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	if(srcID != id || !tap ){
		return GE_SUCCESS;
	}
	if(con->attack.isActive)
	{
		if(baiShiUsed)
		{
			baiShiTarget = con->attack.dstID;
			baiShiUsed = false;
		}
		//攻击换目标则推出百事
		if(baiShiTarget != con->attack.dstID)
		{
			tap = false;
			baiShiTarget = -1;
			GameInfo game_info;
			Coder::tapNotice(id, tap, game_info);
			engine->sendMessage(-1, MSG_GAME, game_info);
			return GE_SUCCESS;
		}
		else
		{
			SkillMsg skill;
			Coder::skillNotice(id, dstID, BAI_SHI_HUAN_LONG_QUAN, skill);
			engine->sendMessage(-1, MSG_SKILL, skill);
			con->harm.point+=2;
			return GE_SUCCESS;
		}
	}
	//应战伤害
	else
	{
		SkillMsg skill;
		Coder::skillNotice(id, dstID, BAI_SHI_HUAN_LONG_QUAN, skill);
		engine->sendMessage(-1, MSG_SKILL, skill);
		con->harm.point+=1;
		return GE_SUCCESS;
	}
}

int GeDou::BaiShiQuitMagic(int playerID)
{
	if(!tap || id != playerID){
		return GE_SUCCESS;
	}
	tap = false;
	baiShiTarget = -1;
	GameInfo game_info;
	Coder::tapNotice(id, tap, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
	return GE_SUCCESS;
}

int GeDou::BaiShiQuitSpecial(int playerID)
{
	if(!tap || id != playerID){
		return GE_SUCCESS;
	}
	tap = false;
	baiShiTarget = -1;
	GameInfo game_info;
	Coder::tapNotice(id, tap, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
	return GE_SUCCESS;
}