#pragma once
#include <string>
#include <optional>
#include <stdexcept>
#include <memory>
#include "infos.h"
#include "parser/ast.h"

class analyzeVisitor;

class analyzeHelper
{
private:
    analyzeVisitor &owner;

public:
    explicit analyzeHelper(analyzeVisitor &owner) : owner(owner) {}

    // 进出作用域
    void enterScope();
    void exitScope();

    // 检查main函数
    bool isValidMainFunction(FunctionDef &funcDef);

    // 符号表的增加、查找
    bool declareSymbol(const std::string &name, Symbol symbol); 
    Symbol *findSymbol(const std::string &name);               

    // 循环流检查
    int loopDepth = 0;
    void enterLoop();
    void exitLoop();
    bool isInLoop() const;

    // 表达式求值（用于除以0检查）
    std::optional<int> evaluateConstant(const std::shared_ptr<Expr>& expr);

    // 抛出错误/输出警告
    void error(const std::string &message, int line = 0, int column = 0);
    void warning(const std::string &message, int line = 0, int column = 0);

    // 未使用变量检查（生成警告） 没实现
    // void checkUnusedVariables();

    // 定位 没实现
    int getLineNumber(Expr &expr);
    int getLineNumber(Stmt &stmt);
    int getLineNumber(const std::shared_ptr<Stmt> &stmt)
    {
        if (stmt)
        {
            return getLineNumber(*stmt);
        }
        return 0;
    }
};

// 语义错误异常类
class SemanticError : public std::runtime_error
{
public:
    int line;
    int column;

    SemanticError(const std::string &message, int line = 0, int column = 0)
        : std::runtime_error(message), line(line), column(column) {}
};
