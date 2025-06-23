#pragma once
#include <string>
#include <vector>

namespace ir {

enum class IROp {
    Add, Sub, Mul, Div,
    Assign,
    Return
};

struct IRInstr {
    IROp op;
    std::string dst;
    std::string arg1;
    std::string arg2;
};

using IRList = std::vector<IRInstr>;

} // namespace ir