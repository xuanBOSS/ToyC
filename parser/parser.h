// parser/parser.h
// Parser.h
#pragma once
#include "lexer/Token.h"
#include "parser/ast.h"
#include <vector>
#include <memory>
#include <stdexcept>

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message) : std::runtime_error(message) {}
};

class Parser {
private:
    std::vector<Token> tokens;
    int current = 0;

public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens) {}
    std::shared_ptr<CompUnit> parse();

private:
    // 辅助方法
    Token peek(int offset) const {
    if (current + offset >= tokens.size()) {
        return tokens.back(); // 返回EOF token
    }
    return tokens[current + offset];
}
    Token previous() const;
    bool isAtEnd() const;
    Token advance();
    bool check(TokenType type) const;
    bool match(std::initializer_list<TokenType> types);
    Token consume(TokenType type, const std::string& message);
    ParseError error(const Token& token, const std::string& message);
    void synchronize();

    // 递归下降解析方法 - 按照文法从上到下实现
    std::shared_ptr<CompUnit> compUnit();
    std::shared_ptr<FunctionDef> funcDef();
    Param param();
    std::shared_ptr<Stmt> stmt();
    std::shared_ptr<BlockStmt> block();
    std::shared_ptr<Stmt> exprStmt();
    std::shared_ptr<Stmt> varDeclStmt();
    std::shared_ptr<Stmt> assignStmt();
    std::shared_ptr<Stmt> ifStmt();
    std::shared_ptr<Stmt> whileStmt();
    std::shared_ptr<Stmt> breakStmt();
    std::shared_ptr<Stmt> continueStmt();
    std::shared_ptr<Stmt> returnStmt();
    
    // 表达式解析方法
    std::shared_ptr<Expr> expr();
    std::shared_ptr<Expr> lorExpr();
    std::shared_ptr<Expr> landExpr();
    std::shared_ptr<Expr> relExpr();
    std::shared_ptr<Expr> addExpr();
    std::shared_ptr<Expr> mulExpr();
    std::shared_ptr<Expr> unaryExpr();
    std::shared_ptr<Expr> primaryExpr();
};

