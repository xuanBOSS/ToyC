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
                helper.warning("变量 '" + name + "' 声明但未使用", symbol.line, symbol.column);
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
                helper.warning("函数 '" + name + "' 定义但未使用", info.line);
            }
        }
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
        helper.error("未定义的变量: " + expr.name, expr.line, expr.column);
        return;
    }

    // 标记变量为已使用
    symbol->used = true;
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
                helper.warning("条件表达式恒为真", expr.line, expr.column);
            } else if (isAlwaysFalse) {
                helper.warning("条件表达式恒为假", expr.line, expr.column);
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
        helper.error("Unary operator '" + expr.op + "' requires int operand", expr.line, expr.column);
    }
}

void analyzeVisitor::visit(CallExpr &expr)
{
    // 函数是否被定义
    std::string callee = expr.callee;
    if (functionTable.find(callee) == functionTable.end() && callee != currentFunction)
    {
        helper.error("未定义的函数: " + expr.callee, expr.line, expr.column);
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
        helper.error("函数 '" + expr.callee + "' 调用参数数量错误", expr.line, expr.column);
    }
    // 实参
    for (size_t i = 0; i < expr.arguments.size(); i++)
    {
        expr.arguments[i]->accept(*this);

         // 增强类型检查 - 检查实参类型
        if (i < funcInfo->paramTypes.size()) {
            std::string argType = typeChecker.getExprType(*expr.arguments[i]);
            if (argType != funcInfo->paramTypes[i]) {
                helper.error("函数 '" + expr.callee + "' 第 " + std::to_string(i+1) + 
                          " 个参数类型不匹配，期望 '" + funcInfo->paramTypes[i] + 
                          "', 得到 '" + argType + "'", 
                          expr.line, expr.column);
            }
        }
    }
    // 实参类型+返回值类型
    if (typeChecker.getExprType(expr) != funcInfo->returnType)
    {
         helper.error("函数 '" + expr.callee + "' 返回类型不匹配", expr.line, expr.column);
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
        helper.error("变量 '" + stmt.name + "' 在当前作用域中已经声明", stmt.line, stmt.column);
    }
    // 检查初始值类型
    if (stmt.initializer)
    {
        stmt.initializer->accept(*this);
        std::string initType = typeChecker.getExprType(*stmt.initializer);
        if (initType != "int")
        {
            helper.error("不能使用非整型表达式初始化整型变量", stmt.line, stmt.column);
        }
    }
    // 声明变量
    Symbol symbol(Symbol::Kind::VARIABLE, "int", stmt.line, stmt.column);
    symbol.used = false; // 初始设置为未使用
    helper.declareSymbol(stmt.name, symbol);
}

void analyzeVisitor::visit(AssignStmt &stmt)
{
    // 检查变量是否已声明
    Symbol *symbol = helper.findSymbol(stmt.name);
    if (!symbol)
    {
        helper.error("未定义的变量: " + stmt.name, stmt.line, stmt.column);
        return;
    }
    
    // 标记变量为已使用
    symbol->used = true;
    
    // 检查变量类型
    if (symbol->kind != Symbol::Kind::VARIABLE && symbol->kind != Symbol::Kind::PARAMETER)
    {
        helper.error("不能给 '" + stmt.name + "' 赋值 (不是变量)", stmt.line, stmt.column);
    }
    // 检查所赋值的类型（避免void函数调用的情况）
    stmt.value->accept(*this);
    std::string valueType = typeChecker.getExprType(*stmt.value);
    if (valueType != "int")
    {
        helper.error("赋值给 '" + stmt.name + "' 的类型不匹配", stmt.line, stmt.column);
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
        helper.error("If条件必须是整型(用作布尔型)", stmt.line, stmt.column);
    }
    
    // 检查恒为真或恒为假的条件
    auto condValue = helper.evaluateConstant(stmt.condition);
    if (condValue) {
        if (*condValue != 0) {
            // 条件恒为真，else分支是死代码
            if (stmt.elseBranch) {
                helper.warning("此else分支永远不会执行（条件恒为真）", 
                             stmt.elseBranch->line, stmt.elseBranch->column);
            }
        } else {
            // 条件恒为假，then分支是死代码
            helper.warning("此if分支永远不会执行（条件恒为假）", 
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

void analyzeVisitor::visit(WhileStmt &stmt)
{
    // 检查条件表达式
    stmt.condition->accept(*this);
    std::string condType = typeChecker.getExprType(*stmt.condition);
    if (condType != "int")
    {
        helper.error("While条件必须是整型(用作布尔型)", stmt.line, stmt.column);
    }
    
    // 检查恒为假的条件
    auto condValue = helper.evaluateConstant(stmt.condition);
    if (condValue && *condValue == 0) {
        helper.warning("此while循环永远不会执行（条件恒为假）", stmt.line, stmt.column);
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
        helper.error("Break语句必须在循环内", stmt.line, stmt.column);
    }
}

void analyzeVisitor::visit(ContinueStmt &stmt)
{
    // 检查continue语句是否在循环内
    if (!helper.isInLoop())
    {
        helper.error("Continue语句必须在循环内", stmt.line, stmt.column);
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
            helper.error("返回类型不匹配: 期望 '" + currentFunctionReturnType +
                          "', 得到 '" + returnType + "'", stmt.line, stmt.column);
        }
    }
    // return ；
    else if (currentFunctionReturnType != "void")
    {
        helper.error("返回类型为 '" + currentFunctionReturnType +
                      "' 的函数必须返回一个值", stmt.line, stmt.column);
    }

    hasReturn = true;
}


// 特殊ast结点
void analyzeVisitor::visit(FunctionDef &funcDef)
{
    // 构建当前函数的函数信息结构体，以放进函数表
    FunctionInfo info;
    info.line = funcDef.line;
    info.column = funcDef.column;

    // 函数名称在全局的唯一性
    if (functionTable.find(funcDef.name) != functionTable.end())
    {
        helper.error("重复的函数名", info.line, info.column);
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
        helper.error("非法的main函数声明", info.line, info.column);
    }

    // 设置当前函数的（全局）上下文
    currentFunction = funcDef.name;
    currentFunctionReturnType = funcDef.returnType;
    hasReturn = false;

    // 进入新作用域(即创建新的符号表压入栈)
    helper.enterScope();

    // 添加函数名到当前作用域的符号表
    Symbol symbol(Symbol::Kind::FUNCTION, funcDef.returnType, info.line, info.column);
    symbol.used = (funcDef.name == "main"); // main函数默认为已使用
    helper.declareSymbol(funcDef.name, symbol);

    // 添加参数到当前作用域的符号表
    for (size_t i = 0; i < funcDef.params.size(); i++)
    {
        const auto &param = funcDef.params[i];
        Symbol symbol(Symbol::Kind::PARAMETER, "int", info.line, info.column, i);
        symbol.used = false; // 初始设置为未使用
        if (!helper.declareSymbol(param.name, symbol))
        {
            helper.error("参数 '" + param.name + "' 已经声明", info.line, info.column);
        }
    }

    // 分析函数体
    // 覆盖了函数体内有return关键字的情况
    funcDef.body->accept(*this);

    // 函数体内无return关键字的情况
    if (funcDef.returnType != "void" && !hasReturn)
    {
        helper.error("函数 '" + funcDef.name + "' 没有返回语句", info.line, info.column);
    }

    // 检查未使用的参数和局部变量
    checkUnusedVariables();

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
        helper.error("程序必须有一个main函数");
    }
    // 分析每个函数定义(main函数的检查包含在内)
    for (const auto &func : compUnit.functions)
    {
        func->accept(*this);
    }
    
    // 检查未使用的函数
    detectDeadCode();
}
