// typeVisitor.h - 定义了类型检查访问者
#pragma once
#include "parser/astVisitor.h"
#include <string>

class analyzeVisitor;
// typeVisitor - 用于计算表达式类型的访问者类
class typeVisitor : public ASTVisitor
{
private:
    analyzeVisitor &owner;// 指向所有者（语义分析访问者）

public:
    explicit typeVisitor(analyzeVisitor &owner) : owner(owner) {}

    std::string type;// 当前表达式的类型

    // 把类型返回给外部
    std::string getExprType(Expr &expr){
        expr.accept(*this);
        return type;
    }

    // 添加检查类型兼容性的辅助方法
    bool isTypeCompatible(const std::string& sourceType, const std::string& targetType);


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
