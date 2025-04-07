cmake_minimum_required(VERSION 3.10.0)
project(testUTFOutput VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(testUTFOutput main.cpp Locale.hpp)

