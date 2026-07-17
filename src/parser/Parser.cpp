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
    const Token global = peek();
    if (global.type != TokenType::GLOBAL)
    {
        const std::string message = "Unexpected token at global declaration. Found: '" + global.value +
                                    "' Expected: 'global'.";
        Error::raise(Error::Phase::Parser, message, global.line);
    }

    const Token lbrace = consume();
    if (lbrace.type != TokenType::RBRACE)
    {
        const std::string message = "Unexpected token at global declaration. Found: '" + lbrace.value +
                                    "' Expected: '{'.";
        Error::raise(Error::Phase::Parser, message, lbrace.line, "Missing a left brace to open the global scope.");
    }
    AST::GlobalBlock globalNode;
    const Token var = consume();
    while (var.type != TokenType::RBRACE)
    {
        if (var.type != TokenType::VAR)
        {
            const std::string message = "Unexpected token at global declaration. Found: '" + lbrace.value +
                                        "' Expected: 'var'.";
            Error::raise(Error::Phase::Parser, message, lbrace.line,
                         "Global block only supports variable declarations. Any other action needs to happen in other scopes.");
        }
        globalNode.declarations.push_back(handleVarDecl());
        consume();
    }
    return globalNode;
}

AST::FunctionDef Parser::handleFuncDecl()
{
    const Token fn = peek();
    if (fn.type != TokenType::FUNCTION)
    {
        const std::string message = "Unexpected token at function declaration. Found: '" + fn.value +
                                    "' Expected: 'fn'.";
        Error::raise(Error::Phase::Parser, message, fn.line);
    }

    const Token funcIdentifier = consume();
    if (funcIdentifier.type != TokenType::IDENTIFIER)
    {
        const std::string message = "Unexpected token at function declaration. Found: '" + funcIdentifier.value +
                                    "' Expected: An Identifier.";
        Error::raise(Error::Phase::Parser, message, funcIdentifier.line,
                     "The name of the function must be an unreserved word with no symbols.");
    }

    const Token paramLParen = consume();
    if (paramLParen.type != TokenType::LPAREN)
    {
        const std::string message = "Unexpected token at function declaration. Found: '" + paramLParen.value +
                                    "' Expected: '('.";
        Error::raise(Error::Phase::Parser, message, paramLParen.line,
                     "Missing left parenthesis for parameter declarations.");
    }

    // Guaranteed expression right now: fn -identifier-(

    std::vector<AST::Parameter> parameters;
    bool foundParen = false;
    // Looping for parameters:
    while (!foundParen)
    {
        AST::Parameter param;
        Token currentToken = consume();
        if (currentToken.type == TokenType::MUT)
        {
            param.is_mutable = true;
            currentToken = consume();
        }
        if (currentToken.type == TokenType::REF)
        {
            param.is_reference = true;
            currentToken = consume();
        }
        if (currentToken.type != TokenType::IDENTIFIER)
        {
            const std::string message = "Unexpected token at function declaration. Found: '" + currentToken.value +
                                        "' Expected: An identifier, 'mut' or 'ref'.";
            Error::raise(Error::Phase::Parser, message, currentToken.line,
                         "The name for a parameter must be an unreserved word with no symbols.");
        }
        param.name = currentToken;
        currentToken = consume();
        if (currentToken.type != TokenType::COLON)
        {
            const std::string message = "Unexpected token at function declaration. Found: '" + currentToken.value +
                                        "' Expected: ':'.";
            Error::raise(Error::Phase::Parser, message, currentToken.line,
                         "Missing colon for type declaration.");
        }
        param.type = handleType();
        currentToken = consume();
        if (currentToken.type == TokenType::ASSIGN)
        {
            consume();
            param.initializer = handleExpr();
        }
        parameters.push_back(std::move(param));
        currentToken = consume();
        if (currentToken.type == TokenType::RPAREN)
            foundParen = true;
        else if (currentToken.type != TokenType::COMMA)
        {
            const std::string message = "Unexpected token at function declaration. Found: '" + currentToken.value +
                                        "' Expected: ',' or ')'.";
            Error::raise(Error::Phase::Parser, message, currentToken.line,
                         "Either missing a comma for next parameter or missing a right paren for closing parameter declaration.");
        }
    }

    // Guaranteed expression right now: fn -identifier(-param1, param2, ...-)

    // Setting the AST node.
    AST::FunctionDef func;
    func.name = funcIdentifier;
    func.parameters = std::move(parameters);

    // Check if the function has a type or if it is a void one.
    if (consume().type == TokenType::ARROW)
    {
        consume();
        func.type = handleType();
        consume();
    }

    // End of function header.
    const Token lbrace = peek();
    if (lbrace.type == TokenType::LBRACE)
    {
        func.type = std::nullopt;
    }
    else
    {
        const std::string message = "Unexpected token at function declaration. Found: '" + lbrace.value +
                                        "' Expected: '{'.";
        Error::raise(Error::Phase::Parser, message, lbrace.line,
                     "Missing arrow for function type declaration.");
    }

    func.body = handleScope();

    return func;
}

void Parser::handleRootError(const Token &actual)
{
    const std::string message = "Unexpected token at root level. Found: '" + actual.value +
                                "' Expected: A Scope Declaration.";
    std::string tips;
    if (actual.type == TokenType::VAR)
    {
        // The user is probably trying to create a global variable.
        tips = "Global variables must be declared inside a global{} scope.";
    } else if (actual.type == TokenType::IF || actual.type == TokenType::WHILE || actual.type == TokenType::FOR ||
               actual.type == TokenType::RETURN)
    {
        // The user is probably trying to run code without declaring a main function.
        tips = "Code flow must be inside of a function. Declare fn main(){} to run.";
    } else if (actual.type == TokenType::RBRACE)
    {
        // The user probably closed too many scopes while debugging.
        tips = "Leftover right brace. Check for a missing/extra brace in previous declarations.";
    } else if (actual.type == TokenType::LBRACE)
    {
        // The user probably forgot to use a keyword for creating a scope.
        tips = "Missing a scope keyword (global, function, struct, enum) before this brace.";
    }

    Error::raise(Error::Phase::Parser, message, actual.line, tips);
}
