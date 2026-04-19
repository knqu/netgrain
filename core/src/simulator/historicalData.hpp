#pragma once

#include "def.hpp"

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

using namespace std;

constexpr i64 PRICE_SCALE_FACTOR = 100000;

struct MarketDataRow {
    u32 date;      
    i64 open;      
    i64 high;      
    i64 low;       
    i64 close;     
    u64 volume;    
    u64 open_int;
};

using MarketDataMap = unordered_map<string, vector<MarketDataRow>>;

class MarketDataManager {
private:
    MarketDataMap market;
    //helpers+parsing functions
    unordered_map<string, string> asset_classes; //asset class mapper for each ticker
    u32 parse_date(string date_str);
    i64 parse_price(const string& price_str);
    vector<MarketDataRow> parse_csv_file(const string& filepath);

public:
    bool load_ticker_data(const string& ticker, const string& filepath, const string& asset_class);
    bool has_ticker(const string& ticker);
    void print_first_row(const string& ticker);
    string get_market_state_json();
    const vector<MarketDataRow>& get_bars(const string& ticker) const;
    vector<string> get_tickers() const;
};
