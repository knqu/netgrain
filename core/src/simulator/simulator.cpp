#include "simulator.hpp"

#include <sstream>
#include <algorithm>

SimulationResult run_csv_simulation(Engine& engine, MarketDataManager& data, const std::vector<std::string>& tickers) {
    if (tickers.empty()) return {engine.get_balance(), {}, {}};

    // find shortest dataset across all tickers
    size_t min_len = data.get_bars(tickers[0]).size();
    for (size_t t = 1; t < tickers.size(); t++) {
        min_len = std::min(min_len, data.get_bars(tickers[t]).size());
    }

    // process bars one by one up to shortest length
    for (size_t i = 0; i < min_len; i++) {
        std::unordered_map<std::string, MarketDataRow> bar_map;
        for (const auto& ticker : tickers) {
            bar_map[ticker] = data.get_bars(ticker)[i];
        }
        engine.process_bar(bar_map);
    }

    return {engine.get_balance(), engine.get_fill_log(), engine.get_positions()};
}

SimulationResult run_generated_simulation(Engine& engine, std::vector<Generator>& generators, int num_bars) {
    for (int i = 0; i < num_bars; i++) {
        u32 date = static_cast<u32>(i);

        // generate bars for each generator
        std::unordered_map<std::string, MarketDataRow> bar_map;
        for (auto& gen : generators) {
            bar_map[gen.get_ticker()] = gen.generate_bar(date);
        }
        engine.process_bar(bar_map);
    }

    return {engine.get_balance(), engine.get_fill_log(), engine.get_positions()};
}

// copied and tweaked from old historicalData.cpp
std::string serialize_simulation_result(const SimulationResult& result) {
    std::ostringstream json;
    json << "{\n";

    json << "  \"final_balance\": " << result.final_balance << ",\n";

    json << "  \"fills\": [";
    for (size_t i = 0; i < result.fills.size(); i++) {
        const auto& f = result.fills[i];
        if (i > 0) json << ", ";
        json << "{";
        json << "\"order_id\":" << f.order_id << ",";
        json << "\"ticker\":\"" << f.ticker << "\",";
        json << "\"quantity\":" << f.quantity << ",";
        json << "\"fill_price\":" << f.fill_price << ",";
        json << "\"side\":" << static_cast<int>(f.side) << ",";
        json << "\"timestamp\":" << f.timestamp;
        json << "}";
    }
    json << "],\n";

    json << "  \"positions\": {";
    bool first = true;
    for (const auto& [ticker, pos] : result.positions) {
        if (!first) json << ", ";
        first = false;
        json << "\"" << ticker << "\": {";
        json << "\"quantity\":" << pos.quantity << ",";
        json << "\"cost_basis\":" << pos.cost_basis << ",";
        json << "\"num_lots\":" << pos.lots.size();
        json << "}";
    }
    json << "}\n";

    json << "}";
    return json.str();
}
