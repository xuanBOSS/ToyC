#include "parser.h"
#include "parser/ast.h"
#include <iostream>

namespace parser {

Parser::Parser(const std::vector<lexer::Token>& tokens)
    : tokens(tokens), pos(0) {}

ExprPtr Parser::parse_expression() {
    std::cout << "Parsing expression..." << std::endl;
    return std::make_shared<ast::IntLiteral>(42);  // 示例
}

ProgramPtr parse(const std::vector<lexer::Token>& tokens) {
    Parser p(tokens);
    std::vector<ExprPtr> exprs;
    exprs.push_back(p.parse_expression());
    return std::make_shared<ast::Program>(exprs);
}


} // namespace parser
