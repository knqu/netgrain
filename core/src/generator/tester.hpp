/* This program is designed to take in the information of generator,
    and compare it with theoretical gbm to ensure it functions correctly */

// in order to do this, we need to get the results of several generation runs
// and then compare the returns with the time delta * drift

#include <iostream>
#include <fstream>
#include "generator.hpp"

struct Testor {
  // basic idea run a bunch of simulations with a known drift,
  // and verify that the average of the result is within a delta
  // of the expected
  static void testSim(char *file_name, Generator &gen, double max_diff) {
    // take data from simulation, then keep a running total of sums
    // find the average of these sums (divided by init price), and
    // compare with expeccted change assume filename is passed.

    std::ifstream in(file_name);
    double curVal = 0.0;
    double prevVal = -1.0;
    double total = 0.0;
    int count = 0;

    // because each val is multiplied by the previous val
    // we divide by the previous to just get the exponential
    // val of the process
    while (in >> curVal) {
      // divide cur by prevVal

      // std::cout << curVal << "\n";
      if (prevVal == -1.0) {
        prevVal = curVal;
        total = 1.0;
        count = 1;
        continue;
      }
      total += (curVal / prevVal);
      count += 1;
      prevVal = curVal;
    }

    // compute how generated average compares with drift * dt
    double avg = (total / count) - 1;
    double expected = gen.get_percent_drift() * gen.get_dt();
    std::cout << "average return percent per time delta: "
              << avg << "\n";

    std::cout << "expected percentage return per time delts: "
              << expected << "\n";

    double percent_diff =
      abs(avg - expected) / ((avg + expected) * 0.5) * 100;
    std::cout
      << "The percent difference between your input, "
      << "and the expected input was: "
      << percent_diff << "%\n";

    if (percent_diff > max_diff) {
      std::cout << "The percent difference was greater than expected\n";
      return;
    }
    std::cout << "The percent difference falls within range\n";
  }
};

