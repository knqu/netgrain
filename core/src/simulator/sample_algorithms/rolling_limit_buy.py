class Strategy:
    def __init__(self):
        self.ticker = "AAPL"
        self.qty = 10
        self.discount = 0.02  # 2% below open
        self.active_order_id = None

    def on_bar(self, market, broker):
        import netgrain

        open_price = market[self.ticker]["open"]
        target = open_price * (1.0 - self.discount)

        if self.active_order_id is not None:
            broker.cancel_order(int(self.active_order_id))
            self.active_order_id = None

        self.active_order_id = broker.place_order(
            self.ticker,
            self.qty,
            target,
            netgrain.Side.BUY,
            netgrain.OrderType.LIMIT,
        )
