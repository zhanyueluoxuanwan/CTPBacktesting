#include "stdafx.h"
#include "MyStrategy.h"
#include <iostream>
#include <iomanip> 
using namespace std;

//根据实时行情进行交易
void MyStrategy::TradeOnMarketData(map<string, vector<FT_DATA>> &market_data, string InstrumentID) {
	cout << "InstrumentID is：" << InstrumentID << " Market time: " << market_data[InstrumentID][market_data[InstrumentID].size() - 1].time << endl;

}

//处理报单回报
//慎用，CTP报单回报功能测试不全
void MyStrategy::OnRtnOrder(MyOrder *order) {
	if (strcmp(order->OrderSysID, "") == 0 || local_order_queue[atoi(order->OrderSysID)])
		cout << "=========报单成功=========" << endl;
	else {
		cout << "=========报单接收=========" << endl;
		cout << "Order submitted! Order ID: " << order->OrderSysID << endl;
		cout << "FrontID is: " << order->FrontID
			<< " SessionID is: " << order->SessionID
			<< " OrderRef is: " << order->OrderRef
			<< " Order direction: " << order->Direction
			<< " Order status: " << order->OrderStatus
			<< " Order status message: " << order->StatusMsg
			<< " MinVolume " << order->MinVolume
			<< endl;
	}
}

//处理成交回报
void MyStrategy::OnRtnTrade(MyTrade *trade) {
	cout << "=========成交返回=========" << endl;
	cout << "InstrumentID is: " << trade->InstrumentID
		<< " Price is: " << trade->Price
		<< " Volume is: " << trade->Volume
		<< " Order direction: " << trade->Direction
		<< endl;
	
}

//提交报单
void MyStrategy::CommitOrder(ORDER &new_order) {
	order_reference++;
	sprintf_s(ORDER_REF, "%d", order_reference);
	strcpy_s(new_order.ORDER_REF, ORDER_REF);
	order_queue.push_back(new_order);
	empty_signal.notify_all();
}

//撤单操作
void MyStrategy::CancelOrder(ORDER &new_order) {
	if (local_order_queue.count(order_reference) == 0)
		cout << "Failed Cancel! No such order!" << endl;
	else if(!local_order_queue[order_reference])
		cout << "Failed Cancel! Order has been traded!" << endl;
	else {
		strcpy_s(new_order.ORDER_REF, ORDER_REF);
		new_order.front_id = front_id;
		new_order.session_id = session_id;
		order_queue.push_back(new_order);
		empty_signal.notify_all();
	}
}