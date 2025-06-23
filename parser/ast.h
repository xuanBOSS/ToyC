#pragma once
#include <memory>
#include <string>
#include <vector>

// 示例：表达式节点基类
struct Expr {
    virtual ~Expr() = default;
};

using ExprPtr = std::shared_ptr<Expr>;

struct IntLiteral : public Expr {
    int value;
    IntLiteral(int v) : value(v) {}
};
