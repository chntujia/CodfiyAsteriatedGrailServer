#include "PlayerEntity.h"
#include "GameGrail.h"

PlayerEntity::PlayerEntity(GameGrail *engine,int ID, int isRed)
{
    //setInfo(0);
    crossNum=0;
    crossMax=3;
    gem=0;
    crystal=0;
    handCardsMax=6;
    handCardsRange=0;
    handCardsMin=0;
    token[0]=tokenMax[0]=0;
    token[1]=tokenMax[1]=0;
    token[2]=tokenMax[2]=0;
    this->setTap(false);
    this->setHandCardsMaxFixed(false);

    this->id = ID;
    this->color = isRed;
    this->engine = engine;
    this->teamArea = engine->getTeamArea();

    //以下属性在子类中修改
    this->crossMax = 2;
    this->energyMax = 3;
}

//移除基础效果卡
int PlayerEntity::removeBasicEffect(int card)
{
	for(list< BasicEffect >::iterator it = basicEffects.begin(); it != basicEffects.end(); it++){
        if(it->card == card){
            basicEffects.erase(it);
			return GE_SUCCESS;
		}
	}
    return GE_MOVECARD_FAILED;
}

int PlayerEntity::addBasicEffect(int effectCard,int srcUserID)
{
	struct BasicEffect be;
	be.card = effectCard;
	be.srcUser = srcUserID;
	this->basicEffects.push_back(be);
	return GE_SUCCESS;
}

int PlayerEntity::checkBasicEffect(int card)
{
	for(list< BasicEffect >::iterator it = basicEffects.begin(); it != basicEffects.end(); it++){
        if(it->card == card){
			return GE_SUCCESS;
		}
	}
    return GE_BASIC_EFFECT_NOT_FOUND;
}

//移除手牌
int PlayerEntity::removeHandCards(int howMany, vector<int> oldCard)
{
	int size = handCards.size();
	for(int i = 0;i < howMany;i++)
	{
		this->handCards.remove(oldCard[i]);
	}
	return size == handCards.size()+howMany ? GE_SUCCESS : GE_MOVECARD_FAILED;
}

//增加手牌
int PlayerEntity::addHandCards(int howMany, vector< int > newCard)
{
    for(int i = 0;i < howMany;i++)
    {
		handCards.push_back(newCard[i]);   
    }
	return GE_SUCCESS;
}

int PlayerEntity::checkHandCards(int howMany, vector<int> cards)
{
	for(int i = 0;i < howMany;i++)
	{
		std::list<int>::iterator findIter = std::find(handCards.begin(), handCards.end(), cards[i]);
		if(findIter == handCards.end()){
			return GE_HANDCARD_NOT_FOUND;
		}
	}
	return GE_SUCCESS;
}

int PlayerEntity::checkOneHandCard(int cardID)
{
	std::list<int>::iterator findIter = std::find(handCards.begin(), handCards.end(), cardID);
	if(findIter == handCards.end()){
		return GE_HANDCARD_NOT_FOUND;
	}
	return GE_SUCCESS;
}

int PlayerEntity::removeCoverCards(int howMany, vector< int > cards)
{
	int size = coverCards.size();
	for(int i = 0;i < howMany;i++)
	{
		coverCards.remove(cards[i]);
	}
	return size == coverCards.size()+howMany ? GE_SUCCESS : GE_MOVECARD_FAILED;
}

int PlayerEntity::addCoverCards(int howMany, vector< int > cards)
{
    for(int i = 0;i < howMany;i++)
    {
		coverCards.push_back(cards[i]);
    }
	return GE_SUCCESS;
}

void PlayerEntity::setHandCardsMaxFixed(bool fixed, int howmany)
{
    handCardsMaxFixed = fixed;
    if(fixed){
        handCardsMax = howmany;
	}
    else{
        handCardsMax = 6 + handCardsRange;
	}
    if(handCardsMax<handCardsMin){
        handCardsMax = handCardsMin;
	}
}

void PlayerEntity::addHandCardsRange(int howMany)
{
    handCardsRange += howMany;
    if(handCardsMaxFixed){
        return;
	}
    else{
        handCardsMax = 6 + handCardsRange;
	}
    if(handCardsMax<handCardsMin){
        handCardsMax = handCardsMin;
	}
}

void PlayerEntity::addCrossNum(int howMany, int atMost)
{
    if(atMost==-1){
        atMost=crossMax;
	}
    if(crossNum>=atMost){
        return;
	}
    crossNum=(crossNum+howMany)<=atMost?(crossNum+howMany):atMost;
}

void PlayerEntity::subCrossNum(int howMany)
{
    crossNum = crossNum>howMany ? crossNum-howMany : 0;
}

void PlayerEntity::setGem(int howMany)
{
    if(howMany+crystal<=energyMax){
        gem=howMany;
	}
    else{
        gem=energyMax-crystal;
	}
}

void PlayerEntity::setCrystal(int howMany)
{
    if(howMany+gem<=energyMax){
        crystal=howMany;
	}
    else{
        crystal=energyMax-gem;
	}
}

void PlayerEntity::setYourTurn(bool yes)
{
    yourTurn=yes;
}

bool PlayerEntity::getYourturn()
{
    return yourTurn;
}

int PlayerEntity::getID()
{
    return id;
}

string PlayerEntity::getName()
{
    return name;
}

int PlayerEntity::getHandCardMax()
{
    return handCardsMax;
}

int PlayerEntity::getHandCardNum()
{
    return this->handCards.size();
}

int PlayerEntity::getCrossNum()
{
    return crossNum;
}
int PlayerEntity::getCrossMax()
{
    return crossMax;
}

int PlayerEntity::getGem()
{
    return gem;
}

int PlayerEntity::getCrystal()
{
    return crystal;
}

int PlayerEntity::getEnergy()
{
    return gem+crystal;
}

int PlayerEntity::getColor()
{
    return color;
}





