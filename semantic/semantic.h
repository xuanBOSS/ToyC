// SemanticAnalyzer.h
#pragma once
#include "parser/ast.h"
#include "parser/astVisitor.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <iostream>

// 语义错误异常类
class SemanticError : public std::runtime_error {
public:
    int line;    // 错误发生的行号
    int column;  // 错误发生的列号

    SemanticError(const std::string& message, int line = 0, int column = 0) 
        : std::runtime_error(message), line(line), column(column) {}
};

//符号类型
struct Symbol {
    enum class Kind { VARIABLE, FUNCTION, PARAMETER };
    Kind kind;
    std::string type;     // "int" 或 "void"
    bool initialized;     // 变量是否已初始化
    int line;             // 声明位置
    int column;           // 声明位置
    
    // 用于参数的额外信息
    int paramIndex;       // 参数索引
    
    Symbol(Kind kind, const std::string& type, bool initialized = false, 
           int line = 0, int column = 0, int paramIndex = -1)
        : kind(kind), type(type), initialized(initialized), 
          line(line), column(column), paramIndex(paramIndex) {}
};

//函数信息
struct FunctionInfo {
    std::string returnType;              // 返回类型
    std::vector<std::string> paramTypes; // 参数类型列表
    std::vector<std::string> paramNames; // 参数名称列表
    bool defined;                        // 是否已定义(不仅声明)
    int line;                            // 定义位置
    int column;                          // 定义位置
    
    FunctionInfo(const std::string& returnType = "void", 
                 bool defined = false, int line = 0, int column = 0)
        : returnType(returnType), defined(defined), line(line), column(column) {}
};

//语义分析器
class SemanticAnalyzer : public ASTVisitor {
private:
    // 符号表栈，用于处理作用域
    std::vector<std::unordered_map<std::string, Symbol>> symbolTables;
    // 函数表
    std::unordered_map<std::string, FunctionInfo> functionTable;
    // 当前处理的函数
    std::string currentFunction;
    // 当前处理的函数返回类型
    std::string currentFunctionReturnType;
    // 嵌套深度计数器，用于检查break/continue
    int loopDepth = 0;
    // 是否已经有return语句，用于检查是否所有路径都有返回值
    bool hasReturn = false;
    // 当前语句的父语句，用于控制流分析
    std::vector<std::shared_ptr<Stmt>> stmtStack;
    // 错误信息
    std::vector<std::string> errorMessages;
    // 警告信息
    std::vector<std::string> warningMessages;
    // 是否分析成功
    bool success = true;

public:
    SemanticAnalyzer();
    ~SemanticAnalyzer();

    // 分析入口
    bool analyze(std::shared_ptr<CompUnit> ast);
    
    // 获取错误信息
    const std::vector<std::string>& getErrors() const { return errorMessages; }
    
    // 获取警告信息
    const std::vector<std::string>& getWarnings() const { return warningMessages; }
    
    // 添加内置函数到函数表
    void addBuiltinFunctions();

    // 实现ASTVisitor接口
    void visit(NumberExpr& expr) override;
    void visit(VariableExpr& expr) override;
    void visit(BinaryExpr& expr) override;
    void visit(UnaryExpr& expr) override;
    void visit(CallExpr& expr) override;
    
    void visit(ExprStmt& stmt) override;
    void visit(VarDeclStmt& stmt) override;
    void visit(AssignStmt& stmt) override;
    void visit(BlockStmt& stmt) override;
    void visit(IfStmt& stmt) override;
    void visit(WhileStmt& stmt) override;
    void visit(BreakStmt& stmt) override;
    void visit(ContinueStmt& stmt) override;
    void visit(ReturnStmt& stmt) override;
    
    void visit(FunctionDef& funcDef) override;
    void visit(CompUnit& compUnit) override;

private:
    //作用域管理
    void enterScope();
    void exitScope();

    //符号管理
    bool declareSymbol(const std::string& name, Symbol symbol);
    bool isDeclared(const std::string& name);
    bool isLocallyDeclared(const std::string& name);
    Symbol* findSymbol(const std::string& name);

    // 函数管理
    bool declareFunction(const std::string& name, const FunctionInfo& info);
    bool isFunctionDeclared(const std::string& name);
    FunctionInfo* findFunction(const std::string& name);
    
    // 错误处理
    void error(const std::string& message, int line = 0, int column = 0);
    void warning(const std::string& message, int line = 0, int column = 0);
    
    // 类型检查
    std::string getExprType(Expr& expr);
    bool checkAssignable(const std::string& leftType, const std::string& rightType);
    bool checkCallable(CallExpr& expr);
    
    // 控制流检查
    void enterLoop();
    void exitLoop();
    bool isInLoop() const;
    void checkReturnPaths(BlockStmt& block, bool isFunction = false);
    
    // 未使用变量检查
    void checkUnusedVariables();
    
    // 其他辅助方法
    bool isValidMainFunction(FunctionDef& funcDef);
    void checkInitialized(const std::string& name);
    void markInitialized(const std::string& name);
    int getLineNumber(Expr& expr);  // 获取表达式所在行号
    int getLineNumber(Stmt& stmt);  // 获取语句所在行号
    int getLineNumber(const std::shared_ptr<Stmt>& stmt) {
    if (stmt) {
        return getLineNumber(*stmt);
    }
    return 0;
}
};

