#include "stdafx.h"
#include "Dealer.h"

Dealer::Dealer(vector<string> &instrument_array) {
	//ע���Ϻ�Լ�б�
	for (int i = 0; i < instrument_array.size(); i++) {
		ctp_order.insert(make_pair(instrument_array[i], map<int,ORDER>()));
		cout << "Dealer-->Register Instrument: " << instrument_array[i] << endl;
	}

	//����ɽ����ͱ�����Ϣ
	my_trade = new MyTrade();
	my_order = new MyOrder();

}

Dealer::~Dealer() {
	delete my_strategy, my_fm, my_trade, my_order;
}

void Dealer::Register(MyStrategy *strategy, FundManager *fm) {
	my_strategy = strategy;
	my_fm = fm;
}

//�������������ض���-->����������
void Dealer::OrderAction() {
	if (order_queue.size() == 0) 
		return;

	while (order_queue.size() > 0) {
		if (!OrderCheck(order_queue[0])) {	//���ȹ�������飬ģ�Ȿ��CTP�Լ�
			cout << "The current order is illegal. Please recheck! Order info: "
				<< " InstrumentID: " << order_queue[0].id
				<< endl;
			order_queue.pop_front();
		}
		else {
			//�ز�ϵͳ�ڴ�����ʱ�ص�OnRtnOrder����
			strcpy_s(my_order->OrderSysID, "999");
			my_order->FrontID = 100;
			my_order->SessionID = 100;
			strcpy_s(my_order->OrderRef, order_queue[0].ORDER_REF);
			my_order->Direction = order_queue[0].direction;
			my_order->OrderStatus = order_queue[0].order_type == ORDER_COMMIT ? '3' : '5';
			my_order->MinVolume = 1;	//Ĭ����С��λһ��

			//ģ�ⱨ�ͽ���������
			if (order_queue[0].order_type == ORDER_COMMIT) {
				strcpy_s(my_order->StatusMsg, "�����ɹ���̫6��");
				UpdateCloseOrder(order_queue[0]);
				ctp_order[order_queue[0].id].insert(make_pair(atoi(order_queue[0].ORDER_REF), order_queue[0]));
				cout << "Current queue size: " << ctp_order[order_queue[0].id].size() << endl;
				order_queue.pop_front();				
				my_strategy->OnRtnOrder(my_order);
			}
			else if (order_queue[0].order_type == ORDER_CANCEL) {
				strcpy_s(my_order->StatusMsg, "�����ɹ�����6��");
				UpdateCloseOrder(order_queue[0]);
				ctp_order[order_queue[0].id].erase(atoi(order_queue[0].ORDER_REF));
				cout << "Current queue size: " << ctp_order[order_queue[0].id].size() << endl;
				order_queue.pop_front();
				my_strategy->OnRtnOrder(my_order);
			}
			else {
				cout << "Unknown order type!" << endl;
				order_queue.pop_front();
			}
		}
	}
}

//�гɽ�
//��һ�汣���жϣ����նԼ۳ɽ���ֻ�㾻�ֲ�
void Dealer::Strike(map<string, vector<FT_DATA>> &market_data, string InstrumentID) {
	size_t cur_idx = market_data[InstrumentID].size() - 1;
	for (map<int, ORDER>::iterator order = ctp_order[InstrumentID].begin(); order != ctp_order[InstrumentID].end();) {
		if (order->second.direction==BID && order->second.price >= market_data[InstrumentID][cur_idx].ask1 || order->second.direction == ASK && order->second.price <= market_data[InstrumentID][cur_idx].bid1) {
			//��ǰ�������Գɽ��������ٵ���
			strcpy_s(my_trade->InstrumentID, InstrumentID.c_str());
			my_trade->Price = order->second.direction == BID ? min(order->second.price, market_data[InstrumentID][cur_idx].ask1) : max(order->second.price, market_data[InstrumentID][cur_idx].bid1);
			my_trade->Direction = order->second.direction;
			my_trade->Volume = order->second.volume;
			//my_fm->InTradeEquity(market_data, my_trade, cur_pos);
			if (order->second.type == TYPE_OPEN) {	//�������
				if (order->second.direction == BID) {
					strike_flag[order->second.id].direction = BID;
					strike_flag[order->second.id].type = TYPE_OPEN;
					account_cost[InstrumentID].bid_cost = (account_cost[InstrumentID].bid_cost * (dealer_pos[InstrumentID].td_bid_pos + dealer_pos[InstrumentID].yd_bid_pos) + order->second.price * order->second.volume) / (dealer_pos[InstrumentID].td_bid_pos + dealer_pos[InstrumentID].yd_bid_pos + order->second.volume);
					account_cost[InstrumentID].bid_pos += order->second.volume;
					account_cost[InstrumentID].commission_fee += trade_param[InstrumentID].commission * trade_param[InstrumentID].multiplier * order->second.price * order->second.volume;
					dealer_pos[InstrumentID].td_bid_pos += order->second.volume;
					//���ֳɱ�Ӧֱ�Ӽ�¼��equity����
					size_t mlen = market_data[InstrumentID].size() - 1;
					double strike_vlt = (market_data[InstrumentID].back().close - order->second.price) * order->second.volume;
					double bid_vlt = (market_data[InstrumentID][mlen].close - market_data[InstrumentID][mlen - 1].close) * (account_cost[InstrumentID].bid_pos - order->second.volume);
					strike_mu.total_interest = equity.back().total_interest + strike_vlt + bid_vlt - account_cost[InstrumentID].commission_fee;
					strike_mu.available_money = strike_mu.total_interest - (account_cost[InstrumentID].bid_cost*account_cost[InstrumentID].bid_pos + account_cost[InstrumentID].ask_cost*account_cost[InstrumentID].ask_pos);
					equity.push_back(strike_mu);
					//equity.back().available_money += (market_data[InstrumentID].back().close - order->second.price) * order->second.volume;
					//equity.back().total_interest += (market_data[InstrumentID].back().close - order->second.price) * order->second.volume;
				}
				else {
					strike_flag[order->second.id].direction = ASK;
					strike_flag[order->second.id].type = TYPE_OPEN;
					account_cost[InstrumentID].ask_cost = (account_cost[InstrumentID].ask_cost * (dealer_pos[InstrumentID].td_ask_pos + dealer_pos[InstrumentID].yd_ask_pos) + order->second.price * order->second.volume) / (dealer_pos[InstrumentID].td_ask_pos + dealer_pos[InstrumentID].yd_ask_pos + order->second.volume);
					account_cost[InstrumentID].ask_pos += order->second.volume;
					account_cost[InstrumentID].commission_fee += trade_param[InstrumentID].commission * trade_param[InstrumentID].multiplier * order->second.price * order->second.volume;
					dealer_pos[InstrumentID].td_ask_pos+= order->second.volume;
					//���ֳɱ�Ӧֱ�Ӽ�¼��equity����
					size_t mlen = market_data[InstrumentID].size() - 1;
					double strike_vlt = (order->second.price - market_data[InstrumentID].back().close) * order->second.volume;
					double ask_vlt = (market_data[InstrumentID][mlen - 1].close - market_data[InstrumentID][mlen].close) * (account_cost[InstrumentID].ask_pos - order->second.volume);
					strike_mu.total_interest = equity.back().total_interest + strike_vlt + ask_vlt - account_cost[InstrumentID].commission_fee;
					strike_mu.available_money = strike_mu.total_interest - (account_cost[InstrumentID].bid_cost*account_cost[InstrumentID].bid_pos + account_cost[InstrumentID].ask_cost*account_cost[InstrumentID].ask_pos);
					equity.push_back(strike_mu);
					//equity.back().available_money += (order->second.price - market_data[InstrumentID].back().close) * order->second.volume;
					//equity.back().total_interest += (order->second.price - market_data[InstrumentID].back().close) * order->second.volume;
				}
			}
			//ƽ�ֳɱ�ֱ�Ӽ�¼��equity����
			//�ȼ���ǰһtick�ֲֵ���ǰtick�ļ۸�仯��Ȼ�����ƽ�ּ۸�仯
			else if(order->second.type == TYPE_TCLOSE || order->second.type == TYPE_YCLOSE) {	//ƽ�����
				if (order->second.direction == BID) {
					strike_flag[order->second.id].direction = BID;
					strike_flag[order->second.id].type = order->second.type;
					account_cost[InstrumentID].commission_fee += trade_param[InstrumentID].commission * trade_param[InstrumentID].multiplier * order->second.price * order->second.volume;
					size_t mlen = market_data[InstrumentID].size() - 1;
					double ask_vlt = (market_data[InstrumentID][mlen - 1].close - market_data[InstrumentID][mlen].close) * account_cost[InstrumentID].ask_pos;
					double strike_vlt = (market_data[InstrumentID].back().close - order->second.price) * order->second.volume;
					double available_money = equity.back().available_money + ask_vlt + strike_vlt - account_cost[InstrumentID].commission_fee;
					double total_interest = equity.back().total_interest + ask_vlt + strike_vlt - account_cost[InstrumentID].commission_fee;
					strike_mu.total_interest = total_interest;
					if(account_cost[InstrumentID].ask_pos != order->second.volume)
						account_cost[InstrumentID].ask_cost = (account_cost[InstrumentID].ask_cost * account_cost[InstrumentID].ask_pos - order->second.price * order->second.volume) / (account_cost[InstrumentID].ask_pos - order->second.volume);
					else
						account_cost[InstrumentID].ask_cost = 0;
					account_cost[InstrumentID].ask_pos -= order->second.volume;
					strike_mu.available_money = strike_mu.total_interest - (account_cost[InstrumentID].bid_cost*account_cost[InstrumentID].bid_pos + account_cost[InstrumentID].ask_cost*account_cost[InstrumentID].ask_pos);
					equity.push_back(strike_mu);
				}
				else {
					strike_flag[order->second.id].direction = ASK;
					strike_flag[order->second.id].type = order->second.type;
					account_cost[InstrumentID].commission_fee += trade_param[InstrumentID].commission * trade_param[InstrumentID].multiplier * order->second.price * order->second.volume;
					size_t mlen = market_data[InstrumentID].size() - 1;
					double bid_vlt = (market_data[InstrumentID][mlen].close - market_data[InstrumentID][mlen - 1].close) * account_cost[InstrumentID].bid_pos;
					double strike_vlt = (order->second.price - market_data[InstrumentID].back().close) * order->second.volume;
					double available_money = equity.back().available_money + bid_vlt + strike_vlt - account_cost[InstrumentID].commission_fee;
					double total_interest = equity.back().total_interest + bid_vlt + strike_vlt - account_cost[InstrumentID].commission_fee;
					strike_mu.total_interest = total_interest;
					if (account_cost[InstrumentID].bid_pos != order->second.volume)
						account_cost[InstrumentID].bid_cost = (account_cost[InstrumentID].bid_cost * account_cost[InstrumentID].bid_pos - order->second.price * order->second.volume) / (account_cost[InstrumentID].bid_pos - order->second.volume);
					else
						account_cost[InstrumentID].bid_cost = 0;
					account_cost[InstrumentID].bid_pos -= order->second.volume;
					strike_mu.available_money = strike_mu.total_interest - (account_cost[InstrumentID].bid_cost*account_cost[InstrumentID].bid_pos + account_cost[InstrumentID].ask_cost*account_cost[InstrumentID].ask_pos);
					equity.push_back(strike_mu);
				}
			}
			//ɾ�����ر�����¼
			ctp_order[InstrumentID].erase(order++);
			my_strategy->OnRtnTrade(my_trade);
		}
		else {
			order++;
		}
	}

}

//��鱨���Ƿ�ϸ�
//���ڿ����ٶ�Ҫ��TYPE_CLOSE��ʱ����
bool Dealer::OrderCheck(ORDER & cur_order)
{
	if (cur_order.order_type == ORDER_CANCEL && ctp_order.count(cur_order.id) && ctp_order[cur_order.id].count(atoi(cur_order.ORDER_REF)))
		return true;
	else if (cur_order.order_type == ORDER_COMMIT && cur_order.type == TYPE_OPEN) 
		return equity.back().available_money - cur_order.price * trade_param[cur_order.id].multiplier * trade_param[cur_order.id].deposit_percent * cur_order.volume >= 0;
	else if (cur_order.type == TYPE_TCLOSE && (dealer_pos[cur_order.id].td_bid_pos >= cur_order.volume && cur_order.direction == ASK || dealer_pos[cur_order.id].td_ask_pos >= cur_order.volume && cur_order.direction == BID))
		return true;
	else if (cur_order.type == TYPE_YCLOSE && (dealer_pos[cur_order.id].yd_bid_pos >= cur_order.volume && cur_order.direction == ASK || dealer_pos[cur_order.id].yd_ask_pos >= cur_order.volume && cur_order.direction == BID))
		return true;
//	else if (cur_order.type == TYPE_CLOSE && (dealer_pos[cur_order.id].td_bid_pos + dealer_pos[cur_order.id].yd_bid_pos >= cur_order.volume && cur_order.direction == ASK || dealer_pos[cur_order.id].td_ask_pos + dealer_pos[cur_order.id].yd_ask_pos >= cur_order.volume && cur_order.direction == BID))
//	return true;
	else
		return false;
}

//����ƽ�ֵ�
void Dealer::UpdateCloseOrder(ORDER & cur_order)
{
	int order_mov = cur_order.order_type == ORDER_COMMIT ? 1 : -1;
	if (cur_order.type == TYPE_TCLOSE)
		if (cur_order.direction == ASK)
			dealer_pos[cur_order.id].td_bid_pos -= order_mov * cur_order.volume;
		else
			dealer_pos[cur_order.id].td_ask_pos -= order_mov * cur_order.volume;
	else if (cur_order.type == TYPE_YCLOSE)
		if (cur_order.direction == ASK)
			dealer_pos[cur_order.id].yd_bid_pos -= order_mov * cur_order.volume;
		else
			dealer_pos[cur_order.id].yd_ask_pos -= order_mov * cur_order.volume;
}

//ģ������г��ռ����
//�����������ڣ����֣����->��֣����������
void Dealer::UpdateMarketState(string InstrumentID)
{
	//����
	dealer_pos[InstrumentID].yd_ask_pos += dealer_pos[InstrumentID].td_ask_pos;
	dealer_pos[InstrumentID].yd_bid_pos += dealer_pos[InstrumentID].td_bid_pos;
	dealer_pos[InstrumentID].td_bid_pos = 0;
	dealer_pos[InstrumentID].td_ask_pos = 0;
	//�������
	order_queue.clear();
	ctp_order.clear();
}
