// ==== codegen/codegen.cpp ====

#include "codegen.h"
#include <fstream>
#include <iostream>
#include <filesystem>  // ✅ 使用 std::filesystem 创建目录

namespace fs = std::filesystem;

namespace codegen {

void generate(const ir::IRList& ir, const std::string& out_path) {
    // 自动创建 output 目录
    fs::create_directories("output");

    std::ofstream out(out_path);
    if (!out.is_open()) {
        std::cerr << "❌ Error: Cannot open output file: " << out_path << "\n";
        return;
    }

    out << ".text\n";
    out << ".globl main\n";
    out << "main:\n";

    for (const auto& instr : ir) {
        switch (instr.op) {
            case ir::IROp::Assign:
                out << "  li " << instr.dst << ", " << instr.arg1 << "\n";
                break;
            case ir::IROp::Add:
                out << "  add " << instr.dst << ", " << instr.arg1 << ", " << instr.arg2 << "\n";
                break;
            case ir::IROp::Sub:
                out << "  sub " << instr.dst << ", " << instr.arg1 << ", " << instr.arg2 << "\n";
                break;
            case ir::IROp::Mul:
                out << "  mul " << instr.dst << ", " << instr.arg1 << ", " << instr.arg2 << "\n";
                break;
            case ir::IROp::Div:
                out << "  div " << instr.dst << ", " << instr.arg1 << ", " << instr.arg2 << "\n";
                break;
            case ir::IROp::Return:
                out << "  mv a0, " << instr.arg1 << "\n";
                out << "  ret\n";
                break;
            default:
                out << "  # Unknown IR operation\n";
                break;
        }
    }

    std::cout << "✅ [CodeGen] Assembly written to " << out_path << "\n";
}

} // namespace codegen
