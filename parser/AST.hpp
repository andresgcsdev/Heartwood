#pragma once

#include <memory>
#include <variant>

#include "../lexer/Token.hpp"

namespace AST
    {
        struct Node;

        struct BinaryExpr
        {
            std::unique_ptr<Node> left, right;
            Token op;
        };

        struct Literal
        {
            Token value;
        };

        struct UnaryExpr
        {
            std::unique_ptr<Node> operand;
            Token op;
        };

        using NodeData = std::variant<BinaryExpr, Literal, UnaryExpr>;
        struct Node { NodeData data; };
    }
