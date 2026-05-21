#pragma once

#include <memory>

#include "../lexer/Token.hpp"

// File for Abstract Syntax Tree nodes declarations.

struct ASTNode
{
    virtual ~ASTNode() = default;
};

struct LiteralNode : ASTNode {
    Token value;
};

struct BinaryOpNode : ASTNode {
    Token op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
};

struct IdentifierNode : ASTNode
{
    Token value;
};