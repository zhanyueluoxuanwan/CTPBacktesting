#include "stdafx.h"
#include "DBReader.h"
#include <string>

//生成数据库reader实例，初始化合约列表
DBReader::DBReader() {
	ifstream future_reader(BACKTEST_INSTRUMENT);
	if (future_reader.is_open()) {
		string tvar;
		getline(future_reader, tvar);
		for (int i = 0; i < atoi(tvar.c_str()); i++) {
			string instrument;
			getline(future_reader, instrument);
			instrument_array.push_back(instrument);
		}
	}
	future_reader.close();
}

void DBReader::Register(MyStrategy *strategy) {
	my_strategy = strategy;
}

DBReader::~DBReader() {
	delete my_db, my_strategy;
}