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

