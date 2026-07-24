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

    enum class ScopeType
    {
        FUNCTION,
        IF,
        WHILE,
        FOR,
        DO
    };

    // Get next token.
    // Does not go past the last element of the token list.
    // Returns a sentinel EOF token when consuming at the end of the token list.
    Token consume()
    {
        if (counter + 1 < tokens.size())
            return tokens.at(++counter);

        return Token{.type = TokenType::EoF, .value = "<End of File>", .line = peek().line};
    }

    // Get current token.
    Token peek() { return tokens.at(counter); }

    // Get previous token.
    // May return an error if counter < 0.
    Token previous() {return tokens.at(--counter);}

    // True when at the end of the token list.
    [[nodiscard]] bool isEof() const { return tokens.size() <= counter; }

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

    // Maps the AST node for type attribution.
    // Current Token at tokens[counter] must be a type.
    // Stops at the last token representing the type: ']' for arrays and 'int'/'str'/etc for primitives.
    AST::TypeNode handleType();

    // Maps the AST node for an expression.
    // Stops at a ';'.
    AST::Node handleExpr();

    // Maps the AST node for a function call.
    // Current Token at tokens[counter] must be an identifier.
    // Stops at a ')'.
    AST::FnCall handleFnCall();

    // Maps the AST node for a assign operation.
    // Current Token at tokens[counter] must be an identifier.
    // Stops at a ';'.
    AST::Assign handleAssign();

    // Raises errors for unexpected declarations at root scope.
    // Has custom messages for each type of error with tips for the user.
    static void handleRootError(const Token &actual);

    // Raises errors for unexpected declarations at function/condition/loop scope.
    // Has custom messages for each type of error with tips for the user.
    static void handleScopeError(const Token &actual, const std::string &scopeKind);
};
