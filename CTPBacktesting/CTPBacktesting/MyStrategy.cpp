#include "stdafx.h"
#include "MyStrategy.h"
#include <iostream>
#include <iomanip> 
using namespace std;

//根据实时行情进行交易
void MyStrategy::TradeOnMarketData(map<string, vector<FT_DATA>> &market_data, string InstrumentID) {
	cout << "InstrumentID is：" << InstrumentID 
//		<< " Market time: " << market_data[InstrumentID][market_data[InstrumentID].size() - 1].time 
		<< " InstrumentID: " << InstrumentID
		<< " Ask Price: " << market_data[InstrumentID][market_data[InstrumentID].size() - 1].ask1
		<< " Bid Price: " << market_data[InstrumentID][market_data[InstrumentID].size() - 1].bid1
		<< endl;
	//测试报单
	if (pos == 0) {
		cout << "Buy rb1801 contract!" << endl;
		ORDER new_order;
		new_order.id = InstrumentID;
		new_order.price = 3600;
		cout << "Open price is: " << new_order.price << endl;
		new_order.direction = BID;				//买
		new_order.type = TYPE_OPEN;				//开仓
		new_order.order_type = ORDER_COMMIT;	//报单
		new_order.volume = 1;					//数量
		CommitOrder(new_order);
//		pos = 1;
	}
	/*
	else if (pos = 1) {		//测试撤单
		cout << "Cancel current order!" << endl;
		ORDER new_order;
		new_order.id = InstrumentID;
		new_order.order_type = ORDER_CANCEL;
		CancelOrder(new_order);
		pos = 0;
	}
	*/
}

//处理报单回报
//慎用，CTP报单回报功能测试不全
void MyStrategy::OnRtnOrder(MyOrder *order) {
	if (strcmp(order->OrderSysID, "") == 0 || local_order_queue[atoi(order->OrderSysID)])
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
}

//处理成交回报
void MyStrategy::OnRtnTrade(MyTrade *trade) {
	cout << "=========成交返回=========" << endl;
	cout << "InstrumentID is: " << trade->InstrumentID << endl;
	cout << "Price is: " << trade->Price << endl;
	cout << "Volume is: " << trade->Volume << endl;
	cout << "Order direction: " << trade->Direction << endl;
		
	
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