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

    // 添加函数相关信息
    std::vector<std::pair<std::string, std::string>> params; // 参数列表 <参数名, 参数类型>

    // 添加使用状态标记
    bool used = false; // 变量是否被使用

    Symbol() {}

    // 普通变量或参数的构造函数
    Symbol(Kind kind, const std::string &type,
           int line = 0, int column = 0, int paramIndex = -1)
        : kind(kind), type(type),
          line(line), column(column), paramIndex(paramIndex), used(false) {}
          
    // 函数的构造函数
    Symbol(Kind kind, const std::string &type,
           const std::vector<std::pair<std::string, std::string>>& parameters,
           int line = 0, int column = 0)
        : kind(kind), type(type),
          line(line), column(column), paramIndex(-1), params(parameters), used(false) {}
};

// 函数信息
struct FunctionInfo
{
    std::string returnType;              // 返回类型
    std::vector<std::string> paramTypes; // 参数类型列表
    std::vector<std::string> paramNames; // 参数名称列表
    int line;                            // 位置
    int column;                          // 位置
    bool used = false;                   // 函数是否被调用

    FunctionInfo(const std::string &returnType = "void",
                 bool defined = false, int line = 0, int column = 0)
        : returnType(returnType), line(line), column(column), used(false) {}
};
