#include "codegen.h"
#include <sstream>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <queue>
#include <stack>

CodeGenerator::CodeGenerator(const std::string& outputFile, 
                           const std::vector<std::shared_ptr<IRInstr>>& instructions,
                           const CodeGenConfig& config)
    : instructions(instructions), config(config) {
    output.open(outputFile);
    if (!output) {
        std::cerr << "错误: 无法打开输出文件: " << outputFile << std::endl;
        exit(1);
    }
    
    // 初始化寄存器信息
    initializeRegisters();
    
    // 输出文件头
    emitComment("由ToyC编译器生成");
    emitComment("RISC-V汇编代码");
    emitSection(".text");
    
    // 如果启用了寄存器分配优化，则执行寄存器分配
    if (config.regAllocStrategy != RegisterAllocStrategy::NAIVE) {
        allocateRegisters();
    }
}

CodeGenerator::~CodeGenerator() {
    if (output.is_open()) {
        output.close();
    }
}

void CodeGenerator::generate() {
    // 处理所有IR指令
    for (const auto& instr : instructions) {
        processInstruction(instr);
    }
    
    // 如果开启了栈布局优化，在生成完代码后进行优化
    if (config.optimizeStackLayout) {
        optimizeStackLayout();
    }
}

std::string CodeGenerator::genLabel() {
    return "L" + std::to_string(labelCount++);
}

void CodeGenerator::emitComment(const std::string& comment) {
    output << "\t# " << comment << "\n";
}

void CodeGenerator::emitInstruction(const std::string& instr) {
    output << "\t" << instr << "\n";
}

void CodeGenerator::emitLabel(const std::string& label) {
    output << label << ":\n";
}

void CodeGenerator::emitGlobal(const std::string& name) {
    output << "\t.global " << name << "\n";
}

void CodeGenerator::emitSection(const std::string& section) {
    output << "\t" << section << "\n";
}

void CodeGenerator::processInstruction(const std::shared_ptr<IRInstr>& instr) {
    // 根据指令类型调用相应的处理函数
    switch (instr->opcode) {
        case OpCode::ADD:
        case OpCode::SUB:
        case OpCode::MUL:
        case OpCode::DIV:
        case OpCode::MOD:
        case OpCode::LT:
        case OpCode::GT:
        case OpCode::LE:
        case OpCode::GE:
        case OpCode::EQ:
        case OpCode::NE:
        case OpCode::AND:
        case OpCode::OR:
            processBinaryOp(std::dynamic_pointer_cast<BinaryOpInstr>(instr));
            break;
            
        case OpCode::NEG:
        case OpCode::NOT:
            processUnaryOp(std::dynamic_pointer_cast<UnaryOpInstr>(instr));
            break;
            
        case OpCode::ASSIGN:
            processAssign(std::dynamic_pointer_cast<AssignInstr>(instr));
            break;
            
        case OpCode::GOTO:
            processGoto(std::dynamic_pointer_cast<GotoInstr>(instr));
            break;
            
        case OpCode::IF_GOTO:
            processIfGoto(std::dynamic_pointer_cast<IfGotoInstr>(instr));
            break;
            
        case OpCode::PARAM:
            processParam(std::dynamic_pointer_cast<ParamInstr>(instr));
            break;
            
        case OpCode::CALL:
            processCall(std::dynamic_pointer_cast<CallInstr>(instr));
            break;
            
        case OpCode::RETURN:
            processReturn(std::dynamic_pointer_cast<ReturnInstr>(instr));
            break;
            
        case OpCode::LABEL:
            processLabel(std::dynamic_pointer_cast<LabelInstr>(instr));
            break;
            
        case OpCode::FUNCTION_BEGIN:
            processFunctionBegin(std::dynamic_pointer_cast<FunctionBeginInstr>(instr));
            break;
            
        case OpCode::FUNCTION_END:
            processFunctionEnd(std::dynamic_pointer_cast<FunctionEndInstr>(instr));
            break;
            
        default:
            std::cerr << "Error: Unknown instruction type" << std::endl;
            break;
    }
}

void CodeGenerator::processBinaryOp(const std::shared_ptr<BinaryOpInstr>& instr) {
    emitComment(instr->toString());
    
    // 获取临时寄存器
    std::string resultReg = allocTempReg();
    std::string leftReg = allocTempReg();
    std::string rightReg = allocTempReg();

    // 加载操作数
    loadOperand(instr->left, leftReg);
    loadOperand(instr->right, rightReg);
    
    
    // 根据操作码生成相应的指令
    switch (instr->opcode) {
        case OpCode::ADD:
            emitInstruction("add " + resultReg + ", " + leftReg + ", " + rightReg);
            break;
        case OpCode::SUB:
            emitInstruction("sub " + resultReg + ", " + leftReg + ", " + rightReg);
            break;
        case OpCode::MUL:
            emitInstruction("mul " + resultReg + ", " + leftReg + ", " + rightReg);
            break;
        case OpCode::DIV:
            emitInstruction("div " + resultReg + ", " + leftReg + ", " + rightReg);
            break;
        case OpCode::MOD:
            emitInstruction("rem " + resultReg + ", " + leftReg + ", " + rightReg);
            break;
        case OpCode::LT:
            emitInstruction("slt " + resultReg + ", " + leftReg + ", " + rightReg);
            break;
        case OpCode::GT:
            emitInstruction("slt " + resultReg + ", " + rightReg + ", " + leftReg);
            break;
        case OpCode::LE:
            emitInstruction("slt " + resultReg + ", " + rightReg + ", " + leftReg);
            emitInstruction("xori " + resultReg + ", " + resultReg + ", 1");
            break;
        case OpCode::GE:
            emitInstruction("slt " + resultReg + ", " + leftReg + ", " + rightReg);
            emitInstruction("xori " + resultReg + ", " + resultReg + ", 1");
            break;
        case OpCode::EQ:
            emitInstruction("xor " + resultReg + ", " + leftReg + ", " + rightReg);
            emitInstruction("seqz " + resultReg + ", " + resultReg);
            break;
        case OpCode::NE:
            emitInstruction("xor " + resultReg + ", " + leftReg + ", " + rightReg);
            emitInstruction("snez " + resultReg + ", " + resultReg);
            break;
        case OpCode::AND:
            emitInstruction("snez " + leftReg + ", " + leftReg);
            emitInstruction("snez " + rightReg + ", " + rightReg);
            emitInstruction("and " + resultReg + ", " + leftReg + ", " + rightReg);
            break;
        case OpCode::OR:
            emitInstruction("or " + resultReg + ", " + leftReg + ", " + rightReg);
            emitInstruction("snez " + resultReg + ", " + resultReg);
            break;
        default:
            std::cerr << "错误: 未知的二元操作" << std::endl;
            break;
    }
    
    // 存储结果
    storeRegister(resultReg, instr->result);
    
    // 释放临时寄存器
    freeTempReg(rightReg);
    freeTempReg(leftReg);
    freeTempReg(resultReg);
}

void CodeGenerator::processUnaryOp(const std::shared_ptr<UnaryOpInstr>& instr) {
    emitComment(instr->toString());
    
    // 获取临时寄存器
    std::string resultReg = allocTempReg();
    std::string operandReg = allocTempReg();

    // 加载操作数
    loadOperand(instr->operand, operandReg);
    
    // 根据操作码生成相应的指令
    switch (instr->opcode) {
        case OpCode::NEG:
            emitInstruction("neg " + resultReg + ", " + operandReg);
            break;
        case OpCode::NOT:
            emitInstruction("seqz " + resultReg + ", " + operandReg);
            break;
        default:
            std::cerr << "错误: 未知的一元操作" << std::endl;
            break;
    }
    
    // 存储结果
    storeRegister(resultReg, instr->result);
    
    // 释放临时寄存器
    freeTempReg(operandReg);
    freeTempReg(resultReg);
}

void CodeGenerator::processAssign(const std::shared_ptr<AssignInstr>& instr) {
    emitComment(instr->toString());
    
    // 获取临时寄存器
    std::string reg = allocTempReg();
    
    // 加载源操作数
    loadOperand(instr->source, reg);
    
    // 存储到目标操作数
    storeRegister(reg, instr->target);
    
    // 释放临时寄存器
    freeTempReg(reg);
}

void CodeGenerator::processGoto(const std::shared_ptr<GotoInstr>& instr) {
    emitComment(instr->toString());
    
    // 直接跳转到目标标签
    emitInstruction("j " + instr->target->name);
}

void CodeGenerator::processIfGoto(const std::shared_ptr<IfGotoInstr>& instr) {
    emitComment(instr->toString());
    
    // 获取临时寄存器
    std::string condReg = allocTempReg();
    
    // 加载条件
    loadOperand(instr->condition, condReg);
    
    // 如果条件为真（非零），则跳转到目标标签
    emitInstruction("bnez " + condReg + ", " + instr->target->name);
    
    // 释放临时寄存器
    freeTempReg(condReg);
}

void CodeGenerator::processParam(const std::shared_ptr<ParamInstr>& instr) {
    static int paramIndex = 0;
    static std::vector<std::shared_ptr<Operand>> params;
    
    emitComment(instr->toString());
    
    // 记录参数
    params.push_back(instr->param);
    paramIndex++;
}

void CodeGenerator::processCall(const std::shared_ptr<CallInstr>& instr) {
    emitComment(instr->toString());
    
    // 获取参数个数
    int paramCount = instr->paramCount;
    
    // 保存调用者保存的寄存器
    saveCallerSavedRegs();
    
    // 处理参数，将参数加载到参数寄存器
    for (int i = 0; i < paramCount && i < 8; i++) {
        std::string argReg = getArgRegister(i);
        
        // 假设参数在栈顶，需要逆序处理
        // 这里我们需要获取到参数，但需要与processParam协同工作
        // 为简单起见，先直接用栈来处理
        emitInstruction("lw " + argReg + ", " + std::to_string(i * 4) + "(sp)");
    }
    
    // 处理超过8个的参数，需要通过栈传递
    // 这部分较复杂，需要更详细的实现
    
    // 调用函数
    emitInstruction("call " + instr->funcName);
    
    // 恢复调用者保存的寄存器
    restoreCallerSavedRegs();
    
    // 如果有返回值，存储a0到结果操作数
    if (instr->result) {
        std::string resultReg = allocTempReg();
        emitInstruction("mv " + resultReg + ", a0");
        storeRegister(resultReg, instr->result);
        freeTempReg(resultReg);
    }
}

void CodeGenerator::processReturn(const std::shared_ptr<ReturnInstr>& instr) {
    emitComment(instr->toString());
    
    // 如果有返回值，加载到a0
    if (instr->value) {
        loadOperand(instr->value, "a0");
    }
    
    // 函数返回，跳转到函数结束处理
    emitInstruction("j " + currentFunction + "_epilogue");
}

void CodeGenerator::processLabel(const std::shared_ptr<LabelInstr>& instr) {
    // 生成标签
    emitLabel(instr->label);
}

void CodeGenerator::processFunctionBegin(const std::shared_ptr<FunctionBeginInstr>& instr) {
    currentFunction = instr->funcName;
    stackSize = 0;
    frameSize = 8; // 初始为ra和fp的保存空间
    localVars.clear();
    frameInitialized = false;
    
    // 生成函数标签
    emitGlobal(instr->funcName);
    emitLabel(instr->funcName);
    
    // 生成函数序言
    emitPrologue(instr->funcName);
}

void CodeGenerator::processFunctionEnd(const std::shared_ptr<FunctionEndInstr>& instr) {
    // 函数结束标签
    emitLabel(currentFunction + "_epilogue");
    
    // 生成函数后记
    emitEpilogue(currentFunction);
    
    // 添加空行以提高可读性
    output << "\n";
}

void CodeGenerator::emitPrologue(const std::string& funcName) {
    emitComment("函数序言");
    
    // 保存返回地址和帧指针
    emitInstruction("addi sp, sp, -" + std::to_string(frameSize));
    emitInstruction("sw ra, " + std::to_string(frameSize - 4) + "(sp)");
    emitInstruction("sw fp, " + std::to_string(frameSize - 8) + "(sp)");
    
    // 设置新的帧指针
    emitInstruction("addi fp, sp, " + std::to_string(frameSize));
    
    // 保存被调用者保存的寄存器
    saveCalleeSavedRegs();
}

void CodeGenerator::emitEpilogue(const std::string& funcName) {
    emitComment("函数后记");
    
    // 恢复被调用者保存的寄存器
    restoreCalleeSavedRegs();
    
    // 恢复帧指针和返回地址
    emitInstruction("lw fp, " + std::to_string(frameSize - 8) + "(sp)");
    emitInstruction("lw ra, " + std::to_string(frameSize - 4) + "(sp)");
    
    // 调整栈指针
    emitInstruction("addi sp, sp, " + std::to_string(frameSize));
    
    // 返回
    emitInstruction("ret");
}

void CodeGenerator::loadOperand(const std::shared_ptr<Operand>& op, const std::string& reg) {
    switch (op->type) {
        case OperandType::CONSTANT:
            emitInstruction("li " + reg + ", " + std::to_string(op->value));
            break;
            
        case OperandType::VARIABLE:
        case OperandType::TEMP:
            {
                // 检查是否已分配到寄存器
                auto it = regAlloc.find(op->name);
                if (it != regAlloc.end() && isValidRegister(it->second)) {
                    // 如果已分配到寄存器，直接移动
                    emitInstruction("mv " + reg + ", " + it->second);
                } else {
                    // 否则从栈中加载
                    int offset = getOperandOffset(op);
                    emitInstruction("lw " + reg + ", " + std::to_string(offset) + "(fp)");
                }
            }
            break;
            
        case OperandType::LABEL:
            // 标签通常不直接加载，而是用于跳转
            std::cerr << "警告: 尝试加载标签操作数" << std::endl;
            break;
            
        default:
            std::cerr << "错误: 未知的操作数类型" << std::endl;
            break;
    }
}

void CodeGenerator::storeRegister(const std::string& reg, const std::shared_ptr<Operand>& op) {
    if (op->type == OperandType::VARIABLE || op->type == OperandType::TEMP) {
        // 检查是否已分配到寄存器
        auto it = regAlloc.find(op->name);
        if (it != regAlloc.end() && isValidRegister(it->second)) {
            // 如果已分配到寄存器，直接移动
            if (reg != it->second) {
                emitInstruction("mv " + it->second + ", " + reg);
            }
        } else {
            // 否则存储到栈中
            int offset = getOperandOffset(op);
            emitInstruction("sw " + reg + ", " + std::to_string(offset) + "(fp)");
        }
    } else {
        std::cerr << "错误: 无法存储到非变量操作数" << std::endl;
    }
}

int CodeGenerator::getOperandOffset(const std::shared_ptr<Operand>& op) {
    if (op->type != OperandType::VARIABLE && op->type != OperandType::TEMP) {
        std::cerr << "错误: 只有变量和临时变量有栈偏移" << std::endl;
        return 0;
    }
    
    auto it = localVars.find(op->name);
    if (it != localVars.end()) {
        return it->second;
    }
    
    // 为变量分配栈空间
    stackSize += 4;
    int offset = -stackSize;
    localVars[op->name] = offset;
    
    // 如果栈帧尚未完全初始化，记录可能的最大栈大小
    if (!frameInitialized) {
        frameSize = 8 + stackSize; // 8字节用于保存ra和fp
    }
    
    return offset;
}

std::string CodeGenerator::allocTempReg() {
    if (nextTempReg >= tempRegs.size()) {
        nextTempReg = 0;
    }
    return tempRegs[nextTempReg++];
}

void CodeGenerator::freeTempReg(const std::string& reg) {
    // 简化实现，不做实际释放，只是记录一下
}

void CodeGenerator::saveCallerSavedRegs() {
    // 保存调用者保存的寄存器
    emitComment("保存调用者保存的寄存器");
    
    // 例如，保存临时寄存器t0-t6和参数寄存器a0-a7
    for (int i = 0; i < 7; i++) {
        emitInstruction("sw t" + std::to_string(i) + ", " + std::to_string(-12 - i * 4) + "(fp)");
    }
}

void CodeGenerator::restoreCallerSavedRegs() {
    // 恢复调用者保存的寄存器
    emitComment("恢复调用者保存的寄存器");
    
    // 例如，恢复临时寄存器t0-t6和参数寄存器a0-a7
    for (int i = 0; i < 7; i++) {
        emitInstruction("lw t" + std::to_string(i) + ", " + std::to_string(-12 - i * 4) + "(fp)");
    }
}

void CodeGenerator::saveCalleeSavedRegs() {
    // 保存被调用者保存的寄存器
    emitComment("保存被调用者保存的寄存器");
    
    // 例如，保存保存寄存器s0-s11
    for (int i = 0; i < 12; i++) {
        emitInstruction("sw s" + std::to_string(i) + ", " + std::to_string(-40 - i * 4) + "(fp)");
    }
}

void CodeGenerator::restoreCalleeSavedRegs() {
    // 恢复被调用者保存的寄存器
    emitComment("恢复被调用者保存的寄存器");
    
    // 例如，恢复保存寄存器s0-s11
    for (int i = 0; i < 12; i++) {
        emitInstruction("lw s" + std::to_string(i) + ", " + std::to_string(-40 - i * 4) + "(fp)");
    }
}

void CodeGenerator::initializeRegisters() {
    // 初始化RISC-V寄存器信息
    registers = {
        // 零寄存器
        {"zero", false, false, false, true, "常量0"},
        
        // 临时寄存器
        {"t0", true, false, true, false, "临时寄存器0"},
        {"t1", true, false, true, false, "临时寄存器1"},
        {"t2", true, false, true, false, "临时寄存器2"},
        {"t3", true, false, true, false, "临时寄存器3"},
        {"t4", true, false, true, false, "临时寄存器4"},
        {"t5", true, false, true, false, "临时寄存器5"},
        {"t6", true, false, true, false, "临时寄存器6"},
        
        // 保存寄存器
        {"s0", false, true, true, false, "保存寄存器0/帧指针"},
        {"s1", false, true, true, false, "保存寄存器1"},
        {"s2", false, true, true, false, "保存寄存器2"},
        {"s3", false, true, true, false, "保存寄存器3"},
        {"s4", false, true, true, false, "保存寄存器4"},
        {"s5", false, true, true, false, "保存寄存器5"},
        {"s6", false, true, true, false, "保存寄存器6"},
        {"s7", false, true, true, false, "保存寄存器7"},
        {"s8", false, true, true, false, "保存寄存器8"},
        {"s9", false, true, true, false, "保存寄存器9"},
        {"s10", false, true, true, false, "保存寄存器10"},
        {"s11", false, true, true, false, "保存寄存器11"},
        
        // 参数/返回值寄存器
        {"a0", true, false, true, false, "参数/返回值寄存器0"},
        {"a1", true, false, true, false, "参数/返回值寄存器1"},
        {"a2", true, false, true, false, "参数/返回值寄存器2"},
        {"a3", true, false, true, false, "参数/返回值寄存器3"},
        {"a4", true, false, true, false, "参数/返回值寄存器4"},
        {"a5", true, false, true, false, "参数/返回值寄存器5"},
        {"a6", true, false, true, false, "参数/返回值寄存器6"},
        {"a7", true, false, true, false, "参数/返回值寄存器7"},
        
        // 特殊寄存器
        {"ra", true, false, false, true, "返回地址"},
        {"sp", false, false, false, true, "栈指针"},
        {"gp", false, false, false, true, "全局指针"},
        {"tp", false, false, false, true, "线程指针"},
        {"fp", false, true, false, true, "帧指针/s0"}
    };
}

bool CodeGenerator::isValidRegister(const std::string& reg) const {
    for (const auto& r : registers) {
        if (r.name == reg) {
            return true;
        }
    }
    return false;
}

std::string CodeGenerator::getArgRegister(int paramIndex) const {
    if (paramIndex >= 0 && paramIndex < argRegs.size()) {
        return argRegs[paramIndex];
    }
    std::cerr << "错误: 参数索引超出范围" << std::endl;
    return "a0"; // 默认返回a0
}

void CodeGenerator::allocateRegisters() {
    // 根据配置选择寄存器分配策略
    switch (config.regAllocStrategy) {
        case RegisterAllocStrategy::LINEAR_SCAN:
            linearScanRegisterAllocation();
            break;
        case RegisterAllocStrategy::GRAPH_COLOR:
            graphColoringRegisterAllocation();
            break;
        default:
            // 默认不做任何分配
            break;
    }
}

void CodeGenerator::linearScanRegisterAllocation() {
    // 创建寄存器分配器
    LinearScanRegisterAllocator allocator;
    
    // 获取可分配的寄存器
    std::vector<Register> allocatableRegs;
    for (const auto& reg : registers) {
        if (reg.isAllocatable && !reg.isReserved) {
            allocatableRegs.push_back(reg);
        }
    }
    
    // 执行分配
    regAlloc = allocator.allocate(instructions, allocatableRegs);
}

void CodeGenerator::graphColoringRegisterAllocation() {
    // 创建寄存器分配器
    GraphColoringRegisterAllocator allocator;
    
    // 获取可分配的寄存器
    std::vector<Register> allocatableRegs;
    for (const auto& reg : registers) {
        if (reg.isAllocatable && !reg.isReserved) {
            allocatableRegs.push_back(reg);
        }
    }
    
    // 执行分配
    regAlloc = allocator.allocate(instructions, allocatableRegs);
}

void CodeGenerator::optimizeStackLayout() {
    // 分析变量的生命周期
    analyzeVariableLifetimes();
    
    // 根据变量的生命周期重新分配栈空间
    // 这部分较复杂，需要更详细的实现
}

void CodeGenerator::analyzeVariableLifetimes() {
    // 分析变量的定义和使用位置
    // 这部分较复杂，需要更详细的实现
}

std::map<std::string, std::set<std::string>> CodeGenerator::buildInterferenceGraph() {
    std::map<std::string, std::set<std::string>> interferenceGraph;
    
    // 构建变量之间的冲突图
    // 这部分较复杂，需要更详细的实现
    
    return interferenceGraph;
}

void CodeGenerator::addPeepholePattern(const std::string& pattern, 
                                     std::function<bool(std::vector<std::string>&)> handler) {
    peepholePatterns[pattern] = handler;
}

void CodeGenerator::peepholeOptimize(std::vector<std::string>& instructions) {
    // 窥孔优化
    // 这部分较复杂，需要更详细的实现
}

// 简单寄存器分配器实现
std::map<std::string, std::string> NaiveRegisterAllocator::allocate(
    const std::vector<std::shared_ptr<IRInstr>>& instructions,
    const std::vector<Register>& availableRegs) {
    std::map<std::string, std::string> allocation;
    
    // 简单地将变量分配到寄存器
    // 这只是一个非常基础的实现
    
    return allocation;
}

// 线性扫描寄存器分配器实现
std::map<std::string, std::string> LinearScanRegisterAllocator::allocate(
    const std::vector<std::shared_ptr<IRInstr>>& instructions,
    const std::vector<Register>& availableRegs) {
    std::map<std::string, std::string> allocation;
    
    // 计算变量的生命周期
    std::vector<LiveInterval> intervals = computeLiveIntervals(instructions);
    
    // 按开始位置排序
    std::sort(intervals.begin(), intervals.end());
    
    // 实现线性扫描算法
    // 这部分较复杂，需要更详细的实现
    
    return allocation;
}

std::vector<LinearScanRegisterAllocator::LiveInterval> LinearScanRegisterAllocator::computeLiveIntervals(
    const std::vector<std::shared_ptr<IRInstr>>& instructions) {
    std::vector<LiveInterval> intervals;
    
    // 计算每个变量的生命周期
    // 这部分较复杂，需要更详细的实现
    
    return intervals;
}

// 图着色寄存器分配器实现
std::map<std::string, std::string> GraphColoringRegisterAllocator::allocate(
    const std::vector<std::shared_ptr<IRInstr>>& instructions,
    const std::vector<Register>& availableRegs) {
    std::map<std::string, std::string> allocation;
    
    // 构建冲突图
    auto interferenceGraph = buildInterferenceGraph(instructions);
    
    // 简化冲突图
    auto simplifiedOrder = simplify(interferenceGraph);
    
    // 为变量分配寄存器
    allocation = color(simplifiedOrder, interferenceGraph, availableRegs);
    
    return allocation;
}

std::map<std::string, std::set<std::string>> GraphColoringRegisterAllocator::buildInterferenceGraph(
    const std::vector<std::shared_ptr<IRInstr>>& instructions) {
    std::map<std::string, std::set<std::string>> graph;
    
    // 构建变量之间的冲突图
    // 这部分较复杂，需要更详细的实现
    
    return graph;
}

std::vector<std::string> GraphColoringRegisterAllocator::simplify(
    std::map<std::string, std::set<std::string>>& graph) {
    std::vector<std::string> order;
    
    // 简化冲突图
    // 这部分较复杂，需要更详细的实现
    
    return order;
}

std::map<std::string, std::string> GraphColoringRegisterAllocator::color(
    const std::vector<std::string>& simplifiedOrder,
    const std::map<std::string, std::set<std::string>>& originalGraph,
    const std::vector<Register>& availableRegs) {
    std::map<std::string, std::string> allocation;
    
    // 为变量分配寄存器
    // 这部分较复杂，需要更详细的实现
    
    return allocation;
}