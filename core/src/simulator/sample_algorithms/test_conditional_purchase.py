import netgrain


class Strategy:
    def __init__(self):
        self.last = 0

    def on_bar(self, market, broker):
        price = market["AAPL"]["open"]
        if price < self.last:
            broker.place_order(
                "AAPL", 10, 0.0, netgrain.Side.BUY, netgrain.OrderType.MARKET
            )
        self.last = price
