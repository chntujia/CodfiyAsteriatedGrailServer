#include "CardEntity.h"
#include <boost/algorithm/string.hpp>
#include <vector>
CardEntity::CardEntity(string info)
{
	vector< string > cardEntry;
	boost::split(cardEntry, info, boost::is_any_of("\t"));
    this->id=atoi(cardEntry[0].c_str());
    this->type=cardEntry[1];
    this->element=cardEntry[2];
    this->property=cardEntry[3];
    this->name=cardEntry[4];
    this->hasSpeceiality = atoi(cardEntry[6].c_str());
    this->magicName = this->checkBasicMagic(this->name);
    int i;
    for(i=0;i<this->hasSpeceiality;i++)
        this->specialityList[i] = cardEntry[i+7];
}

int CardEntity::checkBasicMagic(string cardName)
{
    if(cardName == "Ê¥¶Ü")
        return SHIELDCARD;
    else if(cardName == "ÖÐ¶¾")
        return POISONCARD;
    else if(cardName == "Ä§µ¯")
        return MISSILECARD;
    else if(cardName == "ÐéÈõ")
        return WEAKCARD;
    return -1;
}

int CardEntity::getID()
{
    return this->id;
}
int CardEntity::getHasSpeciality()
{
    return this->hasSpeceiality;
}

string CardEntity::getType()
{
    return this->type;
}

string CardEntity::getElement()
{
    return this->element;
}

string CardEntity::getProperty()
{
    return this->property;
}

string CardEntity::getName()
{
    return this->name;
}

string* CardEntity::getSpecialityList()
{
    return this->specialityList;
}
