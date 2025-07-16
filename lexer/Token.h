// Token.h - 定义了词法分析过程中产生的标记（Token）的结构和类型
#pragma once
#include <string>

// TokenType 枚举类 - 定义了所有可能的标记类型
enum class TokenType {
    // 关键字
    INT, VOID, IF, ELSE, WHILE, BREAK, CONTINUE, RETURN,
    
    // 标识符和字面量
    IDENTIFIER, NUMBER,
    
    // 运算符
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO,
    ASSIGN, 
    EQ, NEQ, LT, GT, LE, GE,
    AND, OR, NOT,
    
    // 分隔符
    LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON, COMMA,
    
    // 其他
    END_OF_FILE, UNKNOWN
};
// Token 结构体 - 表示源代码中的一个词法单元
struct Token {
    TokenType type;     // 标记类型
    std::string lexeme; // 词素（标记的原始文本）
    int line;           // 标记在源代码中的行号
    int column;         // 标记在源代码中的列号
    
    // 构造函数 - 初始化一个标记
    Token(TokenType type, const std::string& lexeme, int line, int column)
        : type(type), lexeme(lexeme), line(line), column(column) {}
};
