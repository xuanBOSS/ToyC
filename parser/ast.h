// AST.h
#pragma once
#include <string>
#include <vector>
#include <memory>

// 所有AST节点的基类
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(class ASTVisitor& visitor) = 0;
};

// 表达式节点基类
class Expr : public ASTNode {
public:
    virtual ~Expr() = default;
};

// 语句节点基类
class Stmt : public ASTNode {
public:
    virtual ~Stmt() = default;
};

// 数字字面量
class NumberExpr : public Expr {
public:
    int value;
    
    NumberExpr(int value) : value(value) {}
    void accept(ASTVisitor& visitor) override;
};

// 变量引用表达式
class VariableExpr : public Expr {
public:
    std::string name;
    
    VariableExpr(const std::string& name) : name(name) {}
    void accept(ASTVisitor& visitor) override;
};

// 二元操作表达式
class BinaryExpr : public Expr {
public:
    std::shared_ptr<Expr> left;
    std::string op;
    std::shared_ptr<Expr> right;
    
    BinaryExpr(std::shared_ptr<Expr> left, const std::string& op, std::shared_ptr<Expr> right)
        : left(left), op(op), right(right) {}
    void accept(ASTVisitor& visitor) override;
};

// 一元操作表达式
class UnaryExpr : public Expr {
public:
    std::string op;
    std::shared_ptr<Expr> operand;
    
    UnaryExpr(const std::string& op, std::shared_ptr<Expr> operand)
        : op(op), operand(operand) {}
    void accept(ASTVisitor& visitor) override;
};

// 函数调用表达式
class CallExpr : public Expr {
public:
    std::string callee;
    std::vector<std::shared_ptr<Expr>> arguments;
    
    CallExpr(const std::string& callee, const std::vector<std::shared_ptr<Expr>>& arguments)
        : callee(callee), arguments(arguments) {}
    void accept(ASTVisitor& visitor) override;
};

// 表达式语句
class ExprStmt : public Stmt {
public:
    std::shared_ptr<Expr> expression;
    
    ExprStmt(std::shared_ptr<Expr> expression) : expression(expression) {}
    void accept(ASTVisitor& visitor) override;
};

// 变量声明
class VarDeclStmt : public Stmt {
public:
    std::string name;
    std::shared_ptr<Expr> initializer;
    
    VarDeclStmt(const std::string& name, std::shared_ptr<Expr> initializer)
        : name(name), initializer(initializer) {}
    void accept(ASTVisitor& visitor) override;
};

// 赋值语句
class AssignStmt : public Stmt {
public:
    std::string name;
    std::shared_ptr<Expr> value;
    
    AssignStmt(const std::string& name, std::shared_ptr<Expr> value)
        : name(name), value(value) {}
    void accept(ASTVisitor& visitor) override;
};

// 语句块
class BlockStmt : public Stmt {
public:
    std::vector<std::shared_ptr<Stmt>> statements;
    
    BlockStmt(const std::vector<std::shared_ptr<Stmt>>& statements)
        : statements(statements) {}
    void accept(ASTVisitor& visitor) override;
};

// if语句
class IfStmt : public Stmt {
public:
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> thenBranch;
    std::shared_ptr<Stmt> elseBranch; // 可能为nullptr
    
    IfStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch)
        : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}
    void accept(ASTVisitor& visitor) override;
};

// while语句
class WhileStmt : public Stmt {
public:
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> body;
    
    WhileStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> body)
        : condition(condition), body(body) {}
    void accept(ASTVisitor& visitor) override;
};

// break语句
class BreakStmt : public Stmt {
public:
    void accept(ASTVisitor& visitor) override;
};

// continue语句
class ContinueStmt : public Stmt {
public:
    void accept(ASTVisitor& visitor) override;
};

// return语句
class ReturnStmt : public Stmt {
public:
    std::shared_ptr<Expr> value; // 可能为nullptr
    
    ReturnStmt(std::shared_ptr<Expr> value) : value(value) {}
    void accept(ASTVisitor& visitor) override;
};

// 函数参数
class Param {
public:
    std::string name;
    
    Param(const std::string& name) : name(name) {}
};

// 函数定义
class FunctionDef : public ASTNode {
public:
    std::string returnType; // "int" 或 "void"
    std::string name;
    std::vector<Param> params;
    std::shared_ptr<BlockStmt> body;
    
    FunctionDef(const std::string& returnType, const std::string& name, 
               const std::vector<Param>& params, std::shared_ptr<BlockStmt> body)
        : returnType(returnType), name(name), params(params), body(body) {}
    void accept(ASTVisitor& visitor) override;
};

// 编译单元（整个程序）
class CompUnit : public ASTNode {
public:
    std::vector<std::shared_ptr<FunctionDef>> functions;
    
    CompUnit(const std::vector<std::shared_ptr<FunctionDef>>& functions)
        : functions(functions) {}
    void accept(ASTVisitor& visitor) override;
};
