#include "Error.hpp"
#include <iostream>


void ErrorList::add(const ErrorPhase phase, const std::string &message, const int line, const bool isEoF, const std::string &tips)
{
    auto e = ErrorNode(phase, line, message, tips);
    if (!tips.empty())
    {
        if (isEoF)
            e.tips = "File ended without completing this previous instruction. Here's what's probably missing or wrong: " +
                     e.tips;
        else
            e.tips = "What might be wrong: " + e.tips;
    }

    errors.push_back(e);
}

void ErrorList::raiseAll(const ErrorPhase phase)
{
    std::string masterPhaseStr;
    switch (phase)
    {
        case ErrorPhase::Lexer: masterPhaseStr = "Lexer";
            break;
        case ErrorPhase::Parser: masterPhaseStr = "Parser";
            break;
        case ErrorPhase::Semantic: masterPhaseStr = "Semantic analysis";
            break;
        case ErrorPhase::Runtime: masterPhaseStr = "Runtime";
            break;
    }

    if (errors.empty())
    {
        std::cout << "<<No errors found at "+ masterPhaseStr +" execution.>>" << std::endl;
        return;
    }

    for (const auto &[nodePhase, line, message, tips]: errors)
    {
        std::string phaseStr;
        switch (nodePhase)
        {
            case ErrorPhase::Lexer: phaseStr = "Lexer";
                break;
            case ErrorPhase::Parser: phaseStr = "Parser";
                break;
            case ErrorPhase::Semantic: phaseStr = "Semantic analysis";
                break;
            case ErrorPhase::Runtime: phaseStr = "Runtime";
                break;
        }
        std::cerr << "[" << phaseStr << " Error] on line " << line << ": " << message << std::endl;
        if (!tips.empty())
            std::cerr << tips << std::endl;
    }

    std::exit(1);
}

void ErrorList::raiseThis(const ErrorNode &node)
{
    std::string phaseStr;
    switch (node.phase)
    {
        case ErrorPhase::Lexer: phaseStr = "Lexer";
            break;
        case ErrorPhase::Parser: phaseStr = "Parser";
            break;
        case ErrorPhase::Semantic: phaseStr = "Semantic analysis";
            break;
        case ErrorPhase::Runtime: phaseStr = "Runtime";
            break;
    }
    std::cerr << "[" << phaseStr << " Error] on line " << node.line << ": " << node.message << std::endl;
    if (!node.tips.empty())
        std::cerr << node.tips << std::endl;

    std::exit(1);
}
