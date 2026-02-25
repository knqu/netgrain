#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>


# include "def.hpp"

constexpr i64 PRICE_SCALE_FACTOR = 100000; // mult prices to avoid floating point issues

// The struct representing a single row of historical market data
struct MarketDataRow {
    u32 date;
    i64  open;
    i64  high;
    i64  low; 
    i64  close;
    u64 volume;
    u64 open_int;
};

using MarketDataMap = std::unordered_map<std::string, std::vector<MarketDataRow>>; // Ticker Historical Data Map

std::vector<MarketDataRow> parse_csv_file(const std::string& filepath);
void load_ticker_data(MarketDataMap& market_map, const std::string& ticker, const std::string& filepath);