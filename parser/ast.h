// AST.h
#pragma once
#include <string>
#include <vector>
#include <memory>

// 所有AST节点的基类
class ASTNode {
public:
    int line = 0;    // 源代码行号
    int column = 0;  // 源代码列号

    virtual ~ASTNode() = default;
    virtual void accept(class ASTVisitor& visitor) = 0;

    // 设置位置信息的方法
    void setLocation(int line, int column) {
        this->line = line;
        this->column = column;
    }
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
    
    NumberExpr(int value, int line = 0, int column = 0) : value(value) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// 变量引用表达式
class VariableExpr : public Expr {
public:
    std::string name;
    
    VariableExpr(const std::string& name, int line = 0, int column = 0) : name(name) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// 二元操作表达式
class BinaryExpr : public Expr {
public:
    std::shared_ptr<Expr> left;
    std::string op;
    std::shared_ptr<Expr> right;
    
    BinaryExpr(std::shared_ptr<Expr> left, const std::string& op, std::shared_ptr<Expr> right,
              int line = 0, int column = 0)
        : left(left), op(op), right(right) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// 一元操作表达式
class UnaryExpr : public Expr {
public:
    std::string op;
    std::shared_ptr<Expr> operand;
    
    UnaryExpr(const std::string& op, std::shared_ptr<Expr> operand,
             int line = 0, int column = 0)
        : op(op), operand(operand) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// 函数调用表达式
class CallExpr : public Expr {
public:
    std::string callee;
    std::vector<std::shared_ptr<Expr>> arguments;
    
    CallExpr(const std::string& callee, const std::vector<std::shared_ptr<Expr>>& arguments,
            int line = 0, int column = 0)
        : callee(callee), arguments(arguments) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// 表达式语句
class ExprStmt : public Stmt {
public:
    std::shared_ptr<Expr> expression;
    
    ExprStmt(std::shared_ptr<Expr> expression, int line = 0, int column = 0)
        : expression(expression) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// 变量声明
class VarDeclStmt : public Stmt {
public:
    std::string name;
    std::shared_ptr<Expr> initializer;
    
    VarDeclStmt(const std::string& name, std::shared_ptr<Expr> initializer,
               int line = 0, int column = 0)
        : name(name), initializer(initializer) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// 赋值语句
class AssignStmt : public Stmt {
public:
    std::string name;
    std::shared_ptr<Expr> value;
    
    AssignStmt(const std::string& name, std::shared_ptr<Expr> value,
              int line = 0, int column = 0)
        : name(name), value(value) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// 语句块
class BlockStmt : public Stmt {
public:
    std::vector<std::shared_ptr<Stmt>> statements;
    
    BlockStmt(const std::vector<std::shared_ptr<Stmt>>& statements,
             int line = 0, int column = 0)
        : statements(statements) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// if语句
class IfStmt : public Stmt {
public:
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> thenBranch;
    std::shared_ptr<Stmt> elseBranch; // 可能为nullptr
    
    IfStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch,
          int line = 0, int column = 0)
        : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// while语句
class WhileStmt : public Stmt {
public:
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> body;
    
    WhileStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> body,
             int line = 0, int column = 0)
        : condition(condition), body(body) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// break语句
class BreakStmt : public Stmt {
public:
    BreakStmt(int line = 0, int column = 0) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// continue语句
class ContinueStmt : public Stmt {
public:
    ContinueStmt(int line = 0, int column = 0) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// return语句
class ReturnStmt : public Stmt {
public:
    std::shared_ptr<Expr> value; // 可能为nullptr
    
    ReturnStmt(std::shared_ptr<Expr> value, int line = 0, int column = 0)
        : value(value) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// 函数参数
class Param {
public:
    std::string name;
    int line = 0;
    int column = 0;
    
    Param(const std::string& name, int line = 0, int column = 0)
        : name(name), line(line), column(column) {}
};

// 函数定义
class FunctionDef : public ASTNode {
public:
    std::string returnType; // "int" 或 "void"
    std::string name;
    std::vector<Param> params;
    std::shared_ptr<BlockStmt> body;
    
    FunctionDef(const std::string& returnType, const std::string& name, 
               const std::vector<Param>& params, std::shared_ptr<BlockStmt> body,
               int line = 0, int column = 0)
        : returnType(returnType), name(name), params(params), body(body) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// 编译单元（整个程序）
class CompUnit : public ASTNode {
public:
    std::vector<std::shared_ptr<FunctionDef>> functions;
    
    CompUnit(const std::vector<std::shared_ptr<FunctionDef>>& functions,
            int line = 0, int column = 0)
        : functions(functions) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};
