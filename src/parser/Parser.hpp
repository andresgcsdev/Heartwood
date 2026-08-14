#pragma once

#include <stack>
#include <vector>

#include "../lexer/Token.hpp"
#include "../error/Error.hpp"
#include "AST.hpp"

// Converts a list of Tokens into readable instructions for the evaluator to consume.
class Parser
{
public:
    // No actual changes are made to the token list, so we borrow a constant reference.
    explicit Parser(const std::vector<Token> &tokens);

    // Returns the root of the Abstract Syntax Tree of the given token list.
    // May raise an error if syntax is invalid.
    std::vector<std::unique_ptr<AST::Node> > parse();

private:
    std::vector<Token> tokens;
    int counter = 0;
    std::stack<Token> braceStack;
    ErrorList errors;

    enum class ScopeType
    {
        FUNCTION,
        IF,
        WHILE,
        FOR,
        DO
    };

    // ----- Token list query -----

    // True when at the end of the token list.
    [[nodiscard]] bool isEof() const { return peek().type == TokenType::EoF; }

    // Advances the counter.
    // Returns the current token, then increments.
    // Does not go past the end of the token list.
    Token advance()
    {
        const Token t = peek();
        if (!isEof())
            counter++;

        return t;
    }

    // Get current token.
    [[nodiscard]] Token peek() const { return tokens.at(counter); }

    // Get next token without advancing the counter.
    [[nodiscard]] Token peekNext() const
    {
        if (!isEof())
            return tokens.at(counter + 1);

        return tokens.back();
    }

    // Decreases the counter.
    // Returns the current token, then decreases.
    // Does not go out of bounds.
    Token previous()
    {
        const Token t = peek();
        if (counter > 0)
            counter--;

        return t;
    }

    // Checks if the current token at peek() is of the given type.
    [[nodiscard]] bool check(const TokenType type) const { return peek().type == type; }

    // Advances the counter and checks if the current token is of the given type.
    // Returns true if the check succeeds, false otherwise.
    bool match(const TokenType type)
    {
        const bool r = check(type);
        if (r) advance();
        return r;
    }

    // Checks if the current token is of the given type, then advances the counter.
    // If the check fails, raises an error with the given message and tip.
    // Returns the current token if the check succeeds, otherwise a sentinel token.
    Token expect(const TokenType expected, const std::string &err_message, const std::string &tip = "")
    {
        if (check(expected))
        {
            return advance();
        }

        const Token actual = peek();
        errors.add(ErrorPhase::Parser, err_message, actual.line, actual.type == TokenType::EoF, tip);

        return Token(expected, "", actual.line);
    }

    // ----- AST Node creation -----

    // Maps the AST node for a global variable scope.
    // Current Token at tokens[counter] must be a GLOBAL.
    // Stops at a '}'.
    AST::GlobalBlock handleGlobal();

    // Maps the AST node for a function declaration.
    // Current Token at tokens[counter] must be a FUNCTION.
    // Stops at a '}'.
    AST::FunctionDef handleFuncDecl();

    // Maps the AST node for a struct declaration.
    // Current Token at tokens[counter] must be a STRUCT.
    // Stops at a '}'.
    AST::StructDecl handleStructDecl();

    // Maps the AST node for an Enum declaration.
    // Current Token at tokens[counter] must be an ENUM.
    // Stops at a '}'.
    AST::EnumDecl handleEnumDecl();

    // Maps the AST node for a scope.
    // Current Token at tokens[counter] must be an LBRACE.
    // Stops at a '}'.
    AST::Scope handleScope(const ScopeType &scopeOf, const std::string &functionName = "");

    // Maps the AST node for a single line instruction inside a conditional or loop block (if/else/while/for)
    // Current Token at tokens[counter] must be an LPAREN.
    // Stops at a ';' or '}'.
    AST::Node handleSingleLiner(const ScopeType &scopeOf);

    // Maps the AST node for an if statement.
    // Current Token at tokens[counter] must be an IF.
    // Stops at a ';' or '}'.
    AST::If handleIfStatement();

    // Maps the AST node for a while statement.
    // Current Token at tokens[counter] must be a WHILE.
    // Stops at a ';' or '}'.
    AST::While handleWhileStatement();

    // Maps the AST node for a for statement.
    // Current Token at tokens[counter] must be a FOR.
    // Stops at a ';' or '}'.
    AST::For handleForStatement();

    // Maps the AST node for a do-while statement.
    // Current Token at tokens[counter] must be a DO.
    // Stops at a ';' or '}'.
    AST::Do_While handleDoStatement();

    // Maps the AST node for a switch statement.
    // Current Token at tokens[counter] must be a SWITCH.
    // Stops at a '}'.
    AST::Switch handleSwitchStatement();

    // Maps the AST node for a variable declaration.
    // Current Token at tokens[counter] must be a VAR.
    // Stops at a ';'.
    AST::VarDecl handleVarDecl();

    // Maps the AST node for a return statement.
    // Current Token at tokens[counter] must be a RETURN.
    // Stops at a ';'.
    AST::Return handleReturnStatement();

    // Maps the AST node for type attribution.
    // Current Token at tokens[counter] must be a type.
    // Stops at the last token representing the type: ']' for arrays and 'int'/'str'/etc for primitives.
    AST::TypeNode handleType();

    // Maps the AST node for an expression.
    // Current Token at tokens[counter] must be a literal or an identifier.
    // Stops at the first token with no operators before it.
    AST::Node handleExpr();

    // Maps the AST node for a function call.
    // Current Token at tokens[counter] must be an identifier.
    // Stops at a ')'.
    AST::FnCall handleFnCall();

    // Maps the AST node for a assign operation.
    // Current Token at tokens[counter] must be an identifier.
    // Stops at a ';'.
    AST::Assign handleAssign();

    // ----- General Errors -----

    // Raises errors for unexpected declarations at root scope.
    // Has custom messages for each type of error with tips for the user.
    void handleRootError(const Token &actual);

    // Raises errors for unexpected declarations at function/condition/loop scope.
    // Has custom messages for each type of error with tips for the user.
    void handleScopeError(const Token &actual, const std::string &scopeKind);
};
