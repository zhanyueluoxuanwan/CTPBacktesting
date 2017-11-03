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
	//分配成交单信息
	my_trade = new MyTrade();
}

Dealer::~Dealer() {
	ORDER stop_order;
	stop_order.id = "EOF";
	order_queue.push_back(stop_order);
	empty_signal.notify_all();
	LittleDealer.join();
	delete my_strategy, my_trade;
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
		if (order_queue[0].order_type == ORDER_COMMIT) 
			ctp_order[order_queue[0].id].insert(make_pair(atoi(order_queue[0].ORDER_REF), order_queue[0]));
		else if (order_queue[0].order_type == ORDER_CANCEL) {
			ctp_order[order_queue[0].id].erase(atoi(order_queue[0].ORDER_REF));
		}
		else {
			cout << "Wrong Order Type! Little Trader Waits!" << endl;
		}
		//弹出单子队列
		order_queue.pop_front();
		lck.unlock();
	}
}

//判成交
//第一版保守判断，按照对价成交
void Dealer::Strike(map<string, vector<FT_DATA>> &market_data, string InstrumentID) {
	size_t cur_idx = market_data[InstrumentID].size() - 1;
	for (map<int, ORDER>::iterator order = ctp_order[InstrumentID].begin(); order != ctp_order[InstrumentID].end();) {
		if (order->second.direction==0 && order->second.price==market_data[InstrumentID][cur_idx].ask1 || order->second.direction == 1 && order->second.price == market_data[InstrumentID][cur_idx].bid1) {
			//当前报单可以成交，做个假单子
			strcpy_s(my_trade->InstrumentID, InstrumentID.c_str());
			my_trade->Price = order->second.price;
			my_trade->Direction = order->second.direction==0?'0':'1';
			my_trade->Volume = order->second.volume;
			my_strategy->OnRtnTrade(my_trade);
			ctp_order[InstrumentID].erase(order++);
		}
		else {
			order++;
		}
	}
}