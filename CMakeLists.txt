cmake_minimum_required(VERSION 3.9)
project(Game_of_Life)

find_package (Threads)

set(CMAKE_CXX_STANDARD 11)

add_executable(Game_of_Life main.cpp)

target_link_libraries (Game_of_Life ${CMAKE_THREAD_LIBS_INIT})