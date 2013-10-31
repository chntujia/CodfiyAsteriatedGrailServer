#include "JianSheng.h"
#include "..\GameGrail.h"

//初始化各种回合变量
int JianSheng::p_before_turn_begin(int &step, int currentPlayerID) 
{
	used_LianXuJi = false;
	used_JianYing = false;
	attackCount = 0;
	return GE_SUCCESS; 
}

//在出错的时候，p_xxxx有可能执行不止一次，若每次都重头来过的话。。。所以需要step记录执行到哪里
int JianSheng::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	//若成功则继续往下走，失败则返回，step会保留，下次再进来就不会重走
	switch(step)
	{
	case 0:
		if(GE_SUCCESS != (ret=LieFengJi(con))){
			break;
		}
		step++;
	case 1:
		if(GE_SUCCESS != (ret=JiFengJi(con))){
			break;
		}
		step++;
	case 2:
		if(GE_SUCCESS != (ret=ShengJian(con))){
			break;
		}
		step++;
	//为了防止step2超时
	case 3:
		ret = GE_SUCCESS;
	}
	//超时的话，直接进入下一步
	if(GE_TIMEOUT == ret){
		step++;
	}
	return ret;
}

//所有额外行动，都是集中到一个地方询问，而不是每个都问一遍
int JianSheng::p_after_attack(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	switch(step)
	{
	case 0:
		if(GE_SUCCESS != (ret=LianXuJi(playerID))){
			break;
		}
		step++;
	case 1:
		if(GE_SUCCESS != (ret=JianYing(playerID))){
			break;
		}
		step++;
	case 2:
		ret = GE_SUCCESS;
	}
	//其实这里没有询问，所以没有超时
	if(GE_TIMEOUT == ret){
		step++;
	}
	return ret;
}

int JianSheng::LieFengJi(CONTEXT_TIMELINE_1 *con)
{
	int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	int cardID = con->attack.cardID;
	CardEntity* card = getCardByID(cardID);
	if(srcID != id || !card->checkSpeciality(LIE_FENG_JI)){
		return GE_SUCCESS;
	}
	PlayerEntity* dst = engine->getPlayerEntity(dstID);
	if(!dst->checkBasicEffectName(NAME_SHIELD)){
		return GE_SUCCESS;
	}
	//if(engine->waitForOne(id, network::MSG_CMD_REQ, weaken_proto)){

	//}
	//else{
	//	return GE_TIMEOUT;
	//}
	return GE_SUCCESS;
}

int JianSheng::JiFengJi(CONTEXT_TIMELINE_1 *con)
{
	return GE_SUCCESS;
}

int JianSheng::ShengJian(CONTEXT_TIMELINE_1 *con)
{
	return GE_SUCCESS;
}

int JianSheng::LianXuJi(int playerID)
{
	return GE_SUCCESS;
}

int JianSheng::JianYing(int playerID)
{
	return GE_SUCCESS;
}