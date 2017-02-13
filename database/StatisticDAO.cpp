#include "StatisticDAO.h"
#include <string>
#include <mysql.h>
#include <prepared_statement.h>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/format.hpp"
#include <boost/interprocess/detail/atomic.hpp>
using namespace std;
using namespace sql;

boost::uint32_t StatisticDAO::maxTableLogId = -1;

StatisticDAO::StatisticDAO(DBConnection* conn) : BaseDAO(conn) 
{
	sql::PreparedStatement* query = connection->prepare("select MAX(id) from tableLog");
	sql::ResultSet* res = connection->executeQuery(query);
	maxTableLogId = res->next() ? res->getInt(1) : 0;
}

void StatisticDAO::insert(const tableLogData& data)
{
	PreparedStatement* insertRow;
	boost::interprocess::ipcdetail::atomic_inc32(&maxTableLogId);
	int logId = maxTableLogId;
	switch (data.playerNums)
	{
		case 6:
			insertRow = connection->prepare("insert into TableLog values(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
			break;
		case 4:
			insertRow = connection->prepare("insert into TableLog values(?,?,?,?,?,?,?,?,?,?,?,?,?,null,null)"); 
			break;
	}
	
	insertRow->setInt(TABLELOG_COL_ID, logId);
	insertRow->setInt(TABLELOG_COL_TABLEMODE, data.tableMode);
	insertRow->setInt(TABLELOG_COL_PLAYERNUMS, data.playerNums);
	insertRow->setInt(TABLELOG_COL_WINNER, data.winner);
	insertRow->setInt(TABLELOG_COL_REDSCORE, data.redScore);
	insertRow->setInt(TABLELOG_COL_BLUESCORE, data.blueScore);
	insertRow->setInt(TABLELOG_COL_REDCUPNUM, data.redCupNum);
	insertRow->setInt(TABLELOG_COL_BLUECUPNUM, data.blueCupNum);
	insertRow->setString(TABLELOG_COL_CREATETIME, data.createTime);

	for (int i = 0; i < data.playerNums; i++) {
		insertRow->setString(TABLELOG_COL_PLAYERID + i, data.tableDetail[i].playerID);  //Ö÷±íID¼ÇÂ¼
	}

	connection->executeUpdate(insertRow);

	for (int i = 0; i < data.playerNums; i++)
	{
		insertRow = connection->prepare("insert into TableDetail values(null,?,?,?,?,?,?)");
		insertRow->setInt(TABLEDETAIL_COL_TABLEID, logId);
		insertRow->setString(TABLEDETAIL_COL_PLAYERID, data.tableDetail[i].playerID);
		insertRow->setInt(TABLEDETAIL_COL_PLAYERSERIAL, data.tableDetail[i].playerSerial);
		insertRow->setInt(TABLEDETAIL_COL_TEAM, data.tableDetail[i].team);
		insertRow->setInt(TABLEDETAIL_COL_ROLE, data.tableDetail[i].role);
		insertRow->setInt(TABLEDETAIL_COL_RESULT, data.tableDetail[i].result);
		connection->executeUpdate(insertRow);
		
	}
		

};