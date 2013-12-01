#include "JianSheng.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

bool JianSheng::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case LIE_FENG_JI:
			//tryNotify负责向游戏主线程传消息，只有id等于当前等待id，声明state等于当前state，声明step等于当前step，游戏主线程才会接受
			session->tryNotify(id, STATE_TIMELINE_1, LIE_FENG_JI, respond);
			return true;
		}
	}
	//没匹配则返回false
	return false;
}

//统一在p_before_turn_begin 初始化各种回合变量
int JianSheng::p_before_turn_begin(int &step, int currentPlayerID) 
{
	used_LianXuJi = false;
	used_JianYing = false;
	using_LianXuJi = false;
	attackCount = 0;
	return GE_SUCCESS; 
}

//在出错的时候，p_xxxx有可能执行不止一次，若每次都重头来过的话。。。所以需要step记录执行到哪里
int JianSheng::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	//剑圣的技能都是要作为攻击方
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	bool success = true;
	while(STEP_DONE != step && success)
	{
		success = false;
		switch(step)
		{
		case STEP_INIT:
			ret = LieFengJi(con);
			if(toNextStep(ret)){
				step = JI_FENG_JI;
				success = true;
			}			
			break;
		case JI_FENG_JI:
			ret = JiFengJi(con);
			if(toNextStep(ret)){
				step = SHENG_JIAN;
				success = true;
			}			
			break;
		case SHENG_JIAN:
			ret = ShengJian(con);
			if(toNextStep(ret)){
				//全部走完后，请把step设成STEP_DONE
			    step = STEP_DONE;
				success = true;
			}			
			break;
		default:
			return GE_INVALID_STEP;
		}
	}
	return ret;
}

//所有额外行动，都是集中到一个地方询问，而不是每个都问一遍
int JianSheng::p_after_attack(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	//不是剑圣就不用跑了
	if(playerID != id){
		return GE_SUCCESS;
	}
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	//一般超时也会继续下一步
	bool success = true;
	while(STEP_DONE != step && success)
	{
		success = false;
		switch(step)
		{
		case STEP_INIT:
			ret = LianXuJi(playerID);
			if(toNextStep(ret)){
				step = JIAN_YING;
				success = true;
			}			
			break;
		case JIAN_YING:
			ret = JianYing(playerID);
			if(toNextStep(ret)){
				//全部走完后，请把step设成STEP_DONE
				step = STEP_DONE;
				success = true;
			}			
			break;
		default:
			return GE_INVALID_STEP;
		}
	}
	return ret;
}

int JianSheng::v_additional_action(int chosen)
{
	switch(chosen)
	{
	case LIAN_XU_JI:
		//回合限定
		if(used_LianXuJi){
			return GE_INVALID_ACTION;
		}
		break;
	case JIAN_YING:
		//回合限定       || 能量
		if(used_JianYing || getEnergy() <= 0){
			return GE_INVALID_ACTION;
		}
		break;
	}
	//通过角色相关的检测，基本检测交给底层
	return PlayerEntity::v_additional_action(chosen);
}

int JianSheng::p_additional_action(int chosen)
{
	GameInfo update_info;
	switch(chosen)
	{
	case LIAN_XU_JI:
		used_LianXuJi = true;
		using_LianXuJi = true;
		break;
	case JIAN_YING:
		used_JianYing = true;
		using_LianXuJi = false;
		if(crystal>0){
			setCrystal(--crystal);
		}
		else{
			setGem(--gem);
		}
		Coder::energyNotice(id, gem, crystal, update_info);
		engine->sendMessage(-1, MSG_GAME, update_info);
		break;
	default:
		using_LianXuJi = false;
	}
	//做完角色相关的操作，扣除额外行动交给底层
	return PlayerEntity::p_additional_action(chosen);
}

int JianSheng::v_attack(int cardID, int dstID, bool realCard)
{
	if(using_LianXuJi){
		CardEntity* card = getCardByID(cardID);
		if(card->getElement() != ELEMENT_WIND){
			return GE_INVALID_ACTION;
		}
	}
	//通过角色相关的检测，其他基本检测交给底层
	return PlayerEntity::v_attack(cardID, dstID, realCard);
}

int JianSheng::LieFengJi(CONTEXT_TIMELINE_1 *con)
{
	int ret;
	int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	int cardID = con->attack.cardID;
	CardEntity* card = getCardByID(cardID);
	//是不是剑圣   || 是不是烈风技
	if(srcID != id || !card->checkSpeciality(LIE_FENG_JI)){
		return GE_SUCCESS;
	}
	PlayerEntity* dst = engine->getPlayerEntity(dstID);
	//目标有没盾
	//FIXME: 天使之墙
	if(GE_SUCCESS != dst->checkBasicEffectByName(NAME_SHIELD)){
		return GE_SUCCESS;
	}
	//满足发动条件，询问客户端是否发动
	CommandRequest cmd_req;
	Coder::askForSkill(id, LIE_FENG_JI, cmd_req);
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
				Coder::skillNotice(id, dstID, LIE_FENG_JI, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
				con->hitRate = RATE_NOREATTACK;
				con->checkShield = false;
			}
		}
		return ret;
	}
	else{
		//超时啥都不用做
		return GE_TIMEOUT;
	}
}

int JianSheng::JiFengJi(CONTEXT_TIMELINE_1 *con)
{
	int srcID = con->attack.srcID;
	int cardID = con->attack.cardID;
	CardEntity* card = getCardByID(cardID);
	//是不是剑圣   || 是不是疾风技                       || 是不是主动攻击
	if(srcID != id || !card->checkSpeciality(JI_FENG_JI) || !con->attack.isActive){
		return GE_SUCCESS;
	}
	SkillMsg skill;
	Coder::skillNotice(id, con->attack.dstID, JI_FENG_JI, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	addAction(ACTION_ATTACK, JI_FENG_JI);
	return GE_SUCCESS;
}

int JianSheng::ShengJian(CONTEXT_TIMELINE_1 *con)
{
	int srcID = con->attack.srcID;
	//是不是剑圣   || 是不是主动攻击
	if(srcID != id || !con->attack.isActive){
		return GE_SUCCESS;
	}
	attackCount++;
	if(3 == attackCount){
		SkillMsg skill;
		Coder::skillNotice(id, con->attack.dstID, SHENG_JIAN, skill);
		engine->sendMessage(-1, MSG_SKILL, skill);
		con->hitRate = RATE_NOMISS;
	}	
	return GE_SUCCESS;
}

//旁观者视角，只要有牌就有可能发动，先加上额外行动
int JianSheng::LianXuJi(int playerID)
{
	//是不是剑圣      || 有没有手牌        || 回合限定      || 已经算上连续技了
	if(playerID != id || handCards.empty() || used_LianXuJi || containsAction(LIAN_XU_JI)){
		return GE_SUCCESS;
	}
	addAction(ACTION_ATTACK, LIAN_XU_JI);
	return GE_SUCCESS;
}

int JianSheng::JianYing(int playerID)
{
	//是不是剑圣      || 有没有手牌        || 有没有能量       || 回合限定      || 已经算上剑影了
	if(playerID != id || handCards.empty() || getEnergy() <= 0 || used_JianYing || containsAction(JIAN_YING)){
		return GE_SUCCESS;
	}
	addAction(ACTION_ATTACK, JIAN_YING);
	return GE_SUCCESS;
}