//������棬�Լ۳ɽ������ع���
//��һ��˳��ϵͳ�����漰�첽����߳�
#pragma once
#include "stdafx.h"
#include "TraderInfo.h"
#include "MyStrategy.h"
#include "FundManager.h"
#include <iostream>
#include <algorithm>
using namespace std;

//extern map<string, vector<int>> net_pos;

//�ֲ���Ϣ
typedef struct {
	int td_bid_pos;		//����
	int td_ask_pos;		//��ղ�
	int yd_bid_pos;		//����
	int yd_ask_pos;		//��ղ�
}POS;

class Dealer {
public:
	Dealer(vector<string> &instrument_array);													
	virtual ~Dealer();
	void Register(MyStrategy *strategy, FundManager *fm);								//ע�����ʵ��
	void OrderAction();																	//α�챨��
	void Strike(map<string, vector<FT_DATA>> &market_data, string InstrumentID);		//�����̿���Ϣ�гɽ�����һ�������ع��ƣ��������
	bool OrderCheck(ORDER &cur_order);													//�����Ƿ�ϸ���
	void UpdateCloseOrder(ORDER &cur_order);											//����ƽ�ֵ�
	void UpdateMarketState(string InstrumentID);										//ģ������г�����
	const Dealer *GetDealer() { return this; }											//����dealerָ�룬���ڲ���
private:	
	MyStrategy *my_strategy;					//���ά������ʵ������OnRtn�ӿ�
	FundManager *my_fm;							//�˻�ʵ��������ÿ�ʽ��׸����˻���Ϣ
	map<string, map<int, ORDER>> ctp_order;		//α��Ľ��������У��ú�Լ�б�ͱ��������˫������
	MyTrade *my_trade;							//α��ĳɽ���
	MyOrder *my_order;							//α��ı���
	map<string, POS> dealer_pos;				//dealerά������ʵ��λ
	money_unit strike_mu;						//���ڼ��㿪��Ȩ���mu
};