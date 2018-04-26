#include "stdafx.h"
#include "FundManager.h"

FundManager::FundManager() {
	//��ʼ�����ֲ�
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
	
	//��ʼ�����ײ���
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

	//��ʼ���˻�Ȩ��
	money_unit init_equity;
	init_equity.available_money = 1000000;
	init_equity.total_interest = 1000000;
	equity.push_back(init_equity);
	bid_vlt = 0;
	ask_vlt = 0;
}

FundManager::~FundManager() {

}

//����ɽ��е��˻�Ȩ��
//���շֱʳɽ�ԭ�Ӽ��㣬��ʱδʹ��
void FundManager::InTradeEquity(map<string, vector<FT_DATA>> &market_data, MyTrade *trade, int cur_pos) {
	/*
	if (trade->Direction == '0') {
		if (cur_pos >= 0) //�࿪
			equity[equity.size() - 1].available_money -= abs(trade->Volume)*trade->Price*trade_param[trade->InstrumentID].multiplier*(trade_param[trade->InstrumentID].deposit_percent + trade_param[trade->InstrumentID].commission);
		else 				//��ƽ
			equity[equity.size() - 1].available_money += abs(trade->Volume)*trade->Price*trade_param[trade->InstrumentID].multiplier*(trade_param[trade->InstrumentID].deposit_percent - trade_param[trade->InstrumentID].commission);
	}
	else {
		if (cur_pos <= 0) 	//�տ�
			equity[equity.size() - 1].available_money -= abs(trade->Volume)*trade->Price*trade_param[trade->InstrumentID].multiplier*(trade_param[trade->InstrumentID].deposit_percent + trade_param[trade->InstrumentID].commission);
		else 				//��ƽ
			equity[equity.size() - 1].available_money += abs(trade->Volume)*trade->Price*trade_param[trade->InstrumentID].multiplier*(trade_param[trade->InstrumentID].deposit_percent - trade_param[trade->InstrumentID].commission);
	}
	int direction = trade->Direction == '0' ? 1 : -1;
	double price_diff = direction * (market_data[trade->InstrumentID][market_data[trade->InstrumentID].size() - 1].close - trade->Price)*trade_param[trade->InstrumentID].multiplier*trade->Volume;
	interest_diff = price_diff - abs(trade->Volume)*trade->Price*trade_param[trade->InstrumentID].multiplier*trade_param[trade->InstrumentID].commission;
	net_pos_change += trade->Volume*direction;
	cout << "Interest change is: " << interest_diff << endl;
	*/
}

//�����мۼ������棬���µ�equity��
void FundManager::InMarketEquity(map<string, vector<FT_DATA>> &market_data, string InstrumentID) {
	size_t mlen = market_data[InstrumentID].size() - 1;
	if (mlen < 1)
		return;
	if (strike_flag[InstrumentID].direction != ASK && strike_flag[InstrumentID].direction != BID) {	//������ֲ����
		bid_vlt = (market_data[InstrumentID][mlen].close - market_data[InstrumentID][mlen - 1].close) * account_cost[InstrumentID].bid_pos;
		ask_vlt = (market_data[InstrumentID][mlen - 1].close - market_data[InstrumentID][mlen].close) * account_cost[InstrumentID].ask_pos;
		money_unit prev_money = equity.back();
		prev_money.available_money += bid_vlt + ask_vlt;
		prev_money.total_interest += bid_vlt + ask_vlt;
		equity.push_back(prev_money);
	}
	cout << "========�ɱ�����========" << endl;
	cout << "Current price: " << market_data[InstrumentID].back().close
		<< " bid cost: " << account_cost[InstrumentID].bid_cost
		<< " ask cost: " << account_cost[InstrumentID].ask_cost
		<< " bid vlt: " << bid_vlt
		<< " ask vlt: " << ask_vlt
		<< " commission_fee: " << account_cost[InstrumentID].commission_fee
		<< endl;
	cout << "Current available money: " << equity.back().available_money
		<< " total equity: " << equity.back().total_interest
		<< endl;
	account_cost[InstrumentID].commission_fee = 0;
	strike_flag[InstrumentID].direction = 'X';
	strike_flag[InstrumentID].type = 'X';
	//���������ж�
	int m = 1;
}

