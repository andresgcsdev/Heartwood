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
    int line;
    std::string message;
    std::string tips;
};

// Wrapper class for an ErrorNode vector.
class ErrorList
{
public:
    // Appends the found error at the end of the list.
    void add(ErrorPhase phase, const std::string &message, int line, bool isEoF = false, const std::string &tips = "");

    // Prints out in the terminal all of the current errors in the list.
    // First-in First-out order.
    // Stops program execution if any errors are found in the list.
    void raiseAll(ErrorPhase phase);

    // Prints out in the terminal the given error.
    // Stops program execution.
    static void raiseThis(const ErrorNode &node);

private:
    std::vector<ErrorNode> errors;
};
