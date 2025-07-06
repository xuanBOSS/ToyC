#pragma once
#include "parser/astVisitor.h"
#include <string>

class analyzeVisitor;

class typeVisitor : public ASTVisitor
{
private:
    analyzeVisitor &owner;

public:
    explicit typeVisitor(analyzeVisitor &owner) : owner(owner) {}

    std::string type;

    // 把类型返回给外部
    std::string getExprType(Expr &expr){
        expr.accept(*this);
        return type;
    }

    // 表达式
    void visit(NumberExpr &expr) override;
    void visit(VariableExpr &expr) override;
    void visit(BinaryExpr &expr) override;
    void visit(UnaryExpr &expr) override;
    void visit(CallExpr &expr) override;
    // 语句
    void visit(ExprStmt &stmt) override;
    void visit(VarDeclStmt &stmt) override;
    void visit(AssignStmt &stmt) override;
    void visit(BlockStmt &stmt) override;
    void visit(IfStmt &stmt) override;
    void visit(WhileStmt &stmt) override;
    void visit(BreakStmt &stmt) override;
    void visit(ContinueStmt &stmt) override;
    void visit(ReturnStmt &stmt) override;
    // 其它
    void visit(FunctionDef &funcDef) override;
    void visit(CompUnit &compUnit) override;
};
