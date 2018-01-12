//回测参数
#pragma once

//订阅的交易合约
#define BACKTEST_INSTRUMENT "E://BacktestInstrumentID.txt"

//合约交易参数
#define TRADE_PARAM_FILE "E://TradeParam.txt"

//回测参数，第一版仅包含起始日期和结束日期+
#define START_DATE 20180111
#define END_DATE 20180113

//数据库参数
#define DB_ADDR "localhost"
#define DB_NAME "future_data"
#define DB_USER "root"
#define DB_PWD  "123456"
#define TABLE_NAME "rb"