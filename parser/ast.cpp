// AST.cpp
#include "ast.h"
#include "astVisitor.h"

void BinaryExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void CallExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void ExprStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void BlockStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void FunctionDef::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void CompUnit::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void NumberExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void VariableExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// 其他节点的accept方法实现...