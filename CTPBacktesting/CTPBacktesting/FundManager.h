//资金管理类，含账户信息更新和记录
//对每个策略管理：净持仓，可用资金，动态权益
//第一版，报单先不占用资金，只计算成交
#pragma once
#include "stdafx.h"
#include "TraderInfo.h"
#include "BacktestParam.h"

class FundManager {
public:
	FundManager();
	virtual ~FundManager();
private:
	double available_money;		//可用资金，初始化100W
	double floating_money;		//动态权益
	map<string, int> net_pos;	//净持仓
};