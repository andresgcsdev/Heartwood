#include "Lexer.hpp"
#include "../error/Error.hpp"
#include <fstream>
#include <cctype>

namespace
{
    // Helper for matching Tokens that are longer than a single character.
    Token matchLargeToken(const std::string &token, const int line)
    {
        // Parsing for String Literals.
        if (token[0] == '"' && token[token.length() - 1] == '"')
        {
            return Token(TokenType::LITERAL_STR, token, line);
        }
        // Parsing for number literals (int or float).
        // Starting an expression or identifier with a number will result in an error.
        if (isdigit(token[0]))
        {
            bool dotFound = false;
            for (const auto &c: token)
            {
                if (c == '.' && !dotFound)
                    dotFound = true;
                else if (!isdigit(c))
                    return Token(TokenType::ERROR, token, line);
            }
            if (!dotFound)
                return Token(TokenType::LITERAL_INT, token, line);

            return Token(TokenType::LITERAL_FLOAT, token, line);
        }
        // Parsing for keywords or Identifiers
        // Using special characters for identifiers or keywords will result in an error.
        if (isalpha(token[0]))
        {
            for (const auto &c: token)
            {
                if (!isalnum(c))
                    return Token(TokenType::ERROR, token, line);
            }
            if (token == "if")
                return Token(TokenType::IF, token, line);
            if (token == "else")
                return Token(TokenType::ELSE, token, line);
            if (token == "while")
                return Token(TokenType::WHILE, token, line);
            if (token == "for")
                return Token(TokenType::FOR, token, line);
            if (token == "break")
                return Token(TokenType::BREAK, token, line);
            if (token == "return")
                return Token(TokenType::RETURN, token, line);

            if (token == "var")
                return Token(TokenType::VAR, token, line);
            if (token == "mut")
                return Token(TokenType::MUT, token, line);
            if (token == "fn")
                return Token(TokenType::FUNCTION, token, line);
            if (token == "global")
                return Token(TokenType::GLOBAL, token, line);
            if (token == "struct")
                return Token(TokenType::STRUCT, token, line);
            if (token == "enum")
                return Token(TokenType::ENUM, token, line);

            if (token == "int")
                return Token(TokenType::TYPE_INT, token, line);
            if (token == "float")
                return Token(TokenType::TYPE_FLOAT, token, line);
            if (token == "str")
                return Token(TokenType::TYPE_STR, token, line);
            if (token == "bool")
                return Token(TokenType::TYPE_BOOL, token, line);

            if (token == "true" || token == "false")
                return Token(TokenType::LITERAL_BOOL, token, line);

            return Token(TokenType::IDENTIFIER, token, line);
        }

        // Parsing for equality operators that require more than one character to type.
        if (token == "==")
            return Token(TokenType::EQUALS, token, line);
        if (token == "!=")
            return Token(TokenType::NOT_EQUALS, token, line);
        if (token == "<=")
            return Token(TokenType::LESS_EQUALS, token, line);
        if (token == ">=")
            return Token(TokenType::GREATER_EQUALS, token, line);
        // Parsing for boolean operators.
        if (token == "or" || token == "||")
            return Token(TokenType::OR, token, line);
        if (token == "and" || token == "&&")
            return Token(TokenType::AND, token, line);
        if (token == "xor")
            return Token(TokenType::XOR, token, line);
        if (token == "not")
            return Token(TokenType::NOT, token, line);


        // Function Type assign.
        if (token == "->")
            return Token(TokenType::ARROW, token, line);

        // Mathematical Power (like 2 to the power of 2, equals 4).
        if (token == "**")
            return Token(TokenType::POWER, token, line);

        // No match.
        return Token(TokenType::ERROR, token, line);
    }

    Token matchSingleCharToken(const char c, const int line)
    {
        const std::string token(1, c);

        switch (c)
        {
            // Math operators.
            case '+':
                return Token(TokenType::PLUS, token, line);
            case '-':
                return Token(TokenType::MINUS, token, line);
                break;
            case '*':
                return Token(TokenType::MULTIPLY, token, line);
                break;
            case '/':
                return Token(TokenType::DIVIDE, token, line);
                break;
            case '%':
                return Token(TokenType::MODULO, token, line);
                break;
            // Boolean operators.
            case '^':
                return Token(TokenType::XOR, token, line);
                break;
            case '!':
                return Token(TokenType::NOT, token, line);
                break;
            // Magnitude operators.
            case '<':
                return Token(TokenType::LESS, token, line);
                break;
            case '>':
                return Token(TokenType::GREATER, token, line);
                break;
            // Expression operators.
            case '=':
                return Token(TokenType::ASSIGN, token, line);
                break;
            case '(':
                return Token(TokenType::LPAREN, token, line);
                break;
            case ')':
                return Token(TokenType::RPAREN, token, line);
                break;
            case '{':
                return Token(TokenType::LBRACE, token, line);
                break;
            case '}':
                return Token(TokenType::RBRACE, token, line);
                break;
            case '[':
                return Token(TokenType::LBRACK, token, line);
                break;
            case ']':
                return Token(TokenType::RBRACK, token, line);
                break;
            case ';':
                return Token(TokenType::SEMICOLON, token, line);
                break;
            case '.':
                return Token(TokenType::DOT, token, line);
                break;
            case ',':
                return Token(TokenType::COMMA, token, line);
                break;
            case ':':
                return Token(TokenType::COLON, token, line);
                break;

            default:
                // No Match.
                return Token(TokenType::ERROR, token, line);
                break;
        }
    }
}

std::vector<Token> Lexer::tokenize(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        Error::raise(Error::Phase::Lexer, "File not found", -1);
    }
    std::vector<Token> tokens;
    char c;
    int lineCounter = 1;
    std::string fullToken;
    while (file.get(c))
    {
        bool matched = false;
        if (c == '\n')
        {
            lineCounter++;
            matched = true;
        }
        if (isdigit(c) && fullToken.empty())
        {
            fullToken.push_back(c);
        }


        if (!matched)
        {
            std::string message = "Unexpected character \"";
            message.append(1, c);
            message.append("\".");
            Error::raise(Error::Phase::Lexer, "Unexpected character", lineCounter);
        }
    }

    return tokens;
}
