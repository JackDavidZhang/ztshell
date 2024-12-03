cmake_minimum_required(VERSION 3.29)
project(ztshell)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-lreadline")

add_executable(ztshell main.cpp
        src/exit.cpp
        include/build_in.hpp
        src/cd.cpp
        src/pwd.cpp
        src/env.cpp
        include/env.hpp
        src/export.cpp
        src/where.cpp
)

add_executable(echo src/echo.cpp)
add_executable(ls src/ls.cpp)
add_executable(cat src/cat.cpp)
