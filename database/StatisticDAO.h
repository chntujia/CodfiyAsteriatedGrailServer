#pragma once

#include "BaseDAO.h"
#include "StatisticData.h"
#include "zLogger.h"


class StatisticDAO: BaseDAO
{
public:
	StatisticDAO(DBConnection* conn);
	void insert(const tableLogData& data);
	void insert(const tableDetailData& data);
	static boost::uint32_t maxTableLogId;
};
