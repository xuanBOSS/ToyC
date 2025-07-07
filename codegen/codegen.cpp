#include "codegen.h"
#include <sstream>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <queue>
#include <stack>
#include <limits>

// 代码生成器构造函数，初始化输出文件和配置
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

// 析构函数，关闭输出文件
CodeGenerator::~CodeGenerator() {
    if (output.is_open()) {
        output.close();
    }
}

// 生成汇编代码的主要方法
void CodeGenerator::generate() {
    // 创建一个临时存储生成的汇编指令的向量
    std::vector<std::string> asmInstructions;
    
    // 处理所有IR指令
    for (const auto& instr : instructions) {
        // 创建一个临时字符串流
        std::stringstream tempOutput;
        
        // 记录当前输出流位置
        std::streampos originalPos = output.tellp();
        
        // 处理指令到临时字符串流
        processInstructionToStream(instr, tempOutput);
        
        // 获取生成的汇编代码
        std::string asmCode = tempOutput.str();
        
        // 拆分成单独的指令
        std::istringstream iss(asmCode);
        std::string line;
        while (std::getline(iss, line)) {
            if (!line.empty()) {
                // 判断是否是指令（不是注释或标签）
                if (line[0] != '#' && line[0] != '\t' && line.back() != ':') {
                    // 这是一条汇编指令，添加到列表
                    asmInstructions.push_back(line);
                } else {
                    // 直接写入注释和标签
                    output << line << "\n";
                }
            }
        }
    }
    
    // 如果启用了窥孔优化
    if (config.enablePeepholeOptimizations) {
        peepholeOptimize(asmInstructions);
    }
    
    // 如果启用了栈布局优化
    if (config.optimizeStackLayout) {
        optimizeStackLayout();
    }
    
    // 输出优化后的指令
    for (const auto& instr : asmInstructions) {
        output << "\t" << instr << "\n";
    }
}

//将指令处理结果写入指定流
void CodeGenerator::processInstructionToStream(const std::shared_ptr<IRInstr>& instr, std::ostream& stream) {
    // 保存原始处理方法的输出
    std::streambuf* originalBuf = output.std::ostream::rdbuf();
    
    // 临时将输出重定向到提供的流
    output.std::ostream::rdbuf(stream.rdbuf());
    
    // 处理指令
    processInstruction(instr);
    
    // 恢复原始输出
    output.std::ostream::rdbuf(originalBuf);
}

// 生成唯一的标签名
std::string CodeGenerator::genLabel() {
    return "L" + std::to_string(labelCount++);
}

// 输出注释
void CodeGenerator::emitComment(const std::string& comment) {
    output << "\t# " << comment << "\n";
}

// 输出指令
void CodeGenerator::emitInstruction(const std::string& instr) {
    output << "\t" << instr << "\n";
}

// 输出标签
void CodeGenerator::emitLabel(const std::string& label) {
    output << label << ":\n";
}

// 输出全局标识符
void CodeGenerator::emitGlobal(const std::string& name) {
    output << "\t.global " << name << "\n";
}

// 输出段定义
void CodeGenerator::emitSection(const std::string& section) {
    output << "\t" << section << "\n";
}

// 处理IR指令，根据操作码调用相应的处理函数
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

// 处理二元操作指令
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

// 处理一元操作指令
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

// 处理赋值指令
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

// 处理无条件跳转指令
void CodeGenerator::processGoto(const std::shared_ptr<GotoInstr>& instr) {
    emitComment(instr->toString());
    
    // 直接跳转到目标标签
    emitInstruction("j " + instr->target->name);
}

// 处理条件跳转指令
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

// 参数传递队列（全局）
static std::vector<std::shared_ptr<Operand>> paramQueue;

// 处理参数指令
void CodeGenerator::processParam(const std::shared_ptr<ParamInstr>& instr) {
    emitComment(instr->toString());
    
    // 将参数添加到参数队列
    paramQueue.push_back(instr->param);
}

// 处理函数调用指令
void CodeGenerator::processCall(const std::shared_ptr<CallInstr>& instr) {
    emitComment(instr->toString());
    
    // 获取参数个数
    int paramCount = instr->paramCount;
    
    // 检查参数队列大小是否匹配
    if (paramQueue.size() < paramCount) {
        std::cerr << "错误: 参数队列大小不匹配, 预期 " << paramCount << ", 实际 " << paramQueue.size() << std::endl;
    }
    
    // 保存调用者保存的寄存器
    saveCallerSavedRegs();
    
    // 处理参数 - 先处理寄存器传递的参数（前8个）
    int stackParamSize = 0;
    for (int i = 0; i < paramCount; i++) {
        if (i < 8) {
            // 通过寄存器传递的参数（RISC-V ABI 规定前8个参数通过 a0-a7 传递）
            std::string argReg = getArgRegister(i);
            loadOperand(paramQueue[paramQueue.size() - paramCount + i], argReg);
        } else {
            // 超过8个参数需要通过栈传递
            std::string tempReg = allocTempReg();
            loadOperand(paramQueue[paramQueue.size() - paramCount + i], tempReg);
            
            // 参数在调用者的栈帧中，相对于 sp 的偏移为 stackParamSize
            emitInstruction("sw " + tempReg + ", " + std::to_string(stackParamSize) + "(sp)");
            stackParamSize += 4; // 假设所有参数大小为4字节
            
            freeTempReg(tempReg);
        }
    }
    
    // 如果有栈传递的参数，需要调整栈指针
    if (stackParamSize > 0) {
        emitInstruction("addi sp, sp, -" + std::to_string(stackParamSize));
    }
    
    // 调用函数
    emitInstruction("call " + instr->funcName);
    
    // 如果调整了栈指针，需要恢复
    if (stackParamSize > 0) {
        emitInstruction("addi sp, sp, " + std::to_string(stackParamSize));
    }
    
    // 清除使用过的参数
    if (paramCount > 0) {
        paramQueue.erase(paramQueue.end() - paramCount, paramQueue.end());
    }
    
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

// 处理返回指令
void CodeGenerator::processReturn(const std::shared_ptr<ReturnInstr>& instr) {
    emitComment(instr->toString());
    
    // 如果有返回值，加载到a0
    if (instr->value) {
        loadOperand(instr->value, "a0");
    }
    
    // 函数返回，跳转到函数结束处理
    emitInstruction("j " + currentFunction + "_epilogue");
}

// 处理标签指令
void CodeGenerator::processLabel(const std::shared_ptr<LabelInstr>& instr) {
    // 生成标签
    emitLabel(instr->label);
}

// 处理函数开始指令
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

// 处理函数结束指令
void CodeGenerator::processFunctionEnd(const std::shared_ptr<FunctionEndInstr>& instr) {
    // 函数结束标签
    emitLabel(currentFunction + "_epilogue");
    
    // 生成函数后记
    emitEpilogue(currentFunction);
    
    // 添加空行以提高可读性
    output << "\n";
}

// 生成函数序言
void CodeGenerator::emitPrologue(const std::string& funcName) {
    emitComment("函数序言");
    
    //计算帧大小（包括保存的寄存器和局部变量空间）
    // 8字节用于保存ra和fp，其他根据函数需要的栈空间计算
    int frameSize = 8 + stackSize;
    
    // 保存返回地址和帧指针
    emitInstruction("addi sp, sp, -" + std::to_string(frameSize));
    emitInstruction("sw ra, " + std::to_string(frameSize - 4) + "(sp)");
    emitInstruction("sw fp, " + std::to_string(frameSize - 8) + "(sp)");
    
    // 设置新的帧指针
    emitInstruction("addi fp, sp, " + std::to_string(frameSize));
    
    // 保存被调用者保存的寄存器
    saveCalleeSavedRegs();

    frameInitialized = true;
    this->frameSize = frameSize;
}

// 生成函数后记
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

// 加载操作数到寄存器
void CodeGenerator::loadOperand(const std::shared_ptr<Operand>& op, const std::string& reg) {
    switch (op->type) {
        case OperandType::CONSTANT: // 常量操作数
            emitInstruction("li " + reg + ", " + std::to_string(op->value));
            break;
            
        case OperandType::VARIABLE: // 变量操作数
        case OperandType::TEMP:     // 临时变量操作数
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
            
        case OperandType::LABEL:  // 标签操作数
            // 标签通常不直接加载，而是用于跳转
            std::cerr << "警告: 尝试加载标签操作数" << std::endl;
            break;
            
        default:
            std::cerr << "错误: 未知的操作数类型" << std::endl;
            break;
    }
}

// 存储寄存器内容到操作数
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

// 获取操作数的栈偏移
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

// 分配临时寄存器
std::string CodeGenerator::allocTempReg() {
    if (nextTempReg >= tempRegs.size()) {
        nextTempReg = 0;
    }
    return tempRegs[nextTempReg++];
}

// 释放临时寄存器
void CodeGenerator::freeTempReg(const std::string& reg) {
    // 简化实现，不做实际释放，只是记录一下
}

// 保存调用者保存的寄存器
void CodeGenerator::saveCallerSavedRegs() {
    // 保存调用者保存的寄存器
    emitComment("保存调用者保存的寄存器");
    
    // 例如，保存临时寄存器t0-t6和参数寄存器a0-a7
    for (int i = 0; i < 7; i++) {
        emitInstruction("sw t" + std::to_string(i) + ", " + std::to_string(-12 - i * 4) + "(fp)");
    }

    // 参数寄存器 a0-a7 (如果需要保存)
    // 注意：通常只需要保存正在使用的寄存器
    for (int i = 0; i < 8; i++) {
        emitInstruction("sw a" + std::to_string(i) + ", " + std::to_string(-36 - i * 4) + "(fp)");
    }
}

// 恢复调用者保存的寄存器
void CodeGenerator::restoreCallerSavedRegs() {
    // 恢复调用者保存的寄存器
    emitComment("恢复调用者保存的寄存器");
    
    // 参数寄存器 a0-a7
    for (int i = 0; i < 8; i++) {
        emitInstruction("lw a" + std::to_string(i) + ", " + std::to_string(-36 - i * 4) + "(fp)");
    }
    
    // 临时寄存器 t0-t6
    for (int i = 0; i < 7; i++) {
        emitInstruction("lw t" + std::to_string(i) + ", " + std::to_string(-8 - i * 4) + "(fp)");
    }
    
}

// 保存被调用者保存的寄存器
void CodeGenerator::saveCalleeSavedRegs() {
    // 实现被调用者保存的寄存器保存，如果需要的话
}

// 恢复被调用者保存的寄存器
void CodeGenerator::restoreCalleeSavedRegs() {
    // 实现被调用者保存的寄存器恢复，如果需要的话
}

// 初始化寄存器信息
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

// 检查寄存器名称是否有效
bool CodeGenerator::isValidRegister(const std::string& reg) const {
    for (const auto& r : registers) {
        if (r.name == reg) {
            return true;
        }
    }
    return false;
}

// 获取参数寄存器名称
std::string CodeGenerator::getArgRegister(int paramIndex) const {
    if (paramIndex >= 0 && paramIndex < argRegs.size()) {
        return argRegs[paramIndex];
    }
    std::cerr << "错误: 参数索引超出范围" << std::endl;
    return "a0"; // 默认返回a0
}

// 根据策略分配寄存器
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

// 线性扫描寄存器分配方法
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

// 图着色寄存器分配方法
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

// 栈布局优化方法
void CodeGenerator::optimizeStackLayout() {
    emitComment("栈布局优化开始");
    
    // 分析变量的生命周期
    std::map<std::string, std::pair<int, int>> varLifetimes;
    analyzeVariableLifetimes(varLifetimes);
    
    // 按照生命周期结束时间排序变量
    std::vector<std::pair<std::string, std::pair<int, int>>> sortedVars(
        varLifetimes.begin(), varLifetimes.end());
    std::sort(sortedVars.begin(), sortedVars.end(), 
        [](const auto& a, const auto& b) {
            return a.second.second < b.second.second;
        });
    
    // 使用贪心算法重新分配栈空间
    std::map<std::string, int> newOffsets;
    std::vector<std::pair<int, int>> allocatedRegions; // 已分配的栈区域 <offset, end_time>
    
    for (const auto& [var, lifetime] : sortedVars) {
        int start = lifetime.first;
        int end = lifetime.second;
        int size = 4; // 假设所有变量大小为4字节
        
        // 查找可重用的栈空间
        bool found = false;
        for (auto& [offset, regionEnd] : allocatedRegions) {
            if (regionEnd < start) {
                // 可以重用这个区域
                newOffsets[var] = offset;
                regionEnd = end; // 更新区域的结束时间
                found = true;
                break;
            }
        }
        
        if (!found) {
            // 需要分配新的栈空间
            int newOffset = allocatedRegions.empty() ? -4 : allocatedRegions.back().first - size;
            newOffsets[var] = newOffset;
            allocatedRegions.push_back({newOffset, end});
        }
    }
    
    // 更新局部变量映射
    localVars = newOffsets;
    
    // 重新计算栈大小
    stackSize = 0;
    for (const auto& [var, offset] : localVars) {
        stackSize = std::max(stackSize, -offset);
    }
    
    emitComment("栈布局优化结束，新栈大小: " + std::to_string(stackSize));
}

// 分析变量的生命周期
void CodeGenerator::analyzeVariableLifetimes(std::map<std::string, std::pair<int, int>>& varLifetimes) {
    // 遍历所有指令，收集变量的定义和使用位置
    for (int i = 0; i < instructions.size(); i++) {
        auto instr = instructions[i];
        
        // 获取指令定义的变量
        auto defined = IRAnalyzer::getDefinedVariables(instr);
        for (const auto& var : defined) {
            if (varLifetimes.find(var) == varLifetimes.end()) {
                varLifetimes[var] = {i, i};
            } else {
                // 更新定义位置（取最小值）
                varLifetimes[var].first = std::min(varLifetimes[var].first, i);
            }
        }
        
        // 获取指令使用的变量
        auto used = IRAnalyzer::getUsedVariables(instr);
        for (const auto& var : used) {
            if (varLifetimes.find(var) == varLifetimes.end()) {
                varLifetimes[var] = {i, i};
            }
            // 更新使用位置（取最大值）
            varLifetimes[var].second = std::max(varLifetimes[var].second, i);
        }
    }
}

// 构建变量冲突图
std::map<std::string, std::set<std::string>> CodeGenerator::buildInterferenceGraph() {
    std::map<std::string, std::set<std::string>> interferenceGraph;
    
    // 分析变量的生命周期
    std::map<std::string, std::pair<int, int>> varLifetimes;
    analyzeVariableLifetimes(varLifetimes);
    
    // 初始化图
    for (const auto& [var, _] : varLifetimes) {
        interferenceGraph[var] = std::set<std::string>();
    }
    
    // 构建冲突图：如果两个变量的生命周期重叠，则它们之间有冲突
    for (const auto& [var1, lifetime1] : varLifetimes) {
        for (const auto& [var2, lifetime2] : varLifetimes) {
            if (var1 != var2) {
                // 检查生命周期是否重叠
                // 两个区间 [a, b] 和 [c, d] 重叠的条件是：max(a, c) <= min(b, d)
                int overlapStart = std::max(lifetime1.first, lifetime2.first);
                int overlapEnd = std::min(lifetime1.second, lifetime2.second);
                
                if (overlapStart <= overlapEnd) {
                    // 生命周期重叠，添加冲突边
                    interferenceGraph[var1].insert(var2);
                    interferenceGraph[var2].insert(var1);
                }
            }
        }
    }
    
    return interferenceGraph;
}

// 添加窥孔优化模式
void CodeGenerator::addPeepholePattern(const std::string& pattern, 
                                     std::function<bool(std::vector<std::string>&)> handler) {
    peepholePatterns[pattern] = handler;
}

// 窥孔优化方法
void CodeGenerator::peepholeOptimize(std::vector<std::string>& instructions) {
    // 初始化窥孔优化规则
    if (peepholePatterns.empty()) {
        // 模式1: 加载后立即存储的相同位置 - 消除冗余的内存操作
        // 例如: lw t0, 0(fp) 后跟 sw t0, 0(fp)
        addPeepholePattern("load_store_same", [](std::vector<std::string>& instrs) -> bool {
            if (instrs.size() < 2) return false;
            
            std::string load = instrs[0];
            std::string store = instrs[1];
            
            // 检查是否是加载后立即存储
            if (load.find("lw ") == 0 && store.find("sw ") == 0) {
                // 提取寄存器和内存位置
                std::string loadReg = load.substr(3, load.find(",") - 3);
                std::string loadMem = load.substr(load.find(",") + 1);
                std::string storeReg = store.substr(3, store.find(",") - 3);
                std::string storeMem = store.substr(store.find(",") + 1);
                
                // 如果是相同的寄存器和内存位置，则可以删除这两条指令
                if (loadReg == storeReg && loadMem == storeMem) {
                    instrs.clear();
                    return true;
                }
            }
            return false;
        });
        
        // 模式2: 加载立即数后立即执行比较 - 可以合并为立即数比较
        // 例如: li t0, 0 后跟 beq t1, t0, label -> beqz t1, label
        addPeepholePattern("li_compare", [](std::vector<std::string>& instrs) -> bool {
            if (instrs.size() < 2) return false;
            
            std::string li = instrs[0];
            std::string branch = instrs[1];
            
            if (li.find("li ") == 0) {
                std::string reg = li.substr(3, li.find(",") - 3);
                std::string imm = li.substr(li.find(",") + 1);
                imm = imm.substr(imm.find_first_not_of(" \t"));
                
                // 检查是否为0
                if (imm == "0") {
                    // 检查分支指令
                    if (branch.find("beq ") == 0) {
                        std::string branchParts = branch.substr(4);
                        size_t commaPos = branchParts.find(",");
                        std::string reg1 = branchParts.substr(0, commaPos);
                        reg1 = reg1.substr(reg1.find_first_not_of(" \t"));
                        
                        std::string rest = branchParts.substr(commaPos + 1);
                        size_t nextCommaPos = rest.find(",");
                        std::string reg2 = rest.substr(0, nextCommaPos);
                        reg2 = reg2.substr(reg2.find_first_not_of(" \t"));
                        
                        std::string label = rest.substr(nextCommaPos + 1);
                        label = label.substr(label.find_first_not_of(" \t"));
                        
                        // 如果比较的是加载的立即数
                        if (reg2 == reg) {
                            instrs.clear();
                            instrs.push_back("beqz " + reg1 + ", " + label);
                            return true;
                        } else if (reg1 == reg) {
                            instrs.clear();
                            instrs.push_back("beqz " + reg2 + ", " + label);
                            return true;
                        }
                    }
                    // 处理其他分支指令...
                }
            }
            return false;
        });
        
        // 模式3: 移动到同一寄存器 - 消除冗余移动
        addPeepholePattern("redundant_move", [](std::vector<std::string>& instrs) -> bool {
            if (instrs.size() < 1) return false;
            
            std::string move = instrs[0];
            if (move.find("mv ") == 0) {
                std::string dst = move.substr(3, move.find(",") - 3);
                std::string src = move.substr(move.find(",") + 1);
                src = src.substr(src.find_first_not_of(" \t"));
                
                // 如果源和目标相同，删除指令
                if (dst == src) {
                    instrs.clear();
                    return true;
                }
            }
            return false;
        });
    }
    
    // 应用窥孔优化
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (size_t i = 0; i < instructions.size(); ) {
            bool patternApplied = false;
            
            // 对每个优化模式尝试应用
            for (auto& [pattern, handler] : peepholePatterns) {
                // 最多查看前3条指令
                size_t windowSize = std::min(size_t(3), instructions.size() - i);
                std::vector<std::string> window(instructions.begin() + i, 
                                               instructions.begin() + i + windowSize);
                
                if (handler(window)) {
                    // 如果模式匹配并应用了优化
                    changed = true;
                    patternApplied = true;
                    
                    // 替换原指令
                    instructions.erase(instructions.begin() + i, 
                                      instructions.begin() + i + windowSize);
                    
                    // 插入优化后的指令(如果有)
                    instructions.insert(instructions.begin() + i, 
                                       window.begin(), window.end());
                    
                    // 跳过已优化的指令
                    i += window.size();
                    break;
                }
            }
            
            // 如果没有应用任何模式，移动到下一条指令
            if (!patternApplied) {
                i++;
            }
        }
    }
}


// 简单寄存器分配器实现
std::map<std::string, std::string> NaiveRegisterAllocator::allocate(
    const std::vector<std::shared_ptr<IRInstr>>& instructions,
    const std::vector<Register>& availableRegs) {
    std::map<std::string, std::string> allocation;
    
    // 收集所有变量
    std::set<std::string> variables;
    for (const auto& instr : instructions) {
        // 获取指令定义的变量
        auto defined = IRAnalyzer::getDefinedVariables(instr);
        for (const auto& var : defined) {
            variables.insert(var);
        }
        
        // 获取指令使用的变量
        auto used = IRAnalyzer::getUsedVariables(instr);
        for (const auto& var : used) {
            variables.insert(var);
        }
    }
    
    // 筛选可分配的寄存器
    std::vector<std::string> allocatableRegs;
    for (const auto& reg : availableRegs) {
        if (reg.isAllocatable && reg.isCalleeSaved) {
            // 优先使用被调用者保存的寄存器
            allocatableRegs.push_back(reg.name);
        }
    }
    
    // 如果被调用者保存的寄存器不够，再使用调用者保存的寄存器
    if (allocatableRegs.size() < variables.size()) {
        for (const auto& reg : availableRegs) {
            if (reg.isAllocatable && reg.isCallerSaved && 
                std::find(allocatableRegs.begin(), allocatableRegs.end(), reg.name) == allocatableRegs.end()) {
                allocatableRegs.push_back(reg.name);
            }
        }
    }
    
    // 简单地将变量分配到寄存器，直到寄存器用完
    auto regIt = allocatableRegs.begin();
    for (const auto& var : variables) {
        if (regIt != allocatableRegs.end()) {
            allocation[var] = *regIt;
            ++regIt;
        } else {
            // 寄存器用完，剩余变量使用栈存储
            break;
        }
    }
    
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
    
    // 可用寄存器列表
    std::vector<std::string> freeRegs;
    for (const auto& reg : availableRegs) {
        // 只使用被调用者保存的寄存器(s0-s11)进行全局分配
        // 这样可以减少函数调用时的寄存器保存/恢复开销
        if (reg.isCalleeSaved && reg.name != "fp" && reg.name != "s0") {
            freeRegs.push_back(reg.name);
        }
    }
    
    // 当前活跃的区间及其分配的寄存器
    std::map<std::string, std::pair<LiveInterval, std::string>> active;
    
    // 线性扫描算法
    for (const auto& interval : intervals) {
        // 过期活跃区间
        std::vector<std::string> expired;
        for (auto& [var, pair] : active) {
            if (pair.first.end < interval.start) {
                freeRegs.push_back(pair.second); // 释放寄存器
                expired.push_back(var);
            }
        }
        
        // 从活跃集合中移除过期区间
        for (const auto& var : expired) {
            active.erase(var);
        }
        
        // 如果没有可用寄存器，需要溢出
        if (freeRegs.empty()) {
            // 找到最晚结束的活跃区间
            std::string victimVar;
            int latestEnd = -1;
            
            for (const auto& [var, pair] : active) {
                if (pair.first.end > latestEnd) {
                    latestEnd = pair.first.end;
                    victimVar = var;
                }
            }
            
            // 如果当前区间比受害者结束更早，则溢出受害者
            if (latestEnd > interval.end && !victimVar.empty()) {
                allocation[interval.var] = active[victimVar].second;
                freeRegs.push_back(active[victimVar].second);
                active.erase(victimVar);
                
                // 将新区间加入活跃集合
                active[interval.var] = {interval, allocation[interval.var]};
            }
            // 否则当前区间溢出（不分配寄存器）
        } else {
            // 有可用寄存器，分配一个
            std::string reg = freeRegs.back();
            freeRegs.pop_back();
            
            allocation[interval.var] = reg;
            active[interval.var] = {interval, reg};
        }
    }
    
    return allocation;
}

// 计算变量的生命周期区间
std::vector<LinearScanRegisterAllocator::LiveInterval> LinearScanRegisterAllocator::computeLiveIntervals(
    const std::vector<std::shared_ptr<IRInstr>>& instructions) {
    
    std::map<std::string, LiveInterval> intervalMap;
    
    // 扫描所有指令，计算变量的定义和使用位置
    for (int i = 0; i < instructions.size(); i++) {
        auto instr = instructions[i];
        
        // 获取指令定义的变量
        auto defined = IRAnalyzer::getDefinedVariables(instr);
        for (const auto& var : defined) {
            // 如果变量尚未有区间，创建一个新区间
            if (intervalMap.find(var) == intervalMap.end()) {
                intervalMap[var] = {var, i, i};
            } 
            // 否则更新开始位置（取最小值）
            else {
                intervalMap[var].start = std::min(intervalMap[var].start, i);
            }
        }
        
        // 获取指令使用的变量
        auto used = IRAnalyzer::getUsedVariables(instr);
        for (const auto& var : used) {
            // 如果变量尚未有区间，创建一个新区间
            if (intervalMap.find(var) == intervalMap.end()) {
                intervalMap[var] = {var, i, i};
            }
            // 更新结束位置（取最大值）
            intervalMap[var].end = std::max(intervalMap[var].end, i);
        }
    }
    
    // 转换为向量形式
    std::vector<LiveInterval> intervals;
    for (const auto& [var, interval] : intervalMap) {
        intervals.push_back(interval);
    }
    
    return intervals;
}

// 图着色寄存器分配器实现
std::map<std::string, std::string> GraphColoringRegisterAllocator::allocate(
    const std::vector<std::shared_ptr<IRInstr>>& instructions,
    const std::vector<Register>& availableRegs) {
    
    std::map<std::string, std::string> allocation;
    
    // 构建冲突图
    auto interferenceGraph = buildInterferenceGraph(instructions);
    
    // 如果图为空，直接返回空分配
    if (interferenceGraph.empty()) {
        return allocation;
    }
    
    // 简化冲突图，获取简化后的变量顺序
    auto simplifiedOrder = simplify(interferenceGraph);
    
    // 根据简化顺序为变量分配寄存器
    allocation = color(simplifiedOrder, interferenceGraph, availableRegs);
    
    return allocation;
}

// 构建变量冲突图
std::map<std::string, std::set<std::string>> GraphColoringRegisterAllocator::buildInterferenceGraph(
    const std::vector<std::shared_ptr<IRInstr>>& instructions) {
    
    std::map<std::string, std::set<std::string>> graph;
    
    // 分析变量的生命周期
    std::map<std::string, std::pair<int, int>> varLifetimes;
    
    // 收集所有变量及其生命周期
    for (int i = 0; i < instructions.size(); i++) {
        auto instr = instructions[i];
        
        // 获取指令定义的变量
        auto defined = IRAnalyzer::getDefinedVariables(instr);
        for (const auto& var : defined) {
            if (varLifetimes.find(var) == varLifetimes.end()) {
                varLifetimes[var] = {i, i};
            } else {
                // 更新定义位置（取最小值）
                varLifetimes[var].first = std::min(varLifetimes[var].first, i);
            }
        }
        
        // 获取指令使用的变量
        auto used = IRAnalyzer::getUsedVariables(instr);
        for (const auto& var : used) {
            if (varLifetimes.find(var) == varLifetimes.end()) {
                varLifetimes[var] = {i, i};
            }
            // 更新使用位置（取最大值）
            varLifetimes[var].second = std::max(varLifetimes[var].second, i);
        }
    }
    
    // 初始化图
    for (const auto& [var, _] : varLifetimes) {
        graph[var] = std::set<std::string>();
    }
    
    // 构建冲突图：如果两个变量的生命周期重叠，则它们之间有冲突
    for (const auto& [var1, lifetime1] : varLifetimes) {
        for (const auto& [var2, lifetime2] : varLifetimes) {
            if (var1 != var2) {
                // 检查生命周期是否重叠
                // 两个区间 [a, b] 和 [c, d] 重叠的条件是：max(a, c) <= min(b, d)
                int overlapStart = std::max(lifetime1.first, lifetime2.first);
                int overlapEnd = std::min(lifetime1.second, lifetime2.second);
                
                if (overlapStart <= overlapEnd) {
                    // 生命周期重叠，添加冲突边
                    graph[var1].insert(var2);
                    graph[var2].insert(var1);
                }
            }
        }
    }
    
    return graph;
}

// 简化冲突图
std::vector<std::string> GraphColoringRegisterAllocator::simplify(
    std::map<std::string, std::set<std::string>>& graph) {
    
    std::vector<std::string> simplifiedOrder;
    
    // 创建图的副本，因为我们会修改它
    auto workGraph = graph;
    
    // 当图不为空时，继续简化
    while (!workGraph.empty()) {
        // 查找度数小于可用寄存器数量的节点
        std::string nodeToRemove;
        int minDegree = std::numeric_limits<int>::max();
        
        for (const auto& [node, neighbors] : workGraph) {
            if (neighbors.size() < minDegree) {
                minDegree = neighbors.size();
                nodeToRemove = node;
            }
        }
        
        // 如果找不到合适的节点，我们需要选择一个"溢出"节点
        // 这里简单地选择度数最大的节点作为溢出候选
        if (nodeToRemove.empty()) {
            int maxDegree = -1;
            for (const auto& [node, neighbors] : workGraph) {
                if (neighbors.size() > maxDegree) {
                    maxDegree = neighbors.size();
                    nodeToRemove = node;
                }
            }
        }
        
        // 从图中移除该节点
        for (auto& [_, neighbors] : workGraph) {
            neighbors.erase(nodeToRemove);
        }
        workGraph.erase(nodeToRemove);
        
        // 将节点添加到简化序列
        simplifiedOrder.push_back(nodeToRemove);
    }
    
    // 逆序返回，这样我们可以从最后一个移除的节点开始着色
    std::reverse(simplifiedOrder.begin(), simplifiedOrder.end());
    return simplifiedOrder;
}

// 图着色算法 - 为变量分配寄存器
std::map<std::string, std::string> GraphColoringRegisterAllocator::color(
    const std::vector<std::string>& simplifiedOrder,
    const std::map<std::string, std::set<std::string>>& originalGraph,
    const std::vector<Register>& availableRegs) {
    
    std::map<std::string, std::string> allocation;
    
    // 筛选可分配的寄存器
    std::vector<std::string> regNames;
    for (const auto& reg : availableRegs) {
        if (reg.isAllocatable && !reg.isReserved) {
            regNames.push_back(reg.name);
        }
    }
    
    // 按照简化顺序为变量分配颜色（寄存器）
    for (const auto& var : simplifiedOrder) {
        // 获取变量的邻居
        auto it = originalGraph.find(var);
        if (it == originalGraph.end()) continue;
        
        const auto& neighbors = it->second;
        
        // 收集邻居已使用的颜色
        std::set<std::string> usedColors;
        for (const auto& neighbor : neighbors) {
            auto allocIt = allocation.find(neighbor);
            if (allocIt != allocation.end()) {
                usedColors.insert(allocIt->second);
            }
        }
        
        // 查找可用的颜色
        std::string selectedReg;
        for (const auto& reg : regNames) {
            if (usedColors.find(reg) == usedColors.end()) {
                selectedReg = reg;
                break;
            }
        }
        
        // 如果找到可用寄存器，进行分配
        if (!selectedReg.empty()) {
            allocation[var] = selectedReg;
        }
        // 否则，变量需要溢出到内存（不分配寄存器）
    }
    
    return allocation;
}
