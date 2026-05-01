class Strategy:
    def __init__(self):
        self.ticker = "AAPL"
        self.did_buy = False

    def on_bar(self, market, broker):
        import netgrain

        if self.did_buy:
            return

        price = market[self.ticker]["open"]
        balance = broker.get_balance()
        qty = int(balance // price)

        broker.place_order(
            self.ticker, qty, 0.0, netgrain.Side.BUY, netgrain.OrderType.MARKET
        )

        self.did_buy = True
