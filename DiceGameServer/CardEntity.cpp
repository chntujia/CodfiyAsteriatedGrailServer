#include "CardEntity.h"
#include <boost/algorithm/string.hpp>
#include <vector>
CardEntity::CardEntity(std::string info)
{
	std::vector< std::string > cardEntry;
	boost::split(cardEntry, info, boost::is_any_of("\t"));
    id=atoi(cardEntry[0].c_str());
    type=atoi(cardEntry[1].c_str());
    element=atoi(cardEntry[2].c_str());
    property=atoi(cardEntry[3].c_str());
    name=atoi(cardEntry[4].c_str());
    specialityCount = atoi(cardEntry[5].c_str());
    int i;
    for(i=0;i<this->specialityCount && i<2;i++){
        specialityList[i] = atoi(cardEntry[6+i].c_str());
	}
}

bool CardEntity::checkSpeciality(int speciality)
{
	if(specialityCount == 0){
		return false;
	}
	if(specialityCount == 1){
		return specialityList[0] == speciality;
	}
	if(specialityCount == 2){
		return specialityList[0] == speciality || specialityList[1] == speciality;
	}
	return false;
}

