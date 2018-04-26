#include "stdafx.h"
#include "MyStrategy.h"
#include <iostream>
#include <iomanip> 
using namespace std;

//根据实时行情进行交易
void MyStrategy::TradeOnMarketData(map<string, vector<FT_DATA>> &market_data, string InstrumentID) {
	cout << "InstrumentID is：" << InstrumentID
		<< " Market time: " << setw(22) << market_data[InstrumentID][market_data[InstrumentID].size() - 1].time
		<< " InstrumentID: " << InstrumentID
		//<< " Ask Price: " << market_data[InstrumentID][market_data[InstrumentID].size() - 1].ask1
		//<< " Bid Price: " << market_data[InstrumentID][market_data[InstrumentID].size() - 1].bid1
		<< " Close Price: " << market_data[InstrumentID][market_data[InstrumentID].size() - 1].close
		<< endl;
	if (market_data[InstrumentID].size() % 10 != 0 || market_data[InstrumentID].size() < 2)
		return;

	//测试成交
	if (pos == 0) {
		cout << "Buy ni1807 contract!" << endl;
		local_reference = CommitOrder(InstrumentID, market_data[InstrumentID].back().ask1, 1, BID, TYPE_OPEN);
		pos = 1;
	}
	else if (pos == 1) {	
		cout << "Sell ni1807 contract!" << endl;
		local_reference = CommitOrder(InstrumentID, market_data[InstrumentID].back().bid1, 1, ASK, TYPE_TCLOSE);
		pos = 0;
	}
	//纯测试用中断
	//int m = 1;
	//测试撤单
	/*
	if (pos == 0) {
		cout << "Buy ni1807 contract!" << endl;
		local_reference = CommitOrder(InstrumentID, market_data[InstrumentID].back().bid1 - 20, 1, BID, TYPE_OPEN);
		pos = 1;
	}
	else if (pos == 1) {
		cout << "Cancel ni1807 contract!" << endl;
		CancelOrder(InstrumentID, local_reference, front_id, session_id);
		pos = 0;
	}
	*/
}

//处理报单回报
void MyStrategy::OnRtnOrder(MyOrder *order) {
	if (strcmp(order->OrderSysID, "") == 0)
		cout << "=========报单成功=========" << endl;
	else {
		cout << "=========报单接收=========" << endl;
		cout << "Order submitted! Order ID: " << order->OrderSysID << endl;
		cout << "FrontID is: " << order->FrontID << endl;
		cout << "SessionID is: " << order->SessionID << endl;
		cout << "OrderRef is: " << order->OrderRef << endl;
		cout << "Order direction: " << order->Direction << endl;
		cout << "Order status: " << order->OrderStatus << endl;
		cout << "Order status message: " << order->StatusMsg << endl;
		cout << "MinVolume: " << order->MinVolume << endl;		
	}
	cout << "Current available money is: " << setprecision(6) << equity.back().available_money
		<< " total money is: " << setprecision(6) << equity.back().total_interest
		<< endl;
}

//处理成交回报
void MyStrategy::OnRtnTrade(MyTrade *trade) {
	cout << "=========成交返回=========" << endl;
	cout << "InstrumentID is: " << trade->InstrumentID << endl;
	cout << "Price is: " << trade->Price << endl;
	cout << "Volume is: " << trade->Volume << endl;
	cout << "Order direction: " << trade->Direction << endl;
	cout << "Current available money is: " << setprecision(6) << equity.back().available_money
		<< " total money is: " << setprecision(6) << equity.back().total_interest
		<< endl;
}
