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

	connection->executeUpdate(insertRow);

	queryID = connection->prepare("select LAST_INSERT_ID() from tableLog");
	ResultSet* res=connection->executeQuery(queryID);
	if (res->next()) { lastID=res->getInt(1); }

	for (int i = 0; i < data.playerNums; i++)
	{
		insertRow = connection->prepare("insert into TableDetail values(null,?,?,?,?,?,?)");
		insertRow->setInt(TABLEDETAIL_COL_TABLEID, lastID);
		insertRow->setString(TABLEDETAIL_COL_PLAYERID, data.tableDetail[i].playerID);
		insertRow->setInt(TABLEDETAIL_COL_PLAYERSERIAL, data.tableDetail[i].playerSerial);
		insertRow->setInt(TABLEDETAIL_COL_TEAM, data.tableDetail[i].team);
		insertRow->setInt(TABLEDETAIL_COL_ROLE, data.tableDetail[i].role);
		insertRow->setInt(TABLEDETAIL_COL_RESULT, data.tableDetail[i].result);
		connection->executeUpdate(insertRow);
		
	}
		

};