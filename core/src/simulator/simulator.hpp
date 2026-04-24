#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "engine.hpp"
#include "strategy.hpp"
#include "../generator/generator.hpp"

struct SimulationResult {
    i64 final_balance;
    std::vector<Fill> fills;
    std::unordered_map<std::string, Position> positions;
};

SimulationResult run_csv_simulation(
    Engine& engine, Strategy& strategy, MarketDataManager& data, const std::vector<std::string>& tickers
);

SimulationResult run_generated_simulation(
    Engine& engine, Strategy& strategy, std::vector<std::unique_ptr<Generator>>& generators, int num_bars
);

std::string serialize_simulation_result(const SimulationResult& result);
