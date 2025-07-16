// analyzeHelper.cpp - 实现语义分析辅助工具类
#include "analyzeHelper.h"
#include "analyzeVisitor.h"
#include "semantic.h"
#include <iostream>

// 初始化静态成员
SemanticAnalyzer* analyzeHelper::semanticOwner = nullptr;

// 设置语义分析器
void analyzeHelper::setSemanticOwner(SemanticAnalyzer& analyzer) {
    semanticOwner = &analyzer;
}

// 进入新作用域 - 创建新的符号表
void analyzeHelper::enterScope()
{
    owner.getSymbolTables().push_back(std::unordered_map<std::string, Symbol>());
}

// 退出当前作用域 - 检查未使用变量并移除当前符号表
void analyzeHelper::exitScope()
{
    if (!owner.getSymbolTables().empty())
    {
        checkUnusedVariables();  // 退出作用域时检查未使用变量
        owner.getSymbolTables().pop_back();
    }
}

// 在当前作用域声明符号
bool analyzeHelper::declareSymbol(const std::string &name, Symbol symbol)
{
    // 检查符号是否已在当前作用域中定义
    if (owner.getSymbolTables().back().find(name) != owner.getSymbolTables().back().end())
    {
        return false; // 已存在，声明失败
    }
    // 添加符号到当前作用域
    owner.getSymbolTables().back()[name] = symbol;
    return true;
}

// 在所有可见作用域中查找符号
Symbol *analyzeHelper::findSymbol(const std::string &name)
{
    // 从当前作用域向上查找符号（反向遍历）
    for (auto tableIt = owner.getSymbolTables().rbegin(); tableIt != owner.getSymbolTables().rend(); ++tableIt)
    {
        auto symIt = tableIt->find(name);
        if (symIt != tableIt->end())
        {
            // 标记变量被使用
            if (symIt->second.kind == Symbol::Kind::VARIABLE) {
                symIt->second.used = true;
            }
            return &symIt->second;
        }
    }
    return nullptr;
}

// 尝试在编译时计算表达式的值
OptionalInt analyzeHelper::evaluateConstant(const std::shared_ptr<Expr>& expr)
{
    // 数字字面量
    if (auto numExpr = std::dynamic_pointer_cast<NumberExpr>(expr)) {
        return OptionalInt(numExpr->value);
    }
    
    // 一元表达式
    if (auto unaryExpr = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        OptionalInt operandValue = evaluateConstant(unaryExpr->operand);
        if (!operandValue.has_value()) return OptionalInt();
        
        if (unaryExpr->op == "+") return OptionalInt(*operandValue);
        if (unaryExpr->op == "-") return OptionalInt(-(*operandValue));
        if (unaryExpr->op == "!") return OptionalInt(!(*operandValue));
        
        return OptionalInt();
    }
    
    // 二元表达式
    if (auto binaryExpr = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        OptionalInt leftValue = evaluateConstant(binaryExpr->left);
        OptionalInt rightValue = evaluateConstant(binaryExpr->right);
        
        if (!leftValue.has_value() || !rightValue.has_value()) return OptionalInt();
        
        if (binaryExpr->op == "+") return OptionalInt(*leftValue + *rightValue);
        if (binaryExpr->op == "-") return OptionalInt(*leftValue - *rightValue);
        if (binaryExpr->op == "*") return OptionalInt(*leftValue * *rightValue);
        if (binaryExpr->op == "/") {
            if (*rightValue == 0) return OptionalInt(); // 避免除以零
            return OptionalInt(*leftValue / *rightValue);
        }
        if (binaryExpr->op == "%") {
            if (*rightValue == 0) return OptionalInt(); // 避免除以零
            return OptionalInt(*leftValue % *rightValue);
        }
        if (binaryExpr->op == "<") return OptionalInt(*leftValue < *rightValue ? 1 : 0);
        if (binaryExpr->op == ">") return OptionalInt(*leftValue > *rightValue ? 1 : 0);
        if (binaryExpr->op == "<=") return OptionalInt(*leftValue <= *rightValue ? 1 : 0);
        if (binaryExpr->op == ">=") return OptionalInt(*leftValue >= *rightValue ? 1 : 0);
        if (binaryExpr->op == "==") return OptionalInt(*leftValue == *rightValue ? 1 : 0);
        if (binaryExpr->op == "!=") return OptionalInt(*leftValue != *rightValue ? 1 : 0);
        if (binaryExpr->op == "&&") return OptionalInt((*leftValue && *rightValue) ? 1 : 0);
        if (binaryExpr->op == "||") return OptionalInt((*leftValue || *rightValue) ? 1 : 0);
    }
    
    // 不是常量表达式
    return OptionalInt();
}

// 报告错误
void analyzeHelper::error(const std::string &message, int line, int column)
{
    // 设置错误标志
    owner.success = false;
    // 构建完整错误消息
    std::string fullMessage = message;
    if (line > 0)
    {
        fullMessage += " at line " + std::to_string(line);
        if (column > 0)
        {
            fullMessage += ", column " + std::to_string(column);
        }
    }

     // 检查是否已报告过相同错误
    if (reportedErrors.find(fullMessage) == reportedErrors.end()) {
     // 将错误添加到集合中
     reportedErrors.insert(fullMessage);
     owner.errorMessages.push_back(fullMessage);

     // 同时添加到语义分析器的错误集合中（如果设置了）
     if (semanticOwner) {
         semanticOwner->success = false;
         semanticOwner->errorMessages.push_back(fullMessage);
     }
 }
}

// 报告警告
void analyzeHelper::warning(const std::string &message, int line, int column)
{
    // 构建完整警告消息
    std::string fullMessage = message;
    if (line > 0)
    {
        fullMessage += " at line " + std::to_string(line);
        if (column > 0)
        {
            fullMessage += ", column " + std::to_string(column);
        }
    }
    // 检查是否已报告过相同警告
    if (reportedWarnings.find(fullMessage) == reportedWarnings.end()) {
    // 将警告添加到集合中
    reportedWarnings.insert(fullMessage);
    owner.warningMessages.push_back(fullMessage);

    // 同时添加到语义分析器的警告集合中（如果设置了）
    if (semanticOwner) {
        semanticOwner->warningMessages.push_back(fullMessage);
    }
}
}
// 进入循环
void analyzeHelper::enterLoop()
{
    loopDepth++;
}
// 退出循环
void analyzeHelper::exitLoop()
{
    loopDepth--;
}
// 检查是否在循环内
bool analyzeHelper::isInLoop() const
{
    return loopDepth > 0;
}
// 检查main函数是否有效
bool analyzeHelper::isValidMainFunction(FunctionDef &funcDef)
{
    // 检查main函数是否合法
    if (funcDef.returnType != "int")
    {
        error("main function must return int", funcDef.line, funcDef.column);
        return false;
    }
    // main函数不能有参数
    if (!funcDef.params.empty())
    {
        error("main function cannot have parameters", funcDef.line, funcDef.column);
        return false;
    }
    return true;
}

// 未使用变量检查实现
void analyzeHelper::checkUnusedVariables()
{
    // 检查当前作用域中的所有变量
    if (!owner.getSymbolTables().empty()) {
        auto& currentScope = owner.getSymbolTables().back();
        for (const auto& [name, symbol] : currentScope) {
            // 只检查变量，不检查函数
            if (symbol.kind == Symbol::Kind::VARIABLE && !symbol.used) {
                warning("Variable '" + name + "' declared but never used", symbol.line, symbol.column);
            }
        }
    }
}

// 死代码检测实现
void analyzeHelper::detectDeadCode(const std::shared_ptr<Stmt>& stmt)
{
    // 检查if语句中恒为真或恒为假的条件
    if (auto ifStmt = dynamic_cast<IfStmt*>(stmt.get())) {
        if (auto constValue = evaluateConstant(ifStmt->condition)) {
            if (*constValue) {
                // 条件恒为真，else分支永远不会执行
                if (ifStmt->elseBranch) {
                    warning("This else branch will never execute (condition always true)", 
                            getLineNumber(ifStmt->elseBranch), 
                            ifStmt->elseBranch->column);
                }
            } else {
                // 条件恒为假，then分支永远不会执行
                warning("This if branch will never execute (condition always false)", 
                        getLineNumber(ifStmt->thenBranch), 
                        ifStmt->thenBranch->column);
            }
        }
    }
    
    // 检查while语句中恒为假的条件
    if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt.get())) {
        if (auto constValue = evaluateConstant(whileStmt->condition)) {
            if (!(*constValue)) {
                warning("This while loop will never execute (condition always false)", 
                        whileStmt->line, whileStmt->column);
            }
        }
    }
}

// 增强函数调用分析
bool analyzeHelper::validateFunctionCall(const std::string& name, const std::vector<std::shared_ptr<Expr>>& args, int line, int column)
{
    // 查找函数符号
    Symbol* symbol = findSymbol(name);
    if (!symbol) {
        error("Call to undeclared function '" + name + "'", line, column);
        return false;
    }
    
    if (symbol->kind != Symbol::Kind::FUNCTION) {
        error("'" + name + "' is not a function", line, column);
        return false;
    }
    
    // 检查参数数量是否匹配
    if (symbol->params.size() != args.size()) {
        error("Function '" + name + "' expects " + std::to_string(symbol->params.size()) + 
              " arguments but got " + std::to_string(args.size()) + " 个", line, column);
        return false;
    }
    
    // 标记函数为已使用
    symbol->used = true;
    
    return true;
}

// 增强类型检查
bool analyzeHelper::checkTypeCompatibility(const std::shared_ptr<Expr>& expr, const std::string& expectedType, int line, int column)
{
    // 在这个简单的语言中，所有表达式都是int类型，所以只需检查expectedType是否为int
    if (expectedType != "int") {
        error("Type mismatch: expected '" + expectedType + "' type", line, column);
        return false;
    }
    
    return true;
}
