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
    std::vector<std::shared_ptr<Function>> functions;
};

} // namespace ast

