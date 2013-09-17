#pragma once
#include "CardEntity.h"
#include "zLogger.h"
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace boost::interprocess;

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) { if (x) { delete (x); (x) = NULL; } }
#endif

enum GrailError{
	GE_SUCCESS,
	GE_TIMEOUT,
	GE_CONTINUE,
	GE_DONE_AND_URGENT,
	GE_EMPTY_HANDLE,
	GE_NO_STATE,
	GE_DECK_OVERFLOW,
	GE_CARD_NOT_ENOUGH,
	GE_HANDCARD_NOT_FOUND,
	GE_BASIC_EFFECT_NOT_FOUND,
	GE_MOVECARD_FAILED,
    GE_INCONSISTENT_STATE,
	GE_FATAL_ERROR,
	GE_INVALID_PLAYERID,
	GE_INVALID_CARDID,
	GE_NOT_SUPPORTED,
	GE_INVALID_ARGUMENT,
	GE_NO_CONTEXT,
	GE_NO_REPLY
};

enum DECK{
	DECK_PILE = 1, 
	DECK_DISCARD = 2,
	DECK_HAND = 4,
	DECK_BASIC_EFFECT = 5,
	DECK_COVER = 6,
};

enum HARM_TYPE{
	HARM_NONE,
	HARM_ATTACK,
	HARM_MAGIC
};

enum ACTION_FLAG{
	ANY_ACTION,
	ATTACK_ACTION,
	MAGIC_ACTION,
	SPECIAL_ACTION,
	ATTACK_MAGIC
};

enum HIT_RATE{
	RATE_NORMAL,
	RATE_NOREATTACK,
	RATE_NOMISS
};

enum REATTACK{
	RA_ATTACK,
	RA_BLOCK,
	RA_GIVEUP
};

#define CARDSUM 150
#define CARDBUF 30
#define MAXPLAYER 8
extern CardEntity* cardList[CARDSUM];
CardEntity* getCardByID(int id);
#define RED  1
#define BLUE 0

//应战行动，注意BLOCKED指圣光和应战，HIT指命中或圣盾（由server判定）
#define REPLYBATTLE  0
#define BLOCKED      1
#define HIT          2

//伤害类型
#define NOTHARMED  0
#define ATTACKHARM 1
#define MAGICHARM  2

//攻击类别，普通、无法应战、必定命中
#define NORMAL  0
#define NOREPLY 1
#define NOMISS  2

//特殊行动
#define BUY     0
#define SYNTHESIZE 1
#define EXTRACT    2

//通讯协议号
#define COMMONMAGIC 0
#define BEGINNOTICE 2
#define TURNBEGINNOTICE 3
#define ACTIONCOMMAND 4
#define ASKFORREBAT 5
#define REBATCOMMAND 6
#define ASKFORDISCARD 7
#define DISCARDCOMMAND 8
#define DRAWNOTICE 9
#define RESHUFFLE 10
#define MORALENOTICE 11
#define ENDNOTICE 12
#define DISCARDNOTICE 13
#define HITNOTICE 14
#define STONENOTICE 15
#define READYBEGIN 16
#define CUPNOTICE 17
#define ENERGYNOTICE 18
#define MOVECARDNOTICE 19
#define ATTACKHURTNOTICE 20
#define MAGICHURTNOTICE 21
#define ASKFORWEAK 22
#define WEAKCOMMAND 23
#define WEAKNOTICE 24
#define SHIELDNOTICE 25
#define ASKFORMISSILE 26
#define MISSILECOMMAND 27
#define USECARDNOTICE 28
#define ASKFORACTION 29
#define CROSSCHANGENOTICE 32
#define ASKFORCROSS 33
#define ANSFORCROSS 34
#define ASKFORSKILL 35
#define ANSFORSKILL 36
#define CHARACTERNOTICE 37
#define UNACTIONAL 30
#define UNACTIONALNOTICE 31
#define NOTICE 38

typedef struct{
	int type;
	int point;
	int srcID;
	int cause;
}HARM;

#define TOQSTR(x)  boost::lexical_cast<std::string>(x)

string combMessage(string item1,string item2 = "",string item3 = "",string item4 = "",string item5 = "",string item6 = "",string item7 = "");
//编码器,编辑各类通讯信息
class Coder
{
public:
    static string beginNotice(string seatCode){return "2;" + seatCode + ";";}
    static string turnBegineNotice(int ID){return "3;" + TOQSTR(ID) + ";";}
    static string askForReBat(int type,int cardID,int dstID,int srcID){return combMessage("5",TOQSTR(type),TOQSTR(cardID),TOQSTR(dstID),TOQSTR(srcID));}
    static string askForDiscard(int ID,int sum, bool show){return show?combMessage("7",TOQSTR(ID),TOQSTR(sum),"1"):combMessage("7",TOQSTR(ID),TOQSTR(sum),"0");}
    static string askForDiscover(int ID, int sum,string show){return combMessage("49",TOQSTR(ID),TOQSTR(sum),show);}
    static string drawNotice(int ID,int howMany,vector < int > cards);
    static string reshuffleNotice(int howManyNew){return combMessage("10",TOQSTR(howManyNew));}
    static string moraleNotice(int color,int value){return combMessage("11",TOQSTR(color),TOQSTR(value));}
    static string endNotice(int winColor){return "12;" + TOQSTR(winColor) + ";";}
    static string discardNotice(int ID,int sum,string show,vector < int > cards);
    static string hitNotice(int result,int isActiveAttack,int dstID,int srcID){return combMessage("14",TOQSTR(result),TOQSTR(isActiveAttack),TOQSTR(dstID),TOQSTR(srcID));}
    static string stoneNotice(int color,int gem,int crystal){return combMessage("15",TOQSTR(color),TOQSTR(gem),TOQSTR(crystal));}
    static string cupNotice(int color,int cup){return combMessage("17",TOQSTR(color),TOQSTR(cup));}
    static string energyNotice(int ID,int gem,int crystal){return combMessage("18",TOQSTR(ID),TOQSTR(gem),TOQSTR(crystal));}
    static string moveCardNotice(int sum,vector < int > cards,int srcID,int srcArea,int dstID,int dstArea);
    static string getCardNotice(int sum,vector < int > cards,int dstID,bool show);
    static string attackHurtNotice(int dstID,int sum,int srcID){return combMessage("20",TOQSTR(dstID),TOQSTR(sum),TOQSTR(srcID));}
    static string magicHurtNotice(int dstID,int sum,int srcID,string reason){return combMessage("21",TOQSTR(dstID),TOQSTR(sum),TOQSTR(srcID),reason);}
    static string askForWeak(int ID,int howMany){return "22;" + TOQSTR(ID) + ";"+ TOQSTR(howMany) + ";";}
    static string weakNotice(int ID,int act,int howMany=3){return combMessage("24",TOQSTR(ID),TOQSTR(act),TOQSTR(howMany));}
    static string shieldNotic(int ID){return "25;" + TOQSTR(ID) + ";";}
    static string askForMissile(int dstID,int srcID,int hurtSum,int nextID){return combMessage("26",TOQSTR(dstID),TOQSTR(srcID),TOQSTR(hurtSum),TOQSTR(nextID));}
    static string useCardNotice(int cardID, int dstID, int srcID, int realCard=1);
	static string askForAction(int playerID,int actionTypeAllowed,bool acted){return combMessage(TOQSTR(ASKFORACTION),TOQSTR(playerID),TOQSTR(actionTypeAllowed),acted?"1":"0");}
    static string askForAdditionalAction(int playerID){return "42;"+TOQSTR(playerID)+";";}
    static string crossChangeNotice(int playerID,int newValue){return combMessage(TOQSTR(CROSSCHANGENOTICE),TOQSTR(playerID),TOQSTR(newValue));}
    static string askForCross(int playerID,int hurtPoint,int type, int crossAvailable){return combMessage(TOQSTR(ASKFORCROSS),TOQSTR(playerID),TOQSTR(hurtPoint),TOQSTR(type),TOQSTR(crossAvailable));}
    static string askForSkill(int playerID,string content,string args=""){return combMessage(TOQSTR(ASKFORSKILL),TOQSTR(playerID),content,args);}
    static string roleNotice(int playerID,int roleID,int show=0){return combMessage(TOQSTR(CHARACTERNOTICE),TOQSTR(playerID),TOQSTR(roleID),TOQSTR(show));}
    static string unactionalNotice(int playerID){return combMessage(TOQSTR(UNACTIONALNOTICE),TOQSTR(playerID));}
    static string notice(string content){return combMessage(TOQSTR(NOTICE),content);}
    static string askForDiscardMagic(int ID){return combMessage("850",TOQSTR(ID));}
    static string askToGiveCard(int ID,int n){return combMessage("750",TOQSTR(ID),TOQSTR(n));}
    static string askForChongYing(int ID,int color){return combMessage("2950",TOQSTR(ID),TOQSTR(color));}
    static string handcardMaxNotice(int ID,int howMany){return combMessage("40",TOQSTR(ID),TOQSTR(howMany));}
    static string tapNotice(int ID,int flag,string content){return combMessage("39",TOQSTR(ID),TOQSTR(flag),content);}
    static string specialNotice(int ID,int type,int flag){return combMessage("43",TOQSTR(ID),TOQSTR(type),TOQSTR(flag));}
    static string tokenNotice(int ID,int tokenID,int howMany){return combMessage("45",TOQSTR(ID),TOQSTR(tokenID),TOQSTR(howMany));}
    static string askForRolePick(int howMany,int *roles);
    static string coverCardNotice(int playerID,int howMany,vector < int > cards,bool remove,bool show);
    static string askForSkillNumber(int playerID,int skillNum){return combMessage(TOQSTR(skillNum));}
    static string optionalRoleNotice(int num, int *roles);
    static string askForBan(int ID);
    static string banNotice(int ID, int role);
    static string askForPick(int ID);
    static string pickNotice(int ID, int role);
    static string nicknameNotice(int id,string name){return combMessage("58",TOQSTR(id),name);}

};

CardEntity* getCardByID(int id);