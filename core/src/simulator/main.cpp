#include "../../../lib/uwebsockets/include/App.h"
#include "historicalData.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <regex>
#include <cstdint>

using namespace std;

//sim config struct -- Placed this here for now as test... move later on to diff file?
struct SimulationConfig {
    int64_t initial_capital = 0; 
    std::vector<std::string> tickers; 
    std::string start_date;
    std::string end_date;
    int64_t trade_fee = 0;

    // Helper to print the config struct for testing purposes
    void print() const {
        std::cout << "\n--- SIMULATION CONFIG STRUCT --- \n";
        std::cout << "Initial Capital (Scaled): " << initial_capital << "\n";
        std::cout << "Trade Fee (Scaled):       " << trade_fee << "\n";
        std::cout << "Start Date:               " << (start_date.empty() ? "None" : start_date) << "\n";
        std::cout << "End Date:                 " << (end_date.empty() ? "None" : end_date) << "\n";
        std::cout << "Tickers to Trade:         ";
        for (const auto& t : tickers) std::cout << t << " ";
        std::cout << "\n--------------------------------------\n\n";
    }
};

// Global object
MarketDataManager data_manager;

int main() {
    // Loading init from /data...
    string default_dir = "../../../data/";

    if (filesystem::exists(default_dir) && filesystem::is_directory(default_dir)) {
        for (const auto& entry : filesystem::directory_iterator(default_dir)) {
            cout << entry.path() << "\n";
            if (entry.is_regular_file()) {
                string file_path = entry.path().string();
                string file_name = entry.path().filename().string();

                std::string ticker = file_name.substr(0, file_name.find('.'));
                std::transform(ticker.begin(), ticker.end(), ticker.begin(),
                    [](unsigned char c){ return std::toupper(c); });
                
                data_manager.load_ticker_data(ticker, file_path);
            }
        }
        cout << "Success Loading in stored data\n";
    }
    else {
        cout << "No Default Data Directory Found, continuing to user inputs\n";
    }

    // THE SERVER CHAIN BEGINS
    uWS::App() 
        // 1. CORS Preflight for Upload
        .options("/api/upload", [](auto *res, auto *req) {
            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->writeHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
            res->writeHeader("Access-Control-Allow-Headers", "Content-Type, X-File-Name");
            res->end();
        })

        // 2. File Upload Handler
        .post("/api/upload", [](auto *res, auto *req) {
            string filename(req->getHeader("x-file-name"));
            if (filename.empty()) {
                filename = "unknown_upload.csv";
            }

            res->onAborted([]() {
                cout << "Upload aborted by client.\n";
            });

            auto buffer = make_shared<string>();

            res->onData([res, buffer, filename](string_view chunk, bool isLast) {
                buffer->append(chunk.data(), chunk.length());

                if (isLast) {
                    string save_path = "temp_data/" + filename;
                    ofstream out_file(save_path, ios::binary);

                    if (out_file.is_open()) {
                        out_file << *buffer;
                        out_file.close();
                        cout << "Saved file: " << save_path << " (" << buffer->size() << " bytes)\n";

                        string ticker = filename.substr(0, filename.find('.'));
                        transform(ticker.begin(), ticker.end(), ticker.begin(),
                            [](unsigned char c){ return toupper(c); });

                        // DUPLICATE HANDLING
                        if (data_manager.has_ticker(ticker)) {
                            cout << "Duplicate " << ticker << " already in map.\n";
                            remove(save_path.c_str());
                            res->writeHeader("Access-Control-Allow-Origin", "*");
                            res->writeStatus("409 Conflict")->end("Error: " + ticker + " already exists.");
                            return;
                        }

                        data_manager.load_ticker_data(ticker, save_path);
                        data_manager.print_first_row(ticker); 

                        if (remove(save_path.c_str()) == 0) {
                            cout << "Removed File after loading (save it in temp, read, delete): " << save_path << "\n";
                        } else {
                            cout << "Error in removing file: " << save_path << "\n";
                        }

                        res->writeHeader("Access-Control-Allow-Origin", "*");
                        res->end(ticker);
                    } else {
                        res->writeHeader("Access-Control-Allow-Origin", "*");
                        res->writeStatus("500 Internal Server Error")->end("Failed to save to disk.");
                    }
                }
            });
        })

        // 3. Market State Endpoint
        .get("/api/market", [](auto *res, auto *req) {
            string json = data_manager.get_market_state_json();
            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->writeHeader("Content-Type", "application/json");
            res->end(json);
        })

        // 4. Demo to show data from map is usable accessible, etc.
        .get("/api/run/:ticker", [](auto *res, auto *req) { 
            std::string ticker(req->getParameter(0));
            std::transform(ticker.begin(), ticker.end(), ticker.begin(),
                [](unsigned char c){ return std::toupper(c); });

            std::string report = data_manager.run_basic_backtest(ticker);

            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->writeHeader("Content-Type", "text/plain");
            res->end(report);
        })

        // 5. CORS Preflight for Simulation ye idk what this does
        .options("/api/simulate", [](auto *res, auto *req) {
            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->writeHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
            res->writeHeader("Access-Control-Allow-Headers", "Content-Type");
            res->end();
        })

        // 6. Simulation Settings Handler
        .post("/api/simulate", [](auto *res, auto *req) {
            res->onAborted([]() {
                std::cout << "Simulation request aborted by client.\n";
            });

            auto buffer = std::make_shared<std::string>();

            res->onData([res, buffer](string_view chunk, bool isLast) {
                buffer->append(chunk.data(), chunk.length());
                //possibly put these parsing functions in a different file later on, but for now this is fine for testing purposes?
                if (isLast) {
                    std::string json = *buffer;
                    SimulationConfig config;
                    std::smatch match;

                    // Parse Initial Capital
                    if (std::regex_search(json, match, std::regex(R"("initial_capital":\s*([0-9.]+))"))) {
                        double raw_cap = std::stod(match[1].str());
                        config.initial_capital = static_cast<int64_t>(raw_cap * 100000); 
                    }

                    // Parse Trade Fee
                    if (std::regex_search(json, match, std::regex(R"("trade_fee":\s*([0-9.]+))"))) {
                        double raw_fee = std::stod(match[1].str());
                        config.trade_fee = static_cast<int64_t>(raw_fee * 100000); 
                    }

                    // Parse Dates
                    if (std::regex_search(json, match, std::regex(R"("start_date":\s*"([^"]*))"))) {
                        config.start_date = match[1].str();
                    }
                    if (std::regex_search(json, match, std::regex(R"("end_date":\s*"([^"]*))"))) {
                        config.end_date = match[1].str();
                    }

                    // Parse Tickers Array
                    if (std::regex_search(json, match, std::regex(R"("tickers":\s*\[(.*?)\])"))) {
                        std::string tickers_str = match[1].str();
                        std::regex ticker_regex(R"("([^"]+))");
                        auto words_begin = std::sregex_iterator(tickers_str.begin(), tickers_str.end(), ticker_regex);
                        auto words_end = std::sregex_iterator();

                        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                            config.tickers.push_back((*i)[1].str());
                        }
                    }

                    // Prove the struct was populated correctly
                    config.print();

                    res->writeHeader("Access-Control-Allow-Origin", "*");
                    res->end("Struct populated successfully! Check C++ terminal.");
                }
            });
        })

        // 7. LISTEN & RUN (Must be absolutely last in the chain)
        .listen(8080, [](auto *listen_socket) {
            if (listen_socket) {
                cout << "uWebSockets Engine running on port 8080\n";
            } else {
                cout << "Failed to start server.\n";
            }
        })
        .run();
}