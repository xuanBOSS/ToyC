#pragma once
#include <string>
#include <stdexcept>
#include <vector>
#include "parser/astVisitor.h"
#include "analyzeVisitor.h"

// 语义分析器
class SemanticAnalyzer
{
private:
    analyzeVisitor visitor;

public:
    SemanticAnalyzer() : visitor() {}
    
    bool success = true;
    // 错误信息集合
    std::vector<std::string> errorMessages;
    // 警告信息集合
    std::vector<std::string> warningMessages;

    // 分析入口（自顶向下扫ast）
    bool analyze(std::shared_ptr<CompUnit> ast);

    // 获取错误信息
    const std::vector<std::string>& getErrors() const { return errorMessages; }
    // 获取警告信息
    const std::vector<std::string>& getWarnings() const { return warningMessages; }
    
    // 执行未使用变量检查
    void checkUnusedVariables();
    
    // 执行死代码检测
    void detectDeadCode();
    
    // 清空错误和警告
    void clearMessages() {
        errorMessages.clear();
        warningMessages.clear();
        success = true;
    }
};
