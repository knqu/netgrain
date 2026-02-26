// Things to bring up, with this class, do we want this to be multithreaded,
// if it is, is locking going to be needed, and should we make out own queue class

// this is based on drifts, but with our events, we might need to be able to model jumps as well

#include "def.hpp"
#include "queue.hpp"
#include <stdlib.h>

class generator {
private:
    double percent_drift;
    double percent_volatility;

    // queue for streaming data
    Queue dataBuffer = Queue<u32>();




public:
    // constructor and destructor
    generator(double drift, double volatility, int price) {
        percent_drift = drift;
        percent_volatility = volatility;
        dataBuffer = new Queue();
        dataBuffer.enqueue(cur_price);
    }

    ~generator() {
        delete[] dataBuffer;
    }

    void get_event(double new_drift, double new_vol, int new_price) {
        percent_drift = drift;
        percent_volatility = new_vol;
        dataBuffer.enqueue(new_preice);
    }

    int sendPrice() {
        return databuffer.dequeue();
    }

    

    

    /* 
     * Basic data generation function,
     * takes, current price
     */
    void generate_new_data_point() {
        u32 new_data = databuffer.peek();
        short multiplier = 1;
        if (rand() % 10 == 0) {
            multiplier = -1;
        }
        new_data = new_data + multiplier * (rand() % (u32) (percent_drift * 100))
        databuffer.enqueue(new_data);
    }


    // TODO: create loop function to keep generating data, (throttling output at some point)
    // also have this function recieve signals from the simulator so that it can stop if needed
}
// TODO: implement data generation with stochastic diff eq
// dS_t = vol*S_t dt + drift S_t dW_t-
// solves to S_t = S_0 e^(( drift - vol^2/2)t + vol*W_t)



// Need: W_t function