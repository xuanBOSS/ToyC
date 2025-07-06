#pragma once
#include <vector>
#include <unordered_map>
#include "parser/astVisitor.h"
#include "typeVisitor.h"
#include "analyzeHelper.h"
#include "infos.h"

class analyzeVisitor : public ASTVisitor
{
private:
    // 符号表栈，用于处理作用域
    std::vector<std::unordered_map<std::string, Symbol>> symbolTables;
    // 函数表
    std::unordered_map<std::string, FunctionInfo> functionTable;
    // 正在分析的函数
    std::string currentFunction;
    // 正在分析的函数的返回类型
    std::string currentFunctionReturnType;
    // 当前函数是否有return语句
    bool hasReturn = false;

    // 类型检查器
    typeVisitor typeChecker;

public:
    bool success = true;

    // 辅助函数类 （给typeVisitor用,故设为公用）
    analyzeHelper helper;

    analyzeVisitor();
    ~analyzeVisitor();

    // 表达式
    void visit(NumberExpr &expr) override;
    void visit(VariableExpr &expr) override;
    void visit(BinaryExpr &expr) override;
    void visit(UnaryExpr &expr) override;
    void visit(CallExpr &expr) override;
    // 语句
    void visit(ExprStmt &stmt) override;
    void visit(VarDeclStmt &stmt) override;
    void visit(AssignStmt &stmt) override;
    void visit(BlockStmt &stmt) override;
    void visit(IfStmt &stmt) override;
    void visit(WhileStmt &stmt) override;
    void visit(BreakStmt &stmt) override;
    void visit(ContinueStmt &stmt) override;
    void visit(ReturnStmt &stmt) override;
    // 其它
    void visit(FunctionDef &funcDef) override;
    void visit(CompUnit &compUnit) override;

    // 暴露符号表、函数表给辅助函数
    std::vector<std::unordered_map<std::string, Symbol>> &getSymbolTables() { return symbolTables; }
    std::unordered_map<std::string, FunctionInfo> &getFunctionTable() { return functionTable; }
};

