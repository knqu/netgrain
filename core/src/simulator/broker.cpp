#include "broker.hpp"

#include <stdexcept>

double Broker::get_balance() {
    if (!engine) {
        throw std::runtime_error("Engine is not initialized");
    }
    return static_cast<double>(engine->get_balance()) / static_cast<double>(PRICE_SCALE_FACTOR);
}

const std::unordered_map<std::string, Position>& Broker::get_positions() const {
    if (!engine) {
        throw std::runtime_error("Engine is not initialized");
    }
    return engine->get_positions();
}

const std::vector<Order>& Broker::get_pending_orders() const {
    if (!engine) {
        throw std::runtime_error("Engine is not initialized");
    }
    return engine->get_pending_orders();
}

const std::vector<Order>& Broker::get_cancelled_orders() const {
    if (!engine) {
        throw std::runtime_error("Engine is not initialized");
    }
    return engine->get_cancelled_orders();
}

const std::vector<Fill>& Broker::get_fill_log() const {
    if (!engine) {
        throw std::runtime_error("Engine is not initialized");
    }
    return engine->get_fill_log();
}

int Broker::place_order(const std::string& ticker, i64 quantity, double price_dollars, Side side, OrderType order_type) {
    if (!engine) {
        throw std::runtime_error("Engine is not initialized");
    }
    i64 scaled = static_cast<i64>(price_dollars * static_cast<double>(PRICE_SCALE_FACTOR));
    return engine->place_order(ticker, quantity, scaled, side, order_type);
}

bool Broker::cancel_order(int order_id) {
    if (!engine) {
        throw std::runtime_error("Engine is not initialized");
    }
    return engine->cancel_order(order_id);
}
