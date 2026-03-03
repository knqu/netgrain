#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include "generator.cpp"
#include "benchmarks.cpp"

int main()
{
    printf("test\n");
    std::random_device rd{};
    std::mt19937 gen{rd()};

    // get info for initial stock data

    double vol;
    double drift;
    int init_price;
    std::cout << "Please enter the percent drift of the stock data: ";
    std::cin >> drift;
    std::cout << "Please enter the percent volatility of the stock data: ";
    std::cin >> vol;
    std::cout << "Please enter the initial price: ";
    std::cin >> init_price;

    // generator gene(drift, vol, init_price);


    
    gen_benchmark tester;
    tester.speed_benchmark(vol, drift, init_price);
    tester.data_benchmark(vol, drift, init_price);


    
}
