//------------------包含常用语义分析任务的辅助方法的工具类-----------
#pragma once
#include <string>
#include <optional>
#include <unordered_map>
#include <stdexcept>
#include <memory>
#include <vector>
#include <set>
#include "infos.h"
#include "parser/ast.h"

class analyzeVisitor;
class SemanticAnalyzer;

// 替代 std::optional 的简单结构体
struct OptionalInt {
    bool hasValue;
    int value;
    
    OptionalInt() : hasValue(false), value(0) {}
    OptionalInt(int v) : hasValue(true), value(v) {}
    
    bool has_value() const { return hasValue; }
    int operator*() const { return value; }
    explicit operator bool() const { return hasValue; }
};

class analyzeHelper
{
private:
    analyzeVisitor &owner;
    static SemanticAnalyzer* semanticOwner; // 添加指向语义分析器的指针

    // 用于跟踪已报告的错误和警告，防止重复
    std::set<std::string> reportedErrors;
    std::set<std::string> reportedWarnings;

public:
    explicit analyzeHelper(analyzeVisitor &owner) : owner(owner) {}

    // 设置语义分析器
    static void setSemanticOwner(SemanticAnalyzer& analyzer);

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
    OptionalInt evaluateConstant(const std::shared_ptr<Expr>& expr);

    // 抛出错误/输出警告
    void error(const std::string &message, int line = 0, int column = 0);
    void warning(const std::string &message, int line = 0, int column = 0);

    // 未使用变量检查（生成警告） 没实现
    void checkUnusedVariables();

    // 死代码检测
    void detectDeadCode(const std::shared_ptr<Stmt>& stmt);
    
    // 增强函数调用分析
    bool validateFunctionCall(const std::string& name, const std::vector<std::shared_ptr<Expr>>& args, int line, int column);
    
    // 增强类型检查
    bool checkTypeCompatibility(const std::shared_ptr<Expr>& expr, const std::string& expectedType, int line, int column);

    // 定位
    int getLineNumber(Expr &expr) {
        return expr.line;
    }

    int getLineNumber(Stmt &stmt) {
        return stmt.line;
    }
    
    int getLineNumber(const std::shared_ptr<Stmt> &stmt)
    {
        if (stmt)
        {
            return getLineNumber(*stmt);
        }
        return 0;
    }

    // 重置错误和警告跟踪
    void resetReports() {
        reportedErrors.clear();
        reportedWarnings.clear();
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
