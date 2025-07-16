// parser/parser.h - 定义了语法分析器的接口和结构
#pragma once
#include "lexer/Token.h"
#include "parser/ast.h"
#include <vector>
#include <memory>
#include <stdexcept>

//用于在语法分析过程中抛出特定的解析错误
class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message) : std::runtime_error(message) {}
};

//实现了一个自顶向下的语法分析器，根据文法规则解析Token序列生成AST
class Parser {
private:
    std::vector<Token> tokens;
    int current = 0;
    bool hadError = false;  // 添加一个标记，记录是否遇到过错误
    int errorCount = 0;  // 添加错误计数
    bool isRecovering = false;  // 标记是否正在从错误中恢复

public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens) {}
    // 开始解析过程，返回编译单元AST根节点
    std::shared_ptr<CompUnit> parse();
    bool hasError() const { return hadError; }  // 公共方法返回是否有错误

private:
    // 辅助方法
    // 查看当前位置向前偏移的标记
    Token peek(int offset) const {
        if (current + offset >= tokens.size()) {
            return tokens.back(); // 返回EOF token
        }
        return tokens[current + offset];
    }
    Token previous() const;// 返回前一个已处理的标记
    bool isAtEnd() const;// 检查是否已到达标记序列末尾
    Token advance();// 前进到下一个标记并返回当前标记
    bool check(TokenType type) const;// 检查当前标记是否为指定类型
    bool match(std::initializer_list<TokenType> types);// 尝试匹配指定类型的标记，成功则消费该标记
    Token consume(TokenType type, const std::string& message);// 消费指定类型的标记，如果类型不匹配则报错
    ParseError error(const Token& token, const std::string& message);// 生成解析错误
    void synchronize();// 错误恢复：同步到下一个语句或声明的开始

    // 递归下降解析方法 - 按照文法从上到下实现
    std::shared_ptr<CompUnit> compUnit();    // 解析编译单元
    std::shared_ptr<FunctionDef> funcDef();  // 解析函数定义
    Param param();                           // 解析函数参数
    std::shared_ptr<Stmt> stmt();           // 解析语句
    std::shared_ptr<BlockStmt> block();     // 解析语句块
    std::shared_ptr<Stmt> exprStmt();       // 解析表达式语句
    std::shared_ptr<Stmt> varDeclStmt();    // 解析变量声明语句
    std::shared_ptr<Stmt> assignStmt();     // 解析赋值语句
    std::shared_ptr<Stmt> ifStmt();         // 解析if语句
    std::shared_ptr<Stmt> whileStmt();      // 解析while语句
    std::shared_ptr<Stmt> breakStmt();      // 解析break语句
    std::shared_ptr<Stmt> continueStmt();   // 解析continue语句
    std::shared_ptr<Stmt> returnStmt();     // 解析return语句
    
    // 表达式解析方法
    std::shared_ptr<Expr> expr();           // 解析表达式
    std::shared_ptr<Expr> lorExpr();        // 解析逻辑或表达式
    std::shared_ptr<Expr> landExpr();       // 解析逻辑与表达式
    std::shared_ptr<Expr> relExpr();        // 解析关系表达式
    std::shared_ptr<Expr> addExpr();        // 解析加减表达式
    std::shared_ptr<Expr> mulExpr();        // 解析乘除模表达式
    std::shared_ptr<Expr> unaryExpr();      // 解析一元表达式
    std::shared_ptr<Expr> primaryExpr();    // 解析基础表达式
};

