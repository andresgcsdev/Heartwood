#include "Error.hpp"
#include <iostream>

namespace
{
    std::string phaseToString(const ErrorPhase phase)
    {
        switch (phase)
        {
            case ErrorPhase::Lexer:    return "Lexer";
            case ErrorPhase::Parser:   return "Parser";
            case ErrorPhase::Semantic: return "Semantic analysis";
            case ErrorPhase::Runtime:  return "Runtime";
        }
        return "Unknown";
    }

    // Returns true when the given index is a valid position inside tokens.
    bool isValidIndex(const int index, const std::vector<Token> &tokens)
    {
        return index >= 0 && index < static_cast<int>(tokens.size());
    }

    // Builds the "location tag" like "line 12, col 5-14"
    // or "line 12, col 5" when only one column is known.
    // Falls back to the node's raw line if token info is not usable.
    std::string formatLocation(const ErrorNode &node, const std::vector<Token> &tokens)
    {
        // No usable range: use the raw line stored in the node.
        if (!isValidIndex(node.startIndex, tokens))
        {
            if (node.line < 0)
                return "at unknown location";

            return "on line " + std::to_string(node.line);
        }

        const Token &start = tokens.at(node.startIndex);
        const int startLine = start.line;
        const int startCol  = start.col.start;

        // Same-token or missing endIndex: single-token span.
        if (!isValidIndex(node.endIndex, tokens) || node.endIndex == node.startIndex)
        {
            const int endCol = start.col.end;

            if (startCol == endCol)
            {
                return "on line " + std::to_string(startLine) +
                       ", col " + std::to_string(startCol);
            }

            return "on line " + std::to_string(startLine) +
                   ", col " + std::to_string(startCol) +
                   "-" + std::to_string(endCol);
        }

        const Token &end = tokens.at(node.endIndex);
        const int endLine = end.line;
        const int endCol  = end.col.end;

        // Range that stays inside a single line.
        if (startLine == endLine)
        {
            return "on line " + std::to_string(startLine) +
                   ", col " + std::to_string(startCol) +
                   "-" + std::to_string(endCol);
        }

        // Multi-line range.
        return "from line " + std::to_string(startLine) +
               ", col " + std::to_string(startCol) +
               " to line " + std::to_string(endLine) +
               ", col " + std::to_string(endCol);
    }

    // Builds the "code excerpt" showing the tokens that belong to this error.
    // Returns an empty string if no valid excerpt can be built.
    std::string formatExcerpt(const ErrorNode &node, const std::vector<Token> &tokens)
    {
        if (!isValidIndex(node.startIndex, tokens))
            return "";

        const int endIdx = isValidIndex(node.endIndex, tokens)
                               ? node.endIndex
                               : node.startIndex;

        std::string excerpt;
        int lastLine = tokens.at(node.startIndex).line;

        for (int i = node.startIndex; i <= endIdx; ++i)
        {
            const Token &t = tokens.at(i);

            if (t.line != lastLine)
            {
                excerpt += "\n    ";
                lastLine = t.line;
            }
            else if (!excerpt.empty())
            {
                excerpt += ' ';
            }
            else
            {
                excerpt += " ";
            }

            excerpt += t.value;
        }

        return excerpt;
    }
}

void ErrorList::add(ErrorNode error, const bool isEoF)
{
    if (!error.tips.empty())
    {
        if (isEoF)
            error.tips = "File ended without completing this previous instruction. Here's what's probably missing or wrong: " +
                     error.tips;
        else
            error.tips = "What might be wrong: " + error.tips;
    }

    errors.push_back(error);
}

void ErrorList::raiseAll(const ErrorPhase phase, const std::vector<Token> &tokens) const
{
    std::string masterPhaseStr = phaseToString(phase);

    if (errors.empty())
    {
        std::cout << "<<No errors found at "+ masterPhaseStr +" execution.>>" << std::endl;
        return;
    }

    for (const auto &err: errors)
    {
        std::string phaseStr = phaseToString(err.phase);
        std::cerr << "[" << phaseStr << " Error] " << formatLocation(err, tokens) << ": " << std::endl;
        std::cerr << "Code -- \"" << formatExcerpt(err, tokens) << " \"" << std::endl;
        std::cerr << err.message << std::endl;

        if (!err.tips.empty())
            std::cerr << err.tips << std::endl;

        std::cerr << std::endl;
    }

    std::exit(1);
}

void ErrorList::raiseThis(const ErrorNode &node)
{
    const std::string phaseStr = phaseToString(node.phase);
    std::cerr << "[" << phaseStr << " Error] on line " << std::to_string(node.line) << ": " << node.message << std::endl;
    if (!node.tips.empty())
        std::cerr << node.tips << std::endl;

    std::exit(1);
}
