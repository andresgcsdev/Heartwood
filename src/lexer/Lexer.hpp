#pragma once
#include "Token.hpp"
#include "../error/Error.hpp"

#include <vector>

// Converts text code into tokens.
namespace Lexer
{
    // Converts a .hw file at `filepath` to a Token vector to be consumed by Parser.
    // All found errors are added to the end of the given ErrorList reference.
    std::vector<Token> tokenize(const std::string& filepath, ErrorList &errors);
}