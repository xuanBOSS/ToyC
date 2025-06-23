#include "parser.h"
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens), current_(0) {}

ExprPtr Parser::parse_expression() {
    std::cout << "Parsing expression..." << std::endl;
    return std::make_shared<IntLiteral>(42);  // 示例返回
}
