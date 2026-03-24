// Things to bring up, with this class, do we want this to be multithreaded,
// if it is, is locking going to be needed, and should we make out own queue class

// this is based on drifts, but with our events, we might need to be able to model jumps as well

#pragma once

#include "def.hpp"
#include "queue.hpp"
#include "dataTransfer.cpp"
#include <cmath>
#include <thread>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <map>
#include <random>
#include <string>
#include <stdlib.h>

class generator {
private:
  double percent_drift;
  double percent_volatility;
  double dt;
  // queue for streaming data
  Queue<double> *dataBuffer;
  // create a random device for normal distribution
  
  std::random_device *device;
  std::mt19937 *gen;
  std::normal_distribution<> *dist;

  std::string ticker;
  int base_price;
  int volatility;
  int liquidity;
  int market_cap;

public:
    // constructor and destructor
  generator(double drift, double volatility, int price) {
    percent_drift = drift;
    percent_volatility = volatility;
    dataBuffer = new Queue<double>();
    (*dataBuffer).enqueue(price);
    dt = 0.01;
  }

  generator(std::string ticker, int base_price, int volatility, int liquidity, int market_cap) {
    this->ticker = ticker;
    this->base_price = base_price;
    this->volatility = volatility;
    this->liquidity = liquidity;
    this->market_cap = market_cap;
  }

  ~generator() {
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
    dataBuffer->enqueue(n_price);
  }

  double sendPrice() {
    return dataBuffer->dequeue();
  }

    
  /* 
   * Basic data generation function,
   * takes, current price
   */
  void generate_new_data_point() {
    u32 new_data = dataBuffer->peek();
    short multiplier = 1;
    if (rand() % 10 == 0) {
      multiplier = -1;
    }
    new_data = new_data + multiplier * (rand() % (u32) (percent_drift * 100));
    dataBuffer->enqueue(new_data);
  }

  /*
   * This function returns the brownian motion at increment t, given the Weiner process at time t
   */
  double gbm(double S_0, std::normal_distribution<double> &d, std::mt19937 &gen) {
    double ret = (this->percent_drift - (this->percent_volatility * this->percent_volatility / 2))
                 * dt + this->percent_volatility * sqrt(dt) * d(gen);
    ret = S_0 * exp(ret);
    return ret;
  }

  // return the number of datapoints generated, if data is not being tested
  int generate(dataTransfer *gen_settings, std::ostream &fout) {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> norm{0.0,1.0};

    // Values near the mean are the most likely. Standard deviation
    // affects the dispersion of generated values from the mean.
    int i = 0;
    while (gen_settings->gen) {
      // generate the next data point in the weiner process and add it onto the data buffer, before dequeuing it
      dataBuffer->enqueue(gbm(dataBuffer->peek(), norm, gen));
      
      // TODO: way to send data would go here, this could be in the format of a
      // second queue, extra fields in the dataTransfer, etc. for now
      // printing to console will suffice
      
      if (gen_settings->send_data) {
        fout << sendPrice() << "\n";
      }

      // TODO: process events, rn all I do is call the call the get_event function
      // but this logic might need to be adjusted
      if (gen_settings->new_event) {
        get_event(gen_settings->n_drift, gen_settings->n_vol, gen_settings->n_price);
      }
      
      i += 1;
      std::this_thread::sleep_for(std::chrono::milliseconds(9));
    }
    return i;
  }

};
