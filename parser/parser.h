#pragma once
#include "../lexer/lexer.h"
#include "ast.h"

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    ExprPtr parse_expression();  // 示例函数
private:
    const std::vector<Token>& tokens_;
    int current_;
};
