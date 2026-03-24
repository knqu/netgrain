// this class serves as a struct to send information to a generator
#pragma once

struct dataTransfer {
    int rate_per_second;
    char gen;
    char send_data;
    char new_event;
    double n_vol;
    double n_drift;
    int n_price; 
};