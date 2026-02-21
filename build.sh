#!/bin/bash

source .env

: ${FLAGS=}

# FLAGS="$FLAGS ./src/*.cpp"
# FLAGS="$FLAGS ./common/*.cpp"
# FLAGS="$FLAGS ./generated/*.cpp"
# FLAGS="$FLAGS -I./generated"
# FLAGS="$FLAGS -I./common"
FLAGS="$FLAGS -std=c++2c"

SIM_FLAGS="$FLAGS ./core/src/simulator/*.cpp"
SIM_FLAGS="$SIM_FLAGS -I./core/src/simulator/"

GEN_FLAGS="$FLAGS ./core/src/generator/*.cpp"
GEN_FLAGS="$GEN_FLAGS -I./core/src/generator/"

simulator_compile_cmd () {
    $clang_path -Wall $SIM_FLAGS -g -o out/debug/macos/simulator
}

generator_compile_cmd () {
    $clang_path -Wall $GEN_FLAGS -g -o out/debug/macos/generator
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

    fi

elif [ $1 = "build" ]; then

    if [ $2 = "sim" ]; then

        mkdir -p out/debug/macos
        simulator_compile_cmd

    elif [ $2 = "gen" ]; then

        mkdir -p out/debug/macos
        generator_compile_cmd

    fi

elif [ $1 = "clean" ]; then

    rm -rf out
    mkdir -p out/debug/macos

else
    
    echo "Unknown build option"

fi

