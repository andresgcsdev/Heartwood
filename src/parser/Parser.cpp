#include "Parser.hpp"
#include "../error/Error.hpp"

#include <format>

Parser::Parser(const std::vector<Token> &tokens)
{
    this->tokens = tokens;
    counter = 0;
    errors = {};
}

std::vector<std::unique_ptr<AST::Node> > Parser::parse()
{
    std::vector<std::unique_ptr<AST::Node> > ast;
    while (!isEof())
    // Root scope for function/struct/enum/global definitions. Everything else raises an error.
    {
        const Token currentToken = peek();
        auto node = std::make_unique<AST::Node>(AST::BadExpression(currentToken), currentToken.line);
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
        node->startLine = currentToken.line;
        ast.push_back(std::move(node));
    }

    return ast;
}

void Parser::handleRootError(const Token &actual)
{
    const std::string message = "Unexpected token at root level. Found: '" + actual.value +
                                "'. Expected: A Scope Declaration.";
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

    errors.add(ErrorPhase::Parser, message, actual.line, actual.type == TokenType::EoF, tips);
}

AST::GlobalBlock Parser::handleGlobal()
{
    expect(
        TokenType::GLOBAL,
        "Unexpected token at global declaration. Found: '" + peek().value + "'. Expected: 'global'."
    );

    expect(
        TokenType::LBRACE,
        "Unexpected token at global declaration. Found: '" + peek().value + "'. Expected: '{'.",
        "Missing a left brace to open the global scope."
    );

    AST::GlobalBlock globalNode;

    while (!check(TokenType::RBRACE) && !isEof())
    {
        if (!check(TokenType::VAR))
        {
            expect(
                TokenType::VAR,
                "Unexpected token at global declaration. Found: '" + peek().value + "'. Expected: 'var'.",
                "Global block only supports variable declarations. Any other action needs to happen in other scopes."
            );
            advance();
            continue;
        }

        globalNode.declarations.push_back(handleVarDecl());
    }

    expect(
        TokenType::RBRACE,
        "Unexpected token at global declaration. Found: '" + peek().value + "'. Expected: '}'."
    );

    return globalNode;
}

AST::FunctionDef Parser::handleFuncDecl()
{
    expect(
        TokenType::FUNCTION,
        "Unexpected token at function declaration. Found: '" + peek().value + "'. Expected: 'fn'."
    );

    const Token funcIdentifier = expect(
        TokenType::IDENTIFIER,
        "Unexpected token at function declaration. Found: '" + peek().value + "'. Expected: An Identifier.",
        "The name of the function must be an unreserved word with no symbols."
    );

    expect(
        TokenType::LPAREN,
        "Unexpected token at function declaration. Found: '" + peek().value + "'. Expected: '('.",
        "Missing left parenthesis for parameter declarations."
    );

    // Guaranteed expression right now: fn -identifier-(

    std::vector<AST::Parameter> parameters;

    while (!check(TokenType::RPAREN) && !isEof())
    {
        AST::Parameter param;

        if (match(TokenType::MUT))
            param.is_mutable = true;

        if (match(TokenType::REF))
            param.is_reference = true;

        param.name = expect(
            TokenType::IDENTIFIER,
            "Unexpected token at function declaration. Found: '" + peek().value +
            "'. Expected: An identifier, 'mut' or 'ref'.",
            "The name for a parameter must be an unreserved word with no symbols."
        );

        expect(
            TokenType::COLON,
            "Unexpected token at function declaration. Found: '" + peek().value + "'. Expected: ':'.",
            "Missing colon for type declaration."
        );

        param.type = handleType();

        if (match(TokenType::ASSIGN))
        {
            param.initializer = std::make_unique<AST::Node>(handleExpr());
        }

        parameters.push_back(std::move(param));

        if (check(TokenType::RPAREN))
            break;

        if (check(TokenType::LBRACE))
        {
            expect(
                TokenType::RPAREN,
                "Unexpected token at function declaration. Found: '" + peek().value + "'. Expected: ')'.",
                "Missing a right paren for closing parameter declaration."
            );
            break;
        }

        expect(
            TokenType::COMMA,
            "Unexpected token at function declaration. Found: '" + peek().value + "'. Expected: ',' or ')'.",
            "Either missing a comma for next parameter or missing a right paren for closing parameter declaration."
        );

        if (!check(TokenType::IDENTIFIER) && !check(TokenType::MUT) && !check(TokenType::REF) && !check(
                TokenType::RPAREN))
            advance();
    }

    expect(
        TokenType::RPAREN,
        "Unexpected token at function declaration. Found: '" + peek().value + "'. Expected: ')'.",
        "Missing a right paren for closing parameter declaration."
    );

    // Guaranteed expression right now: fn -identifier-(param1, param2, ...-)

    AST::FunctionDef func;
    func.name = funcIdentifier;
    func.parameters = std::move(parameters);

    // Check if the function has a type or if it is a void one.
    if (match(TokenType::ARROW))
    {
        func.type = handleType();
    }

    if (!check(TokenType::LBRACE))
    {
        expect(
            TokenType::LBRACE,
            "Unexpected token at function declaration. Found: '" + peek().value + "'. Expected: '{'.",
            "Missing arrow for function type declaration."
        );
    }

    func.body = handleScope(ScopeType::FUNCTION, func.name.value);

    return func;
}

AST::StructDecl Parser::handleStructDecl()
{
    expect(
        TokenType::STRUCT,
        "Unexpected token at struct declaration. Found: '" + peek().value + "'. Expected: 'struct'."
    );

    const Token structIdentifier = expect(
        TokenType::IDENTIFIER,
        "Unexpected token at struct declaration. Found: '" + peek().value + "'. Expected: An identifier.",
        "Missing a name for the struct."
    );

    AST::StructDecl structDecl;
    structDecl.name = structIdentifier;

    expect(
        TokenType::LBRACE,
        "Unexpected token at struct declaration. Found: '" + peek().value + "'. Expected: '{'.",
        "Missing a left brace for struct fields definition."
    );

    // Guaranteed expression right now: struct -identifier-{

    bool firstRun = true;
    std::vector<AST::VarDecl> fields;

    while (!check(TokenType::RBRACE) && !isEof())
    {
        AST::VarDecl varDecl;

        varDecl.name = expect(
            TokenType::IDENTIFIER,
            "Unexpected token at struct declaration. Found: '" + peek().value + "'. Expected: An identifier.",
            firstRun ? "The struct must have at least one field." : "Missing a name for the next field."
        );

        expect(
            TokenType::COLON,
            "Unexpected token at struct declaration. Found: '" + peek().value + "'. Expected: ':'.",
            "Missing colon for field type attribution."
        );

        varDecl.type = handleType();

        if (match(TokenType::ASSIGN))
        {
            varDecl.initializer = std::make_unique<AST::Node>(handleExpr());
        }

        fields.push_back(std::move(varDecl));

        if (check(TokenType::RBRACE))
            break;

        expect(
            TokenType::COMMA,
            "Unexpected token at struct declaration. Found: '" + peek().value + "'. Expected: ',' or '}'.",
            "Missing a right brace to close struct fields scope."
        );

        if (!check(TokenType::IDENTIFIER) && !check(TokenType::RBRACE))
            advance();

        firstRun = false;
    }

    expect(
        TokenType::RBRACE,
        "Unexpected token at struct declaration. Found: '" + peek().value + "'. Expected: '}'.",
        "Missing a right brace to close struct fields scope."
    );

    structDecl.fields = std::move(fields);

    return structDecl;
}

AST::EnumDecl Parser::handleEnumDecl()
{
    expect(
        TokenType::ENUM,
        "Unexpected token at enum declaration. Found: '" + peek().value + "'. Expected: 'enum'."
    );

    const Token enumIdentifier = expect(
        TokenType::IDENTIFIER,
        "Unexpected token at enum declaration. Found: '" + peek().value + "'. Expected: An identifier.",
        "Missing a name for enumeration."
    );

    AST::EnumDecl enumDecl;
    enumDecl.name = enumIdentifier;

    expect(
        TokenType::LBRACE,
        "Unexpected token at enum declaration. Found: '" + peek().value + "'. Expected: '{'.",
        "Missing left brace for enumeration definition."
    );

    // Guaranteed expression: enum -identifier-{

    bool firstRun = true;
    std::vector<Token> members;

    while (!check(TokenType::RBRACE) && !isEof())
    {
        members.push_back(expect(
            TokenType::IDENTIFIER,
            "Unexpected token at enum declaration. Found: '" + peek().value + "'. Expected: An identifier.",
            firstRun ? "Enums must have at least one member." : "Missing a name for this member."
        ));

        if (check(TokenType::RBRACE))
            break;

        expect(
            TokenType::COMMA,
            "Unexpected token at enum declaration. Found: '" + peek().value + "'. Expected: ',' or '}'.",
            "Missing a right brace to close enum members scope."
        );

        if (!check(TokenType::IDENTIFIER) && !check(TokenType::RBRACE))
            advance();

        firstRun = false;
    }

    expect(
        TokenType::RBRACE,
        "Unexpected token at enum declaration. Found: '" + peek().value + "'. Expected: '}'.",
        "Missing a right brace to close enum members scope."
    );

    enumDecl.members = std::move(members);
    return enumDecl;
}

AST::Scope Parser::handleScope(const ScopeType &scopeOf, const std::string &functionName)
{
    std::string scopeName;
    switch (scopeOf)
    {
        case ScopeType::FUNCTION:
            scopeName = "function-'" + functionName + "'";
            break;
        case ScopeType::IF:
            scopeName = "if";
            break;
        case ScopeType::WHILE:
            scopeName = "while";
            break;
        case ScopeType::FOR:
            scopeName = "for";
            break;
        case ScopeType::DO:
            scopeName = "do-while";
            break;
        default:
            scopeName = "error-undefined-scope-type";
            break;
    }

    expect(
        TokenType::LBRACE,
        "Unexpected token inside " + scopeName + " scope definition. Found: '" + peek().value + "'. Expected: '{'."
    );

    AST::Scope currentScope;

    while (!check(TokenType::RBRACE) && !isEof())
    {
        const Token currentToken = peek();

        switch (currentToken.type)
        {
            case TokenType::IDENTIFIER:
                // Function call.
                if (peekNext().type == TokenType::LPAREN)
                {
                    currentScope.statements.push_back(std::make_unique<AST::Node>(handleFnCall(), currentToken.line));
                }
                // Assign to a variable.
                else
                {
                    currentScope.statements.push_back(std::make_unique<AST::Node>(handleAssign(), currentToken.line));
                }
                break;

            case TokenType::VAR:
                currentScope.statements.push_back(std::make_unique<AST::Node>(handleVarDecl(), currentToken.line));
                break;

            case TokenType::IF:
                currentScope.statements.push_back(std::make_unique<AST::Node>(handleIfStatement(), currentToken.line));
                break;

            case TokenType::WHILE:
                currentScope.statements.push_back(
                    std::make_unique<AST::Node>(handleWhileStatement(), currentToken.line));
                break;

            case TokenType::FOR:
                currentScope.statements.push_back(std::make_unique<AST::Node>(handleForStatement(), currentToken.line));
                break;

            case TokenType::DO:
                currentScope.statements.push_back(std::make_unique<AST::Node>(handleDoStatement(), currentToken.line));
                break;

            case TokenType::SWITCH:
                currentScope.statements.push_back(
                    std::make_unique<AST::Node>(handleSwitchStatement(), currentToken.line));
                break;

            case TokenType::RETURN:
                currentScope.statements.push_back(
                    std::make_unique<AST::Node>(handleReturnStatement(), currentToken.line));
                break;

            default:
                handleScopeError(currentToken, scopeName);
                advance();
                break;
        }
    }

    expect(
        TokenType::RBRACE,
        "Unexpected token inside " + scopeName + " scope definition. Found: '" + peek().value + "'. Expected: '}'.",
        "Missing a right brace to close the scope."
    );

    return currentScope;
}

void Parser::handleScopeError(const Token &actual, const std::string &scopeKind)
{
    const std::string message = "Unexpected token inside " + scopeKind + " definition. Found: '" + actual.value +
                                "'. Expected: A function call, variable assign, variable declaration or a condition/loop block.";
    std::string tips;
    switch (actual.type)
    {
        case TokenType::GLOBAL:
            // The user is probably trying to create a global scope.
            tips = "Global scope declaration must be outside of a function.";
            break;

        case TokenType::STRUCT:
            tips = "Struct declaration must be outside of a function.";
            break;

        case TokenType::ENUM:
            tips = "Enum declaration must be outside of a function.";
            break;

        case TokenType::LBRACE:
            // The user probably forgot to use a keyword for creating a scope.
            tips = "Leftover left brace. Check code above for missing/extra braces.";
            break;

        case TokenType::EoF:
            // The user probably forgot to use a keyword for creating a scope.
            tips = "Missing a right brace to close the scope.";
            break;

        default:
            tips = "Invalid initial instruction token.";
            break;
    }

    errors.add(ErrorPhase::Parser, message, actual.line, actual.type == TokenType::EoF, tips);
}

AST::Node Parser::handleSingleLiner(const ScopeType &scopeOf)
{
    Token currentToken = peek();
    if (scopeOf == ScopeType::FUNCTION)
    {
        expect(
            TokenType::null,
            "Unexpected scope type for single line instruction definition.",
            "Single line instructions aren't allowed for functions."
        );
    }

    std::string scopeName;
    switch (scopeOf)
    {
        case ScopeType::IF:
            scopeName = "if";
            break;
        case ScopeType::WHILE:
            scopeName = "while";
            break;
        case ScopeType::FOR:
            scopeName = "for";
            break;
        case ScopeType::DO:
            scopeName = "do-while";
            break;
        default:
            scopeName = "error-undefined-scope-type";
            break;
    }

    switch (currentToken.type)
    {
        case TokenType::IDENTIFIER:
            // Function call with no attribution.
            if (peekNext().type == TokenType::LPAREN)
            {
                return AST::Node(handleFnCall(), currentToken.line);
            }
            // Assign to a variable.
            else
            {
                return AST::Node(handleAssign(), currentToken.line);
            }

        case TokenType::VAR:
            return AST::Node(handleVarDecl(), currentToken.line);

        case TokenType::IF:
            return AST::Node(handleIfStatement(), currentToken.line);

        case TokenType::WHILE:
            return AST::Node(handleWhileStatement(), currentToken.line);

        case TokenType::FOR:
            return AST::Node(handleForStatement(), currentToken.line);

        case TokenType::DO:
            return AST::Node(handleDoStatement(), currentToken.line);

        case TokenType::SWITCH:
            return AST::Node(handleSwitchStatement(), currentToken.line);

        case TokenType::RETURN:
            return AST::Node(handleReturnStatement(), currentToken.line);

        case TokenType::BREAK:
            if (scopeOf == ScopeType::FOR || scopeOf == ScopeType::DO || scopeOf == ScopeType::WHILE)
                return AST::Node(handleBreakStatement(), currentToken.line);

            expect(
                TokenType::null,
                "Unexpected first token at '" + scopeName + "' definition. Found: '" +
                currentToken.value +
                "'. Expected: A function call, variable assign, variable declaration or a condition/loop block.",
                "Must be inside of a loop block to use the 'break' keyword."
            );
            break;

        default:
            expect(
                TokenType::IDENTIFIER,
                "Unexpected first token at '" + scopeName + "' definition. Found: '" +
                currentToken.value +
                "'. Expected: A function call, variable assign, variable declaration or a condition/loop block.",
                "Invalid initial instruction token."
            );
            break;
    }

    advance();
    return AST::Node(AST::BadExpression(currentToken), currentToken.line);
}

AST::If Parser::handleIfStatement()
{
    const Token ifText = peek();

    expect(
        TokenType::IF,
        "Unexpected token at if statement declaration. Found: '" + ifText.value + "'. Expected: 'if'."
    );

    expect(
        TokenType::LPAREN,
        "Unexpected token at if statement declaration. Found: '" + peek().value + "'. Expected: '('.",
        "Missing left parenthesis for declaring the condition."
    );

    AST::If ifNode;
    ifNode.condition = std::make_unique<AST::Node>(handleExpr()); // Expected to stop at a right paren.

    if (check(TokenType::LBRACE))
    {
        expect(
            TokenType::RPAREN,
            "Unexpected token at if statement declaration. Found: '" + peek().value + "'. Expected: ')'.",
            "Missing a right parenthesis to finish the condition declaration."
        );
    } else if (!check(TokenType::RPAREN))
    {
        expect(
            TokenType::RPAREN,
            "Unexpected token at if statement declaration. Found: '" + peek().value + "'. Expected: An operator.",
            "Missing an operator between the elements of the expression."
        );
    }

    expect(TokenType::RPAREN);
    // Stopped at either a right brace or a semi-colon.
}
