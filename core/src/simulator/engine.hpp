#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "order.hpp"
#include "historicalData.hpp"
#include "persistence_queue.hpp"

class Engine {
    i64 init_balance;
    i64 balance;
    std::unordered_map<std::string, Position> positions;

    int next_order_id = 0;
    std::vector<Order> pending_orders;
    std::vector<Order> cancelled_orders;
    std::vector<Fill> fill_log;

    FeesAndTaxes default_fees_taxes = {1.0, 1.0};
    TickerConfig default_config = {0.001, 5, 0.15, default_fees_taxes};
    std::unordered_map<std::string, TickerConfig> ticker_configs;

    PersistenceQueue *pq;

    i64 apply_slippage(i64 price, i64 fill_qty, u64 bar_volume, double volatility, Side side) {
        double half_spread = price * volatility;
        double volume_ratio = (bar_volume > 0) ? static_cast<double>(fill_qty) / static_cast<double>(bar_volume) : 1.0;
        double slippage = half_spread * (1.0 + volume_ratio);
        return (side == Side::BUY) ? price + static_cast<i64>(slippage) : price - static_cast<i64>(slippage);
    }

public:
    Engine(i64 starting_balance, PersistenceQueue *pq = nullptr) {
        init_balance = starting_balance;
        balance = starting_balance;
        this->pq = pq;
    }

    i64 get_balance() {
        return balance;
    }

    const std::unordered_map<std::string, Position>& get_positions() const {
        return positions;
    }

    const std::vector<Order>& get_pending_orders() const {
        return pending_orders;
    }

    const std::vector<Order>& get_cancelled_orders() const {
        return cancelled_orders;
    }

    const std::vector<Fill>& get_fill_log() const {
        return fill_log;
    }
    
    i64 get_final_balance() {  // returns post-short-term-tax
      if (balance > init_balance) {
        return static_cast<i64>(balance * (1 - default_config.short_term_tax));
      }
      return balance;
    }

    TickerConfig& get_config(const std::string& ticker) {
        auto config = ticker_configs.find(ticker);
        return (config != ticker_configs.end()) ? config->second : default_config;
    }

    void set_config(const std::string& ticker, TickerConfig config) {
        ticker_configs[ticker] = config;
    }

    int place_order(const std::string& ticker, i64 quantity, i64 target_price, Side side, OrderType type) {
        int id = next_order_id++;
        Order order = {id, ticker, quantity, target_price, side, type, OrderStatus::PENDING};
        pending_orders.push_back(order);
        if (pq) pq->push(OrderPlacedEvent{order});
        return id;
    }

    bool cancel_order(int order_id) {
        for (auto order = pending_orders.begin(); order != pending_orders.end(); ++order) {
            if (order->id == order_id) {
                order->status = OrderStatus::CANCELLED;
                cancelled_orders.push_back(*order);
                pending_orders.erase(order);
                if (pq) pq->push(OrderCancelledEvent{order_id});
                return true;
            }
        }
        return false;
    }

    i64 calculate_fee(TickerConfig* conf, i64 fill_quantity, i64 trade_value) {  // HX
      double total_fee = 0.0;
      total_fee += conf->fees_and_taxes.flat_fee_comm_per_share * fill_quantity;
      total_fee += conf->fees_and_taxes.percentage_comm_per_share * trade_value;
      return static_cast<i64>(total_fee);
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
            i64 fill_price = 0;

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
                i64 fill_quantity = std::min(order->quantity, static_cast<i64>(bar.volume));
                bar.volume -= fill_quantity;

                fill_price = apply_slippage(fill_price, fill_quantity, bar.volume, config.volatility, order->side);

                fills.push_back({order->id, order->ticker, fill_quantity, fill_price, order->side, bar.date});

                i64 trade_value = fill_price * fill_quantity;
                i64 fee = static_cast<i64>(trade_value * config.trade_fee);

                auto& pos = positions[order->ticker];  // get reference to position (or create if it doesn't exist)
                pos.ticker = order->ticker;  // set ticker in case new position was created

                if (order->side == Side::BUY) {
                    i64 total_cost = trade_value + fee;
                    if (balance < total_cost) {
                        ++order;
                        continue;
                    }
                    balance -= total_cost;

                    pos.quantity += fill_quantity;
                    pos.cost_basis += trade_value;
                    pos.lots.push_back({fill_price, fill_quantity, bar.date});
                } else {
                    if (balance < fee) {
                        ++order;
                        continue;
                    }
                    balance += trade_value - fee;

                    pos.quantity -= fill_quantity;
                    i64 remaining = fill_quantity;
                    
                    auto lot = pos.lots.begin();  // default to fifo disposal (todo: add other methods in the future)
                    while (remaining > 0 && lot != pos.lots.end()) {
                        i64 taken = std::min(remaining, lot->quantity);
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
                    order->status = OrderStatus::PARTIAL;
                    ++order;
                } else {
                    order->status = OrderStatus::FILLED;
                    order = pending_orders.erase(order);
                }
            } else {
                ++order;
            }
        }

        fill_log.insert(fill_log.end(), fills.begin(), fills.end());

        if (pq && !fills.empty()) {
            for (auto& f : fills)
                pq->push(FillEvent{f});

            pq->push(BalanceEvent{balance, fills.back().timestamp});

            // push position snapshots for each ticker that was filled
            std::unordered_map<std::string, bool> seen;
            for (auto& f : fills) {
                if (!seen[f.ticker]) {
                    seen[f.ticker] = true;
                    auto it = positions.find(f.ticker);
                    if (it != positions.end())
                        pq->push(PositionSnapshotEvent{it->second});
                }
            }
        }

        return fills;
    }
};
