//�ز�Ĳ����ļ����뽻��ϵͳ�����ͱ���һ��
//ʵ��һ���򵥵Ĳ��Խ��в���
//ÿ���������͹�����û�гֲ־���һ�����Ƹ֣��гֲ־���һ�����Ƹ�
#pragma once
#include "stdafx.h"
#include "TraderInfo.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>
using namespace std;
typedef TThostFtdcOrderRefType MyOrderRef;

class MyStrategy {
public:
	//�ṹ�Ժ����������޸�
	//���캯��������uid
	MyStrategy(int id) :uid(id), pos(0), count(0), order_request(0), front_id(0), session_id(0) {
		//local_order_queue.clear();
	}
	//���»���������Ϣ
	void UpdateTradeBasis(int front_id, int session_id) {
		this->front_id = front_id;
		this->session_id = session_id;
	}
	//�����߼�����,��MdSpi��RtnMarketData�е���
	void TradeOnMarketData(map<string, vector<FT_DATA>> &market_data, string InstrumentID);
	//�����ر�
	void OnRtnOrder(MyOrder *order);
	//���׻ر�
	void OnRtnTrade(MyTrade *trade);
	//��ȡ����id
	int GetUid() { return uid; }

	//����Ϊ����º���λ�ã�

private:
	int uid;				//���Ա�ţ����ں��ڶ����ע��ʹ��
	int pos;				//��Լ�ֲ�
	int count;				//�������������÷�ֹ���Թ���������ģ���ʽ�
	int order_request;		//��������Ŀǰûɶ�ã���Ԥ��
	int local_reference;	//����������¼

	int front_id;			//����ǰ��id
	int session_id;			//��ǰ�ػ�id
};
