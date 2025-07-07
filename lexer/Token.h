// Token.h
#pragma once
#include <string>

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

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
    
    Token(TokenType type, const std::string& lexeme, int line, int column)
        : type(type), lexeme(lexeme), line(line), column(column) {}
};
