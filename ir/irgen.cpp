// ==== ir/irgen.cpp ====

#include "irgen.h"
#include <iostream>

namespace irgen {

using namespace ast;

namespace {

// 临时变量编号生成器
int tempVarCount = 0;
std::string newTemp() {
    return "t" + std::to_string(tempVarCount++);
}

// 递归处理表达式，返回结果变量名
std::string generateExpr(std::shared_ptr<Expr> expr, ir::IRList& ir) {
    if (auto intLit = std::dynamic_pointer_cast<IntLiteral>(expr)) {
        std::string dst = newTemp();
        ir.push_back({ir::IROp::Assign, dst, std::to_string(intLit->value), ""});
        return dst;
    }
    else if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        std::string lhs = generateExpr(bin->lhs, ir);
        std::string rhs = generateExpr(bin->rhs, ir);
        std::string dst = newTemp();
        ir::IROp op;

        if (bin->op == "+") op = ir::IROp::Add;
        else if (bin->op == "-") op = ir::IROp::Sub;
        else if (bin->op == "*") op = ir::IROp::Mul;
        else if (bin->op == "/") op = ir::IROp::Div;
        else op = ir::IROp::Add; // 默认

        ir.push_back({op, dst, lhs, rhs});
        return dst;
    }
    return "t_undef";
}

// 处理语句列表
void generateFunction(const std::shared_ptr<Function>& func, ir::IRList& ir) {
    for (const auto& stmt : func->body) {
        if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
            std::string retVal = generateExpr(ret->expr, ir);
            ir.push_back({ir::IROp::Return, "", retVal, ""});
        }
    }
}

} // 匿名 namespace

// 入口函数：AST -> IRList
ir::IRList generateIR(const std::shared_ptr<Program>& program) {
    ir::IRList ir;
    for (const auto& func : program->functions) {
        generateFunction(func, ir);
    }
    return ir;
}

} // namespace irgen
