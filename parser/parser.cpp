#include "parser.h"
#include "ast.h"
#include <iostream>

bool Parser::isAtEnd() const {
    return current >= tokens.size();
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw error(peek(0), message);
}

Token Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek(1).type == type;
}


std::shared_ptr<CompUnit> Parser::parse() {
    try {
        return compUnit();
    } catch (const ParseError& error) {
        // 错误处理
        return nullptr;
    }
}

std::shared_ptr<CompUnit> Parser::compUnit() {
    std::vector<std::shared_ptr<FunctionDef>> functions;
    
    while (!isAtEnd()) {
        functions.push_back(funcDef());
    }
    
    return std::make_shared<CompUnit>(functions);
}

std::shared_ptr<FunctionDef> Parser::funcDef() {
    std::string returnType;
    if (match({TokenType::INT})) {
        returnType = "int";
    } else if (match({TokenType::VOID})) {
        returnType = "void";
    } else {
        throw error(peek(0), "Expected return type 'int' or 'void'.");
    }
    
    Token name = consume(TokenType::IDENTIFIER, "Expected function name.");
    
    consume(TokenType::LPAREN, "Expected '(' after function name.");
    
    std::vector<Param> parameters;
    if (!check(TokenType::RPAREN)) {
        do {
            parameters.push_back(param());
        } while (match({TokenType::COMMA}));
    }
    
    consume(TokenType::RPAREN, "Expected ')' after parameters.");
    
    std::shared_ptr<BlockStmt> body = block();
    
    return std::make_shared<FunctionDef>(returnType, name.lexeme, parameters, body);
}

Param Parser::param() {
    consume(TokenType::INT, "Parameter type must be 'int'.");
    Token name = consume(TokenType::IDENTIFIER, "Expected parameter name.");
    return Param(name.lexeme);
}

std::shared_ptr<BlockStmt> Parser::block() {
    consume(TokenType::LBRACE, "Expected '{' before block.");
    
    std::vector<std::shared_ptr<Stmt>> statements;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        statements.push_back(stmt());
    }
    
    consume(TokenType::RBRACE, "Expected '}' after block.");
    
    return std::make_shared<BlockStmt>(statements);
}

std::shared_ptr<Stmt> Parser::stmt() {
    if (match({TokenType::LBRACE})) {
        current--; // 回退，让block()处理
        return block();
    }
    
    if (match({TokenType::SEMICOLON})) {
        return std::make_shared<ExprStmt>(nullptr); // 空语句
    }
    
    if (match({TokenType::INT})) {
        return varDeclStmt();
    }
    
    if (match({TokenType::IF})) {
        return ifStmt();
    }
    
    if (match({TokenType::WHILE})) {
        return whileStmt();
    }
    
    if (match({TokenType::BREAK})) {
        return breakStmt();
    }
    
    if (match({TokenType::CONTINUE})) {
        return continueStmt();
    }
    
    if (match({TokenType::RETURN})) {
        return returnStmt();
    }
    
    // 检查是否是赋值语句
    if (check(TokenType::IDENTIFIER) && peek(1).type == TokenType::ASSIGN) {
        return assignStmt();
    }
    
    // 默认为表达式语句
    return exprStmt();
}

// 表达式解析示例实现
std::shared_ptr<Expr> Parser::expr() {
    return lorExpr();
}

std::shared_ptr<Expr> Parser::lorExpr() {
    auto expr = landExpr();
    
    while (match({TokenType::OR})) {
        std::string op = previous().lexeme;
        auto right = landExpr();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    
    return expr;
}

// 其他表达式解析方法类似...

std::shared_ptr<Expr> Parser::primaryExpr() {
    if (match({TokenType::NUMBER})) {
        int value = std::stoi(previous().lexeme);
        return std::make_shared<NumberExpr>(value);
    }
    
    if (match({TokenType::IDENTIFIER})) {
        std::string name = previous().lexeme;
        
        // 检查是否是函数调用
        if (match({TokenType::LPAREN})) {
            std::vector<std::shared_ptr<Expr>> arguments;
            
            if (!check(TokenType::RPAREN)) {
                do {
                    arguments.push_back(expr());
                } while (match({TokenType::COMMA}));
            }
            
            consume(TokenType::RPAREN, "Expected ')' after arguments.");
            
            return std::make_shared<CallExpr>(name, arguments);
        }
        
        // 否则是变量引用
        return std::make_shared<VariableExpr>(name);
    }
    
    if (match({TokenType::LPAREN})) {
        auto expression = expr();
        consume(TokenType::RPAREN, "Expected ')' after expression.");
        return expression;
    }
    
    throw error(peek(0), "Expected expression.");
}