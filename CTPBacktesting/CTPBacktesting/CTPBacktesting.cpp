// CTPBacktesting.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "DBReader.h"
#include "Dealer.h"
#include "MyStrategy.h"
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <deque>
#include <string>
using namespace std;

std::condition_variable empty_signal;			//报单队列是否为空
std::mutex mtx;									//全局锁
deque<ORDER> order_queue;						//报单队列

int main()
{
	DBReader *my_reader = new DBReader();
	MyStrategy *my_strategy = new MyStrategy(0);
	vector<string> instrument_array = my_reader->RtnInstrumentArray();
	Dealer *my_dealer = new Dealer(instrument_array);
	my_reader->Register(my_strategy, my_dealer);
	my_dealer->Register(my_strategy);
	my_reader->PlayMarketData();
	delete my_reader, my_strategy, my_dealer;
	system("pause");
    return 0;
}

