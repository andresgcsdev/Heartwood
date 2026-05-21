#pragma once

#include <vector>

#include "../lexer/Token.hpp"
#include "AST.hpp"

// Converts Tokens into readable instructions for the evaluator to consume.
namespace Parser
{
    // Returns the root of the Abstract Syntax Tree of the given token list.
    // May raise an error if syntax is invalid.
    std::vector<ASTNode> parse(const std::vector<Token> &tks);
}