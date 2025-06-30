// IR.h
#pragma once
#include "parser/ast.h"
#include "parser/astVisitor.h"
#include <vector>
#include <string>
#include <memory>
#include <map>

// 操作数类型
enum class OperandType {
    VARIABLE,    // 变量
    TEMP,        // 临时变量
    CONSTANT,    // 常量
    LABEL        // 标签
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

    std::string toString() const;
    bool isTemp() const { return type == OperandType::TEMP; }
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
    
    FunctionBeginInstr(const std::string& funcName)
        : IRInstr(OpCode::FUNCTION_BEGIN), funcName(funcName) {}
    
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

// IR输出器
class IRPrinter {
public:
    static void print(const std::vector<std::shared_ptr<IRInstr>>& instructions, std::ostream& out);
};