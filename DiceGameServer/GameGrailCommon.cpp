#include "GameGrailCommon.h"
CardEntity* getCardByID(int id)
{
    if(id>=0 && id<150)
        return cardList[id];
	ztLoggerWrite(ZONE, e_Error, "Card: %d Requested out of Rank!", id);
	throw GE_INVALID_CARDID;
}

bool isValidRoleID(int roleID)
{
	for(int i = 0; i < sizeof(SUMMON)/sizeof(int); i++){
		if(SUMMON[i] == roleID)
			return true;
	}
	return false;
}