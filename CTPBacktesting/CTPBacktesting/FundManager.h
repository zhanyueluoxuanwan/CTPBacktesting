//�ʽ�����࣬���˻���Ϣ���ºͼ�¼
//��ÿ�����Թ������ֲ֣������ʽ𣬶�̬Ȩ��
//��һ�棬�����Ȳ�ռ���ʽ�ֻ����ɽ�
#pragma once
#include "stdafx.h"
#include "TraderInfo.h"
#include "BacktestParam.h"

extern map<string, vector<int>> net_pos;

class FundManager {
public:
	FundManager();
	virtual ~FundManager();
	void InTradeEquity(map<string, vector<FT_DATA>> &market_data, MyTrade *trade, int cur_pos);				//�ɽ��У������˻���Ϣ
	void InMarketEquity(map<string, vector<FT_DATA>> &market_data, string InstrumentID);					//�����мۼ���Ȩ��										
private:
	//double interest_diff;		//���ڳɽ����µ�Ȩ��仯
	//int net_pos_change;			//��¼���ֲֵı仯�����ڼ��㶯̬Ȩ��
	double bid_vlt;		//�򵥼۸�䶯	
	double ask_vlt;		//�����۸�䶯
};