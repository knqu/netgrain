#pragma once

#include "def.hpp"
#include <string>
#include <unordered_map>

enum class Side : int { BUY, SELL };
enum class OrderType : int { MARKET, LIMIT };
enum class OrderStatus : int { PENDING, FILLED, CANCELLED };

struct Order {
    int id;
    std::string ticker;
    Side side;
    int quantity;
    OrderType order_type;
    int limit_price;
    OrderStatus status;
};

struct Fill {
    int order_id;
    std::string ticker;
    Side side;
    int fill_price;
    int quantity;
    int date;
};

struct Position {
    std::string ticker;
    i32 quantity;
    int avg_cost;
};

struct Portfolio {
    int balance;
    std::unordered_map<std::string, Position> positions;
};

struct TickerConfig {
    double volatility;  // basis points
    double max_volume;  // liquidity as a percentage of total bar volume
    double trade_fee;
};
