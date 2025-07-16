// AST.h - 定义了抽象语法树(Abstract Syntax Tree)的各种节点类型
#pragma once
#include <string>
#include <vector>
#include <memory>

// ASTNode - 所有AST节点的基类，提供基本的位置信息和访问者模式接口
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

// Expr - 表达式节点的基类，所有表达式类型都继承自此类
class Expr : public ASTNode {
public:
    virtual ~Expr() = default;
};

// Stmt - 语句节点的基类，所有语句类型都继承自此类
class Stmt : public ASTNode {
public:
    virtual ~Stmt() = default;
};

// NumberExpr - 表示数字字面量的表达式节点
class NumberExpr : public Expr {
public:
    int value;// 数字的值
    
    NumberExpr(int value, int line = 0, int column = 0) : value(value) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// VariableExpr - 表示变量引用的表达式节点
class VariableExpr : public Expr {
public:
    std::string name;
    
    VariableExpr(const std::string& name, int line = 0, int column = 0) : name(name) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// BinaryExpr - 表示二元操作的表达式节点(如加减乘除、比较、逻辑运算等)
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

// UnaryExpr - 表示一元操作的表达式节点(如正负号、逻辑非等)
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

// CallExpr - 表示函数调用的表达式节点
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

// ExprStmt - 表示表达式语句的节点(如函数调用语句)
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

// VarDeclStmt - 表示变量声明语句的节点
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

// AssignStmt - 表示赋值语句的节点
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

// BlockStmt - 表示语句块的节点(由大括号括起的一系列语句)
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

// IfStmt - 表示if条件语句的节点
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

// WhileStmt - 表示while循环语句的节点
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

// BreakStmt - 表示break语句的节点
class BreakStmt : public Stmt {
public:
    BreakStmt(int line = 0, int column = 0) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// ContinueStmt - 表示continue语句的节点
class ContinueStmt : public Stmt {
public:
    ContinueStmt(int line = 0, int column = 0) {
        this->line = line;
        this->column = column;
    }
    void accept(ASTVisitor& visitor) override;
};

// ReturnStmt - 表示return语句的节点
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

// Param - 表示函数参数的类
class Param {
public:
    std::string name;
    int line = 0;
    int column = 0;
    
    Param(const std::string& name, int line = 0, int column = 0)
        : name(name), line(line), column(column) {}
};

// FunctionDef - 表示函数定义的节点
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

// CompUnit - 表示编译单元的节点(整个程序的根节点)
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
