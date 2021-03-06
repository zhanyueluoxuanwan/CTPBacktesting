// CTPBacktesting.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "DBReader.h"
#include "Dealer.h"
#include "FundManager.h"
#include "MyStrategy.h"
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <deque>
#include <string>
using namespace std;

//全局交易撮合引擎
Dealer *my_dealer;

//需要用到的全局变量
map<string, COST> account_cost;			//持仓成本
vector<money_unit> equity;				//权益
deque<ORDER> order_queue;				//报单队列
map<string, fee> trade_param;			//交易参数
map<string, vector<int>> net_pos;		//净持仓，保存一个队列，用于计算账户权益
map<string, STRIKE_TYPE> strike_flag;	//全局成交flag，用来记录哪个合约成交

//报单索引
int order_reference = 0;

//全局函数
//简单报单函数
int CommitOrder(string InstrumentID, double price, int volume, char direction, char type) {
	ORDER new_order;
	new_order.id = InstrumentID;
	new_order.price = price;
	new_order.direction = direction;
	new_order.type = type;
	new_order.volume = volume;
	new_order.order_type = ORDER_COMMIT;
	order_reference++;
	sprintf_s(new_order.ORDER_REF, "%d", order_reference);
	order_queue.push_back(new_order);
	return order_reference;
}

//撤单函数
void CancelOrder(string InstrumentID, int order_reference, int front_id, int session_id) {
	ORDER new_order;
	sprintf_s(new_order.ORDER_REF, "%d", order_reference);
	new_order.front_id = front_id;
	new_order.session_id = session_id;
	new_order.id = InstrumentID;
	new_order.order_type = ORDER_CANCEL;
	order_queue.push_back(new_order);
}

int main()
{
	FundManager *my_fm = new FundManager();
	DBReader *my_reader = new DBReader();
	MyStrategy *my_strategy = new MyStrategy(0);
	vector<string> instrument_array = my_reader->RtnInstrumentArray();
	my_dealer = new Dealer(instrument_array);
	my_reader->Register(my_strategy, my_dealer, my_fm);
	my_dealer->Register(my_strategy, my_fm);
	vector<string> InstrumentID = my_reader->RtnInstrumentArray();
	if (InstrumentID.size() == 1)
		my_reader->PlayMarketData();
	else if (InstrumentID.size() > 1)
		my_reader->PlayMarketDataArray();
	delete my_reader, my_strategy, my_dealer, my_fm;
	system("pause");
    return 0;
}

