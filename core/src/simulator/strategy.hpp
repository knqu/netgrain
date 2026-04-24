#pragma once

#include "broker.hpp"
#include "historicalData.hpp"

#include <unordered_map>

struct Strategy {
    virtual ~Strategy() = default;
    virtual void on_bar(const std::unordered_map<std::string, MarketDataRow>& bars, Broker& broker) = 0;
};
