#pragma once

#include <memory>
#include <variant>
#include <optional>

#include "../lexer/Token.hpp"

namespace AST
    {
        struct Node;

        // Leave nodes - actual data

        struct Literal
        {
            Token value;
        };

        struct Identifier
        {
            Token name;
        };

        // Data declaration

        struct TypeNode; // Forward declaration

        struct PrimitiveType
        {
            Token name; // int, string, etc.
        };

        struct ArrayType
        {
            std::unique_ptr<TypeNode> element_type; // Primitive or ArrayType
            std::unique_ptr<Node> size; // Expression, literal or identifier
        };

        using TypeData = std::variant<PrimitiveType, ArrayType>;

        struct TypeNode
        {
            TypeData data;
        };

        struct Parameter
        {
            Token name; // Identifier
            TypeNode type;
            bool is_reference = false;
            bool is_mutable = false;
            std::unique_ptr<Node> initializer; // Expression, literal or identifier
        };

        struct VarDecl
        {
            Token name; // identifier
            TypeNode type;
            bool is_mutable = false;
            std::unique_ptr<Node> initializer;
            // Expression, Literal or Identifier - can be nullptr (default initializer)
        };

        struct StructDecl
        {
            Token name; // Identifier
            std::vector<Parameter> fields;
        };

        struct EnumDecl
        {
            Token name; // Identifier
            std::vector<Token> members; // multiple Identifiers
        };

        // Basic expressions - operations on the data

        struct UnaryExpr
        {
            Token op;
            std::unique_ptr<Node> operand; // Expression, literal or identifier
        };

        struct BinaryExpr
        {
            Token op;
            std::unique_ptr<Node> left; // Expression, literal or identifier
            std::unique_ptr<Node> right; // Expression, literal or identifier
        };

        struct StructLiteral
        {
            Token name; // mapping to a struct declaration
            std::vector<std::pair<Token, std::unique_ptr<Node> > > members;
        };

        struct ArrayExpr
        {
            std::vector<std::unique_ptr<Node> > elements; // Expression, literal or identifier
        };

        struct FnCall
        {
            Token callee; // an Identifier
            std::vector<std::unique_ptr<Node> > arguments; // Expressions, literals or identifiers
        };

        struct MemberAccess
        {
            std::unique_ptr<Node> object; // Grows from right to left
            Token member;
        };

        struct ArrayAccess
        {
            std::unique_ptr<Node> array;
            std::unique_ptr<Node> index;
        };

        struct EnumCall
        {
            Token callee; // an Identifier
            Token member; // an Identifier
        };

        struct Assign
        {
            std::unique_ptr<Node> target; // Can be Identifier, MemberAccess or ArrayAccess
            std::unique_ptr<Node> value; // Literal, Identifier, Expression
        };

        // Scope - container of data and operations

        struct Scope
        {
            std::vector<std::unique_ptr<Node> > statements;
        };

        // Code blocks - expressions that own scopes

        struct If
        {
            std::unique_ptr<Node> condition; // an Expression node
            std::unique_ptr<Node> thenBranch; // a ScopeNode or a single line of instructions
            std::unique_ptr<Node> elseBranch; // a ScopeNode, a single line of instructions or nullptr
        };

        struct While
        {
            std::unique_ptr<Node> condition; // an Expression node
            std::unique_ptr<Node> body; // a ScopeNode or a single line of instructions
        };


        struct For
        {
            std::optional<VarDecl> localCountVariable;
            std::unique_ptr<Node> initialAssign; // nullptr if there is a localCountVariable
            std::unique_ptr<Node> condition; // an Expression node
            std::unique_ptr<Node> operation; // an Expression node
            std::unique_ptr<Node> body; // a ScopeNode or a single line of instructions
        };

        struct FunctionDef
        {
            Token name; // identifier
            std::optional<TypeNode> type;
            std::vector<Parameter> parameters; // identifier + type + value
            Scope body; // a ScopeNode
        };

        struct GlobalBlock
        {
            std::vector<std::unique_ptr<VarDecl>> declarations;
        };

        // Exit

        struct Return
        {
            std::unique_ptr<Node> value; // Expression, Literal or Identifier - can be nullptr
        };

        struct Break
        {
            Token where;
        };

        struct Continue
        {
            Token where;
        };


        using NodeData =
        std::variant<
            Literal, Identifier, UnaryExpr, FnCall, ArrayAccess,
            BinaryExpr, Scope, If, Assign, ArrayExpr, GlobalBlock,
            While, For, FunctionDef, MemberAccess, StructLiteral,
            VarDecl, StructDecl, EnumDecl, Return, EnumCall, Break, Continue
        >;

        struct Node
        {
            NodeData data;
        };
    }
