#include "JianSheng.h"
#include "..\GameGrail.h"
int JianSheng::p_before_turn_begin(int currentPlayerID) 
{
	has_LianXuJi = false;
	has_JianYing = false;
	attackCount = 0;
	return GE_SUCCESS; 
}

int JianSheng::p_timeline_1(CONTEXT_TIMELINE_1 *con)
{
	int ret;
	if(GE_SUCCESS != (ret=LieFengJi(con))){
		return ret;
	}
	if(GE_SUCCESS != (ret=JiFengJi(con))){
		return ret;
	}
	if(GE_SUCCESS != (ret=ShengJian(con))){
		return ret;
	}
	return GE_SUCCESS;
}

int JianSheng::p_after_attack(int playerID)
{
	int ret;
	if(GE_SUCCESS != (ret=LianXuJi(playerID))){
		return ret;
	}
	return GE_SUCCESS;
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
