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
    Token currentToken = peek();
    while (currentToken.type != TokenType::EoF)
    // Root scope for function/struct/enum/global definitions. Everything else raises an error.
    {
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
        node->startLine = currentToken.line;
        ast.push_back(std::move(node));
        currentToken = consume();
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

    Error::raise(Error::Phase::Parser, message, actual.line, tips);
}

AST::GlobalBlock Parser::handleGlobal()
{
    const Token global = peek();
    if (global.type != TokenType::GLOBAL)
    {
        const std::string message = "Unexpected token at global declaration. Found: '" + global.value +
                                    "'. Expected: 'global'.";
        Error::raise(Error::Phase::Parser, message, global.line);
    }

    const Token lbrace = consume();
    if (lbrace.type != TokenType::RBRACE)
    {
        const std::string message = "Unexpected token at global declaration. Found: '" + lbrace.value +
                                    "'. Expected: '{'.";
        Error::raise(Error::Phase::Parser, message, lbrace.line, "Missing a left brace to open the global scope.");
    }
    AST::GlobalBlock globalNode;
    Token var = consume();
    while (var.type != TokenType::RBRACE && var.type != TokenType::EoF)
    {
        if (var.type != TokenType::VAR)
        {
            const std::string message = "Unexpected token at global declaration. Found: '" + lbrace.value +
                                        "'. Expected: 'var'.";
            Error::raise(Error::Phase::Parser, message, lbrace.line,
                         "Global block only supports variable declarations. Any other action needs to happen in other scopes.");
        }
        globalNode.declarations.push_back(handleVarDecl());
        var = consume();
    }
    return globalNode;
}

AST::FunctionDef Parser::handleFuncDecl()
{
    const Token fn = peek();
    if (fn.type != TokenType::FUNCTION)
    {
        const std::string message = "Unexpected token at function declaration. Found: '" + fn.value +
                                    "'. Expected: 'fn'.";
        Error::raise(Error::Phase::Parser, message, fn.line);
    }

    const Token funcIdentifier = consume();
    if (funcIdentifier.type != TokenType::IDENTIFIER)
    {
        const std::string message = "Unexpected token at function declaration. Found: '" + funcIdentifier.value +
                                    "'. Expected: An Identifier.";
        Error::raise(Error::Phase::Parser, message, funcIdentifier.line,
                     "The name of the function must be an unreserved word with no symbols.");
    }

    const Token paramLParen = consume();
    if (paramLParen.type != TokenType::LPAREN)
    {
        const std::string message = "Unexpected token at function declaration. Found: '" + paramLParen.value +
                                    "'. Expected: '('.";
        Error::raise(Error::Phase::Parser, message, paramLParen.line,
                     "Missing left parenthesis for parameter declarations.");
    }

    // Guaranteed expression right now: fn -identifier-(

    std::vector<AST::Parameter> parameters;
    bool foundParen = consume().type == TokenType::RPAREN;
    // Looping for parameters:
    while (!foundParen)
    {
        AST::Parameter param;
        Token currentToken = peek();
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
                                        "'. Expected: An identifier, 'mut' or 'ref'.";
            Error::raise(Error::Phase::Parser, message, currentToken.line,
                         "The name for a parameter must be an unreserved word with no symbols.");
        }
        param.name = currentToken;
        currentToken = consume();
        if (currentToken.type != TokenType::COLON)
        {
            const std::string message = "Unexpected token at function declaration. Found: '" + currentToken.value +
                                        "'. Expected: ':'.";
            Error::raise(Error::Phase::Parser, message, currentToken.line,
                         "Missing colon for type declaration.");
        }
        param.type = handleType();
        currentToken = consume();
        if (currentToken.type == TokenType::ASSIGN)
        {
            consume();
            param.initializer = std::make_unique<AST::Node>(handleExpr());
        }
        parameters.push_back(std::move(param));
        currentToken = consume();
        if (currentToken.type == TokenType::RPAREN)
            foundParen = true;
        else if (currentToken.type != TokenType::COMMA)
        {
            if (currentToken.type == TokenType::LBRACE)
            {
                const std::string message = "Unexpected token at function declaration. Found: '" + currentToken.value +
                                            "'. Expected: ')'.";
                Error::raise(Error::Phase::Parser, message, currentToken.line,
                             "Missing a right paren for closing parameter declaration.");
            } else
            {
                const std::string message = "Unexpected token at function declaration. Found: '" + currentToken.value +
                                            "'. Expected: ',' or ')'.";
                Error::raise(Error::Phase::Parser, message, currentToken.line,
                             "Either missing a comma for next parameter or missing a right paren for closing parameter declaration.");
            }
        }
        consume();
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
    } else
    {
        const std::string message = "Unexpected token at function declaration. Found: '" + lbrace.value +
                                    "'. Expected: '{'.";
        Error::raise(Error::Phase::Parser, message, lbrace.line,
                     "Missing arrow for function type declaration.");
    }

    func.body = handleScope(ScopeType::FUNCTION, func.name.value);

    return func;
}

AST::StructDecl Parser::handleStructDecl()
{
    const Token structText = peek();
    if (structText.type != TokenType::STRUCT)
    {
        const std::string message = "Unexpected token at struct declaration. Found: '" + structText.value +
                                    "'. Expected: 'struct'.";
        Error::raise(Error::Phase::Parser, message, structText.line);
    }

    const Token structIdentifier = consume();
    if (structIdentifier.type != TokenType::IDENTIFIER)
    {
        const std::string message = "Unexpected token at struct declaration. Found: '" + structIdentifier.value +
                                    "'. Expected: An identifier.";
        Error::raise(Error::Phase::Parser, message, structIdentifier.line,
                     "Missing a name for the struct.");
    }

    AST::StructDecl structDecl;
    structDecl.name = structIdentifier;

    const Token leftBrace = consume();
    if (leftBrace.type != TokenType::LBRACE)
    {
        const std::string message = "Unexpected token at struct declaration. Found: '" + leftBrace.value +
                                    "'. Expected: '{'.";
        Error::raise(Error::Phase::Parser, message, leftBrace.line,
                     "Missing a left brace for struct fields definition.");
    }

    // Guaranteed expression right now: struct -identifier-{

    // Looping through the struct fields.
    bool foundRBrace = false;
    bool firstRun = true;
    std::vector<AST::VarDecl> fields;
    while (!foundRBrace)
    {
        AST::VarDecl varDecl;
        Token currentToken = consume();
        if (currentToken.type != TokenType::IDENTIFIER)
        {
            const std::string message = "Unexpected token at struct declaration. Found: '" + currentToken.value +
                                        "'. Expected: An identifier.";
            if (firstRun)
                Error::raise(Error::Phase::Parser, message, currentToken.line,
                             "The struct must have at least one field.");
            else
                Error::raise(Error::Phase::Parser, message, currentToken.line,
                             "Missing a name for the next field.");
        }
        varDecl.name = currentToken;

        currentToken = consume();
        if (currentToken.type != TokenType::COLON)
        {
            const std::string message = "Unexpected token at struct declaration. Found: '" + currentToken.value +
                                        "'. Expected: ':'.";
            Error::raise(Error::Phase::Parser, message, currentToken.line,
                         "Missing colon for field type attribution.");
        }

        currentToken = consume();
        varDecl.type = handleType();

        currentToken = consume();
        if (currentToken.type == TokenType::ASSIGN)
        {
            consume();
            varDecl.initializer = std::make_unique<AST::Node>(handleExpr());
            currentToken = consume();
        }
        fields.push_back(std::move(varDecl));

        if (currentToken.type == TokenType::RBRACE)
        {
            foundRBrace = true;
        } else if (currentToken.type != TokenType::COMMA)
        {
            const std::string message = "Unexpected token at struct declaration. Found: '" + currentToken.value +
                                        "'. Expected: ',' or '}'.";
            Error::raise(Error::Phase::Parser, message, currentToken.line,
                         "Missing a right brace to close struct fields scope.");
        }
        firstRun = false;
    }

    structDecl.fields = std::move(fields);

    return structDecl;
}

AST::EnumDecl Parser::handleEnumDecl()
{
    const Token enumText = consume();
    if (enumText.type != TokenType::ENUM)
    {
        const std::string message = "Unexpected token at enum declaration. Found: '" + enumText.value +
                                    "'. Expected: 'enum'.";
        Error::raise(Error::Phase::Parser, message, enumText.line);
    }

    const Token enumIdentifier = consume();
    if (enumIdentifier.type != TokenType::IDENTIFIER)
    {
        const std::string message = "Unexpected token at enum declaration. Found: '" + enumText.value +
                                    "'. Expected: An identifier.";
        Error::raise(Error::Phase::Parser, message, enumText.line,
                     "Missing a name for enumeration.");
    }

    AST::EnumDecl enumDecl;
    enumDecl.name = enumIdentifier;

    const Token lbrace = consume();
    if (lbrace.type != TokenType::LBRACE)
    {
        const std::string message = "Unexpected token at enum declaration. Found: '" + lbrace.value +
                                    "'. Expected: '{'.";
        Error::raise(Error::Phase::Parser, message, lbrace.line,
                     "Missing left brace for enumeration definition.");
    }

    // Guaranteed expression: enum -identifier-{

    bool foundRBrace = false;
    bool firstRun = true;
    std::vector<Token> members;
    while (!foundRBrace)
    {
        const Token member = consume();
        if (member.type != TokenType::IDENTIFIER)
        {
            const std::string message = "Unexpected token at enum declaration. Found: '" + member.value +
                                        "'. Expected: An identifier.";
            if (firstRun)
                Error::raise(Error::Phase::Parser, message, member.line,
                             "Enums must have at least one member.");

            Error::raise(Error::Phase::Parser, message, member.line,
                         "Missing a name for this member.");
        }

        members.push_back(member);

        const Token rbraceOrComma = consume();
        if (rbraceOrComma.type == TokenType::RBRACE)
        {
            foundRBrace = true;
        } else if (rbraceOrComma.type != TokenType::COMMA)
        {
            const std::string message = "Unexpected token at enum declaration. Found: '" + rbraceOrComma.value +
                                        "'. Expected: ',' or '}'.";
            Error::raise(Error::Phase::Parser, message, rbraceOrComma.line,
                         "Missing a right brace to close enum members scope.");
        }

        firstRun = false;
    }

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

    const Token lbrace = peek();
    if (lbrace.type != TokenType::LBRACE)
    {
        const std::string message = "Unexpected token inside " + scopeName + " scope definition. Found: '" + lbrace.
                                    value +
                                    "'. Expected: '{'.";
        Error::raise(Error::Phase::Parser, message, lbrace.line);
    }

    Token currentToken = consume();
    AST::Scope currentScope;
    while (currentToken.type != TokenType::EoF && currentToken.type != TokenType::RBRACE)
    {
        switch (currentToken.type)
        {
            case TokenType::IDENTIFIER:
                currentToken = consume();
                // Function call.
                if (currentToken.type == TokenType::LPAREN)
                {
                    previous(); // back to the identifier.
                    currentScope.statements.push_back(std::make_unique<AST::Node>(handleFnCall(), currentToken.line));
                }
                // Assign to a variable.
                else
                {
                    previous();
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
                break;
        }
        currentToken = peek();
        // Increment until hitting '}'.
        if (currentToken.type != TokenType::RBRACE)
            currentToken = consume();
    }

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

    Error::raise(Error::Phase::Parser, message, actual.line, tips);
}

AST::Node Parser::handleSingleLiner(const ScopeType &scopeOf)
{
    Token currentToken = peek();
    if (scopeOf == ScopeType::FUNCTION)
    {
        const std::string message = "Unexpected scope type for single line instruction definition.";
        Error::raise(Error::Phase::Parser, message, currentToken.line,
                     "Single line instructions aren't allowed for functions.");
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
            currentToken = consume();
            // Function call with no attribution.
            if (currentToken.type == TokenType::LPAREN)
            {
                previous(); // back to the identifier.
                return AST::Node(handleFnCall(), currentToken.line);
            }
            // Assign to a variable.
            else
            {
                previous();
                return AST::Node(handleAssign(), currentToken.line);
            }
            break;

        case TokenType::VAR:
            return AST::Node(handleVarDecl(), currentToken.line);
            break;

        case TokenType::IF:
            return AST::Node(handleIfStatement(), currentToken.line);
            break;

        case TokenType::WHILE:
            return AST::Node(handleWhileStatement(), currentToken.line);
            break;

        case TokenType::FOR:
            return AST::Node(handleForStatement(), currentToken.line);
            break;

        case TokenType::DO:
            return AST::Node(handleDoStatement(), currentToken.line);
            break;

        case TokenType::SWITCH:
            return AST::Node(handleSwitchStatement(), currentToken.line);
            break;

        case TokenType::RETURN:
            return AST::Node(handleReturnStatement(), currentToken.line));
            break;

        case TokenType::BREAK:
            if (scopeOf == ScopeType::FOR || scopeOf == ScopeType::DO || scopeOf == ScopeType::WHILE)
                return AST::Node(handleBreakStatement(), currentToken.line);
            else
            {
                const std::string message = "Unexpected first token at '" + scopeName + "' definition. Found: '" +
                                        currentToken
                                        .value + "'. Expected: A function call, variable assign, variable declaration or a condition/loop block.";
                Error::raise(Error::Phase::Parser, message, currentToken.line,
                             "Must be inside of a loop block to use the 'break' keyword.");
            }
            break;

        default:
            const std::string message = "Unexpected first token at '" + scopeName + "' definition. Found: '" +
                                        currentToken
                                        .value + "'. Expected: A function call, variable assign, variable declaration or a condition/loop block.";
            Error::raise(Error::Phase::Parser, message, currentToken.line,
                         "Invalid initial instruction token.");
            break;
    }

    return AST::Node(AST::BadExpression(), currentToken.line);
}

AST::If Parser::handleIfStatement()
{
    const Token ifText = peek();

    if (ifText.type != TokenType::IF)
    {
        const std::string message = "Unexpected token at if statement declaration. Found: '" + ifText.value +
                                    "'. Expected: 'if'.";
        Error::raise(Error::Phase::Parser, message, ifText.line);
    }

    const Token lparen = consume();
    if (lparen.type != TokenType::RPAREN)
    {
        const std::string message = "Unexpected token at if statement declaration. Found: '" + lparen.value +
                                    "'. Expected: '('.";
        Error::raise(Error::Phase::Parser, message, lparen.line,
                     "Missing left parenthesis for declaring the condition.");
    }
    consume();

    AST::If ifNode;
    ifNode.condition = std::make_unique<AST::Node>(handleExpr()); // Expected to stop at a right paren.

    const Token rparen = peek();
    if (rparen.type != TokenType::RPAREN)
    {
        if (rparen.type == TokenType::LBRACE)
        {
            const std::string message = "Unexpected token at if statement declaration. Found: '" + lparen.value +
                                        "'. Expected: ')'.";
            Error::raise(Error::Phase::Parser, message, lparen.line,
                         "Missing a right parenthesis to finish the condition declaration.");
        } else
        {
            const std::string message = "Unexpected token at if statement declaration. Found: '" + lparen.value +
                                        "'. Expected: An operator.";
            Error::raise(Error::Phase::Parser, message, lparen.line,
                         "Missing an operator between the elements of the expression.");
        }
    }

    const Token lbrace = consume();
    if (lbrace.type == TokenType::LBRACE)
        ifNode.thenBranch = std::make_unique<AST::Node>(handleScope(ScopeType::IF), lbrace.line);
    else
        ifNode.thenBranch = std::make_unique<AST::Node>(handleSingleLiner(ScopeType::IF));
    // Stopped at either a right brace or a semi-colon.
}
