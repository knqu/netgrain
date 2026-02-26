// Things to bring up, with this class, do we want this to be multithreaded,
// if it is, is locking going to be needed, and should we make out own queue class

// this is based on drifts, but with our events, we might need to be able to model jumps as well

#include "def.hpp"
#include "queue.hpp"
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
  Queue<u32> *dataBuffer;
  // create a random device for normal distribution
  
  std::random_device *device;
  std::mt19937 *gen;
  std::normal_distribution<> *dist;


public:
    // constructor and destructor
  generator(double drift, double volatility, int price) {
    percent_drift = drift;
    percent_volatility = volatility;
    dataBuffer = new Queue<u32>();
    (*dataBuffer).enqueue(price);
    
  }

  ~generator() {
  }

  void get_event(double new_drift, double new_vol, int new_price) {
    percent_drift = new_drift;
    percent_volatility = new_vol;
    dataBuffer->enqueue(new_price);
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

    // TODO: create loop function to keep generating data, (throttling output at some point)
    // also have this function recieve signals from the simulator so that it can stop if needed
    
    // dW = sqrt(dt) * Normal (0, dt) 
    // W = sum(dW)
  double weiner_process(int t, int n, std::normal_distribution<double> &d, std::mt19937 &gen) {
    // generate a normal distribution of 0 to dt
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
      sum += d(gen);
    }
    return sum * sqrt(n);
  }

  /*
   * This function returns the brownian motion at increment t, given the Weiner process at time t
   */
  double geometirc_brownian_motion(int t, double w, double S_0) {
    double ret = this->percent_drift - (this->percent_volatility * this->percent_volatility / 2)
                 * t + this->percent_volatility * w;
    ret = S_0 * exp(ret);
    return ret;
  }


  void generate() {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> norm{0.0,1.0};

    printf("%lf\n", weiner_process(5, 500, norm, gen));
    // Values near the mean are the most likely. Standard deviation
    // affects the dispersion of generated values from the mean.
    
  }

};
// TODO: implement data generation with stochastic diff eq
// dS_t = vol*S_t dt + drift S_t dW_t-
// solves to S_t = S_0 e^(( drift - vol^2/2)t + vol*W_t)
