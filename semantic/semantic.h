#pragma once
#include "parser/ast.h"
#include <string>
#include <unordered_map>

namespace semantic {

struct Symbol {
    std::string name;
    std::string type;
};

class SymbolTable {
public:
    void enterScope();
    void exitScope();
    void declare(const Symbol& symbol);
    const Symbol* lookup(const std::string& name) const;

private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes;
};

void check(const std::shared_ptr<ast::Program>& program);

} // namespace semantic