#pragma once

#include "def.hpp"
#include "queue.hpp"
#include "dataTransfer.cpp"
#include <cmath>
#include <random>
#include <iostream>
#include <fstream>
#include <algorithm> // for std::max

class bullGen {
private:
    double percent_drift;       // The strong upward trend (e.g., 0.20)
    
    // Heston Stochastic Volatility Parameters
    double current_variance;    // Tracks the live volatility squared
    double theta_variance;      // The long-term "quiet" bull market variance
    double kappa;               // How fast volatility reverts to the quiet state
    double xi;                  // "Vol of Vol" - how wildly the volatility itself swings
    double rho;                 // Correlation (usually negative: price goes up, vol goes down)
    
    Queue<double> *dataBuffer;

public:
    // Constructor
    bullGen(double drift, double init_vol, double long_term_vol, double mean_rev_speed, double vol_of_vol, double correlation, double price) {
        this->percent_drift = drift;
        this->current_variance = init_vol * init_vol;
        this->theta_variance = long_term_vol * long_term_vol;
        this->kappa = mean_rev_speed;
        this->xi = vol_of_vol;
        this->rho = correlation;
        
        dataBuffer = new Queue<double>();
        dataBuffer->enqueue(price);
    }

    ~bullGen() { 
        delete dataBuffer; 
    }

// The Math: Heston Model (Stochastic Volatility) Dont ask, math model is based on gemini
    inline double heston_step(double dt, double S_t, std::normal_distribution<double> &norm, std::mt19937 &gen) {
        // 1. Generate independent normal random variables
        double Z1 = norm(gen);
        double Z2 = norm(gen);
        
        // 2. Cache repeated expensive calculations
        double sqrt_dt = std::sqrt(dt);
        
        // 3. Correlate shocks (Leverage effect)
        double dW_price = Z1 * sqrt_dt;
        double dW_vol = (this->rho * Z1 + std::sqrt(1.0 - this->rho * this->rho) * Z2) * sqrt_dt;
        
        // 4. Lord et al. Full Truncation Scheme (Industry Standard)
        // We evaluate equations using a strictly non-negative variance
        double v_plus = std::max(this->current_variance, 0.0);
        
        // Cache the square root so the CPU only calculates it once per cycle
        double sqrt_v_plus = std::sqrt(v_plus); 
        
        // 5. Update the true variance state
        // Notice we update 'current_variance' but used 'v_plus' for the calculation.
        // This allows the mathematical state to briefly dip below zero without throwing NaN errors,
        // letting the mean-reversion (kappa) pull it naturally back into positive territory.
        this->current_variance += this->kappa * (this->theta_variance - v_plus) * dt 
                                + this->xi * sqrt_v_plus * dW_vol;
        
        // 6. Calculate discrete price jump
        double expected_return = (this->percent_drift - 0.5 * v_plus) * dt;
        double random_shock = sqrt_v_plus * dW_price;
        
        return S_t * std::exp(expected_return + random_shock);
    }

    // The Generator Loop
    int gen(dataTransfer *gen_settings) {
        std::random_device rd{};
        std::mt19937 gen{rd()};
        std::normal_distribution<double> norm{0.0, 1.0};
        
        double dt = 0.01; // Time delta

        std::ofstream myfile;
        myfile.open("bull-run.txt");

        int i = 0;
        while (gen_settings->gen) {
            // Generate next point using Heston math and push to queue
            dataBuffer->enqueue(heston_step(dt, dataBuffer->peek(), norm, gen));
            
            // Pop and write to file/stream
            myfile << dataBuffer->dequeue() << "\n";
            i += 1;
        }
        myfile.close();
        return i;
    }
};