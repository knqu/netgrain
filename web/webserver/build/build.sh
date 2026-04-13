#!/bin/bash

clang++ ../src/main.cpp ../../../database/connector.cpp /Users/hyen/netgrain/lib/mailio/*.cpp -std=c++20 \
  -I/opt/homebrew/include -I/opt/homebrew/opt/libpq/include -I/Users/hyen/netgrain/lib \
  -L/opt/homebrew/lib -L/opt/homebrew/opt/libpq/lib -I/Users/hyen/netgrain/common/ \
  -o my_crow_app -lpqxx -lpq -lssl -lcrypto -lfmt
