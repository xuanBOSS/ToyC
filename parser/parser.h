// parser/parser.h
#pragma once
#include "lexer/lexer.h"
#include "ast.h"
#include <memory>
#include <vector>

namespace parser {

using ExprPtr = std::shared_ptr<ast::Expr>;
using ProgramPtr = std::shared_ptr<ast::Program>;

class Parser {
public:
    Parser(const std::vector<lexer::Token>& tokens);
    ExprPtr parse_expression();
private:
    std::vector<lexer::Token> tokens;
    int pos;
};

// 在头文件中声明 parse 函数（非类成员）
ProgramPtr parse(const std::vector<lexer::Token>& tokens);

} // namespace parser

