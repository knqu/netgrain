#include "../../../lib/uwebsockets/include/App.h"
#include "historicalData.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

int main() {
    uWS::App()
        // 1. Handle the OPTIONS preflight for CORS (Browsers require this)
        .options("/api/upload", [](auto *res, auto *req) {
            res->writeHeader("Access-Control-Allow-Origin", "*");
            res->writeHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
            res->writeHeader("Access-Control-Allow-Headers", "Content-Type");
            res->end();
        })
        
        // 2. Handle the actual File Upload
        .post("/api/upload", [](auto *res, auto *req) {
            
            // uWS requires handling aborted connections to prevent crashes
            res->onAborted([]() {
                std::cout << "Upload aborted by client.\n";
            });

            // Create a buffer to hold the incoming file chunks
            auto buffer = std::make_shared<std::string>();

            // Read the data stream
            res->onData([res, buffer](std::string_view chunk, bool isLast) {
                buffer->append(chunk.data(), chunk.length());

                // When the entire file has arrived
                if (isLast) {
                    std::string save_path = "temp_data/uploaded_test.csv";
                    std::ofstream out_file(save_path, std::ios::binary);
                    
                    if (out_file.is_open()) {
                        out_file << *buffer;
                        out_file.close();
                        std::cout << "✅ Saved file. Size: " << buffer->size() << " bytes\n";
                        
                        // Parse the data!
                        // std::vector<MarketDataRow> history = parse_csv_file(save_path);
                        
                        res->writeHeader("Access-Control-Allow-Origin", "*");
                        res->end("File uploaded and processed successfully.");
                    } else {
                        res->writeHeader("Access-Control-Allow-Origin", "*");
                        res->writeStatus("500 Internal Server Error")->end("Failed to save.");
                    }
                }
            });
        })
        .listen(8080, [](auto *listen_socket) {
            if (listen_socket) {
                std::cout << "uWebSockets Engine running on port 8080\n";
            } else {
                std::cout << "Failed to start server. Port 8080 might be in use.\n";
            }
        })
        .run();
}