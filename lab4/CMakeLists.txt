cmake_minimum_required(VERSION 3.11)

project(ParallelProgramming CXX)

set(CMAKE_CXX_STANDARD 23)

add_executable(server "server.cpp")
add_executable(client "client.cpp")