cmake_minimum_required(VERSION 4.2)
project(Heartwood)

set(CMAKE_CXX_STANDARD 20)

add_executable(Heartwood
        src/main.cpp
        src/lexer/Lexer.cpp
        src/lexer/Token.hpp
        src/error/Error.cpp
        src/parser/AST.hpp
        src/parser/Parser.cpp)

set(CMAKE_EXE_LINKER_FLAGS "-static")