// ASTVisitor.h - 定义了AST访问者接口
#pragma once
#include "ast.h"
// ASTVisitor类 - 抽象基类，定义了访问AST各种节点的接口
// 使用访问者模式，允许在不修改节点类的情况下扩展对节点的操作
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    // 表达式节点的访问方法
    virtual void visit(NumberExpr& expr) = 0;     // 访问数字表达式
    virtual void visit(VariableExpr& expr) = 0;   // 访问变量表达式
    virtual void visit(BinaryExpr& expr) = 0;     // 访问二元表达式
    virtual void visit(UnaryExpr& expr) = 0;      // 访问一元表达式
    virtual void visit(CallExpr& expr) = 0;       // 访问函数调用表达式
    
    // 语句节点的访问方法
    virtual void visit(ExprStmt& stmt) = 0;       // 访问表达式语句
    virtual void visit(VarDeclStmt& stmt) = 0;    // 访问变量声明语句
    virtual void visit(AssignStmt& stmt) = 0;     // 访问赋值语句
    virtual void visit(BlockStmt& stmt) = 0;      // 访问语句块
    virtual void visit(IfStmt& stmt) = 0;         // 访问if语句
    virtual void visit(WhileStmt& stmt) = 0;      // 访问while语句
    virtual void visit(BreakStmt& stmt) = 0;      // 访问break语句
    virtual void visit(ContinueStmt& stmt) = 0;   // 访问continue语句
    virtual void visit(ReturnStmt& stmt) = 0;     // 访问return语句
    
    // 其他节点的访问方法
    virtual void visit(FunctionDef& funcDef) = 0; // 访问函数定义
    virtual void visit(CompUnit& compUnit) = 0;   // 访问编译单元
};

