#pragma once

#include "strategy.hpp"

#include <memory>
#include <string>

class PythonUserStrategy : public Strategy {
public:
    explicit PythonUserStrategy(const std::string& source_code);
    ~PythonUserStrategy() override;

    PythonUserStrategy(PythonUserStrategy&&) noexcept;
    PythonUserStrategy& operator=(PythonUserStrategy&&) noexcept;

    PythonUserStrategy(const PythonUserStrategy&) = delete;
    PythonUserStrategy& operator=(const PythonUserStrategy&) = delete;

    void on_bar(const std::unordered_map<std::string, MarketDataRow>& bars, Broker& broker) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
