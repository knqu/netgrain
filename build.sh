#!/bin/bash

source .env

: ${FLAGS=}

OS="`uname`"

if [[ "$OS" != "Darwin" ]]; then # for HX
  FLAGS="$FLAGS -I./lib/usockets/include"
  FLAGS="$FLAGS -I./lib/uwebsockets/include"
  FLAGS="$FLAGS -luSockets"
  FLAGS="$FLAGS -L./lib/usockets/bin/debug"
fi

# FLAGS="$FLAGS ./src/*.cpp"
# FLAGS="$FLAGS ./generated/*.cpp"
# FLAGS="$FLAGS -I./generated"
FLAGS="$FLAGS -std=c++20"
FLAGS="$FLAGS -L./lib/fmtlib/bin"
FLAGS="$FLAGS -lfmt"
FLAGS="$FLAGS -I./lib/fmtlib/include"
FLAGS="$FLAGS -I./common"
FLAGS="$FLAGS -I./lib/crow/include"
FLAGS="$FLAGS -I./lib/asio/include"
FLAGS="$FLAGS ./patch/__hashing.cpp"
FLAGS="$FLAGS -Wno-deprecated-declarations"
FLAGS="$FLAGS -DUWS_NO_ZLIB"
FLAGS="$FLAGS -DASIO_NO_DEPRECATED"
FLAGS="$FLAGS -DASIO_STANDALONE"

SIM_FLAGS="$FLAGS ./core/src/simulator/main.cpp"
SIM_FLAGS="$SIM_FLAGS ./core/src/simulator/historicalData.cpp"
SIM_FLAGS="$SIM_FLAGS -I./core/src/simulator/"

SIM_SERVER_FLAGS="$FLAGS ./core/src/simulator/server/*.cpp"
SIM_SERVER_FLAGS="$SIM_SERVER_FLAGS -I./core/src/simulator/server"

SIM_CLIENT_FLAGS="$FLAGS ./core/src/simulator/client/*.cpp"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -I./core/src/simulator/client"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -DPy_NO_LINK_LIB"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -I./lib/pybind11/include"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -isystem ./lib/pybind11/include/pybind11"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -isystem ./lib/pybind11/include/python3.12"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -mmacosx-version-min=15.7"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -L./lib/pybind11/bin/debug"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -lpython3.12d"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -lintl"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS $sdk_path/usr/lib/libiconv.tbd"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS $sdk_path/usr/lib/libcharset.tbd"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -framework CoreFoundation"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -framework Foundation"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -framework Security"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -DIXWEBSOCKET_USE_SECURE_TRANSPORT"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -DIXWEBSOCKET_USE_TLS"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -DIXWEBSOCKET_USE_ZLIB"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -I./lib/ixwebsocket/include"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -L./lib/ixwebsocket/bin/debug"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -lz"
SIM_CLIENT_FLAGS="$SIM_CLIENT_FLAGS -lixwebsocket"

GEN_FLAGS="$FLAGS ./core/src/generator/main.cpp"
GEN_FLAGS="$GEN_FLAGS -I./core/src/generator/"

DB_FLAGS="$FLAGS ./database/*.cpp"
DB_FLAGS="$DB_FLAGS -L$pqxx_lib"
DB_FLAGS="$DB_FLAGS -L$pq_lib"
DB_FLAGS="$DB_FLAGS -lpqxx -lpq"
DB_FLAGS="$DB_FLAGS -I$libpqxx"

GEN_SERVER_FLAGS="$FLAGS ./core/src/generator/send.cpp"
GEN_SERVER_FLAGS="$GEN_SERVER_FLAGS -I./core/src/generator/"

simulator_compile_cmd () {
    $clang_path -Wall $SIM_FLAGS -g -o out/debug/macos/simulator
}

simulator_server_compile_cmd () {
    $clang_path -Wall $SIM_SERVER_FLAGS -g -o out/debug/macos/sim_server
}

simulator_client_compile_cmd () {
    $clang_path -Wall $SIM_CLIENT_FLAGS -g -o out/debug/macos/sim_client
}

generator_compile_cmd () {
    $clang_path -Wall $GEN_FLAGS -g -o out/debug/macos/generator
}

generator_server_compile_cmd () {
    $clang_path -Wall $GEN_SERVER_FLAGS -g -o out/debug/macos/gen_server
}

db_compile_cmd () {
    $clang_path -Wall $DB_FLAGS -g -o out/debug/macos/db
}

root_dir="$( cd -- "$( dirname -- "${BASH_SOURCE[0]:-$0}"; )" &> /dev/null && pwd 2> /dev/null; )";

cd $root_dir

if [[ $# -eq 0 ]]; then script_arg="run"; else script_arg=$1; fi

if [ $1 = "run" ]; then

    if [ $2 = "sim" ]; then

        mkdir -p out/debug/macos
        simulator_compile_cmd

        if [ $? -eq 0 ]; then
            ./out/debug/macos/simulator
        fi

    elif [ $2 = "gen" ]; then

        mkdir -p out/debug/macos
        generator_compile_cmd


        if [ $? -eq 0 ]; then
            ./out/debug/macos/generator
        fi

    elif [ $2 = "serve" ]; then

        printf "compiling...\n"

        mkdir -p out/debug/macos
        simulator_server_compile_cmd

        printf "running...\n"

        if [ $? -eq 0 ]; then
            ./out/debug/macos/sim_server
        fi

    elif [ $2 = "client" ]; then

        printf "compiling...\n"

        mkdir -p out/debug/macos
        simulator_client_compile_cmd

        printf "running...\n"

        if [ $? -eq 0 ]; then
            ./out/debug/macos/sim_client
        fi

    elif [ $2 = "gen_serve" ]; then

        printf "compiling...\n"

        mkdir -p out/debug/macos
        generator_server_compile_cmd

        printf "running...\n"

        if [ $? -eq 0 ]; then
            ./out/debug/macos/gen_server
        fi

    elif [ $2 = "db" ]; then

        mkdir -p out/debug/macos
        db_compile_cmd

        if [ $? -eq 0 ]; then
            ./out/debug/macos/db
        fi

    fi

elif [ $1 = "build" ]; then

    if [ $2 = "sim" ]; then

        mkdir -p out/debug/macos
        simulator_compile_cmd

    elif [ $2 = "gen" ]; then

        mkdir -p out/debug/macos
        generator_compile_cmd

    elif [ $2 = "serve" ]; then

        mkdir -p out/debug/macos
        simulator_server_compile_cmd

    elif [ $2 = "client" ]; then

        mkdir -p out/debug/macos
        simulator_client_compile_cmd

    elif [ $2 = "gen_serve" ]; then

        mkdir -p out/debug/macos
        generator_server_compile_cmd

    elif [ $2 = "db" ]; then

        mkdir -p out/debug/macos
        db_compile_cmd

    fi

elif [ $1 = "clean" ]; then

    rm -rf out
    mkdir -p out/debug/macos

else

    echo "Unknown build option"

fi

