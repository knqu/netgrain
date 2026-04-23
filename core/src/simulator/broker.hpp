#pragma once

#include "engine.hpp"
#include "historicalData.hpp"
#include "order.hpp"

#include <string>
#include <unordered_map>
#include <vector>

struct Broker {
    Engine* engine = nullptr;

    explicit Broker(Engine* e) : engine(e) {}

    // getters
    double get_balance();  // in dollars (engine uses scaled i64)
    const std::unordered_map<std::string, Position>& get_positions() const;
    const std::vector<Order>& get_pending_orders() const;
    const std::vector<Order>& get_cancelled_orders() const;
    const std::vector<Fill>& get_fill_log() const;

    // ordering
    int place_order(const std::string& ticker, i64 quantity, double price_dollars, Side side, OrderType order_type);
    bool cancel_order(int order_id);
};
