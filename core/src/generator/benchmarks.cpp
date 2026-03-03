#pragma once

#include "generator.cpp"
#include <iostream>
#include <chrono>
#include <thread>

#define NONE (-1)
#define SPEED (0)
#define THROTTLE (1)
#define OK (0)
// benchmark program designed to stream data to an output file, based on user
// entered data, keep count of how much data is streamed per second,
// this is mainly to validate data generation speed, as well as show that data gen
// can be stopped and throttled


class gen_benchmark {
    private:
        int test;
    public:
    gen_benchmark() {
         this->test = 1;
    }
    ~gen_benchmark() {}


    /*
     * This function works as a timer, returns 0
     * when timer runs out
     */
    static int timer(int seconds, dataTransfer &test) {
        while (seconds >= 1 ) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            seconds -= 1;
        }
        // update value in val to indicate that loop should end
        test.gen = 0;
        return 0;
    }

    int speed_benchmark(double drift, double vol, int init_price) {
        // step one, create generator

        generator new_gen(drift, vol, init_price);
        dataTransfer tester;
        tester.gen = 1;
        tester.send_data = 0;
        tester.new_event = 0;
        int count = 0;
        int seconds;
        std::cout << "How many seconds do you want the program to run for (int)? ";
        std::cin >> seconds;
        // start a timer for entered seconds and have it run as a separate
        // process
        std::thread t(timer, seconds, std::ref(tester));
        t.detach();
        
        // generate while the timer is still going on
        count = new_gen.generate(&tester);
        std::cout << count << "\n";
        return count;
    }

    int data_benchmark(double drift, double vol, int init_price) {
        // create generator

        generator new_gen(drift, vol, init_price);
        dataTransfer tester;
        tester.gen = 1;
        tester.new_event = 0;
        tester.send_data = 1;
        // generate 1 second of data, and print out all values generated
        std::thread t(timer, 2, std::ref(tester));
        t.detach();

        new_gen.generate(&tester);
        return OK;
    }
};

