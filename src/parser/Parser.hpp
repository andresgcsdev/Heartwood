#pragma once

#include <stack>
#include <vector>

#include "../lexer/Token.hpp"
#include "AST.hpp"

// Converts a list of Tokens into readable instructions for the evaluator to consume.
class Parser
{
public:
    // No actual changes are made to the token list, so we borrow a constant reference
    explicit Parser(const std::vector<Token> &tokens);

    // Returns the root of the Abstract Syntax Tree of the given token list.
    // May raise an error if syntax is invalid.
    std::vector<std::unique_ptr<AST::Node> > parse();

private:
    std::vector<Token> tokens;
    int counter = 0;
    std::stack<Token> braceStack;

    // Get next token.
    Token consume() { return tokens.at(++counter); }
    // Get current token.
    Token peek() { return tokens.at(counter); }
    // True when at the end of the token list.
    [[nodiscard]] bool isEof() const { return tokens.size() <= counter; }

    // ----- AST Node creation -----

    // Maps the AST node for a global variable scope.
    // Current Token at tokens[counter] must be a GLOBAL.
    AST::GlobalBlock handleGlobal();

    // Maps the AST node for a function declaration.
    // Current Token at tokens[counter] must be a FUNCTION.
    AST::FunctionDef handleFuncDecl();

    // Maps the AST node for a struct declaration.
    // Current Token at tokens[counter] must be a STRUCT.
    AST::StructDecl handleStructDecl();

    // Maps the AST node for an Enum declaration.
    // Current Token at tokens[counter] must be an ENUM.
    AST::EnumDecl handleEnumDecl();

    // Maps the AST node for a scope.
    // Current Token at tokens[counter] must be an LBRACE.
    AST::Scope handleScope();

    // Maps the AST node for a variable declaration.
    // Current Token at tokens[counter] must be a VAR.
    AST::VarDecl handleVarDecl();

    // Maps the AST node for type attribution.
    // Current Token at tokens[counter] must be a type.
    AST::TypeNode handleType();

    // Maps the AST node for an expression.
    AST::Node handleExpr();

    // Raises errors for unexpected declarations at root scope.
    // Has custom messages for each type of error with tips for the user.
    static void handleRootError(const Token &actual);
};
