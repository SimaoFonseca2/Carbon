cmake_minimum_required(VERSION 3.27)
project(Carbon)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
add_subdirectory(googletest)
add_executable(Carbon
        src/main.cpp
        src/Token.h
        src/Lexer.hpp
        src/Parser.hpp
        src/AssemblyGen.hpp
        src/Allocator.hpp
        src/Node.hpp
)
add_executable(CarbonTest
        Tests/TestMain.cpp
        Tests/TestLexer.cpp
        Tests/TestParser.cpp
        Tests/TestAssemblyGen.cpp
)
enable_testing()
target_link_libraries(CarbonTest gtest gtest_main)