// SemanticAnalyzer.h
#pragma once
#include <string>
#include <stdexcept>
#include "parser/astVisitor.h"
#include "analyzeVisitor.h"

// 语义分析器
class SemanticAnalyzer
{
private:
    analyzeVisitor visitor = analyzeVisitor();
    bool success = true;

    // 需要？
    // 错误信息  不是直接在catch里return了?
    // std::vector<std::string> errorMessages;
    // 警告信息
    // std::vector<std::string> warningMessages;

public:
    // 分析入口（自顶向下扫ast）
    bool analyze(std::shared_ptr<CompUnit> ast);

    // 获取错误信息 没实现
    // const std::vector<std::string>& getErrors() const { return errorMessages; }
    // 获取警告信息 没实现
    // const std::vector<std::string>& getWarnings() const { return warningMessages; }
};
