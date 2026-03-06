#include "order.hpp"
#include "historicalData.hpp"

#include <string>
#include <vector>
#include <unordered_map>

class Engine {
    std::vector<Order> pending_orders;  // orders submitted but not yet filled
    std::vector<Fill> fill_log;  // every fill recorded
    Portfolio portfolio;
    int next_order_id = 0;

public:
    Engine(int starting_balance) {
        portfolio.balance = starting_balance;
    }

    // currently supports market and limit orders
    int process_order(const std::string& ticker, Side side, int quantity, OrderType type, int limit_price = 0) {
        int id = next_order_id++;
        pending_orders.push_back({id, ticker, side, quantity, type, limit_price, OrderStatus::PENDING});
        return id;
    }

    // simulate one time step for a single ticker
    std::vector<Fill> process_bar(const std::string& ticker, const MarketDataRow& bar) {
        std::vector<Fill> fills;  // list of fills for only current call

        for (auto itr = pending_orders.begin(); itr != pending_orders.end(); ) {
            if (itr->ticker != ticker) {
                ++itr;
                continue;
            }

            int fill_price = 0;
            bool should_fill = false;

            // market orders are always filled using the open price, while limit orders are conditionally filled
            if (itr->order_type == OrderType::MARKET) {
                fill_price = bar.open;
                should_fill = true;
            } else if (itr->order_type == OrderType::LIMIT) {
                if (itr->side == Side::BUY && bar.low <= itr->limit_price) {
                    fill_price = itr->limit_price;
                    should_fill = true;
                } else if (itr->side == Side::SELL && bar.high >= itr->limit_price) {
                    fill_price = itr->limit_price;
                    should_fill = true;
                }
            }

            if (should_fill) {
                fills.push_back({itr->id, ticker, itr->side, fill_price, itr->quantity, bar.date});
            
                int cost = fill_price * itr->quantity;
                auto& pos = portfolio.positions[ticker];  // get reference to position (or create if it doesn't exist)
                pos.ticker = ticker;
            
                if (itr->side == Side::BUY) {
                    portfolio.balance -= cost;
                    int total_cost = pos.avg_cost * pos.quantity + cost;
                    pos.quantity += itr->quantity;
                    pos.avg_cost = (pos.quantity > 0) ? total_cost / pos.quantity : 0;
                } else {
                    portfolio.balance += cost;
                    pos.quantity -= itr->quantity;
                    if (pos.quantity == 0) {
                        pos.avg_cost = 0;
                    }
                }
            
                itr = pending_orders.erase(itr);
            } else {
                ++itr;
            }
        }

        fill_log.insert(fill_log.end(), fills.begin(), fills.end());
        return fills;
    }
};
