cmake_minimum_required(VERSION 3.9)
project(bmatching)
find_package (Threads)

set(CMAKE_CXX_STANDARD 17)
SET(GCC_COVERAGE_COMPILE_FLAGS "-O2")

add_executable(bmatching blimit.hpp thread_pool.h b-matching-con.cpp b-matching.h main.cpp min_heap.cpp min_heap.h)

target_link_libraries (bmatching  ${CMAKE_THREAD_LIBS_INIT})
