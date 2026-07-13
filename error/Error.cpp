#include "Error.hpp"
#include <iostream>

void Error::raise(const Phase phase, const std::string &message, const int line, const std::string &tips)
{
    std::string phaseStr;
    switch (phase)
    {
        case Phase::Lexer: phaseStr = "Lexer";
            break;
        case Phase::Parser: phaseStr = "Parser";
            break;
        case Phase::Semantic: phaseStr = "Semantic";
            break;
        case Phase::Runtime: phaseStr = "Runtime";
            break;
    }
    std::cerr << "[" << phaseStr << " Error] on line " << line << ": " << message << std::endl;
    if (!tips.empty())
        std::cerr << tips << std::endl;

    std::exit(1);
}
