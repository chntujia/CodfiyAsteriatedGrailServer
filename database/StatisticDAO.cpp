#include "StatisticDAO.h"
#include <string>
#include <mysql.h>
#include <prepared_statement.h>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/format.hpp"
using namespace std;
using namespace sql;
void StatisticDAO::update()
{

}
void StatisticDAO::insert(const tableLogData& data)
{
	PreparedStatement* insertRow;
	PreparedStatement* queryID;
	switch (data.playerNums)
	{
		case 6:
			insertRow = connection->prepare("insert into TableLog values(null,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
			break;
		case 4:
			insertRow = connection->prepare("insert into TableLog values(null,?,?,?,?,?,?,?,?,?,?,?,?,null,null)"); 
			break;
	}
		

	int lastID=0;
	insertRow->setInt(TABLELOG_COL_TABLEMODE, data.tableMode);
	insertRow->setInt(TABLELOG_COL_PLAYERNUMS, data.playerNums);
	insertRow->setInt(TABLELOG_COL_WINNER, data.winner);
	insertRow->setInt(TABLELOG_COL_REDSCORE, data.redScore);
	insertRow->setInt(TABLELOG_COL_BLUESCORE, data.blueScore);
	insertRow->setInt(TABLELOG_COL_REDCUPNUM, data.redCupNum);
	insertRow->setInt(TABLELOG_COL_BLUECUPNUM, data.blueCupNum);
	insertRow->setString(TABLELOG_COL_CREATETIME, data.createTime);
	for (int i = 0; i < data.playerNums; i++)
		insertRow->setString(TABLELOG_COL_PLAYERID + i, data.tableDetail[i].playerID);  //Ö÷±íID¼ÇÂ¼
	ztLoggerWrite(ZONE, e_Debug, "TableLog: %s %s %s %s %s %s %d %d %d %d %d %d %d %s", data.tableDetail[0].playerID, data.tableDetail[1].playerID, data.tableDetail[2].playerID, data.tableDetail[3].playerID, data.tableDetail[4].playerID, data.tableDetail[5].playerID, data.tableMode, data.playerNums
		, data.winner, data.redScore, data.blueScore, data.redCupNum, data.blueCupNum,data.createTime);
	connection->executeUpdate(insertRow);

	queryID = connection->prepare("select max(id) from tableLog");
	ResultSet* res=connection->executeQuery(queryID);
	if (res->next()) { lastID=res->getInt(1); }

	//lastID = insertRow->getGeneratedKeys();
	//insertLog->executeUpdate();
	for (int i = 0; i < data.playerNums; i++)
	{
		insertRow = connection->prepare("insert into TableDetail values(null,?,?,?,?,?,?)");
		insertRow->setInt(TABLEDETAIL_COL_TABLEID, lastID);
		insertRow->setString(TABLEDETAIL_COL_PLAYERID, data.tableDetail[i].playerID);
		insertRow->setInt(TABLEDETAIL_COL_PLAYERSERIAL, data.tableDetail[i].playerSerial);
		insertRow->setInt(TABLEDETAIL_COL_TEAM, data.tableDetail[i].team);
		insertRow->setInt(TABLEDETAIL_COL_ROLE, data.tableDetail[i].role);
		insertRow->setInt(TABLEDETAIL_COL_RESULT, data.tableDetail[i].result);
		ztLoggerWrite(ZONE, e_Debug, "TableDetail: %d %s %d %d %d %d", lastID, data.tableDetail[i].playerID, data.tableDetail[i].playerSerial, data.tableDetail[i].team, data.tableDetail[i].role, data.tableDetail[i].result);
		connection->executeUpdate(insertRow);
		
	}
		

};