class Strategy:
    def __init__(self):
        self.did = False

    def on_bar(self, market, broker):
        import netgrain

        if not self.did:
            broker.place_order(
                "AAPL", 10, 0.0, netgrain.Side.BUY, netgrain.OrderType.MARKET
            )
        self.did = True
