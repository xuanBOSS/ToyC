#pragma once
#include <string>
#include <memory>
#include <vector>

namespace ast {

struct Expr {
    virtual ~Expr() = default;
};

struct IntLiteral : public Expr {
    int value;
    IntLiteral(int v) : value(v) {}  
};

struct BinaryExpr : public Expr {
    std::string op;
    std::shared_ptr<Expr> lhs, rhs;
};

struct Stmt {
    virtual ~Stmt() = default;
};

struct ReturnStmt : public Stmt {
    std::shared_ptr<Expr> expr;
};

struct Function {
    std::string name;
    std::vector<std::string> params;
    std::vector<std::shared_ptr<Stmt>> body;
};

struct Program {
    std::vector<std::shared_ptr<Function>> functions;  // 存储函数列表
    std::vector<std::shared_ptr<Expr>> expressions;    // 存储全局表达式列表
    Program(
        std::vector<std::shared_ptr<Function>> funcs,
        std::vector<std::shared_ptr<Expr>> exprs
    ) : functions(std::move(funcs)), expressions(std::move(exprs)) {}};
} // namespace ast

