cmake_minimum_required(VERSION 3.20)
project(Karl)

set(CMAKE_CXX_STANDARD 14)

add_executable(Karl main.cpp Position.cpp Position.h Definitions.h MoveGenerator.cpp MoveGenerator.h Cli.cpp Cli.h)
