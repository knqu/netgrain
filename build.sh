#!/bin/bash

source .env

: ${FLAGS=}

# FLAGS="$FLAGS ./src/*.cpp"
# FLAGS="$FLAGS ./generated/*.cpp"
# FLAGS="$FLAGS -I./generated"
FLAGS="$FLAGS -std=c++2c"
FLAGS="$FLAGS -L./lib/usockets/bin/debug"
FLAGS="$FLAGS -luSockets"
FLAGS="$FLAGS -L./lib/fmtlib/bin"
FLAGS="$FLAGS -lfmt"
FLAGS="$FLAGS -I./lib/fmtlib/include"
FLAGS="$FLAGS -I./common"
FLAGS="$FLAGS -I./lib/usockets/include"
FLAGS="$FLAGS -I./lib/uwebsockets/include"
FLAGS="$FLAGS -I./lib/crow/include"
FLAGS="$FLAGS -I./lib/asio/include"
FLAGS="$FLAGS ./patch/__hashing.cpp"
FLAGS="$FLAGS -Wno-deprecated-declarations"
FLAGS="$FLAGS -DUWS_NO_ZLIB"
FLAGS="$FLAGS -DASIO_NO_DEPRECATED"
FLAGS="$FLAGS -DASIO_STANDALONE"

SIM_FLAGS="$FLAGS ./core/src/simulator/*.cpp"
SIM_FLAGS="$SIM_FLAGS -I./core/src/simulator/"

GEN_FLAGS="$FLAGS ./core/src/generator/*.cpp"
GEN_FLAGS="$GEN_FLAGS -I./core/src/generator/"

DB_FLAGS="$FLAGS ./database/*.cpp"
DB_FLAGS="$DB_FLAGS -L$pqxx_lib"
DB_FLAGS="$DB_FLAGS -L$pq_lib"
DB_FLAGS="$DB_FLAGS -lpqxx -lpq"
DB_FLAGS="$DB_FLAGS -I$libpqxx"

simulator_compile_cmd () {
    $clang_path -Wall $SIM_FLAGS -g -o out/debug/macos/simulator
}

generator_compile_cmd () {
    $clang_path -Wall $GEN_FLAGS -g -o out/debug/macos/generator
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
