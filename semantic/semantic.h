// SemanticAnalyzer.h
#pragma once
#include "parser/ast.h"
#include "parser/astVisitor.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>

class SemanticError : public std::runtime_error {
public:
    SemanticError(const std::string& message) : std::runtime_error(message) {}
};

struct Symbol {
    enum class Kind { VARIABLE, FUNCTION, PARAMETER };
    Kind kind;
    std::string type; // "int" 或 "void"
    bool initialized;
    
    Symbol(Kind kind, const std::string& type, bool initialized = false)
        : kind(kind), type(type), initialized(initialized) {}
};

struct FunctionInfo {
    std::string returnType;
    std::vector<std::string> paramTypes;
};

class SemanticAnalyzer : public ASTVisitor {
private:
    // 符号表栈，用于处理作用域
    std::vector<std::unordered_map<std::string, Symbol>> symbolTables;
    // 函数表
    std::unordered_map<std::string, FunctionInfo> functionTable;
    
    // 当前处理的函数返回类型
    std::string currentFunctionReturnType;
    // 是否在循环内（用于检查break/continue）
    bool inLoop = false;
    // 是否分析成功
    bool success = true;

public:
    SemanticAnalyzer();
    bool analyze(std::shared_ptr<CompUnit> ast);
    
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
    void enterScope();
    void exitScope();
    bool declareSymbol(const std::string& name, Symbol symbol);
    bool isDeclared(const std::string& name);
    Symbol* findSymbol(const std::string& name);
    void error(const std::string& message);
    std::string getExprType(Expr& expr);
};

