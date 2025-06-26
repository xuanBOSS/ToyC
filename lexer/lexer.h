// Lexer.h
#pragma once
#include "Token.h"
#include <vector>
#include <string>
#include <unordered_map>

class Lexer {
private:
    std::string source;
    int position = 0;
    int line = 1;
    int column = 1;
    std::unordered_map<std::string, TokenType> keywords;
    std::unordered_map<std::string, TokenType> operators;

public:
    // 原有接口
    Lexer(const std::string& source);
    std::vector<Token> tokenize();
    
    // 新增接口
    Lexer(); // 默认构造函数
    Token nextToken(); // 获取下一个词法单元
    Token peekToken(); // 查看下一个词法单元但不消耗它
    int getLine() const { return line; }
    int getColumn() const { return column; }
    std::vector<Token> tokenize(const std::string& source); // 带参数版本
    
private:
    // 原有私有方法
    char peek(int offset = 0) const;
    char advance();
    bool isAtEnd() const;
    void skipWhitespace();
    Token scanToken();
    Token scanIdentifier();
    Token scanNumber();
    
    // 新增私有方法
    void skipComment();
    Token readOperatorOrPunctuator();
    void initOperators(); // 初始化运算符映射表
};