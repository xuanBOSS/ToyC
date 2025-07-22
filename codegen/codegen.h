// codegen.h - 定义代码生成器接口和实现
// 本文件包含将中间表示(IR)转换为RISC-V汇编代码的类和函数
#pragma once
#include "parser/astVisitor.h"
#include "ir/ir.h"
#include <vector>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <memory>
#include <functional>

// 寄存器分配策略
// 定义不同的寄存器分配算法选项
enum class RegisterAllocStrategy {
    NAIVE,        // 简单分配，不考虑寄存器冲突
    LINEAR_SCAN,  // 线性扫描寄存器分配算法
    GRAPH_COLOR   // 图着色寄存器分配算法
};

// 代码生成配置选项结构体
// 控制代码生成过程中的各种优化和行为
struct CodeGenConfig {
    bool optimizeStackLayout = false;     // 优化栈布局
    bool eliminateDeadStores = false;     // 消除无用存储
    bool enablePeepholeOptimizations = false; // 启用窥孔优化
    bool enableInlineAsm = false;         // 启用内联汇编
    RegisterAllocStrategy regAllocStrategy = RegisterAllocStrategy::NAIVE; // 寄存器分配策略
};

// 寄存器信息结构体
// 描述目标架构中每个寄存器的属性
struct Register {
    std::string name;        // 寄存器名称
    bool isCallerSaved;      // 是否为调用者保存
    bool isCalleeSaved;      // 是否为被调用者保存
    bool isAllocatable;      // 是否可分配
    bool isReserved;         // 是否保留
    std::string purpose;     // 寄存器用途

    bool isUsed;             // 是否已被使用
};

// 代码生成器类
// 负责将IR指令转换为目标架构的汇编代码
class CodeGenerator {
private:
    std::ostream& output;                      // 输出文件流
    std::map<std::string, int> localVars;      // 局部变量到栈偏移的映射
    std::map<std::string, std::string> regAlloc; // 变量到寄存器的分配映射
    std::set<std::string> activeVars;          // 当前活跃的变量集合
    int labelCount = 0;                        // 标签计数器
    int stackSize = 0;                         // 当前函数栈帧大小
    int frameSize = 0;                         // 当前函数帧大小
    int localVarsSize = 0;                     // 局部变量占用空间
    int calleeRegsSize = 0;                    // 被调用者寄存器保存区大小
    int callerRegsSize = 0;                    // 调用者寄存器保存区大小
    int paramStackSize = 0;                    // 栈传递参数区大小
    bool frameInitialized = false;             // 栈帧是否已初始化
    std::set<std::string> usedCalleeSavedRegs;  // 实际使用的被调用者保存寄存器
    std::set<std::string> usedCallerSavedRegs;  // 实际使用的调用者保存寄存器
    std::map<std::string, int> regOffsetMap;  // 记录寄存器到栈偏移量的映射
    int currentStackOffset = 0;                   // 当前栈顶偏移
    
    // 当前函数的上下文信息
    std::string currentFunction;               // 当前处理的函数名
    bool isInLoop = false;                     // 是否在循环内
    std::vector<std::string> breakLabels;      // break语句跳转目标标签栈
    std::vector<std::string> continueLabels;   // continue语句跳转目标标签栈
    std::vector<std::string> currentFunctionParams; // 当前函数的参数名列表
    std::string currentFunctionReturnType;     // 当前函数的返回类型

    // 参数处理
    std::vector<std::shared_ptr<Operand>> paramQueue; // 函数调用参数队列

    // IR指令列表
    const std::vector<std::shared_ptr<IRInstr>>& instructions;
    
    // 配置选项
    CodeGenConfig config;
    
    // 寄存器信息
    std::vector<Register> registers;
    
    // 临时寄存器分配信息
    int nextTempReg = 0;
    std::vector<std::string> tempRegs = {"t0", "t1", "t2", "t3", "t4", "t5", "t6"};
    
    // 函数调用约定
    std::vector<std::string> argRegs = {"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
    
    // 指令优化表 - 用于窥孔优化
    // 键是模式名称，值是处理函数，接收指令序列并返回是否应用了优化
    std::map<std::string, std::function<bool(std::vector<std::string>&)>> peepholePatterns;

public:
    // 构造函数
    // 参数:
    //   outputFile - 输出汇编文件路径
    //   instructions - IR指令序列
    //   config - 代码生成配置
    CodeGenerator(std::ostream& outputStream,  
                 const std::vector<std::shared_ptr<IRInstr>>& instructions,
                 const CodeGenConfig& config = CodeGenConfig());
    ~CodeGenerator();
    
    // 生成代码
    // 处理所有IR指令并生成目标汇编代码
    void generate();

    // 将单条IR指令处理结果写入指定流
    // 参数:
    //   instr - IR指令
    //   stream - 输出流
    void processInstructionToStream(const std::shared_ptr<IRInstr>& instr, std::ostream& stream);

    // 添加窥孔优化规则
    // 参数:
    //   pattern - 优化模式名称
    //   handler - 处理函数，接收指令序列并返回是否应用了优化   
    void addPeepholePattern(const std::string& pattern, 
                           std::function<bool(std::vector<std::string>&)> handler);

private:
     // 生成标签
    std::string genLabel();
    
    // 输出函数 - 生成汇编代码的各种元素
    void emitComment(const std::string& comment);    // 输出注释
    void emitInstruction(const std::string& instr);  // 输出指令
    void emitLabel(const std::string& label);        // 输出标签
    void emitGlobal(const std::string& name);        // 输出全局标识符声明
    void emitSection(const std::string& section);    // 输出段声明
    
    // 处理IR指令
    void processInstruction(const std::shared_ptr<IRInstr>& instr);
    
    // 处理各种IR指令类型
    // 处理各种IR指令类型
    void processBinaryOp(const std::shared_ptr<BinaryOpInstr>& instr);    // 处理二元操作
    void processUnaryOp(const std::shared_ptr<UnaryOpInstr>& instr);      // 处理一元操作
    void processAssign(const std::shared_ptr<AssignInstr>& instr);        // 处理赋值
    void processGoto(const std::shared_ptr<GotoInstr>& instr);            // 处理无条件跳转
    void processIfGoto(const std::shared_ptr<IfGotoInstr>& instr);        // 处理条件跳转
    void processParam(const std::shared_ptr<ParamInstr>& instr);          // 处理参数
    void processCall(const std::shared_ptr<CallInstr>& instr);            // 处理函数调用
    void processReturn(const std::shared_ptr<ReturnInstr>& instr);        // 处理返回
    void processLabel(const std::shared_ptr<LabelInstr>& instr);          // 处理标签
    void processFunctionBegin(const std::shared_ptr<FunctionBeginInstr>& instr); // 处理函数开始
    void processFunctionEnd(const std::shared_ptr<FunctionEndInstr>& instr);     // 处理函数结束
    
    // 操作数和寄存器处理
    void loadOperand(const std::shared_ptr<Operand>& op, const std::string& reg); // 加载操作数到寄存器
    void storeRegister(const std::string& reg, const std::shared_ptr<Operand>& op); // 存储寄存器值到操作数
    int getOperandOffset(const std::shared_ptr<Operand>& op);  // 获取操作数在栈中的偏移
    std::string allocTempReg();                                // 分配临时寄存器
    void freeTempReg(const std::string& reg);                  // 释放临时寄存器
    
    // 寄存器保存和恢复
    void saveCallerSavedRegs();      // 保存调用者保存的寄存器
    void restoreCallerSavedRegs();   // 恢复调用者保存的寄存器
    void saveCalleeSavedRegs();      // 保存被调用者保存的寄存器
    void restoreCalleeSavedRegs();   // 恢复被调用者保存的寄存器
    
    // 寄存器管理
    void initializeRegisters();      // 初始化寄存器信息
    void resetStackOffset();         // 初始化栈顶偏移
    void allocateRegisters();        // 根据策略分配寄存器
    bool isValidRegister(const std::string& reg) const;  // 检查寄存器名是否有效
    std::string getArgRegister(int paramIndex) const;    // 获取参数寄存器名
    void analyzeUsedCalleeSavedRegs();                   //分析使用的被调用者保存寄存器
    void analyzeUsedCallerSavedRegs();                   //分析使用的调用者保存寄存器
    int countUsedCallerSavedRegs();                      // 计算使用的调用者保存寄存器数量
    int countUsedCalleeSavedRegs();                      // 计算使用的被调用者保存寄存器数量
    int getRegisterStackOffset(const std::string& reg);  //返回被调用者保存寄存器在当前函数栈帧中的偏移地址
    int getCallerSavedRegsSize() const {                // 获取调用者寄存器保存区大小
        // 每个寄存器占4字节
        return usedCallerSavedRegs.size() * 4;  
    }
    int getCalleeSavedRegsSize() const {                // 获取被调用者保存寄存器占用的栈空间大小
        return usedCalleeSavedRegs.size() * 4;
    }
    int getTotalFrameSize() const {
        int total = getCalleeSavedRegsSize() + localVarsSize + 8; // +8 for ra/fp
        return (total + 15) & ~15; // 16-byte aligned
    }
    int getLocalVarsSize() const {                      // 获取局部变量区大小
        return localVarsSize; 
    }
    // 增加局部变量区大小
    void incrementLocalVarsSize(int size) {             // 增加局部变量区大小
        localVarsSize += size;
    }
    
    // 函数管理
    void emitPrologue(const std::string& funcName);  // 生成函数序言
    void emitEpilogue(const std::string& funcName);  // 生成函数后记
    
    // 优化方法
    void optimizeStackLayout();                      // 优化栈布局
    void peepholeOptimize(std::vector<std::string>& instructions);  // 窥孔优化
    void linearScanRegisterAllocation();             // 线性扫描寄存器分配
    void graphColoringRegisterAllocation();          // 图着色寄存器分配
    
    // 分析方法
    // 分析变量生命周期，结果存储在varLifetimes中
    void analyzeVariableLifetimes(std::map<std::string, std::pair<int, int>>& varLifetimes);

    int analyzeTempVars();      //遍历指令序列，记录临时变量的生命周期和冲突关系

    // 判断是否为临时寄存器（如虚拟寄存器或物理临时寄存器t0-t6）
    bool isTempReg(const std::string& reg) {
        return reg[0] == 't';
        //return reg[0] == 't' || isVirtualReg(reg);  // 当前代码未使用虚拟寄存器
    }
    // 检查寄存器是否已分配到物理寄存器（无需栈空间）
    bool isRegisterAllocated(const std::string& reg) {
        return regAlloc.find(reg) != regAlloc.end();
    }

    // 构建变量冲突图
    std::map<std::string, std::set<std::string>> buildInterferenceGraph();
};

// 寄存器分配器基类
// 定义寄存器分配器的通用接口
class RegisterAllocator {
public:
    virtual ~RegisterAllocator() = default;
    // 分配寄存器
    // 参数:
    //   instructions - IR指令序列
    //   availableRegs - 可用寄存器列表
    // 返回:
    //   变量到寄存器的映射
    virtual std::map<std::string, std::string> allocate(
        const std::vector<std::shared_ptr<IRInstr>>& instructions,
        const std::vector<Register>& availableRegs) = 0;
};

// 简单寄存器分配器
// 实现最基本的寄存器分配策略
class NaiveRegisterAllocator : public RegisterAllocator {
public:
    std::map<std::string, std::string> allocate(
        const std::vector<std::shared_ptr<IRInstr>>& instructions,
        const std::vector<Register>& availableRegs) override;
};

// 线性扫描寄存器分配器
// 基于变量生命周期的线性扫描算法
class LinearScanRegisterAllocator : public RegisterAllocator {
public:
    std::map<std::string, std::string> allocate(
        const std::vector<std::shared_ptr<IRInstr>>& instructions,
        const std::vector<Register>& availableRegs) override;
    
private:
    struct LiveInterval {
        std::string var;
        int start;
        int end;
        
        bool operator<(const LiveInterval& other) const {
            return start < other.start;
        }
    };
    
    std::vector<LiveInterval> computeLiveIntervals(
        const std::vector<std::shared_ptr<IRInstr>>& instructions);
};

// 图着色寄存器分配器
// 基于变量冲突图的图着色算法
class GraphColoringRegisterAllocator : public RegisterAllocator {
public:
    std::map<std::string, std::string> allocate(
        const std::vector<std::shared_ptr<IRInstr>>& instructions,
        const std::vector<Register>& availableRegs) override;
    
private:
    std::map<std::string, std::set<std::string>> buildInterferenceGraph(
        const std::vector<std::shared_ptr<IRInstr>>& instructions);
    
    std::vector<std::string> simplify(
        std::map<std::string, std::set<std::string>>& graph);
    
    std::map<std::string, std::string> color(
        const std::vector<std::string>& simplifiedOrder,
        const std::map<std::string, std::set<std::string>>& originalGraph,
        const std::vector<Register>& availableRegs);
};
