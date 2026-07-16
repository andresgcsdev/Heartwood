#pragma once
#include <string>

// Simple code Block for Error Raising.
namespace Error
    {
        // Determine which phase of the interpreter the error has occurred.
        enum class Phase
        {
            Lexer,
            Parser,
            Semantic,
            Runtime
        };

        // Prints out the error and stops execution.
        [[noreturn]] void raise(Phase phase, const std::string &message, int line, const std::string &tips = "");
    }
