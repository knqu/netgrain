#pragma once

#include "def.hpp"
#include "queue.hpp"
#include "dataTransfer.cpp"
#include <cmath>
#include <random>
#include <iostream>
#include <fstream>

class bearGen {
private:
    double percent_drift;      // standard drift (usually negative)
    double percent_volatility; // standard vol (usually high)
    
    // Merton's Jump param
    double jump_frequency;     // lambda: expected jumps per period
    double jump_mean;          // average jump size (severity of the crash)
    double jump_volatility;    // variance of the crash size
    
    Queue<double> *dataBuffer;

public:
    bearGen(double drift, double vol, double j_freq, double j_mean, double j_vol, double price) {
        this->percent_drift = drift;
        this->percent_volatility = vol;
        this->jump_frequency = j_freq;
        this->jump_mean = j_mean;
        this->jump_volatility = j_vol;
        dataBuffer = new Queue<double>();
        dataBuffer->enqueue(price);
    }

    ~bearGen() { 
        delete dataBuffer; 
    }

    double merton_jump(double dt, double S_t, std::normal_distribution<double> &norm, std::poisson_distribution<int> &poisson, std::normal_distribution<double> &jump_norm, std::mt19937 &gen) {
        double gbm_part = (this->percent_drift - 0.5 * pow(this->percent_volatility, 2)) * dt + this->percent_volatility * sqrt(dt) * norm(gen);
        
        int num_jumps = poisson(gen);
        double jump_part = 0.0;
        
        for (int i = 0; i < num_jumps; i++) {
            jump_part += jump_norm(gen);
        }
        return S_t * exp(gbm_part + jump_part);
    }

    int gen(dataTransfer *gen_settings) {
        std::random_device rd{};
        std::mt19937 gen{rd()};
        std::normal_distribution<double> norm{0.0, 1.0};
        
        double dt = 0.01;
        
        // Poisson distribution calculates the exact probability of a jump happening today
        std::poisson_distribution<int> poisson(this->jump_frequency * dt);
        std::normal_distribution<double> jump_norm{this->jump_mean, this->jump_volatility};

        std::ofstream myfile;
        myfile.open("bear-run.txt");

        int i = 0;
        while (gen_settings->gen) {
            dataBuffer->enqueue(merton_jump(dt, dataBuffer->peek(), norm, poisson, jump_norm, gen));            
            myfile << dataBuffer->dequeue() << "\n";
            i += 1;
        }
        myfile.close();
        return i;
    }
};