cmake_minimum_required(VERSION 3.14)
project(webserver)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        10.2.1  # use a tag instead of a commit hash
)
FetchContent_MakeAvailable(fmt)
add_executable(my_app src/main.cpp)
target_include_directories(my_app PRIVATE "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/include")
target_link_libraries(my_app PRIVATE fmt::fmt)