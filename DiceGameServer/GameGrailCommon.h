#pragma once
#include "CardEntity.h"
#include "GameGrailPlayerContext.h"
#include "zLogger.h"
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include "base.pb.h"
#include "action_respond.pb.h"

#include <list>

using namespace boost::interprocess;
using namespace network;
using namespace std;

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) { if (x) { delete (x); (x) = NULL; } }
#endif

class GameGrailPlayerContext;
typedef map< int, GameGrailPlayerContext* > PlayerContextList;

const int SUMMON[] = {1, 2, 3, 4, 5, 6, 7, 8, 9,10,
	                 11,12,13,14,15,16,17,18,19,20,
			         21,22,23,24,25,26,27,28,29,30,
					 31,108};
const int BASIC_ROLE[] = {1, 2, 3, 4, 5, 6, 7, 9,10,
	                 11,12,13,14,15,16,17,18,19,20,
					 21,22,23,24};
const int MO_DAO[] ={8};
const int SP_MO_DAO[] ={108};
const int FIRST_EXT[] = {26, 28, 29};
const int SECOND_EXT[] = {25, 27, 30, 31};
bool isValidRoleID(int roleID);

enum GrailError{
	GE_SUCCESS,
	GE_TIMEOUT,
	GE_URGENT,
	GE_EMPTY_HANDLE,	
	GE_DECK_OVERFLOW,
	GE_CARD_NOT_ENOUGH,
	GE_USERID_NOT_FOUND,
	GE_HANDCARD_NOT_FOUND,
	GE_BASIC_EFFECT_NOT_FOUND,
	GE_BASIC_EFFECT_ALREADY_EXISTS,
	GE_EXCLUSIVE_EFFECT_NOT_FOUND,
	GE_COVERCARD_NOT_FOUND,
	GE_MOVECARD_FAILED,
    GE_INCONSISTENT_STATE,
	GE_FATAL_ERROR,
	GE_INVALID_TABLEID,
	GE_INVALID_PLAYERID,
	GE_INVALID_CARDID,
	GE_INVALID_ROLEID,
	GE_INVALID_ACTION,
	GE_INVALID_STEP,
	GE_INVALID_EXCLUSIVE_EFFECT,
	GE_INVALID_ARGUMENT,
	GE_NO_STATE,
	GE_NO_CONTEXT,
	GE_NO_REPLY,
	GE_NOT_SUPPORTED,
	GE_PLAYER_FULL,
	GE_GUEST_FULL,
	GE_DISCONNECTED,
	GE_NOT_WELCOME,
	GE_WRONG_PASSWORD,
	GE_VIP_ONLY
};

enum CAUSE{
	CAUSE_DEFAULT,
	CAUSE_OVERLOAD,
	CAUSE_USE,
	CAUSE_ATTACK,
	CAUSE_POISON,
	CAUSE_WEAKEN,
	CAUSE_MISSILE,
	CAUSE_BUY,
	CAUSE_SYNTHESIZE,
	CAUSE_UNACTIONAL,
	JI_FENG_JI = 101,
	LIE_FENG_JI = 102,
	LIAN_XU_JI = 103,
	SHENG_JIAN = 104,
	JIAN_YING = 105,
	XUE_YING_KUANG_DAO = 201,
	XUE_XING_PAO_XIAO = 202,
	KUANG_HUA = 203,
	XUE_YING_KUANG_DAO_USED = 204,
	SI_LIE = 205,
	JING_ZHUN_SHE_JI = 301,
	SHAN_GUANG_XIAN_JING = 302,
	JU_JI = 303,
	SHAN_DIAN_JIAN = 304,
	GUAN_CHUAN_SHE_JI = 305,
	FENG_ZHI_FENG_YIN = 401,
	SHUI_ZHI_FENG_YIN = 402,
	HUO_ZHI_FENG_YIN = 403,
	DI_ZHI_FENG_YIN = 404,
	LEI_ZHI_FENG_YIN = 405,
	FA_SHU_JI_DONG = 406,
	WU_XI_SHU_FU = 407,
	FENG_YIN_PO_SUI = 408,
	FAN_SHI = 501,
	SHUI_YING = 502,
	QIAN_XING = 503,
	ZHI_LIAO_SHU = 601,
	ZHI_YU_ZHI_GUANG = 602,
	BING_SHUANG_DAO_YAN = 603,
	LIAN_MIN = 604,
	SHENG_LIAO = 605,
	TIAN_SHI_ZHI_QIANG = 701,
	TIAN_SHI_ZHU_FU = 702,
	FENG_ZHI_JIE_JING = 703,
	TIAN_SHI_JI_BAN = 704,
	TIAN_SHI_ZHI_GE = 705,
	SHEN_ZHI_BI_HU = 706,
	MO_BAO_CHONG_JI = 801,
	MO_DAN_ZHANG_WO = 802,
	MO_DAN_RONG_HE = 803,
	HUI_MIE_FENG_BAO = 804,
	SP_MO_BAO_CHONG_JI=805,
	FA_LI_HU_DUN=806,
	XIU_LUO_LIAN_ZHAN = 901,
    AN_YING_NING_JU = 902,
    AN_YING_ZHI_LI = 903,
    AN_YING_KANG_JU = 904,
    AN_YING_LIU_XING = 905,
    HEI_AN_ZHEN_CHAN = 906,
	HEI_AN_ZHEN_CHAN_BU_PAI = 9061,
	HUI_YAO = 1001,
    CHENG_JIE = 1002,
    SHENG_GUANG_QI_YU = 1003,
	TIAN_QIANG = 1004,
	DI_QIANG = 1005,
	SHEN_SHENG_XIN_YANG = 1006,
	SHENG_JI = 1007,
	FENG_REN = 1101,
	BING_DONG = 1102,
	HUO_QIU = 1103,
	YUN_SHI = 1104,
	LEI_JI = 1105,
	YUAN_SU_DIAN_RAN = 1106,
	YUE_GUANG = 1107,
	YUAN_SU_XI_SHOU = 1108,
	QI_ZHA = 1201,
	MAO_XIAN_ZHE_TIAN_TANG = 1202,
	TOU_TIAN_HUAN_RI = 1203,
	TE_SHU_JIA_GONG = 1204,
	BU_XIU = 1301,
	SHENG_DU = 1302,
	WEN_YI = 1303,
	SI_WANG_ZHI_CHU = 1304,
	MU_BEI_YUN_LUO = 1305,
	YI_SHI_ZHONG_DUAN = 1401,
	ZHONG_CAI_YI_SHI = 1402,
	MO_RI_SHEN_PAN = 1403,
	SHEN_PAN_LANG_CHAO = 1404,
	PAN_JUE_TIAN_PING = 1405,
	SHEN_SHENG_QI_SHI = 1501,
	SHEN_SHENG_QI_FU = 1502,
	SHUI_ZHI_SHEN_LI = 1503,
	SHENG_SHI_SHOU_HU = 1504,
	SHEN_SHENG_QI_YUE = 1505,
	SHEN_SHENG_LING_YU = 1506,
	SHUI_ZHI_SHEN_LI_GIVE = 1531,
	SHUI_ZHI_SHEN_LI_CROSS = 1532,
	XUN_JIE_CI_FU = 1601,
	WEI_LI_CI_FU = 1602,
	QI_DAO = 1603,
	GUANG_HUI_XIN_YANG = 1604,
	QI_HEI_XIN_YANG = 1605,
	FA_LI_CHAO_XI = 1606,
	ZHI_HUI_FA_DIAN = 1701,
	MO_DAO_FA_DIAN = 1702,
	SHENG_JIE_FA_DIAN = 1703,
	FA_SHU_FAN_TAN = 1704,
	FENG_XING = 1801,
	LEI_MING = 1802,
	NIAN_ZHOU = 1803,
	BAI_GUI_YE_XING = 1804,
	LING_LI_BENG_JIE = 1805,
    JIAN_HUN_SHOU_HU=1901,
    YANG_GONG=1902,
    JIAN_QI_ZHAN=1903,
    TIAN_SHI_YU_E_MO=1904,
    TIAN_SHI_ZHI_HUN=1905,
    E_MO_ZHI_HUN=1906,
    BU_QU_YI_ZHI=1907,
	NIAN_QI_LI_CHANG = 2001,
	XU_LI_YI_JI = 2002,
	NIAN_DAN = 2003,
	NIAN_DAN_SELF = 2031,
	BAI_SHI_HUAN_LONG_QUAN = 2004,
	CANG_YAN_ZHI_HUN = 2005,
	DOU_SHEN_TIAN_QU = 2006,
	XU_LI_CANG_YAN = 2025,
	BAI_SHI_DOU_SHEN = 2046,
	TIAO_XIN = 2101,
    JIN_DUAN_ZHI_LI = 2102,
	NU_HOU = 2103,
	MING_JING_ZHI_SHUI = 2104,
	SI_DOU = 2105,
	JING_PI_LI_JIE = 2106,
    LING_HUN_ZHEN_BAO =2201,
    LING_HUN_CI_YU =2202,
    LING_HUN_ZENG_FU=2203,
    LING_HUN_TUN_SHI=2204,
    LING_HUN_ZHAO_HUAN=2205,
    LING_HUN_ZHUAN_HUAN=2206,
    LING_HUN_JING_XIANG=2207,
    LING_HUN_LIAN_JIE=2208,
	LING_HUN_LIAN_JIE_REACT=2209,
	XUE_ZHI_BEI_MING = 2301,
	TONG_SHENG_GONG_SI = 2302,
	XUE_ZHI_AI_SHANG = 2303,
	XUE_ZHI_AI_SHANG_HARM = 2331,
	NI_LIU = 2304,
	XUE_ZHI_ZU_ZHOU = 2351,
	XUE_ZHI_ZU_ZHOU_QI_PAI = 2352,
	LIU_XUE = 2306,
	SHENG_MING_ZHI_HUO=2401,
    WU_DONG=2402,
    WU_DONG_EXTRA=24021,
    DU_FEN=2403,
    CHAO_SHENG=2404,
    JING_HUA_SHUI_YUE=2405,
    DIAO_LING=2406,
    YONG_HUA=2407,
    DAO_NI_ZHI_DIE=2408,
	SHEN_SHENG_ZHUI_JI = 2501,
	ZHI_XU_ZHI_YIN = 2502,
	HE_PING_XING_ZHE = 2503,
	JUN_SHEN_WEI_GUANG = 2504,
	YING_LING_ZHAO_HUAN = 2505,
	MO_GUAN_CHONG_JI=2601,
    MO_GUAN_CHONG_JI_HIT=26011,
    LEI_GUANG_SAN_SHE =2602,
    LEI_GUANG_SAN_SHE_EXTRA=26021,
    DUO_CHONG_SHE_JI=2603,
    CHONG_NENG=2604,
    CHONG_NENG_GAI_PAI=26041,
    MO_YAN=2605,
    MO_YAN_GAI_PAI=26051,
    CHONG_NENG_MO_YAN=2606,
	NU_HUO_MO_WEN = 2701,
	NU_HUO_YA_ZHI = 2702,
	ZHAN_WEN_SUI_JI = 2703,
	MO_WEN_RONG_HE = 2704,
	FU_WEN_GAI_ZAO = 2705,
	FU_WEN_GAI_ZAO_TOKEN = 27051,
	SHUANG_CHONG_HUI_XIANG =2706,
	XING_HONG_SHENG_YUE = 2801,
    XING_HONG_XIN_YANG = 2802,
	XUE_XING_DAO_YAN = 2803,
    XUE_XING_DAO_YAN_1 = 2804,
	XUE_XING_DAO_YAN_2 = 2805,
	SHA_LU_SHENG_YAN = 2806,
	RE_XUE_FEI_TENG = 2807,
	JIE_JIAO_JIE_ZAO = 2808,
	JIE_JIAO_JIE_ZAO_AFTER_MAGIC = 28081,
	XING_HONG_SHI_ZI =2809,
	AN_ZHI_JIE_FANG=2901,
    HUAN_YING_XING_CHEN=2902,
	HUAN_YING_XING_CHEN_EFFECT=29021,
    HEI_AN_SHU_FU=2903,
    AN_ZHI_ZHANG_BI=2904,
    CHONG_YING=2905,
	CHONG_YING_DISCARD=29051,
    QI_HEI_ZHI_QIANG=2906,
	AN_ZHI_HUAN_YING=2907,
	CANG_YAN_FA_DIAN=3001,
	TIAN_HUO_DUAN_KONG=3002,
	MO_NV_ZHI_NU=3003,
	MO_NV_ZHI_NU_ATTACK=30031,
	MO_NV_ZHI_NU_DRAW=30032,
	TI_SHEN_WAN_OU=3004,
	YONG_SHENG_YIN_SHI_JI=3005,
	TONG_KU_LIAN_JIE=3006,
	TONG_KU_LIAN_JIE_CARD=30061,
	MO_NENG_FAN_ZHUAN=3007,
	CHEN_LUN_XIE_ZOU_QU=3101,
	BU_XIE_HE_XIAN=3102,
	GE_YONG_TIAN_FU=3103,
	BAO_FENG_QIAN_ZOU_QU=3104,
	JI_ANG_KUANG_XIANG_QU=3105,
	JI_ANG_KUANG_XIANG_QU_STONE=31051,
	JI_ANG_KUANG_XIANG_QU_2=31052,
	JI_ANG_KUANG_XIANG_QU_HARM=31053,
	SHENG_LI_JIAO_XIANG_SHI=3106,
	SHENG_LI_JIAO_XIANG_SHI_2=31061,
	SHENG_LI_JIAO_XIANG_SHI_STONE=31062,
	XI_WANG_FU_GE_QU=3107
};

enum CHANGE{
	CHANGE_ADD,
	CHANGE_REMOVE
};

enum DECK{
	DECK_PILE = 1, 
	DECK_DISCARD = 2,
	DECK_HAND = 4,
	DECK_BASIC_EFFECT = 5,
	DECK_COVER = 6
};

enum HARM_TYPE{
	HARM_NONE,
	HARM_ATTACK,
	HARM_MAGIC
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

enum SPECIAL_ACTION{
	SPECIAL_BUY,
	SPECIAL_SYNTHESIZE,
	SPECIAL_EXTRACT
};

enum STEP{
	STEP_INIT = 0,
	STEP_DONE = 9999
};

#define CARDSUM 150
#define CARDBUF 30
#define MAXPLAYER 8
#define MAXROLES 20
const int BP_ALTERNATIVE_NUM[] = {12,16,20};
extern CardEntity* cardList[CARDSUM];
CardEntity* getCardByID(int id);
#define RESULT_UNKNOWN 30000
#define RESULT_WIN 1
#define RESULT_LOSE 0
#define RED  1
#define BLUE 0


//应战行动，注意BLOCKED指圣光和应战，HIT指命中或圣盾（由server判定）
#define REPLYBATTLE  0
#define BLOCKED      1
#define HIT          2

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

typedef struct{	
	int allowAction;
	int cause;
}ACTION_QUOTA;

#define TOQSTR(x)  boost::lexical_cast<std::string>(x)

string combMessage(string item1,string item2 = "",string item3 = "",string item4 = "",string item5 = "",string item6 = "",string item7 = "");
//编码器,编辑各类通讯信息
class Coder
{
public:
	static void logInResponse(int state, string nickname, LoginResponse& rep)
	{
		rep.set_state(state);
		rep.set_nickname(nickname);
	}
    static void askForReBat(int type,int cardID,int dstID,int srcID, CommandRequest& cmd_req)
	{
		cmd_req.set_cmd_type(CMD_RESPOND);
		Command *cmd;

		cmd = cmd_req.add_commands();
		cmd->set_respond_id(network::RESPOND_REPLY_ATTACK);
		cmd->add_args(type);
		cmd->add_args(cardID);
		cmd->add_args(dstID);
		cmd->add_args(srcID);
	}
    static void askForDiscard(int ID, int sum, int cause, bool show, CommandRequest& cmd_req)
	{
		cmd_req.set_cmd_type(CMD_RESPOND);
		Command *cmd;

		cmd = cmd_req.add_commands();
		cmd->set_respond_id(network::RESPOND_DISCARD);
		cmd->add_dst_ids(ID);
		cmd->add_args(cause);
		cmd->add_args(sum);
		cmd->add_args(show ? 1 : 0);
	}
	static void askForDiscardCover(int ID, int sum, int cause, CommandRequest& cmd_req)
	{
		cmd_req.set_cmd_type(CMD_RESPOND);
		Command *cmd;

		cmd = cmd_req.add_commands();
		cmd->set_respond_id(network::RESPOND_DISCARD_COVER);
		cmd->add_dst_ids(ID);
		cmd->add_args(cause);
		cmd->add_args(sum);
	}
	static void handNotice(int ID, list<int> handCards, GameInfo& game_info)
	{
		SinglePlayerInfo* player_info = game_info.add_player_infos();
		player_info->set_id(ID);

		list<int>::iterator it;
		for (it = handCards.begin(); it != handCards.end(); ++it)
		{
			player_info->add_hands(*it);
		}
		player_info->set_hand_count(handCards.size());
		if (handCards.size() == 0)
			player_info->add_delete_field("hands");
	}
	static void coverNotice(int ID, list<int> coverCards, GameInfo& game_info)
	{
		SinglePlayerInfo* player_info = game_info.add_player_infos();
		player_info->set_id(ID);

		list<int>::iterator it;
		for (it = coverCards.begin(); it != coverCards.end(); ++it)
		{
			player_info->add_covereds(*it);
		}
		player_info->set_covered_count(coverCards.size());
		if (coverCards.size() == 0)
			player_info->add_delete_field("covereds");
	}
	static void basicNotice(int ID, list<BasicEffect> basicCards, GameInfo& game_info)
	{
		SinglePlayerInfo* player_info = game_info.add_player_infos();
		player_info->set_id(ID);

		list<BasicEffect>::iterator it;
		for (it = basicCards.begin(); it != basicCards.end(); ++it)
		{
			player_info->add_basic_cards(it->card);
		}
		if (basicCards.size() == 0)
			player_info->add_delete_field("basic_cards");
	}
	static void exclusiveNotice(int ID, bool* exclusive, GameInfo& game_info)
	{
		SinglePlayerInfo* player_info = game_info.add_player_infos();
		player_info->set_id(ID);
		bool hasEx = false;
		for (int i = 0; i < EXCLUSIVE_NUM; i++)
		{
			if(exclusive[i]){
				player_info->add_ex_cards(i);
				hasEx = true;
			}
		}
		if (!hasEx)
			player_info->add_delete_field("ex_cards");
	}
    static void hitNotice(int result,int isActiveAttack,int dstID,int srcID, HitMsg& hit_msg)
	{
		hit_msg.set_cmd_id(isActiveAttack);
		hit_msg.set_hit(result);
		hit_msg.set_src_id(srcID);
		hit_msg.set_dst_id(dstID);
	}
    static void stoneNotice(int color, int gem, int crystal, GameInfo& update_info)
	{
	    if(color == RED){
	        update_info.set_red_gem(gem);
			update_info.set_red_crystal(crystal);
		}
		else{
		    update_info.set_blue_gem(gem);
			update_info.set_blue_crystal(crystal);
		}
	}
    static void energyNotice(int ID, int gem, int crystal, GameInfo& update_info)
	{
		SinglePlayerInfo* player_info = update_info.add_player_infos();
		player_info->set_id(ID);
		player_info->set_gem(gem);
		player_info->set_crystal(crystal);
	}
	static void crossNotice(int ID, int cross, GameInfo& game_info)
	{
		SinglePlayerInfo* player_info = game_info.add_player_infos();
		player_info->set_id(ID);
		player_info->set_heal_count(cross);
	}
    static void hurtNotice(int dstID, int srcID, int type, int point, int cause, HurtMsg& hurt_msg)
	{
		hurt_msg.set_dst_id(dstID);
		hurt_msg.set_src_id(srcID);
		hurt_msg.set_type(type);
		hurt_msg.set_hurt(point);
		hurt_msg.set_cause(cause);
	}
	static void askForWeak(int ID, int howMany, CommandRequest& cmd_req)
	{
		cmd_req.set_cmd_type(CMD_RESPOND);
		Command *cmd;

		cmd = cmd_req.add_commands();
		cmd->set_respond_id(network::RESPOND_WEAKEN);
		cmd->add_args(ID);
		cmd->add_args(howMany);
	}
    static void askForMissile(int dstID,int srcID,int hurtSum,int nextID, CommandRequest& cmd_req)
	{
		cmd_req.set_cmd_type(CMD_RESPOND);
		Command *cmd;

		cmd = cmd_req.add_commands();
		cmd->set_respond_id(network::RESPOND_BULLET);
		cmd->add_args(dstID);
		cmd->add_args(srcID);
		cmd->add_args(hurtSum);
		cmd->add_args(nextID);
	}
    static void useCardNotice(int cardID, int dstID, int srcID, CardMsg& use_card, bool realCard = true)
	{
		use_card.set_type(CM_USE);
		use_card.set_dst_id(dstID);
		use_card.set_src_id(srcID);
		use_card.add_card_ids(cardID);
		use_card.set_is_real(realCard);
	}
	static void showCardNotice(int ID, int howMany, int cardID, CardMsg& show_card)
	{
		show_card.set_type(CM_SHOW);
		show_card.set_src_id(ID);
		show_card.add_card_ids(cardID);
	}
	static void showCardNotice(int ID, int howMany, vector<int> cards, CardMsg& show_card)
	{
		show_card.set_type(CM_SHOW);
		show_card.set_src_id(ID);
		for (int i = 0; i < howMany; i++){
			show_card.add_card_ids(cards[i]);
		}
	}
	static void askForAction(int playerID, int actionTypeAllowed, bool canGiveUp, CommandRequest& cmd_req)
	{
		cmd_req.set_cmd_type(CMD_ACTION);
		Command *cmd;

		if (canGiveUp) {
			cmd = cmd_req.add_commands();
			cmd->set_respond_id(ACTION_NONE);
			cmd->set_src_id(playerID);
		}

		cmd = cmd_req.add_commands();
		cmd->set_respond_id(actionTypeAllowed);
		cmd->set_src_id(playerID);
	}
    static void askForAdditionalAction(int playerID, list<ACTION_QUOTA> quota, CommandRequest& cmd_req)
    {
        cmd_req.set_cmd_type(CMD_RESPOND);
		Command *cmd;

        list<ACTION_QUOTA>::iterator it;
        cmd = cmd_req.add_commands();
		cmd->set_respond_id(network::RESPOND_ADDITIONAL_ACTION);
		cmd->set_src_id(playerID);
        for(it = quota.begin(); it != quota.end(); it++){
            cmd->add_args(it->cause);
        }
		
    }
    static void askForCross(int playerID,int hurtPoint,int type, int crossAvailable, CommandRequest& cmd_req)
	{
		cmd_req.set_cmd_type(CMD_RESPOND);
		Command *cmd;

		cmd = cmd_req.add_commands();
		cmd->set_respond_id(network::RESPOND_HEAL);
		cmd->add_args(playerID);
		cmd->add_args(hurtPoint);
		cmd->add_args(type);
		cmd->add_args(crossAvailable);
	}
	//只填基本信息，若需其他arg，请调用完后再自填
    static void askForSkill(int playerID, int skillID, CommandRequest& cmd_req)
	{
		cmd_req.set_cmd_type(CMD_RESPOND);
		Command *cmd;

		cmd = cmd_req.add_commands();
		cmd->set_respond_id(skillID);
		cmd->set_src_id(playerID);
	}
	static void skillNotice(int srcID, int dstID, int skillID, SkillMsg& skill_msg)
	{
		list<int> dstIDs;
		dstIDs.push_back(dstID);
		skillNotice(srcID, dstIDs, skillID, skill_msg);
	}
	static void skillNotice(int srcID, list<int> dstIDs, int skillID, SkillMsg& skill_msg)
	{
		skill_msg.set_src_id(srcID);
		skill_msg.set_skill_id(skillID);
		list<int>::iterator it;
		for (it = dstIDs.begin(); it != dstIDs.end(); ++it)
		{
			skill_msg.add_dst_ids(*it);
		}
	}
    static void roleNotice(int playerID,int roleID, GameInfo& game_info)
	{
		SinglePlayerInfo* player_info;
		for (int i = 0; i < game_info.player_infos_size(); ++i)
		{
			player_info = (SinglePlayerInfo*)&(game_info.player_infos(i));
			if (player_info->id() == playerID)
			{
				player_info->set_role_id(roleID);
				break;
			}
		}
	}
    static void handcardMaxNotice(int ID,int howMany, GameInfo& game_info)
	{
		SinglePlayerInfo* player_info = game_info.add_player_infos();
		player_info->set_id(ID);
		player_info->set_max_hand(howMany);
	}
    static void tapNotice(int ID, bool flag, GameInfo& game_info)
	{
		SinglePlayerInfo* player_info = game_info.add_player_infos();
		player_info->set_id(ID);
		player_info->set_is_knelt(flag);
	}
    static void tokenNotice(int ID,int tokenID,int howMany, GameInfo& game_info)
	{
		SinglePlayerInfo* player_info = game_info.add_player_infos();
		player_info->set_id(ID);
		if (tokenID == 0)
			player_info->set_yellow_token(howMany);
		else
			player_info->set_blue_token(howMany);
	}
	static void askForRole(int ID, int howMany, const int *roles, RoleRequest& cmd_req)
	{
		cmd_req.set_id(ID);
		cmd_req.set_strategy(ROLE_STRATEGY_31);
		for(int i = 0; i < howMany; i++){
			cmd_req.add_role_ids(roles[i]);
		}
	}
	static void setAlternativeRoles(int ID, int howMany, const int *roles, const int *chosen, RoleRequest& cmd_req)
	{
		cmd_req.set_id(ID);
		cmd_req.set_strategy(ROLE_STRATEGY_BP);
		for(int i = 0; i < howMany; i ++){
			cmd_req.add_role_ids(roles[i]);
			cmd_req.add_args(chosen[i]);
		}
	}
	static void roomInfo(PlayerContextList players, list< int > teamA, list< int > teamB, GameInfo& room_info)
	{
		SinglePlayerInfo *player_info;

		for(PlayerContextList::iterator it = players.begin(); it != players.end(); it++)
		{
			int id = it->first;
			GameGrailPlayerContext* context = it->second;
			if(context->isConnected()){	
				player_info = room_info.add_player_infos();
				player_info->set_id(id);						
				player_info->set_nickname(context->getName());
				player_info->set_ready(context->isReady());
				if(teamA.end() != std::find(teamA.begin(), teamA.end(), id))
					player_info->set_team(1);
				else if(teamB.end() != std::find(teamB.begin(), teamB.end(), id))
					player_info->set_team(0);
			}
		}
	}
	static void errorMsg(int id, int dstId, Error& error)
	{
		error.set_id(id);
		error.set_dst_id(dstId);
	}
};

CardEntity* getCardByID(int id);