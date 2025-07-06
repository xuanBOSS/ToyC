#include <string>
#include "analyzeVisitor.h"

// 构造
analyzeVisitor::analyzeVisitor() : typeChecker(*this), helper(*this)
{
    // 初始全局作用域
    helper.enterScope();
}

// 析构
analyzeVisitor::~analyzeVisitor()
{
    // 清理所有作用域
    while (!symbolTables.empty())
    {
        helper.exitScope();
    }
}

// 表达式结点

void analyzeVisitor::visit(NumberExpr &expr)
{
    // 数字字面量总是int类型，无需额外检查
}

void analyzeVisitor::visit(VariableExpr &expr)
{
    // 检查变量在当前作用域是否被声明定义
    Symbol *symbol = helper.findSymbol(expr.name);
    if (!symbol)
    {
        helper.error("Undefined variable: " + expr.name, helper.getLineNumber(expr));
    }
}

void analyzeVisitor::visit(BinaryExpr &expr)
{
    // 检查操作数子节点
    expr.left->accept(*this);
    expr.right->accept(*this);
    // 类型检查
    std::string leftType = typeChecker.getExprType(*expr.left);
    std::string rightType = typeChecker.getExprType(*expr.right);
    if (leftType != "int" || rightType != "int")
    {
        helper.error("Binary operator '" + expr.op + "' requires int operands", helper.getLineNumber(expr));
    }
    // 除以0检查（不包含调用函数的情况）
    if (expr.op == "/" || expr.op == "%")
    {
        if (auto rval = helper.evaluateConstant(expr.right))
        {
            if (*rval == 0)
            {
                helper.error("Division by zero", helper.getLineNumber(expr));
            }
        }
    }
}

void analyzeVisitor::visit(UnaryExpr &expr)
{
    // 检查操作数子节点
    expr.operand->accept(*this);
    // 类型检查
    std::string operandType = typeChecker.getExprType(*expr.operand);
    if (operandType != "int")
    {
        helper.error("Unary operator '" + expr.op + "' requires int operand", helper.getLineNumber(expr));
    }
}

void analyzeVisitor::visit(CallExpr &expr)
{
    // 函数是否被定义
    std::string callee = expr.callee;
    if (functionTable.find(callee) == functionTable.end() && callee != currentFunction)
    {
        helper.error("Undefined function: " + expr.callee, helper.getLineNumber(expr));
    }
    FunctionInfo *funcInfo = &(functionTable.find(callee)->second);
    // 参数数量
    if (funcInfo->paramTypes.size() != expr.arguments.size())
    {
        helper.error("Function '" + expr.callee + "' called with wrong number of arguments",
                     helper.getLineNumber(expr));
    }
    // 实参
    for (size_t i = 0; i < expr.arguments.size(); i++)
    {
        expr.arguments[i]->accept(*this);
    }
    // 实参类型+返回值类型
    if (typeChecker.getExprType(expr) != funcInfo->returnType)
    {
        helper.error("Function '" + expr.callee + "' returned with a type mismatch",
                     helper.getLineNumber(expr));
    }
}

// 语句结点

void analyzeVisitor::visit(ExprStmt &stmt)
{
    if (stmt.expression)
    {
        stmt.expression->accept(*this);
    }
}

void analyzeVisitor::visit(VarDeclStmt &stmt)
{
    // 检查变量是否已声明
    std::string name = stmt.name;
    if (symbolTables.back().find(name) != symbolTables.back().end())
    {
        helper.error("Variable '" + stmt.name + "' is already declared in this scope",
                     helper.getLineNumber(stmt));
    }
    // 检查初始值类型
    if (stmt.initializer)
    {
        stmt.initializer->accept(*this);
        std::string initType = typeChecker.getExprType(*stmt.initializer);
        if (initType != "int")
        {
            helper.error("Cannot initialize int variable with non-int expression",
                         helper.getLineNumber(stmt));
        }
    }
    // 声明变量
    Symbol symbol(Symbol::Kind::VARIABLE, "int",
                  helper.getLineNumber(stmt));
    helper.declareSymbol(stmt.name, symbol);
}

void analyzeVisitor::visit(AssignStmt &stmt)
{
    // 检查变量是否已声明
    Symbol *symbol = helper.findSymbol(stmt.name);
    if (!symbol)
    {
        helper.error("Undefined variable: " + stmt.name, helper.getLineNumber(stmt));
    }
    // 检查变量类型
    if (symbol->kind != Symbol::Kind::VARIABLE && symbol->kind != Symbol::Kind::PARAMETER)
    {
        helper.error("Cannot assign to '" + stmt.name + "' (not a variable)",
                     helper.getLineNumber(stmt));
    }
    // 检查所赋值的类型（避免void函数调用的情况）
    stmt.value->accept(*this);
    std::string valueType = typeChecker.getExprType(*stmt.value);
    if (valueType != "int")
    {
        helper.error("Type mismatch in assignment to '" + stmt.name + "'",
                     helper.getLineNumber(stmt));
    }
}

void analyzeVisitor::visit(BlockStmt &stmt)
{
    // 进入新的作用域
    helper.enterScope();
    // 分析块中的每个语句
    for (auto &s : stmt.statements)
    {
        s->accept(*this);
    }
    // 离开作用域
    helper.exitScope();
}

void analyzeVisitor::visit(IfStmt &stmt)
{
    // 检查条件表达式
    stmt.condition->accept(*this);
    std::string condType = typeChecker.getExprType(*stmt.condition);
    if (condType != "int")
    {
        helper.error("If condition must be of type int(used as bool)", helper.getLineNumber(stmt));
    }
    // 分析then分支
    stmt.thenBranch->accept(*this);
    // 分析else分支(如果有)
    if (stmt.elseBranch)
    {
        stmt.elseBranch->accept(*this);
    }
}

void analyzeVisitor::visit(WhileStmt &stmt)
{
    // 检查条件表达式
    stmt.condition->accept(*this);
    std::string condType = typeChecker.getExprType(*stmt.condition);
    if (condType != "int")
    {
        helper.error("While condition must be of type int(used as bool)", helper.getLineNumber(stmt));
    }
    // 进入循环
    helper.enterLoop();
    // 分析循环体
    stmt.body->accept(*this);
    // 离开循环
    helper.exitLoop();
}

void analyzeVisitor::visit(BreakStmt &stmt)
{
    // 检查break语句是否在循环内
    if (!helper.isInLoop())
    {
        helper.error("Break statement outside of loop", helper.getLineNumber(stmt));
    }
}

void analyzeVisitor::visit(ContinueStmt &stmt)
{
    // 检查continue语句是否在循环内
    if (!helper.isInLoop())
    {
        helper.error("Continue statement outside of loop", helper.getLineNumber(stmt));
    }
}

void analyzeVisitor::visit(ReturnStmt &stmt)
{
    // return后有非空表达式
    if (stmt.value)
    {
        stmt.value->accept(*this);
        std::string returnType = typeChecker.getExprType(*stmt.value);
        if (returnType != currentFunctionReturnType)
        {
            helper.error("Return type mismatch: expected '" + currentFunctionReturnType +
                             "', got '" + returnType + "'",
                         helper.getLineNumber(stmt));
        }
    }
    // return ；
    else if (currentFunctionReturnType != "void")
    {
        helper.error("Function with return type '" + currentFunctionReturnType +
                         "' must return a value",
                     helper.getLineNumber(stmt));
    }

    hasReturn = true;
}

// 特殊ast结点

void analyzeVisitor::visit(FunctionDef &funcDef)
{
    // 构建当前函数的函数信息结构体，以放进函数表
    FunctionInfo info;
    info.line = helper.getLineNumber(funcDef.body);

    // 函数名称在全局的唯一性
    if (functionTable.find(funcDef.name) != functionTable.end())
    {
        helper.error("Repeated function name", info.line);
    }

    // 续函数信息
    info.returnType = funcDef.returnType;
    for (const auto &param : funcDef.params)
    {
        info.paramTypes.push_back("int"); // ToyC中所有参数都是int
        info.paramNames.push_back(param.name);
    }

    // 如果是main函数，检查其合法性
    if (funcDef.name == "main" && !helper.isValidMainFunction(funcDef))
    {
        helper.error("Invalid main function declaration", info.line);
    }

    // 设置当前函数的（全局）上下文
    currentFunction = funcDef.name;
    currentFunctionReturnType = funcDef.returnType;
    hasReturn = false;

    // 进入新作用域(即创建新的符号表压入栈)
    helper.enterScope();

    // 添加函数名到当前作用域的符号表
    Symbol symbol(Symbol::Kind::FUNCTION, funcDef.returnType, info.line);
    helper.declareSymbol(funcDef.name, symbol);

    // 添加参数到当前作用域的符号表
    for (size_t i = 0; i < funcDef.params.size(); i++)
    {
        const auto &param = funcDef.params[i];
        Symbol symbol(Symbol::Kind::PARAMETER, "int", info.line, 0, i);
        if (!helper.declareSymbol(param.name, symbol))
        {
            helper.error("Parameter '" + param.name + "' is already declared", info.line);
        }
    }

    // 分析函数体
    // 覆盖了函数体内有return关键字的情况
    funcDef.body->accept(*this);

    // 函数体内无return关键字的情况
    if (funcDef.returnType != "void" && !hasReturn)
    {
        helper.error("Function '" + funcDef.name + "' has no return statement", info.line);
    }

    // 当前函数加入函数表
    functionTable[currentFunction] = info;

    // 离开作用域
    helper.exitScope();

    // 清除当前函数上下文
    currentFunction = "";
    currentFunctionReturnType = "";
    hasReturn = false;
}

void analyzeVisitor::visit(CompUnit &compUnit)
{
    // 检查是否有main函数
    bool hasMain = false;
    for (const auto &func : compUnit.functions)
    {
        if (func->name == "main")
        {
            hasMain = true;
        }
    }
    if (!hasMain)
    {
        helper.error("Program must have a main function");
    }
    // 分析每个函数定义(main函数的检查包含在内)
    for (const auto &func : compUnit.functions)
    {
        func->accept(*this);
    }
}