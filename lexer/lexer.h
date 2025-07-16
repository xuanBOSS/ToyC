// Lexer.h - 定义了词法分析器的接口和基本结构
#pragma once
#include "Token.h"
#include <vector>
#include <string>
#include <unordered_map>

// Lexer 类 - 负责将源代码字符串分解为标记序列
class Lexer {
private:
    std::string source;   //源代码字符串
    int position = 0;     //当前处理位置
    int line = 1;         //当前行号
    int column = 1;       //当前列号
    // 关键字映射表：将关键字字符串映射到相应的标记类型
    std::unordered_map<std::string, TokenType> keywords;
    // 运算符映射表：将运算符字符串映射到相应的标记类型
    std::unordered_map<std::string, TokenType> operators;

public:
    // === 公共接口 ===

    // 带参数的构造函数 - 使用源代码初始化词法分析器
    Lexer(const std::string& source);
    // 标记化方法 - 将源代码转换为标记序列
    std::vector<Token> tokenize();
    
    Lexer(); // 默认构造函数
    Token nextToken(); // 获取下一个词法单元
    Token peekToken(); // 查看下一个词法单元但不消耗它
    int getLine() const { return line; }// 获取当前行号
    int getColumn() const { return column; }// 获取当前列号
    // 带参数的标记化方法 - 使用提供的源代码创建标记序列
    std::vector<Token> tokenize(const std::string& source); // 带参数版本
    
private:
    // 原有私有方法
    char peek(int offset = 0) const;// 查看当前位置向前偏移的字符
    char advance();// 读取当前字符并前进
    bool isAtEnd() const;// 检查是否已到达源代码末尾
    void skipWhitespace();// 跳过空白字符
    Token scanToken();// 扫描并返回下一个标记
    Token scanIdentifier();// 扫描标识符或关键字
    Token scanNumber();// 扫描数字字面量
    void skipComment();// 跳过注释
    Token readOperatorOrPunctuator();// 读取运算符或标点符号
    void initOperators(); // 初始化运算符映射表
};
