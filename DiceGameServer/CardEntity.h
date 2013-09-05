#pragma once
#include <iostream>
using namespace std;

#define SHIELDCARD 0
#define POISONCARD 1
#define WEAKCARD 2
#define MISSILECARD 3
#define LIGHTCARD 4

#define NOTEXIST           0
#define PILE               1
#define DISCARDPILE        2
#define DISCARDPILECOVERED 3
#define HAND               4
#define EFFECT             5
#define COVERED            6

//卡牌类
class CardEntity
{
public:
    //构造,输入参数是包含卡牌信息的一列String
    CardEntity(string cardEntry);
    int getID();
    //获取卡牌独有技信息
    int getHasSpeciality();
    //获取攻击/法术类别
    string getType();
    //获取卡牌属性
    string getElement();
    //获取卡牌技、血、咏、圣、幻类别
    string getProperty();
    //获取卡牌名称，注意是中文
    string getName();
    //获取卡牌独有技列表
    string* getSpecialityList();
    //将法术牌名称转为int
    int getMagicName(){return this->magicName;}
    void setMagicName(int name){this->magicName = name;}
    //核查卡牌是否是基本法术牌
    int checkBasicMagic(string cardName);
protected:
    int id;
    int hasSpeceiality;
    string type;
    string element;
    string property;
    string name;
    string specialityList[2];
    int magicName;
};

