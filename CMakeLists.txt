cmake_minimum_required(VERSION 4.2)
project(Heartwood)

set(CMAKE_CXX_STANDARD 20)

add_executable(Heartwood
        main.cpp
        lexer/Lexer.cpp
        lexer/Token.hpp
        error/Error.cpp
        parser/AST.hpp
        parser/Parser.cpp)

set(CMAKE_EXE_LINKER_FLAGS "-static")