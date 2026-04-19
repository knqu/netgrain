#pragma once

#include "def.hpp"

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
    PARTIAL,
    CANCELLED
};

struct Order {
    int id;
    std::string ticker;
    i64 quantity;
    i64 target_price;  // limit or stop price
    Side side;
    OrderType order_type;
    OrderStatus status;
};

struct Fill {
    int order_id;
    std::string ticker;
    i64 quantity;
    i64 fill_price;
    Side side;
    u32 timestamp;
};

struct Lot {
    i64 fill_price;
    i64 quantity;
    u32 timestamp;
};

struct Position {
    std::string ticker;
    i64 quantity = 0;
    i64 cost_basis = 0;
    std::vector<Lot> lots;
};

struct FeesAndTaxes { // HX; each ticker/stock specific
    double flat_fee_comm_per_share;
    double percentage_comm_per_share;
};

struct TickerConfig {
    double volatility;
    double trade_fee;
    double short_term_tax;
    FeesAndTaxes fees_and_taxes;
};
