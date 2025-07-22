// codegen.cpp - 代码生成器实现
// 本文件实现了将IR指令转换为RISC-V汇编代码的功能
#include "codegen.h"
#include <sstream>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <queue>
#include <stack>
#include <limits>

// 代码生成器构造函数，初始化输出文件和配置
CodeGenerator::CodeGenerator(std::ostream& outputStream,  
                           const std::vector<std::shared_ptr<IRInstr>>& instructions,
                           const CodeGenConfig& config)
    : output(outputStream), instructions(instructions), config(config) {
    
    
    std::cerr << "CodeGenerator构造函数开始\n";
    
    // 打印IR指令数量
    std::cerr << "IR指令数量: " << instructions.size() << "\n";
    
    // 初始化寄存器信息
    std::cerr << "初始化寄存器信息\n";

    // 初始化寄存器信息
    initializeRegisters();

    std::cerr << "寄存器信息初始化完成\n";
    std::cerr << "输出文件头\n";

    // 输出文件头
    emitComment("由ToyC编译器生成");
    emitComment("RISC-V汇编代码");
    emitSection(".text");
    
    // 如果启用了寄存器分配优化，则执行寄存器分配
    if (config.regAllocStrategy != RegisterAllocStrategy::NAIVE) {
        std::cerr << "执行寄存器分配\n";
        allocateRegisters();
        std::cerr << "寄存器分配完成\n";
    }
    std::cerr << "CodeGenerator构造函数完成\n";
}

// 析构函数，关闭输出文件
CodeGenerator::~CodeGenerator() {
}

// 生成汇编代码的主方法
// 处理所有IR指令，应用优化，并输出最终的汇编代码
void CodeGenerator::generate() {
    std::cerr << "进入generate方法\n";

    // 创建一个临时存储生成的汇编指令的向量
    std::vector<std::string> asmInstructions;

    std::cerr << "开始处理IR指令, 总数: " << instructions.size() << "\n";
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
    
    std::cerr << "IR指令处理完成\n";

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

    std::cerr << "generate方法执行完成\n";
}

// 将单条IR指令处理结果写入指定流
// 临时重定向输出，处理指令，然后恢复原始输出
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
// 使用递增计数器确保标签唯一性
std::string CodeGenerator::genLabel() {
    return "L" + std::to_string(labelCount++);
}

// 输出注释
// 在汇编代码中添加注释行
void CodeGenerator::emitComment(const std::string& comment) {
    output << "# " << comment << "\n";
}

// 输出指令
// 在汇编代码中添加指令行
void CodeGenerator::emitInstruction(const std::string& instr) {
    output << "\t" << instr << "\n";
}

// 输出标签
// 在汇编代码中添加标签定义
void CodeGenerator::emitLabel(const std::string& label) {
    output << label << ":\n";
}

// 输出全局标识符
// 声明一个全局可见的符号
void CodeGenerator::emitGlobal(const std::string& name) {
    output << "\t.global " << name << "\n";
}

// 输出段定义
// 定义代码或数据段
void CodeGenerator::emitSection(const std::string& section) {
    output << section << "\n";
}

// 处理IR指令
// 根据指令的操作码类型调用相应的处理函数
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
// 为二元运算生成相应的RISC-V汇编代码
// AND 和 OR 要短路求值
void CodeGenerator::processBinaryOp(const std::shared_ptr<BinaryOpInstr>& instr) {
    /*emitComment(instr->toString());
    
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
    freeTempReg(resultReg);*/

    emitComment(instr->toString());

    

    // 获取临时寄存器

    std::string resultReg = allocTempReg();

    

    // 特殊处理逻辑运算符的短路求值

    if (instr->opcode == OpCode::AND || instr->opcode == OpCode::OR) {

        std::string leftReg = allocTempReg();

        

        // 加载左操作数

        loadOperand(instr->left, leftReg);

        

        // 为短路求值生成标签

        std::string endLabel = genLabel();

        std::string shortCircuitLabel = genLabel();

        

        if (instr->opcode == OpCode::AND) {

            // 对于 AND：如果左操作数为假，直接跳转到短路标签

            emitInstruction("beqz " + leftReg + ", " + shortCircuitLabel);

            

            // 左操作数为真，计算右操作数

            std::string rightReg = allocTempReg();

            loadOperand(instr->right, rightReg);

            

            // 将右操作数转换为布尔值（0或1）

            emitInstruction("snez " + resultReg + ", " + rightReg);

            freeTempReg(rightReg);

            

            // 跳转到结束

            emitInstruction("j " + endLabel);

            

            // 短路标签：结果为假（0）

            emitLabel(shortCircuitLabel);

            emitInstruction("li " + resultReg + ", 0");

            

        } else { // OpCode::OR

            // 对于 OR：如果左操作数为真，直接跳转到短路标签

            emitInstruction("bnez " + leftReg + ", " + shortCircuitLabel);

            

            // 左操作数为假，计算右操作数

            std::string rightReg = allocTempReg();

            loadOperand(instr->right, rightReg);

            

            // 将右操作数转换为布尔值（0或1）

            emitInstruction("snez " + resultReg + ", " + rightReg);

            freeTempReg(rightReg);

            

            // 跳转到结束

            emitInstruction("j " + endLabel);

            

            // 短路标签：结果为真（1）

            emitLabel(shortCircuitLabel);

            emitInstruction("li " + resultReg + ", 1");

        }

        

        // 结束标签

        emitLabel(endLabel);

        

        // 释放左操作数寄存器

        freeTempReg(leftReg);

        

    } else {

        // 其他二元操作符的处理保持不变

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

            default:

                std::cerr << "错误: 未知的二元操作" << std::endl;

                break;

        }

        

        // 释放临时寄存器

        freeTempReg(rightReg);

        freeTempReg(leftReg);

    }

    

    // 存储结果

    storeRegister(resultReg, instr->result);

    

    // 释放结果寄存器

    freeTempReg(resultReg);


}

// 处理一元操作指令
// 为一元运算生成相应的RISC-V汇编代码
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
// 将源操作数的值赋给目标操作数
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
// 生成无条件跳转到目标标签的指令
void CodeGenerator::processGoto(const std::shared_ptr<GotoInstr>& instr) {
    emitComment(instr->toString());
    
    // 直接跳转到目标标签
    emitInstruction("j " + instr->target->name);
}

// 处理条件跳转指令
// 如果条件为真，则跳转到目标标签
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

// 参数传递队列（全局静态变量）
// 用于收集函数调用的参数，直到调用函数时使用
//static std::vector<std::shared_ptr<Operand>> paramQueue;

// 处理参数指令
// 将参数添加到参数队列中，等待后续函数调用使用
void CodeGenerator::processParam(const std::shared_ptr<ParamInstr>& instr) {
    emitComment(instr->toString());
    
    // 将参数添加到参数队列
    paramQueue.push_back(instr->param);
}


// 处理函数调用指令
// 准备参数，调用函数，处理返回值
void CodeGenerator::processCall(const std::shared_ptr<CallInstr>& instr) {
    // 添加空指针检查
    if (!instr) {
        std::cerr << "错误: 空的函数调用指令" << std::endl;
        return;
    }

    emitComment(instr->toString());
    
    // 获取参数个数
    int paramCount = instr->paramCount;
    std::vector<std::shared_ptr<Operand>> params;
    
    // 优先使用 CallInstr 中存储的参数列表
    if (!instr->params.empty()) {
        params = instr->params;
    } else if (!paramQueue.empty()) {
        // 兼容旧代码，使用 paramQueue
        if (paramQueue.size() >= paramCount) {
            //params.assign(paramQueue.end() - paramCount, paramQueue.end());
            // 添加安全检查，确保不会访问越界
            size_t startIdx = paramQueue.size() - paramCount;
            params.reserve(paramCount);
            for (size_t i = 0; i < paramCount && (startIdx + i) < paramQueue.size(); ++i) {
                params.push_back(paramQueue[startIdx + i]);
            }
        } else {
            std::cerr << "错误: 参数队列大小不匹配, 预期 " << paramCount 
                      << ", 实际 " << paramQueue.size() << std::endl;
            // 防止后续的段错误
            return;
        }
    } else {
        std::cerr << "错误: 没有可用的参数" << std::endl;
        return;
    }
    
    // 保存调用者保存的寄存器
    //saveCallerSavedRegs();
    
    // 处理参数 - 先处理寄存器传递的参数（前8个）
    // int stackParamSize = 0;
    // for (int i = 0; i < paramCount; i++) {
    //     if (i < 8) {
    //         // 通过寄存器传递的参数（RISC-V ABI 规定前8个参数通过 a0-a7 传递）
    //         std::string argReg = getArgRegister(i);
    //         loadOperand(paramQueue[paramQueue.size() - paramCount + i], argReg);
    //     } else {
    //         // 超过8个参数需要通过栈传递
    //         std::string tempReg = allocTempReg();
    //         loadOperand(paramQueue[paramQueue.size() - paramCount + i], tempReg);
            
    //         // 参数在调用者的栈帧中，相对于 sp 的偏移为 stackParamSize
    //         emitInstruction("sw " + tempReg + ", " + std::to_string(stackParamSize) + "(sp)");
    //         stackParamSize += 4; // 假设所有参数大小为4字节
            
    //         freeTempReg(tempReg);
    //     }
    // }

    // 处理参数 - 先处理寄存器传递的参数（前8个）
    /*int stackParamSize = 0;
    for (int i = 0; i < paramCount && i < params.size(); i++) {
        // 添加空指针检查
        if (!params[i]) {
            std::cerr << "错误: 参数 " << i << " 为空" << std::endl;
            continue;
        }
        
        if (i < 8) {
            // 通过寄存器传递的参数（RISC-V ABI 规定前8个参数通过 a0-a7 传递）
            std::string argReg = getArgRegister(i);
            
            // 使用安全索引访问
            size_t paramIndex = paramQueue.size() - paramCount + i;
            if (paramIndex < paramQueue.size()) {
                loadOperand(paramQueue[paramIndex], argReg);
            } else {
                std::cerr << "错误: 参数索引越界: " << paramIndex << std::endl;
            }
        } else {
            // 超过8个参数需要通过栈传递
            std::string tempReg = allocTempReg();
            
            // 使用安全索引访问
            size_t paramIndex = paramQueue.size() - paramCount + i;
            if (paramIndex < paramQueue.size()) {
                loadOperand(paramQueue[paramIndex], tempReg);
                
                // 参数在调用者的栈帧中，相对于 sp 的偏移为 stackParamSize
                emitInstruction("sw " + tempReg + ", " + std::to_string(stackParamSize) + "(sp)");
                stackParamSize += 4; // 假设所有参数大小为4字节
            } else {
                std::cerr << "错误: 参数索引越界: " << paramIndex << std::endl;
            }
            
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
    }*/

    // 1. 分析需要保存的寄存器
    analyzeUsedCallerSavedRegs();  // 确定哪些调用者寄存器需要保存
    analyzeUsedCalleeSavedRegs();  // 确定哪些被调用者寄存器需要保存

    // 2. 计算栈空间需求
    /*int stackParamCount = std::max(0, paramCount - 8);  // 需要栈传递的参数数量
    int stackParamSize = stackParamCount * 4;          // 栈参数总大小
    int callerRegsSize = usedCallerSavedRegs.size() * 4; // 调用者寄存器保存空间
    int calleeRegsSize = usedCalleeSavedRegs.size() * 4; // 被调用者寄存器保存空间
    
    // 总栈空间（16字节对齐）
    int totalStackSize = stackParamSize + callerRegsSize + calleeRegsSize;
    totalStackSize = (totalStackSize + 15) & ~15;

    // 3. 分配栈空间
    emitInstruction("addi sp, sp, -" + std::to_string(totalStackSize));*/

    // 4. 保存被调用者寄存器（Callee-Saved）
    //saveCalleeSavedRegs();

    // 5. 保存调用者寄存器（Caller-Saved）
    saveCallerSavedRegs();

    // 6. 传递参数
    // 6.1 寄存器参数（a0-a7）
    for (int i = 0; i < std::min(8, paramCount); ++i) {
        if (!params[i]) continue;
        loadOperand(params[i], "a" + std::to_string(i));
    }

    // 6.2 栈参数
    int stackParamOffset = callerRegsSize + calleeRegsSize; // 栈参数起始偏移
    /*for (int i = 8; i < paramCount; ++i) {
        if (!params[i]) continue;
        std::string tempReg = allocTempReg();
        loadOperand(params[i], tempReg);
        emitInstruction("sw " + tempReg + ", " + std::to_string(stackParamOffset) + "(sp)");
        stackParamOffset += 4;
        freeTempReg(tempReg);
    }*/
    for (int i = paramCount - 1; i >= 8; --i) {
        if (!params[i]) continue;
        std::string tempReg = allocTempReg();
        loadOperand(params[i], tempReg);
        emitInstruction("sw " + tempReg + ", " + std::to_string(stackParamOffset) + "(sp)");
        stackParamOffset += 4;
        freeTempReg(tempReg);
    }

    // 7. 调用函数
    emitInstruction("call " + instr->funcName);

    // 8. 恢复调用者寄存器
    restoreCallerSavedRegs();

    // 9. 恢复被调用者寄存器
    //restoreCalleeSavedRegs();

    // 10. 释放栈空间
    //emitInstruction("addi sp, sp, " + std::to_string(totalStackSize));

    // 11. 处理返回值
    if (instr->result) {
        std::string resultReg = allocTempReg();
        //emitInstruction("mv " + resultReg + ", a0");      //伪指令
        emitInstruction("addi " + resultReg + ", a0" + ", 0");
        storeRegister(resultReg, instr->result);
        freeTempReg(resultReg);
    }

    // 12. 清除已使用的参数
    if (!instr->params.empty() && paramCount > 0) {
        paramQueue.erase(paramQueue.end() - paramCount, paramQueue.end());
    }
}                                     

// 处理返回指令
// 如果有返回值，将其加载到a0寄存器，然后跳转到函数结束处理
void CodeGenerator::processReturn(const std::shared_ptr<ReturnInstr>& instr) {
    emitComment(instr->toString());
    
    // 如果有返回值，加载到a0
    if (instr->value) {
        loadOperand(instr->value, "a0");
    }else if (currentFunctionReturnType != "void") {
        // 如果函数返回类型不是void，但没有明确的返回值，默认返回0
        emitInstruction("li a0, 0");
    }
    
    // 函数返回，跳转到函数结束处理
    emitInstruction("j " + currentFunction + "_epilogue");
}

// 处理标签指令
// 在汇编代码中生成标签定义
void CodeGenerator::processLabel(const std::shared_ptr<LabelInstr>& instr) {
    // 生成标签
    emitLabel(instr->label);
}

// 处理函数开始指令
// 初始化函数上下文，生成函数标签和序言
void CodeGenerator::processFunctionBegin(const std::shared_ptr<FunctionBeginInstr>& instr) {
    // 添加空指针检查
    if (!instr) {
        std::cerr << "错误: 空的函数开始指令" << std::endl;
        return;
    }
    
    // 初始化函数上下文
    currentFunction = instr->funcName;
    currentFunctionReturnType = instr->returnType;
    currentFunctionParams = instr->paramNames;

    // 重置栈相关状态
    stackSize = 0;
    frameSize = 8;          // 初始为ra和fp的保存空间
    localVarsSize = 0;      // 临时变量的空间
    calleeRegsSize = 0;     // 被调用者保存寄存器占用的空间
    localVars.clear();
    frameInitialized = false;

    // 分析实际使用的被调用者保存寄存器
    analyzeUsedCalleeSavedRegs();
    
    // 为函数参数预分配栈空间（区分寄存器传递和栈传递）
    for (size_t i = 0; i < currentFunctionParams.size(); i++) {
        if (i < 8) {
            // 寄存器参数：绑定到a0-a7，不默认分配栈空间
            regAlloc[currentFunctionParams[i]] = "a" + std::to_string(i);
        } else {
            // 栈传递参数：分配栈空间并记录偏移
            localVars[currentFunctionParams[i]] = -12 - (i-8)*4;  // 跳过ra/fp
            localVarsSize += 4;
        }
    }

    // 确保分配足够的栈空间
    int minStackSize = 120;
    if (stackSize < minStackSize) {
        stackSize = minStackSize;
    }

    // 生成函数标签
    emitGlobal(instr->funcName);
    emitLabel(instr->funcName);
    
    // 生成函数序言
    emitPrologue(instr->funcName);

    if (currentFunctionParams.empty()) {
        // 没有参数，跳过参数处理
        return;
    }

    // 清空localvars
    localVars.clear();

    // 处理函数参数
    emitComment("函数形参压栈");
    for (size_t i = 0; i < currentFunctionParams.size(); i++) {
        // 创建参数变量
        std::shared_ptr<Operand> paramVar = std::make_shared<Operand>(OperandType::VARIABLE, currentFunctionParams[i]);
        
        // 获取参数变量的栈偏移
        int offset = getOperandOffset(paramVar);
        
        // 从参数寄存器保存到栈
        if (i < 8) {
            std::string argReg = getArgRegister(i);
            // 压栈之后从regalloc中删除当前参数的寄存器绑定
            regAlloc.erase(currentFunctionParams[i]);  // 删除寄存器绑定
            // 将参数从寄存器保存到栈
            emitInstruction("sw " + argReg + ", " + std::to_string(offset) + "(fp)");
        } else {
            // 对于超过8个的参数，它们在调用方的栈上
            // 需要从调用方的栈中加载
            std::string tempReg = allocTempReg();
            int callerStackOffset = (i - 8) * 4 ; 
            emitInstruction("lw " + tempReg + ", " + std::to_string(callerStackOffset) + "(fp)");   //从调用者栈加载参数
            emitInstruction("sw " + tempReg + ", " + std::to_string(offset) + "(fp)");              //将参数存入当前栈帧，方便后续访问
            freeTempReg(tempReg);
        }
    }
}

// 处理函数结束指令
// 生成函数结束标签和后记
void CodeGenerator::processFunctionEnd(const std::shared_ptr<FunctionEndInstr>& instr) {
    // 函数结束标签
    emitLabel(currentFunction + "_epilogue");
    
    // 生成函数后记
    emitEpilogue(currentFunction);
    
    // 添加空行以提高可读性
    output << "\n";

    // 清除函数上下文
    currentFunction = "";
    currentFunctionReturnType = "";
    currentFunctionParams.clear();
    regAlloc.clear();                   // 清除寄存器绑定
    localVars.clear();                  // 清除局部变量偏移
    frameInitialized = false;
    stackSize = 0;
}

// 生成函数序言
// 设置栈帧，保存返回地址和帧指针
void CodeGenerator::emitPrologue(const std::string& funcName) {
    emitComment("函数序言");
    
    //重置栈帧偏移量
    resetStackOffset();

    // 1. 分析实际使用的寄存器
    //analyzeUsedCalleeSavedRegs();  // 确定需要保存的s0-s11
    //analyzeUsedCallerSavedRegs();  // 确定需要保存的t0-t6/a0-a7（供后续调用使用）

    // 2. 计算各区域大小
    calleeRegsSize = countUsedCalleeSavedRegs() * 4;  // 被调用者保存寄存器空间
    callerRegsSize = countUsedCallerSavedRegs() * 4;  // 调用者保存寄存器空间
    //int localsAndPadding = (localVarsSize + 15) & ~15; // 局部变量+对齐填充
    int localsAndPadding = analyzeTempVars();  // 分析临时变量需求
    int totalFrameSize = calleeRegsSize + callerRegsSize + localsAndPadding + 8; // +8 for ra/fp
    totalFrameSize = (totalFrameSize + 15) & ~15;      // 最终16字节对齐
//---------------------修改18-----------------------------
    // 3. 分配栈空间
    //emitInstruction("addi sp, sp, -" + std::to_string(totalFrameSize));

    // 4. 保存关键寄存器
    //emitInstruction("sw ra, " + std::to_string(totalFrameSize - 4) + "(sp)");
    //emitInstruction("sw fp, " + std::to_string(totalFrameSize - 8) + "(sp)");

    // 5. 设置新帧指针（指向旧sp）
    //emitInstruction("addi fp, sp, " + std::to_string(totalFrameSize));

    // 3. 分配栈空间
    // 修改这部分：
    if (totalFrameSize <= 2048) {
        emitInstruction("addi sp, sp, -" + std::to_string(totalFrameSize));
    } else {
        // 栈帧太大，需要使用多条指令
        emitInstruction("li t0, -" + std::to_string(totalFrameSize));
        emitInstruction("add sp, sp, t0");
    }

    // 4. 保存关键寄存器
    // 修改这部分：
    if (totalFrameSize - 4 <= 2047) {
        emitInstruction("sw ra, " + std::to_string(totalFrameSize - 4) + "(sp)");
    } else {
        emitInstruction("li t0, " + std::to_string(totalFrameSize - 4));
        emitInstruction("add t0, sp, t0");
        emitInstruction("sw ra, 0(t0)");
    }
    
    if (totalFrameSize - 8 <= 2047) {
        emitInstruction("sw fp, " + std::to_string(totalFrameSize - 8) + "(sp)");
    } else {
        emitInstruction("li t0, " + std::to_string(totalFrameSize - 8));
        emitInstruction("add t0, sp, t0");
        emitInstruction("sw fp, 0(t0)");
    }

    // 5. 设置新帧指针（指向旧sp）
    if (totalFrameSize <= 2048) {
        emitInstruction("addi fp, sp, " + std::to_string(totalFrameSize));
    } else {
        emitInstruction("li t0, " + std::to_string(totalFrameSize));
        emitInstruction("add fp, sp, t0");
    }
//---------------------修改18-----------------------------
    // 6. 保存被调用者寄存器
    saveCalleeSavedRegs();

    // 更新状态
    frameInitialized = true;
    this->frameSize = totalFrameSize;
}

// 生成函数后记
// 恢复保存的寄存器，恢复栈帧，返回
void CodeGenerator::emitEpilogue(const std::string& funcName) {
    emitComment("函数后记");
    
    // 恢复被调用者保存的寄存器
    restoreCalleeSavedRegs();
//---------------------修改18------------------------

    // 恢复帧指针和返回地址
    //emitInstruction("lw fp, " + std::to_string(frameSize - 8) + "(sp)");
    //emitInstruction("lw ra, " + std::to_string(frameSize - 4) + "(sp)");
    
    // 调整栈指针
    //emitInstruction("addi sp, sp, " + std::to_string(frameSize));
    
    // 恢复帧指针和返回地址
    if (frameSize - 8 <= 2047) {
        emitInstruction("lw fp, " + std::to_string(frameSize - 8) + "(sp)");
    } else {
        emitInstruction("li t0, " + std::to_string(frameSize - 8));
        emitInstruction("add t0, sp, t0");
        emitInstruction("lw fp, 0(t0)");
    }
    
    if (frameSize - 4 <= 2047) {
        emitInstruction("lw ra, " + std::to_string(frameSize - 4) + "(sp)");
    } else {
        emitInstruction("li t0, " + std::to_string(frameSize - 4));
        emitInstruction("add t0, sp, t0");
        emitInstruction("lw ra, 0(t0)");
    }
    
    // 调整栈指针
    if (frameSize <= 2048) {
        emitInstruction("addi sp, sp, " + std::to_string(frameSize));
    } else {
        emitInstruction("li t0, " + std::to_string(frameSize));
        emitInstruction("add sp, sp, t0");
    }
//---------------------修改18------------------------
    // 返回
    emitInstruction("ret");
}

// 加载操作数到寄存器
// 根据操作数类型生成不同的加载指令
/*void CodeGenerator::loadOperand(const std::shared_ptr<Operand>& op, const std::string& reg) {
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
                    //emitInstruction("mv " + reg + ", " + it->second);     //伪指令
                    emitInstruction("addi " + reg + ", " + it->second + ", 0");
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
}*/
//-----------------------修改18------------------------------
void CodeGenerator::loadOperand(const std::shared_ptr<Operand>& op, const std::string& reg) {
    switch (op->type) {
        case OperandType::CONSTANT:
            emitInstruction("li " + reg + ", " + std::to_string(op->value));
            break;
            
        case OperandType::VARIABLE:
        case OperandType::TEMP:
            {
                auto it = regAlloc.find(op->name);
                if (it != regAlloc.end() && isValidRegister(it->second)) {
                    emitInstruction("addi " + reg + ", " + it->second + ", 0");
                } else {
                    // 从栈中加载
                    int offset = getOperandOffset(op);
                    if (std::abs(offset) <= 2047) {
                        emitInstruction("lw " + reg + ", " + std::to_string(offset) + "(fp)");
                    } else {
                        // 偏移量太大，需要先计算地址
                        std::string tempReg = (reg != "t0") ? "t0" : "t1";
                        emitInstruction("li " + tempReg + ", " + std::to_string(offset));
                        emitInstruction("add " + tempReg + ", fp, " + tempReg);
                        emitInstruction("lw " + reg + ", 0(" + tempReg + ")");
                    }
                }
            }
            break;
            
        case OperandType::LABEL:
            std::cerr << "警告: 尝试加载标签操作数" << std::endl;
            break;
            
        default:
            std::cerr << "错误: 未知的操作数类型" << std::endl;
            break;
    }
}
//----------------------------修改18-----------------------

// 存储寄存器内容到操作数
// 根据操作数是否分配了寄存器，生成不同的存储指令
/*void CodeGenerator::storeRegister(const std::string& reg, const std::shared_ptr<Operand>& op) {
    if (op->type == OperandType::VARIABLE || op->type == OperandType::TEMP) {
        // 检查是否已分配到寄存器
        auto it = regAlloc.find(op->name);
        if (it != regAlloc.end() && isValidRegister(it->second)) {
            // 如果已分配到寄存器，直接移动
            if (reg != it->second) {
                //emitInstruction("mv " + it->second + ", " + reg);     //伪指令
                emitInstruction("addi " + it->second + ", " + reg + ", 0");
            }
        } else {
            // 否则存储到栈中
            int offset = getOperandOffset(op);
            emitInstruction("sw " + reg + ", " + std::to_string(offset) + "(fp)");
        }
    } else {
        std::cerr << "错误: 无法存储到非变量操作数" << std::endl;
    }
}*/
//-----------------------修改18----------------------------
void CodeGenerator::storeRegister(const std::string& reg, const std::shared_ptr<Operand>& op) {
    if (op->type == OperandType::VARIABLE || op->type == OperandType::TEMP) {
        auto it = regAlloc.find(op->name);
        if (it != regAlloc.end() && isValidRegister(it->second)) {
            if (reg != it->second) {
                emitInstruction("addi " + it->second + ", " + reg + ", 0");
            }
        } else {
            // 存储到栈中
            int offset = getOperandOffset(op);
            if (std::abs(offset) <= 2047) {
                emitInstruction("sw " + reg + ", " + std::to_string(offset) + "(fp)");
            } else {
                // 偏移量太大，需要先计算地址
                std::string tempReg = (reg != "t0") ? "t0" : "t1";
                emitInstruction("li " + tempReg + ", " + std::to_string(offset));
                emitInstruction("add " + tempReg + ", fp, " + tempReg);
                emitInstruction("sw " + reg + ", 0(" + tempReg + ")");
            }
        }
    } else {
        std::cerr << "错误: 无法存储到非变量操作数" << std::endl;
    }
}
//---------------------------修改18--------------------------------

// 获取操作数的栈偏移
// 如果操作数尚未分配栈空间，则分配新的栈空间
int CodeGenerator::getOperandOffset(const std::shared_ptr<Operand>& op) {
    // 添加空指针检查
    if (!op) {
        std::cerr << "错误: 空操作数" << std::endl;
        return 0;
    }

    if (op->type != OperandType::VARIABLE && op->type != OperandType::TEMP) {
        std::cerr << "错误: 只有变量和临时变量有栈偏移" << std::endl;
        return 0;
    }
    
    auto it = localVars.find(op->name);
    if (it != localVars.end()) {
        return it->second;
    }
    
    // 检查是否是函数参数
    /*for (size_t i = 0; i < currentFunctionParams.size(); i++) {
        if (currentFunctionParams[i] == op->name) {
            // 参数变量的偏移量计算
            //int offset = -4 * (i + 1);
            int offset = -12 - i * 4; // 从-12开始，避开ra和fp的位置
            localVars[op->name] = offset;
            return offset;
        }
    }*/
    /*for (size_t i = 0; i < currentFunctionParams.size(); i++) {
        if (currentFunctionParams[i] == op->name) {
            // 参数偏移量 = 寄存器保存区大小 + 参数索引 * 4
            int offset = getCalleeSavedRegsSize() + getCallerSavedRegsSize() + i * 4;
            localVars[op->name] = offset;
            return offset;
        }
    }*/
    for (size_t i = 0; i < currentFunctionParams.size(); i++) {
        if (currentFunctionParams[i] == op->name) {
            // 参数偏移量 = 寄存器保存区大小 + 参数索引 * 4
            int offset = currentStackOffset;
            currentStackOffset -= 4;
            localVars[op->name] = offset;
            return offset;
        }
    }

    // 为变量分配栈空间
    //stackSize += 4;
    //int offset = -stackSize;
    //int offset = -12 - stackSize; // 从-12开始，避开ra和fp的位置
    //localVars[op->name] = offset;
    
    // // 如果栈帧尚未完全初始化，记录可能的最大栈大小
    // if (!frameInitialized) {
    //     frameSize = 8 + stackSize; // 8字节用于保存ra和fp
    // }

    // 分配局部变量空间（从栈顶向下增长）
    //int offset = -getCalleeSavedRegsSize() - getLocalVarsSize() - 4;
    int offset = currentStackOffset;
    currentStackOffset -= 4;
    localVars[op->name] = offset;
    incrementLocalVarsSize(4);  // 更新局部变量区大小
    
    return offset;
}

// 分配临时寄存器
// 从临时寄存器池中轮流分配寄存器
// 没有处理临时寄存器不够的情况
std::string CodeGenerator::allocTempReg() {
    if (nextTempReg >= tempRegs.size()) {
        nextTempReg = 0;
        for(Register& reg : registers) {
            reg.isUsed = false; // 重置所有寄存器的使用状态
        }
    }
    //return tempRegs[nextTempReg++];

    // 修改临时寄存器isUsed状态
    for(Register& reg : registers) {
        if (reg.name == tempRegs[nextTempReg]) {
            if (reg.isUsed) {
                std::cerr << "错误: 临时寄存器 " << reg.name << " 已经被使用" << std::endl;
                return ""; // 返回空字符串表示分配失败
            }
            reg.isUsed = true; // 标记为已使用
            usedCallerSavedRegs.insert(reg.name); // 添加到已使用的调用者保存寄存器集合
            break;
        }
    }
    nextTempReg++;
    return tempRegs[nextTempReg - 1];
}

// 释放临时寄存器
void CodeGenerator::freeTempReg(const std::string& reg) {
    // 简化实现，不做实际释放，只是记录一下
}

//----------------------修改18------------------------------
// 保存调用者保存的寄存器
/*void CodeGenerator::saveCallerSavedRegs() {
    // 保存调用者保存的寄存器
    emitComment("保存调用者保存的寄存器");

    for (const auto& reg : usedCallerSavedRegs) {
        int offset = getRegisterStackOffset(reg); //返回指定寄存器在栈帧中的偏移地址
        emitInstruction("sw " + reg + ", " + std::to_string(offset) + "(fp)");
    }
}

// 恢复调用者保存的寄存器
void CodeGenerator::restoreCallerSavedRegs() {
    // 恢复调用者保存的寄存器
    emitComment("恢复调用者保存的寄存器");

    for (const auto& reg : usedCallerSavedRegs) {
        int offset = getRegisterStackOffset(reg); //返回指定寄存器在栈帧中的偏移地址
        emitInstruction("lw " + reg + ", " + std::to_string(offset) + "(fp)");
    }
}

// 保存被调用者保存的寄存器
void CodeGenerator::saveCalleeSavedRegs() {
    // 实现被调用者保存的寄存器保存，如果需要的话
    emitComment("保存被调用者保存的寄存器");
    
    for (const auto& reg : usedCalleeSavedRegs) {
        int offset = getRegisterStackOffset(reg); //返回指定寄存器在栈帧中的偏移地址
        emitInstruction("sw " + reg + ", " + std::to_string(offset) + "(fp)");
    }
    
}

// 恢复被调用者保存的寄存器
void CodeGenerator::restoreCalleeSavedRegs() {
    // 实现被调用者保存的寄存器恢复，如果需要的话
     emitComment("恢复被调用者保存的寄存器");

    for (const auto& reg : usedCalleeSavedRegs) {
        int offset = getRegisterStackOffset(reg); //返回指定寄存器在栈帧中的偏移地址
        emitInstruction("lw " + reg + ", " + std::to_string(offset) + "(fp)");
    }
    
}*/
// 保存调用者保存的寄存器
void CodeGenerator::saveCallerSavedRegs() {
    emitComment("保存调用者保存的寄存器");
    
    for (const auto& reg : usedCallerSavedRegs) {
        int offset = getRegisterStackOffset(reg);
        if (std::abs(offset) <= 2047) {
            emitInstruction("sw " + reg + ", " + std::to_string(offset) + "(fp)");
        } else {
            emitInstruction("li t0, " + std::to_string(offset));
            emitInstruction("add t0, fp, t0");
            emitInstruction("sw " + reg + ", 0(t0)");
        }
    }
}

// 恢复调用者保存的寄存器
void CodeGenerator::restoreCallerSavedRegs() {
    emitComment("恢复调用者保存的寄存器");
    
    for (const auto& reg : usedCallerSavedRegs) {
        int offset = getRegisterStackOffset(reg);
        if (std::abs(offset) <= 2047) {
            emitInstruction("lw " + reg + ", " + std::to_string(offset) + "(fp)");
        } else {
            emitInstruction("li t0, " + std::to_string(offset));
            emitInstruction("add t0, fp, t0");
            emitInstruction("lw " + reg + ", 0(t0)");
        }
    }
}

// 保存被调用者保存的寄存器
void CodeGenerator::saveCalleeSavedRegs() {
    emitComment("保存被调用者保存的寄存器");
    
    for (const auto& reg : usedCalleeSavedRegs) {
        int offset = getRegisterStackOffset(reg);
        if (std::abs(offset) <= 2047) {
            emitInstruction("sw " + reg + ", " + std::to_string(offset) + "(fp)");
        } else {
            emitInstruction("li t0, " + std::to_string(offset));
            emitInstruction("add t0, fp, t0");
            emitInstruction("sw " + reg + ", 0(t0)");
        }
    }
}

// 恢复被调用者保存的寄存器
void CodeGenerator::restoreCalleeSavedRegs() {
    emitComment("恢复被调用者保存的寄存器");
    
    for (const auto& reg : usedCalleeSavedRegs) {
        int offset = getRegisterStackOffset(reg);
        if (std::abs(offset) <= 2047) {
            emitInstruction("lw " + reg + ", " + std::to_string(offset) + "(fp)");
        } else {
            emitInstruction("li t0, " + std::to_string(offset));
            emitInstruction("add t0, fp, t0");
            emitInstruction("lw " + reg + ", 0(t0)");
        }
    }
}


//----------------------修改18------------------------------

// 初始化寄存器信息
void CodeGenerator::initializeRegisters() {
    // 初始化RISC-V寄存器信息
    registers = {
        // 零寄存器
        {"zero", false, false, false, true, "常量0", false},
        
        // 临时寄存器
        {"t0", true, false, true, false, "临时寄存器0", false},
        {"t1", true, false, true, false, "临时寄存器1", false},
        {"t2", true, false, true, false, "临时寄存器2", false},
        {"t3", true, false, true, false, "临时寄存器3", false},
        {"t4", true, false, true, false, "临时寄存器4", false},
        {"t5", true, false, true, false, "临时寄存器5", false},
        {"t6", true, false, true, false, "临时寄存器6", false},
        
        // 保存寄存器
        {"s0", false, true, true, false, "保存寄存器0/帧指针", false},
        {"s1", false, true, true, false, "保存寄存器1", false},
        {"s2", false, true, true, false, "保存寄存器2", false},
        {"s3", false, true, true, false, "保存寄存器3", false},
        {"s4", false, true, true, false, "保存寄存器4", false},
        {"s5", false, true, true, false, "保存寄存器5", false},
        {"s6", false, true, true, false, "保存寄存器6", false},
        {"s7", false, true, true, false, "保存寄存器7", false},
        {"s8", false, true, true, false, "保存寄存器8", false},
        {"s9", false, true, true, false, "保存寄存器9", false},
        {"s10", false, true, true, false, "保存寄存器10", false},
        {"s11", false, true, true, false, "保存寄存器11", false},
        
        // 参数/返回值寄存器
        {"a0", true, false, true, false, "参数/返回值寄存器0", false},
        {"a1", true, false, true, false, "参数/返回值寄存器1", false},
        {"a2", true, false, true, false, "参数/返回值寄存器2", false},
        {"a3", true, false, true, false, "参数/返回值寄存器3", false},
        {"a4", true, false, true, false, "参数/返回值寄存器4", false},
        {"a5", true, false, true, false, "参数/返回值寄存器5", false},
        {"a6", true, false, true, false, "参数/返回值寄存器6", false},
        {"a7", true, false, true, false, "参数/返回值寄存器7", false},
        
        // 特殊寄存器
        {"ra", true, false, false, true, "返回地址", false},
        {"sp", false, false, false, true, "栈指针", false},
        {"gp", false, false, false, true, "全局指针", false},
        {"tp", false, false, false, true, "线程指针", false},
        {"fp", false, true, false, true, "帧指针/s0", false}
    };
}

void CodeGenerator::resetStackOffset() {
    regOffsetMap.clear();
    currentStackOffset = -12;  // 从 fp-12 开始分配（跳过保存的fp和ra）
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

//分析被调用者保存的寄存器
void CodeGenerator::analyzeUsedCalleeSavedRegs() {
    usedCalleeSavedRegs.clear();

    const std::vector<std::string> calleeSavedRegs = {
        "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11"
    };

    /*for (const auto& instr : instructions) {
        std::string s = instr->toString();
        for (const auto& reg : calleeSavedRegs) {
            if (s.find(reg) != std::string::npos) {
                usedCalleeSavedRegs.insert(reg);
            }
        }
    }*/

    // 遍历所有寄存器，检查是否被标记为已使用   
   for(Register& reg : registers) {
        if (reg.isCalleeSaved && reg.isUsed) {
            usedCalleeSavedRegs.insert(reg.name);
        }
    }
}

// 根据IR分析被调用者保存寄存器的数量
int CodeGenerator::countUsedCalleeSavedRegs() {
    //usedCalleeSavedRegs.clear();
    int count = 0;

    const std::vector<std::string> calleeSavedRegs = {
        "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11"
    };

    for (const auto& instr : instructions) {
        std::string s = instr->toString();
        for (const auto& reg : calleeSavedRegs) {
            if (s.find(reg) != std::string::npos) {
                count++;
            }
        }
    }
    return count;
}

//分析调用者保存的寄存器
void CodeGenerator::analyzeUsedCallerSavedRegs() {
    usedCallerSavedRegs.clear();

    const std::vector<std::string> callerSavedRegs = {
        "t0", "t1", "t2", "t3", "t4", "t5", "t6",  // 临时寄存器
        "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", // 参数寄存器
        "ra"   // 返回地址
    };

    /*for (const auto& instr : instructions) {
        std::string s = instr->toString();
        for (const auto& reg : callerSavedRegs) {
            if (s.find(reg) != std::string::npos) {
                usedCallerSavedRegs.insert(reg);
            }
        }
    }*/

    // 遍历所有寄存器，检查是否被标记为已使用
    for(Register& reg : registers) {
        if (reg.isCallerSaved && reg.isUsed) {
            usedCallerSavedRegs.insert(reg.name);
        }
    }
}

// 根据IR分析调用者保存寄存器的数量
int CodeGenerator::countUsedCallerSavedRegs() {
    //usedCallerSavedRegs.clear();
    int count = 0;

    const std::vector<std::string> callerSavedRegs = {
        "t0", "t1", "t2", "t3", "t4", "t5", "t6",  // 临时寄存器
        "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", // 参数寄存器
        "ra"   // 返回地址
    };

    for (const auto& instr : instructions) {
        std::string s = instr->toString();
        for (const auto& reg : callerSavedRegs) {
            if (s.find(reg) != std::string::npos) {
                count++;
            }
        }
    }
    return count;
}

int CodeGenerator::getRegisterStackOffset(const std::string& reg) {
    // 如果寄存器已分配过偏移量，直接返回
    if (regOffsetMap.count(reg)) {
        return regOffsetMap[reg];
    }
        
    // 为新寄存器分配偏移量（按4字节对齐）
    int offset = currentStackOffset;
    regOffsetMap[reg] = offset;
    currentStackOffset -= 4;  // 下一个偏移量
        
    return offset;
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

//遍历指令序列，记录临时变量的生命周期和冲突关系
int CodeGenerator::analyzeTempVars() {
    std::set<std::string> activeTemps;  // 当前活跃的临时变量
    int maxTempSize = 0;
    
    for (const auto& instr : instructions) {
        // 步骤1: 分析指令的def/use（定义和使用）
        //auto defRegs = getDefRegisters(instr);
        auto defRegs = instr -> getDefRegisters();  // 指令定义的寄存器
        //auto useRegs = getUseRegisters(instr); 
        auto useRegs = instr -> getUseRegisters(); // 指令使用的寄存器
        
        // 步骤2: 释放已失效的临时变量
        for (const auto& reg : defRegs) {
            if (isTempReg(reg)) {
                activeTemps.erase(reg);  // 定义新值时旧值失效
            }
        }
        
        // 步骤3: 记录需要栈存储的临时变量
        for (const auto& reg : useRegs) {
            if (isTempReg(reg) && !isRegisterAllocated(reg)) {
                activeTemps.insert(reg);  // 需要栈存储的临时变量
            }
        }
        
        // 步骤4: 更新峰值空间需求
        maxTempSize = std::max(maxTempSize, (int)activeTemps.size() * 4);
    }
    
    return maxTempSize;
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
