#pragma once

#include <string>
#include <vector>

enum class Side: int {
    BUY,
    SELL
};

enum class OrderType: int {
    MARKET,  // instant execution at open price
    LIMIT,   // buy at or below limit price or sell at or above limit price
    STOP     // buy at or above stop price or sell at or below stop price
};

enum class OrderStatus : int {
    PENDING,
    FILLED,
    CANCELLED
};

struct Order {
    int id;
    std::string ticker;
    int quantity;
    int target_price;  // limit or stop price
    Side side;
    OrderType order_type;
    OrderStatus status;
};

struct Fill {
    int order_id;
    std::string ticker;
    int quantity;
    int fill_price;
    Side side;
    int timestamp;
};

struct Lot {
    int fill_price;
    int quantity;
    int timestamp;
};

struct Position {
    std::string ticker;
    int quantity;
    int cost_basis;
    std::vector<Lot> lots;
};

struct TickerConfig {
    double volatility;
    double trade_fee;
};
