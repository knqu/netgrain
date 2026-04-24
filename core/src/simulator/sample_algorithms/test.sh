#!/usr/bin/env bash
set -euo pipefail

# Prints (does not execute) sample curl payloads for the simulator API.
# Copy/paste any printed line into your shell to run it.

echo "### 1) Buy once on first bar (baseline sanity check)"
echo "curl -X POST 'http://localhost:18080/api/simulate' -H 'Content-Type: application/json' -d '{\"mode\":\"generated\",\"initial_capital\":10000.0,\"num_bars\":50,\"stocks\":[{\"ticker\":\"AAA\",\"base_price\":100,\"volatility\":20,\"liquidity\":1000,\"market_cap\":5000}],\"strategy_code\":\"import netgrain\nclass Strategy:\n    def __init__(self):\n        self.did=False\n\n    def on_bar(self, market, broker):\n        if self.did: return\n        broker.place_order(\\\"AAA\\\",10,0.0,netgrain.Side.BUY,netgrain.OrderType.MARKET)\n        self.did=True\n\"}'"
echo

echo "### 2) Conditional buy (simple mean-reversion-ish trigger)"
echo "curl -X POST 'http://localhost:18080/api/simulate' -H 'Content-Type: application/json' -d '{\"mode\":\"generated\",\"initial_capital\":10000.0,\"num_bars\":50,\"stocks\":[{\"ticker\":\"AAPL\",\"base_price\":150,\"volatility\":20,\"liquidity\":1000,\"market_cap\":5000}],\"strategy_code\":\"import netgrain\n\nclass Strategy:\n    def __init__(self):\n        self.last = 0\n\n    def on_bar(self, market, broker):\n        price = market[\\\"AAPL\\\"][\\\"open\\\"]\n        if self.last != 0 and price < self.last:\n            broker.place_order(\\\"AAPL\\\", 10, 0.0, netgrain.Side.BUY, netgrain.OrderType.MARKET)\n        self.last = price\n\"}'"
echo

echo "### 3) Momentum: buy above last open, sell below (single position)"
echo "curl -X POST 'http://localhost:18080/api/simulate' -H 'Content-Type: application/json' -d '{\"mode\":\"generated\",\"initial_capital\":10000.0,\"num_bars\":75,\"stocks\":[{\"ticker\":\"AAPL\",\"base_price\":150,\"volatility\":25,\"liquidity\":1200,\"market_cap\":5000}],\"strategy_code\":\"import netgrain\n\nclass Strategy:\n    def __init__(self):\n        self.last = 0\n        self.pos = 0\n\n    def on_bar(self, market, broker):\n        p = market[\\\"AAPL\\\"][\\\"open\\\"]\n        if self.last == 0:\n            self.last = p\n            return\n\n        if p > self.last and self.pos == 0:\n            broker.place_order(\\\"AAPL\\\", 5, 0.0, netgrain.Side.BUY, netgrain.OrderType.MARKET)\n            self.pos = 1\n        elif p < self.last and self.pos == 1:\n            broker.place_order(\\\"AAPL\\\", 5, 0.0, netgrain.Side.SELL, netgrain.OrderType.MARKET)\n            self.pos = 0\n\n        self.last = p\n\"}'"
echo

echo "### 4) Buy-the-dip ladder: buy 1 each time open drops by >= 1.0 (cap buys)"
echo "curl -X POST 'http://localhost:18080/api/simulate' -H 'Content-Type: application/json' -d '{\"mode\":\"generated\",\"initial_capital\":10000.0,\"num_bars\":80,\"stocks\":[{\"ticker\":\"AAPL\",\"base_price\":150,\"volatility\":30,\"liquidity\":1500,\"market_cap\":5000}],\"strategy_code\":\"import netgrain\n\nclass Strategy:\n    def __init__(self):\n        self.last = 0\n        self.buys = 0\n\n    def on_bar(self, market, broker):\n        p = market[\\\"AAPL\\\"][\\\"open\\\"]\n        if self.last != 0 and (self.last - p) >= 1.0 and self.buys < 10:\n            broker.place_order(\\\"AAPL\\\", 1, 0.0, netgrain.Side.BUY, netgrain.OrderType.MARKET)\n            self.buys += 1\n        self.last = p\n\"}'"
echo

echo "### 5) Stop-loss: buy once, sell if open falls >= 3% from entry"
echo "curl -X POST 'http://localhost:18080/api/simulate' -H 'Content-Type: application/json' -d '{\"mode\":\"generated\",\"initial_capital\":10000.0,\"num_bars\":80,\"stocks\":[{\"ticker\":\"AAPL\",\"base_price\":150,\"volatility\":25,\"liquidity\":1200,\"market_cap\":5000}],\"strategy_code\":\"import netgrain\n\nclass Strategy:\n    def __init__(self):\n        self.entry = 0.0\n        self.sold = False\n\n    def on_bar(self, market, broker):\n        p = market[\\\"AAPL\\\"][\\\"open\\\"]\n        if self.entry == 0.0:\n            broker.place_order(\\\"AAPL\\\", 10, 0.0, netgrain.Side.BUY, netgrain.OrderType.MARKET)\n            self.entry = p\n            return\n\n        if (not self.sold) and p <= (self.entry * 0.97):\n            broker.place_order(\\\"AAPL\\\", 10, 0.0, netgrain.Side.SELL, netgrain.OrderType.MARKET)\n            self.sold = True\n\"}'"
echo

echo "### 6) Limit order: place a limit buy 2% below current open (once)"
echo "curl -X POST 'http://localhost:18080/api/simulate' -H 'Content-Type: application/json' -d '{\"mode\":\"generated\",\"initial_capital\":10000.0,\"num_bars\":80,\"stocks\":[{\"ticker\":\"AAPL\",\"base_price\":150,\"volatility\":25,\"liquidity\":1200,\"market_cap\":5000}],\"strategy_code\":\"import netgrain\n\nclass Strategy:\n    def __init__(self):\n        self.placed = False\n\n    def on_bar(self, market, broker):\n        if self.placed: return\n        p = market[\\\"AAPL\\\"][\\\"open\\\"]\n        broker.place_order(\\\"AAPL\\\", 10, p * 0.98, netgrain.Side.BUY, netgrain.OrderType.LIMIT)\n        self.placed = True\n\"}'"
echo

echo "### 7) Alternator: buy then sell every 10 bars"
echo "curl -X POST 'http://localhost:18080/api/simulate' -H 'Content-Type: application/json' -d '{\"mode\":\"generated\",\"initial_capital\":10000.0,\"num_bars\":60,\"stocks\":[{\"ticker\":\"AAPL\",\"base_price\":150,\"volatility\":15,\"liquidity\":2000,\"market_cap\":5000}],\"strategy_code\":\"import netgrain\n\nclass Strategy:\n    def __init__(self):\n        self.i = 0\n        self.pos = 0\n\n    def on_bar(self, market, broker):\n        self.i += 1\n        if self.i % 10 != 0:\n            return\n        if self.pos == 0:\n            broker.place_order(\\\"AAPL\\\", 2, 0.0, netgrain.Side.BUY, netgrain.OrderType.MARKET)\n            self.pos = 1\n        else:\n            broker.place_order(\\\"AAPL\\\", 2, 0.0, netgrain.Side.SELL, netgrain.OrderType.MARKET)\n            self.pos = 0\n\"}'"
