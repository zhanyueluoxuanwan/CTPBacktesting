//��������ݿ��ж�ȡ����
#pragma once
#include "stdafx.h"
#include "mysql.h"
#include "MyStrategy.h"
#include "Dealer.h"
#include "FundManager.h"
#include <fstream>
#include <iostream>
#include <algorithm>
using namespace std;

//���鵥Ԫ
typedef pair<string, FT_DATA> MarketUnit;

class DBReader {
public:
	DBReader();																	//����ʵ������ȡ���ĵĺ�Լ
	void Register(MyStrategy *strategy, Dealer *dealer, FundManager *fm);		//ע�����ʵ��
	//������Լ����
	void PlayMarketData();														//�������飬�����첽���ƣ��ȶ�ȡ��ϣ�Ȼ��ز�
	//���Լ����
	void PlayMarketDataArray();
	static bool cmp(const MarketUnit &A, const MarketUnit &B) {
		if (A.first < B.first)
			return true;
		else return false;
	}
	vector<string> RtnInstrumentArray() {										//��¶���ĵĺ�Լ�б�
		return instrument_array;
	}
	virtual ~DBReader();						//������Դ
private:
	MYSQL *my_db;								//���ݿ��ȡָ��
	MyStrategy *my_strategy;					//����ʵ�����˴����Խ�����չ
	Dealer *my_dealer;							//���ʵ��
	FundManager *my_fm;							//�ʽ����ʵ��
	map<string, vector<FT_DATA>> market_data;	//������Ϣ
	vector<string> instrument_array;			//���ĵ�����
	
	//���ݿ�������Ϣ
	string user;
	string pswd;
	string host;
	string database;
	string table;
	
	//���Լ�ز��ñ���
	string localtime;							//��Լ���ı���ʱ��
	vector<MarketUnit> market_data_array;		//���������
	MarketUnit tmp_data;						//�м����
};