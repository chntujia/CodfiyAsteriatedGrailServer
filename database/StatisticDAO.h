#pragma once

#include "BaseDAO.h"
#include "StatisticData.h"
#include "zLogger.h"


class StatisticDAO: BaseDAO
{
public:
	StatisticDAO(DBConnection* conn): BaseDAO(conn) {};
	void insert(const tableLogData& data);	
	static void initNextTableLogId(DBConnection* conn);
private:
	static boost::uint32_t nextTableLogId;
};
