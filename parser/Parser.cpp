#include "Parser.hpp"

#include <stack>

#include "../error/Error.hpp"

namespace
    {
        std::vector<Token> tokens;
        int current = 0;

        std::stack<Token> curlyBraceStack;

        Token peek() { return tokens[current]; }
        Token consume() { return tokens[current++]; }
        bool isEnd() { return current >= tokens.size(); }

        // Function used to convert a line of tokens into a chain of AST nodes.
        // May raise an error if unexpected chain of tokens is found.
        void matchInstruction(const std::vector<Token> &instructionLine, std::vector<AST::Node> &ast)
        {
            if (instructionLine.empty())
                return;

            if (instructionLine.at(0).type == TokenType::VAR)
            {
                // Variable declaration.
            } else if (instructionLine.at(0).type == TokenType::IF)
            {
                // If statement.
            }
        }
    }

std::vector<AST::Node> Parser::parse(const std::vector<Token> &tks)
{
    tokens = tks;
    current = 0;
    std::vector<AST::Node> ast;
    // Sub vector to copy each instruction at each separation (e.g. semicolon, lbrace).
    std::vector<Token> instructionLine;
    while (!isEnd())
    {
        // Loop to get sub vectors of instructions.
        // Clears sub vector once finds ';', '{' or '}'.

        const Token currentToken = consume();
        // Keep adding elements until next breaker.
        if (currentToken.type != TokenType::SEMICOLON && currentToken.type != TokenType::LBRACE && currentToken.type !=
            TokenType::RBRACE)
            instructionLine.push_back(currentToken);
            // Found a breaker.
        else if (!instructionLine.empty())
        {
            // Only allow left braces on certain statements.
            if (currentToken.type == TokenType::LBRACE)
            {
                if (instructionLine.at(0).type == TokenType::IF || instructionLine.at(0).type == TokenType::ELSE ||
                    instructionLine.at(0).type == TokenType::WHILE || instructionLine.at(0).type == TokenType::FOR ||
                    instructionLine.at(0).type == TokenType::FUNCTION || instructionLine.at(0).type == TokenType::STRUCT
                    ||
                    instructionLine.at(0).type == TokenType::ENUM || instructionLine.at(0).type == TokenType::GLOBAL)
                {
                    instructionLine.push_back(currentToken);
                    curlyBraceStack.push(currentToken);
                    matchInstruction(instructionLine, ast);
                    instructionLine.clear();
                } else
                {
                    Error::raise(Error::Phase::Parser, " Unexpected curly braces, invalid scope starter.",
                                 currentToken.line);
                }
            }
            // Raise error if right braces has an unfinished instruction before it.
            else if (currentToken.type == TokenType::RBRACE)
                Error::raise(Error::Phase::Parser, " Unexpected block closing, previous instruction not finished.",
                             currentToken.line);
                // Always allow semicolons.
            else if (currentToken.type == TokenType::SEMICOLON)
            {
                instructionLine.push_back(currentToken);
                matchInstruction(instructionLine, ast);
                instructionLine.clear();
            }
        } else
        {
            // Raise error for no scope starter.
            if (currentToken.type == TokenType::LBRACE)
                Error::raise(Error::Phase::Parser, " Unexpected curly braces, no scope declaration.",
                             currentToken.line);
                // Right braces are only valid as standalone tokens.
            else if (currentToken.type == TokenType::RBRACE)
            {
                if (!curlyBraceStack.empty())
                {
                    instructionLine.push_back(currentToken);
                    matchInstruction(instructionLine, ast);
                    instructionLine.clear();
                    curlyBraceStack.pop();
                } else
                {
                    Error::raise(Error::Phase::Parser, " Unexpected curly braces, no previous scope defined.",
                                 currentToken.line);
                }
            }
            // Always allow semicolons.
            else if (currentToken.type == TokenType::SEMICOLON)
            {
                instructionLine.push_back(currentToken);
                matchInstruction(instructionLine, ast);
                instructionLine.clear();
            }
        }
    }
    // Unclosed curly brace.
    if (!curlyBraceStack.empty())
        Error::raise(Error::Phase::Parser, " Unclosed scope.", curlyBraceStack.top().line);

    return ast;
}
