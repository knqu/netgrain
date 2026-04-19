// Things to bring up, with this class, do we want this to be multithreaded,
// if it is, is locking going to be needed, and should we make out own queue class

// this is based on drifts, but with our events, we might need to be able to model jumps as well

#pragma once

#include <crow.h>

#include <fmt/format.h>
#include <fmt/core.h>

#include "def.hpp"
#include "queue.hpp"
#include "blocking_queue.hpp"
#include "data_transfer.hpp"
#include "../simulator/historicalData.hpp"
#include <cmath>
#include <thread>
#include <iostream>
#include <ostream>
#include <random>
#include <unordered_set>
#include <map>
#include <string>
#include <stdlib.h>
#include <queue>
#include <thread>
#include <chrono>

class Generator {
public:
  double percent_drift;
  double percent_volatility;
  double dt;

  // queue for streaming data
  // Queue<double> *data_buffer;
  std::queue<double> *data_buffer;
  // create a random device for normal distribution

  // std::random_device *device;
  // std::mt19937 *gen;
  // std::normal_distribution<> *dist;

  std::string ticker;
  int base_price;
  int volatility;
  int liquidity;
  int market_cap;
  int target_price;

  double last_bar_close = 0.0;
  bool has_bar_state = false;
  bool last_was_clamped = false; // Tracks if the very last tick was an outlier correction

  // constructor and destructor
  Generator(double drift, double volatility, int price, int target) {
    this->percent_drift = drift;
    this->percent_volatility = volatility;
    this->base_price = price;
    this->target_price = target;
    this->dt = 0.01;
    this->data_buffer = new std::queue<double>;
    this->data_buffer->push(price);
  }

  Generator(
    std::string ticker,
    int base_price,
    int volatility,
    int liquidity,
    int market_cap) {

    this->ticker = ticker;
    this->base_price = base_price;
    this->volatility = volatility;
    this->liquidity = liquidity;
    this->market_cap = market_cap;
    this->percent_drift = 0.0;
    this->percent_volatility = volatility / 100.0;
    this->dt = 0.01;
    this->target_price = base_price;
    this->data_buffer = new std::queue<double>;
    this->data_buffer->push(static_cast<double>(base_price));
    // NOTE: even though the simulator passes in an int scaled by 100, the gbm math looks to be scale invariant
  }

  ~Generator() {
  }

  std::string get_ticker() const {
    return ticker;
  }

  double get_dt() {
    return dt;
  }

  double get_percent_drift() {
    return percent_drift;
  }

  double get_percent_volatility() {
    return percent_volatility;
  }

  void get_event(double n_drift, double n_vol, int n_price) {
    percent_drift = n_drift;
    percent_volatility = n_vol;
    data_buffer->push(n_price);
  }

  void reset() {
    (*data_buffer) = {};
    data_buffer->push(static_cast<double>(base_price));
  }

  void overwrite(double x) {
    (*data_buffer) = {};
    data_buffer->push(x);
  }

  double send_price() {
    if (data_buffer->size() == 0) return base_price;
    double data = data_buffer->front();
    data_buffer->pop();
    return data;
  }

  double clamp_price(double old_price, double generated_price) {
    double max_threshold = 0.05; 
    
    double max_allowed = old_price * (1.0 + max_threshold);
    double min_allowed = old_price * (1.0 - max_threshold);
    
    if (generated_price > max_allowed) {
        last_was_clamped = true;
        return max_allowed;
    } else if (generated_price < min_allowed) {
        last_was_clamped = true;
        return min_allowed;
    }
    last_was_clamped = false;
    return generated_price;
  }


  /*
   * Basic data generation function,
   * takes, current price
   */
  void generate_new_data_point() {
    u32 new_data = data_buffer->front();
    short multiplier = 1;
    if (rand() % 10 == 0) {
      multiplier = -1;
    }
    new_data = new_data + multiplier * (rand() % (u32) (percent_drift * 100));
    data_buffer->push(new_data);
  }

  /*
   * This function returns the brownian motion at increment t, given the
   * Weiner process at time t
   */
  double gbm(double S_0, std::normal_distribution<double> &d, std::mt19937 &gen) {
    double ret =
      (
        this->percent_drift
        - (this->percent_volatility * this->percent_volatility / 2)
      ) * dt + this->percent_volatility * sqrt(dt) * d(gen);
    ret = S_0 * exp(ret);
    return clamp_price(S_0, ret);
  }

  // generates one ohlcv bar by simulating ticks_per_bar intra-bar price movements via gbm, then aggregating
  MarketDataRow generate_bar(u32 date, int ticks_per_bar = 50) {
    // setup is basically copied from generate method below
    std::random_device rd{};
    std::mt19937 rng{rd()};
    std::normal_distribution<double> norm{0.0, 1.0};

    // state is carried between calls so close(N) == open(N+1)
    double cur = has_bar_state ? last_bar_close : static_cast<double>(base_price);
    double open = cur;
    double high = cur;
    double low = cur;

    for (int i = 0; i < ticks_per_bar; i++) {
      cur = gbm(cur, norm, rng);
      if (cur > high) high = cur;
      if (cur < low) low = cur;
    }

    last_bar_close = cur;
    has_bar_state = true;

    // simulate volume from liquidity parameter - made this up, values are arbitrary (between 80-120%)
    u64 volume = (liquidity > 0)
      ? static_cast<u64>(liquidity * (80 + (rand() % 41)) / 100)
      : static_cast<u64>(500 + rand() % 1000);

    return MarketDataRow{
      date,
      static_cast<i64>(open),
      static_cast<i64>(high),
      static_cast<i64>(low),
      static_cast<i64>(cur),
      volume,
      0
    };
  }

  // return the number of datapoints generated, if data is not being tested
  int generate(Data_Transfer *gen_settings, std::ostream &fout) {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> norm{0.0, 1.0};

    // Values near the mean are the most likely. Standard deviation
    // affects the dispersion of generated values from the mean.
    int i = 0;
    while (gen_settings->gen.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      // generate the next data point in the weiner process and add it onto
      // the data buffer, before dequeuing it
      data_buffer->push(gbm(data_buffer->front(), norm, gen));

      // TODO: way to send data would go here, this could be in the format
      // of a second queue, extra fields in the Data_Transfer, etc. for now
      // printing to console will suffice
      if (gen_settings->send_data.load()) {
        fout << send_price() << "\n";
      }

      // TODO: process events, rn all I do is call the get_event
      // function but this logic might need to be adjusted
      if (gen_settings->new_event.load()) {
        get_event(
          gen_settings->n_drift,
          gen_settings->n_vol,
          gen_settings->n_price
        );
      }

      i += 1;
      std::this_thread::sleep_for(std::chrono::milliseconds(9));
    }
    return i;
  }

  /*
   * This function implements Ornstein–Uhlenbeck process for a sideways trading market.
   */
  double ou(double x_t, std::normal_distribution<double> &d, std::mt19937 &gen) {
    double ret = x_t + ((this->percent_drift * (this->target_price - x_t)) * dt) + (this->percent_volatility * sqrt(dt) * d(gen));
    return clamp_price(x_t, ret);
  }



  /*
   * bear maket logic -> based on gbm, customized to have a higher probability of large negative moves, and a lower probability of large positive moves
   * Uses class drift/volatility, but occasionally triggers "panic selling"
   */
  double bear_math(double x_t, std::normal_distribution<double> &d, std::mt19937 &gen) {
    double expected_return = (this->percent_drift - (this->percent_volatility * this->percent_volatility / 2.0)) * this->dt;
    double random_shock = this->percent_volatility * sqrt(this->dt) * d(gen);
    double total_move = expected_return + random_shock;

    if (rand() % 100 < 3) {
        total_move -= 0.05;
    }

    return clamp_price(x_t, x_t * exp(total_move));
  }

  /*
   * Bull Market logic based on gbm, customized to have a higher probability of large positive moves, and a lower probability of large negative moves
   * Uses class drift/volatility, but incorporates "buy the dip" and FOMO behavior
   */
  double bull_math(double x_t, std::normal_distribution<double> &d, std::mt19937 &gen) {
    double expected_return = (this->percent_drift - (this->percent_volatility * this->percent_volatility / 2.0)) * this->dt;
    double random_shock = this->percent_volatility * sqrt(this->dt) * d(gen);
    double total_move = expected_return + random_shock;

    if (total_move < -0.01) {
        total_move *= 0.5;
    }

    if (rand() % 100 < 2) {
        total_move += 0.03;
    }

    return clamp_price(x_t, x_t * exp(total_move));
  }

  // return the number of datapoints generated, if data is not being tested
  int generate_ws(Data_Transfer *gen_settings) {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> norm{0.0, 1.0};

    // Values near the mean are the most likely. Standard deviation
    // affects the dispersion of generated values from the mean.
    int tracker = 0;
    int i = 0;
    double precede = 0.0;
    double curr = 0.0;
    double larger = 0.0;
    int flash_crash_points = 15;
    while (gen_settings->gen.load()) {
      if (gen_settings->pause.load()) {
        continue;
      }
      if (gen_settings->conn.load() == nullptr)
      {
        goto skip;
      }

      if (gen_settings->new_event.load()) {
        switch (gen_settings->new_event.load())
        {
          // flash crash
          case 1:
            {
              if (tracker == 0)
              {
                flash_crash_points = 14 + rand() % 11;
                precede = data_buffer->front();
                curr = gbm(precede, norm, gen);
              }

              if (tracker < flash_crash_points)
              {
                double offset = (rand() % ((int) ((double) base_price * 0.34)))
                  + (double) base_price * 0.5436;

                while (offset > curr * 0.8)
                {
                  offset *= 0.7891;
                }

                while (offset < curr * 0.54)
                {
                  offset *= 1.2142;
                }

                if (gen_settings->send_data.load()) {
                  gen_settings->conn.load()
                    ->send_text(fmt::format("{{\"price\": {}, \"clamped\": {}, \"type\": \"tagged\"}}",
                                (double) (curr - offset), this->last_was_clamped ? "true" : "false"));
                }

                precede = curr;
                curr = gbm(precede, norm, gen);
              }
              else {
                gen_settings->new_event.store(0);

                if (gen_settings->send_data.load()) {
                  gen_settings->conn.load()
                    ->send_text(fmt::format("{{\"price\": {}, \"clamped\": {}, \"type\": \"normal\"}}",
                                curr, this->last_was_clamped ? "true" : "false"));
                }

                tracker = 0;
                flash_crash_points = 15;
              }

              tracker += 1;

              break;
            }
          case 2:
            {
              if (tracker == 0)
              {
                larger = data_buffer->front();
                curr = gbm(larger, norm, gen);
              }

              if (curr > gen_settings->threshold.load())
              {
                fmt::print("threshold reached\n");

                gen_settings->new_event.store(0);

                data_buffer->push(gbm(curr * 0.3213, norm, gen));

                if (gen_settings->send_data.load()) {
                  gen_settings->conn.load()
                    ->send_text(fmt::format("{{\"price\": {}, \"clamped\": {}, \"type\": \"normal\"}}",
                                send_price(), this->last_was_clamped ? "true" : "false"));
                }

                tracker = 0;
              }
              else {
                while (curr < (larger * 0.93481))
                {
                  curr *= 1.2435;
                }

                if (gen_settings->send_data.load()) {
                  gen_settings->conn.load()
                    ->send_text(fmt::format("{{\"price\": {}, \"clamped\": {}, \"type\": \"tagged\"}}",
                                curr, this->last_was_clamped ? "true" : "false"));
                }

                tracker += 1;

                larger = curr > larger ? curr : larger;
                curr = gbm(larger, norm, gen);
              }

              break;
            }
          case 3: // Ornstein–Uhlenbeck process
            {
              this->percent_drift = 0.02;
              this->percent_volatility = 2;
              data_buffer->push(ou(data_buffer->front(), norm, gen));
              if (gen_settings->send_data.load()) {
                double res = send_price();
                gen_settings->conn.load()->send_text(fmt::format("{{\"price\": {}, \"clamped\": {}, \"type\": \"sideways\"}}", res, this->last_was_clamped ? "true" : "false"));
              }
              break;
            }
          case 4: //Bear market
            {
              this->percent_drift = -5.0;
              this->percent_volatility = 0.30;

              data_buffer->push(bear_math(data_buffer->front(), norm, gen));

              if (gen_settings->send_data.load()) {
                gen_settings->conn.load()->send_text(fmt::format("{{\"price\": {}, \"clamped\": {}, \"type\": \"bear\"}}", send_price(), this->last_was_clamped ? "true" : "false"));
              }
              break;
            }

          case 5: //Bull market
            {
              this->percent_drift = 5.0;
              this->percent_volatility = 0.15;

              data_buffer->push(bull_math(data_buffer->front(), norm, gen));

              if (gen_settings->send_data.load()) {
                gen_settings->conn.load()->send_text(fmt::format("{{\"price\": {}, \"clamped\": {}, \"type\": \"bull\"}}", send_price(), this->last_was_clamped ? "true" : "false"));
              }
              break;
            }
          default:
            break;
        }

        /*
        get_event(
          gen_settings->n_drift,
          gen_settings->n_vol,
          gen_settings->n_price
        );
        */
      }
      else {
        // no event
        // generate the next data point in the weiner process and add it onto
        // the data buffer, before dequeuing it
        data_buffer->push(gbm(data_buffer->front(), norm, gen));

        // TODO: way to send data would go here, this could be in the format
        // of a second queue, extra fields in the Data_Transfer, etc. for now
        // printing to console will suffice
        if (gen_settings->send_data.load()) {
          gen_settings->conn.load()->send_text(fmt::format("{{\"price\": {}, \"clamped\": {}, \"type\": \"normal\"}}", send_price(), this->last_was_clamped ? "true" : "false"));
        }
      }

      // goto
skip:
      // goto

      i += 1;
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    return i;
  }

  int generate_ws(Data_Transfer *gen_settings,
                  std::vector<double> *streamed_points) {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> norm{0.0, 1.0};

    // Values near the mean are the most likely. Standard deviation
    // affects the dispersion of generated values from the mean.
    int tracker = 0;
    int i = 0;
    double precede = 0.0;
    double curr = 0.0;
    double larger = 0.0;
    int flash_crash_points = 15;
    while (gen_settings->gen.load()) {
      if (gen_settings->pause.load()) {
        continue;
      }
      if (gen_settings->conn.load() == nullptr)
      {
        goto skip;
      }

      if (gen_settings->new_event.load()) {
        switch (gen_settings->new_event.load())
        {
          // flash crash
          case 1:
            {
              if (tracker == 0)
              {
                flash_crash_points = 14 + rand() % 11;
                precede = data_buffer->front();
                curr = gbm(precede, norm, gen);
              }

              if (tracker < flash_crash_points)
              {
                double offset = (rand() % ((int) ((double) base_price * 0.34)))
                  + (double) base_price * 0.5436;

                while (offset > curr * 0.8)
                {
                  offset *= 0.7891;
                }

                while (offset < curr * 0.54)
                {
                  offset *= 1.2142;
                }

                if (gen_settings->send_data.load()) {
                  gen_settings->conn.load()
                    ->send_text(fmt::format("{{\"price\": {}, \"clamped\": {}, \"type\": \"tagged\"}}",
                                (double) (curr - offset), this->last_was_clamped ? "true" : "false"));
                }

                precede = curr;
                curr = gbm(precede, norm, gen);
              }
              else {
                gen_settings->new_event.store(0);

                if (gen_settings->send_data.load()) {
                  gen_settings->conn.load()
                    ->send_text(fmt::format("{{\"price\": {}, \"clamped\": {}, \"type\": \"normal\"}}",
                                curr, this->last_was_clamped ? "true" : "false"));
                }

                tracker = 0;
                flash_crash_points = 15;
              }

              tracker += 1;

              break;
            }
          case 2:
            {
              if (tracker == 0)
              {
                larger = data_buffer->front();
                curr = gbm(larger, norm, gen);
              }

              if (curr > gen_settings->threshold.load())
              {
                fmt::print("threshold reached\n");

                gen_settings->new_event.store(0);

                data_buffer->push(gbm(curr * 0.3213, norm, gen));

                if (gen_settings->send_data.load()) {
                  gen_settings->conn.load()
                    ->send_text(fmt::format("{{\"price\": {}, \"clamped\": {}, \"type\": \"normal\"}}",
                                send_price(), this->last_was_clamped ? "true" : "false"));
                }

                tracker = 0;
              }
              else {
                while (curr < (larger * 0.93481))
                {
                  curr *= 1.2435;
                }

                if (gen_settings->send_data.load()) {
                  gen_settings->conn.load()
                    ->send_text(fmt::format("{{\"price\": {}, \"clamped\": {}, \"type\": \"tagged\"}}",
                                curr, this->last_was_clamped ? "true" : "false"));
                }

                tracker += 1;

                larger = curr > larger ? curr : larger;
                curr = gbm(larger, norm, gen);
              }

              break;
            }
          case 3: // Ornstein–Uhlenbeck process
            {
              data_buffer->push(ou(data_buffer->front(), norm, gen));
              if (gen_settings->send_data.load()) {
                gen_settings->conn.load()
                  ->send_text(fmt::format("{{\"price\": {}, \"clamped\": {}, \"type\": \"sideways\"}}", send_price(), this->last_was_clamped ? "true" : "false"));
              }
            }
          default:
            break;
        }

        /*
        get_event(
          gen_settings->n_drift,
          gen_settings->n_vol,
          gen_settings->n_price
        );
        */
      }
      else {
        // no event
        // generate the next data point in the weiner process and add it onto
        // the data buffer, before dequeuing it
        data_buffer->push(gbm(data_buffer->front(), norm, gen));

        // TODO: way to send data would go here, this could be in the format
        // of a second queue, extra fields in the Data_Transfer, etc. for now
        // printing to console will suffice
        if (gen_settings->send_data.load()) {
          double price_point = send_price();
          gen_settings->conn.load()
            ->send_text(fmt::format("{{\"price\": {}, \"clamped\": {}, \"type\": \"normal\"}}", price_point, this->last_was_clamped ? "true" : "false"));
          streamed_points->push_back(price_point);
        }
      }

      // goto
skip:
      // goto

      i += 1;
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    return i;
  }
};
