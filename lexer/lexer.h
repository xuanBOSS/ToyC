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
    // ԭ�нӿ�
    Lexer(const std::string& source);
    std::vector<Token> tokenize();

    // �����ӿ�
    Lexer(); // Ĭ�Ϲ��캯��
    Token nextToken(); // ��ȡ��һ���ʷ���Ԫ
    Token peekToken(); // �鿴��һ���ʷ���Ԫ����������
    int getLine() const { return line; }
    int getColumn() const { return column; }
    std::vector<Token> tokenize(const std::string& source); // �������汾

private:
    // ԭ��˽�з���
    char peek(int offset = 0) const;
    char advance();
    bool isAtEnd() const;
    void skipWhitespace();
    Token scanToken();
    Token scanIdentifier();
    Token scanNumber();

    // ����˽�з���
    void skipComment();
    Token readOperatorOrPunctuator();
    void initOperators(); // ��ʼ�������ӳ���
};