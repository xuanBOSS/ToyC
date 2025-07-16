// typeVisitor.cpp - 实现类型检查访问者
#include "typeVisitor.h"
#include "analyzeVisitor.h"

// 检查类型兼容性
bool typeVisitor::isTypeCompatible(const std::string& sourceType, const std::string& targetType) {
    // 在这个简单的语言中，类型系统相对简单
    // 目前只有 int 和 void 类型，并且 void 只能作为函数返回类型
    return sourceType == targetType;
}
// 数字字面量总是int类型
void typeVisitor::visit(NumberExpr &) { type = "int"; }
// 变量引用的类型就是变量的声明类型
void typeVisitor::visit(VariableExpr &expr)
{
    Symbol *symbol = owner.helper.findSymbol(expr.name);
    if (!symbol) {
        // 如果符号不存在，设置为错误类型
        type = "error";
        return;
    }
    
    // 标记变量为已使用
    symbol->used = true;
    type = symbol->type;
}
// 二元表达式的类型检查
void typeVisitor::visit(BinaryExpr &expr)
{
    expr.left->accept(*this);
    std::string leftType = type;
    expr.right->accept(*this);
    std::string rightType = type;
    if (leftType != "int" || rightType != "int")
    {
        owner.helper.error("Binary operator '" + expr.op + "' requires integer operands", expr.line, expr.column);
        type = "error";
    }
    else
    {
        type = "int";
    }
}
// 一元表达式的类型检查
void typeVisitor::visit(UnaryExpr &expr)
{
    expr.operand->accept(*this);
    if (type != "int")
    {
        owner.helper.error("Unary operator '" + expr.op + "' requires integer operand", expr.line, expr.column);
        type = "error";
    }
    else
    {
        type = "int";
    }
}
// 函数调用表达式的类型检查
void typeVisitor::visit(CallExpr &expr)
{
    auto it = owner.getFunctionTable().find(expr.callee);
    if (it == owner.getFunctionTable().end()) {
        // 如果函数未定义，设置为错误类型
        type = "error";
        return;
    }
    
    // 标记函数为已使用
    Symbol *symbol = owner.helper.findSymbol(expr.callee);
    if (symbol && symbol->kind == Symbol::Kind::FUNCTION) {
        symbol->used = true;
    }
    
    // 增强参数类型检查
    if (expr.arguments.size() != it->second.paramTypes.size()) {
        owner.helper.error("Incorrect number of arguments for function '" + expr.callee + "'", expr.line, expr.column);
        type = it->second.returnType; // 尽管有错误，仍返回函数的返回类型
        return;
    }
    
    for (size_t i = 0; i < expr.arguments.size(); ++i)
    {
        expr.arguments[i]->accept(*this);
        std::string argType = type;
        
        if (!isTypeCompatible(argType, it->second.paramTypes[i]))
        {
            owner.helper.error("Function '" + expr.callee + "' argument " + std::to_string(i+1) + 
                           " type mismatch", expr.line, expr.column);
        }
    }
    
    type = it->second.returnType;
}

// === 以下是语句节点的访问方法（主要用于完整性） ===

// 表达式语句的类型检查
void typeVisitor::visit(ExprStmt &stmt)
{
    if (stmt.expression)
        stmt.expression->accept(*this);
}
// 变量声明语句的类型检查
void typeVisitor::visit(VarDeclStmt &stmt)
{
    if (stmt.initializer){
        stmt.initializer->accept(*this);

        // 检查初始化表达式类型
        if (type != "int") {
            owner.helper.error("Cannot initialize integer variable with non-integer expression", stmt.line, stmt.column);
        }
    }
    
    // 变量声明总是返回 void
    type = "void";
}
// 赋值语句的类型检查
void typeVisitor::visit(AssignStmt &stmt)
{
    // 获取变量类型
    Symbol *symbol = owner.helper.findSymbol(stmt.name);
    if (!symbol) {
        type = "error";
        return;
    }
    
    // 标记变量为已使用
    symbol->used = true;
    
    // 检查赋值表达式类型
    stmt.value->accept(*this);
    std::string valueType = type;
    
    if (!isTypeCompatible(valueType, symbol->type)) {
        owner.helper.error("Assignment type mismatch: variable '" + stmt.name + "' has type '" + 
                       symbol->type + "', expression has type '" + valueType + "'", 
                       stmt.line, stmt.column);
    }
    
    // 赋值语句总是返回 void
    type = "void";
}
// 语句块的类型检查
void typeVisitor::visit(BlockStmt &stmt)
{
    for (auto &s : stmt.statements)
        s->accept(*this);
    
    // 代码块总是返回 void
    type = "void";
}
// if语句的类型检查
void typeVisitor::visit(IfStmt &stmt)
{
    stmt.condition->accept(*this);
    if (type != "int") {
        owner.helper.error("If condition must be integer (used as boolean)", stmt.line, stmt.column);
    }
    
    stmt.thenBranch->accept(*this);
    if (stmt.elseBranch)
        stmt.elseBranch->accept(*this);
    
    // if语句总是返回 void
    type = "void";
}
// while语句的类型检查
void typeVisitor::visit(WhileStmt &stmt)
{
    stmt.condition->accept(*this);
    if (type != "int") {
        owner.helper.error("While condition must be integer (used as boolean)", stmt.line, stmt.column);
    }
    
    stmt.body->accept(*this);
    
    // while语句总是返回 void
    type = "void";
}
// break语句的类型检查
void typeVisitor::visit(BreakStmt &) {
    // break语句总是返回 void
    type = "void";
}
// continue语句的类型检查
void typeVisitor::visit(ContinueStmt &) {
    // continue语句总是返回 void
    type = "void";
}
// return语句的类型检查
void typeVisitor::visit(ReturnStmt &stmt)
{
    if (stmt.value) {
        stmt.value->accept(*this);
        // 返回值类型检查在analyzeVisitor中处理
    } else {
        // 空return语句返回void
        type = "void";
    }
}
// 函数定义的类型检查
void typeVisitor::visit(FunctionDef &funcDef)
{
    funcDef.body->accept(*this);
    // 函数定义总是返回 void
    type = "void";
}
// 编译单元的类型检查
void typeVisitor::visit(CompUnit &compUnit)
{
    for (auto &func : compUnit.functions)
        func->accept(*this);
    
    // 编译单元总是返回 void
    type = "void";
}
