#include "historicalData.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

//helper function to parse date string in format "YYYY-MM-DD" and convert to u32 in format YYYYMMDD for easier storage and comparison
u32 MarketDataManager::parse_date(std::string date_str) {
    std::string out = "";
    for (char c : date_str) {
        if (c != '-') out += c;
    }
    return std::stoul(out);
}

//helper function to parse price string and convert to i64 with scaling factor for precision
i64 MarketDataManager::parse_price(const std::string& price_str) {
    double raw_price = std::stod(price_str);
    return static_cast<i64>(raw_price * PRICE_SCALE_FACTOR);
}

//main parsing function
std::vector<MarketDataRow> MarketDataManager::parse_csv_file(const std::string& filepath) {
    std::vector<MarketDataRow> rows;
    std::ifstream file(filepath);  

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }

    std::string line;

    if (std::getline(file, line)) {
        // Skip header
    }

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line); // grab the line and make a stringstream for parsing
        std::string token;

        std::string t_date, t_open, t_high, t_low, t_close, t_vol, t_oi; 
        
        //validation check
        if (std::getline(ss, t_date, ',') && std::getline(ss, t_open, ',') &&
            std::getline(ss, t_high, ',') && std::getline(ss, t_low, ',') &&
            std::getline(ss, t_close, ',') && std::getline(ss, t_vol, ',') &&
            std::getline(ss, t_oi, ',')) {
            try {
                MarketDataRow row;
                row.date     = parse_date(t_date);
                row.open     = parse_price(t_open);
                row.high     = parse_price(t_high);
                row.low      = parse_price(t_low);
                row.close    = parse_price(t_close);
                row.volume   = std::stoull(t_vol);
                row.open_int = std::stoull(t_oi);

                rows.push_back(row);
            } catch (const std::exception& e) {
                std::cerr << " Error: Incorrect data found in row: " << line << "\n"; //data type err
                file.close();
                return {}; // on any parsing error, INSTANTLY ABORT and return empty vector to signal failure to load ticker data
            }
        } else {
            std::cerr << " Error: Incorrect number of columns in row: " << line << "\n"; //format err
            file.close();
            return {}; // on any parsing error, INSTANTLY ABORT and return empty vector to signal failure to load ticker data
        }
    }
    file.close();
    return rows;
}

//main function to load ticker data and ticker from file, calls parsing function and stores result in market map for later retrieval
//returns true on succesful storege of map
bool MarketDataManager::load_ticker_data(const std::string& ticker, const std::string& filepath) {
    std::cout << "Loading data for " << ticker << "...\n";
    
    // Call the parser
    std::vector<MarketDataRow> data = parse_csv_file(filepath);
    
    if (!data.empty()) {
        market[ticker] = data; // Store it in map
        std::cout<< ticker << " loaded successfully into memory.\n";
        return true;
    } else {
        std::cerr << "Failed to load data for " << ticker << " due to corrupted data.\n";
        return false;
    }
}

//helper function to check if a ticker's data is in the market map
bool MarketDataManager::has_ticker(const std::string& ticker) {
    return market.find(ticker) != market.end();
}

//also another function for testing purposes to print the first row of a ticker's data to verify it was loaded correctly
void MarketDataManager::print_first_row(const std::string& ticker) {
    if (!market[ticker].empty()) {
        const auto& first_row = market[ticker].front();
        std::cout << "\n--- FIRST ROW OF " << ticker << " ---\n";
        std::cout << "Date:     " << first_row.date << "\n";
        std::cout << "Open:     " << first_row.open << "\n";
        std::cout << "High:     " << first_row.high << "\n";
        std::cout << "Low:      " << first_row.low << "\n";
        std::cout << "Close:    " << first_row.close << "\n";
        std::cout << "Volume:   " << first_row.volume << "\n";
        std::cout << "Open Int: " << first_row.open_int << "\n";
        std::cout << "---------------------------\n\n";
    }
}

//added helper function to get market state as json string for testing purposes
std::string MarketDataManager::get_market_state_json() {
    std::string json = "{";
    bool first = true;

    for (const auto& [ticker, data_vector] : market) {
        if (!first) json += ", ";
        json += "\"" + ticker + "\": " + std::to_string(data_vector.size());
        first = false;
    }
    json += "}";
    return json;
}

//added very basic testing function to show a basic execution on the stored data
std::string MarketDataManager::run_basic_backtest(const std::string& ticker) {
    //check for data
    if (market.find(ticker) == market.end() || market[ticker].empty()) {
        return "Error: No data loaded for " + ticker + "\n";
    }

    const auto& data = market[ticker];
    int winning_trades = 0;
    int losing_trades = 0;
    i64 total_profit_scaled = 0;

    for (const auto& row : data) {
        i64 daily_profit = row.close - row.open;
        total_profit_scaled += daily_profit;

        if (daily_profit > 0) {
            winning_trades++;
        } else if (daily_profit < 0) {
            losing_trades++;
        }
    }

    double actual_profit = static_cast<double>(total_profit_scaled) / 100000.0; 
    
    std::string result = "BACKTEST RESULTS FOR " + ticker + "\n";
    result += "Days Analyzed: " + std::to_string(data.size()) + "\n";
    result += "Winning Days:  " + std::to_string(winning_trades) + "\n";
    result += "Losing Days:   " + std::to_string(losing_trades) + "\n";
    result += "Total Net PnL: $" + std::to_string(actual_profit) + "\n";
    result += "-----------------------------------\n";

    return result;
}