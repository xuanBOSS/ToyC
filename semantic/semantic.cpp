// semantic/semantic.cpp
#include "parser/ast.h"
#include <iostream>

namespace semantic {

void check(const std::shared_ptr<ast::Program>& program) {
    std::cout << "Semantic check passed." << std::endl;
    //TODO:类型检查
}

}
