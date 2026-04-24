#pragma once

#include <string>
#include <iostream>
#include <pqxx/pqxx>

#include "../core/src/simulator/persistence_queue.hpp"

inline const char *side_to_str(Side s) {
    return s == Side::BUY ? "BUY" : "SELL";
}

inline const char *order_type_to_str(OrderType t) {
    switch (t) {
        case OrderType::MARKET: return "MARKET";
        case OrderType::LIMIT: return "LIMIT";
        case OrderType::STOP: return "STOP";
    }
}

inline const char *order_status_to_str(OrderStatus s) {
    switch (s) {
        case OrderStatus::PENDING: return "PENDING";
        case OrderStatus::FILLED: return "FILLED";
        case OrderStatus::PARTIAL: return "PARTIAL";
        case OrderStatus::CANCELLED: return "CANCELLED";
    }
}

class SimWriter : public PersistenceWriter {
    pqxx::connection conn;  // owns own pqxx::connection so the background writer thread never contends with the webserver's connection
    int sim_id;  // scoped to single simulation run

public:
    SimWriter(const std::string& conn_string, int sim_id) : conn(conn_string), sim_id(sim_id) {}

    void write_fills(const std::vector<FillEvent>& batch) override {
        try {
            pqxx::work tx(conn);
            for (auto& e : batch) {
                auto& f = e.fill;
                tx.exec(
                    "INSERT INTO sim_fills "
                    "(sim_id, order_id, ticker, quantity, fill_price, side, bar_timestamp) "
                    "VALUES ($1, $2, $3, $4, $5, $6, $7)",
                    pqxx::params{
                        sim_id, f.order_id, f.ticker, f.quantity, f.fill_price, side_to_str(f.side), f.timestamp
                    }
                );
            }
            tx.commit();
        } catch (const std::exception& e) {
            std::cerr << "write_fills: " << e.what() << "\n";
        }
    }

    void write_orders_placed(const std::vector<OrderPlacedEvent>& batch) override {
        try {
            pqxx::work tx(conn);
            for (auto& e : batch) {
                auto& o = e.order;
                tx.exec(
                    "INSERT INTO sim_orders "
                    "(sim_id, order_id, ticker, quantity, target_price, side, order_type, status) "
                    "VALUES ($1, $2, $3, $4, $5, $6, $7, $8)",
                    pqxx::params{
                        sim_id, o.id, o.ticker, o.quantity, o.target_price,
                        side_to_str(o.side),
                        order_type_to_str(o.order_type),
                        order_status_to_str(o.status)
                    }
                );
            }
            tx.commit();
        } catch (const std::exception& e) {
            std::cerr << "write_orders_placed: " << e.what() << "\n";
        }
    }

    void write_orders_cancelled(const std::vector<OrderCancelledEvent>& batch) override {
        try {
            pqxx::work tx(conn);
            for (auto& e : batch) {
                tx.exec(
                    "UPDATE sim_orders SET status = 'CANCELLED' "
                    "WHERE sim_id = $1 AND order_id = $2",
                    pqxx::params{sim_id, e.order_id}
                );
            }
            tx.commit();
        } catch (const std::exception& e) {
            std::cerr << "write_orders_cancelled: " << e.what() << "\n";
        }
    }

    void write_balance(const BalanceEvent& snapshot) override {
        try {
            pqxx::work tx(conn);
            tx.exec(
                "INSERT INTO sim_balance_log (sim_id, balance, bar_timestamp) "
                "VALUES ($1, $2, $3)",
                pqxx::params{sim_id, snapshot.balance, snapshot.timestamp}
            );
            tx.commit();
        } catch (const std::exception& e) {
            std::cerr << "write_balance: " << e.what() << "\n";
        }
    }

    void write_positions(const std::vector<PositionSnapshotEvent>& batch) override {
        try {
            pqxx::work tx(conn);
            for (auto& e : batch) {
                auto& p = e.position;
                tx.exec(
                    "INSERT INTO sim_positions "
                    "(sim_id, ticker, quantity, cost_basis, market_price, timestamp) "
                    "VALUES ($1, $2, $3, $4, $5, $6)",
                    pqxx::params{sim_id, p.ticker, p.quantity, p.cost_basis, e.market_price, e.timestamp}
                );
            }
            tx.commit();
        } catch (const std::exception& e) {
            std::cerr << "write_positions: " << e.what() << "\n";
        }
    }
};
