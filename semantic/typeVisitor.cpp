#include "typeVisitor.h"
#include "analyzeVisitor.h"

// 检查类型兼容性
bool typeVisitor::isTypeCompatible(const std::string& sourceType, const std::string& targetType) {
    // 在这个简单的语言中，类型系统相对简单
    // 目前只有 int 和 void 类型，并且 void 只能作为函数返回类型
    return sourceType == targetType;
}

void typeVisitor::visit(NumberExpr &) { type = "int"; }

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

void typeVisitor::visit(BinaryExpr &expr)
{
    expr.left->accept(*this);
    std::string leftType = type;
    expr.right->accept(*this);
    std::string rightType = type;
    if (leftType != "int" || rightType != "int")
    {
        owner.helper.error("二元运算符 '" + expr.op + "' 需要整型操作数", expr.line, expr.column);
        type = "error";
    }
    else
    {
        type = "int";
    }
}

void typeVisitor::visit(UnaryExpr &expr)
{
    expr.operand->accept(*this);
    if (type != "int")
    {
        owner.helper.error("一元运算符 '" + expr.op + "' 需要整型操作数", expr.line, expr.column);
        type = "error";
    }
    else
    {
        type = "int";
    }
}

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
        owner.helper.error("函数 '" + expr.callee + "' 调用参数数量错误", expr.line, expr.column);
        type = it->second.returnType; // 尽管有错误，仍返回函数的返回类型
        return;
    }
    
    for (size_t i = 0; i < expr.arguments.size(); ++i)
    {
        expr.arguments[i]->accept(*this);
        std::string argType = type;
        
        if (!isTypeCompatible(argType, it->second.paramTypes[i]))
        {
            owner.helper.error("函数 '" + expr.callee + "' 第 " + std::to_string(i+1) + 
                           " 个参数类型不匹配", expr.line, expr.column);
        }
    }
    
    type = it->second.returnType;
}

// 以下是必须实现的其他typeVisitor::visit方法（其实没用）
void typeVisitor::visit(ExprStmt &stmt)
{
    if (stmt.expression)
        stmt.expression->accept(*this);
}

void typeVisitor::visit(VarDeclStmt &stmt)
{
    if (stmt.initializer){
        stmt.initializer->accept(*this);

        // 检查初始化表达式类型
        if (type != "int") {
            owner.helper.error("无法使用非整型表达式初始化整型变量", stmt.line, stmt.column);
        }
    }
    
    // 变量声明总是返回 void
    type = "void";
}

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
        owner.helper.error("赋值类型不匹配：变量 '" + stmt.name + "' 的类型为 '" + 
                       symbol->type + "', 表达式类型为 '" + valueType + "'", 
                       stmt.line, stmt.column);
    }
    
    // 赋值语句总是返回 void
    type = "void";
}

void typeVisitor::visit(BlockStmt &stmt)
{
    for (auto &s : stmt.statements)
        s->accept(*this);
    
    // 代码块总是返回 void
    type = "void";
}

void typeVisitor::visit(IfStmt &stmt)
{
    stmt.condition->accept(*this);
    if (type != "int") {
        owner.helper.error("If条件必须是整型(用作布尔型)", stmt.line, stmt.column);
    }
    
    stmt.thenBranch->accept(*this);
    if (stmt.elseBranch)
        stmt.elseBranch->accept(*this);
    
    // if语句总是返回 void
    type = "void";
}

void typeVisitor::visit(WhileStmt &stmt)
{
    stmt.condition->accept(*this);
    if (type != "int") {
        owner.helper.error("While条件必须是整型(用作布尔型)", stmt.line, stmt.column);
    }
    
    stmt.body->accept(*this);
    
    // while语句总是返回 void
    type = "void";
}

void typeVisitor::visit(BreakStmt &) {
    // break语句总是返回 void
    type = "void";
}

void typeVisitor::visit(ContinueStmt &) {
    // continue语句总是返回 void
    type = "void";
}

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

void typeVisitor::visit(FunctionDef &funcDef)
{
    funcDef.body->accept(*this);
    // 函数定义总是返回 void
    type = "void";
}

void typeVisitor::visit(CompUnit &compUnit)
{
    for (auto &func : compUnit.functions)
        func->accept(*this);
    
    // 编译单元总是返回 void
    type = "void";
}
