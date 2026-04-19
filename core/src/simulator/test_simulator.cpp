#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>

#include "engine.hpp"
#include "../generator/generator.hpp"

void test_generated_bar_ohlcv_invariants() {
    Generator gen("AAPL", 150, 10, 1000, 1000000);

    for (int i = 0; i < 100; i++) {
        auto bar = gen.generate_bar(20260101 + i);
        assert(bar.high >= bar.open);
        assert(bar.high >= bar.close);
        assert(bar.low <= bar.open);
        assert(bar.low <= bar.close);
        assert(bar.high >= bar.low);
        assert(bar.volume > 0);
    }

    std::cout << "PASSED: generated bars satisfy OHLCV invariants\n";
}

void test_generated_bar_continuity() {
    Generator gen("AAPL", 150, 10, 1000, 1000000);

    auto bar1 = gen.generate_bar(20260101);
    for (int i = 1; i < 50; i++) {
        auto bar2 = gen.generate_bar(20260101 + i);
        assert(bar2.open == bar1.close);
        bar1 = bar2;
    }

    std::cout << "PASSED: close(N) == open(N+1) across bars\n";
}

void test_market_buy_with_generated_bar() {
    Generator gen("AAPL", 150, 10, 1000, 1000000);
    Engine engine(100000);

    engine.place_order("AAPL", 10, 0, Side::BUY, OrderType::MARKET);

    auto bar = gen.generate_bar(20260101);
    std::unordered_map<std::string, MarketDataRow> bars;
    bars["AAPL"] = bar;
    auto fills = engine.process_bar(bars);

    assert(fills.size() == 1);
    assert(fills[0].fill_price == bar.open);
    assert(fills[0].quantity == 10);
    assert(fills[0].side == Side::BUY);
    assert(engine.get_balance() < 100000);

    std::cout << "PASSED: market buy fills at generated bar open\n";
}

void test_market_sell_with_generated_bars() {
    Generator gen("AAPL", 150, 10, 1000, 1000000);
    Engine engine(100000);
    std::unordered_map<std::string, MarketDataRow> bars;

    engine.place_order("AAPL", 10, 0, Side::BUY, OrderType::MARKET);
    bars["AAPL"] = gen.generate_bar(20260101);
    engine.process_bar(bars);

    i64 balance_after_buy = engine.get_balance();

    engine.place_order("AAPL", 10, 0, Side::SELL, OrderType::MARKET);
    bars["AAPL"] = gen.generate_bar(20260102);
    auto fills = engine.process_bar(bars);

    assert(fills.size() == 1);
    assert(fills[0].side == Side::SELL);
    assert(engine.get_balance() != balance_after_buy);
    assert(engine.get_positions().at("AAPL").quantity == 0);

    std::cout << "PASSED: market sell clears position with generated bars\n";
}

void test_multi_ticker_generated() {
    Generator aapl("AAPL", 150, 10, 1000, 1000000);
    Generator goog("GOOG", 2800, 15, 500, 2000000);
    Engine engine(1000000);
    std::unordered_map<std::string, MarketDataRow> bars;

    engine.place_order("AAPL", 5, 0, Side::BUY, OrderType::MARKET);
    engine.place_order("GOOG", 3, 0, Side::BUY, OrderType::MARKET);

    bars["AAPL"] = aapl.generate_bar(20260101);
    bars["GOOG"] = goog.generate_bar(20260101);
    auto fills = engine.process_bar(bars);

    assert(fills.size() == 2);

    auto& positions = engine.get_positions();
    assert(positions.at("AAPL").quantity == 5);
    assert(positions.at("GOOG").quantity == 3);

    std::cout << "PASSED: multi-ticker with generated bars\n";
}

void test_limit_buy_fills_within_generated_range() {
    Generator gen("AAPL", 150, 10, 1000, 1000000);
    Engine engine(100000);
    std::unordered_map<std::string, MarketDataRow> bars;

    auto bar = gen.generate_bar(20260101);
    i64 limit_price = bar.low;

    engine.place_order("AAPL", 5, limit_price, Side::BUY, OrderType::LIMIT);
    bars["AAPL"] = bar;
    auto fills = engine.process_bar(bars);

    assert(fills.size() == 1);
    assert(fills[0].fill_price == limit_price);

    std::cout << "PASSED: limit buy at bar.low always fills\n";
}

void test_limit_buy_misses_outside_generated_range() {
    Generator gen("AAPL", 150, 10, 1000, 1000000);
    Engine engine(100000);
    std::unordered_map<std::string, MarketDataRow> bars;

    auto bar = gen.generate_bar(20260101);
    i64 limit_price = bar.low - 1000;

    engine.place_order("AAPL", 5, limit_price, Side::BUY, OrderType::LIMIT);
    bars["AAPL"] = bar;
    auto fills = engine.process_bar(bars);

    assert(fills.size() == 0);
    assert(engine.get_balance() == 100000);

    std::cout << "PASSED: limit buy below bar range does not fill\n";
}

void test_stop_sell_triggers_within_generated_range() {
    Generator gen("AAPL", 150, 10, 1000, 1000000);
    Engine engine(100000);
    std::unordered_map<std::string, MarketDataRow> bars;

    engine.place_order("AAPL", 10, 0, Side::BUY, OrderType::MARKET);
    bars["AAPL"] = gen.generate_bar(20260101);
    engine.process_bar(bars);

    auto bar2 = gen.generate_bar(20260102);
    i64 stop_price = bar2.low;

    engine.place_order("AAPL", 10, stop_price, Side::SELL, OrderType::STOP);
    bars["AAPL"] = bar2;
    auto fills = engine.process_bar(bars);

    assert(fills.size() == 1);
    assert(fills[0].fill_price == stop_price);

    std::cout << "PASSED: stop sell at bar.low triggers with generated bars\n";
}

void test_balance_conservation_round_trip() {
    Generator gen("AAPL", 150, 10, 1000, 1000000);
    Engine engine(100000);
    engine.set_config("AAPL", {0.0, 0.0, 0.0, {0.0, 0.0}});
    std::unordered_map<std::string, MarketDataRow> bars;

    engine.place_order("AAPL", 10, 0, Side::BUY, OrderType::MARKET);
    auto bar1 = gen.generate_bar(20260101);
    bars["AAPL"] = bar1;
    engine.process_bar(bars);

    i64 cost = bar1.open * 10;
    assert(engine.get_balance() == 100000 - cost);

    engine.place_order("AAPL", 10, 0, Side::SELL, OrderType::MARKET);
    auto bar2 = gen.generate_bar(20260102);
    bars["AAPL"] = bar2;
    engine.process_bar(bars);

    i64 revenue = bar2.open * 10;
    assert(engine.get_balance() == 100000 - cost + revenue);

    std::cout << "PASSED: balance is exactly correct with zero fees\n";
}

void test_multi_bar_position_accumulation() {
    Generator gen("AAPL", 150, 10, 1000, 1000000);
    Engine engine(1000000);
    std::unordered_map<std::string, MarketDataRow> bars;

    for (int i = 0; i < 5; i++) {
        engine.place_order("AAPL", 10, 0, Side::BUY, OrderType::MARKET);
        bars["AAPL"] = gen.generate_bar(20260101 + i);
        engine.process_bar(bars);
    }

    assert(engine.get_positions().at("AAPL").quantity == 50);
    assert(engine.get_fill_log().size() == 5);

    std::cout << "PASSED: position accumulates over multiple generated bars\n";
}

int main() {
    std::cout << "Running simulator tests...\n";

    test_generated_bar_ohlcv_invariants();
    test_generated_bar_continuity();
    test_market_buy_with_generated_bar();
    test_market_sell_with_generated_bars();
    test_multi_ticker_generated();
    test_limit_buy_fills_within_generated_range();
    test_limit_buy_misses_outside_generated_range();
    test_stop_sell_triggers_within_generated_range();
    test_balance_conservation_round_trip();
    test_multi_bar_position_accumulation();

    std::cout << "All simulator tests passed\n";
}
