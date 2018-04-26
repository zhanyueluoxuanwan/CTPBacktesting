//撮合引擎，对价成交，保守估计
//第一版顺序系统，不涉及异步撮合线程
#pragma once
#include "stdafx.h"
#include "TraderInfo.h"
#include "MyStrategy.h"
#include "FundManager.h"
#include <iostream>
#include <algorithm>
using namespace std;

//extern map<string, vector<int>> net_pos;

//持仓信息
typedef struct {
	int td_bid_pos;		//今多仓
	int td_ask_pos;		//今空仓
	int yd_bid_pos;		//今多仓
	int yd_ask_pos;		//今空仓
}POS;

class Dealer {
public:
	Dealer(vector<string> &instrument_array);													
	virtual ~Dealer();
	void Register(MyStrategy *strategy, FundManager *fm);								//注册策略实例
	void OrderAction();																	//伪造报单
	void Strike(map<string, vector<FT_DATA>> &market_data, string InstrumentID);		//根据盘口信息判成交，第一版做保守估计，不算队列
	bool OrderCheck(ORDER &cur_order);													//报单是否合格检查
	void UpdateCloseOrder(ORDER &cur_order);											//更新平仓单
	void UpdateMarketState(string InstrumentID);										//模拟进行市场更新
	const Dealer *GetDealer() { return this; }											//返回dealer指针，用于操作
private:	
	MyStrategy *my_strategy;					//撮合维护策略实例，调OnRtn接口
	FundManager *my_fm;							//账户实例，用于每笔交易更新账户信息
	map<string, map<int, ORDER>> ctp_order;		//伪造的交易所队列，用合约列表和报单编号做双重索引
	MyTrade *my_trade;							//伪造的成交单
	MyOrder *my_order;							//伪造的报单
	map<string, POS> dealer_pos;				//dealer维护的真实仓位
	money_unit strike_mu;						//用于计算开仓权益的mu
};