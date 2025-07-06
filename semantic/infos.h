#pragma once
#include <string>
#include <vector>

// 符号表（索引为ID）
struct Symbol
{
    enum class Kind
    {
        VARIABLE,
        FUNCTION,
        PARAMETER
    };
    Kind kind;
    std::string type; // "int" 或 "void"
    int line;         // 行
    int column;       // 列
    // 只用于参数的额外信息
    int paramIndex; // 参数索引

    Symbol() {}
    Symbol(Kind kind, const std::string &type,
           int line = 0, int column = 0, int paramIndex = -1)
        : kind(kind), type(type),
          line(line), column(column), paramIndex(paramIndex) {}
};

// 函数信息
struct FunctionInfo
{
    std::string returnType;              // 返回类型
    std::vector<std::string> paramTypes; // 参数类型列表
    std::vector<std::string> paramNames; // 参数名称列表
    int line;                            // 位置
    int column;                          // 位置

    FunctionInfo(const std::string &returnType = "void",
                 bool defined = false, int line = 0, int column = 0)
        : returnType(returnType), line(line), column(column) {}
};