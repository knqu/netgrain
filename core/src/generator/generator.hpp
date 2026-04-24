// Things to bring up, with this class, do we want this to be multithreaded,
// if it is, is locking going to be needed, and should we make out own queue class

// this is based on drifts, but with our events, we might need to be able to model jumps as well

#pragma once

#include <crow.h>

#include <fmt/format.h>

#include "../simulator/def.hpp"
#include "data_transfer.hpp"
#include "../simulator/historicalData.hpp"
#include <cmath>
#include <thread>
#include <iostream>
#include <ostream>
#include <random>
#include <string>
#include <stdlib.h>
#include <queue>
#include <thread>
#include <chrono>
#include <cstdint>
#include <limits>

class Generator {
public:
  double percent_drift;
  double percent_volatility;
  double dt;

  std::vector<double> *streamed_points;

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

  int id;
  Data_Transfer gen_settings;

  double last_bar_close = 0.0;
  bool has_bar_state = false;
  bool last_was_clamped = false; // Tracks if the very last tick was an outlier correction

  std::mt19937 rng_{};
  bool rng_seeded_ = false;
  uint32_t rng_seed_ = 0;
  bool normal_has_spare_ = false;
  double normal_spare_ = 0.0;

  void seed_rng_nondeterministic_() {
    std::random_device rd{};
    std::seed_seq seq{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
    rng_.seed(seq);
    rng_seeded_ = false;
    normal_has_spare_ = false;
  }

  void seed_rng_deterministic_(uint32_t seed) {
    rng_.seed(seed);
    rng_seeded_ = true;
    rng_seed_ = seed;
    normal_has_spare_ = false;
  }

  double rand_unit_() {
    return std::generate_canonical<double, std::numeric_limits<double>::digits>(rng_);
  }

  double rand_normal01_() {
    if (normal_has_spare_) {
      normal_has_spare_ = false;
      return normal_spare_;
    }

    double u1 = rand_unit_();
    while (u1 <= 0.0) u1 = rand_unit_();
    const double u2 = rand_unit_();

    constexpr double kTwoPi = 6.283185307179586476925286766559;
    const double r = std::sqrt(-2.0 * std::log(u1));
    const double theta = kTwoPi * u2;

    const double z0 = r * std::cos(theta);
    const double z1 = r * std::sin(theta);

    normal_spare_ = z1;
    normal_has_spare_ = true;
    return z0;
  }

  int rand_int_inclusive_(int lo, int hi) {
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(rng_);
  }

  // constructor and destructor
  Generator(double drift, double volatility, int price, int target, int id, bool deterministic = false, uint32_t seed = 0) {
    this->percent_drift = drift;
    this->percent_volatility = volatility;
    this->base_price = price;
    this->target_price = target;
    this->dt = 0.01;
    this->data_buffer = new std::queue<double>;
    this->data_buffer->push(price);
    this->streamed_points = new std::vector<double>;
    this->id = id;

    // initialize Data Transfer
    this->gen_settings.conn.store(nullptr);
    this->gen_settings.gen.store(true);
    this->gen_settings.new_event.store(0);
    this->gen_settings.send_data.store(false);

    deterministic ? seed_rng_deterministic_(seed) : seed_rng_nondeterministic_();
  }

  Generator(
    std::string ticker,
    int base_price,
    int volatility,
    int liquidity,
    int market_cap,
    int id,
    bool deterministic = false,
    uint32_t seed = 0) {

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
    this->streamed_points = new std::vector<double>;
    this->id = id;

    this->gen_settings.conn.store(nullptr);
    this->gen_settings.gen.store(true);
    this->gen_settings.new_event.store(0);
    this->gen_settings.send_data.store(false);
    // NOTE: even though the simulator passes in an int scaled by 100,
    // the gbm math looks to be scale invariant

    deterministic ? seed_rng_deterministic_(seed) : seed_rng_nondeterministic_();
  }

  ~Generator() {
      delete data_buffer;
      delete streamed_points;
  }

  Generator(const Generator& ) = delete;

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

  // Deterministic seeding. Call before generating to make outputs repeatable.
  void set_seed(uint32_t seed) {
    seed_rng_deterministic_(seed);
  }

  bool is_seeded_deterministic() const {
    return rng_seeded_;
  }

  uint32_t get_seed() const {
    return rng_seed_;
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
    if (rand_int_inclusive_(0, 9) == 0) {
      multiplier = -1;
    }
    const int max_step = std::max(0, static_cast<int>(percent_drift * 100));
    const int step = (max_step > 0) ? rand_int_inclusive_(0, max_step - 1) : 0;
    new_data = new_data + multiplier * static_cast<u32>(step);
    data_buffer->push(new_data);
  }

  /*
   * This function returns the brownian motion at increment t, given the
   * Weiner process at time t
   */
  double gbm(double S_0) {
    double ret =
      (
        this->percent_drift
        - (this->percent_volatility * this->percent_volatility / 2)
      ) * dt + this->percent_volatility * sqrt(dt) * rand_normal01_();
    ret = S_0 * exp(ret);
    return clamp_price(S_0, ret);
  }

  // generates one ohlcv bar by simulating ticks_per_bar intra-bar price movements via gbm, then aggregating
  MarketDataRow generate_bar(u32 date, int ticks_per_bar = 50) {
    // state is carried between calls so close(N) == open(N+1)
    double cur = has_bar_state ? last_bar_close : static_cast<double>(base_price);
    double open = cur;
    double high = cur;
    double low = cur;

    for (int i = 0; i < ticks_per_bar; i++) {
      cur = gbm(cur);
      if (cur > high) high = cur;
      if (cur < low) low = cur;
    }

    last_bar_close = cur;
    has_bar_state = true;

    // simulate volume from liquidity parameter - made this up,
    // values are arbitrary (between 80-120%)
    u64 volume = (liquidity > 0)
      ? static_cast<u64>(liquidity * rand_int_inclusive_(80, 120) / 100)
      : static_cast<u64>(rand_int_inclusive_(500, 1499));

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
    // Values near the mean are the most likely. Standard deviation
    // affects the dispersion of generated values from the mean.
    int i = 0;
    while (gen_settings->gen.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      // generate the next data point in the weiner process and add it onto
      // the data buffer, before dequeuing it
      data_buffer->push(gbm(data_buffer->front()));

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
  double ou(double x_t) {
    double ret =
      x_t + ((this->percent_drift * (this->target_price - x_t)) * dt)
      + (this->percent_volatility * sqrt(dt) * rand_normal01_());
    return clamp_price(x_t, ret);
  }

  /*
   * bear maket logic -> based on gbm, customized to have a higher probability
   * of large negative moves, and a lower probability of large positive moves
   * Uses class drift/volatility, but occasionally triggers "panic selling"
   */
  double bear_math(double x_t) {
    double expected_return =
      (this->percent_drift - (this->percent_volatility * this->percent_volatility / 2.0)) * this->dt;
    double random_shock = this->percent_volatility * sqrt(this->dt) * rand_normal01_();
    double total_move = expected_return + random_shock;

    if (rand_int_inclusive_(0, 99) < 3) {
        total_move -= 0.05;
    }

    return clamp_price(x_t, x_t * exp(total_move));
  }

  /*
   * Bull Market logic based on gbm, customized to have a higher probability of
   * large positive moves, and a lower probability of large negative moves
   * Uses class drift/volatility, but incorporates "buy the dip" and FOMO behavior
   */
  double bull_math(double x_t) {
    double expected_return =
      (this->percent_drift - (this->percent_volatility * this->percent_volatility / 2.0)) * this->dt;
    double random_shock = this->percent_volatility * sqrt(this->dt) * rand_normal01_();
    double total_move = expected_return + random_shock;

    if (total_move < -0.01) {
        total_move *= 0.5;
    }

    if (rand_int_inclusive_(0, 99) < 2) {
        total_move += 0.03;
    }

    return clamp_price(x_t, x_t * exp(total_move));
  }

  // return the number of datapoints generated, if data is not being tested
  int generate_ws() {
    // Values near the mean are the most likely. Standard deviation
    // affects the dispersion of generated values from the mean.
    int tracker = 0;
    int i = 0;
    double precede = 0.0;
    double curr = 0.0;
    double larger = 0.0;
    int flash_crash_points = 15;
    while (gen_settings.gen.load()) {
      if (gen_settings.pause.load()) {
        continue;
      }

      if (gen_settings.conn.load() == nullptr)
      {
        goto skip;
      }

      if (gen_settings.new_event.load()) {
        switch (gen_settings.new_event.load())
        {
          // flash crash
          case 1:
            {
              if (tracker == 0)
              {
                flash_crash_points = rand_int_inclusive_(14, 24);
                precede = data_buffer->front();
                curr = gbm(precede);
              }

              if (tracker < flash_crash_points)
              {
                const int max_off = std::max(1, static_cast<int>(static_cast<double>(base_price) * 0.34));
                double offset = rand_int_inclusive_(0, max_off - 1) + (double) base_price * 0.5436;

                while (offset > curr * 0.8)
                {
                  offset *= 0.7891;
                }

                while (offset < curr * 0.54)
                {
                  offset *= 1.2142;
                }

                if (gen_settings.send_data.load()) {
                  gen_settings.conn.load()
                    ->send_text(fmt::format(
                      "{{\"price\": {}, \"clamped\": {}, \"type\": \"tagged\", \"id\": {}, \"drift\": {}, \"volatility\": {}, \"msg_type\": \" stock \"}}",
                      (double) (curr - offset), this->last_was_clamped ? "true" : "false", this->id, this->percent_drift, this->percent_volatility));
                  this->streamed_points->push_back((double) curr - offset);
                }

                precede = curr;
                curr = gbm(precede);
              }
              else {
                gen_settings.new_event.store(0);

                if (gen_settings.send_data.load()) {
                  gen_settings.conn.load()
                    ->send_text(fmt::format(
                      "{{\"price\": {}, \"clamped\": {}, \"type\": \"normal\", \"id\": {}, \"drift\": {}, \"volatility\": {}, \"msg_type\": \" stock \"}}",
                      curr, this->last_was_clamped ? "true" : "false", this->id, this->percent_drift, this->percent_volatility));
                  this->streamed_points->push_back((double) curr);
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
                curr = gbm(larger);
              }

              if (curr > gen_settings.threshold.load())
              {
                fmt::print("threshold reached\n");

                gen_settings.new_event.store(0);

                data_buffer->push(gbm(curr * 0.3213));

                double price_val = send_price();

                if (gen_settings.send_data.load()) {
                  gen_settings.conn.load()
                    ->send_text(fmt::format(
                      "{{\"price\": {}, \"clamped\": {}, \"type\": \"normal\", \"id\": {}, \"drift\": {}, \"volatility\": {}, \"msg_type\": \" stock \"}}",
                      price_val, this->last_was_clamped ? "true" : "false", this->id, this->percent_drift, this->percent_volatility));
                  this->streamed_points->push_back(price_val);
                }

                tracker = 0;
              }
              else {
                while (curr < (larger * 0.93481))
                {
                  curr *= 1.2435;
                }

                if (gen_settings.send_data.load()) {
                  gen_settings.conn.load()
                    ->send_text(fmt::format(
                      "{{\"price\": {}, \"clamped\": {}, \"type\": \"tagged\", \"id\": {}, \"drift\": {}, \"volatility\": {}, \"msg_type\": \" stock \"}}",
                      curr, this->last_was_clamped ? "true" : "false", this->id, this->percent_drift, this->percent_volatility));
                  this->streamed_points->push_back(curr);
                }

                tracker += 1;

                larger = curr > larger ? curr : larger;
                curr = gbm(larger);
              }

              break;
            }
          case 3: // Ornstein–Uhlenbeck process
            {
              this->percent_drift = 0.02;
              this->percent_volatility = 2;
              data_buffer->push(ou(data_buffer->front()));
              if (gen_settings.send_data.load()) {
                double res = send_price();
                gen_settings.conn.load()->send_text(fmt::format(
                  "{{\"price\": {}, \"clamped\": {}, \"type\": \"sideways\", \"id\": {}, \"drift\": {}, \"volatility\": {}, \"msg_type\": \" stock \"}}",
                  res, this->last_was_clamped ? "true" : "false", this->id, this->percent_drift, this->percent_volatility));
                this->streamed_points->push_back(res);
              }
              break;
            }
          case 4: //Bear market
            {
              this->percent_drift = -5.0;
              this->percent_volatility = 0.30;

              data_buffer->push(bear_math(data_buffer->front()));

              if (gen_settings.send_data.load()) {
                double price_val = send_price();
                gen_settings.conn.load()->send_text(fmt::format(
                  "{{\"price\": {}, \"clamped\": {}, \"type\": \"bear\", \"id\": {}, \"drift\": {}, \"volatility\": {}, \"msg_type\": \" stock \"}}",
                  price_val, this->last_was_clamped ? "true" : "false", this->id, this->percent_drift, this->percent_volatility));
                this->streamed_points->push_back(price_val);
              }
              break;
            }

          case 5: //Bull market
            {
              this->percent_drift = 5.0;
              this->percent_volatility = 0.15;

              data_buffer->push(bull_math(data_buffer->front()));

              if (gen_settings.send_data.load()) {
                double price_val = send_price();
                gen_settings.conn.load()->send_text(fmt::format(
                  "{{\"price\": {}, \"clamped\": {}, \"type\": \"bull\", \"id\": {}, \"drift\": {}, \"volatility\": {}, \"msg_type\": \" stock \"}}",
                  price_val, this->last_was_clamped ? "true" : "false", this->id, this->percent_drift, this->percent_volatility));                this->streamed_points->push_back(price_val);
              }
              break;
            }

          case 6: // Reset
            {
              (*data_buffer) = {};
              data_buffer->push(static_cast<double>(gen_settings.reset_price.load()));

              gen_settings.new_event.store(0);
              gen_settings.pause.store(true);

              break;
            }

          default:
            break;
        }
      }
      else {
        // no event
        // generate the next data point in the weiner process and add it onto
        // the data buffer, before dequeuing it
        data_buffer->push(gbm(data_buffer->front()));

        // TODO: way to send data would go here, this could be in the format
        // of a second queue, extra fields in the Data_Transfer, etc. for now
        // printing to console will suffice
        if (gen_settings.send_data.load()) {
          double price_val = send_price();
          gen_settings.conn.load()->send_text(fmt::format(
            "{{\"price\": {}, \"clamped\": {}, \"type\": \"normal\", \"id\": {}, \"drift\": {}, \"volatility\": {}, \"msg_type\": \" stock \"}}",
            price_val, this->last_was_clamped ? "true" : "false", this->id, this->percent_drift, this->percent_volatility));
          this->streamed_points->push_back(price_val);
        }
      }

      // goto
skip:
      // goto

      i += 1;
      std::this_thread::sleep_for(std::chrono::milliseconds(gen_settings.snapshot_interval.load()));
    }
    return i;
  }

  template<typename T>
  void write_to_buffer(std::vector<char> *buffer, T value, size_t *curr_size)
  {
    size_t prev_size = buffer->size();
    buffer->resize(prev_size + sizeof(T));
    memcpy(buffer->data() + prev_size, &value, sizeof(T));
    *curr_size += sizeof(T);
  }

  template<typename T>
  void read_from_buffer(std::vector<char> *buffer, T *value, size_t *curr_size)
  {
    memcpy(value, buffer->data() + *curr_size, sizeof(T));
    *curr_size += sizeof(T);
  }

  std::vector<char> save_simulation(size_t streamed_len) {
    streamed_len = std::min(streamed_len, this->streamed_points->size());

    std::vector streamed_subset(streamed_points->begin(),
                                streamed_points->begin() + streamed_len);

    this->gen_settings.pause.store(true);

    std::vector<char> buffer;
    size_t curr_size = 0;

    double percent_drift = this->get_percent_drift();
    double percent_volatility = this->get_percent_volatility();
    double dt = this->get_dt();
    std::string ticker = this->get_ticker();
    size_t ticker_len = ticker.size();
    int base_price = this->base_price;
    int volatility = this->volatility;
    int liquidity = this->liquidity;
    int market_cap = this->market_cap;
    int target_price = this->target_price;

    write_to_buffer(&buffer, percent_drift, &curr_size);
    write_to_buffer(&buffer, percent_volatility, &curr_size);
    write_to_buffer(&buffer, dt, &curr_size);
    write_to_buffer(&buffer, ticker_len, &curr_size);
    for (int i = 0; i < ticker_len; i++)
    {
      write_to_buffer(&buffer, ticker.at(i), &curr_size);
    }
    write_to_buffer(&buffer, base_price, &curr_size);
    write_to_buffer(&buffer, volatility, &curr_size);
    write_to_buffer(&buffer, liquidity, &curr_size);
    write_to_buffer(&buffer, market_cap, &curr_size);
    write_to_buffer(&buffer, target_price, &curr_size);

    int rate_per_second = this->gen_settings.rate_per_second.load();
    int new_event = this->gen_settings.new_event.load();
    double n_vol = this->gen_settings.n_vol.load();
    double n_drift = this->gen_settings.n_drift.load();
    int n_price = this->gen_settings.n_price.load();
    int threshold = this->gen_settings.threshold.load();
    bool pause = this->gen_settings.threshold.load();

    write_to_buffer(&buffer, rate_per_second, &curr_size);
    write_to_buffer(&buffer, new_event, &curr_size);
    write_to_buffer(&buffer, n_vol, &curr_size);
    write_to_buffer(&buffer, n_drift, &curr_size);
    write_to_buffer(&buffer, n_price, &curr_size);
    write_to_buffer(&buffer, threshold, &curr_size);
    write_to_buffer(&buffer, pause, &curr_size);

    size_t buffer_len = streamed_subset.size();
    fmt::print("streamed_data_len: {}\n", buffer_len);
    write_to_buffer(&buffer, buffer_len, &curr_size);
    for (int i = 0; i < buffer_len; i++)
    {
      write_to_buffer(&buffer, streamed_subset.at(i), &curr_size);
    }

    fmt::print("binary buffer length: {} = {}\n", curr_size, buffer.size());
    return buffer;
  }

  void load_simulation(std::string buffer) {
    std::vector<char> char_buffer(buffer.begin(), buffer.end());

    size_t curr_size = 0;
    read_from_buffer(&char_buffer, &this->percent_drift, &curr_size);
    read_from_buffer(&char_buffer, &this->percent_volatility, &curr_size);
    read_from_buffer(&char_buffer, &this->dt, &curr_size);

    size_t ticker_len;
    read_from_buffer(&char_buffer, &ticker_len, &curr_size);
    char *ticker_buf = (char *) calloc(ticker_len, sizeof(char));
    memcpy(ticker_buf, char_buffer.data() + curr_size, ticker_len);
    curr_size += ticker_len;
    this->ticker = std::string(ticker_buf);

    read_from_buffer(&char_buffer, &this->base_price, &curr_size);
    read_from_buffer(&char_buffer, &this->volatility, &curr_size);
    read_from_buffer(&char_buffer, &this->liquidity, &curr_size);
    read_from_buffer(&char_buffer, &this->market_cap, &curr_size);
    read_from_buffer(&char_buffer, &this->target_price, &curr_size);

    int rate_per_second;
    int new_event;
    double n_vol;
    double n_drift;
    int n_price;
    int threshold;
    bool pause;

    read_from_buffer(&char_buffer, &rate_per_second, &curr_size);
    read_from_buffer(&char_buffer, &new_event, &curr_size);
    read_from_buffer(&char_buffer, &n_vol, &curr_size);
    read_from_buffer(&char_buffer, &n_drift, &curr_size);
    read_from_buffer(&char_buffer, &n_price, &curr_size);
    read_from_buffer(&char_buffer, &threshold, &curr_size);
    read_from_buffer(&char_buffer, &pause, &curr_size);

    this->gen_settings.rate_per_second.store(rate_per_second);
    this->gen_settings.new_event.store(new_event);
    this->gen_settings.n_vol.store(n_vol);
    this->gen_settings.n_drift.store(n_drift);
    this->gen_settings.n_price.store(n_price);
    this->gen_settings.threshold.store(threshold);
    this->gen_settings.pause.store(pause);

    size_t buffer_len;
    read_from_buffer(&char_buffer, &buffer_len, &curr_size);
    this->streamed_points->clear();
    for (int i = 0; i < buffer_len; i++)
    {
      double point;
      read_from_buffer(&char_buffer, &point, &curr_size);
      this->streamed_points->push_back(point);
    }
  }

  void refresh() {
    (*data_buffer) = {};
    data_buffer->push(static_cast<double>(this->streamed_points->back()));
  }

  void set_fields(
    int base_price, double percent_drift, double percent_volatility,
    int market_cap, int target_price
  )
  {
    this->base_price = base_price;
    this->percent_drift = percent_drift;
    this->percent_volatility = percent_volatility;
    this->market_cap = market_cap;
    this->target_price = target_price;

    (*data_buffer) = {};
    data_buffer->push(static_cast<double>(base_price));
  }
};
