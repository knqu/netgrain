#!/bin/bash

clang++ -Wno-everything ../src/main.cpp ../../../database/connector.cpp ../../../core/src/simulator/*.cpp /Users/danluo/compsci_projects/netgrain/lib/mailio/*.cpp -std=c++20 \
  -I/opt/homebrew/include -I/opt/homebrew/opt/libpq/include -I/Users/danluo/compsci_projects/netgrain/lib \
  -L/opt/homebrew/lib -L/opt/homebrew/opt/libpq/lib -I/Users/danluo/compsci_projects/netgrain/common/ \
  -I/Users/danluo/compsci_projects/netgrain/lib/crow/include \
  -I/Users/danluo/compsci_projects/netgrain/lib/asio/include \
  -I/Users/danluo/compsci_projects/netgrain/common \
  -o my_crow_app -lpqxx -lpq -lssl -lcrypto -lfmt \
  -I/Users/danluo/compsci_projects/netgrain/core/src/simulator/client \
  -DPy_NO_LINK_LIB -I/Users/danluo/compsci_projects/netgrain/lib/pybind_files/include \
  -isystem /Users/danluo/compsci_projects/netgrain/lib/pybind_files/include/pybind11 \
  -isystem /Users/danluo/compsci_projects/netgrain/lib/pybind_files/include/python3.12 \
  -L/Users/danluo/compsci_projects/netgrain/lib/pybind_files/bin/debug -lpython3.12d -lintl
