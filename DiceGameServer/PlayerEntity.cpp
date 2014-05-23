#include "PlayerEntity.h"
#include "GameGrail.h"
#include "role\QiDao.h"
using namespace network;
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
	memset(exclusiveEffects, 0, sizeof(exclusiveEffects));
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

int PlayerEntity::checkBasicEffectByCard(int card)
{
	for(list< BasicEffect >::iterator it = basicEffects.begin(); it != basicEffects.end(); it++){
		if(it->card == card){
			return GE_SUCCESS;
		}
	}
    return GE_BASIC_EFFECT_NOT_FOUND;
}

int PlayerEntity::checkBasicEffectByName(int name, int* cardID, int* src)
{
	for(list< BasicEffect >::iterator it = basicEffects.begin(); it != basicEffects.end(); it++){
		CardEntity* card = getCardByID(it->card);
        if(card->getName() == name || card->checkSpeciality(name)){
			if(cardID != NULL){
				*cardID = it->card;
			}
			if(src != NULL){
				*src = it->srcUser;
			}
			return GE_SUCCESS;
		}
	}
    return GE_BASIC_EFFECT_NOT_FOUND;
}

int PlayerEntity::checkBasicEffectBySpeciality(int speciality, int* cardID, int* src)
{
	for(list< BasicEffect >::iterator it = basicEffects.begin(); it != basicEffects.end(); it++){
		if(getCardByID(it->card)->checkSpeciality(speciality)){
			if(cardID != NULL){
				*cardID = it->card;
			}
			if(src != NULL){
				*src = it->srcUser;
			}
			return GE_SUCCESS;
		}
	}
    return GE_BASIC_EFFECT_NOT_FOUND;
}

int PlayerEntity::checkExclusiveEffect(int name)
{
	if(name < 0 || name > EXCLUSIVE_NUM){
		return GE_INVALID_EXCLUSIVE_EFFECT;
	}
    return exclusiveEffects[name] ? GE_SUCCESS : GE_EXCLUSIVE_EFFECT_NOT_FOUND;
}

void PlayerEntity::addExclusiveEffect(int name)
{
	if(name < 0 || name > EXCLUSIVE_NUM){
		throw GE_INVALID_EXCLUSIVE_EFFECT;
	}
	exclusiveEffects[name] = true;
}

void PlayerEntity::removeExclusiveEffect(int name)
{
	if(name < 0 || name > EXCLUSIVE_NUM){
		throw GE_INVALID_EXCLUSIVE_EFFECT;
	}
	exclusiveEffects[name] = false;
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

int PlayerEntity::checkCoverCards(int howMany, vector<int> cards)
{
	for(int i = 0; i < howMany; i++)
	{
		std::list<int>::iterator findIter = std::find(coverCards.begin(), coverCards.end(), cards[i]);
		if(findIter == coverCards.end()){
			return GE_COVERCARD_NOT_FOUND;
		}
	}
	return GE_SUCCESS;
}

int PlayerEntity::checkOneCoverCard(int cardID)
{
	std::list<int>::iterator findIter = std::find(coverCards.begin(), coverCards.end(), cardID);
	if(findIter == coverCards.end()){
		return GE_COVERCARD_NOT_FOUND;
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
	else if (atMost == -2) {
		// 完全无视上限
		crossNum=(crossNum+howMany);
		return;
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

bool PlayerEntity::containsAction(int cause)
{
	list<ACTION_QUOTA>::iterator it;
	for(it = additionalActions.begin(); it != additionalActions.end(); it++){
		if(it->cause == cause){
			return true;
		}
	}
	return false;
}

int PlayerEntity::v_allow_action(Action* action, int allow, bool canGiveUp)
{
	int claim = action->action_type();
	if(claim == ACTION_ATTACK_SKILL){
		claim = ACTION_ATTACK;
	}
	if(claim == ACTION_MAGIC_SKILL){
		claim = ACTION_MAGIC;
	}
	if(claim == ACTION_SPECIAL_SKILL){
		claim = ACTION_SPECIAL;
	}
	switch(allow)
	{
	case ACTION_ANY:
		if(claim == ACTION_ATTACK || claim == ACTION_MAGIC || claim == ACTION_SPECIAL){
			return GE_SUCCESS;
		}
		//只有正常行动能宣告无法行动
		if(claim == ACTION_UNACTIONAL){
			return v_unactional();
		}
		break;
	case ACTION_ATTACK_MAGIC:
		if(claim == ACTION_ATTACK || claim == ACTION_MAGIC){
			return GE_SUCCESS;
		}
		break;
	case ACTION_ATTACK:
	case ACTION_MAGIC:
	case ACTION_SPECIAL:
	case ACTION_NONE:
		if(claim == allow){
			return GE_SUCCESS;
		}
		break;
	}
	if(canGiveUp && ACTION_NONE == claim){
		return GE_SUCCESS;
	}
	return GE_INVALID_ACTION;
}

//FIXME: 潜行 挑衅 多重
int PlayerEntity::v_attack(int cardID, int dstID, bool realCard)
{
	int ret;
	PlayerEntity* dst = engine->getPlayerEntity(dstID);
	
	if( GE_SUCCESS == checkExclusiveEffect(EX_TIAO_XIN) && dst->getRoleID() != 21){
		return GE_INVALID_PLAYERID;
	}
	if (GE_SUCCESS != (ret = dst->v_attacked())){
		return ret;
	}
	if(realCard){
		if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
			return ret;
		}
	}
	if(getCardByID(cardID)->getType() != TYPE_ATTACK){
		return GE_INVALID_CARDID; 
	}
	
	if(dst->getColor() == color){
		return GE_INVALID_PLAYERID;
	}
	return GE_SUCCESS;
}

int PlayerEntity::v_reattack(int cardID, int orignCardID, int dstID, int orignID, int rate, bool realCard)
{
	if(rate != RATE_NORMAL){
		return GE_INVALID_ACTION;
	}
	if(realCard){
		int ret;
		if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
			return ret;
		}
	}
	CardEntity* orignCard = getCardByID(orignCardID);
	if(orignCard->getElement() == ELEMENT_DARKNESS){
		return GE_INVALID_CARDID; 
	}
	CardEntity* card = getCardByID(cardID);		
	if(card->getType() != TYPE_ATTACK){
		return GE_INVALID_CARDID; 
	}
	if(card->getElement() != ELEMENT_DARKNESS && card->getElement() != orignCard->getElement()){
		return GE_INVALID_CARDID; 
	}

	PlayerEntity *dst = engine->getPlayerEntity(dstID);
	if(dstID == orignID || dst->getColor() == color){
		return GE_INVALID_PLAYERID;
	}
	return GE_SUCCESS;
}

int PlayerEntity::v_missile(int cardID, int dstID, bool realCard)
{
	if(realCard){
		int ret;
		if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
			return ret;
		}
	}
	if(getCardByID(cardID)->getName() != NAME_MISSILE){
		return GE_INVALID_CARDID; 
	}
	//PlayerEntity *it = this;
	//while((it = it->getPost())->getColor() == color)
	//	;
	//if(dstID != it->getID()){
	//	return GE_INVALID_PLAYERID;
	//}
	return GE_SUCCESS;
}

int PlayerEntity::v_remissile(int cardID, bool realCard)
{
	if(realCard){
		int ret;
		if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
			return ret;
		}
	}
	if(getCardByID(cardID)->getName() != NAME_MISSILE){
		return GE_INVALID_CARDID; 
	}
	return GE_SUCCESS;
}

int PlayerEntity::v_block(int cardID)
{
	int ret;
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
		return ret;
	}
	if(getCardByID(cardID)->getName() != NAME_HOLYLIGHT){
		return GE_INVALID_CARDID; 
	}
	return GE_SUCCESS;
}

int PlayerEntity::v_shield(int cardID, PlayerEntity* dst)
{
	int ret;
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
		return ret;
	}
	if(GE_SUCCESS == dst->checkBasicEffectByName(NAME_SHIELD) || GE_SUCCESS == dst->checkBasicEffectByName(TIAN_SHI_ZHI_QIANG)){
		return GE_BASIC_EFFECT_ALREADY_EXISTS;
	}
	return GE_SUCCESS;
}

int PlayerEntity::v_weaken(int cardID, PlayerEntity* dst)
{
	int ret;
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID))){
		return ret;
	}
	if(GE_SUCCESS == dst->checkBasicEffectByName(NAME_WEAKEN)){
		return GE_BASIC_EFFECT_ALREADY_EXISTS;
	}
	return GE_SUCCESS;
}

int PlayerEntity::v_buy(Action *action)
{
	if(handCards.size()+3 > handCardsMax){
		return GE_INVALID_ACTION;
	}
	int gem = action->args(0);
	int crystal = action->args(1);
	if((gem==0 || gem==1) && (crystal==0 || crystal==1)){
		return GE_SUCCESS;
	}
	return GE_INVALID_ACTION;
}

int PlayerEntity::v_synthesize(Action *action, TeamArea* team)
{
	if(handCards.size()+3 > handCardsMax || team->getEnergy(color) < 3){
		return GE_INVALID_ACTION;
	}
	int gem = action->args(0);
	int crystal = action->args(1);
	if(gem+crystal!=3 || gem>team->getGem(color) || crystal>team->getCrystal(color)){
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int PlayerEntity::v_extract(Action *action, TeamArea* team)
{
	int gem = action->args(0);
	int crystal = action->args(1);
	if(gem > team->getGem(color) || crystal > team->getCrystal(color) || getGem()+getCrystal() >= energyMax){
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int PlayerEntity::v_additional_action(int chosen)
{
	list<ACTION_QUOTA>::iterator it;
	for(it = additionalActions.begin(); it != additionalActions.end(); it++){
		if(chosen == it->cause){
			return GE_SUCCESS;
		}
	}
	return GE_INVALID_ACTION;
}

int PlayerEntity::p_additional_action(int chosen)
{
	//[QiDao]如果是迅捷赐福的额外行动，无法调用祈祷的p_additional_action,因此在这里移除效果。
	int XunJieCardID = -1;
	if(chosen == XUN_JIE_CI_FU){XunJieCardID =  QiDao::GetXunJieEffectCard(engine,id);}
	
	
	list<ACTION_QUOTA>::iterator it;
	for(it = additionalActions.begin(); it != additionalActions.end(); it++){
		if(chosen == it->cause){
			SkillMsg skill;
			Coder::skillNotice(id, id, chosen, skill);
			engine->sendMessage(-1, MSG_SKILL, skill);			
			engine->popGameState();
			engine->pushGameState(new StateActionPhase(it->allowAction, true));
			additionalActions.erase(it);		
			if(chosen == XUN_JIE_CI_FU){engine->setStateMoveOneCardNotToHand(id, DECK_BASIC_EFFECT, -1, DECK_DISCARD, XunJieCardID, id, XUN_JIE_CI_FU, true);return GE_URGENT;}//[QiDao]移除迅捷赐福
			return GE_SUCCESS;
		}
	}
	return GE_INVALID_ACTION;
}

void PlayerEntity::toProto(SinglePlayerInfo *playerInfo)
{
	playerInfo->set_id(id);
	playerInfo->set_team(color);
	playerInfo->set_role_id(roleID);
	playerInfo->set_hand_count(handCards.size());
	playerInfo->set_max_hand(handCardsMax);	
	playerInfo->set_heal_count(crossNum);

	list<BasicEffect>::iterator it;
	for (it = basicEffects.begin(); it != basicEffects.end(); ++it)
	{
		playerInfo->add_basic_cards(it->card);
	}
	if (basicEffects.size() == 0)
		playerInfo->add_delete_field("basic_cards");

	bool hasEx = false;
	for (int i = 0; i < EXCLUSIVE_NUM; i++)
	{
		if(exclusiveEffects[i]){
			playerInfo->add_ex_cards(i);
			hasEx = true;
		}
	}
	if (!hasEx)
		playerInfo->add_delete_field("ex_cards");

	playerInfo->set_gem(gem);
	playerInfo->set_crystal(crystal);
	playerInfo->set_yellow_token(token[0]);
	playerInfo->set_blue_token(token[1]);
	playerInfo->set_covered_count(coverCards.size());
	playerInfo->set_is_knelt(tap);

}