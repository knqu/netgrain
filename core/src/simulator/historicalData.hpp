#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

#include "def.hpp"


constexpr i64 PRICE_SCALE_FACTOR = 100000;

struct MarketDataRow {
    u32 date;      
    i64 open;      
    i64 high;      
    i64 low;       
    i64 close;     
    uint64_t volume;    
    uint64_t open_int;  
};

using MarketDataMap = std::unordered_map<std::string, std::vector<MarketDataRow>>;

class MarketDataManager {
private:
    MarketDataMap market;
    //helpers+parsing functions
    u32 parse_date(std::string date_str);
    i64 parse_price(const std::string& price_str);
    std::vector<MarketDataRow> parse_csv_file(const std::string& filepath);

public:
    //main func + tests
    void load_ticker_data(const std::string& ticker, const std::string& filepath);
    bool has_ticker(const std::string& ticker);
    void print_first_row(const std::string& ticker);
    std::string get_market_state_json();
    std::string run_basic_backtest(const std::string& ticker); //basic backtest engine to demonstrate usage
};