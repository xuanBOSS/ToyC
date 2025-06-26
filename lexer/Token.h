#pragma once
#include <string>

enum class TokenType {
    //�ؼ���
    INT, VOID, IF, ELSE, WHILE, BREAK, CONTINUE, RETURN,

    //��ʶ����������
    IDENTIFIER, NUMBER,

    //�����
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO,
    ASSIGN,
    EQ, NEQ, LT, GT, LE, GE,
    AND, OR, NOT,

    //�ָ���
    LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON, COMMA,

    //����
    END_OF_FILE, UNKNOWN
};

struct Token 
{
    TokenType type;    //token����
    std::string lexeme;//token����
    int line;          //�����к�
    int column;        //�����к�

    Token(TokenType type, const std::string& lexeme, int line, int column)
        : type(type), lexeme(lexeme), line(line), column(column) 
    {}
};
