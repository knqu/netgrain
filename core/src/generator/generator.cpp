// Things to bring up, with this class, do we want this to be multithreaded,
// if it is, is locking going to be needed, and should we make out own queue class

// this is based on drifts, but with our events, we might need to be able to model jumps as well

#pragma once

#include "def.hpp"
#include "queue.hpp"
#include "dataTransfer.cpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <stdlib.h>


class generator {
private:
  double percent_drift;
  double percent_volatility;

  // queue for streaming data
  Queue<double> *dataBuffer;
  // create a random device for normal distribution
  
  std::random_device *device;
  std::mt19937 *gen;
  std::normal_distribution<> *dist;


public:
    // constructor and destructor
  generator(double drift, double volatility, int price) {
    percent_drift = drift;
    percent_volatility = volatility;
    dataBuffer = new Queue<double>();
    (*dataBuffer).enqueue(price);
    
  }

  ~generator() {
  }

  void get_event(double new_drift, double new_vol, int new_price) {
    percent_drift = new_drift;
    percent_volatility = new_vol;
    dataBuffer->enqueue(0.00);
  }

  int sendPrice() {
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
  double gbm(double dt, double S_0, std::normal_distribution<double> &d, std::mt19937 &gen) {
    double ret = (this->percent_drift - (this->percent_volatility * this->percent_volatility / 2))
                 * dt + this->percent_volatility * sqrt(dt) * d(gen);
    ret = S_0 * exp(ret);
    return ret;
  }

  // return the number of datapoints generated
  int generate(dataTransfer *gen_settings) {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> norm{0.0,1.0};

    // Values near the mean are the most likely. Standard deviation
    // affects the dispersion of generated values from the mean.
    int i = 0;
    while (gen_settings->gen) {
      // generate the next data point in the weiner process and add it onto the data buffer, before dequeuing it
      dataBuffer->enqueue(gbm(0.02, dataBuffer->peek(), norm, gen));
      dataBuffer->dequeue();
      i += 1;
    }
    return i;
  }

};
