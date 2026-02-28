#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include "generator.cpp"

int main()
{
    printf("test\n");
    std::random_device rd{};
    std::mt19937 gen{rd()};
    generator gene(0.05, 0.25, 500);

    gene.generate();


    
}
