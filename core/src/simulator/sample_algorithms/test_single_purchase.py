import netgrain


class Strategy:
    def __init__(self):
        self.did = False

    def on_bar(self, market, broker):
        if not self.did:
            broker.place_order(
                "AAPL", 10, 0.0, netgrain.Side.BUY, netgrain.OrderType.MARKET
            )
        self.did = True
