#include "KuangZhan.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool KuangZhan::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case XUE_YING_KUANG_DAO:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_1, XUE_YING_KUANG_DAO, respond);
			return true;
			break;
		case XUE_XING_PAO_XIAO:
			session->tryNotify(id, STATE_TIMELINE_1, XUE_XING_PAO_XIAO, respond);
			return true;
			break;
		case SI_LIE:
			session->tryNotify(id, STATE_TIMELINE_2_HIT, SI_LIE, respond);
			return true;
			break;
		}
	}
	//没匹配则返回false
	return false;
}

//在出错的时候，p_xxxx有可能执行不止一次，若每次都重头来过的话。。。所以需要step记录执行到哪里
int KuangZhan::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	//狂战血影血咆均主动攻击可发动
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	if(step == STEP_INIT){
		step = XUE_YING_KUANG_DAO;
	}
	if(step == XUE_YING_KUANG_DAO)
	{
		ret = XueYingKuangDao(con);
		if(toNextStep(ret)){
			step = XUE_XING_PAO_XIAO;
		}
	}
	if(step == XUE_XING_PAO_XIAO)
	{
		ret = XueXingPaoXiao(con);
		if(toNextStep(ret)){
			step = STEP_DONE;
		}
	}				
	return ret;
}

int KuangZhan::p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con)
{
	int ret = GE_INVALID_STEP;
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	if(step == STEP_INIT){
		step = KUANG_HUA;
	}
	if(step == KUANG_HUA)
	{
		ret = KuangHua(con);
		if(toNextStep(ret)){
			step = SI_LIE;
		}			
	}
	if(step == SI_LIE)
	{
		ret = SiLie(con);
		if(toNextStep(ret)){
			step = STEP_DONE;
		}
	}
	return ret;
}

int KuangZhan::XueYingKuangDao(CONTEXT_TIMELINE_1 *con)
{
	int ret;
	int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	int cardID = con->attack.cardID;
	CardEntity* card = getCardByID(cardID);
	if(srcID != id || !card->checkSpeciality(XUE_YING_KUANG_DAO) || !con->attack.isActive ){
		return GE_SUCCESS;
	}
	int dstHandCardNum = engine->getPlayerEntity(dstID)->getHandCardNum();
	if(dstHandCardNum < 2 || dstHandCardNum > 3){
		return GE_SUCCESS;
	}
	//满足发动条件，询问客户端是否发动
	CommandRequest cmd_req;
	Coder::askForSkill(id, XUE_YING_KUANG_DAO, cmd_req);
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
				Coder::skillNotice(id, dstID, XUE_YING_KUANG_DAO, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				if(dstHandCardNum == 2)
				{
					con->harm.point+=2;
				}
				if(dstHandCardNum == 3)
				{
					con->harm.point+=1;
				}
			}
			return GE_SUCCESS;
			
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int KuangZhan::XueXingPaoXiao(CONTEXT_TIMELINE_1 *con)
{
	int ret;
	int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	int cardID = con->attack.cardID;
	CardEntity* card = getCardByID(cardID);
	if(srcID != id || !card->checkSpeciality(XUE_XING_PAO_XIAO) || !con->attack.isActive){
		return GE_SUCCESS;
	}
	if(engine->getPlayerEntity(dstID)->getCrossNum() != 2){
		return GE_SUCCESS;
	}
	//满足发动条件，询问客户端是否发动
	CommandRequest cmd_req;
	Coder::askForSkill(id, XUE_XING_PAO_XIAO, cmd_req);
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
				Coder::skillNotice(id, dstID, XUE_XING_PAO_XIAO, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				con->hitRate = RATE_NOMISS;
			}
			return GE_SUCCESS;
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int KuangZhan::KuangHua(CONTEXT_TIMELINE_2_HIT *con)
{
	int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	if(srcID != id){
		return GE_SUCCESS;
	}
	SkillMsg skill;
	Coder::skillNotice(id, con->attack.dstID, KUANG_HUA, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	con->harm.point += 1;
	return GE_SUCCESS;
}

int KuangZhan::SiLie(CONTEXT_TIMELINE_2_HIT *con)
{
	int ret;
	int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	GameInfo update_info;
	if(srcID != id || getGem() <= 0){
		return GE_SUCCESS;
	}
	//满足发动条件，询问客户端是否发动
	CommandRequest cmd_req;
	Coder::askForSkill(id, SI_LIE, cmd_req);
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
				Coder::skillNotice(id, dstID, SI_LIE, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				con->harm.point += 2;
				setGem(--gem);
				Coder::energyNotice(id, gem, crystal, update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
			}
			return GE_SUCCESS;
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}