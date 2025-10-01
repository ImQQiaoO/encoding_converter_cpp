cmake_minimum_required(VERSION 3.10.0)
project(testUTFOutput VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(testUTFOutput main.cpp Locale.hpp)

if (MSVC OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
  target_compile_options(testUTFOutput PRIVATE /utf-8)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(testUTFOutput PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(testUTFOutput PRIVATE -finput-charset=UTF-8)
endif()