#pragma once

#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <variant>
#include <chrono>
#include <functional>
#include <condition_variable>

#include "order.hpp"

struct FillEvent {
    Fill fill;
};

struct OrderPlacedEvent {
    Order order;
};

struct OrderCancelledEvent {
    int order_id;
};

struct BalanceEvent {
    int balance;
    int timestamp;
};

struct PositionSnapshotEvent {
    Position position;
};

struct TickerConfigEvent {
    std::string ticker;
    TickerConfig config;
};

// struct that represents all possible events that can be pushed to the persistence queue
using PersistEvent = std::variant<
    FillEvent,
    OrderPlacedEvent,
    OrderCancelledEvent,
    BalanceEvent,
    PositionSnapshotEvent,
    TickerConfigEvent
>;

// interface for writing events to the database (implemented by database/sim_writer.hpp)
class PersistenceWriter {
public:
    virtual ~PersistenceWriter() = default;

    virtual void write_fills(const std::vector<FillEvent>& batch) = 0;
    virtual void write_orders_placed(const std::vector<OrderPlacedEvent>& batch) = 0;
    virtual void write_orders_cancelled(const std::vector<OrderCancelledEvent>& batch) = 0;
    virtual void write_balance(const BalanceEvent& snapshot) = 0;
    virtual void write_positions(const std::vector<PositionSnapshotEvent>& batch) = 0;
    virtual void write_ticker_configs(const std::vector<TickerConfigEvent>& batch) = 0;
};

// async write-back buffer for pushing events to the persistence writer
class PersistenceQueue {
public:
    PersistenceQueue(PersistenceWriter *writer, size_t batch_size = 64, std::chrono::milliseconds flush_interval = std::chrono::milliseconds(100)) {
        this->writer = writer;
        this->batch_size = batch_size;
        this->flush_interval = flush_interval;
        this->running = true;
        this->worker = std::thread(&PersistenceQueue::workerloop, this);
    }

    ~PersistenceQueue() {
        stop();
    }

    void push(PersistEvent event) {
        {  // construct lock guard to ensure atomicity of queue operations
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(std::move(event));
        }
        cv.notify_one();
    }

    void flush() {
        std::unique_lock<std::mutex> lock(mutex);
        flush_requested = true;
        lock.unlock();
        cv.notify_one();

        lock.lock();
        flushed_cv.wait(lock, [this] { return !flush_requested; });
    }

    void stop() {
        if (!running.exchange(false)) {
            return;
        }

        cv.notify_one();
        if (worker.joinable()) {
            worker.join();
        }
    }

    // returns number of unwritten events in queue
    size_t pending() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }

private:
    PersistenceWriter *writer;
    size_t batch_size;
    std::chrono::milliseconds flush_interval;

    std::queue<PersistEvent> queue;
    mutable std::mutex mutex;
    std::condition_variable cv;
    std::condition_variable flushed_cv;
    bool flush_requested = false;

    std::thread worker;
    std::atomic<bool> running;

    void workerloop() {
        std::vector<PersistEvent> batch;
        batch.reserve(batch_size);

        while (running.load()) {
            {
                std::unique_lock<std::mutex> lock(mutex);
                cv.wait_for(lock, flush_interval, [this] {
                    return queue.size() >= batch_size || flush_requested || !running.load();
                });

                size_t n = std::min(queue.size(), batch_size);
                for (size_t i = 0; i < n; i++) {
                    batch.push_back(std::move(queue.front()));
                    queue.pop();
                }

                if (flush_requested && queue.empty()) {
                    flush_requested = false;
                    flushed_cv.notify_all();
                }
            }

            if (!batch.empty()) {
                dispatch(batch);
                batch.clear();
            }
        }

        flush_remaining();
    }

    void flush_remaining() {
        std::vector<PersistEvent> batch;
        {
            std::lock_guard<std::mutex> lock(mutex);
            while (!queue.empty()) {
                batch.push_back(std::move(queue.front()));
                queue.pop();
            }
        }
        if (!batch.empty()) {
            dispatch(batch);
        }
    }

    void dispatch(const std::vector<PersistEvent>& batch) {
        std::vector<FillEvent> fills;
        std::vector<OrderPlacedEvent> placed;
        std::vector<OrderCancelledEvent> cancelled;
        std::vector<PositionSnapshotEvent> positions;
        std::vector<TickerConfigEvent> configs;
        BalanceEvent latest_balance{};
        bool has_balance = false;

        // dispatch events to the appropriate vector
        for (auto& event : batch) {
            std::visit([&](auto&& e) {
                using T = std::decay_t<decltype(e)>;

                if constexpr (std::is_same_v<T, FillEvent>) {
                    fills.push_back(e);
                } else if constexpr (std::is_same_v<T, OrderPlacedEvent>) {
                    placed.push_back(e);
                } else if constexpr (std::is_same_v<T, OrderCancelledEvent>) {
                    cancelled.push_back(e);
                } else if constexpr (std::is_same_v<T, BalanceEvent>) {
                    latest_balance = e;
                    has_balance = true;
                } else if constexpr (std::is_same_v<T, PositionSnapshotEvent>) {
                    positions.push_back(e);
                } else if constexpr (std::is_same_v<T, TickerConfigEvent>) {
                    configs.push_back(e);
                }
            }, event);
        }

        // write events to database in batches
        if (!fills.empty()) {
            writer->write_fills(fills);
        }
        if (!placed.empty()) {
            writer->write_orders_placed(placed);
        }
        if (!cancelled.empty()) {
            writer->write_orders_cancelled(cancelled);
        }
        if (has_balance) {
            writer->write_balance(latest_balance);
        }
        if (!positions.empty()) {
            writer->write_positions(positions);
        }
        if (!configs.empty()) {
            writer->write_ticker_configs(configs);
        }
    }
};
