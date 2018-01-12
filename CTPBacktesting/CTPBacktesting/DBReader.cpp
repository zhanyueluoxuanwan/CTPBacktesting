#include "stdafx.h"
#include "DBReader.h"
#include <string>

//�������ݿ�readerʵ������ʼ����Լ�б�
DBReader::DBReader() {
	//��ȡ��Լ�б�,�ȿ��ǵ���Լ
	ifstream future_reader(BACKTEST_INSTRUMENT);
	if (future_reader.is_open()) {
		string tvar, instrument;
		getline(future_reader, tvar);
		getline(future_reader, instrument);
		instrument_array.push_back(instrument);
		market_data.insert(make_pair(instrument, vector<FT_DATA>()));
	}
	future_reader.close();
	
	//��ʼ�����ݿ���Ϣ
	user = DB_USER;
	pswd = DB_PWD;
	host = DB_ADDR;
	database = DB_NAME;
	table = TABLE_NAME;
	cout << "Table is��" << table << endl;
	cout << "Database is: " << database << endl;
	
	//�������ݿ�
	my_db = new MYSQL();
	mysql_init(my_db);
	if (mysql_real_connect(my_db, host.c_str(), user.c_str(), pswd.c_str(), database.c_str(), 3306, NULL, 0)) {
		cout << "Future database connect succeed!" << endl;
	}
	else
		cout << "Failed to connect future database!" << endl;
}

void DBReader::Register(MyStrategy *strategy, Dealer *dealer, FundManager *fm) {
	my_strategy = strategy;
	my_dealer = dealer;
	my_fm = fm;
}

//�ز��������ݣ���һ���ȿ��ǵ���Լ�������ٿ���ʱ�����
void DBReader::PlayMarketData() {
	//��ѯ�Ƿ��в���ʵ��
	if (my_strategy == nullptr) {
		cout << "No strategy instance!" << endl;
		return;
	}
	//����ɲ�ѯ�����ݴ洢
	string query = "select * from " + table + " where future_id = '" + instrument_array[0] +"' and time>"+std::to_string(START_DATE) +" and time<"+ std::to_string(END_DATE)+";";
	int query_status = mysql_query(my_db, query.c_str());
	if (query_status == 0) {
		cout << "Query init!" << endl;
		MYSQL_RES *query_result = mysql_store_result(my_db);
		if (query_result) {
			MYSQL_ROW sql_row;
			FT_DATA tick_data;
			while (sql_row = mysql_fetch_row(query_result)) {
				//map<string, vector<FT_DATA>> &market_data;
				/*
				cout << "id: " << sql_row[0]  
					<< " time: " << sql_row[1]
					<< " open: " << sql_row[2]
					<< " high: " << sql_row[3]
					<< " low: " << sql_row[4]
					<< " close: " << sql_row[5]
					<< " ask1: " << sql_row[6]
					<< " bid1: " << sql_row[7]
					<< " askvol1: " << sql_row[8]
					<< " bidvol1: " << sql_row[9]
					<< " vol: " << sql_row[10]
					<< " turnover: " << sql_row[11]
					<< " holding: " << sql_row[12]
					<< endl;
					*/
				
				tick_data.id       = sql_row[0];
				tick_data.time     = sql_row[1];
				tick_data.open     = atof(sql_row[2]);
				tick_data.high     = atof(sql_row[3]);
				tick_data.low      = atof(sql_row[4]);
				tick_data.close    = atof(sql_row[5]);
				tick_data.ask1     = atof(sql_row[6]);
				tick_data.bid1     = atof(sql_row[7]);
				tick_data.askvol1  = atof(sql_row[8]);
				tick_data.bidvol1  = atof(sql_row[9]);
				tick_data.vol      = atof(sql_row[10]);
				tick_data.interest = atof(sql_row[11]);
				tick_data.holding  = atof(sql_row[12]);
				market_data[instrument_array[0]].push_back(tick_data);
				
			}
			mysql_free_result(query_result);
		}
		else {
			cout << "No data in table: " << table << "!" << endl;
		}			
	}
	else {
		cout << "Query error! Error code: " << query_status << endl;
	}
	//��������
	map<string, vector<FT_DATA>> cur_market;	//α��ʵʱ����
	cur_market.insert(make_pair(instrument_array[0], vector<FT_DATA>()));
	net_pos.insert(make_pair(instrument_array[0], vector<int>()));
	net_pos[instrument_array[0]].push_back(0);
	for (int i = 0; i < market_data[instrument_array[0]].size(); i++) {
		cur_market[instrument_array[0]].push_back(market_data[instrument_array[0]][i]);
		//��һ�棬���гɽ����ٽ�����
		//����������
		my_strategy->TradeOnMarketData(cur_market, instrument_array[0]);
		//����
		my_dealer->OrderAction();
		//�гɽ�
		my_dealer->Strike(cur_market, instrument_array[0]);
		//�����˻�Ȩ��
		my_fm->InMarketEquity(cur_market, instrument_array[0]);
	}
}

DBReader::~DBReader() {
	if (my_db != nullptr) {
		mysql_close(my_db);
		cout << "Database disconnected!" << endl;
	}
	delete my_db, my_strategy, my_dealer, my_fm;
}