#include "Parser.hpp"
#include "../error/Error.hpp"

#include <format>

Parser::Parser(const std::vector<Token> &tokens)
{
    this->tokens = tokens;
    counter = 0;
}

std::vector<std::unique_ptr<AST::Node> > Parser::parse()
{
    std::vector<std::unique_ptr<AST::Node> > ast;
    while (!isEof()) // Root scope for function/struct/enum/global definitions. Everything else raises an error.
    {
        const Token currentToken = peek();
        auto node = std::make_unique<AST::Node>();
        switch (currentToken.type)
        {
            case TokenType::GLOBAL:
                node->data = handleGlobal();
                break;
            case TokenType::FUNCTION:
                node->data = handleFuncDecl();
                break;
            case TokenType::STRUCT:
                node->data = handleStructDecl();
                break;
            case TokenType::ENUM:
                node->data = handleEnumDecl();
                break;
            default:
                handleRootError(currentToken);
                break;
        }
        ast.push_back(std::move(node));
    }

    return ast;
}

AST::GlobalBlock Parser::handleGlobal()
{
    const Token lbrace = consume();
    if (lbrace.type != TokenType::RBRACE)
    {
        const std::string message = "Unexpected token at global declaration. Found: '" + lbrace.value +
                                    "' Expected: '{'";
        Error::raise(Error::Phase::Parser, message, lbrace.line, "Missing a left brace to open the global scope.");
    }
    AST::GlobalBlock globalNode;
    const Token var = consume();
    while (var.type != TokenType::RBRACE)
    {
        if (var.type != TokenType::VAR)
        {
            const std::string message = "Unexpected token at global declaration. Found: '" + lbrace.value +
                                        "' Expected: 'var'";
            Error::raise(Error::Phase::Parser, message, lbrace.line,
                         "Global block only supports variable declarations. Any other action needs to happen in other scopes.");
        }
        globalNode.declarations.push_back(handleVarDecl());
    }
    return globalNode;
}

void Parser::handleRootError(const Token &actual)
{
    const std::string message = "Unexpected token at root level. Found: '" + actual.value +
                                "' Expected: A Scope Declaration";
    std::string tips;
    if (actual.type == TokenType::VAR)
    {
        tips = "Global variables must be declared inside a global{} scope.";
    } else if (actual.type == TokenType::IF || actual.type == TokenType::WHILE || actual.type == TokenType::FOR ||
               actual.type == TokenType::RETURN)
    {
        tips = "Code flow must be inside of a function. Declare fn main(){} to run.";
    } else if (actual.type == TokenType::RBRACE)
    {
        tips = "Leftover right brace. Check for a missing/extra brace in previous declarations.";
    } else if (actual.type == TokenType::LBRACE)
    {
        tips = "Missing a scope keyword (global, function, struct, enum) before this brace.";
    }

    Error::raise(Error::Phase::Parser, message, actual.line, tips);
}
