cmake_minimum_required(VERSION 3.20)
project(Karl)

# add compiler optimization flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(CMAKE_CXX_STANDARD 20)

add_executable(Karl main.cpp Position.cpp Position.h Defs.h MoveGen.cpp MoveGen.h Cli.cpp Cli.h Magics.cpp Magics.h Search.cpp Search.h Eval.h Moves.h Notation.cpp Notation.h Zobrist.cpp Zobrist.h
        Eval.cpp)
