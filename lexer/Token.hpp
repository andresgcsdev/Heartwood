#pragma once

#include <string>

// Type of instruction.
enum class TokenType
{
    VAR, MUT, FUNCTION, IF, ELSE, WHILE, FOR, GLOBAL, STRUCT, ENUM,
    IDENTIFIER,
    TYPE_INT, TYPE_FLOAT, TYPE_STR, TYPE_BOOL,
    LITERAL_INT, LITERAL_FLOAT, LITERAL_STR, LITERAL_BOOL,
    ASSIGN, COLON, SEMICOLON, COMMA, DOT, ARROW, // ->
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACK, RBRACK,
    RETURN, BREAK,
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO, POWER,
    EQUALS, NOT_EQUALS, LESS, LESS_EQUALS, GREATER, GREATER_EQUALS,
    OR, AND, XOR, NOT,
    ERROR // No match.
};

// Not validated execution instruction.
struct Token
{
    TokenType type;
    std::string value; // For identifiers or literals.
    int line; // For better error messages.
};
