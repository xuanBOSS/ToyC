// ASTVisitor.h
#pragma once
#include "ast.h"

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    // 表达式
    virtual void visit(NumberExpr& expr) = 0;
    virtual void visit(VariableExpr& expr) = 0;
    virtual void visit(BinaryExpr& expr) = 0;
    virtual void visit(UnaryExpr& expr) = 0;
    virtual void visit(CallExpr& expr) = 0;
    
    // 语句
    virtual void visit(ExprStmt& stmt) = 0;
    virtual void visit(VarDeclStmt& stmt) = 0;
    virtual void visit(AssignStmt& stmt) = 0;
    virtual void visit(BlockStmt& stmt) = 0;
    virtual void visit(IfStmt& stmt) = 0;
    virtual void visit(WhileStmt& stmt) = 0;
    virtual void visit(BreakStmt& stmt) = 0;
    virtual void visit(ContinueStmt& stmt) = 0;
    virtual void visit(ReturnStmt& stmt) = 0;
    
    // 其他节点
    virtual void visit(FunctionDef& funcDef) = 0;
    virtual void visit(CompUnit& compUnit) = 0;
};

