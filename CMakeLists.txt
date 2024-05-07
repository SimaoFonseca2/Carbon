cmake_minimum_required(VERSION 3.27)
project(Carbon)

set(CMAKE_CXX_STANDARD 20)

add_executable(Carbon
        src/main.cpp
        src/Token.h
        src/Lexer.hpp
        src/Parser.hpp
        src/AssemblyGen.hpp
        src/Allocator.hpp
)
