#pragma once
#include <iostream>
using namespace std;

enum CardType{
	ATTACK = 1,
	MAGIC = 2,
};

enum CardProperty{
	BLOOD = 1,
	MARTIAL = 2,
	PHANTOM = 3,
	CAST = 4,
	HOLY = 5,
};

enum CardElement{
	WIND = 1,
	WATER = 2,
	FIRE = 3,
	EARTH = 4,
	THUNDER = 5,
	LIGHT = 6,
	DARKNESS = 7,
};

enum CardName{
	POISON = 1,
	WEAKEN = 2,
	SHIELD = 3,
	MISSILE = 4,
	HOLYLIGHT = 5,
	WINDSLASH = 6,
	WATERSLASH = 7,
	FIRESLASH = 8,
	EARTHSLASH = 9,
	THUNDERSLASH = 10,
	DARKSLASH = 11,
};

//卡牌类
class CardEntity
{
public:
    //构造,输入参数是包含卡牌信息的一列String
    CardEntity(string cardEntry);
    int id;
    int hasSpeceiality;
    int type;
    int element;
    int property;
    int name;
    int specialityList[2];
};

