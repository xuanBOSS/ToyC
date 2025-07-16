// analyzeHelper.h - 包含常用语义分析任务的辅助方法的工具类
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

// OptionalInt - 替代 std::optional<int> 的简单结构体
// 用于表示可能存在也可能不存在的整数值
struct OptionalInt {
    bool hasValue;  // 是否有值
    int value;      // 存储的值
    
    // 默认构造函数 - 创建没有值的实例
    OptionalInt() : hasValue(false), value(0) {}
    
    // 带值构造函数 - 创建有值的实例
    OptionalInt(int v) : hasValue(true), value(v) {}
    
    // 检查是否有值
    bool has_value() const { return hasValue; }
    
    // 解引用操作符 - 获取存储的值
    int operator*() const { return value; }
    
    // 布尔转换操作符 - 检查是否有值
    explicit operator bool() const { return hasValue; }
};

// analyzeHelper - 语义分析辅助工具类
class analyzeHelper
{
private: 
    analyzeVisitor &owner;                  // 所属的分析访问者
    static SemanticAnalyzer* semanticOwner; // 添加指向语义分析器的指针

    // 用于跟踪已报告的错误和警告，防止重复
    std::set<std::string> reportedErrors;  // 已报告的错误集合
    std::set<std::string> reportedWarnings;// 已报告的警告集合

public:
    explicit analyzeHelper(analyzeVisitor &owner) : owner(owner) {}

    // 设置语义分析器
    static void setSemanticOwner(SemanticAnalyzer& analyzer);

    // === 作用域管理 ===
    
    // 进入新作用域
    void enterScope();
    
    // 退出当前作用域
    void exitScope();

    // === 特殊检查 ===
    
    // 检查main函数是否有效
    bool isValidMainFunction(FunctionDef &funcDef);

    // === 符号表操作 ===
    
    // 在当前作用域声明符号
    bool declareSymbol(const std::string &name, Symbol symbol); 
    
    // 在所有可见作用域中查找符号
    Symbol *findSymbol(const std::string &name);                  

    // === 循环控制流检查 ===
    
    int loopDepth = 0;  // 当前循环嵌套深度
    
    // 进入循环
    void enterLoop();
    
    // 退出循环
    void exitLoop();
    
    // 检查是否在循环内
    bool isInLoop() const;

    // === 常量表达式求值 ===
    
    // 尝试在编译时计算表达式的值（用于除零检查和死代码检测）
    OptionalInt evaluateConstant(const std::shared_ptr<Expr>& expr);

    // === 错误和警告处理 ===
    
    // 报告错误
    void error(const std::string &message, int line = 0, int column = 0);
    
    // 报告警告
    void warning(const std::string &message, int line = 0, int column = 0);


    // === 高级检查 ===
    
    // 未使用变量检查
    void checkUnusedVariables();

    // 死代码检测
    void detectDeadCode(const std::shared_ptr<Stmt>& stmt);
    
    // 函数调用验证
    bool validateFunctionCall(const std::string& name, const std::vector<std::shared_ptr<Expr>>& args, int line, int column);
    
    // 类型兼容性检查
    bool checkTypeCompatibility(const std::shared_ptr<Expr>& expr, const std::string& expectedType, int line, int column);

     // === 辅助方法 ===
    
    // 获取表达式的行号
    int getLineNumber(Expr &expr) {
        return expr.line;
    }

    // 获取语句的行号
    int getLineNumber(Stmt &stmt) {
        return stmt.line;
    }
    
    // 获取语句指针的行号
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
