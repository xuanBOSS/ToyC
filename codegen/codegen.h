#pragma once
#include "ir/ir.h"
#include <string>

namespace codegen {

void generate(const ir::IRList& ir, const std::string& out_path);

} // namespace codegen
