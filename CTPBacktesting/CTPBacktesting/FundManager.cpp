#include "stdafx.h"
#include "FundManager.h"

FundManager::FundManager() {
	//初始化资金
	available_money = 1000000;
	
	//初始化净持仓
	ifstream instrumentFile(BACKTEST_INSTRUMENT);
	if (instrumentFile.is_open()) {
		string len;
		getline(instrumentFile, len);
		int instrumentNum = stoi(len);
		for (int line = 0; line < instrumentNum; line++) {
			string instrument;
			getline(instrumentFile, instrument);
			net_pos.insert(make_pair(instrument, 0));
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

}

FundManager::~FundManager() {

}