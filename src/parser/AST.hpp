#pragma once

#include <memory>
#include <variant>
#include <optional>

#include "../lexer/Token.hpp"

namespace AST
{
    struct Node;

    // Leave nodes - actual data.

    struct Literal
    {
        Token value;
    };

    struct Identifier
    {
        Token name;
    };

    // Data declaration.

    struct TypeNode; // Forward declaration.

    struct PrimitiveType
    {
        Token name; // int, string, etc.
    };

    struct ArrayType
    {
        std::unique_ptr<TypeNode> element_type; // Primitive or ArrayType.
        std::unique_ptr<Node> size; // Expression, literal or identifier.
    };

    struct StructType
    {
        Token name;
    };

    using TypeData = std::variant<PrimitiveType, ArrayType, StructType>;

    struct TypeNode
    {
        TypeData data;
    };

    struct Parameter
    {
        Token name; // Identifier.
        TypeNode type;
        bool is_reference = false;
        bool is_mutable = false;
        std::unique_ptr<Node> initializer; // Expression, literal or identifier.
    };

    struct VarDecl
    {
        Token name; // identifier.
        TypeNode type;
        bool is_mutable = false;
        std::optional<std::unique_ptr<Node> > initializer = std::nullopt;
        // Expression, Literal or Identifier - can be nullopt (default initializer).
    };

    struct StructDecl
    {
        Token name; // Identifier.
        std::vector<VarDecl> fields;
    };

    struct EnumDecl
    {
        Token name; // Identifier.
        std::vector<Token> members; // multiple Identifiers.
    };

    // Basic expressions - operations on the data.

    struct UnaryExpr
    {
        Token op;
        std::unique_ptr<Node> operand; // Expression, literal or identifier.
    };

    struct BinaryExpr
    {
        Token op;
        std::unique_ptr<Node> left; // Expression, literal or identifier.
        std::unique_ptr<Node> right; // Expression, literal or identifier.
    };

    // Complex literals - literals that don't map directly into pure data.

    struct StructLiteral
    {
        Token name; // mapping to a struct declaration.
        std::vector<std::pair<Token, std::unique_ptr<Node> > > members;
    };

    struct ArrayExpr
    {
        std::vector<std::unique_ptr<Node> > elements; // Expression, literal or identifier.
    };

    struct FnCall
    {
        Token callee; // an Identifier.
        std::vector<std::unique_ptr<Node> > arguments; // Expressions, literals or identifiers.
    };

    struct MemberAccess
    {
        std::unique_ptr<Node> object;
        std::unique_ptr<Node> member;
    };

    struct ArrayAccess
    {
        std::unique_ptr<Node> array;
        std::unique_ptr<Node> index;
    };

    struct EnumCall
    {
        Token callee; // an Identifier.
        Token member; // an Identifier.
    };

    // Special operation

    struct Assign
    {
        std::unique_ptr<Node> target; // Can be Identifier, MemberAccess or ArrayAccess.
        std::unique_ptr<Node> value; // Literal, Identifier, Expression.
    };

    // Scope - container of data and operations.

    struct Scope
    {
        std::vector<std::unique_ptr<Node> > statements;
    };

    // Code blocks - expressions that own scopes.

    struct If
    {
        std::unique_ptr<Node> condition; // an Expression node.
        std::unique_ptr<Node> thenBranch; // a ScopeNode or a single line of instructions.
        std::optional<std::unique_ptr<Node> > elseBranch = std::nullopt; // a ScopeNode, a single line of instructions or nullopt.
    };

    struct Case
    {
        std::unique_ptr<Node> value; //  Simple literal only expression.
        std::vector<std::unique_ptr<Node> > instructions; // Can't declare variables.
    };

    struct Switch
    {
        std::unique_ptr<Node> variable; // Identifier or member access.
        std::vector<Case> cases;
    };

    struct While
    {
        std::unique_ptr<Node> condition; // an Expression node.
        std::unique_ptr<Node> body; // a ScopeNode or a single line of instructions.
    };

    struct Do_While
    {
        std::unique_ptr<Node> body; // a ScopeNode or a single line of instructions.
        std::unique_ptr<Node> condition; // an Expression node.
    };


    struct For
    {
        std::optional<VarDecl> localCountVariable = std::nullopt;
        std::optional<std::unique_ptr<Node> > initialAssign = std::nullopt; // nullopt if there is a localCountVariable.
        std::unique_ptr<Node> condition; // an Expression node.
        std::unique_ptr<Node> operation; // an Expression node.
        std::unique_ptr<Node> body; // a ScopeNode or a single line of instructions.
    };

    struct FunctionDef
    {
        Token name; // identifier.
        std::optional<TypeNode> type;
        std::vector<Parameter> parameters; // identifier + type + value.
        Scope body; // a ScopeNode.
    };

    struct GlobalBlock
    {
        std::vector<VarDecl> declarations;
    };

    // Exit.

    struct Return
    {
        std::optional<std::unique_ptr<Node> > value; // Expression, Literal or Identifier - can be nullopt.
    };

    struct Break
    {
        Token where;
    };

    struct Continue
    {
        Token where;
    };

    // Error.

    struct BadExpression
    {
        Token where;
    };

    // Union.

    using NodeData =
    std::variant<
        Literal, Identifier, UnaryExpr, FnCall, ArrayAccess, BadExpression,
        BinaryExpr, Scope, If, Assign, ArrayExpr, GlobalBlock, Switch,
        While, Do_While, For, FunctionDef, MemberAccess, StructLiteral,
        VarDecl, StructDecl, EnumDecl, Return, EnumCall, Break, Continue
    >;

    struct Node
    {
        NodeData data; // Tree of instructions at this line.
        int startLine = -1; // Line of the root node.
    };
}
