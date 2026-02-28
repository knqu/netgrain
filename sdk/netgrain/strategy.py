from dataclasses import dataclass, field
from enum import Enum
from typing import Optional


# bar class represents historical per-day data values
@dataclass()
class Bar:
    date: int
    open: int
    high: int
    low: int
    close: int
    volume: int
    open_int: int


class Side(Enum):
    BUY = "buy"
    SELL = "sell"


# future: add more order types like stop, stop-limit, etc.
class OrderType(Enum):
    MARKET = "market"
    LIMIT = "limit"


class OrderStatus(Enum):
    PENDING = "pending"
    FILLED = "filled"
    CANCELLED = "cancelled"


@dataclass
class Order:
    id: str
    ticker: str
    side: Side
    quantity: int
    order_type: OrderType = OrderType.MARKET
    limit_price: Optional[int] = None
    status: OrderStatus = OrderStatus.PENDING


@dataclass()
class Fill:
    order_id: str
    ticker: str
    side: Side
    fill_price: int
    quantity: int
    date: int


@dataclass
class Position:
    ticker: str
    quantity: int
    avg_cost: int


@dataclass
class Portfolio:
    balance: int
    positions: dict[str, Position] = field(default_factory=dict)
