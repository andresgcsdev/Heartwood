#include <iostream>
#include <string>
#include <vector>

#include "lexer/Lexer.hpp"
#include "lexer/Token.hpp"
#include "parser/AST.hpp"
#include "parser/Parser.hpp"

// Helper functions for printing Structs.
namespace
    {
        std::string tokenTypeToString(TokenType type)
        {
            switch (type)
            {
                case TokenType::BIGGER: return "BIGGER";
                case TokenType::VAR: return "VAR";
                case TokenType::MUT: return "MUT";
                case TokenType::FUNCTION: return "FUNCTION";
                case TokenType::IF: return "IF";
                case TokenType::ELSE: return "ELSE";
                case TokenType::WHILE: return "WHILE";
                case TokenType::FOR: return "FOR";
                case TokenType::GLOBAL: return "GLOBAL";
                case TokenType::STRUCT: return "STRUCT";
                case TokenType::ENUM: return "ENUM";
                case TokenType::IDENTIFIER: return "IDENTIFIER";
                case TokenType::TYPE_INT: return "TYPE_INT";
                case TokenType::TYPE_FLOAT: return "TYPE_FLOAT";
                case TokenType::TYPE_STR: return "TYPE_STR";
                case TokenType::TYPE_BOOL: return "TYPE_BOOL";
                case TokenType::LITERAL_INT: return "LITERAL_INT";
                case TokenType::LITERAL_FLOAT: return "LITERAL_FLOAT";
                case TokenType::LITERAL_STR: return "LITERAL_STR";
                case TokenType::LITERAL_BOOL: return "LITERAL_BOOL";
                case TokenType::ASSIGN: return "ASSIGN";
                case TokenType::COLON: return "COLON";
                case TokenType::SEMICOLON: return "SEMICOLON";
                case TokenType::COMMA: return "COMMA";
                case TokenType::ARROW: return "ARROW";
                case TokenType::QUOTE: return "QUOTE";
                case TokenType::LPAREN: return "LPAREN";
                case TokenType::RPAREN: return "RPAREN";
                case TokenType::LBRACE: return "LBRACE";
                case TokenType::RBRACE: return "RBRACE";
                case TokenType::LBRACK: return "LBRACK";
                case TokenType::RBRACK: return "RBRACK";
                case TokenType::RETURN: return "RETURN";
                case TokenType::BREAK: return "BREAK";
                case TokenType::PLUS: return "PLUS";
                case TokenType::MINUS: return "MINUS";
                case TokenType::MULTIPLY: return "MULTIPLY";
                case TokenType::DIVIDE: return "DIVIDE";
                case TokenType::MODULO: return "MODULO";
                case TokenType::EQUALS: return "EQUALS";
                case TokenType::NOT_EQUALS: return "NOT_EQUALS";
                case TokenType::LESS: return "LESS";
                case TokenType::LESS_EQUALS: return "LESS_EQUALS";
                case TokenType::GREATER: return "GREATER";
                case TokenType::GREATER_EQUALS: return "GREATER_EQUALS";
                case TokenType::OR: return "OR";
                case TokenType::AND: return "AND";
                case TokenType::XOR: return "XOR";
                case TokenType::NOT: return "NOT";
                case TokenType::SPACE: return "SPACE";
                case TokenType::ERROR: return "ERROR";
                case TokenType::DOT: return "DOT";
                default: return "UNKNOWN";
            }
        }
    }

int main(int argc, char *argv[])
{
    // if (argc < 2)
    // {
    //     std::cerr << "Usage: heartwood <file.hw>\n";
    //     return 1;
    // }
    const std::vector<Token> tokens = Lexer::tokenize("test.hw");
    for (const auto &token: tokens)
    {
        std::cout << tokenTypeToString(token.type) << std::endl;
    }
}
