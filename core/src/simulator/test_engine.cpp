#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>

#include "engine.hpp"

MarketDataRow make_bar(u32 date, i64 open, i64 high, i64 low, i64 close, u64 volume) {
    return {date, open, high, low, close, volume, 0};
}

void test_market_order_buy() {
    Engine engine(100000);
    std::unordered_map<std::string, MarketDataRow> bars;

    engine.place_order("AAPL", 10, 0, Side::BUY, OrderType::MARKET);
    bars["AAPL"] = make_bar(20260101, 150, 155, 148, 152, 1000);
    auto fills = engine.process_bar(bars);

    assert(fills.size() == 1);
    assert(fills[0].fill_price == 150);
    assert(fills[0].quantity == 10);
    assert(fills[0].side == Side::BUY);
    assert(engine.get_balance() < 100000);

    std::cout << "PASSED: market order buy\n";
}

void test_market_order_sell() {
    Engine engine(100000);
    std::unordered_map<std::string, MarketDataRow> bars;

    engine.place_order("AAPL", 10, 0, Side::BUY, OrderType::MARKET);
    bars["AAPL"] = make_bar(20260101, 150, 155, 148, 152, 1000);
    engine.process_bar(bars);

    engine.place_order("AAPL", 10, 0, Side::SELL, OrderType::MARKET);
    bars["AAPL"] = make_bar(20260102, 160, 165, 158, 162, 1000);
    auto fills = engine.process_bar(bars);

    assert(fills.size() == 1);
    assert(fills[0].fill_price == 160);
    assert(fills[0].side == Side::SELL);

    std::cout << "PASSED: market order sell\n";
}

void test_limit_buy_fills_when_price_reached() {
    Engine engine(100000);
    std::unordered_map<std::string, MarketDataRow> bars;

    engine.place_order("AAPL", 5, 145, Side::BUY, OrderType::LIMIT);
    bars["AAPL"] = make_bar(20260101, 150, 155, 140, 152, 1000);
    auto fills = engine.process_bar(bars);

    assert(fills.size() == 1);
    assert(fills[0].fill_price == 145);

    std::cout << "PASSED: limit buy fills when low <= target\n";
}

void test_limit_buy_no_fill_when_price_not_reached() {
    Engine engine(100000);
    std::unordered_map<std::string, MarketDataRow> bars;

    engine.place_order("AAPL", 5, 145, Side::BUY, OrderType::LIMIT);
    bars["AAPL"] = make_bar(20260101, 150, 155, 148, 152, 1000);
    auto fills = engine.process_bar(bars);

    assert(fills.size() == 0);
    assert(engine.get_balance() == 100000);

    std::cout << "PASSED: limit buy no fill when low > target\n";
}

void test_stop_sell_fills_when_price_drops() {
    Engine engine(100000);
    std::unordered_map<std::string, MarketDataRow> bars;

    engine.place_order("AAPL", 10, 0, Side::BUY, OrderType::MARKET);
    bars["AAPL"] = make_bar(20260101, 150, 155, 148, 152, 1000);
    engine.process_bar(bars);

    engine.place_order("AAPL", 10, 145, Side::SELL, OrderType::STOP);
    bars["AAPL"] = make_bar(20260102, 150, 152, 140, 142, 1000);
    auto fills = engine.process_bar(bars);

    assert(fills.size() == 1);
    assert(fills[0].fill_price == 145);

    std::cout << "PASSED: stop sell fills when low <= target\n";
}

void test_cancel_order() {
    Engine engine(100000);
    std::unordered_map<std::string, MarketDataRow> bars;

    int id = engine.place_order("AAPL", 10, 0, Side::BUY, OrderType::MARKET);

    assert(engine.cancel_order(id) == true);
    assert(engine.cancel_order(999) == false);

    bars["AAPL"] = make_bar(20260101, 150, 155, 148, 152, 1000);
    auto fills = engine.process_bar(bars);

    std::cout << "PASSED: cancel order\n";
}

void test_partial_fill_due_to_volume() {
    Engine engine(100000);
    std::unordered_map<std::string, MarketDataRow> bars;

    engine.place_order("AAPL", 100, 0, Side::BUY, OrderType::MARKET);
    bars["AAPL"] = make_bar(20260101, 150, 155, 148, 152, 30);
    auto fills = engine.process_bar(bars);

    assert(fills.size() == 1);
    assert(fills[0].quantity == 30);  // only 30/100 shares should fill

    bars["AAPL"] = make_bar(20260102, 151, 156, 149, 153, 1000);
    fills = engine.process_bar(bars);

    assert(fills.size() == 1);
    assert(fills[0].quantity == 70);  // remaining 70/100 shares should fill using new liquidity

    std::cout << "PASSED: partial fill due to volume\n";
}

void test_multi_ticker() {
    Engine engine(100000);
    std::unordered_map<std::string, MarketDataRow> bars;

    engine.place_order("AAPL", 5, 0, Side::BUY, OrderType::MARKET);
    engine.place_order("GOOG", 3, 0, Side::BUY, OrderType::MARKET);
    bars["AAPL"] = make_bar(20260101, 150, 155, 148, 152, 1000);
    bars["GOOG"] = make_bar(20260101, 2800, 2850, 2780, 2820, 1000);
    auto fills = engine.process_bar(bars);

    assert(fills.size() == 2);
    
    std::cout << "PASSED: multi-ticker processing\n";
}

void test_fifo_lot_disposal() {
    Engine engine(100000);
    std::unordered_map<std::string, MarketDataRow> bars;

    // buy 2 lots of 10 shares
    engine.place_order("AAPL", 10, 0, Side::BUY, OrderType::MARKET);
    engine.place_order("AAPL", 10, 0, Side::BUY, OrderType::MARKET);
    bars["AAPL"] = make_bar(20260101, 150, 155, 148, 152, 1000);
    engine.process_bar(bars);

    int balance_before_sell = engine.get_balance();

    // selling 15 shares should consume 10/10 shares from lot 1 and 5/10 shares from lot 2
    engine.place_order("AAPL", 15, 0, Side::SELL, OrderType::MARKET);
    bars["AAPL"] = make_bar(20260103, 250, 260, 245, 255, 1000);
    auto fills = engine.process_bar(bars);

    assert(fills.size() == 1);
    assert(fills[0].quantity == 15);

    std::cout << "PASSED: FIFO lot disposal\n";
}

void test_fee_deduction() {
    Engine engine(100000);
    std::unordered_map<std::string, MarketDataRow> bars;

    engine.set_config("AAPL", {0.0, 0.01});  // 1% fee
    engine.place_order("AAPL", 10, 0, Side::BUY, OrderType::MARKET);
    bars["AAPL"] = make_bar(20260101, 100, 110, 95, 105, 1000);
    engine.process_bar(bars);

    assert(engine.get_balance() == 98990);  // 100000 - 10 (fee) - 1000 (trade) = 98990

    std::cout << "PASSED: fee deduction\n";
}

int main() {
    std::cout << "Running engine tests...\n";

    test_market_order_buy();
    test_market_order_sell();
    test_limit_buy_fills_when_price_reached();
    test_limit_buy_no_fill_when_price_not_reached();
    test_stop_sell_fills_when_price_drops();
    test_cancel_order();
    test_partial_fill_due_to_volume();
    test_multi_ticker();
    test_fifo_lot_disposal();
    test_fee_deduction();

    std::cout << "All engine tests passed\n";
}
