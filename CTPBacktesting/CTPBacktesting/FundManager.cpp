#include "stdafx.h"
#include "FundManager.h"

FundManager::FundManager() {
	//初始化净持仓
	ifstream instrumentFile(BACKTEST_INSTRUMENT);
	if (instrumentFile.is_open()) {
		string len;
		getline(instrumentFile, len);
		int instrumentNum = stoi(len);
		for (int line = 0; line < instrumentNum; line++) {
			string instrument;
			getline(instrumentFile, instrument);
			net_pos.insert(make_pair(instrument, vector<int>()));
			cout << "Current instrument is: " << instrument << endl;
		}
	}
	else
		cout << "Cannot read instruments!" << endl;
	instrumentFile.close();
	
	//初始化交易参数
	ifstream feeFile(TRADE_PARAM_FILE);
	string param;
	while (getline(feeFile, param)) {
		string tmp = "";
		int field_index = 0;
		string InstrumentID;
		fee new_fee;
		for (int i = 0; i < param.size(); i++) {
			if (param[i] == ',') {
				switch (field_index) {
				case 0:
					cout << "InstrumentID is: " << tmp << endl;
					InstrumentID = tmp;
					break;
				case 1:
					cout << "Multiplier is: " << atof(tmp.c_str()) << endl;
					new_fee.multiplier = atof(tmp.c_str());
					break;
				case 2:
					cout << "Commission is: " << atof(tmp.c_str()) << endl;
					new_fee.commission = atof(tmp.c_str());
					break;
				}
				field_index++;
				tmp = "";
			}
			else
				tmp += param[i];
		}
		cout << "Deposit is: " << atof(tmp.c_str()) << endl;
		new_fee.deposit_percent = atof(tmp.c_str());
		trade_param.insert(make_pair(InstrumentID, new_fee));
	}
	feeFile.close();

	//初始化账户权益
	money_unit init_equity;
	init_equity.available_money = 1000000;
	init_equity.total_interest = 1000000;
	interest_diff = 0;
	net_pos_change = 0;
	equity.push_back(init_equity);
}

FundManager::~FundManager() {

}

//计算成交中的账户权益
//按照分笔成交原子计算，可用资金计算可能暂时有点问题
void FundManager::InTradeEquity(map<string, vector<FT_DATA>> &market_data, MyTrade *trade, int cur_pos) {
	if (trade->Direction == '0') {
		if (cur_pos >= 0) //多开
			equity[equity.size() - 1].available_money -= abs(trade->Volume)*trade->Price*trade_param[trade->InstrumentID].multiplier*(trade_param[trade->InstrumentID].deposit_percent + trade_param[trade->InstrumentID].commission);
		else 				//空平
			equity[equity.size() - 1].available_money += abs(trade->Volume)*trade->Price*trade_param[trade->InstrumentID].multiplier*(trade_param[trade->InstrumentID].deposit_percent - trade_param[trade->InstrumentID].commission);
	}
	else {
		if (cur_pos <= 0) 	//空开
			equity[equity.size() - 1].available_money -= abs(trade->Volume)*trade->Price*trade_param[trade->InstrumentID].multiplier*(trade_param[trade->InstrumentID].deposit_percent + trade_param[trade->InstrumentID].commission);
		else 				//多平
			equity[equity.size() - 1].available_money += abs(trade->Volume)*trade->Price*trade_param[trade->InstrumentID].multiplier*(trade_param[trade->InstrumentID].deposit_percent - trade_param[trade->InstrumentID].commission);
	}
	int direction = trade->Direction == '0' ? 1 : -1;
	double price_diff = direction * (market_data[trade->InstrumentID][market_data[trade->InstrumentID].size() - 1].close - trade->Price)*trade_param[trade->InstrumentID].multiplier*trade->Volume;
	interest_diff = price_diff - abs(trade->Volume)*trade->Price*trade_param[trade->InstrumentID].multiplier*trade_param[trade->InstrumentID].commission;
	net_pos_change += trade->Volume*direction;
	cout << "Interest change is: " << interest_diff << endl;
}

//按照市价计算损益
void FundManager::InMarketEquity(map<string, vector<FT_DATA>> &market_data, string InstrumentID) {
	int cur_pos = net_pos[InstrumentID][net_pos[InstrumentID].size() - 1];
	money_unit cur_equity = equity[equity.size() - 1];
	cur_equity.total_interest += (market_data[InstrumentID].size() <= 1 ? 0 : market_data[InstrumentID][market_data[InstrumentID].size() - 1].close - market_data[InstrumentID][market_data[InstrumentID].size() - 2].close)*cur_pos*trade_param[InstrumentID].multiplier;
	if (interest_diff != 0)
		cur_equity.total_interest += interest_diff;
	if (cur_pos + net_pos_change == 0)
		cur_equity.available_money = cur_equity.total_interest;
	//更新净持仓
	net_pos[InstrumentID].push_back(cur_pos + net_pos_change);
	equity.push_back(cur_equity);
	interest_diff = 0;
	net_pos_change = 0;
	cout << "Current total_interest is: " << cur_equity.total_interest << " Current net pos is: " << net_pos[InstrumentID][net_pos[InstrumentID].size() - 1] << endl;
}

