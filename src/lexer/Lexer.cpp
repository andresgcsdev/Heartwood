#include "Lexer.hpp"
#include "../error/Error.hpp"
#include <fstream>
#include <cctype>

namespace
{
    // Helper for matching Tokens that can be longer than a single character.
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
        if (isalpha(token[0]) || token[0] == '_')
        {
            for (const auto &c: token)
            {
                if (!isalnum(c) && c != '_')
                    return Token(TokenType::ERROR, token, line);
            }
            if (token == "if")
                return Token(TokenType::IF, token, line);
            if (token == "else")
                return Token(TokenType::ELSE, token, line);
            if (token == "switch")
                return Token(TokenType::SWITCH, token, line);
            if (token == "case")
                return Token(TokenType::CASE, token, line);
            if (token == "while")
                return Token(TokenType::WHILE, token, line);
            if (token == "do")
                return Token(TokenType::DO, token, line);
            if (token == "for")
                return Token(TokenType::FOR, token, line);
            if (token == "break")
                return Token(TokenType::BREAK, token, line);
            if (token == "return")
                return Token(TokenType::RETURN, token, line);
            if (token == "continue")
                return Token(TokenType::CONTINUE, token, line);

            if (token == "var")
                return Token(TokenType::VAR, token, line);
            if (token == "mut")
                return Token(TokenType::MUT, token, line);
            if (token == "ref")
                return Token(TokenType::REF, token, line);
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
            if (token == "or")
                return Token(TokenType::OR, token, line);
            if (token == "and")
                return Token(TokenType::AND, token, line);
            if (token == "xor")
                return Token(TokenType::XOR, token, line);
            if (token == "not")
                return Token(TokenType::NOT, token, line);

            return Token(TokenType::IDENTIFIER, token, line);
        }

        // Parsing for boolean operators.
        if (token == "||")
            return Token(TokenType::OR, token, line);
        if (token == "&&")
            return Token(TokenType::AND, token, line);

        // Parsing for equality operators that require more than one character to type.
        if (token == "==")
            return Token(TokenType::EQUALS, token, line);
        if (token == "!=")
            return Token(TokenType::NOT_EQUALS, token, line);
        if (token == "<=")
            return Token(TokenType::LESS_EQUALS, token, line);
        if (token == ">=")
            return Token(TokenType::GREATER_EQUALS, token, line);

        // Function Type assign.
        if (token == "->")
            return Token(TokenType::ARROW, token, line);

        // No match.
        return Token(TokenType::ERROR, token, line);
    }

    // Helper function to match special single character tokens.
    // May return a Token with TokenType::BIGGER,
    //   which means it can't identify the token based off one character.
    // When TokenType::BIGGER is found, use matchLargeToken instead.
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
            case '*':
                return Token(TokenType::MULTIPLY, token, line);
            case '/':
                return Token(TokenType::DIVIDE, token, line);
            case '%':
                return Token(TokenType::MODULO, token, line);
            // Boolean operators.
            case '^':
                return Token(TokenType::XOR, token, line);
            case '!':
                return Token(TokenType::NOT, token, line);
            // Magnitude operators.
            case '<':
                return Token(TokenType::LESS, token, line);
            case '>':
                return Token(TokenType::GREATER, token, line);
            // Expression operators.
            case '=':
                return Token(TokenType::ASSIGN, token, line);
            case '(':
                return Token(TokenType::LPAREN, token, line);
            case ')':
                return Token(TokenType::RPAREN, token, line);
            case '{':
                return Token(TokenType::LBRACE, token, line);
            case '}':
                return Token(TokenType::RBRACE, token, line);
            case '[':
                return Token(TokenType::LBRACK, token, line);
            case ']':
                return Token(TokenType::RBRACK, token, line);
            case ';':
                return Token(TokenType::SEMICOLON, token, line);
            case ',':
                return Token(TokenType::COMMA, token, line);
            case ':':
                return Token(TokenType::COLON, token, line);
            case ' ':
                return Token(TokenType::SPACE, token, line);
            case '"':
                return Token(TokenType::QUOTE, token, line);
            // Special bigger tokens.
            case '&':
                return Token(TokenType::SINGLE_AND, token, line);
            case '|':
                return Token(TokenType::SINGLE_OR, token, line);
            case '.':
                return Token(TokenType::DOT, token, line);
            default:
                break;
        }
        // This single character token may form a large one.
        if (isalnum(c) || c == '_')
            return Token(TokenType::BIGGER, token, line);

        return Token(TokenType::ERROR, token, line);
    }

    // Transforms the given string into a token.
    // Then appends it to the end of the given token list.
    // Moves the column start counter to the end of the token.
    // Returns true for successful conversion, false otherwise.
    // Does not clear the tokenBuffer if it finds an invalid token.
    bool flushToken(std::string &tokenBuffer, std::vector<Token> &tokens, const int &line, int &columnStart,
                    const int &columnCounter)
    {
        if (tokenBuffer.empty())
        {
            columnStart = columnCounter + 1;
            return true;
        }

        Token actual;
        if (tokenBuffer.size() == 1)
            actual = matchSingleCharToken(tokenBuffer[0], line);
        else
            actual = matchLargeToken(tokenBuffer, line);


        if (actual.type == TokenType::ERROR)
        {
            tokens.push_back(Token(TokenType::ERROR, tokenBuffer, line, columnSpan(columnStart, columnCounter)));
            columnStart = columnCounter + 1;
            return false;
        }

        if (actual.type == TokenType::BIGGER) actual = matchLargeToken(tokenBuffer, line);

        actual.col = columnSpan(columnStart, columnCounter);
        tokens.push_back(actual);
        tokenBuffer.clear();
        columnStart = columnCounter + 1;
        return true;
    }
}

std::vector<Token> Lexer::tokenize(const std::string &filepath, ErrorList &errors)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        ErrorList::raiseThis(ErrorNode(ErrorPhase::Lexer, 0, 0, 0, "File not found"));
    }
    std::vector<Token> tokens;
    char c;
    int lineCounter = 1;
    int columnCounter = 1;
    int columnStart = 1;
    std::string tokenBuffer;
    bool inString = false;
    while (file.get(c))
    {
        // Single character match token, does not consider keywords.
        Token currentToken = matchSingleCharToken(c, lineCounter);
        currentToken.col = columnSpan(columnStart, columnCounter);
        bool matched = true;
        columnCounter++;
        // Start of string literal check.
        if (c == '"' && !inString)
        {
            inString = true;
            // Flushing token buffer.
            matched = flushToken(tokenBuffer, tokens, currentToken.line, columnStart, columnCounter);
            tokenBuffer.push_back('"');
        }
        // New Line. Will break any token being built before.
        else if (c == '\n')
        {
            // Checking for unclosed string literals.
            if (inString)
            {
                matched = false;
                flushToken(tokenBuffer, tokens, currentToken.line, columnStart, columnCounter);
            }
            // Break previous token.
            else if (!tokenBuffer.empty())
            {
                matched = flushToken(tokenBuffer, tokens, currentToken.line, columnStart, columnCounter);
            }
            if (matched)
            {
                lineCounter++;
                columnCounter = 1;
                columnStart = 1;
            }
        }
        // Inside string literal check.
        else if (inString)
        {
            // End of literal check.
            if (c == '"')
            {
                tokenBuffer.push_back('"');
                Token largeToken = matchLargeToken(tokenBuffer, lineCounter);
                largeToken.col = columnSpan(columnStart, columnCounter);
                tokens.push_back(largeToken);
                tokenBuffer.clear();
                columnStart = columnCounter + 1;
                inString = false;
            } else
            {
                tokenBuffer.push_back(c);
            }
        }
        // Discard previous token and spaces or tabs.
        else if (c == '\t' || currentToken.type == TokenType::SPACE)
        {
            // Flush previous token.
            matched = flushToken(tokenBuffer, tokens, currentToken.line, columnStart, columnCounter);
        }
        // Ignore comments.
        else if (c == '#')
        {
            // Flush previous token.
            matched = flushToken(tokenBuffer, tokens, currentToken.line, columnStart, columnCounter);

            // Looks for next line break or end of file.
            while (file.get(c) && c != '\n')
            {
            }
            lineCounter++;
        }
        // Unknown character.
        else if (currentToken.type == TokenType::ERROR)
        {
            tokenBuffer.push_back(c);
        }
        // New Token.
        else if (tokenBuffer.empty())
        {
            // Checking if current char could be a part of a bigger token. (e.g. >=, var, ->)
            if (currentToken.type == TokenType::BIGGER || currentToken.type == TokenType::LESS || currentToken.type ==
                TokenType::GREATER ||
                currentToken.type == TokenType::ASSIGN || currentToken.type == TokenType::NOT || currentToken.type ==
                TokenType::MINUS ||
                currentToken.type == TokenType::SINGLE_AND || currentToken.type == TokenType::SINGLE_OR)
            {
                // Bigger token.
                tokenBuffer.push_back(c);
            } else
            {
                // Single character token.
                tokens.push_back(currentToken);
                columnStart = columnCounter + 1;
            }
        }
        // Separating identifiers and Dots without messing up Float Literals.
        else if (!tokenBuffer.empty() && c == '.')
        {
            Token prevToken = matchLargeToken(tokenBuffer, lineCounter);
            if (prevToken.type == TokenType::IDENTIFIER)
            {
                prevToken.col = columnSpan(columnStart, columnCounter - 1);
                tokens.push_back(prevToken);
                columnStart = columnCounter;
                tokenBuffer.clear();
                tokens.push_back(Token(TokenType::DOT, ".", lineCounter, columnSpan(columnStart, columnCounter)));
                columnStart = columnCounter + 1;
            } else
            {
                tokenBuffer.push_back(c);
            }
        }
        // Checking for two character tokens that are only symbols (==, !=, >=, <=, ->, &&, ||).
        // Previous character at fullToken is always (<, >, =, !, -, &, |) or another character that is an identifier.
        else if (tokenBuffer.length() == 1)
        {
            Token prevToken = matchSingleCharToken(tokenBuffer.back(), lineCounter);
            prevToken.col = columnSpan(columnStart, columnCounter - 1);
            // Looking for ==, !=, >=, <=
            if (currentToken.type == TokenType::ASSIGN)
            {
                switch (prevToken.type)
                {
                    case TokenType::LESS:
                        tokenBuffer.clear();
                        tokens.push_back(Token(TokenType::LESS_EQUALS, "<=", lineCounter,
                                               columnSpan(columnStart, columnCounter)));
                        columnStart = columnCounter + 1;
                        break;
                    case TokenType::GREATER:
                        tokenBuffer.clear();
                        tokens.push_back(Token(TokenType::GREATER_EQUALS, ">=", lineCounter,
                                               columnSpan(columnStart, columnCounter)));
                        columnStart = columnCounter + 1;
                        break;
                    case TokenType::ASSIGN:
                        tokenBuffer.clear();
                        tokens.push_back(Token(TokenType::EQUALS, "==", lineCounter,
                                               columnSpan(columnStart, columnCounter)));
                        columnStart = columnCounter + 1;
                        break;
                    case TokenType::NOT:
                        tokenBuffer.clear();
                        tokens.push_back(Token(TokenType::NOT_EQUALS, "!=", lineCounter,
                                               columnSpan(columnStart, columnCounter)));
                        columnStart = columnCounter + 1;
                        break;
                    default:
                        // Begins with something other than the expected tokens. Probably a separate token or identifier.
                        if (prevToken.type == TokenType::ERROR)
                            matched = false;

                        tokens.push_back(prevToken);
                        columnStart = columnCounter;
                        tokenBuffer.clear();
                        // Since this character is an '=', it could be a 2-character token.
                        tokenBuffer.push_back(c);
                        break;
                }
            }
            // Looking for ->
            else if (currentToken.type == TokenType::GREATER && prevToken.type == TokenType::MINUS)
            {
                tokenBuffer.clear();
                tokens.push_back(Token(TokenType::ARROW, "->", lineCounter, columnSpan(columnStart, columnCounter)));
                columnStart = columnCounter + 1;
            }
            // Looking for &&
            else if (currentToken.type == TokenType::SINGLE_AND && prevToken.type == TokenType::SINGLE_AND)
            {
                tokenBuffer.clear();
                tokens.push_back(Token(TokenType::AND, "&&", lineCounter, columnSpan(columnStart, columnCounter)));
                columnStart = columnCounter + 1;
            }
            // Looking for ||
            else if (currentToken.type == TokenType::SINGLE_OR && prevToken.type == TokenType::SINGLE_OR)
            {
                tokenBuffer.clear();
                tokens.push_back(Token(TokenType::OR, "||", lineCounter, columnSpan(columnStart, columnCounter)));
                columnStart = columnCounter + 1;
            }
            // If not completing for a two-special-character token, append to end of string or discard previous token.
            else if (currentToken.type != TokenType::ERROR)
            {
                // If both are tokens that can be part of a bigger one, ignore for now.
                if (currentToken.type == TokenType::BIGGER && prevToken.type == TokenType::BIGGER)
                {
                    tokenBuffer.push_back(c);
                } else
                {
                    // Discard previous token, they're unrelated.
                    if (prevToken.type == TokenType::BIGGER)
                    {
                        Token largePrevToken = matchLargeToken(tokenBuffer, lineCounter);
                        tokens.push_back(largePrevToken);
                    } else
                        tokens.push_back(prevToken);
                    columnStart = columnCounter;

                    tokenBuffer.clear();
                    // This token could be one that completes a bigger two-special-character token.
                    if (currentToken.type == TokenType::BIGGER || currentToken.type == TokenType::LESS || currentToken.
                        type == TokenType::GREATER ||
                        currentToken.type == TokenType::ASSIGN || currentToken.type == TokenType::NOT || currentToken.
                        type == TokenType::MINUS)
                    // Bigger token.
                        tokenBuffer.push_back(c);
                    else
                    {
                        // Single character token.
                        tokens.push_back(currentToken);
                        columnStart = columnCounter + 1;
                    }
                }
            }
        }
        // Matching for tokens after 2 characters.
        else
        {
            // Since the current token is evaluated for a single character, the IDENTIFIER type just refers to a letter or number.
            if (currentToken.type == TokenType::BIGGER)
            {
                tokenBuffer.push_back(c);
            } else
            {
                // Current token is not related to previous token.
                matched = flushToken(tokenBuffer, tokens, lineCounter, columnStart, columnCounter);
                // Checking for bigger token possibility.
                if (currentToken.type == TokenType::LESS || currentToken.type == TokenType::GREATER ||
                    currentToken.type == TokenType::ASSIGN || currentToken.type == TokenType::NOT || currentToken.type
                    == TokenType::MINUS)
                    // Bigger token.
                    tokenBuffer.push_back(c);
                else
                    // Single character token.
                    tokens.push_back(currentToken);
            }
        }
        if (!matched)
        {
            int index;
            if (tokens.back().type == TokenType::ERROR)
                index = tokens.size() - 1;
            else
                index = tokens.size() - 2;

            if (inString)
                errors.add(ErrorNode(ErrorPhase::Lexer, lineCounter, index, index,
                                     "Unterminated string literal"));
            else
                errors.add(ErrorNode(ErrorPhase::Lexer, lineCounter, index, index,
                                     "Invalid token \"" + tokenBuffer + "\"."));

            tokenBuffer.clear();
        }
    }
    file.close();

    // Flushing anything that remained in the buffer.

    if (!tokenBuffer.empty() && !flushToken(tokenBuffer, tokens, lineCounter, columnStart, columnCounter))
    {
        int index;
        if (tokens.back().type == TokenType::ERROR)
            index = tokens.size() - 1;
        else
            index = tokens.size() - 2;

        if (inString)
            errors.add(ErrorNode(ErrorPhase::Lexer, lineCounter, index, index,
                                 "Unterminated string literal"));
        else
            errors.add(ErrorNode(ErrorPhase::Lexer, lineCounter, index, index,
                                 "Invalid token \"" + tokenBuffer + "\"."));
    }


    tokens.push_back(Token(TokenType::EoF, "", lineCounter, columnSpan(columnStart, columnCounter)));

    return tokens;
}
