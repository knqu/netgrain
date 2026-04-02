cmake_minimum_required(VERSION 3.14)

# Auto-include vcpkg toolchain if not already set
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

project(webserver)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CROW_ENABLE_SSL ON)


include(FetchContent)

# fmt via FetchContent
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        10.2.1
)
FetchContent_MakeAvailable(fmt)

find_package(OpenSSL REQUIRED)

add_executable(my_app src/main.cpp)
target_compile_definitions(my_app PRIVATE CROW_ENABLE_SSL)

target_link_libraries(my_app 
  PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Link fmt
target_link_libraries(my_app PRIVATE fmt::fmt)

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