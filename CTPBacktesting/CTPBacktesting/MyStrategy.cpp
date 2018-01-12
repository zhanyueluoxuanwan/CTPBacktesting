#include "stdafx.h"
#include "MyStrategy.h"
#include <iostream>
#include <iomanip> 
using namespace std;

//����ʵʱ������н���
void MyStrategy::TradeOnMarketData(map<string, vector<FT_DATA>> &market_data, string InstrumentID) {
	cout << "InstrumentID is��" << InstrumentID 
		<< " Market time: " << market_data[InstrumentID][market_data[InstrumentID].size() - 1].time 
		<< " InstrumentID: " << InstrumentID
		<< " Ask Price: " << market_data[InstrumentID][market_data[InstrumentID].size() - 1].ask1
		<< " Bid Price: " << market_data[InstrumentID][market_data[InstrumentID].size() - 1].bid1
		<< endl;
	//���Գɽ�
	/*
	if (pos == 1) {
		cout << "Buy rb1801 contract!" << endl;
		ORDER new_order;
		new_order.id = InstrumentID;
		new_order.price = market_data[InstrumentID][market_data[InstrumentID].size() - 1].ask1;
		cout << "Buy price is: " << new_order.price << endl;
		new_order.direction = BID;				//��
		new_order.type = TYPE_OPEN;				//����
		new_order.order_type = ORDER_COMMIT;	//����
		new_order.volume = 1;					//����
		CommitOrder(new_order);
		pos = 0;
	}
	else if (pos == 0) {	
		cout << "Sell rb1801 contract!" << endl;
		ORDER new_order;
		new_order.id = InstrumentID;
		new_order.price = market_data[InstrumentID][market_data[InstrumentID].size() - 1].bid1;
		cout << "Sell price is: " << new_order.price << endl;
		new_order.direction = ASK;				//��
		new_order.type = TYPE_TCLOSE;			//ƽ��
		new_order.order_type = ORDER_COMMIT;	//����
		new_order.volume = 1;
		CommitOrder(new_order);
		pos = 1;
	}
	*/
	/*
	else if (pos = 1) {		//���Գ���
		cout << "Cancel current order!" << endl;
		ORDER new_order;
		new_order.id = InstrumentID;
		new_order.order_type = ORDER_CANCEL;
		CancelOrder(new_order);
		pos = 0;
	}
	*/
}

//�������ر�
//���ã�CTP�����ر����ܲ��Բ�ȫ
void MyStrategy::OnRtnOrder(MyOrder *order) {
	/*
	if (strcmp(order->OrderSysID, "") == 0 || local_order_queue[atoi(order->OrderSysID)])
		cout << "=========�����ɹ�=========" << endl;
	else {
		cout << "=========��������=========" << endl;
		cout << "Order submitted! Order ID: " << order->OrderSysID << endl;
		cout << "FrontID is: " << order->FrontID << endl;
		cout << "SessionID is: " << order->SessionID << endl;
		cout << "OrderRef is: " << order->OrderRef << endl;
		cout << "Order direction: " << order->Direction << endl;
		cout << "Order status: " << order->OrderStatus << endl;
		cout << "Order status message: " << order->StatusMsg << endl;
		cout << "MinVolume: " << order->MinVolume << endl;
			
	}
	*/
}

//����ɽ��ر�
void MyStrategy::OnRtnTrade(MyTrade *trade) {
	/*
	cout << "=========�ɽ�����=========" << endl;
	cout << "InstrumentID is: " << trade->InstrumentID << endl;
	cout << "Price is: " << trade->Price << endl;
	cout << "Volume is: " << trade->Volume << endl;
	cout << "Order direction: " << trade->Direction << endl;
		*/
	
}

//�ύ����
void MyStrategy::CommitOrder(ORDER &new_order) {
	order_reference++;
	sprintf_s(ORDER_REF, "%d", order_reference);
	strcpy_s(new_order.ORDER_REF, ORDER_REF);
	order_queue.push_back(new_order);
	empty_signal.notify_all();
}

//��������
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