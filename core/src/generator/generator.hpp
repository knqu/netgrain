// Things to bring up, with this class, do we want this to be multithreaded,
// if it is, is locking going to be needed, and should we make out own queue class

// this is based on drifts, but with our events, we might need to be able to model jumps as well

#pragma once

#include <crow.h>

#include <fmt/format.h>
#include <fmt/core.h>

#include "def.hpp"
#include "queue.hpp"
#include "data_transfer.hpp"
#include <cmath>
#include <thread>
#include <iostream>
#include <ostream>
#include <random>
#include <string>
#include <stdlib.h>

class Generator {
private:
  double percent_drift;
  double percent_volatility;
  double dt;

  // queue for streaming data
  Queue<double> *data_buffer;
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

public:
  // constructor and destructor
  Generator(double drift, double volatility, int price, int target) {
    percent_drift = drift;
    percent_volatility = volatility;
    data_buffer = new Queue<double>();
    data_buffer->enqueue(price);
    base_price = price;
    dt = 0.01;
    target_price = target;
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
  }

  ~Generator() {
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
    data_buffer->enqueue(n_price);
  }

  double send_price() {
    return data_buffer->dequeue();
  }

  /*
   * Basic data generation function,
   * takes, current price
   */
  void generate_new_data_point() {
    u32 new_data = data_buffer->peek();
    short multiplier = 1;
    if (rand() % 10 == 0) {
      multiplier = -1;
    }
    new_data = new_data + multiplier * (rand() % (u32) (percent_drift * 100));
    data_buffer->enqueue(new_data);
  }

  /*
   * This function returns the brownian motion at increment t, given the
   * Weiner process at time t
   */
  double gbm(double S_0, std::normal_distribution<double> &d,
             std::mt19937 &gen) {
    double ret =
      (
        this->percent_drift
        - (this->percent_volatility * this->percent_volatility / 2)
      ) * dt + this->percent_volatility * sqrt(dt) * d(gen);
    ret = S_0 * exp(ret);
    return ret;
  }

  /*
   * This function implements Ornstein–Uhlenbeck process for a sideways trading market.
   */
  double ou(double x_t, std::normal_distribution<double> &d, std::mt19937 &gen) {
    return x_t + ((this->speed_of_reversion * (this->mean - x_t)) * dt) + (this->volatility * sqrt(dt) * d(gen));
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
      // generate the next data point in the weiner process and add it onto
      // the data buffer, before dequeuing it
      data_buffer->enqueue(gbm(data_buffer->peek(), norm, gen));

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
                precede = data_buffer->peek();
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
                    ->send_text(fmt::format("tagged: {}",
                                (double) (curr - offset)));
                }

                precede = curr;
                curr = gbm(precede, norm, gen);
              }
              else {
                gen_settings->new_event.store(0);

                if (gen_settings->send_data.load()) {
                  gen_settings->conn.load()
                    ->send_text(fmt::format("{}", curr));
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
                larger = data_buffer->peek();
                curr = gbm(larger, norm, gen);
              }

              if (curr > gen_settings->threshold.load())
              {
                fmt::print("threshold reached\n");

                gen_settings->new_event.store(0);

                data_buffer->enqueue(gbm(curr * 0.3213, norm, gen));

                if (gen_settings->send_data.load()) {
                  gen_settings->conn.load()
                    ->send_text(fmt::format("{}", send_price()));
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
                    ->send_text(fmt::format("tagged: {}", curr));
                }

                tracker += 1;

                larger = curr > larger ? curr : larger;
                curr = gbm(larger, norm, gen);
              }

              break;
            }
          case 3: // Ornstein–Uhlenbeck process 
            {
              data_buffer->enqueue(ou(data_buffer->peek(), norm, gen));
              if (gen_settings->send_data.load()) {
                gen_settings->conn.load()
                  ->send_text(fmt::format("Sideways: {}", send_price()));
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
        data_buffer->enqueue(gbm(data_buffer->peek(), norm, gen));

        // TODO: way to send data would go here, this could be in the format
        // of a second queue, extra fields in the Data_Transfer, etc. for now
        // printing to console will suffice
        if (gen_settings->send_data.load()) {
          gen_settings->conn.load()
            ->send_text(fmt::format("{}", send_price()));
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
