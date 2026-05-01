#include "simulator.hpp"

#include "broker.hpp"

#include <sstream>
#include <algorithm>
#include <chrono>

std::string month_to_string(int month) {
    const char* months[] = {
        "January", "February", "March", "April",
        "May", "June", "July", "August",
        "September", "October", "November", "December"
    };

    return months[month - 1];
}

std::vector<i64> calculate_equity_and_write_PL(unsigned simID) {
    std::ostringstream oss;
    oss << "../src/sims/" << simID << "/";
    std::string path = oss.str();;

    std::vector<i64> retVec;
    std::unordered_map<std::string, i64> posMap; // ticker -> quantity held
    std::unordered_map<std::string, std::deque<std::pair<i64, i64>>> entryMap; // ticker -> ([price, quantity])
    std::unordered_map<std::string, i64> marketMap; // ticker -> last market_price
    std::unordered_map<i64, int> PnLMap;

    fstream algorithmActions(path + "algorithmActions", std::ios::in);
    fstream marketData(path + "marketData", std::ios::in);

    std::string line;
    std::string market_line;
    i64 balance = 0;

    while (std::getline(algorithmActions, line)) {
        if (line == "End Bar") {
            i64 equity = balance;
            while (std::getline(marketData, market_line)) {
                if (market_line == "End Bar") {
                    break;
                }
                std::string tick_name;
                i64 market_price;

                std::istringstream miss(market_line);
                miss >> tick_name;
                miss >> market_price;

                marketMap[tick_name] = market_price;

                if (posMap[tick_name] != 0) {
                    equity += posMap[tick_name] * market_price;
                }
            }
            retVec.push_back(equity);
            continue;
        }

        std::string ticker;
        std::string side;
        i64 quantity;
        i64 fill_price;
        
        std::istringstream aiss(line);
        aiss >> ticker;
        aiss >> side;
        aiss >> quantity;
        aiss >> fill_price;
        aiss >> balance;

        if (side == "BUY"){
            posMap[ticker] += quantity;
            entryMap[ticker].push_back({fill_price, quantity});
        }
        else {
            posMap[ticker] -= quantity;

            i64 PL = 0;
            while (quantity > 0) {
                std::pair<i64, i64> temp = entryMap[ticker].front();
                entryMap[ticker].pop_front();

                if (quantity - temp.second >= 0) {
                    PL += (fill_price - temp.first) * temp.second;
                    quantity -= temp.second;
                }
                else {
                    PL += (fill_price - temp.first) * temp.second;
                    temp.second -= quantity;
                    quantity = 0;
                    entryMap[ticker].push_front(temp);
                }
            }
            PnLMap[PL]++;
        }
    }
    algorithmActions.close();
    marketData.close();

    for (auto& [ticker, deque] : entryMap) {
        i64 market_price = marketMap[ticker];

        while (deque.empty() == false) {
            i64 PL = 0;

            std::pair<i64, i64> temp = deque.front();
            deque.pop_front();

            PL += (market_price - temp.first) * temp.second;
            PnLMap[PL]++;
        }
    }

    fstream simResults(path + "simResults", std::ios::out | std::ios::app);
    simResults << "[";
    for (const auto [PnL, quantity] : PnLMap) {
         simResults << "{ \"time\" : " << PnL << ", \"value\" : " << quantity << "\"color\" : " << (PnL >= 0 ? "\"green\"" : "\"red\"") << " },";
    }
    simResults << "]\n";
    simResults.close();

    return retVec;
}

std::string equity_to_str(std::vector<i64> equityVec) {
    std::ostringstream retStr;
    retStr << "[";
    for (size_t i = 0; i < equityVec.size(); i++) {
        retStr << "{ \"time\" : " << i << ", \"value\" : " << equityVec[i] << " },";
    }
    retStr << "]";
    return retStr.str();
}

void write_metrics(int simID, double duration) {
    std::ostringstream oss;
    oss << "../src/sims/" << simID << "/";
    std::string path = oss.str();;

    fstream simResults(path + "simResults", std::ios::out | std::ios::app);
    simResults << "Simulation " << simID << "\n";

    auto now = std::chrono::system_clock::now();
    auto today = std::chrono::floor<std::chrono::days>(now);
    std::chrono::year_month_day ymd{today};

    simResults << unsigned(ymd.day()) << " " << month_to_string(unsigned(ymd.month())) << " " << int(ymd.year()) << "\n";
    simResults << duration << "\n";

    simResults << "PL Start\n";
    std::vector<i64> equity_vec = calculate_equity_and_write_PL(simID);
    simResults << "PL End\n";

    std::string equity_gain = "";
    std::string equity_str = equity_to_str(equity_vec);
    if (equity_vec.size() == 0) {
        equity_gain = "0";
    }
    else if (equity_vec.size() == 1) {
        equity_gain = std::to_string(equity_vec[0]);
    }
    else {
        equity_gain = std::to_string(equity_vec[equity_vec.size() - 1] - equity_vec[0]);
    }

    simResults << equity_gain << "\n";
    simResults << "\n";
    simResults << "fees and stuff\n";

    simResults << "Equity Start\n";
    simResults << equity_str << "\n";
    simResults << "Equity End\n";
    
    std::ostringstream dds;
    i64 peak = -1;
    dds << "[";
    for (size_t i = 0; i < equity_vec.size(); i++) {
        double drawdown;
        if (equity_vec[i] > peak) {
            drawdown = 0.0;
            peak =  equity_vec[i];
        }
        else {
            drawdown = ((double) (equity_vec[i] - peak)) / ((double) peak);
        }

        dds << "{ \"time\" : " << i << ", \"value\" : " << drawdown << " },"; 
    }
    dds << "]";

    simResults << "Drawdown Start\n";
    simResults << dds.str() << "\n";
    simResults << "Drawdown End\n";

    simResults.close();
}

SimulationResult run_csv_simulation(Engine& engine, Strategy& strategy, MarketDataManager& data,
                                    const std::vector<std::string>& tickers) {
    if (tickers.empty()) return {engine.get_balance(), {}, {}};

    // find shortest dataset across all tickers
    size_t min_len = data.get_bars(tickers[0]).size();
    for (size_t t = 1; t < tickers.size(); t++) {
        min_len = std::min(min_len, data.get_bars(tickers[t]).size());
    }

    Broker broker(&engine);

    // process bars one by one up to shortest length
    for (size_t i = 0; i < min_len; i++) {
        std::unordered_map<std::string, MarketDataRow> bar_map;
        for (const auto& ticker : tickers) {
            bar_map[ticker] = data.get_bars(ticker)[i];
        }
        strategy.on_bar(bar_map, broker);
        engine.process_bar(bar_map);
    }

    return {engine.get_balance(), engine.get_fill_log(), engine.get_positions()};
}

SimulationResult run_generated_simulation(Engine& engine, Strategy& strategy,
                                          std::vector<std::unique_ptr<Generator>>& generators, int num_bars) {
    Broker broker(&engine);

    auto start = std::chrono::high_resolution_clock::now();
    engine.running = true;
    int i = 0;
    while (engine.running) {
        u32 date = static_cast<u32>(i);

        // generate bars for each generator
        std::unordered_map<std::string, MarketDataRow> bar_map;
        for (auto& gen : generators) {
            bar_map[gen->get_ticker()] = gen->generate_bar(date);
        }
        strategy.on_bar(bar_map, broker);
        engine.process_bar(bar_map);
        i++;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    for (auto& gen : generators) {
        gen->gen_settings.send_data.store(false);
        gen->gen_settings.conn.load()->close();
        gen->gen_settings.conn.store(nullptr);
    }
    engine.conn->close();
    engine.conn = nullptr;
    
    std::cerr << "Writing metrics now" << std::endl;
    write_metrics(engine.simID, duration.count());

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