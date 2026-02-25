#include "../../../lib/uwebsockets/include/App.h"
#include "historicalData.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <algorithm>
#include <cstdio>

MarketDataMap market; //contains all objectified csvs.

int main() {
    uWS::App()
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
            std::string filename(req->getHeader("x-file-name"));
            if (filename.empty()) {
                filename = "unknown_upload.csv";
            }

            res->onAborted([]() {
                std::cout << "Upload aborted by client.\n";
            });

            auto buffer = std::make_shared<std::string>();

            // Note: We capture 'filename' by value so it survives until the stream finishes
            res->onData([res, buffer, filename](std::string_view chunk, bool isLast) {
                buffer->append(chunk.data(), chunk.length());

                if (isLast) {
                    // Prepend the data directory to the filename
                    std::string save_path = "temp_data/" + filename;
                    std::ofstream out_file(save_path, std::ios::binary);

                    if (out_file.is_open()) {
                        out_file << *buffer;
                        out_file.close();
                        std::cout << "Saved file: " << save_path << " (" << buffer->size() << " bytes)\n";

                        std::string ticker = filename.substr(0, filename.find('.'));
                        std::transform(ticker.begin(), ticker.end(), ticker.begin(),
                            [](unsigned char c){ return std::toupper(c); });//idk what this does dawg

                        // DUPLICATE HANDLING
                        if (market.find(ticker) != market.end()) {
                            std::cout << "Duplicate " << ticker << " already in map.\n";
                            std::remove(save_path.c_str());
                            res->writeHeader("Access-Control-Allow-Origin", "*");
                            res->writeStatus("409 Conflict")->end("Error: " + ticker + " already exists.");
                            return;
                        }

                        //Parsing and Loading main() -> load_ticker_data() -> parse_csv_file() -> helpers for parsing decimals and dates.
                        load_ticker_data(market, ticker, save_path);

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

                        if (std::remove(save_path.c_str()) == 0) { //rmv file once in object.
                            std::cout << "Removed File after loading" << save_path << "\n";
                        }
                        else {
                            std::cout << "error in remvoing file" << save_path << "\n";

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
            std::string json = "{";
            bool first = true;

            // Loop through the global map
            for (const auto& [ticker, data_vector] : market) {
                if (!first) json += ", ";
                // Format: "AAPL": 10543
                json += "\"" + ticker + "\": " + std::to_string(data_vector.size());
                first = false;
            }
            json += "}";

            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->writeHeader("Content-Type", "application/json");
            res->end(json);
        })
        .listen(8080, [](auto *listen_socket) {
            if (listen_socket) {
                std::cout << "uWebSockets Engine running on port 8080\n";
            } else {
                std::cout << "Failed to start server.\n";
            }
        })
        .run();
}
