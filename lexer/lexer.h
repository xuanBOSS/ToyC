#pragma once
#include <string>
#include <vector>

namespace lexer {

enum class TokenType { Identifier, Keyword, Number, Symbol, Operator, String, EndOfFile };

struct Token {
    TokenType type;
    std::string lexeme;
    int line, column;
};

std::vector<Token> tokenize(const std::string& source);

} // namespace lexer