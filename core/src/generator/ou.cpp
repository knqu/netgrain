#pragma once

#include "def.hpp"
#include "queue.hpp"
#include "dataTransfer.cpp"
#include <cmath>
#include <random>
#include <stdlib.h>

class ouGen {
private:
  double volatility;
  double speed_of_reversion;
  double mean;
  Queue<double> *dataBuffer;

public:
    // constructor and destructor
  ouGen(double price, double vol, double speed, double mean) {
    volatility = vol;
    speed_of_reversion = speed;
    this->mean = mean;
    dataBuffer = new Queue<double>();
    (*dataBuffer).enqueue(price);
  }

  ~ouGen() {
  }

  // HX: Pass in mu (long-term mean, avg b/w ceil and floor)
  double ou(double dt, double x_t, std::normal_distribution<double> &d, std::mt19937 &gen) {
    return x_t + ((this->speed_of_reversion * (this->mean - x_t)) * dt) + (this->volatility * sqrt(dt) * d(gen));
  }

  // HX
  int gen(dataTransfer *gen_settings) {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> norm{0.0,1.0};

    int i = 0;
    while (gen_settings->gen) {
      dataBuffer->enqueue(ou(0.02, dataBuffer->peek(), norm, gen));
      dataBuffer->dequeue();
      i += 1;
    }
    return i;
  }
};
