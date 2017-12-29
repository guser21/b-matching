cmake_minimum_required(VERSION 3.9)
project(bmatching)
find_package (Threads)

set(CMAKE_CXX_STANDARD 14)

add_executable(bmatching blimit.hpp
#        ThreadPool.h
        thread_pool.h
        b-matching-con.cpp b-matching.h main.cpp min_heap.cpp min_heap.h)
#add_executable(bmatching th_pool.cpp)
target_link_libraries (bmatching  ${CMAKE_THREAD_LIBS_INIT})
#target_link_libraries (bmatching_  ${CMAKE_THREAD_LIBS_INIT})

#add_executable(bmatching_exp exp.cpp)
#target_link_libraries (bmatching_exp ${CMAKE_THREAD_LIBS_INIT})
