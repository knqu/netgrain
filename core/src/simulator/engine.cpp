#include "order.hpp"
#include "historicalData.hpp"

#include <string>
#include <vector>
#include <unordered_map>

class Engine {
    std::vector<Order> pending_orders;  // orders submitted but not yet filled
    std::vector<Fill> fill_log;  // every fill recorded
    Portfolio portfolio;
    int next_order_id = 0; // unique id assigned to each order is more precise than relying on timestamp

    // default config if left unspecified
    TickerConfig default_config = {0.001, 0.10, 5.0};
    std::unordered_map<std::string, TickerConfig> ticker_configs;

    const TickerConfig& get_config(const std::string& ticker) {
        auto itr = ticker_configs.find(ticker);
        return (itr != ticker_configs.end()) ? itr->second : default_config;
    }

public:
    Engine(int starting_balance) {
        portfolio.balance = starting_balance;
    }

    void set_config(const std::string& ticker, TickerConfig config) {
        ticker_configs[ticker] = config;
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
        const auto& config = get_config(ticker);

        // how many shares can be filled this bar (liquidity cap)
        int volume_remaining = static_cast<int>(bar.volume * config.max_volume);

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
                // determine how many shares can be filled based on remaining liquidity
                int fill_qty = std::min(itr->quantity, volume_remaining);
                if (fill_qty <= 0) {
                    ++itr;
                    continue;
                }
                volume_remaining -= fill_qty;

                // apply price slippage based on volatility config
                double slippage = config.volatility / 10000.0;
                if (itr->side == Side::BUY) {
                    fill_price += static_cast<int>(fill_price * slippage);
                } else {
                    fill_price -= static_cast<int>(fill_price * slippage);
                }

                // make trade and record fill
                fills.push_back({itr->id, ticker, itr->side, fill_price, fill_qty, bar.date});

                // calculate total cost of trade and collect fee
                int trade_value = fill_price * fill_qty;
                int fee = static_cast<int>(trade_value * config.trade_fee);
                portfolio.balance -= fee;

                auto& pos = portfolio.positions[ticker];  // get reference to position (or create if it doesn't exist)
                pos.ticker = ticker;

                if (itr->side == Side::BUY) {
                    portfolio.balance -= trade_value;
                    pos.quantity += fill_qty;
                    int total_cost = (pos.avg_cost * pos.quantity) + trade_value;
                    pos.avg_cost = (pos.quantity > 0) ? total_cost / pos.quantity : 0;  // avoid division by zero
                } else {
                    portfolio.balance += trade_value;
                    pos.quantity -= fill_qty;
                    if (pos.quantity == 0) pos.avg_cost = 0;
                }

                // keep remaining quantity pending if order isn't entirely filled
                if (fill_qty < itr->quantity) {
                    itr->quantity -= fill_qty;
                    ++itr;
                } else {
                    itr = pending_orders.erase(itr);
                }
            } else {
                ++itr;
            }
        }

        fill_log.insert(fill_log.end(), fills.begin(), fills.end());
        return fills;
    }
};
