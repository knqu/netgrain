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

struct Stocks {
  std::string name;
  int base_price;
  int liquidity;
  int volatility;
  int market_cap;
};

struct SimulationConfig {
    int64_t initial_capital = 0; 
    vector<Stocks> stocks; 
    string start_date;
    string end_date;
    int64_t trade_fee = 0;
};

//this might be moved somewhere else later on.
struct SimulationMetrics {
    int total_trades;
    int winning_trades;
    int losing_trades;
    double win_rate_percent;
    double initial_balance;
    double final_balance;
    double net_profit;
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
    string serialize_results_to_json(const SimulationConfig& config, const SimulationMetrics& metrics);

public:
    //main func + tests
    bool load_ticker_data(const string& ticker, const string& filepath, const string& asset_class);
    bool has_ticker(const string& ticker);
    void print_first_row(const string& ticker);
    string get_market_state_json();
    string run_simulation(const SimulationConfig& config);
    void print_config(const SimulationConfig& config);
    void initialize_generators(const std::vector<Stocks>& stocks);
};
