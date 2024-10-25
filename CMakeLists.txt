cmake_minimum_required(VERSION 3.29)
project(ztshell)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-lreadline")

add_executable(ztshell main.cpp
        src/exit.cpp
        include/build_in.hpp
        src/cd.cpp
        src/pwd.cpp
        src/ls.cpp
        src/env.cpp
        include/env.hpp
)
