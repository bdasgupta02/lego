cmake_minimum_required(VERSION 3.14)
project(lego VERSION 1.0.0 LANGUAGES CXX)

add_library(lego INTERFACE)
target_include_directories(lego INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(lego INTERFACE cxx_std_20)

add_executable(lego_example example.cpp)
target_link_libraries(lego_example PRIVATE lego)

set_target_properties(lego_example PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
