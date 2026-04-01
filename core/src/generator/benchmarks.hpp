#pragma once

#include "generator.hpp"
#include <functional>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include "tester.hpp"

#define NONE (-1)
#define SPEED (0)
#define THROTTLE (1)
#define OK (0)
// benchmark program designed to stream data to an output file, based on user
// entered data, keep count of how much data is streamed per second,
// this is mainly to validate data generation speed, as well as show that data gen
// can be stopped and throttled

class Gen_Benchmark {
  private:
    int test;
  public:
    Gen_Benchmark() {
      this->test = 1;
    }
    ~Gen_Benchmark() {}

    /*
     * This function works as a timer, returns 0
     * when timer runs out
     */
    static int timer(int seconds, Data_Transfer &test) {
      while (seconds >= 1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        seconds -= 1;
      }
      // update value in val to indicate that loop should end
      test.gen.store(false);
      return 0;
    }

    int speed_benchmark(double drift, double vol, int init_price) {
      // step one, create generator
      Generator new_gen(drift, vol, init_price);
      Data_Transfer tester;
      tester.gen.store(true);
      tester.new_event.store(0);
      tester.send_data.store(false);
      int count = 0;
      int seconds;
      std::cout
        << "How many seconds do you want the program to run for (int)? ";
      std::cin >> seconds;

      // start a timer for entered seconds and have it run as a separate
      // process
      std::thread t(timer, seconds, std::ref(tester));
      t.detach();

      // generate while the timer is still going on
      count = new_gen.generate(&tester, std::cout);
      std::cout << count << "\n";
      return count;
    }

    int data_benchmark(double drift, double vol, int init_price) {
      // create generator
      Generator new_gen(drift, vol, init_price);
      std::cout << "Generator info:\n";
      std::cout << "Volatility: " << new_gen.get_percent_volatility() << "\n";
      std::cout << "Drift: " << new_gen.get_percent_drift() << "\n\n";
      Data_Transfer tester;
      tester.gen.store(true);
      tester.new_event.store(0);
      tester.send_data.store(true);

      // generate 1 second of data, and print out all values generated
      std::thread t(timer, 2, std::ref(tester));
      t.detach();

      new_gen.generate(&tester, std::cout);
      return OK;
    }

    int verification_benchmark(double drift, double vol, int init_price) {
      // create generator
      Generator new_gen(drift, vol, init_price);
      Data_Transfer tester;
      tester.gen.store(true);
      tester.new_event.store(0);
      tester.send_data.store(true);
      std::ofstream out("test.txt");
      // generate 1 second of data, and print out all values generated
      std::thread t(timer, 15, std::ref(tester));
      t.detach();

      new_gen.generate(&tester, out);

      out.close();

      double percent_diff;
      std::cout << "What is the max percent difference expected? ";
      std::cin >> percent_diff;
      Testor::testSim((char *) "test.txt", new_gen, percent_diff);

      return OK;
    }
};
