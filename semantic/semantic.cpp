#include "semantic.h"
#include <iostream>
#include <sstream>

// 语义分析入口
bool SemanticAnalyzer::analyze(std::shared_ptr<CompUnit> ast)
{
    try
    {
        ast->accept(visitor);
        success = visitor.success;
        return success;
    }
    catch (const SemanticError &e)
    {
        std::string location = e.line > 0 ? " at line " + std::to_string(e.line) +
                                                (e.column > 0 ? ", column " + std::to_string(e.column) : "")
                                          : "";

        std::cerr << "Semantic error" << location << ": " << e.what() << std::endl;
        // errorMessages.push_back(std::string(e.what()) + location);
        return false;
    }
}
