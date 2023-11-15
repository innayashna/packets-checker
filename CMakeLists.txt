cmake_minimum_required(VERSION 3.26)
project(packets_checker)

set(CMAKE_CXX_STANDARD 17)

add_executable(packets_checker src/main.cpp
        lib/Sender.cpp
        include/Sender.h
        lib/Receiver.cpp
        include/Receiver.h
        include/Checksum.h
        lib/Checksum.cpp
        lib/Proxy.cpp
        include/Proxy.h
        lib/SocketConfigurator.cpp
        include/SocketConfigurator.h
)