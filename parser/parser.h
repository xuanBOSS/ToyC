#pragma once
#include "lexer/lexer.h"
#include "ast.h"
#include <vector>
#include <memory>

namespace parser {

std::shared_ptr<ast::Program> parse(const std::vector<lexer::Token>& tokens);

} // namespace parser