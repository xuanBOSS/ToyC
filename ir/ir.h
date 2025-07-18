// ir.h - 定义中间表示(IR)的数据结构和接口
#pragma once
#include "parser/ast.h"
#include "parser/astVisitor.h"
#include <vector>
#include <string>
#include <memory>
#include <map>

// OperandType - 操作数类型枚举
enum class OperandType {
    VARIABLE,    // 变量操作数（命名变量）
    TEMP,        // 临时变量操作数（编译器生成的临时变量）
    CONSTANT,    // 常量操作数（字面值）
    LABEL        // 标签操作数（用于控制流）
};

// 操作数
class Operand {
public:
    OperandType type;
    std::string name;  // 变量名或标签名
    int value;         // 常量值

    // 构造函数
    Operand(OperandType type, const std::string& name) : type(type), name(name), value(0) {}
    Operand(int value) : type(OperandType::CONSTANT), value(value) {}

    std::string toString() const;// 将操作数转换为字符串表示
    bool isTemp() const { return type == OperandType::TEMP; } // 检查操作数是否为临时变量
};

// 指令操作码
enum class OpCode {
    ADD, SUB, MUL, DIV, MOD,   // 算术运算
    NEG, NOT,                  // 一元运算
    LT, GT, LE, GE, EQ, NE,    // 比较运算
    AND, OR,                   // 逻辑运算
    ASSIGN,                    // 赋值
    GOTO, IF_GOTO,             // 控制流
    PARAM, CALL, RETURN,       // 函数调用
    LABEL,                     // 标签定义
    FUNCTION_BEGIN, FUNCTION_END // 函数开始和结束
};

// IR指令基类
class IRInstr {
public:
    OpCode opcode;
    
    IRInstr(OpCode opcode) : opcode(opcode) {}
    virtual ~IRInstr() {}
    // 将指令转换为字符串表示（纯虚函数，由子类实现）
    virtual std::string toString() const = 0;
};

// 二元运算指令
class BinaryOpInstr : public IRInstr {
public:
    std::shared_ptr<Operand> result;
    std::shared_ptr<Operand> left;
    std::shared_ptr<Operand> right;
    
    BinaryOpInstr(OpCode opcode, 
                 std::shared_ptr<Operand> result,
                 std::shared_ptr<Operand> left,
                 std::shared_ptr<Operand> right)
        : IRInstr(opcode), result(result), left(left), right(right) {}
    
    std::string toString() const override;
};

// 一元运算指令
class UnaryOpInstr : public IRInstr {
public:
    std::shared_ptr<Operand> result;
    std::shared_ptr<Operand> operand;
    
    UnaryOpInstr(OpCode opcode,
                std::shared_ptr<Operand> result,
                std::shared_ptr<Operand> operand)
        : IRInstr(opcode), result(result), operand(operand) {}
    
    std::string toString() const override;
};

// 赋值指令
class AssignInstr : public IRInstr {
public:
    std::shared_ptr<Operand> target;
    std::shared_ptr<Operand> source;
    
    AssignInstr(std::shared_ptr<Operand> target,
               std::shared_ptr<Operand> source)
        : IRInstr(OpCode::ASSIGN), target(target), source(source) {}
    
    std::string toString() const override;
};

// 跳转指令
class GotoInstr : public IRInstr {
public:
    std::shared_ptr<Operand> target; // 标签
    
    GotoInstr(std::shared_ptr<Operand> target)
        : IRInstr(OpCode::GOTO), target(target) {}
    
    std::string toString() const override;
};

// 条件跳转指令
class IfGotoInstr : public IRInstr {
public:
    std::shared_ptr<Operand> condition;
    std::shared_ptr<Operand> target; // 标签
    
    IfGotoInstr(std::shared_ptr<Operand> condition,
               std::shared_ptr<Operand> target)
        : IRInstr(OpCode::IF_GOTO), condition(condition), target(target) {}
    
    std::string toString() const override;
};

// 函数参数指令
class ParamInstr : public IRInstr {
public:
    std::shared_ptr<Operand> param;
    
    ParamInstr(std::shared_ptr<Operand> param)
        : IRInstr(OpCode::PARAM), param(param) {}
    
    std::string toString() const override;
};

// 函数调用指令
class CallInstr : public IRInstr {
public:
    std::shared_ptr<Operand> result; // 可能为nullptr表示无返回值
    std::string funcName;
    int paramCount;
    
    std::vector<std::shared_ptr<Operand>> params; // 新增：存储参数列表，便于代码生成

    CallInstr(std::shared_ptr<Operand> result,
             const std::string& funcName,
             int paramCount)
        : IRInstr(OpCode::CALL), result(result), funcName(funcName), paramCount(paramCount) {}
    
    std::string toString() const override;
};

// 返回指令
class ReturnInstr : public IRInstr {
public:
    std::shared_ptr<Operand> value; // 可能为nullptr表示无返回值
    
    ReturnInstr(std::shared_ptr<Operand> value = nullptr)
        : IRInstr(OpCode::RETURN), value(value) {}
    
    std::string toString() const override;
};

// 标签指令
class LabelInstr : public IRInstr {
public:
    std::string label;
    
    LabelInstr(const std::string& label)
        : IRInstr(OpCode::LABEL), label(label) {}
    
    std::string toString() const override;
};

// 函数开始指令
class FunctionBeginInstr : public IRInstr {
public:
    std::string funcName;
    std::vector<std::string> paramNames; //参数名列表
    std::string returnType;  // 新增：函数返回类型
    
    FunctionBeginInstr(const std::string& funcName, const std::string& returnType = "int")
        : IRInstr(OpCode::FUNCTION_BEGIN), funcName(funcName), returnType(returnType) {}
    
    std::string toString() const override;
};

// 函数结束指令
class FunctionEndInstr : public IRInstr {
public:
    std::string funcName;
    
    FunctionEndInstr(const std::string& funcName)
        : IRInstr(OpCode::FUNCTION_END), funcName(funcName) {}
    
    std::string toString() const override;
};

// IRPrinter - IR输出器，用于将IR指令序列输出为文本
class IRPrinter {
public:
    // 将IR指令序列输出到指定流
    static void print(const std::vector<std::shared_ptr<IRInstr>>& instructions, std::ostream& out);
};

// IRAnalyzer - IR分析器，提供IR指令分析工具
class IRAnalyzer {
public:
    // 查找定义特定操作数的指令
    static int findDefinition(const std::vector<std::shared_ptr<IRInstr>>& instructions, 
                             const std::string& operandName);
                             
    // 查找使用特定操作数的指令
    static std::vector<int> findUses(const std::vector<std::shared_ptr<IRInstr>>& instructions, 
                                   const std::string& operandName);
                                   
    // 检查变量是否活跃
    static bool isVariableLive(const std::vector<std::shared_ptr<IRInstr>>& instructions,
                              const std::string& varName,
                              int position);
                              
    // 获取指令定义的变量
    static std::vector<std::string> getDefinedVariables(const std::shared_ptr<IRInstr>& instr);
    
    // 获取指令使用的变量
    static std::vector<std::string> getUsedVariables(const std::shared_ptr<IRInstr>& instr);

    //用于检查函数是否被使用
    static bool isFunctionUsed(const std::vector<std::shared_ptr<IRInstr>>& instructions,
                          const std::string& funcName);
};
