#pragma once

#include <string>
#include <vector>
#include <cstdint>
# include "def.hpp"

constexpr int64_t PRICE_SCALE_FACTOR = 100000;

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


std::vector<MarketDataRow> parse_csv_file(const std::string& filepath);