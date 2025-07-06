#include "analyzeHelper.h"
#include "analyzeVisitor.h"
#include <iostream>

void analyzeHelper::enterScope()
{
    owner.getSymbolTables().push_back(std::unordered_map<std::string, Symbol>());
}

void analyzeHelper::exitScope()
{
    if (!owner.getSymbolTables().empty())
    {
        // checkUnusedVariables();
        owner.getSymbolTables().pop_back();
    }
}

bool analyzeHelper::declareSymbol(const std::string &name, Symbol symbol)
{
    if (owner.getSymbolTables().back().find(name) != owner.getSymbolTables().back().end())
    {
        return false;
    }
    owner.getSymbolTables().back()[name] = symbol;
    return true;
}

Symbol *analyzeHelper::findSymbol(const std::string &name)
{
    // 从当前作用域向上查找符号（反向遍历）
    for (auto tableIt = owner.getSymbolTables().rbegin(); tableIt != owner.getSymbolTables().rend(); ++tableIt)
    {
        auto symIt = tableIt->find(name);
        if (symIt != tableIt->end())
        {
            return &symIt->second;
        }
    }
    return nullptr;
}

std::optional<int> analyzeHelper::evaluateConstant(const std::shared_ptr<Expr>& expr)
{
    if (!expr) return std::nullopt;
    if (auto num = dynamic_cast<NumberExpr*>(expr.get())) {
            return num->value;
        }
    if (auto bin = dynamic_cast<BinaryExpr*>(expr.get())) {
        auto left = evaluateConstant(bin->left);
        auto right = evaluateConstant(bin->right);
        if (!left || !right) return std::nullopt;
        
        if (bin->op == "+") return *left + *right;
        if (bin->op == "-") return *left - *right;
        if (bin->op == "*") return *left * *right;
        if (bin->op == "/" && *right != 0) return *left / *right;
        if (bin->op == "%" && *right != 0) return *left % *right;
    }
    if (auto unary = dynamic_cast<UnaryExpr*>(expr.get())) {
        if (auto val = evaluateConstant(unary->operand)) {
            if (unary->op == "-") return -(*val);
            if (unary->op == "+") return *val;
        }
    }
    return std::nullopt;
}

void analyzeHelper::error(const std::string &message, int line, int column)
{
    owner.success = false;
    std::string fullMessage = message;
    if (line > 0)
    {
        fullMessage += " at line " + std::to_string(line);
        if (column > 0)
        {
            fullMessage += ", column " + std::to_string(column);
        }
    }
    // errorMessages.push_back(fullMessage);
    throw SemanticError(message, line, column);
}

void analyzeHelper::warning(const std::string &message, int line, int column)
{
    std::string fullMessage = message;
    if (line > 0)
    {
        fullMessage += " at line " + std::to_string(line);
        if (column > 0)
        {
            fullMessage += ", column " + std::to_string(column);
        }
    }
    // warningMessages.push_back(fullMessage);
    std::cerr << "Warning: " << fullMessage << std::endl;
}

void analyzeHelper::enterLoop()
{
    loopDepth++;
}

void analyzeHelper::exitLoop()
{
    loopDepth--;
}

bool analyzeHelper::isInLoop() const
{
    return loopDepth > 0;
}

bool analyzeHelper::isValidMainFunction(FunctionDef &funcDef)
{
    // 检查main函数是否合法
    if (funcDef.returnType != "int")
    {
        return false;
    }
    if (!funcDef.params.empty())
    {
        return false;
    }
    return true;
}

int analyzeHelper::getLineNumber(Expr &expr)
{
    // 这个方法需要基于AST节点的位置信息实现
    // 为简单起见，这里返回0
    return 0;
}

int analyzeHelper::getLineNumber(Stmt &stmt)
{
    // 这个方法需要基于AST节点的位置信息实现
    // 为简单起见，这里返回0
    return 0;
}