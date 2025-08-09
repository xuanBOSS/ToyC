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
    // 初始化循环变量寄存器映射
    loopVarToRegMap["i"] = "s1";
    loopVarToRegMap["j"] = "s2";
    loopVarToRegMap["k"] = "s3";
    
    // 添加常用的被调用者保存寄存器到使用集合
    usedCalleeSavedRegs.insert("s1");
    usedCalleeSavedRegs.insert("s2");
    usedCalleeSavedRegs.insert("s3");
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
    emitComment(instr->toString());

    // 获取临时寄存器
    std::string resultReg = allocTempReg();

//------------------------修改性能loop--------------------------
    // 循环优化: 检测是否是循环计数器更新
    bool isLoopCounter = false;
    if (instr->opcode == OpCode::ADD && 
        instr->left->type == OperandType::VARIABLE && 
        instr->right->type == OperandType::CONSTANT &&
        instr->right->value == 1) {
        
        auto it = loopVarToRegMap.find(instr->left->name);
        if (it != loopVarToRegMap.end()) {
            // 这是循环计数器递增操作
            emitInstruction("addi " + it->second + ", " + it->second + ", 1");
            storeRegister(it->second, instr->result);
            freeTempReg(resultReg);
            return;
        }
    }
//------------------------修改性能loop--------------------------
    
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

        // 指令模式匹配优化
        if (instr->opcode == OpCode::ADD) {
            // 针对常量操作数优化
            if (instr->right->type == OperandType::CONSTANT) {
                int value = instr->right->value;
                if (value == 0) {
                    // x + 0 = x
                    emitInstruction("mv " + resultReg + ", " + leftReg);
                } else if (value >= -2048 && value <= 2047) {
                    // 小立即数使用addi
                    emitInstruction("addi " + resultReg + ", " + leftReg + ", " + std::to_string(value));
                } else {
                    // 大立即数需要先加载到寄存器
                    emitInstruction("li " + rightReg + ", " + std::to_string(value));
                    emitInstruction("add " + resultReg + ", " + leftReg + ", " + rightReg);
                }
            } else if (instr->left->type == OperandType::CONSTANT) {
                int value = instr->left->value;
                if (value == 0) {
                    // 0 + x = x
                    emitInstruction("mv " + resultReg + ", " + rightReg);
                } else if (value >= -2048 && value <= 2047) {
                    // 小立即数使用addi
                    emitInstruction("addi " + resultReg + ", " + rightReg + ", " + std::to_string(value));
                } else {
                    // 大立即数需要先加载到寄存器
                    emitInstruction("add " + resultReg + ", " + leftReg + ", " + rightReg);
                }
            } else {
                // 两个非常量操作数
                emitInstruction("add " + resultReg + ", " + leftReg + ", " + rightReg);
            }
        } else if (instr->opcode == OpCode::SUB) {
            // 针对常量操作数优化
            if (instr->right->type == OperandType::CONSTANT) {
                int value = instr->right->value;
                if (value == 0) {
                    // x - 0 = x
                    emitInstruction("mv " + resultReg + ", " + leftReg);
                } else if (-value >= -2048 && -value <= 2047) {
                    // 如果负值在addi范围内，使用addi
                    emitInstruction("addi " + resultReg + ", " + leftReg + ", " + std::to_string(-value));
                } else {
                    // 大立即数需要先加载到寄存器
                    emitInstruction("sub " + resultReg + ", " + leftReg + ", " + rightReg);
                }
            } else if (instr->left->type == OperandType::CONSTANT && instr->left->value == 0) {
                // 0 - x = -x
                emitInstruction("neg " + resultReg + ", " + rightReg);
            } else {
                // 两个非常量操作数
                emitInstruction("sub " + resultReg + ", " + leftReg + ", " + rightReg);
            }
        } else if (instr->opcode == OpCode::MUL) {
            // 乘法优化
            if (instr->right->type == OperandType::CONSTANT) {
                int value = instr->right->value;
                if (value == 0) {
                    // x * 0 = 0
                    emitInstruction("mv " + resultReg + ", zero");
                } else if (value == 1) {
                    // x * 1 = x
                    emitInstruction("mv " + resultReg + ", " + leftReg);
                } else if (value == 2) {
                    // x * 2 = x + x
                    emitInstruction("add " + resultReg + ", " + leftReg + ", " + leftReg);
                } else if (isPowerOfTwo(value)) {
                    // x * 2^n = x << n
                    int shift = log2(value);
                    emitInstruction("slli " + resultReg + ", " + leftReg + ", " + std::to_string(shift));
                } else {
                    // 一般乘法
                    emitInstruction("mul " + resultReg + ", " + leftReg + ", " + rightReg);
                }
            } else if (instr->left->type == OperandType::CONSTANT) {
                // 交换操作数，左操作数为变量
                int value = instr->left->value;
                if (value == 0) {
                    // 0 * x = 0
                    emitInstruction("mv " + resultReg + ", zero");
                } else if (value == 1) {
                    // 1 * x = x
                    emitInstruction("mv " + resultReg + ", " + rightReg);
                } else if (value == 2) {
                    // 2 * x = x + x
                    emitInstruction("add " + resultReg + ", " + rightReg + ", " + rightReg);
                } else if (isPowerOfTwo(value)) {
                    // 2^n * x = x << n
                    int shift = log2(value);
                    emitInstruction("slli " + resultReg + ", " + rightReg + ", " + std::to_string(shift));
                } else {
                    // 一般乘法
                    emitInstruction("mul " + resultReg + ", " + leftReg + ", " + rightReg);
                }
            } else {
                // 两个非常量操作数
                emitInstruction("mul " + resultReg + ", " + leftReg + ", " + rightReg);
            }
        } else if (instr->opcode == OpCode::DIV) {
            // 除法优化
            if (instr->right->type == OperandType::CONSTANT) {
                int value = instr->right->value;
                if (value == 1) {
                    // x / 1 = x
                    emitInstruction("mv " + resultReg + ", " + leftReg);
                } else if (value == 2) {
                    // 整数除以2可以用右移一位实现
                    emitInstruction("srai " + resultReg + ", " + leftReg + ", 1");
                } else if (isPowerOfTwo(value)) {
                    // x / 2^n = x >> n (针对无符号除法)
                    // 或者使用更复杂的带符号除法优化
                    int shift = log2(value);
                    emitInstruction("srai " + resultReg + ", " + leftReg + ", " + std::to_string(shift));
                } else {
                    // 一般除法
                    emitInstruction("div " + resultReg + ", " + leftReg + ", " + rightReg);
                }
            } else if (instr->left->type == OperandType::CONSTANT && instr->left->value == 0) {
                // 0 / x = 0
                emitInstruction("mv " + resultReg + ", zero");
            } else {
                // 两个非常量操作数
                emitInstruction("div " + resultReg + ", " + leftReg + ", " + rightReg);
            }
        } else if (instr->opcode == OpCode::MOD) {
            // 取模运算优化
            if (instr->right->type == OperandType::CONSTANT) {
                int value = instr->right->value;
                if (isPowerOfTwo(value)) {
                    // x % 2^n = x & (2^n - 1)
                    int mask = value - 1;
                    emitInstruction("andi " + resultReg + ", " + leftReg + ", " + std::to_string(mask));
                } else {
                    // 一般取模
                    emitInstruction("rem " + resultReg + ", " + leftReg + ", " + rightReg);
                }
            } else {
                // 两个非常量操作数
                emitInstruction("rem " + resultReg + ", " + leftReg + ", " + rightReg);
            }
        } else if (instr->opcode == OpCode::LT) {
            // 小于比较优化
            if (instr->right->type == OperandType::CONSTANT && instr->right->value == 0) {
                // x < 0 等价于检查符号位
                emitInstruction("slt " + resultReg + ", " + leftReg + ", zero");
            } else {
                emitInstruction("slt " + resultReg + ", " + leftReg + ", " + rightReg);
            }
        } else if (instr->opcode == OpCode::GT) {
            // 大于比较优化
            if (instr->right->type == OperandType::CONSTANT && instr->right->value == 0) {
                // x > 0 等价于 0 < x
                emitInstruction("slt " + resultReg + ", zero, " + leftReg);
            } else {
                emitInstruction("slt " + resultReg + ", " + rightReg + ", " + leftReg);
            }
        } else if (instr->opcode == OpCode::LE) {
            // 小于等于优化
            if (instr->right->type == OperandType::CONSTANT && instr->right->value == 0) {
                // x <= 0 等价于 !(0 < x)
                emitInstruction("slt " + resultReg + ", zero, " + leftReg);
                emitInstruction("xori " + resultReg + ", " + resultReg + ", 1");
            } else {
                // x <= y 等价于 !(y < x)
                emitInstruction("slt " + resultReg + ", " + rightReg + ", " + leftReg);
                emitInstruction("xori " + resultReg + ", " + resultReg + ", 1");
            }
        } else if (instr->opcode == OpCode::GE) {
            // 大于等于优化
            if (instr->right->type == OperandType::CONSTANT && instr->right->value == 0) {
                // x >= 0 等价于 !(x < 0)
                emitInstruction("slt " + resultReg + ", " + leftReg + ", zero");
                emitInstruction("xori " + resultReg + ", " + resultReg + ", 1");
            } else {
                // x >= y 等价于 !(x < y)
                emitInstruction("slt " + resultReg + ", " + leftReg + ", " + rightReg);
                emitInstruction("xori " + resultReg + ", " + resultReg + ", 1");
            }
        } else if (instr->opcode == OpCode::EQ) {
            // 等于比较优化
            if (instr->right->type == OperandType::CONSTANT && instr->right->value == 0) {
                // x == 0
                emitInstruction("seqz " + resultReg + ", " + leftReg);
            } else if (instr->left->type == OperandType::CONSTANT && instr->left->value == 0) {
                // 0 == x
                emitInstruction("seqz " + resultReg + ", " + rightReg);
            } else {
                // 使用异或和比较0实现相等比较
                emitInstruction("xor " + resultReg + ", " + leftReg + ", " + rightReg);
                emitInstruction("seqz " + resultReg + ", " + resultReg);
            }
        } else if (instr->opcode == OpCode::NE) {
            // 不等于比较优化
            if (instr->right->type == OperandType::CONSTANT && instr->right->value == 0) {
                // x != 0
                emitInstruction("snez " + resultReg + ", " + leftReg);
            } else if (instr->left->type == OperandType::CONSTANT && instr->left->value == 0) {
                // 0 != x
                emitInstruction("snez " + resultReg + ", " + rightReg);
            } else {
                // 使用异或实现不等比较
                emitInstruction("xor " + resultReg + ", " + leftReg + ", " + rightReg);
                emitInstruction("snez " + resultReg + ", " + resultReg);
            }
        } else {
            // 其他二元操作，使用默认实现
            switch (instr->opcode) {
                case OpCode::AND:
                    emitInstruction("and " + resultReg + ", " + leftReg + ", " + rightReg);
                    break;
                case OpCode::OR:
                    emitInstruction("or " + resultReg + ", " + leftReg + ", " + rightReg);
                    break;
                default:
                    std::cerr << "错误: 未知的二元操作" << std::endl;
                    break;
            }
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
// 辅助函数：检查一个数是否是2的幂
bool CodeGenerator::isPowerOfTwo(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

// 辅助函数：计算log2(n)，假设n是2的幂
int CodeGenerator::log2(int n) {
    int result = 0;
    while (n > 1) {
        n >>= 1;
        result++;
    }
    return result;
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
    
    // 指令选择优化
    switch (instr->opcode) {
        case OpCode::NEG:
            if (instr->operand->type == OperandType::CONSTANT) {
                // 常量取负优化
                emitInstruction("li " + resultReg + ", " + std::to_string(-instr->operand->value));
            } else {
                // 使用neg指令
                emitInstruction("neg " + resultReg + ", " + operandReg);
            }
            break;
            
        case OpCode::NOT:
            if (instr->operand->type == OperandType::CONSTANT) {
                // 常量逻辑非优化
                bool value = (instr->operand->value != 0);
                emitInstruction("li " + resultReg + ", " + std::to_string(!value));
            } else {
                // 使用seqz实现逻辑非
                emitInstruction("seqz " + resultReg + ", " + operandReg);
            }
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
    
    // 如果源和目标都分配了相同的寄存器，跳过
    auto srcIt = regAlloc.find(instr->source->name);
    auto dstIt = regAlloc.find(instr->target->name);
    if (srcIt != regAlloc.end() && dstIt != regAlloc.end() && 
        srcIt->second == dstIt->second) {
        // 源和目标使用相同寄存器，无需生成指令
        return;
    }
    
    // 针对源是常量的优化
    if (instr->source->type == OperandType::CONSTANT) {
        int value = instr->source->value;
        
        // 目标已分配寄存器
        if (dstIt != regAlloc.end()) {
            std::string dstReg = dstIt->second;
            
            // 优化常见常量的加载
            if (value == 0) {
                emitInstruction("mv " + dstReg + ", zero");
            } else if (value >= -2048 && value <= 2047) {
                emitInstruction("addi " + dstReg + ", zero, " + std::to_string(value));
            } else {
                emitInstruction("li " + dstReg + ", " + std::to_string(value));
            }
        } else {
            // 目标在栈上，需要先加载到临时寄存器
            std::string tempReg = allocTempReg();
            
            // 优化常见常量的加载
            if (value == 0) {
                emitInstruction("mv " + tempReg + ", zero");
            } else if (value >= -2048 && value <= 2047) {
                emitInstruction("addi " + tempReg + ", zero, " + std::to_string(value));
            } else {
                emitInstruction("li " + tempReg + ", " + std::to_string(value));
            }
            
            // 存储到目标
            storeRegister(tempReg, instr->target);
            freeTempReg(tempReg);
            return;
        }
    } else {
        // 源不是常量，使用常规处理
        std::string reg = allocTempReg();
        
        // 加载源操作数
        loadOperand(instr->source, reg);
        
        // 存储到目标操作数
        storeRegister(reg, instr->target);
        
        // 释放临时寄存器
        freeTempReg(reg);
    }
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
    
    // 针对常量条件优化
    if (instr->condition->type == OperandType::CONSTANT) {
        if (instr->condition->value != 0) {
            // 条件为真，生成无条件跳转
            emitInstruction("j " + instr->target->name);
        } else {
            // 条件为假，不生成跳转
        }
        return;
    }

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
    

    // 1. 分析需要保存的寄存器
    analyzeUsedCallerSavedRegs();  // 确定哪些调用者寄存器需要保存
    analyzeUsedCalleeSavedRegs();  // 确定哪些被调用者寄存器需要保存


    // 2. 保存调用者寄存器（Caller-Saved）
    saveCallerSavedRegs();

    // 3. 传递参数
    // 3.1 寄存器参数（a0-a7）
    for (int i = 0; i < std::min(8, paramCount); ++i) {
        if (!params[i]) continue;
        loadOperand(params[i], "a" + std::to_string(i));
    }

    // 3.2 栈参数
    // 越靠近栈顶的参数索引越小
    int stackParamOffset = 0;
    for (int i = 8; i < paramCount; ++i) {
        if (!params[i]) continue;
        std::string tempReg = allocTempReg();
        loadOperand(params[i], tempReg);
        emitInstruction("sw " + tempReg + ", " + std::to_string(stackParamOffset) + "(sp)");
        stackParamOffset += 4;
        freeTempReg(tempReg);
    }

    // 4. 调用函数
    emitInstruction("call " + instr->funcName);

    // 5. 恢复调用者寄存器
    restoreCallerSavedRegs();

    // 6. 处理返回值
    if (instr->result) {
        std::string resultReg = allocTempReg();
        //emitInstruction("mv " + resultReg + ", a0");      
        emitInstruction("addi " + resultReg + ", a0" + ", 0");
        storeRegister(resultReg, instr->result);
        freeTempReg(resultReg);
    }

    // 7. 清除已使用的参数
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
    //stackSize = 0;
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
}

// 生成函数序言
// 设置栈帧，保存返回地址和帧指针
void CodeGenerator::emitPrologue(const std::string& funcName) {
    // emitComment("函数序言");
    
    // //重置栈帧偏移量
    // resetStackOffset();

    // // 1. 计算各区域大小
    // calleeRegsSize = countUsedCalleeSavedRegs() * 4;  // 被调用者保存寄存器空间
    // callerRegsSize = countUsedCallerSavedRegs() * 4;  // 调用者保存寄存器空间
    // int localsAndPadding = analyzeTempVars();  // 分析临时变量需求
    // int totalFrameSize = calleeRegsSize + callerRegsSize + localsAndPadding + 8; // +8 for ra/fp
    // totalFrameSize = (totalFrameSize + 15) & ~15;      // 最终16字节对齐
    // frameSize = totalFrameSize; // 更新当前帧大小

    // // 2. 分配栈空间
    // // 修改这部分：
    // if (totalFrameSize <= 2048) {
    //     emitInstruction("addi sp, sp, -" + std::to_string(totalFrameSize));
    // } else {
    //     // 栈帧太大，需要使用多条指令
    //     emitInstruction("li t0, -" + std::to_string(totalFrameSize));
    //     emitInstruction("add sp, sp, t0");
    // }

    // // 3. 保存关键寄存器
    // // 修改这部分：
    // if (totalFrameSize - 4 <= 2047) {
    //     emitInstruction("sw ra, " + std::to_string(totalFrameSize - 4) + "(sp)");
    // } else {
    //     emitInstruction("li t0, " + std::to_string(totalFrameSize - 4));
    //     emitInstruction("add t0, sp, t0");
    //     emitInstruction("sw ra, 0(t0)");
    // }
    
    // if (totalFrameSize - 8 <= 2047) {
    //     emitInstruction("sw fp, " + std::to_string(totalFrameSize - 8) + "(sp)");
    // } else {
    //     emitInstruction("li t0, " + std::to_string(totalFrameSize - 8));
    //     emitInstruction("add t0, sp, t0");
    //     emitInstruction("sw fp, 0(t0)");
    // }

    // // 4. 设置新帧指针（指向旧sp）
    // if (totalFrameSize <= 2048) {
    //     emitInstruction("addi fp, sp, " + std::to_string(totalFrameSize));
    // } else {
    //     emitInstruction("li t0, " + std::to_string(totalFrameSize));
    //     emitInstruction("add fp, sp, t0");
    // }

    // // 5. 保存被调用者寄存器
    // saveCalleeSavedRegs();

    // // 更新状态
    // frameInitialized = true;
    // this->frameSize = totalFrameSize;
//---------------------修改性能loop----------------------------
    emitComment("函数序言");
    
    // 重置栈帧偏移量
    resetStackOffset();
    
    // 检测是否包含循环
    bool isLoopIntensive = detectIntensiveLoops();
    
    // 计算栈帧大小
    calleeRegsSize = countUsedCalleeSavedRegs() * 4;
    callerRegsSize = countUsedCallerSavedRegs() * 4;
    
    // 分析临时变量
    int localsSize = analyzeTempVars();
    
    // 为循环密集型代码额外分配空间，但限制最大大小
    if (isLoopIntensive) {
        localsSize += 64; // 额外预留64字节
    }
    
    // 计算总栈帧大小并16字节对齐
    int totalFrameSize = calleeRegsSize + callerRegsSize + localsSize + 8; // +8 for ra/fp
    totalFrameSize = (totalFrameSize + 15) & ~15;
    frameSize = totalFrameSize;
    
    // 分配栈空间 - 使用稳定实现
    if (totalFrameSize <= 2048) {
        emitInstruction("addi sp, sp, -" + std::to_string(totalFrameSize));
    } else {
        // 大栈帧需要分步计算
        emitInstruction("li t0, " + std::to_string(totalFrameSize));
        emitInstruction("sub sp, sp, t0");
    }
    
    // 保存返回地址和帧指针
    if (totalFrameSize - 4 <= 2047) {
        emitInstruction("sw ra, " + std::to_string(totalFrameSize - 4) + "(sp)");
    } else {
        // 大偏移量
        emitInstruction("li t0, " + std::to_string(totalFrameSize - 4));
        emitInstruction("add t0, sp, t0");
        emitInstruction("sw ra, 0(t0)");
    }
    
    if (totalFrameSize - 8 <= 2047) {
        emitInstruction("sw fp, " + std::to_string(totalFrameSize - 8) + "(sp)");
    } else {
        // 大偏移量
        emitInstruction("li t0, " + std::to_string(totalFrameSize - 8));
        emitInstruction("add t0, sp, t0");
        emitInstruction("sw fp, 0(t0)");
    }
    
    // 设置新帧指针
    if (totalFrameSize <= 2048) {
        emitInstruction("addi fp, sp, " + std::to_string(totalFrameSize));
    } else {
        emitInstruction("li t0, " + std::to_string(totalFrameSize));
        emitInstruction("add fp, sp, t0");
    }
    
    // 为循环密集型代码特殊优化
    if (isLoopIntensive) {
        optimizeForLoops();
    }
    
    // 保存被调用者寄存器
    saveCalleeSavedRegs();
    
    // 更新状态
    frameInitialized = true;
//---------------------修改性能loop----------------------------
}

//---------------------修改性能loop-------------------------
bool CodeGenerator::detectIntensiveLoops() {
    int loopVarCount = 0;
    int branchCount = 0;
    int arrayAccessCount = 0;
    
    for (const auto& instr : instructions) {
        std::string instrStr = instr->toString();
        
        // 计数循环变量
        if ((instrStr.find(" i ") != std::string::npos || 
            instrStr.find(" j ") != std::string::npos || 
            instrStr.find(" k ") != std::string::npos) &&
            (instrStr.find("=") != std::string::npos)) {
            loopVarCount++;
        }
        
        // 计数条件分支
        if (instrStr.find("if_goto") != std::string::npos) {
            branchCount++;
        }
        
        // 计数数组访问
        if (instrStr.find("[") != std::string::npos && instrStr.find("]") != std::string::npos) {
            arrayAccessCount++;
        }
    }
    
    // 返回更精确的循环密集型代码检测
    return (loopVarCount > 3 && branchCount > 5) || arrayAccessCount > 10;
    // // 简单启发式：检查是否包含 i, j, k 变量和多个比较指令
    // int loopVarCount = 0;
    // int branchCount = 0;
    
    // for (const auto& instr : instructions) {
    //     std::string instrStr = instr->toString();
        
    //     // 计数循环变量
    //     if (instrStr.find(" i ") != std::string::npos || 
    //         instrStr.find(" j ") != std::string::npos || 
    //         instrStr.find(" k ") != std::string::npos) {
    //         loopVarCount++;
    //     }
        
    //     // 计数条件分支
    //     if (instrStr.find("if_goto") != std::string::npos) {
    //         branchCount++;
    //     }
    // }
    
    // // 如果循环变量和分支指令都较多，可能是循环密集型代码
    // return (loopVarCount > 5 && branchCount > 10);
}

// 添加 optimizeForLoops 函数，为循环密集代码做优化
void CodeGenerator::optimizeForLoops() {
    emitComment("启用循环优化");
    
    // 为常见循环变量预先分配寄存器 - 这部分很关键
    loopVarToRegMap.clear(); // 确保清空旧映射
    loopVarToRegMap["i"] = "s1";
    loopVarToRegMap["j"] = "s2"; 
    loopVarToRegMap["k"] = "s3";
    
    // 把这些寄存器也加入被调用者保存寄存器列表
    usedCalleeSavedRegs.insert("s1");
    usedCalleeSavedRegs.insert("s2");
    usedCalleeSavedRegs.insert("s3");
    
    // 保存这些寄存器的状态到栈
    int s1Offset = getRegisterStackOffset("s1");
    int s2Offset = getRegisterStackOffset("s2");
    int s3Offset = getRegisterStackOffset("s3");
    
    emitInstruction("sw s1, " + std::to_string(s1Offset) + "(fp)");
    emitInstruction("sw s2, " + std::to_string(s2Offset) + "(fp)");
    emitInstruction("sw s3, " + std::to_string(s3Offset) + "(fp)");
    
    // 初始化循环寄存器为0
    emitInstruction("mv s1, zero");
    emitInstruction("mv s2, zero");
    emitInstruction("mv s3, zero");
}
//---------------------修改性能loop-------------------------


// 生成函数后记
// 恢复保存的寄存器，恢复栈帧，返回
void CodeGenerator::emitEpilogue(const std::string& funcName) {
    emitComment("函数后记");
    
    // 恢复被调用者保存的寄存器
    restoreCalleeSavedRegs();
//---------------------修改18------------------------
    
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


//-----------------------修改18------------------------------
void CodeGenerator::loadOperand(const std::shared_ptr<Operand>& op, const std::string& reg) {
//----------------------修改性能loop-------------------------    
    if (!op) return;
    
    // 保留专用循环寄存器逻辑，但避免过度优化
    if (op->type == OperandType::VARIABLE) {
        auto it = loopVarToRegMap.find(op->name);
        if (it != loopVarToRegMap.end()) {
            // 如果目标寄存器与循环变量寄存器不同，使用mv指令
            if (reg != it->second) {
                emitInstruction("mv " + reg + ", " + it->second);
            }
            return;
        }
    }

    switch (op->type) {
        case OperandType::CONSTANT:
            // 针对特定常量进行优化 - 特别是0和1这些频繁出现的值
            if (op->value == 0) {
                emitInstruction("mv " + reg + ", zero");
            } else if (op->value == 1) {
                emitInstruction("li " + reg + ", 1");
            } else if (op->value == -1) {
                emitInstruction("li " + reg + ", -1");
            } else if (op->value >= -2048 && op->value <= 2047) {
                // 小立即数使用addi
                emitInstruction("addi " + reg + ", zero, " + std::to_string(op->value));
            } else {
                // 大立即数使用li
                emitInstruction("li " + reg + ", " + std::to_string(op->value));
            }
            break;
            
        case OperandType::VARIABLE:
        case OperandType::TEMP:
            {
                auto it = regAlloc.find(op->name);
                if (it != regAlloc.end() && isValidRegister(it->second)) {
                    // 避免不必要的移动
                    if (reg != it->second) {
                        emitInstruction("mv " + reg + ", " + it->second);
                    }
                } else {
                    // 从栈中加载
                    int offset = getOperandOffset(op);
                    
                    // 添加偏移量范围检查
                    if (offset <= 2047 && offset >= -2048) {
                        emitInstruction("lw " + reg + ", " + std::to_string(offset) + "(fp)");
                    } else {
                        // 大偏移量需要分步计算
                        std::string tempReg = (reg != "t0") ? "t0" : "t1";
                        emitInstruction("li " + tempReg + ", " + std::to_string(offset));
                        emitInstruction("add " + tempReg + ", fp, " + tempReg);
                        emitInstruction("lw " + reg + ", 0(" + tempReg + ")");
                    }
                }
            }
            break;
            
        default:
            // 忽略标签和其他类型
            break;
    }
    
    // switch (op->type) {
    //     case OperandType::CONSTANT:
    //         emitInstruction("li " + reg + ", " + std::to_string(op->value));
    //         break;
            
    //     case OperandType::VARIABLE:
    //     case OperandType::TEMP:
    //         {
    //             auto it = regAlloc.find(op->name);
    //             if (it != regAlloc.end() && isValidRegister(it->second)) {
    //                 emitInstruction("addi " + reg + ", " + it->second + ", 0");
    //             } else {
    //                 // 从栈中加载
    //                 int offset = getOperandOffset(op);
    //                 if (std::abs(offset) <= 2047) {
    //                     emitInstruction("lw " + reg + ", " + std::to_string(offset) + "(fp)");
    //                 } else {
    //                     // 偏移量太大，需要先计算地址
    //                     std::string tempReg = (reg != "t0") ? "t0" : "t1";
    //                     emitInstruction("li " + tempReg + ", " + std::to_string(offset));
    //                     emitInstruction("add " + tempReg + ", fp, " + tempReg);
    //                     emitInstruction("lw " + reg + ", 0(" + tempReg + ")");
    //                 }
    //             }
    //         }
    //         break;
            
    //     case OperandType::LABEL:
    //         std::cerr << "警告: 尝试加载标签操作数" << std::endl;
    //         break;
            
    //     default:
    //         std::cerr << "错误: 未知的操作数类型" << std::endl;
    //         break;
    // }
//----------------------修改性能loop-------------------------

}

//-----------------------修改18----------------------------
void CodeGenerator::storeRegister(const std::string& reg, const std::shared_ptr<Operand>& op) {
//----------------------修改性能loop-------------------------    
    if (!op) return;
    
    // 专用循环寄存器处理
    if (op->type == OperandType::VARIABLE) {
        auto it = loopVarToRegMap.find(op->name);
        if (it != loopVarToRegMap.end()) {
            // 更新专用循环寄存器
            if (reg != it->second) {
                emitInstruction("mv " + it->second + ", " + reg);
            }
            return;
        }
    }

    if (op->type == OperandType::VARIABLE || op->type == OperandType::TEMP) {
        auto it = regAlloc.find(op->name);
        if (it != regAlloc.end() && isValidRegister(it->second)) {
            // 避免不必要的移动
            if (reg != it->second) {
                emitInstruction("mv " + it->second + ", " + reg);
            }
        } else {
            // 存储到栈
            int offset = getOperandOffset(op);
            
            // 检查偏移量范围
            if (offset <= 2047 && offset >= -2048) {
                emitInstruction("sw " + reg + ", " + std::to_string(offset) + "(fp)");
            } else {
                // 大偏移量需要分步计算
                std::string tempReg = (reg != "t0") ? "t0" : "t1";
                emitInstruction("li " + tempReg + ", " + std::to_string(offset));
                emitInstruction("add " + tempReg + ", fp, " + tempReg);
                emitInstruction("sw " + reg + ", 0(" + tempReg + ")");
            }
        }
    }
    // if (op->type == OperandType::VARIABLE || op->type == OperandType::TEMP) {
    //     auto it = regAlloc.find(op->name);
    //     if (it != regAlloc.end() && isValidRegister(it->second)) {
    //         if (reg != it->second) {
    //             emitInstruction("addi " + it->second + ", " + reg + ", 0");
    //         }
    //     } else {
    //         // 存储到栈中
    //         int offset = getOperandOffset(op);
    //         if (std::abs(offset) <= 2047) {
    //             emitInstruction("sw " + reg + ", " + std::to_string(offset) + "(fp)");
    //         } else {
    //             // 偏移量太大，需要先计算地址
    //             std::string tempReg = (reg != "t0") ? "t0" : "t1";
    //             emitInstruction("li " + tempReg + ", " + std::to_string(offset));
    //             emitInstruction("add " + tempReg + ", fp, " + tempReg);
    //             emitInstruction("sw " + reg + ", 0(" + tempReg + ")");
    //         }
    //     }
    // } else {
    //     std::cerr << "错误: 无法存储到非变量操作数" << std::endl;
    // }
//----------------------修改性能loop-------------------------
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
    for (size_t i = 0; i < currentFunctionParams.size(); i++) {
        if (currentFunctionParams[i] == op->name) {
            // 参数偏移量 = 寄存器保存区大小 + 参数索引 * 4
            int offset = currentStackOffset;
            currentStackOffset -= 4;
            localVars[op->name] = offset;
            return offset;
        }
    }

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
    // // 根据配置选择寄存器分配策略
    // switch (config.regAllocStrategy) {
    //     case RegisterAllocStrategy::LINEAR_SCAN:
    //         linearScanRegisterAllocation();
    //         break;
    //     case RegisterAllocStrategy::GRAPH_COLOR:
    //         graphColoringRegisterAllocation();
    //         break;
    //     default:
    //         // 默认不做任何分配
    //         break;
    // }
    std::vector<Register> allocatableRegs;
    for (const auto& reg : registers) {
        if (reg.isAllocatable && !reg.isReserved) {
            allocatableRegs.push_back(reg);
        }
    }
    
    // 根据配置选择寄存器分配策略
    switch (config.regAllocStrategy) {
        case RegisterAllocStrategy::LINEAR_SCAN: {
            std::cerr << "使用线性扫描寄存器分配" << std::endl;
            LinearScanRegisterAllocator allocator;
            regAlloc = allocator.allocate(instructions, allocatableRegs);
            break;
        }
        case RegisterAllocStrategy::GRAPH_COLOR: {
            std::cerr << "使用图着色寄存器分配" << std::endl;
            GraphColoringRegisterAllocator allocator;
            regAlloc = allocator.allocate(instructions, allocatableRegs);
            break;
        }
        default: {
            std::cerr << "使用朴素寄存器分配" << std::endl;
            NaiveRegisterAllocator allocator;
            regAlloc = allocator.allocate(instructions, allocatableRegs);
            break;
        }
    }
    
    // 为重要的循环变量预分配寄存器
    if (config.optimizeLoops) {
        preAllocateLoopRegisters();
    }
}
// 为常见循环变量预分配寄存器
void CodeGenerator::preAllocateLoopRegisters() {
    // 检测常见循环变量
    std::set<std::string> loopVars;
    for (const auto& instr : instructions) {
        std::string instrStr = instr->toString();
        // 识别循环变量定义和使用
        if (instrStr.find(" i ") != std::string::npos || 
            instrStr.find(" j ") != std::string::npos || 
            instrStr.find(" k ") != std::string::npos) {
            
            auto defined = IRAnalyzer::getDefinedVariables(instr);
            for (const auto& var : defined) {
                if (var == "i" || var == "j" || var == "k") {
                    loopVars.insert(var);
                }
            }
        }
    }
    
    // 为循环变量分配固定寄存器
    if (loopVars.find("i") != loopVars.end()) {
        regAlloc["i"] = "s1";
        usedCalleeSavedRegs.insert("s1");
    }
    if (loopVars.find("j") != loopVars.end()) {
        regAlloc["j"] = "s2";
        usedCalleeSavedRegs.insert("s2");
    }
    if (loopVars.find("k") != loopVars.end()) {
        regAlloc["k"] = "s3";
        usedCalleeSavedRegs.insert("s3");
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
    
    emitComment("栈布局优化结束，新栈大小: " + std::to_string(frameSize));
}

//遍历指令序列，记录临时变量的生命周期和冲突关系
int CodeGenerator::analyzeTempVars() {
    std::set<std::string> activeTemps;  // 当前活跃的临时变量
    int maxTempSize = 0;
    
    for (const auto& instr : instructions) {
        // 步骤1: 分析指令的def/use（定义和使用）
        auto defRegs = instr -> getDefRegisters();  // 指令定义的寄存器
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
                    // 处理其他分支指令
                    else if (branch.find("bne ") == 0) {
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
                            instrs.push_back("bnez " + reg1 + ", " + label);
                            return true;
                        } else if (reg1 == reg) {
                            instrs.clear();
                            instrs.push_back("bnez " + reg2 + ", " + label);
                            return true;
                        }
                    }
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

        // 模式4: 优化连续加载同一内存位置
        addPeepholePattern("consecutive_loads", [](std::vector<std::string>& instrs) -> bool {
            if (instrs.size() < 3) return false;
            
            std::string load1 = instrs[0];
            std::string load2 = instrs[1];
            
            if (load1.find("lw ") == 0 && load2.find("lw ") == 0) {
                std::string reg1 = load1.substr(3, load1.find(",") - 3);
                std::string mem1 = load1.substr(load1.find(",") + 1);
                
                std::string reg2 = load2.substr(3, load2.find(",") - 3);
                std::string mem2 = load2.substr(load2.find(",") + 1);
                
                // 如果两次加载相同位置但到不同寄存器
                if (mem1 == mem2 && reg1 != reg2) {
                    // 替换为加载+移动
                    instrs.erase(instrs.begin() + 1);
                    instrs.push_back("mv " + reg2 + ", " + reg1);
                    return true;
                }
            }
            return false;
        });
        
        // 模式5: 优化加减零
        addPeepholePattern("add_sub_zero", [](std::vector<std::string>& instrs) -> bool {
            if (instrs.size() < 1) return false;
            
            std::string instr = instrs[0];
            if (instr.find("addi ") == 0 || instr.find("subi ") == 0) {
                size_t pos1 = instr.find(",");
                size_t pos2 = instr.find(",", pos1 + 1);
                
                std::string dst = instr.substr(5, pos1 - 5);
                std::string src = instr.substr(pos1 + 1, pos2 - pos1 - 1);
                std::string imm = instr.substr(pos2 + 1);
                
                // 去除前导空格
                src = src.substr(src.find_first_not_of(" \t"));
                imm = imm.substr(imm.find_first_not_of(" \t"));
                
                // 如果立即数是0
                if (imm == "0") {
                    // 如果源和目标不同，替换为mv
                    if (src != dst) {
                        instrs[0] = "mv " + dst + ", " + src;
                    } else {
                        // 如果源和目标相同，删除指令
                        instrs.clear();
                    }
                    return true;
                }
            }
            return false;
        });

        // 模式6: 优化循环计数器处理
        addPeepholePattern("loop_counter_reg", [](std::vector<std::string>& instrs) -> bool {
            if (instrs.size() < 2) return false;
            
            // 检查循环变量初始化和使用模式
            std::string instr1 = instrs[0];
            std::string instr2 = instrs[1];
            
            if ((instr1.find("li ") == 0 || instr1.find("addi ") == 0) && 
                (instr1.find(", 0") != std::string::npos || instr1.find(", 1") != std::string::npos)) {
                
                // 检查是否是循环变量 i, j, k 的初始化
                std::string reg = instr1.substr(instr1.find(" ") + 1, instr1.find(",") - instr1.find(" ") - 1);
                
                if (instr2.find("sw " + reg) == 0 && 
                    (instr2.find("i_") != std::string::npos || 
                     instr2.find("j_") != std::string::npos || 
                     instr2.find("k_") != std::string::npos)) {
                    
                    // 替换为专用寄存器初始化
                    std::string targetReg;
                    if (instr2.find("i_") != std::string::npos) targetReg = "s1";
                    else if (instr2.find("j_") != std::string::npos) targetReg = "s2";
                    else if (instr2.find("k_") != std::string::npos) targetReg = "s3";
                    
                    instrs.clear();
                    if (instr1.find(", 0") != std::string::npos) {
                        instrs.push_back("mv " + targetReg + ", zero");
                    } else {
                        instrs.push_back("addi " + targetReg + ", zero, 1");
                    }
                    return true;
                }
            }
            return false;
        });
        
        // 模式7: 合并连续加减
        addPeepholePattern("merge_addi", [](std::vector<std::string>& instrs) -> bool {
            if (instrs.size() < 2) return false;
            
            std::string instr1 = instrs[0];
            std::string instr2 = instrs[1];
            
            if (instr1.find("addi ") == 0 && instr2.find("addi ") == 0) {
                // 提取第一条指令的寄存器和立即数
                size_t pos1 = instr1.find(",");
                size_t pos2 = instr1.find(",", pos1 + 1);
                
                std::string reg1 = instr1.substr(5, pos1 - 5);
                std::string src1 = instr1.substr(pos1 + 1, pos2 - pos1 - 1);
                src1 = src1.substr(src1.find_first_not_of(" \t"));
                std::string imm1Str = instr1.substr(pos2 + 1);
                imm1Str = imm1Str.substr(imm1Str.find_first_not_of(" \t"));
                int imm1 = std::stoi(imm1Str);
                
                // 提取第二条指令的寄存器和立即数
                pos1 = instr2.find(",");
                pos2 = instr2.find(",", pos1 + 1);
                
                std::string reg2 = instr2.substr(5, pos1 - 5);
                std::string src2 = instr2.substr(pos1 + 1, pos2 - pos1 - 1);
                src2 = src2.substr(src2.find_first_not_of(" \t"));
                std::string imm2Str = instr2.substr(pos2 + 1);
                imm2Str = imm2Str.substr(imm2Str.find_first_not_of(" \t"));
                int imm2 = std::stoi(imm2Str);
                
                // 如果目标寄存器相同，且第二条指令的源是第一条的目标
                if (reg1 == src2 && reg2 == reg1) {
                    // 合并为一条指令
                    int combinedImm = imm1 + imm2;
                    if (combinedImm == 0) {
                        // 两个立即数相互抵消
                        instrs.clear();
                        instrs.push_back("mv " + reg1 + ", " + src1);
                        return true;
                    } else if (combinedImm >= -2048 && combinedImm <= 2047) {
                        instrs.clear();
                        instrs.push_back("addi " + reg1 + ", " + src1 + ", " + std::to_string(combinedImm));
                        return true;
                    }
                }
            }
            return false;
        });
        
        // 模式8: 优化循环边界检查
        addPeepholePattern("loop_bound_check", [](std::vector<std::string>& instrs) -> bool {
            if (instrs.size() < 3) return false;
            
            std::string instr1 = instrs[0];
            std::string instr2 = instrs[1];
            std::string instr3 = instrs[2];
            
            // 检测常见的循环比较模式
            if (instr1.find("lw ") == 0 && 
                (instr2.find("blt ") == 0 || instr2.find("bge ") == 0) && 
                instr3.find("addi ") == 0) {
                
                // 提取寄存器
                std::string loadReg = instr1.substr(3, instr1.find(",") - 3);
                
                // 检查比较的是否是循环计数器 (s1/s2/s3)
                if (instr2.find("s1,") != std::string::npos || 
                    instr2.find("s2,") != std::string::npos || 
                    instr2.find("s3,") != std::string::npos) {
                    
                    // 直接返回false，保持指令不变但让调用者知道这是循环边界检查
                    return false;
                }
            }
            return false;
        });

        // 模式9: 优化循环归纳变量
        addPeepholePattern("loop_induction_var", [](std::vector<std::string>& instrs) -> bool {
            if (instrs.size() < 3) return false;
    
            // 识别模式: 循环归纳变量模式 (i = i + 1)
            std::string instr1 = instrs[0];
            std::string instr2 = instrs[1];
            std::string instr3 = instrs[2];
    
            if (instr1.find("lw ") == 0 && 
                instr2.find("addi ") == 0 && 
                instr3.find("sw ") == 0) {
        
                // 提取寄存器和内存位置
                std::string loadReg = instr1.substr(3, instr1.find(",") - 3);
                std::string loadAddr = instr1.substr(instr1.find(",") + 1);
        
                size_t pos1 = instr2.find(",");
                size_t pos2 = instr2.find(",", pos1 + 1);
                std::string addDest = instr2.substr(5, pos1 - 5);
                std::string addSrc = instr2.substr(pos1 + 1, pos2 - pos1 - 1);
                addSrc = addSrc.substr(addSrc.find_first_not_of(" \t"));
                std::string addImm = instr2.substr(pos2 + 1);
                addImm = addImm.substr(addImm.find_first_not_of(" \t"));
        
                std::string storeReg = instr3.substr(3, instr3.find(",") - 3);
                std::string storeAddr = instr3.substr(instr3.find(",") + 1);
        
                // 检查是否是同一变量自增
                if (loadAddr == storeAddr && 
                    loadReg == addSrc && 
                    addDest == storeReg && 
                    addImm == "1") {
            
                    // 优化为一条加载+自增+存储序列
                    instrs.clear();
                    instrs.push_back("lw " + loadReg + ", " + loadAddr);
                    instrs.push_back("addi " + loadReg + ", " + loadReg + ", 1");
                    instrs.push_back("sw " + loadReg + ", " + storeAddr);
                    return true;
                }
            }
            return false;
        });

        // 模式10: 针对矩阵操作的优化
        addPeepholePattern("matrix_indexing", [](std::vector<std::string>& instrs) -> bool {
            if (instrs.size() < 4) return false;
    
            // 检测矩阵索引计算模式: 基址 + i*宽度 + j
            if (instrs[0].find("mul ") == 0 && 
                instrs[1].find("add ") == 0 && 
                instrs[2].find("add ") == 0 &&
                instrs[3].find("lw ") == 0) {
        
                // 这是矩阵访问模式，但我们保持指令不变
                // 在实际编译器中，可以使用更高效的寻址模式
                return false;
            }
            return false;
        });

        // 模式11: 消除嵌套循环中的冗余计算
        addPeepholePattern("nested_loop_invariant", [](std::vector<std::string>& instrs) -> bool {
            if (instrs.size() < 3) return false;
            
            // 检查是否有常见的循环不变量计算模式
            // 例如，在内层循环中重复计算外层循环变量的表达式
            
            // 这是一个相对复杂的优化，需要理解嵌套循环结构
            // 暂时返回false，在更完整的实现中可以添加此优化
            return false;
        });

        // 模式12: 优化条件分支序列
        addPeepholePattern("branch_sequence", [](std::vector<std::string>& instrs) -> bool {
            if (instrs.size() < 2) return false;
            
            std::string instr1 = instrs[0];
            std::string instr2 = instrs[1];
            
            // 检测 beqz/bnez 后跟无条件跳转的模式
            if ((instr1.find("beqz ") == 0 || instr1.find("bnez ") == 0) && 
                 instr2.find("j ") == 0) {
                
                // 这里可以优化条件分支链，但需要更深入的流程分析
                // 暂时保持不变
                return false;
            }
            return false;
        });
        
        // 模式13: 优化函数返回序列
        addPeepholePattern("return_sequence", [](std::vector<std::string>& instrs) -> bool {
            if (instrs.size() < 3) return false;
            
            // 检查是否是常见的函数返回序列
            // 例如，可能的冗余寄存器恢复
            
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
    // const std::vector<std::shared_ptr<IRInstr>>& instructions,
    // const std::vector<Register>& availableRegs) {
    
    // std::map<std::string, std::string> allocation;
    
    // // 计算变量的生命周期
    // std::vector<LiveInterval> intervals = computeLiveIntervals(instructions);
    
    // // 按开始位置排序
    // std::sort(intervals.begin(), intervals.end());
    
    // // 可用寄存器列表
    // std::vector<std::string> freeRegs;
    // for (const auto& reg : availableRegs) {
    //     // 只使用被调用者保存的寄存器(s0-s11)进行全局分配
    //     // 这样可以减少函数调用时的寄存器保存/恢复开销
    //     if (reg.isCalleeSaved && reg.name != "fp" && reg.name != "s0") {
    //         freeRegs.push_back(reg.name);
    //     }
    // }
    
    // // 当前活跃的区间及其分配的寄存器
    // std::map<std::string, std::pair<LiveInterval, std::string>> active;
    
    // // 线性扫描算法
    // for (const auto& interval : intervals) {
    //     // 过期活跃区间
    //     std::vector<std::string> expired;
    //     for (auto& [var, pair] : active) {
    //         if (pair.first.end < interval.start) {
    //             freeRegs.push_back(pair.second); // 释放寄存器
    //             expired.push_back(var);
    //         }
    //     }
        
    //     // 从活跃集合中移除过期区间
    //     for (const auto& var : expired) {
    //         active.erase(var);
    //     }
        
    //     // 如果没有可用寄存器，需要溢出
    //     if (freeRegs.empty()) {
    //         // 找到最晚结束的活跃区间
    //         std::string victimVar;
    //         int latestEnd = -1;
            
    //         for (const auto& [var, pair] : active) {
    //             if (pair.first.end > latestEnd) {
    //                 latestEnd = pair.first.end;
    //                 victimVar = var;
    //             }
    //         }
            
    //         // 如果当前区间比受害者结束更早，则溢出受害者
    //         if (latestEnd > interval.end && !victimVar.empty()) {
    //             allocation[interval.var] = active[victimVar].second;
    //             freeRegs.push_back(active[victimVar].second);
    //             active.erase(victimVar);
                
    //             // 将新区间加入活跃集合
    //             active[interval.var] = {interval, allocation[interval.var]};
    //         }
    //         // 否则当前区间溢出（不分配寄存器）
    //     } else {
    //         // 有可用寄存器，分配一个
    //         std::string reg = freeRegs.back();
    //         freeRegs.pop_back();
            
    //         allocation[interval.var] = reg;
    //         active[interval.var] = {interval, reg};
    //     }
    // }
    
    // return allocation;
    const std::vector<std::shared_ptr<IRInstr>>& instructions,
    const std::vector<Register>& availableRegs) {
    
    std::map<std::string, std::string> allocation;
    
    // 计算变量的生命周期
    std::vector<LiveInterval> intervals = computeLiveIntervals(instructions);
    
    // 按开始位置排序
    std::sort(intervals.begin(), intervals.end());
    
    // 创建可用寄存器列表，按优先级排序
    std::vector<std::string> freeRegs;
    
    // 先添加被调用者保存的寄存器(s0-s11)，减少函数调用开销
    for (const auto& reg : availableRegs) {
        if (reg.isCalleeSaved && reg.isAllocatable && reg.name != "fp" && reg.name != "s0") {
            freeRegs.push_back(reg.name);
        }
    }
    
    // 再添加调用者保存的寄存器(t0-t6)
    for (const auto& reg : availableRegs) {
        if (reg.isCallerSaved && reg.isAllocatable && 
            reg.name != "ra" && reg.name.substr(0, 1) != "a") {
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
                
                // 从活跃集合中移除受害者
                std::string freedReg = active[victimVar].second;
                active.erase(victimVar);
                
                // 将新区间加入活跃集合
                active[interval.var] = {interval, freedReg};
            }
            // 否则当前区间溢出（不分配寄存器）
        } else {
            // 有可用寄存器，分配最先可用的一个
            std::string reg = freeRegs.front();
            freeRegs.erase(freeRegs.begin());
            
            allocation[interval.var] = reg;
            active[interval.var] = {interval, reg};
        }
    }
    
    return allocation;
}

// 计算变量的生命周期区间
std::vector<LinearScanRegisterAllocator::LiveInterval> LinearScanRegisterAllocator::computeLiveIntervals(
    // const std::vector<std::shared_ptr<IRInstr>>& instructions) {
    
    // std::map<std::string, LiveInterval> intervalMap;
    
    // // 扫描所有指令，计算变量的定义和使用位置
    // for (int i = 0; i < instructions.size(); i++) {
    //     auto instr = instructions[i];
        
    //     // 获取指令定义的变量
    //     auto defined = IRAnalyzer::getDefinedVariables(instr);
    //     for (const auto& var : defined) {
    //         // 如果变量尚未有区间，创建一个新区间
    //         if (intervalMap.find(var) == intervalMap.end()) {
    //             intervalMap[var] = {var, i, i};
    //         } 
    //         // 否则更新开始位置（取最小值）
    //         else {
    //             intervalMap[var].start = std::min(intervalMap[var].start, i);
    //         }
    //     }
        
    //     // 获取指令使用的变量
    //     auto used = IRAnalyzer::getUsedVariables(instr);
    //     for (const auto& var : used) {
    //         // 如果变量尚未有区间，创建一个新区间
    //         if (intervalMap.find(var) == intervalMap.end()) {
    //             intervalMap[var] = {var, i, i};
    //         }
    //         // 更新结束位置（取最大值）
    //         intervalMap[var].end = std::max(intervalMap[var].end, i);
    //     }
    // }
    
    // // 转换为向量形式
    // std::vector<LiveInterval> intervals;
    // for (const auto& [var, interval] : intervalMap) {
    //     intervals.push_back(interval);
    // }
    
    // return intervals;
    const std::vector<std::shared_ptr<IRInstr>>& instructions) {
    
    std::map<std::string, LiveInterval> intervalMap;
    
    // 为每个基本块收集存活变量
    std::vector<std::set<std::string>> liveOut(instructions.size());
    
    // 构建基本块和CFG
    std::map<int, std::vector<int>> cfg;
    std::map<int, std::vector<int>> reverseCfg;
    
    // 找到所有标签位置
    std::map<std::string, int> labelToIndex;
    for (int i = 0; i < instructions.size(); i++) {
        if (auto labelInstr = std::dynamic_pointer_cast<LabelInstr>(instructions[i])) {
            labelToIndex[labelInstr->label] = i;
        }
    }
    
    // 构建CFG
    for (int i = 0; i < instructions.size(); i++) {
        // 默认下一条指令
        if (i + 1 < instructions.size()) {
            cfg[i].push_back(i + 1);
            reverseCfg[i + 1].push_back(i);
        }
        
        // 处理跳转
        if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(instructions[i])) {
            auto it = labelToIndex.find(gotoInstr->target->name);
            if (it != labelToIndex.end()) {
                cfg[i].push_back(it->second);
                reverseCfg[it->second].push_back(i);
                
                // 无条件跳转移除默认下一条
                if (!cfg[i].empty()) {
                    cfg[i].pop_back();
                    if (i + 1 < instructions.size()) {
                        auto pos = std::find(reverseCfg[i + 1].begin(), 
                                          reverseCfg[i + 1].end(), i);
                        if (pos != reverseCfg[i + 1].end()) {
                            reverseCfg[i + 1].erase(pos);
                        }
                    }
                }
            }
        } else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instructions[i])) {
            auto it = labelToIndex.find(ifGotoInstr->target->name);
            if (it != labelToIndex.end()) {
                cfg[i].push_back(it->second);
                reverseCfg[it->second].push_back(i);
            }
        }
    }
    
    // 使用迭代算法计算每个点的liveOut集合
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (int i = instructions.size() - 1; i >= 0; i--) {
            std::set<std::string> oldLiveOut = liveOut[i];
            
            // 收集所有后继的活跃变量
            std::set<std::string> newLiveOut;
            for (int succ : cfg[i]) {
                if (succ < liveOut.size()) {
                    std::set<std::string> liveInSucc = liveOut[succ];
                    
                    // 移除succ定义的变量
                    auto defined = IRAnalyzer::getDefinedVariables(instructions[succ]);
                    for (const auto& def : defined) {
                        liveInSucc.erase(def);
                    }
                    
                    // 添加succ使用的变量
                    auto used = IRAnalyzer::getUsedVariables(instructions[succ]);
                    for (const auto& use : used) {
                        liveInSucc.insert(use);
                    }
                    
                    // 合并到当前指令的liveOut
                    newLiveOut.insert(liveInSucc.begin(), liveInSucc.end());
                }
                
                // 合并后继的liveOut
                if (succ < liveOut.size()) {
                    newLiveOut.insert(liveOut[succ].begin(), liveOut[succ].end());
                }
            }
            
            // 更新liveOut
            if (newLiveOut != oldLiveOut) {
                liveOut[i] = newLiveOut;
                changed = true;
            }
        }
    }
    
    // 使用liveOut信息计算变量的活跃区间
    for (int i = 0; i < instructions.size(); i++) {
        auto instr = instructions[i];
        
        // 获取指令定义的变量
        auto defined = IRAnalyzer::getDefinedVariables(instr);
        for (const auto& var : defined) {
            // 如果变量尚未有区间，创建一个新区间
            if (intervalMap.find(var) == intervalMap.end()) {
                intervalMap[var] = {var, i, i};
            } else {
                // 更新定义位置（取最小值）
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
        
        // 使用liveOut信息延长变量的生命周期
        for (const auto& var : liveOut[i]) {
            if (intervalMap.find(var) != intervalMap.end()) {
                intervalMap[var].end = std::max(intervalMap[var].end, i);
            }
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
    // const std::vector<std::shared_ptr<IRInstr>>& instructions,
    // const std::vector<Register>& availableRegs) {
    
    // std::map<std::string, std::string> allocation;
    
    // // 构建冲突图
    // auto interferenceGraph = buildInterferenceGraph(instructions);
    
    // // 如果图为空，直接返回空分配
    // if (interferenceGraph.empty()) {
    //     return allocation;
    // }
    
    // // 简化冲突图，获取简化后的变量顺序
    // auto simplifiedOrder = simplify(interferenceGraph);
    
    // // 根据简化顺序为变量分配寄存器
    // allocation = color(simplifiedOrder, interferenceGraph, availableRegs);
    
    // return allocation;
    const std::vector<std::shared_ptr<IRInstr>>& instructions,
    const std::vector<Register>& availableRegs) {
    
    std::map<std::string, std::string> allocation;
    
    // 构建冲突图
    auto interferenceGraph = buildInterferenceGraph(instructions);
    
    // 如果图为空，直接返回空分配
    if (interferenceGraph.empty()) {
        return allocation;
    }
    
    // 计算每个变量的溢出代价（使用频率）
    std::map<std::string, int> spillCosts = calculateSpillCosts(instructions);
    
    // 简化冲突图，获取简化后的变量顺序
    auto simplifiedOrder = simplify(interferenceGraph, spillCosts);
    
    // 根据简化顺序为变量分配寄存器
    allocation = color(simplifiedOrder, interferenceGraph, availableRegs);
    
    return allocation;
}

// 计算变量的溢出代价
std::map<std::string, int> GraphColoringRegisterAllocator::calculateSpillCosts(
    const std::vector<std::shared_ptr<IRInstr>>& instructions) {
    
    std::map<std::string, int> spillCosts;
    
    // 基于变量使用频率计算溢出代价
    for (const auto& instr : instructions) {
        // 使用变量增加代价
        auto used = IRAnalyzer::getUsedVariables(instr);
        for (const auto& var : used) {
            spillCosts[var] += 1;
            
            // 循环内使用的变量溢出代价更高
            if (instr->toString().find("loop") != std::string::npos) {
                spillCosts[var] += 5;
            }
        }
        
        // 定义变量增加代价
        auto defined = IRAnalyzer::getDefinedVariables(instr);
        for (const auto& var : defined) {
            spillCosts[var] += 1;
            
            // 循环内定义的变量溢出代价更高
            if (instr->toString().find("loop") != std::string::npos) {
                spillCosts[var] += 5;
            }
        }
    }
    
    return spillCosts;
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
    // std::map<std::string, std::set<std::string>>& graph) {
    
    // std::vector<std::string> simplifiedOrder;
    
    // // 创建图的副本，因为我们会修改它
    // auto workGraph = graph;
    
    // // 当图不为空时，继续简化
    // while (!workGraph.empty()) {
    //     // 查找度数小于可用寄存器数量的节点
    //     std::string nodeToRemove;
    //     int minDegree = std::numeric_limits<int>::max();
        
    //     for (const auto& [node, neighbors] : workGraph) {
    //         if (neighbors.size() < minDegree) {
    //             minDegree = neighbors.size();
    //             nodeToRemove = node;
    //         }
    //     }
        
    //     // 如果找不到合适的节点，我们需要选择一个"溢出"节点
    //     // 这里简单地选择度数最大的节点作为溢出候选
    //     if (nodeToRemove.empty()) {
    //         int maxDegree = -1;
    //         for (const auto& [node, neighbors] : workGraph) {
    //             if (neighbors.size() > maxDegree) {
    //                 maxDegree = neighbors.size();
    //                 nodeToRemove = node;
    //             }
    //         }
    //     }
        
    //     // 从图中移除该节点
    //     for (auto& [_, neighbors] : workGraph) {
    //         neighbors.erase(nodeToRemove);
    //     }
    //     workGraph.erase(nodeToRemove);
        
    //     // 将节点添加到简化序列
    //     simplifiedOrder.push_back(nodeToRemove);
    // }
    
    // // 逆序返回，这样我们可以从最后一个移除的节点开始着色
    // std::reverse(simplifiedOrder.begin(), simplifiedOrder.end());
    // return simplifiedOrder;
    std::map<std::string, std::set<std::string>>& graph,
    const std::map<std::string, int>& spillCosts) {
    
    std::vector<std::string> simplifiedOrder;
    
    // 创建图的副本，因为我们会修改它
    auto workGraph = graph;
    
    // 寄存器数量（估计值，实际会根据可用寄存器数量调整）
    const int K = 10; 
    
    // 当图不为空时，继续简化
    while (!workGraph.empty()) {
        // 查找度数小于K的节点，优先选择度数最小的
        std::string nodeToRemove;
        int minDegree = K;
        
        for (const auto& [node, neighbors] : workGraph) {
            if (neighbors.size() < minDegree) {
                minDegree = neighbors.size();
                nodeToRemove = node;
            }
        }
        
        // 如果找不到度数小于K的节点，选择一个溢出节点
        if (nodeToRemove.empty()) {
            // 选择溢出代价/度数最小的节点
            double minRatio = std::numeric_limits<double>::max();
            
            for (const auto& [node, neighbors] : workGraph) {
                // 获取变量的溢出代价
                int cost = 1;  // 默认代价
                auto costIt = spillCosts.find(node);
                if (costIt != spillCosts.end()) {
                    cost = costIt->second;
                }
                
                // 计算代价/度数比率
                double ratio = static_cast<double>(cost) / (neighbors.size() + 1);
                if (ratio < minRatio) {
                    minRatio = ratio;
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
    // const std::vector<std::string>& simplifiedOrder,
    // const std::map<std::string, std::set<std::string>>& originalGraph,
    // const std::vector<Register>& availableRegs) {
    
    // std::map<std::string, std::string> allocation;
    
    // // 筛选可分配的寄存器
    // std::vector<std::string> regNames;
    // for (const auto& reg : availableRegs) {
    //     if (reg.isAllocatable && !reg.isReserved) {
    //         regNames.push_back(reg.name);
    //     }
    // }
    
    // // 按照简化顺序为变量分配颜色（寄存器）
    // for (const auto& var : simplifiedOrder) {
    //     // 获取变量的邻居
    //     auto it = originalGraph.find(var);
    //     if (it == originalGraph.end()) continue;
        
    //     const auto& neighbors = it->second;
        
    //     // 收集邻居已使用的颜色
    //     std::set<std::string> usedColors;
    //     for (const auto& neighbor : neighbors) {
    //         auto allocIt = allocation.find(neighbor);
    //         if (allocIt != allocation.end()) {
    //             usedColors.insert(allocIt->second);
    //         }
    //     }
        
    //     // 查找可用的颜色
    //     std::string selectedReg;
    //     for (const auto& reg : regNames) {
    //         if (usedColors.find(reg) == usedColors.end()) {
    //             selectedReg = reg;
    //             break;
    //         }
    //     }
        
    //     // 如果找到可用寄存器，进行分配
    //     if (!selectedReg.empty()) {
    //         allocation[var] = selectedReg;
    //     }
    //     // 否则，变量需要溢出到内存（不分配寄存器）
    // }
    
    // return allocation;
    const std::vector<std::string>& simplifiedOrder,
    const std::map<std::string, std::set<std::string>>& originalGraph,
    const std::vector<Register>& availableRegs) {
    
    std::map<std::string, std::string> allocation;
    
    // 筛选可分配的寄存器，按优先级排序
    std::vector<std::string> regNames;
    
    // 先添加被调用者保存的寄存器
    for (const auto& reg : availableRegs) {
        if (reg.isCalleeSaved && reg.isAllocatable && reg.name != "fp" && reg.name != "s0") {
            regNames.push_back(reg.name);
        }
    }
    
    // 再添加调用者保存的寄存器
    for (const auto& reg : availableRegs) {
        if (reg.isCallerSaved && reg.isAllocatable && 
            reg.name != "ra" && reg.name.substr(0, 1) != "a") {
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

