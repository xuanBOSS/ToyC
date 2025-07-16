// infos.h - 定义了符号表和函数信息结构
#pragma once
#include <string>
#include <vector>

// Symbol - 符号表条目，表示变量、函数或参数
struct Symbol
{
   // 符号种类枚举
    enum class Kind
    {
        VARIABLE,  // 变量
        FUNCTION,  // 函数
        PARAMETER  // 函数参数
    };

    Kind kind;                 // 符号种类
    std::string type;          // 符号类型 ("int" 或 "void")
    int line;                  // 符号在源代码中的行号
    int column;                // 符号在源代码中的列号
    int paramIndex;            // 参数索引（仅用于函数参数）

    // 函数相关信息
    std::vector<std::pair<std::string, std::string>> params; // 参数列表 <参数名, 参数类型>
    
    // 使用状态标记（用于未使用变量警告）
    bool used = false;         // 符号是否被使用
    
    // 默认构造函数
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

// FunctionInfo - 保存函数相关信息的结构体
struct FunctionInfo
{
    std::string returnType;              // 函数返回类型
    std::vector<std::string> paramTypes; // 参数类型列表
    std::vector<std::string> paramNames; // 参数名称列表
    int line;                            // 函数在源代码中的行号
    int column;                          // 函数在源代码中的列号
    bool used = false;                   // 函数是否被调用（用于未使用函数警告）

    FunctionInfo(const std::string &returnType = "void",
                 bool defined = false, int line = 0, int column = 0)
        : returnType(returnType), line(line), column(column), used(false) {}
};
