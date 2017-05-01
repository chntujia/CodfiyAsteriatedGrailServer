#pragma once
#include <string>

#define TABLEDETAIL_COL_START 0
#define TABLELOG_COL_ID TABLEDETAIL_COL_START + 1
#define TABLELOG_COL_TABLEMODE TABLELOG_COL_ID + 1
#define TABLELOG_COL_PLAYERNUMS TABLELOG_COL_TABLEMODE + 1
#define TABLELOG_COL_WINNER  TABLELOG_COL_PLAYERNUMS + 1
#define TABLELOG_COL_REDSCORE TABLELOG_COL_WINNER + 1
#define TABLELOG_COL_BLUESCORE TABLELOG_COL_REDSCORE + 1
#define TABLELOG_COL_REDCUPNUM TABLELOG_COL_BLUESCORE + 1
#define TABLELOG_COL_BLUECUPNUM TABLELOG_COL_REDCUPNUM + 1
#define TABLELOG_COL_CREATETIME TABLELOG_COL_BLUECUPNUM + 1
#define TABLELOG_COL_MVP TABLELOG_COL_CREATETIME + 1
#define TABLELOG_COL_PLAYERID TABLELOG_COL_MVP + 1

#define TABLEDETAIL_COL_START 0
#define TABLEDETAIL_COL_TABLEID TABLEDETAIL_COL_START + 1
#define TABLEDETAIL_COL_PLAYERID TABLEDETAIL_COL_TABLEID + 1
#define TABLEDETAIL_COL_PLAYERSERIAL TABLEDETAIL_COL_PLAYERID + 1
#define TABLEDETAIL_COL_TEAM TABLEDETAIL_COL_PLAYERSERIAL + 1
#define TABLEDETAIL_COL_ROLE TABLEDETAIL_COL_TEAM + 1
#define TABLEDETAIL_COL_RESULT TABLEDETAIL_COL_ROLE + 1



/*enum TABLE_MODE{
	MODE_RANDOM = 0,
	MODE_31 = 1,
	MODE_BANPICK = 2,
	MODE_CM01 = 3,
};*/
class tableDetailData{
public:
	tableDetailData() :playerID(""), playerSerial(0), team(0), role(0), result(0){};
	~tableDetailData(){};
	std::string playerID;
	int playerSerial;
	int team;
	int role;
	int result;
};
class tableLogData{
	public:
		tableLogData() :tableMode(-1), playerNums(-1), winner(30000), redScore(15), blueScore(15), redCupNum(0), blueCupNum(0), createTime(""){};
		~tableLogData(){};
		tableDetailData tableDetail[8];
		int tableMode;
		int playerNums;
		int winner;
		int redScore, blueScore, redCupNum, blueCupNum;
		std::string createTime;
		int mvp;
};


