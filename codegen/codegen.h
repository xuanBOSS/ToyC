// CodeGenerator.h
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
enum class RegisterAllocStrategy {
    NAIVE,        // 简单分配，不考虑寄存器冲突
    LINEAR_SCAN,  // 线性扫描寄存器分配算法
    GRAPH_COLOR   // 图着色寄存器分配算法
};

// 代码生成配置选项
struct CodeGenConfig {
    bool optimizeStackLayout = false;     // 优化栈布局
    bool eliminateDeadStores = false;     // 消除无用存储
    bool enablePeepholeOptimizations = false; // 启用窥孔优化
    bool enableInlineAsm = false;         // 启用内联汇编
    RegisterAllocStrategy regAllocStrategy = RegisterAllocStrategy::NAIVE; // 寄存器分配策略
};

// 寄存器信息
struct Register {
    std::string name;        // 寄存器名称
    bool isCallerSaved;      // 是否为调用者保存
    bool isCalleeSaved;      // 是否为被调用者保存
    bool isAllocatable;      // 是否可分配
    bool isReserved;         // 是否保留
    std::string purpose;     // 寄存器用途
};

// 代码生成器类
class CodeGenerator {
private:
    std::ofstream output;
    std::map<std::string, int> localVars;       // 局部变量到栈偏移的映射
    std::map<std::string, std::string> regAlloc; // 变量到寄存器的分配
    std::set<std::string> activeVars;           // 当前活跃的变量
    int labelCount = 0;
    int stackSize = 0;
    int frameSize = 0;
    bool frameInitialized = false;
    
    // 当前函数的信息
    std::string currentFunction;
    bool isInLoop = false;
    std::vector<std::string> breakLabels;
    std::vector<std::string> continueLabels;
    
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
    std::map<std::string, std::function<bool(std::vector<std::string>&)>> peepholePatterns;

public:
    CodeGenerator(const std::string& outputFile, 
                 const std::vector<std::shared_ptr<IRInstr>>& instructions,
                 const CodeGenConfig& config = CodeGenConfig());
    ~CodeGenerator();
    
    // 生成代码
    void generate();
    
    // 添加优化规则
    void addPeepholePattern(const std::string& pattern, 
                           std::function<bool(std::vector<std::string>&)> handler);

private:
     // 生成标签
    std::string genLabel();
    
    // 输出注释
    void emitComment(const std::string& comment);
    
    // 输出指令
    void emitInstruction(const std::string& instr);
    
    // 输出标签
    void emitLabel(const std::string& label);
    
    // 输出全局声明
    void emitGlobal(const std::string& name);
    
    // 输出节区
    void emitSection(const std::string& section);
    
    // 处理IR指令
    void processInstruction(const std::shared_ptr<IRInstr>& instr);
    
    // 处理各种IR指令类型
    void processBinaryOp(const std::shared_ptr<BinaryOpInstr>& instr);
    void processUnaryOp(const std::shared_ptr<UnaryOpInstr>& instr);
    void processAssign(const std::shared_ptr<AssignInstr>& instr);
    void processGoto(const std::shared_ptr<GotoInstr>& instr);
    void processIfGoto(const std::shared_ptr<IfGotoInstr>& instr);
    void processParam(const std::shared_ptr<ParamInstr>& instr);
    void processCall(const std::shared_ptr<CallInstr>& instr);
    void processReturn(const std::shared_ptr<ReturnInstr>& instr);
    void processLabel(const std::shared_ptr<LabelInstr>& instr);
    void processFunctionBegin(const std::shared_ptr<FunctionBeginInstr>& instr);
    void processFunctionEnd(const std::shared_ptr<FunctionEndInstr>& instr);
    
    // 加载操作数到寄存器
    void loadOperand(const std::shared_ptr<Operand>& op, const std::string& reg);
    
    // 存储寄存器到操作数
    void storeRegister(const std::string& reg, const std::shared_ptr<Operand>& op);
    
    // 获取操作数的存储位置
    int getOperandOffset(const std::shared_ptr<Operand>& op);

    // 分配一个临时寄存器
    std::string allocTempReg();
    
    // 释放临时寄存器
    void freeTempReg(const std::string& reg);
    
    // 保存所有调用者保存的寄存器
    void saveCallerSavedRegs();
    
    // 恢复所有调用者保存的寄存器
    void restoreCallerSavedRegs();
    
    // 保存所有被调用者保存的寄存器
    void saveCalleeSavedRegs();
    
    // 恢复所有被调用者保存的寄存器
    void restoreCalleeSavedRegs();
    
    // 初始化寄存器信息
    void initializeRegisters();
    
    // 执行寄存器分配
    void allocateRegisters();
    
    // 生成函数序言
    void emitPrologue(const std::string& funcName);
    
    // 生成函数后记
    void emitEpilogue(const std::string& funcName);
    
    // 优化栈布局
    void optimizeStackLayout();
    
    // 窥孔优化
    void peepholeOptimize(std::vector<std::string>& instructions);
    
    // 线性扫描寄存器分配
    void linearScanRegisterAllocation();
    
    // 图着色寄存器分配
    void graphColoringRegisterAllocation();
    
    // 检查是否是有效的RISC-V寄存器名
    bool isValidRegister(const std::string& reg) const;
    
    // 获取参数寄存器
    std::string getArgRegister(int paramIndex) const;
    
    // 分析变量的生命周期
    void analyzeVariableLifetimes();
    
    // 创建冲突图
    std::map<std::string, std::set<std::string>> buildInterferenceGraph();
};

// 寄存器分配器基类
class RegisterAllocator {
public:
    virtual ~RegisterAllocator() = default;
    virtual std::map<std::string, std::string> allocate(
        const std::vector<std::shared_ptr<IRInstr>>& instructions,
        const std::vector<Register>& availableRegs) = 0;
};

// 简单寄存器分配器
class NaiveRegisterAllocator : public RegisterAllocator {
public:
    std::map<std::string, std::string> allocate(
        const std::vector<std::shared_ptr<IRInstr>>& instructions,
        const std::vector<Register>& availableRegs) override;
};

// 线性扫描寄存器分配器
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