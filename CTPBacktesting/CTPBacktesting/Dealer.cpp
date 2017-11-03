#include "stdafx.h"
#include "Dealer.h"

Dealer::Dealer(vector<string> &instrument_array) {
	//注册撮合合约列表
	for (int i = 0; i < instrument_array.size(); i++) {
		ctp_order.insert(make_pair(instrument_array[i], map<int,ORDER>()));
		cout << "Dealer-->Register Instrument: " << instrument_array[i] << endl;
	}
	//启动本地报单线程
	LittleDealer = thread(&Dealer::OrderAction, this);
	//分配成交单和报单信息
	my_trade = new MyTrade();
	my_order = new MyOrder();
}

Dealer::~Dealer() {
	ORDER stop_order;
	stop_order.id = "EOF";
	order_queue.push_back(stop_order);
	empty_signal.notify_all();
	LittleDealer.join();
	delete my_strategy, my_trade, my_order;
}

void Dealer::Register(MyStrategy *strategy) {
	my_strategy = strategy;
}

//报单函数，本地队列-->交易所队列
void Dealer::OrderAction() {
	while (true) {
		std::unique_lock<std::mutex> lck(mtx);
		while (order_queue.size() == 0) {
			empty_signal.wait(lck);
		}
		if (order_queue[0].id == "EOF")			//报单的合约代码为EOF表示线程结束
			break;
		cout << "Get new order!" << endl;
		
		//回测系统在处理报单时回调OnRtnOrder函数
		strcpy_s(my_order->OrderSysID, "999");
		my_order->FrontID = 100;
		my_order->SessionID = 100;
		strcpy_s(my_order->OrderRef, order_queue[0].ORDER_REF);
		my_order->Direction = order_queue[0].direction;
		my_order->OrderStatus = '3';
		strcpy_s(my_order->StatusMsg, "太6了");
		my_order->MinVolume = 1;	//默认最小单位一手
		this->my_strategy->UpdateOnRtnOrder(my_order, true);
		this->my_strategy->OnRtnOrder(my_order);

		//模拟报送交易所过程
		if (order_queue[0].order_type == ORDER_COMMIT) 
			ctp_order[order_queue[0].id].insert(make_pair(atoi(order_queue[0].ORDER_REF), order_queue[0]));
		else if (order_queue[0].order_type == ORDER_CANCEL) {
			ctp_order[order_queue[0].id].erase(atoi(order_queue[0].ORDER_REF));
		}
		else {
			cout << "Wrong Order Type! Little Trader Waits!" << endl;
		}

		//删除当前已处理报单
		order_queue.pop_front();
		lck.unlock();
	}
}

//判成交
//第一版保守判断，按照对价成交
void Dealer::Strike(map<string, vector<FT_DATA>> &market_data, string InstrumentID) {
	size_t cur_idx = market_data[InstrumentID].size() - 1;
	for (map<int, ORDER>::iterator order = ctp_order[InstrumentID].begin(); order != ctp_order[InstrumentID].end();) {
		if (order->second.direction=='0' && order->second.price==market_data[InstrumentID][cur_idx].ask1 || order->second.direction == '1' && order->second.price == market_data[InstrumentID][cur_idx].bid1) {
			//当前报单可以成交，做个假单子
			strcpy_s(my_trade->InstrumentID, InstrumentID.c_str());
			my_trade->Price = order->second.price;
			my_trade->Direction = order->second.direction;
			my_trade->Volume = order->second.volume;
			my_strategy->UpdateOnRtnTrade(my_trade);	
			my_strategy->OnRtnTrade(my_trade);
			ctp_order[InstrumentID].erase(order++);
		}
		else {
			order++;
		}
	}
}