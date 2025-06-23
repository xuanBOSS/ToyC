#pragma once
#include <string>
#include <vector>

enum class TokenType {
    Identifier, Keyword, Number, Symbol, EndOfFile
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
};

std::vector<Token> tokenize(const std::string& source);
