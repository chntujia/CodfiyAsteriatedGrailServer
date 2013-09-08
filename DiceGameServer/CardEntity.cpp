#include "CardEntity.h"
#include <boost/algorithm/string.hpp>
#include <vector>
CardEntity::CardEntity(string info)
{
	vector< string > cardEntry;
	boost::split(cardEntry, info, boost::is_any_of("\t"));
    id=atoi(cardEntry[0].c_str());
    type=atoi(cardEntry[1].c_str());
    element=atoi(cardEntry[2].c_str());
    property=atoi(cardEntry[3].c_str());
    name=atoi(cardEntry[4].c_str());
/*    hasSpeceiality = atoi(cardEntry[5].c_str());
    int i;
    for(i=0;i<this->hasSpeceiality;i++)
        specialityList[i] = atoi(cardEntry[6+i].c_str());
		*/
}


