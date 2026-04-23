#!/bin/bash

clang++ ../src/main.cpp ../../../database/connector.cpp C:/Users/james/netgrain/lib/mailio/*.cpp -std=c++20 \
  -I/opt/homebrew/include -I/opt/homebrew/opt/libpq/include -IC:/Users/james/netgrain/lib \
  -L/opt/homebrew/lib -L/opt/homebrew/opt/libpq/lib -IC:/Users/james/netgrain/common/ \
  -o my_crow_app -lpqxx -lpq -lssl -lcrypto -lfmt
