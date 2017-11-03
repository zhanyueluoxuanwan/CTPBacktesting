//负责从数据库中读取行情
#pragma once
#include "stdafx.h"
#include "mysql.h"
#include "MyStrategy.h"
#include "Dealer.h"
#include "FundManager.h"
#include <fstream>
#include <iostream>
using namespace std;

class DBReader {
public:
	DBReader();																	//生成实例，获取订阅的合约
	void Register(MyStrategy *strategy, Dealer *dealer, FundManager *fm);		//注册策略实例
	void PlayMarketData();														//播放行情，采用异步机制，先读取完毕，然后回播
	vector<string> RtnInstrumentArray() {										//暴露订阅的合约列表
		return instrument_array;
	}
	virtual ~DBReader();										//回收资源
private:
	MYSQL *my_db;								//数据库读取指针
	MyStrategy *my_strategy;					//策略实例，此处可以进行拓展
	Dealer *my_dealer;							//撮合实例
	FundManager *my_fm;							//资金管理实例
	map<string, vector<FT_DATA>> market_data;	//行情信息
	vector<string> instrument_array;			//订阅的行情
	
	//数据库链接信息
	string user;
	string pswd;
	string host;
	string database;
	string table;
};