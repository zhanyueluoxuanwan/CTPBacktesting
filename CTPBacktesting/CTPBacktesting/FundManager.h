//资金管理类，含账户信息更新和记录
//对每个策略管理：净持仓，可用资金，动态权益
//第一版，报单先不占用资金，只计算成交
#pragma once
#include "stdafx.h"
#include "TraderInfo.h"
#include "BacktestParam.h"

extern map<string, vector<int>> net_pos;

typedef struct {
	double available_money;	//资金余额
	double total_interest;	//账户动态权益
}money_unit;

class FundManager {
public:
	FundManager();
	virtual ~FundManager();
	void InTradeEquity(MyTrade *trade, int cur_pos);											//成交中，更新账户信息
	void InMarketEquity(map<string, vector<FT_DATA>> &market_data, string InstrumentID);		//按照市价计算权益
private:
	double available_money;					//可用资金，初始化100W，暂时变量
	vector<money_unit> equity;	//全时间序列的动态权益，用于显示效果
};