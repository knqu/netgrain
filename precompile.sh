#!/bin/bash

source .env

: ${FLAGS=}

FLAGS="$FLAGS -std=c++2c"
FLAGS="$FLAGS -I./common"
FLAGS="$FLAGS -I./lib/crow/"
FLAGS="$FLAGS -I./lib/asio/include"
FLAGS="$FLAGS -Xclang -emit-pch"

root_dir="$( cd -- "$( dirname -- "${BASH_SOURCE[0]:-$0}"; )" &> /dev/null && pwd 2> /dev/null; )";

cd $root_dir

$clang_path -x c++-header ./lib/crow/crow.h $FLAGS -o ./pch/crow.h.pch

