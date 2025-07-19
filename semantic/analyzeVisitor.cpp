// analyzeVisitor.cpp - 实现语义分析访问者
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

// 未使用变量检查
void analyzeVisitor::checkUnusedVariables()
{
    // 遍历所有符号表检查未使用的变量
    for (size_t scopeIndex = 0; scopeIndex < symbolTables.size(); ++scopeIndex) {
        const auto& scope = symbolTables[scopeIndex];
        for (const auto& [name, symbol] : scope) {
            if ((symbol.kind == Symbol::Kind::VARIABLE || symbol.kind == Symbol::Kind::PARAMETER) && 
                !symbol.used) {
                helper.warning("Variable '" + name + "' declared but never used", symbol.line, symbol.column);
            }
        }
    }
}

// 死代码检测
void analyzeVisitor::detectDeadCode()
{
    // 遍历函数表检查未使用的函数
    for (const auto& [name, info] : functionTable) {
        if (name != "main") {
            bool used = false;
            for (const auto& scope : symbolTables) {
                auto it = scope.find(name);
                if (it != scope.end() && it->second.kind == Symbol::Kind::FUNCTION && it->second.used) {
                    used = true;
                    break;
                }
            }
            if (!used) {
                helper.warning("Function '" + name + "' defined but never used", info.line);
            }
        }
    }
}

// 表达式结点
void analyzeVisitor::visit(NumberExpr &expr)
{
    // 数字字面量总是int类型，无需额外检查
}
// 访问变量引用
void analyzeVisitor::visit(VariableExpr &expr)
{
    // 检查变量在当前作用域是否被声明定义
    Symbol *symbol = helper.findSymbol(expr.name);
    if (!symbol)
    {
        helper.error("Undefined variable: " + expr.name, expr.line, expr.column);
        return;
    }

    // 标记变量为已使用
    symbol->used = true;
}
// 访问二元表达式
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
        helper.error("Binary operator '" + expr.op + "' requires int operands", expr.line, expr.column);
    }
    // 除以0检查（不包含调用函数的情况）
    if (expr.op == "/" || expr.op == "%")
    {
        if (auto rval = helper.evaluateConstant(expr.right))
        {
            if (*rval == 0)
            {
                helper.error("Division by zero", expr.line, expr.column);
            }
        }
    }
    // 检查恒为真或恒为假的条件表达式
    if (expr.op == "==" || expr.op == "!=" || expr.op == "<" || 
        expr.op == ">" || expr.op == "<=" || expr.op == ">=" ||
        expr.op == "&&" || expr.op == "||") {
        auto leftVal = helper.evaluateConstant(expr.left);
        auto rightVal = helper.evaluateConstant(expr.right);
        
        if (leftVal && rightVal) {
            // 如果两个操作数都是常量，那么这个表达式也是常量
            bool isAlwaysTrue = false;
            bool isAlwaysFalse = false;
            
            if (expr.op == "==") isAlwaysTrue = (*leftVal == *rightVal);
            else if (expr.op == "!=") isAlwaysTrue = (*leftVal != *rightVal);
            else if (expr.op == "<") isAlwaysTrue = (*leftVal < *rightVal);
            else if (expr.op == ">") isAlwaysTrue = (*leftVal > *rightVal);
            else if (expr.op == "<=") isAlwaysTrue = (*leftVal <= *rightVal);
            else if (expr.op == ">=") isAlwaysTrue = (*leftVal >= *rightVal);
            else if (expr.op == "&&") isAlwaysTrue = (*leftVal && *rightVal);
            else if (expr.op == "||") isAlwaysTrue = (*leftVal || *rightVal);
            
            isAlwaysFalse = !isAlwaysTrue;
            
            if (isAlwaysTrue) {
                helper.warning("Condition expression is always true", expr.line, expr.column);
            } else if (isAlwaysFalse) {
                helper.warning("Condition expression is always false", expr.line, expr.column);
            }
        }
    }
}
// 访问一元表达式
void analyzeVisitor::visit(UnaryExpr &expr)
{
    // 检查操作数子节点
    expr.operand->accept(*this);
    // 类型检查
    std::string operandType = typeChecker.getExprType(*expr.operand);
    if (operandType != "int")
    {
        helper.error("Unary operator '" + expr.op + "' requires int operand", expr.line, expr.column);
    }
}
// 访问函数调用表达式
void analyzeVisitor::visit(CallExpr &expr)
{
    // 函数是否被定义
    std::string callee = expr.callee;
    if (functionTable.find(callee) == functionTable.end() && callee != currentFunction)
    {
        helper.error("Undefined function: " + expr.callee, expr.line, expr.column);
        return;
    }

    // 标记函数为已使用
    Symbol *symbol = helper.findSymbol(callee);
    if (symbol && symbol->kind == Symbol::Kind::FUNCTION) {
        symbol->used = true;
    }

    FunctionInfo *funcInfo = &(functionTable.find(callee)->second);
    // 参数数量
    if (funcInfo->paramTypes.size() != expr.arguments.size())
    {
        helper.error("Incorrect number of arguments for function '" + expr.callee + "'", expr.line, expr.column);
    }
    // 实参
    for (size_t i = 0; i < expr.arguments.size(); i++)
    {
        expr.arguments[i]->accept(*this);

         // 增强类型检查 - 检查实参类型
        if (i < funcInfo->paramTypes.size()) {
            std::string argType = typeChecker.getExprType(*expr.arguments[i]);
            if (argType != funcInfo->paramTypes[i]) {
                helper.error("Function '" + expr.callee + "' argument " + std::to_string(i+1) + 
                          " type mismatch, expected '" + funcInfo->paramTypes[i] + 
                          "', got '" + argType + "'", 
                          expr.line, expr.column);
            }
        }
    }
    // 实参类型+返回值类型
    if (typeChecker.getExprType(expr) != funcInfo->returnType)
    {
         helper.error("Function '" + expr.callee + "' return type mismatch", expr.line, expr.column);
    }
}

// 访问表达式语句
void analyzeVisitor::visit(ExprStmt &stmt)
{
    if (stmt.expression)
    {
        stmt.expression->accept(*this);
    }
}
// 访问变量声明语句
void analyzeVisitor::visit(VarDeclStmt &stmt)
{
    // 检查变量是否已声明
    std::string name = stmt.name;
    if (symbolTables.back().find(name) != symbolTables.back().end())
    {
        helper.error("Variable '" + stmt.name + "' already declared in current scope", stmt.line, stmt.column);
    }
    // 检查初始值类型
    if (stmt.initializer)
    {
        stmt.initializer->accept(*this);
        std::string initType = typeChecker.getExprType(*stmt.initializer);
        if (initType != "int")
        {
            helper.error("Cannot initialize int variable with non-integer expression", stmt.line, stmt.column);
        }
    }
    // 声明变量
    Symbol symbol(Symbol::Kind::VARIABLE, "int", stmt.line, stmt.column);
    symbol.used = false; // 初始设置为未使用
    helper.declareSymbol(stmt.name, symbol);
}
// 访问赋值语句
void analyzeVisitor::visit(AssignStmt &stmt)
{
    // 检查变量是否已声明
    Symbol *symbol = helper.findSymbol(stmt.name);
    if (!symbol)
    {
        helper.error("Undefined variable: " + stmt.name, stmt.line, stmt.column);
        return;
    }
    
    // 标记变量为已使用
    symbol->used = true;
    
    // 检查变量类型
    if (symbol->kind != Symbol::Kind::VARIABLE && symbol->kind != Symbol::Kind::PARAMETER)
    {
        helper.error("Cannot assign to '" + stmt.name + "' (not a variable)", stmt.line, stmt.column);
    }
    // 检查所赋值的类型（避免void函数调用的情况）
    stmt.value->accept(*this);
    std::string valueType = typeChecker.getExprType(*stmt.value);
    if (valueType != "int")
    {
        helper.error("Type mismatch in assignment to '" + stmt.name + "'", stmt.line, stmt.column);
    }
}
// 访问语句块
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
// 访问if语句
void analyzeVisitor::visit(IfStmt &stmt)
{
    // 检查条件表达式
    stmt.condition->accept(*this);
    std::string condType = typeChecker.getExprType(*stmt.condition);
    if (condType != "int")
    {
        helper.error("If condition must be integer (used as boolean)", stmt.line, stmt.column);
    }
    
    // 检查恒为真或恒为假的条件
    auto condValue = helper.evaluateConstant(stmt.condition);
    if (condValue) {
        if (*condValue != 0) {
            // 条件恒为真，else分支是死代码
            if (stmt.elseBranch) {
                helper.warning("This else branch will never execute (condition always true)", 
                             stmt.elseBranch->line, stmt.elseBranch->column);
            }
        } else {
            // 条件恒为假，then分支是死代码
            helper.warning("This if branch will never execute (condition always false)", 
                         stmt.thenBranch->line, stmt.thenBranch->column);
        }
    }
    
    // 分析then分支
    stmt.thenBranch->accept(*this);
    // 分析else分支(如果有)
    if (stmt.elseBranch)
    {
        stmt.elseBranch->accept(*this);
    }
}
// 访问while语句
void analyzeVisitor::visit(WhileStmt &stmt)
{
    // 检查条件表达式
    stmt.condition->accept(*this);
    std::string condType = typeChecker.getExprType(*stmt.condition);
    if (condType != "int")
    {
        helper.error("While condition must be integer (used as boolean)", stmt.line, stmt.column);
    }
    
    // 检查恒为假的条件
    auto condValue = helper.evaluateConstant(stmt.condition);
    if (condValue && *condValue == 0) {
        helper.warning("This while loop will never execute (condition always false)", stmt.line, stmt.column);
    }
    
    // 进入循环
    helper.enterLoop();
    // 分析循环体
    stmt.body->accept(*this);
    // 离开循环
    helper.exitLoop();
}
// 访问break语句
void analyzeVisitor::visit(BreakStmt &stmt)
{
    // 检查break语句是否在循环内
    if (!helper.isInLoop())
    {
        helper.error("Break statement must be inside loop", stmt.line, stmt.column);
    }
}
// 访问continue语句
void analyzeVisitor::visit(ContinueStmt &stmt)
{
    // 检查continue语句是否在循环内
    if (!helper.isInLoop())
    {
        helper.error("Continue statement must be inside loop", stmt.line, stmt.column);
    }
}
// 访问return语句
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
                          "', got '" + returnType + "'", stmt.line, stmt.column);
        }
    }
    // 检查不带返回值的return语句
    else if (currentFunctionReturnType != "void")
    {
        helper.error("Function with return type '" + currentFunctionReturnType +
                      "' must return a value", stmt.line, stmt.column);
    }

    hasReturn = true;
}

// 访问函数定义
void analyzeVisitor::visit(FunctionDef &funcDef)
{
    int line = funcDef.line;
    int column = funcDef.column;
    std::string name = funcDef.name;

    // 函数名不能重复
    if (functionTable.count(name)) {
        helper.error("Duplicate function name", line, column);
    }

    // 构建函数信息（完整）
    FunctionInfo info;
    info.returnType = funcDef.returnType;
    info.line = line;
    info.column = column;
    for (const auto &param : funcDef.params) {
        info.paramTypes.push_back("int");
        info.paramNames.push_back(param.name);
    }

    // 提前注册函数信息（支持递归）
    functionTable[name] = info;

    // 检查 main 函数合法性
    if (name == "main" && !helper.isValidMainFunction(funcDef)) {
        helper.error("Invalid main function declaration", line, column);
    }

    // 设置当前上下文
    currentFunction = name;
    currentFunctionReturnType = funcDef.returnType;
    hasReturn = false;

    // 进入新作用域
    helper.enterScope();

    // 注册函数符号
    Symbol funcSymbol(Symbol::Kind::FUNCTION, funcDef.returnType, line, column);
    funcSymbol.used = (name == "main");
    helper.declareSymbol(name, funcSymbol);

    // 注册参数符号
    for (size_t i = 0; i < funcDef.params.size(); i++) {
        const auto &param = funcDef.params[i];
        Symbol paramSymbol(Symbol::Kind::PARAMETER, "int", line, column, i);
        paramSymbol.used = false;
        if (!helper.declareSymbol(param.name, paramSymbol)) {
            helper.error("Parameter '" + param.name + "' already declared", line, column);
        }
    }

    // 访问函数体
    funcDef.body->accept(*this);

    // 检查 return 语句是否遗漏
    if (funcDef.returnType != "void" && !hasReturn) {
        helper.error("Function '" + name + "' has no return statement", line, column);
    }

    // 检查未使用的局部变量和参数
    checkUnusedVariables();

    // 离开作用域
    helper.exitScope();

    // 清除上下文
    currentFunction = "";
    currentFunctionReturnType = "";
    hasReturn = false;
}
// 访问编译单元
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
    
    // 检查未使用的函数
    detectDeadCode();
}
