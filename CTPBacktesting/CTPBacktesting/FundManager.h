//资金管理类，含账户信息更新和记录
//对每个策略管理：净持仓，可用资金，动态权益
//第一版，报单先不占用资金，只计算成交
#pragma once
#include "stdafx.h"
#include "TraderInfo.h"
#include "BacktestParam.h"

extern map<string, vector<int>> net_pos;

class FundManager {
public:
	FundManager();
	virtual ~FundManager();
	void InTradeEquity(map<string, vector<FT_DATA>> &market_data, MyTrade *trade, int cur_pos);				//成交中，更新账户信息
	void InMarketEquity(map<string, vector<FT_DATA>> &market_data, string InstrumentID);					//按照市价计算权益										
private:
	//double interest_diff;		//由于成交导致的权益变化
	//int net_pos_change;			//记录净持仓的变化，用于计算动态权益
	double bid_vlt;		//买单价格变动	
	double ask_vlt;		//卖单价格变动
};