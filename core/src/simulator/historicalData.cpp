#include "historicalData.hpp"

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

const vector<MarketDataRow>& MarketDataManager::get_bars(const string& ticker) const {
    return market.at(ticker);
}

vector<string> MarketDataManager::get_tickers() const {
    vector<string> tickers;
    for (const auto& [ticker, _] : market) {
        tickers.push_back(ticker);
    }
    return tickers;
}
