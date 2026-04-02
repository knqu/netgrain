// this class serves as a struct to send information to a generator
#pragma once

#include <atomic>

#include <crow.h>

struct Data_Transfer {
  std::atomic<crow::websocket::connection *> conn;

  std::atomic<int> rate_per_second;
  std::atomic<bool> gen;
  std::atomic<bool> send_data;
  std::atomic<int> new_event;
  std::atomic<double> n_vol;
  std::atomic<double> n_drift;
  std::atomic<int> n_price; 

  std::atomic<int> threshold; 
};

