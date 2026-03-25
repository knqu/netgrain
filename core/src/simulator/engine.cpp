#include <string>
#include <vector>
#include <unordered_map>

#include "order.hpp"
#include "historicalData.hpp"

class Engine {
    int balance;
    std::unordered_map<std::string, Position> positions;

    int next_order_id = 0;
    std::vector<Order> pending_orders;
    std::vector<Fill> fill_log;

    TickerConfig default_config = {0.001, 5.0};
    std::unordered_map<std::string, TickerConfig> ticker_configs;

public:
    Engine(int starting_balance) {
        balance = starting_balance;
    }

    int get_balance() {
        return balance;
    }

    TickerConfig& get_config(const std::string& ticker) {
        auto config = ticker_configs.find(ticker);
        return (config != ticker_configs.end()) ? config->second : default_config;
    }

    void set_config(const std::string& ticker, TickerConfig config) {
        ticker_configs[ticker] = config;
    }

    int place_order(const std::string& ticker, int quantity, int target_price, Side side, OrderType type) {
        int id = next_order_id++;
        pending_orders.push_back({id, ticker, quantity, target_price, side, type, OrderStatus::PENDING});
        return id;
    }

    bool cancel_order(int order_id) {
        for (auto order = pending_orders.begin(); order != pending_orders.end(); ++order) {
            if (order->id == order_id) {
                order->status = OrderStatus::CANCELLED;
                return true;
            }
        }
        return false;
    }

    std::vector<Fill> process_bar(std::unordered_map<std::string, MarketDataRow> bars) {
        std::vector<Fill> fills;

        for (auto order = pending_orders.begin(); order != pending_orders.end(); ) {
            auto bar_itr = bars.find(order->ticker);
            if (bar_itr == bars.end()) {
                ++order;
                continue;
            }

            auto& bar = bar_itr->second;
            auto& config = get_config(order->ticker);

            bool should_fill = false;
            int fill_price = 0;

            if (order->order_type == OrderType::MARKET) {
                should_fill = true;
                fill_price = bar.open;
            } else if (order->order_type == OrderType::LIMIT) {
                if (order->side == Side::BUY && bar.low <= order->target_price) {
                    should_fill = true;
                    fill_price = order->target_price;
                } else if (order->side == Side::SELL && bar.high >= order->target_price) {
                    should_fill = true;
                    fill_price = order->target_price;
                }
            } else if (order->order_type == OrderType::STOP) {
                if (order->side == Side::BUY && bar.high >= order->target_price) {
                    should_fill = true;
                    fill_price = order->target_price;
                } else if (order->side == Side::SELL && bar.low <= order->target_price) {
                    should_fill = true;
                    fill_price = order->target_price;
                }
            }

            if (should_fill) {
                int fill_quantity = std::min(order->quantity, static_cast<int>(bar.volume));
                bar.volume -= fill_quantity;

                fills.push_back({order->id, order->ticker, fill_quantity, fill_price, order->side, bar.date});

                int trade_value = fill_price * fill_quantity;
                int fee = static_cast<int>(trade_value * config.trade_fee);
                balance -= fee;

                auto& pos = positions[order->ticker];  // get reference to position (or create if it doesn't exist)
                pos.ticker = order->ticker;  // set ticker in case new position was created

                if (order->side == Side::BUY) {
                    balance -= trade_value;
                    pos.quantity += fill_quantity;
                    pos.cost_basis += trade_value;
                    pos.lots.push_back({fill_price, fill_quantity, bar.date});
                } else {
                    balance += trade_value;
                    pos.quantity -= fill_quantity;

                    int remaining = fill_quantity;
                    auto lot = pos.lots.begin();  // default to fifo disposal (todo: add other methods in the future)

                    while (remaining > 0 && lot != pos.lots.end()) {
                        int taken = std::min(remaining, lot->quantity);
                        pos.cost_basis -= lot->fill_price * taken;
                        lot->quantity -= taken;
                        remaining -= taken;

                        if (lot->quantity == 0) {
                            lot = pos.lots.erase(lot);
                        } else {
                            ++lot;
                        }
                    }
                }

                // keep remaining quantity pending if order isn't entirely filled
                if (fill_quantity < order->quantity) {
                    order->quantity -= fill_quantity;
                    ++order;
                } else {
                    order = pending_orders.erase(order);
                }
            } else {
                ++order;
            }
        }

        fill_log.insert(fill_log.end(), fills.begin(), fills.end());
        return fills;
    }
};
