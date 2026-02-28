#include "historicalData.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

u32 parse_date(std::string date_str) {

    std::string out = "";

    for (char c : date_str) {
        if (c != '-') out += c;
    }

    return std::stoul(out);
}

i64 parse_price(const std::string& price_str) {
    double raw_price = std::stod(price_str);
    return static_cast<i64>(raw_price * PRICE_SCALE_FACTOR);
}

std::vector<MarketDataRow> parse_csv_file(const std::string& filepath) {
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

        std::stringstream ss(line);
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
                std::cerr << "Incorrect data found in row: " << line << "\n"; //return on incorrect data format
                return {};
            }
        } else {
            std::cerr << "Warning: Incorrect number of columns in row: " << line << "\n";
        }
    }

    file.close();
    return rows;
}


void load_ticker_data(MarketDataMap& market_map, const std::string& ticker, const std::string& filepath) {
    std::cout << "Loading data for " << ticker << "...\n";
    
    // Call the parser
    std::vector<MarketDataRow> data = parse_csv_file(filepath);
    
    if (!data.empty()) {
        market_map[ticker] = data; // Store it in map
        std::cout<< ticker << " loaded successfully into memory.\n";
    } else {
        std::cerr << "Failed to load data for " << ticker << "\n";
    }



}