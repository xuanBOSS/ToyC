// AST.cpp - 实现AST节点的访问者模式接口方法
#include "ast.h"
#include "astVisitor.h"
// 各个AST节点类型实现的accept方法
// 这些方法使用访问者模式，将自身传递给访问者对象处理

// 数字表达式节点接受访问者
void NumberExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// 变量表达式节点接受访问者
void VariableExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// 二元表达式节点接受访问者
void BinaryExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// 一元表达式节点接受访问者
void UnaryExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// 函数调用表达式节点接受访问者
void CallExpr::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// 表达式语句节点接受访问者
void ExprStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// 变量声明语句节点接受访问者
void VarDeclStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// 赋值语句节点接受访问者
void AssignStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// 语句块节点接受访问者
void BlockStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// if语句节点接受访问者
void IfStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// while语句节点接受访问者
void WhileStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// break语句节点接受访问者
void BreakStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// continue语句节点接受访问者
void ContinueStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// return语句节点接受访问者
void ReturnStmt::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// 函数定义节点接受访问者
void FunctionDef::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
// 编译单元节点接受访问者
void CompUnit::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
