#!/bin/bash

clang++ -Wno-everything ../src/main.cpp ../../../database/connector.cpp ../../../core/src/simulator/*.cpp /Users/danluo/Dropbox/netgrain_repos/temp1/netgrain/lib/mailio/*.cpp -std=c++20 \
  -I/opt/homebrew/include -I/opt/homebrew/opt/libpq/include -I/Users/danluo/Dropbox/netgrain_repos/temp1/netgrain/lib \
  -L/opt/homebrew/lib -L/opt/homebrew/opt/libpq/lib -I/Users/danluo/Dropbox/netgrain_repos/temp1/netgrain/common/ \
  -I/Users/danluo/Dropbox/netgrain_repos/temp1/netgrain/lib/crow/include \
  -I/Users/danluo/Dropbox/netgrain_repos/temp1/netgrain/lib/asio/include \
  -I/Users/danluo/Dropbox/netgrain_repos/temp1/netgrain/common \
  -o my_crow_app -lpqxx -lpq -lssl -lcrypto -lfmt \
  -mmacosx-version-min=15.7 \
  -I/Users/danluo/Dropbox/netgrain_repos/temp1/netgrain/core/src/simulator/client \
  -DPy_NO_LINK_LIB -I/Users/danluo/Dropbox/netgrain_repos/temp1/netgrain/lib/pybind_files/include \
  -isystem /Users/danluo/Dropbox/netgrain_repos/temp1/netgrain/lib/pybind_files/include/pybind11 \
  -isystem /Users/danluo/Dropbox/netgrain_repos/temp1/netgrain/lib/pybind_files/include/python3.12 \
  -L/Users/danluo/Dropbox/netgrain_repos/temp1/netgrain/lib/pybind_files/bin/debug -lpython3.12d -lintl
