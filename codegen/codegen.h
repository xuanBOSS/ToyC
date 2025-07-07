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
    void processInstructionToStream(const std::shared_ptr<IRInstr>& instr, std::ostream& stream);

    // 添加优化规则
    void addPeepholePattern(const std::string& pattern, 
                           std::function<bool(std::vector<std::string>&)> handler);

private:
     // 生成标签
    std::string genLabel();
    
    // 输出函数
    void emitComment(const std::string& comment);
    void emitInstruction(const std::string& instr);
    void emitLabel(const std::string& label);
    void emitGlobal(const std::string& name);
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
    
    // 操作数和寄存器处理
    void loadOperand(const std::shared_ptr<Operand>& op, const std::string& reg);
    void storeRegister(const std::string& reg, const std::shared_ptr<Operand>& op);
    int getOperandOffset(const std::shared_ptr<Operand>& op);
    std::string allocTempReg();
    void freeTempReg(const std::string& reg);
    
    // 寄存器保存和恢复
    void saveCallerSavedRegs();
    void restoreCallerSavedRegs();
    void saveCalleeSavedRegs();
    void restoreCalleeSavedRegs();
    
    // 寄存器管理
    void initializeRegisters();
    void allocateRegisters();
    bool isValidRegister(const std::string& reg) const;
    std::string getArgRegister(int paramIndex) const;
    
    // 函数管理
    void emitPrologue(const std::string& funcName);
    void emitEpilogue(const std::string& funcName);
    
    // 优化方法
    void optimizeStackLayout();
    void peepholeOptimize(std::vector<std::string>& instructions);
    void linearScanRegisterAllocation();
    void graphColoringRegisterAllocation();
    
    // 分析方法
    void analyzeVariableLifetimes(std::map<std::string, std::pair<int, int>>& varLifetimes);
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
