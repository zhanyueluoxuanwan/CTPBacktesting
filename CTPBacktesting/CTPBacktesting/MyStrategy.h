//回测的策略文件，与交易系统的类型保持一致
//实现一个简单的策略进行测试
//每次行情推送过来，没有持仓就买一手螺纹钢，有持仓就卖一手螺纹钢
#pragma once
#include "stdafx.h"
#include "TraderInfo.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>
using namespace std;
typedef TThostFtdcOrderRefType MyOrderRef;

class MyStrategy {
public:
	//结构性函数，请勿修改
	//构造函数，加上uid
	MyStrategy(int id) :uid(id), pos(0), count(0), order_request(0), front_id(0), session_id(0) {
		//local_order_queue.clear();
	}
	//更新基本交易信息
	void UpdateTradeBasis(int front_id, int session_id) {
		this->front_id = front_id;
		this->session_id = session_id;
	}
	//交易逻辑核心,在MdSpi中RtnMarketData中调用
	void TradeOnMarketData(map<string, vector<FT_DATA>> &market_data, string InstrumentID);
	//报单回报
	void OnRtnOrder(MyOrder *order);
	//交易回报
	void OnRtnTrade(MyTrade *trade);
	//获取策略id
	int GetUid() { return uid; }

	//以下为添加新函数位置：

private:
	int uid;				//策略编号，用于后期多策略注册使用
	int pos;				//合约持仓
	int count;				//计数器，测试用防止测试过快消耗完模拟资金
	int order_request;		//报单请求，目前没啥用，先预留
	int local_reference;	//本地索引记录

	int front_id;			//交易前置id
	int session_id;			//当前回话id
};
