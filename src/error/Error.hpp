#pragma once
#include <string>
#include <vector>

#include "../lexer/Token.hpp"

// Determine which phase of the interpreter the error has occurred.
enum class ErrorPhase
{
    Lexer,
    Parser,
    Semantic,
    Runtime
};

// Struct to store error data.
struct ErrorNode
{
    ErrorPhase phase;
    Token token;
    std::string message;
    std::string tips;
};

// Wrapper class for an ErrorNode vector.
class ErrorList
{
public:
    // Appends the found error at the end of the list.
    void add(const ErrorPhase &phase, const std::string &message, const Token &token, const std::string &tips = "");

    // Prints out in the terminal all of the current errors in the list.
    // First-in First-out order.
    void raise(const ErrorPhase &phase);

private:
    std::vector<ErrorNode> errors;
};
