#pragma once

#include <string>

// Type of instruction.
enum class TokenType
{
    BIGGER, // Signal that this token can be part of a bigger one.
    VAR, MUT, FUNCTION, IF, ELSE, WHILE, FOR, GLOBAL, STRUCT, ENUM,
    IDENTIFIER,
    TYPE_INT, TYPE_FLOAT, TYPE_STR, TYPE_BOOL, ARRAY,
    LITERAL_INT, LITERAL_FLOAT, LITERAL_STR, LITERAL_BOOL,
    ASSIGN, COLON, SEMICOLON, COMMA, ARROW/* -> */, QUOTE,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACK, RBRACK,
    RETURN, BREAK,
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO,
    EQUALS, NOT_EQUALS, LESS, LESS_EQUALS, GREATER, GREATER_EQUALS,
    OR, AND, XOR, NOT,
    SPACE,
    COMMENT,
    ERROR // No match.
};

// Not validated execution instruction.
struct Token
{
    TokenType type;
    std::string value; // For identifiers or literals.
    int line; // For better error messages.
};

// Basic generalization functions:

inline bool isLiteral(const TokenType &t)
{
    return t == TokenType::LITERAL_INT || t == TokenType::LITERAL_FLOAT || t == TokenType::LITERAL_STR ||
           t == TokenType::LITERAL_BOOL;
}

inline bool isOperator(const TokenType &t)
{
    return t == TokenType::PLUS || t == TokenType::MINUS || t == TokenType::MULTIPLY ||
           t == TokenType::DIVIDE || t == TokenType::MODULO || t == TokenType::EQUALS ||
           t == TokenType::NOT_EQUALS || t == TokenType::LESS || t == TokenType::LESS_EQUALS ||
           t == TokenType::GREATER || t == TokenType::GREATER_EQUALS || t == TokenType::OR ||
           t == TokenType::AND || t == TokenType::XOR || t == TokenType::NOT;
}

inline bool isVarType(const TokenType &t)
{
    return t == TokenType::TYPE_INT || t == TokenType::TYPE_FLOAT || t == TokenType::TYPE_STR ||
           t == TokenType::TYPE_BOOL || t == TokenType::ARRAY;
}
