#include "typeVisitor.h"
#include "analyzeVisitor.h"

void typeVisitor::visit(NumberExpr &) { type = "int"; }

void typeVisitor::visit(VariableExpr &expr)
{
    Symbol *symbol = owner.helper.findSymbol(expr.name);
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
        owner.helper.error("Binary operator requires int operands");
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
        owner.helper.error("Unary operator requires int operand");
    }
    type = "int";
}

void typeVisitor::visit(CallExpr &expr)
{
    auto it = owner.getFunctionTable().find(expr.callee);
    for (size_t i = 0; i < expr.arguments.size(); ++i)
    {
        expr.arguments[i]->accept(*this);
        if (type != it->second.paramTypes[i])
        {
            owner.helper.error("Argument type mismatch in call to " + expr.callee);
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
    if (stmt.initializer)
        stmt.initializer->accept(*this);
}
void typeVisitor::visit(AssignStmt &stmt)
{
    stmt.value->accept(*this);
}
void typeVisitor::visit(BlockStmt &stmt)
{
    for (auto &s : stmt.statements)
        s->accept(*this);
}
void typeVisitor::visit(IfStmt &stmt)
{
    stmt.condition->accept(*this);
    stmt.thenBranch->accept(*this);
    if (stmt.elseBranch)
        stmt.elseBranch->accept(*this);
}
void typeVisitor::visit(WhileStmt &stmt)
{
    stmt.condition->accept(*this);
    stmt.body->accept(*this);
}
void typeVisitor::visit(BreakStmt &) {}
void typeVisitor::visit(ContinueStmt &) {}
void typeVisitor::visit(ReturnStmt &stmt)
{
    if (stmt.value)
        stmt.value->accept(*this);
}
void typeVisitor::visit(FunctionDef &funcDef)
{
    funcDef.body->accept(*this);
}
void typeVisitor::visit(CompUnit &compUnit)
{
    for (auto &func : compUnit.functions)
        func->accept(*this);
}