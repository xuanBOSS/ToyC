#pragma once
#include "parser/ast.h"
#include "ir.h"

namespace irgen {

ir::IRList generateIR(const std::shared_ptr<ast::Program>& program);

} // namespace irgen