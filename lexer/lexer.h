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

public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();
    
private:
    char peek(int offset = 0) const;
    char advance();
    bool isAtEnd() const;
    void skipWhitespace();
    Token scanToken();
    Token scanIdentifier();
    Token scanNumber();
};