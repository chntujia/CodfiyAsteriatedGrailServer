#pragma once

#include "BaseDAO.h"
#include "StatisticData.h"
#include "zLogger.h"


class StatisticDAO: BaseDAO
{
public:
	StatisticDAO(DBConnection* conn) : BaseDAO(conn){}
	void update();
	void insert(const tableLogData& data);
	void insert(const tableDetailData& data);
};
