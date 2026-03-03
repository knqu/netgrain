#include "../../../lib/uwebsockets/include/App.h"
#include "historicalData.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <algorithm>
#include <cstdio>
#include <filesystem>

using namespace std;

//changed to global object
MarketDataManager data_manager;

int main() {
    // Loading int from /data...
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


    uWS::App() // lowk just let chat route with uws bc idk how to do it and its not fun and I was bored
        // 1. CORS Preflight
        .options("/api/upload", [](auto *res, auto *req) {
            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->writeHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
            // IMPORTANT: Allow our custom header through CORS!
            res->writeHeader("Access-Control-Allow-Headers", "Content-Type, X-File-Name");
            res->end();
        })

        // 2. File Upload Handler
        .post("/api/upload", [](auto *res, auto *req) {

            // Extract the filename from the custom header (uWS headers are strictly lowercase)
            string filename(req->getHeader("x-file-name"));
            if (filename.empty()) {
                filename = "unknown_upload.csv";
            }

            res->onAborted([]() {
                cout << "Upload aborted by client.\n";
            });

            auto buffer = make_shared<string>();

            // Note: We capture 'filename' by value so it survives until the stream finishes
            res->onData([res, buffer, filename](string_view chunk, bool isLast) {
                buffer->append(chunk.data(), chunk.length());

                if (isLast) {
                    // Prepend the data directory to the filename
                    string save_path = "temp_data/" + filename;
                    ofstream out_file(save_path, ios::binary);

                    if (out_file.is_open()) {
                        out_file << *buffer;
                        out_file.close();
                        cout << "Saved file: " << save_path << " (" << buffer->size() << " bytes)\n";

                        string ticker = filename.substr(0, filename.find('.'));
                        transform(ticker.begin(), ticker.end(), ticker.begin(),
                            [](unsigned char c){ return toupper(c); });//idk what this does dawg

                        // DUPLICATE HANDLING
                        if (data_manager.has_ticker(ticker)) {
                            cout << "Duplicate " << ticker << " already in map.\n";
                            remove(save_path.c_str());
                            res->writeHeader("Access-Control-Allow-Origin", "*");
                            res->writeStatus("409 Conflict")->end("Error: " + ticker + " already exists.");
                            return;
                        }

                        //Parsing and Loading main() -> load_ticker_data() -> parse_csv_file() -> helpers for parsing decimals and dates.
                        data_manager.load_ticker_data(ticker, save_path);
                        data_manager.print_first_row(ticker); //print first row to verify loading and parsing worked

                        if (remove(save_path.c_str()) == 0) { //rmv file once in object.
                            cout << "Removed File after loading" << save_path << "\n";
                        }
                        else {
                            cout << "error in remvoing file" << save_path << "\n";
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

        .get("/api/market", [](auto *res, auto *req) {
            string json = data_manager.get_market_state_json();//call to jsonify market state to index.html for testing purposes.

            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->writeHeader("Content-Type", "application/json");
            res->end(json);
        })
        .listen(8080, [](auto *listen_socket) {
            if (listen_socket) {
                cout << "uWebSockets Engine running on port 8080\n";
            } else {
                cout << "Failed to start server.\n";
            }
        })
        .get("/api/run/:ticker", [](auto *res, auto *req) { //added this to run execution on the stored data as a demo purpose, will delete later
               
            // Grab the ticker from the URL (e.g., "AAPL")
            std::string ticker(req->getParameter(0));
            
            // Ensure it's uppercase
            std::transform(ticker.begin(), ticker.end(), ticker.begin(),
                [](unsigned char c){ return std::toupper(c); });

            // Run the algorithm
            std::string report = data_manager.run_basic_backtest(ticker);

            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->writeHeader("Content-Type", "text/plain");
            res->end(report);
        })

        .run();
        
}