#include "stdafx.h"
#include "Dealer.h"

Dealer::Dealer(vector<string> &instrument_array) {
	//注册撮合合约列表
	for (int i = 0; i < instrument_array.size(); i++) {
		ctp_order.insert(make_pair(instrument_array[i], map<int,ORDER>()));
		cout << "Dealer-->Register Instrument: " << instrument_array[i] << endl;
	}

	//分配成交单和报单信息
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

//报单函数，本地队列-->交易所队列
void Dealer::OrderAction() {
	if (order_queue.size() == 0) 
		return;

	while (order_queue.size() > 0) {
		if (!OrderCheck(order_queue[0])) {	//首先过报单检查，模拟本地CTP自检
			cout << "The current order is illegal. Please recheck! Order info: "
				<< " InstrumentID: " << order_queue[0].id
				<< endl;
			order_queue.pop_front();
		}
		else {
			//回测系统在处理报单时回调OnRtnOrder函数
			strcpy_s(my_order->OrderSysID, "999");
			my_order->FrontID = 100;
			my_order->SessionID = 100;
			strcpy_s(my_order->OrderRef, order_queue[0].ORDER_REF);
			my_order->Direction = order_queue[0].direction;
			my_order->OrderStatus = order_queue[0].order_type == ORDER_COMMIT ? '3' : '5';
			my_order->MinVolume = 1;	//默认最小单位一手

			//模拟报送交易所过程
			if (order_queue[0].order_type == ORDER_COMMIT) {
				strcpy_s(my_order->StatusMsg, "报单成功，太6了");
				UpdateCloseOrder(order_queue[0]);
				ctp_order[order_queue[0].id].insert(make_pair(atoi(order_queue[0].ORDER_REF), order_queue[0]));
				cout << "Current queue size: " << ctp_order[order_queue[0].id].size() << endl;
				order_queue.pop_front();				
				my_strategy->OnRtnOrder(my_order);
			}
			else if (order_queue[0].order_type == ORDER_CANCEL) {
				strcpy_s(my_order->StatusMsg, "撤单成功，更6了");
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

//判成交
//第一版保守判断，按照对价成交，只算净持仓
void Dealer::Strike(map<string, vector<FT_DATA>> &market_data, string InstrumentID) {
	size_t cur_idx = market_data[InstrumentID].size() - 1;
	for (map<int, ORDER>::iterator order = ctp_order[InstrumentID].begin(); order != ctp_order[InstrumentID].end();) {
		if (order->second.direction==BID && order->second.price >= market_data[InstrumentID][cur_idx].ask1 || order->second.direction == ASK && order->second.price <= market_data[InstrumentID][cur_idx].bid1) {
			//当前报单可以成交，做个假单子
			strcpy_s(my_trade->InstrumentID, InstrumentID.c_str());
			my_trade->Price = order->second.direction == BID ? min(order->second.price, market_data[InstrumentID][cur_idx].ask1) : max(order->second.price, market_data[InstrumentID][cur_idx].bid1);
			my_trade->Direction = order->second.direction;
			my_trade->Volume = order->second.volume;
			//my_fm->InTradeEquity(market_data, my_trade, cur_pos);
			if (order->second.type == TYPE_OPEN) {	//开仓情况
				if (order->second.direction == BID) {
					strike_flag[order->second.id].direction = BID;
					strike_flag[order->second.id].type = TYPE_OPEN;
					account_cost[InstrumentID].bid_cost = (account_cost[InstrumentID].bid_cost * (dealer_pos[InstrumentID].td_bid_pos + dealer_pos[InstrumentID].yd_bid_pos) + order->second.price * order->second.volume) / (dealer_pos[InstrumentID].td_bid_pos + dealer_pos[InstrumentID].yd_bid_pos + order->second.volume);
					account_cost[InstrumentID].bid_pos += order->second.volume;
					account_cost[InstrumentID].commission_fee += trade_param[InstrumentID].commission * trade_param[InstrumentID].multiplier * order->second.price * order->second.volume;
					dealer_pos[InstrumentID].td_bid_pos += order->second.volume;
					//开仓成本应直接记录在equity上面
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
					//开仓成本应直接记录在equity上面
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
			//平仓成本直接记录在equity上面
			//先计算前一tick持仓到当前tick的价格变化，然后更新平仓价格变化
			else if(order->second.type == TYPE_TCLOSE || order->second.type == TYPE_YCLOSE) {	//平仓情况
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
			//删除本地报单记录
			ctp_order[InstrumentID].erase(order++);
			my_strategy->OnRtnTrade(my_trade);
		}
		else {
			order++;
		}
	}

}

//检查报单是否合格
//由于开发速度要求，TYPE_CLOSE暂时弃用
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

//更新平仓单
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

//模拟进行市场日间更新
//包括但不限于：换仓（今仓->昨仓），清出报单
void Dealer::UpdateMarketState(string InstrumentID)
{
	//换仓
	dealer_pos[InstrumentID].yd_ask_pos += dealer_pos[InstrumentID].td_ask_pos;
	dealer_pos[InstrumentID].yd_bid_pos += dealer_pos[InstrumentID].td_bid_pos;
	dealer_pos[InstrumentID].td_bid_pos = 0;
	dealer_pos[InstrumentID].td_ask_pos = 0;
	//清出报单
	order_queue.clear();
	ctp_order.clear();
}
