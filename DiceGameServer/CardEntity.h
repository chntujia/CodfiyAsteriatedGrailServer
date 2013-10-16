#pragma once
#include <iostream>
using namespace std;

struct BasicEffect
{
    int card;
    int srcUser;
};

enum CardType{
	TYPE_ATTACK = 1,
	TYPE_MAGIC = 2,
};

enum CardProperty{
	PROPERTY_BLOOD = 1,
	PROPERTY_MARTIAL = 2,
	PROPERTY_PHANTOM = 3,
	PROPERTY_CAST = 4,
	PROPERTY_HOLY = 5,
};

enum CardElement{
	ELEMENT_WIND = 1,
	ELEMENT_WATER = 2,
	ELEMENT_FIRE = 3,
	ELEMENT_EARTH = 4,
	ELEMENT_THUNDER = 5,
	ELEMENT_LIGHT = 6,
	ELEMENT_DARKNESS = 7,
};

enum CardName{
	NAME_POISON = 1,
	NAME_WEAKEN = 2,
	NAME_SHIELD = 3,
	NAME_MISSILE = 4,
	NAME_HOLYLIGHT = 5,
	NAME_WINDSLASH = 6,
	NAME_WATERSLASH = 7,
	NAME_FIRESLASH = 8,
	NAME_EARTHSLASH = 9,
	NAME_THUNDERSLASH = 10,
	NAME_DARKSLASH = 11,
};

//卡牌类
class CardEntity
{
public:
    //构造,输入参数是包含卡牌信息的一列String
    CardEntity(string cardEntry);
	int getID() { return id; }   
    //获取攻击/法术类别
	int getType() { return type; }
    //获取卡牌属性
	int getElement() { return element; }
    //获取卡牌技、血、咏、圣、幻类别
	int getProperty() { return property; }
    //获取卡牌名称
	int getName() { return name; }
	//获取卡牌独有技信息
	int getHasSpeciality() { return hasSpeciality; }
    //获取卡牌独有技
	int getSpeciality(int id) { return specialityList[id]; }
private:
    int id;
    int hasSpeciality;
    int type;
    int element;
    int property;
    int name;
    int specialityList[2];
};

