#include "engine.cpp"
#include <iostream>

int main() {
    Engine engine(10000000);

    int order1 = engine.process_order("AAPL", Side::BUY, 10, OrderType::MARKET);
    int order2 = engine.process_order("AAPL", Side::BUY, 5, OrderType::LIMIT, 14800000);
    int order3 = engine.process_order("AAPL", Side::BUY, 5, OrderType::LIMIT, 14000000);  // should not fill

    // open=$150, high=$155, low=$148, close=$152
    MarketDataRow bar = {20260305, 15000000, 15500000, 14800000, 15200000, 100000, 0};
    auto fills = engine.process_bar("AAPL", bar);

    std::cout << "Fills: " << fills.size() << "\n";  // expect 2

    for (auto& f : fills) {
        std::cout << "Order " << f.order_id << ": " << f.quantity << " shares @ $" << f.fill_price << "\n";
    }

    return 0;
}
