#include "../../../lib/uwebsockets/include/App.h"
#include "historicalData.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <regex>
#include <cstdint>


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

                string ticker = file_name.substr(0, file_name.find('.'));
                transform(ticker.begin(), ticker.end(), ticker.begin(),
                    [](unsigned char c){ return toupper(c); });
                
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
                        //CHANGED TO BOOLEAN TO PROPOGATE CHANGES TO SIMULATION.TSX
                        bool load_success = data_manager.load_ticker_data(ticker, save_path);
                        
                        if (load_success) {
                            data_manager.print_first_row(ticker); 
                        }

                        if (remove(save_path.c_str()) == 0) {
                            cout << "Removed File after loading (save it in temp, read, delete): " << save_path << "\n";
                        } else {
                            cout << "Error in removing file: " << save_path << "\n";
                        }
                        //message based on load_success
                        res->writeHeader("Access-Control-Allow-Origin", "*");
                        if (load_success) {
                            res->end(ticker); // Sends standard 200 OK
                        } else {
                            res->writeStatus("400 Bad Request")->end("Invalid or corrupted CSV data");
                        }

                    } else {
                        res->writeHeader("Access-Control-Allow-Origin", "*");
                        res->writeStatus("500 Internal Server Error")->end("Failed to save to disk.");
                    }
                }
            });
        })
        .options("/api/results", [](auto *res, auto *req) {
            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->writeHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
            res->writeHeader("Access-Control-Allow-Headers", "Content-Type");
            res->end();
        })
        // THIS WILL BE REMOVED IN THE FUTURE IT IS TO SHOW JSON WORKS
        .get("/api/results", [](auto *res, auto *req) {
            
            SimulationConfig dummy_config;
            dummy_config.initial_capital = 100000;
            dummy_config.trade_fee = 1.50 * 100000;
            dummy_config.start_date = "2013-12-10";
            dummy_config.end_date = "2017-11-10";
            dummy_config.stocks.push_back(Stocks());
            dummy_config.stocks[0].name = "AAPL";
            
            string output_json = data_manager.run_simulation(dummy_config);
            
            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->writeHeader("Content-Type", "application/json");
            res->end(output_json);
        })

        // 3. Market State Endpoint
        .get("/api/market", [](auto *res, auto *req) {
            string json = data_manager.get_market_state_json();
            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->writeHeader("Content-Type", "application/json");
            res->end(json);
        })
        // 4. CORS Preflight for Simulation ye idk what this does
        .options("/api/simulate", [](auto *res, auto *req) {
            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->writeHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
            res->writeHeader("Access-Control-Allow-Headers", "Content-Type");
            res->end();
        })

        // 5. Simulation Settings Handler
        .post("/api/simulate", [](auto *res, auto *req) {
            res->onAborted([]() {
                cout << "Simulation request aborted by client.\n";
            });

            auto buffer = make_shared<string>();

            res->onData([res, buffer](string_view chunk, bool isLast) {
                buffer->append(chunk.data(), chunk.length());
                //possibly put these parsing functions in a different file later on, but for now this is fine for testing purposes?
                if (isLast) {
                    string json = *buffer;
                    SimulationConfig config;
                    smatch match;

                    /* Acceptance Critera - HX */
                    std::ofstream outfile("initial_market.json");
                    if (outfile.is_open()) {
                        outfile << json;
                        outfile.close();
                        std::cout << "successfully written\n";
                    } else {
                        std::cerr << "failed to write\n";
                    }

                    // Parse Initial Capital
                    if (regex_search(json, match, regex(R"("initial_capital":\s*([0-9.]+))"))) {
                        double raw_cap = stod(match[1].str());
                        config.initial_capital = static_cast<int64_t>(raw_cap * 100000); 
                    }

                    // Parse Trade Fee
                    if (regex_search(json, match, regex(R"("trade_fee":\s*([0-9.]+))"))) {
                        double raw_fee = stod(match[1].str());
                        config.trade_fee = static_cast<int64_t>(raw_fee * 100000); 
                    }

                    // Parse Dates
                    if (regex_search(json, match, regex(R"("start_date":\s*"([^"]*))"))) {
                        config.start_date = match[1].str();
                    }
                    if (regex_search(json, match, regex(R"("end_date":\s*"([^"]*))"))) {
                        config.end_date = match[1].str();
                    }

                    // Parse Tickers Array
                     std::regex stock_regex(R"-("ticker":\s*"([^"]*)",\s*"base_price":\s*([^,]*),\s*"liquidity":\s*([^,]*),\s*"volatility":\s*([^,]*),\s*"market_cap":\s*([^}]*))-");

                    int i = 0;

                    while (std::regex_search(json, match, stock_regex)) {
                      config.stocks.push_back(Stocks());
                      
                      config.stocks[i].name = match[1].str(); 
                      config.stocks[i].base_price = std::stod(match[2].str()); 
                      config.stocks[i].liquidity  = std::stod(match[3].str());
                      config.stocks[i].volatility = std::stod(match[4].str());
                      config.stocks[i].market_cap = std::stod(match[5].str());
                      
                      i++;
                      
                      json = match.suffix().str(); 
                    }

                    // Prove the struct was populated correctly
                    data_manager.print_config(config);
                    string output_json = data_manager.run_simulation(config);
                    res->writeHeader("Access-Control-Allow-Origin", "*");
                    res->writeHeader("Content-Type", "application/json");
                    res->end(output_json);                
                }
            });
        })

        // 6. LISTEN & RUN (Must be absolutely last in the chain)
        .listen(8080, [](auto *listen_socket) {
            if (listen_socket) {
                cout << "uWebSockets Engine running on port 8080\n";
            } else {
                cout << "Failed to start server.\n";
            }
        })
        .run();
}
