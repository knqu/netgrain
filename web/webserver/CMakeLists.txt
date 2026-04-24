cmake_minimum_required(VERSION 3.14)

# Auto-include vcpkg toolchain if not already set
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

project(webserver)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/bigobj)
endif()

include(FetchContent)

# fmt via FetchContent
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        10.2.1
)
FetchContent_MakeAvailable(fmt)

set(PYBIND11_FINDPYTHON ON)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG        v2.12.0
)
FetchContent_MakeAvailable(pybind11)


set(NETGRAIN_SIMULATOR_SOURCES
  ../../core/src/simulator/simulator.cpp
  ../../core/src/simulator/historicalData.cpp
  ../../core/src/simulator/broker.cpp
  ../../core/src/simulator/python_strategy.cpp
  ../../core/src/simulator/python_broker_module.cpp
)

add_executable(my_app src/main.cpp ${NETGRAIN_SIMULATOR_SOURCES})
target_compile_definitions(my_app PRIVATE CROW_ENABLE_SSL)

target_include_directories(my_app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../core/src/simulator
  ${CMAKE_CURRENT_SOURCE_DIR}/../../core/src/generator
)

# Link fmt
target_link_libraries(my_app PRIVATE fmt::fmt)

target_link_libraries(my_app PRIVATE pybind11::embed)

find_package(Crow CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE Crow::Crow)

find_package(libpqxx CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE libpqxx::pqxx)

# ---- Mailio manual setup ----
# Adjust the vcpkg triplet and path if needed (x64-windows here)
set(CMAKE_MAP_IMPORTED_CONFIG_DEBUG Debug)
set(CMAKE_MAP_IMPORTED_CONFIG_RELEASE Release)
set(CMAKE_DEBUG_POSTFIX d)
find_package(mailio CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE mailio::mailio)
