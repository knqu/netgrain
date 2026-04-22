#!/bin/bash

clang++ -Wno-everything ../src/main.cpp ../../../database/connector.cpp /Users/danluo/compsci_projects/netgrain/lib/mailio/*.cpp -std=c++20 \
  -I/opt/homebrew/include -I/opt/homebrew/opt/libpq/include -I/Users/danluo/compsci_projects/netgrain/lib \
  -L/opt/homebrew/lib -L/opt/homebrew/opt/libpq/lib -I/Users/danluo/netgrain/common/ \
  -I/Users/danluo/compsci_projects/netgrain/lib/crow/include \
  -I/Users/danluo/compsci_projects/netgrain/lib/asio/include \
  -I/Users/danluo/compsci_projects/netgrain/common \
  -o my_crow_app -lpqxx -lpq -lssl -lcrypto -lfmt
