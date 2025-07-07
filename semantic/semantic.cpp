#include "semantic.h"
#include "analyzeHelper.h"
#include <iostream>
#include <sstream>

// 语义分析入口
bool SemanticAnalyzer::analyze(std::shared_ptr<CompUnit> ast)
{
    // 清空上次分析的错误和警告
    clearMessages();
    
    // 设置analyzeHelper的owner
    analyzeHelper::setSemanticOwner(*this);
    
    // 遍历AST进行语义分析
    ast->accept(visitor);
    
    // 执行额外的检查
    if (success) {
        // 检查未使用的变量
        checkUnusedVariables();
        
        // 检查死代码
        detectDeadCode();
    }
    
    // 输出所有收集到的错误
    for (const auto& error : errorMessages) {
        std::cerr << "Semantic error: " << error << std::endl;
    }
    
    // 输出所有收集到的警告
    for (const auto& warning : warningMessages) {
        std::cerr << "Warning: " << warning << std::endl;
    }
    
    return success;
}

// 检查未使用的变量
void SemanticAnalyzer::checkUnusedVariables()
{
    visitor.checkUnusedVariables();
}

// 检测死代码
void SemanticAnalyzer::detectDeadCode()
{
    visitor.detectDeadCode();
}
