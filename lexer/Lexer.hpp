#pragma once
#include "Token.hpp"
#include <vector>

// Converts text code into tokens.
namespace Lexer
{
    // Converts .hw file at filepath to a Token vector to be consumed by Parser.
    std::vector<Token> tokenize(const std::string& filepath);
}