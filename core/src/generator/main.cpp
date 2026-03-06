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
    std::random_device rd{};
    std::mt19937 gen{rd()};

    // get info for initial stock data

    double vol;
    double drift;
    int init_price;
    gen_benchmark tester;

    while (1) {
        int option;
        std::cout << "Please enter what benchmark you want to test:\n1. speed benchmark\n2: data benchmark\n3: data validation benchmark\n4: exit\n";
        std::cin >> option;
        switch (option)
        {
        case 1:
            std::cout << "Please enter the percent drift of the stock data: ";
            std::cin >> drift;
            std::cout << "Please enter the percent volatility of the stock data: ";
            std::cin >> vol;
            std::cout << "Please enter the initial price: ";
            std::cin >> init_price;
            std::cout << "beginning speed benchmark program\n";
            tester.speed_benchmark(drift, vol, init_price);
            break;
        case 2:
            std::cout << "Please enter the percent drift of the stock data: ";
            std::cin >> drift;
            std::cout << "Please enter the percent volatility of the stock data: ";
            std::cin >> vol;
            std::cout << "Please enter the initial price: ";
            std::cin >> init_price;
            std::cout << "beginning data benchmark program\n";
            tester.data_benchmark(drift, vol, init_price);
            break;
        case 3:
            std::cout << "Please enter the percent drift of the stock data: ";
            std::cin >> drift;
            std::cout << "Please enter the percent volatility of the stock data: ";
            std::cin >> vol;
            std::cout << "Please enter the initial price: ";
            std::cin >> init_price;
            std::cout << "beginning data verification benchmark program\n";
            tester.verification_benchmark(drift, vol, init_price);
            break;
        case 4:
            goto end;
            break;
        default:
            std::cout << "Invalid option\n\n";
            break;
        }

    }
end:
    return 0;
}
