#include "historicalData.hpp"
#include "../generator/generator.cpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

using namespace std;

//helper function to parse date string in format "YYYY-MM-DD" and convert to u32 in format YYYYMMDD for easier storage and comparison
u32 MarketDataManager::parse_date(string date_str) {
    string out = "";
    for (char c : date_str) {
        if (c != '-') out += c;
    }
    return stoul(out);
}

//helper function to parse price string and convert to i64 with scaling factor for precision
i64 MarketDataManager::parse_price(const string& price_str) {
    double raw_price = stod(price_str);
    return static_cast<i64>(raw_price * PRICE_SCALE_FACTOR);
}

//main parsing function
vector<MarketDataRow> MarketDataManager::parse_csv_file(const string& filepath) {
    vector<MarketDataRow> rows;
    ifstream file(filepath);  

    if (!file.is_open()) {
        throw runtime_error("Could not open file: " + filepath);
    }

    string line;

    if (getline(file, line)) {
        // Skip header
    }

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line); // grab the line and make a stringstream for parsing
        string token;

        string t_date, t_open, t_high, t_low, t_close, t_vol, t_oi; 
        
        //validation check
        if (getline(ss, t_date, ',') && getline(ss, t_open, ',') &&
            getline(ss, t_high, ',') && getline(ss, t_low, ',') &&
            getline(ss, t_close, ',') && getline(ss, t_vol, ',') &&
            getline(ss, t_oi, ',')) {
            try {
                MarketDataRow row;
                row.date     = parse_date(t_date);
                row.open     = parse_price(t_open);
                row.high     = parse_price(t_high);
                row.low      = parse_price(t_low);
                row.close    = parse_price(t_close);
                row.volume   = stoull(t_vol);
                row.open_int = stoull(t_oi);

                rows.push_back(row);
            } catch (const exception& e) {
                cerr << " Error: Incorrect data found in row: " << line << "\n"; //data type err
                file.close();
                return {}; // on any parsing error, INSTANTLY ABORT and return empty vector to signal failure to load ticker data
            }
        } else {
            cerr << " Error: Incorrect number of columns in row: " << line << "\n"; //format err
            file.close();
            return {}; // on any parsing error, INSTANTLY ABORT and return empty vector to signal failure to load ticker data
        }
    }
    file.close();
    return rows;
}

//main function to load ticker data and ticker from file, calls parsing function and stores result in market map for later retrieval
//returns true on succesful storege of map
bool MarketDataManager::load_ticker_data(const string& ticker, const string& filepath, const string& asset_class) {
    cout << "Loading data for " << ticker << " (Class: " << asset_class << ")...\n";
    
    vector<MarketDataRow> data = parse_csv_file(filepath);
    
    if (!data.empty()) {
        market[ticker] = data; 
        asset_classes[ticker] = asset_class; // <-- NEW: Store its category!
        cout << ticker << " loaded successfully into memory.\n";
        return true;
    } else {
        cerr << "Failed to load data for " << ticker << " due to corrupted data.\n";
        return false;
    }
}

//helper function to check if a ticker's data is in the market map
bool MarketDataManager::has_ticker(const string& ticker) {
    return market.find(ticker) != market.end();
}

//also another function for testing purposes to print the first row of a ticker's data to verify it was loaded correctly
void MarketDataManager::print_first_row(const string& ticker) {
    if (!market[ticker].empty()) {
        const auto& first_row = market[ticker].front();
        cout << "\n--- FIRST ROW OF " << ticker << " ---\n";
        cout << "Date:     " << first_row.date << "\n";
        cout << "Open:     " << first_row.open << "\n";
        cout << "High:     " << first_row.high << "\n";
        cout << "Low:      " << first_row.low << "\n";
        cout << "Close:    " << first_row.close << "\n";
        cout << "Volume:   " << first_row.volume << "\n";
        cout << "Open Int: " << first_row.open_int << "\n";
        cout << "---------------------------\n\n";
    }
}

//added helper function to get market state as json string for testing purposes
string MarketDataManager::get_market_state_json() {
    //group tickers by asset class
    unordered_map<string, vector<string>> grouped;
    for (const auto& [ticker, data_vector] : market) {
        //fallback just in case asset class not found
        string a_class = asset_classes.count(ticker) ? asset_classes[ticker] : "Stocks";
        grouped[a_class].push_back(ticker);
    }

    string json = "{";
    bool first_class = true;
    
    for (const auto& [a_class, tickers] : grouped) {
        if (!first_class) json += ", ";
        
        json += "\"" + a_class + "\": [";
        bool first_ticker = true;
        for (const string& t : tickers) {
            if (!first_ticker) json += ", ";
            json += "\"" + t + "\"";
            first_ticker = false;
        }
        json += "]";
        
        first_class = false;
    }
    json += "}";
    
    return json;
}

//added a serialize to json function to conver sum results to json, can be changed in the future******
string MarketDataManager::serialize_results_to_json(const SimulationConfig& config, const SimulationMetrics& metrics) {
    ostringstream json;
    json << "{\n";
    
    json << "  \"config\": {\n";
    json << "    \"initial_capital\": " << metrics.initial_balance << ",\n";
    
    json << "    \"stocks\": [";
    for (size_t i = 0; i < config.stocks.size(); ++i) {
        json << "{";
        json << "\"ticker\":\"" << config.stocks[i].name << "\",";
        json << "\"base_price\":\"" << to_string(config.stocks[i].base_price) << "\",";
        json << "\"liquidity\":\"" << to_string(config.stocks[i].liquidity) << "\",";
        json << "\"volatility\":\"" << to_string(config.stocks[i].volatility) << "\",";
        json << "\"market_cap\":\"" << to_string(config.stocks[i].market_cap) << "\",";
        json << "}";
        if (i < config.stocks.size() - 1) json << ", ";
    }
    json << "],\n";
    
    json << "    \"start_date\": \"" << config.start_date << "\",\n";
    json << "    \"end_date\": \"" << config.end_date << "\",\n";
    
    double unscaled_fee = static_cast<double>(config.trade_fee) / 100000.0;
    json << "    \"trade_fee\": " << unscaled_fee << "\n";
    json << "  },\n";

    json << "  \"metrics\": {\n";
    json << "    \"total_trades\": " << metrics.total_trades << ",\n";
    json << "    \"winning_trades\": " << metrics.winning_trades << ",\n";
    json << "    \"losing_trades\": " << metrics.losing_trades << ",\n";
    json << "    \"win_rate_percent\": " << metrics.win_rate_percent << ",\n";
    json << "    \"initial_balance\": " << metrics.initial_balance << ",\n";
    json << "    \"final_balance\": " << metrics.final_balance << ",\n";
    json << "    \"net_profit\": " << metrics.net_profit << "\n";
    json << "  }\n";
    
    json << "}";

    return json.str();
}


void MarketDataManager::initialize_generators(const std::vector<Stocks>& stocks) {
  for (const auto& stock : stocks)
  {
    // Waiting for generator class to work

  }

}

//added very basic testing function to show a basic execution on the stored data -> change made to accept simulation config struct to handle output of simulation
// as well as sending packet to frontend with output for user to download metrics.
string MarketDataManager::run_simulation(const SimulationConfig& config) {
    //check if empty
    if (config.stocks.empty()) {
        return "{\"error\": \"No tickers provided in config\"}";
    }

    string ticker = config.stocks[0].name; //for now grab first ticker in list for functional demonstration.

    if (market.find(ticker) == market.end() || market[ticker].empty()) {
        return "{\"error\": \"Ticker data not found: " + ticker + "\"}";
    }

    const auto& data = market[ticker];
    int winning_trades = 0;
    int losing_trades = 0;
    i64 total_profit_scaled = 0;

    //random ahh algorithm -> idk how it works dont ask, its to show data in storage is operable
    for (const auto& row : data) {
        // Buy at open, sell at close, AND deduct the trading fee!
        i64 daily_profit = (row.close - row.open) - config.trade_fee; 
        
        total_profit_scaled += daily_profit;

        if (daily_profit > 0) {
            winning_trades++;
        } else if (daily_profit < 0) {
            losing_trades++;
        }
    }
    //calc final metrics:
    SimulationMetrics metrics;
    metrics.total_trades = winning_trades + losing_trades;
    metrics.winning_trades = winning_trades;
    metrics.losing_trades = losing_trades;
    metrics.win_rate_percent = metrics.total_trades > 0 ? (static_cast<double>(winning_trades) / metrics.total_trades) * 100.0 : 0.0;
    
    metrics.net_profit = static_cast<double>(total_profit_scaled) / 100000.0;
    metrics.initial_balance = static_cast<double>(config.initial_capital) / 100000.0;
    metrics.final_balance = metrics.initial_balance + metrics.net_profit;

    //send to serializer
    return serialize_results_to_json(config, metrics);
}


void MarketDataManager::print_config(const SimulationConfig& config) {
        cout << "\n--- SIMULATION CONFIG STRUCT --- \n";
        cout << "Initial Capital (Scaled): " << config.initial_capital << "\n";
        cout << "Trade Fee (Scaled):       " << config.trade_fee << "\n";
        cout << "Start Date:               " << (config.start_date.empty() ? "None" : config.start_date) << "\n";
        cout << "End Date:                 " << (config.end_date.empty() ? "None" : config.end_date) << "\n";
        cout << "Tickers to Trade:         ";
        for (const auto& t : config.stocks) cout << t.name << " ";
        cout << "\n--------------------------------------\n\n";
}
