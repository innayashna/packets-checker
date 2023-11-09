cmake_minimum_required(VERSION 3.26)
project(packets_checker)

set(CMAKE_CXX_STANDARD 17)

add_executable(packets_checker main.cpp
        Sender.cpp
        Sender.h
)
