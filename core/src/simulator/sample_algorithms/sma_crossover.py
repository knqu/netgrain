class Strategy:
    def __init__(self):
        self.ticker = "AAPL"
        self.fast_n = 5
        self.slow_n = 20
        self.trade_qty = 10

        self.closes = []
        self.position_qty = 0
        self.prev_fast = None
        self.prev_slow = None

    # simple moving average
    def sma(self, n):
        if len(self.closes) < n:
            return None
        window = self.closes[-n:]
        return sum(window) / float(n)

    def on_bar(self, market, broker):
        import netgrain

        close = market[self.ticker]["close"]
        self.closes.append(close)

        fast = self.sma(self.fast_n)
        slow = self.sma(self.slow_n)
        if fast is None or slow is None:
            return

        crossed_up = (
            self.prev_fast is not None
            and self.prev_slow is not None
            and self.prev_fast <= self.prev_slow
            and fast > slow
        )
        crossed_down = (
            self.prev_fast is not None
            and self.prev_slow is not None
            and self.prev_fast >= self.prev_slow
            and fast < slow
        )

        if crossed_up and self.position_qty <= 0:
            broker.place_order(
                self.ticker,
                self.trade_qty,
                0.0,
                netgrain.Side.BUY,
                netgrain.OrderType.MARKET,
            )
            self.position_qty += self.trade_qty
        elif crossed_down and self.position_qty > 0:
            broker.place_order(
                self.ticker,
                self.trade_qty,
                0.0,
                netgrain.Side.SELL,
                netgrain.OrderType.MARKET,
            )
            self.position_qty -= self.trade_qty

        self.prev_fast = fast
        self.prev_slow = slow
