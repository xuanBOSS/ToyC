#include "irgen.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

// Operand toString方法
std::string Operand::toString() const {
    switch (type) {
        case OperandType::VARIABLE:
            return name;
        case OperandType::TEMP:
            return name;
        case OperandType::CONSTANT:
            return std::to_string(value);
        case OperandType::LABEL:
            return name;
        default:
            return "unknown";
    }
}

// BinaryOpInstr toString方法
std::string BinaryOpInstr::toString() const {
    std::string opStr;
    switch (opcode) {
        case OpCode::ADD: opStr = "+"; break;
        case OpCode::SUB: opStr = "-"; break;
        case OpCode::MUL: opStr = "*"; break;
        case OpCode::DIV: opStr = "/"; break;
        case OpCode::MOD: opStr = "%"; break;
        case OpCode::LT: opStr = "<"; break;
        case OpCode::GT: opStr = ">"; break;
        case OpCode::LE: opStr = "<="; break;
        case OpCode::GE: opStr = ">="; break;
        case OpCode::EQ: opStr = "=="; break;
        case OpCode::NE: opStr = "!="; break;
        case OpCode::AND: opStr = "&&"; break;
        case OpCode::OR: opStr = "||"; break;
        default: opStr = "unknown"; break;
    }
    
    return result->toString() + " = " + left->toString() + " " + opStr + " " + right->toString();
}

// UnaryOpInstr toString方法
std::string UnaryOpInstr::toString() const {
    std::string opStr;
    switch (opcode) {
        case OpCode::NEG: opStr = "-"; break;
        case OpCode::NOT: opStr = "!"; break;
        default: opStr = "unknown"; break;
    }
    
    return result->toString() + " = " + opStr + operand->toString();
}

// AssignInstr toString方法
std::string AssignInstr::toString() const {
    return target->toString() + " = " + source->toString();
}

// GotoInstr toString方法
std::string GotoInstr::toString() const {
    return "goto " + target->toString();
}

// IfGotoInstr toString方法
std::string IfGotoInstr::toString() const {
    return "if " + condition->toString() + " goto " + target->toString();
}

// ParamInstr toString方法
std::string ParamInstr::toString() const {
    return "param " + param->toString();
}

// CallInstr toString方法
std::string CallInstr::toString() const {
    if (result) {
        return result->toString() + " = call " + funcName + ", " + std::to_string(paramCount);
    } else {
        return "call " + funcName + ", " + std::to_string(paramCount);
    }
}

// ReturnInstr toString方法
std::string ReturnInstr::toString() const {
    if (value) {
        return "return " + value->toString();
    } else {
        return "return";
    }
}

// LabelInstr toString方法
std::string LabelInstr::toString() const {
    return label + ":";
}

// FunctionBeginInstr toString方法
std::string FunctionBeginInstr::toString() const {
    return "function " + funcName + " begin";
}

// FunctionEndInstr toString方法
std::string FunctionEndInstr::toString() const {
    return "function " + funcName + " end";
}

// IRGenerator方法
void IRGenerator::generate(std::shared_ptr<CompUnit> ast) {
    if (ast) {
        ast->accept(*this);

        // 如果启用了优化，则优化IR
        if (config.enableOptimizations) {
            optimize();
        }
    }
}

std::shared_ptr<Operand> IRGenerator::createTemp() {
    std::string name = "t" + std::to_string(tempCount++);
    return std::make_shared<Operand>(OperandType::TEMP, name);
}

std::shared_ptr<Operand> IRGenerator::createLabel() {
    std::string name = "L" + std::to_string(labelCount++);
    return std::make_shared<Operand>(OperandType::LABEL, name);
}

void IRGenerator::addInstruction(std::shared_ptr<IRInstr> instr) {
    instructions.push_back(instr);
}

std::shared_ptr<Operand> IRGenerator::getTopOperand() {
    if (operandStack.empty()) {
        std::cerr << "Error: Operand stack is empty" << std::endl;
        return std::make_shared<Operand>(0); // 默认返回常量0
    }
    
    std::shared_ptr<Operand> result = operandStack.back();
    operandStack.pop_back();
    return result;
}

void IRGenerator::enterScope() {
    scopeStack.push_back(std::map<std::string, std::shared_ptr<Operand>>());
}

void IRGenerator::exitScope() {
    if (!scopeStack.empty()) {
        scopeStack.pop_back();
    }
}

std::shared_ptr<Operand> IRGenerator::findVariableInCurrentScope(const std::string& name) {
    if (scopeStack.empty()) {
        return nullptr;
    }
    
    auto& currentScope = scopeStack.back();
    auto it = currentScope.find(name);
    if (it != currentScope.end()) {
        return it->second;
    }
    
    return nullptr;
}

std::shared_ptr<Operand> IRGenerator::findVariable(const std::string& name) {
    // 从内层作用域向外层作用域查找
    for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
        auto varIt = it->find(name);
        if (varIt != it->end()) {
            return varIt->second;
        }
    }
    
    return nullptr;
}

void IRGenerator::defineVariable(const std::string& name, std::shared_ptr<Operand> var) {
    if (scopeStack.empty()) {
        enterScope();
    }
    
    scopeStack.back()[name] = var;
}

std::shared_ptr<Operand> IRGenerator::getVariable(const std::string& name) {
    // 首先在现有作用域中查找变量
    std::shared_ptr<Operand> var = findVariable(name);
    if (var) {
        return var;
    }
    
    // 如果变量不存在，创建一个新的并添加到当前作用域
    var = std::make_shared<Operand>(OperandType::VARIABLE, name);
    defineVariable(name, var);
    return var;
}

void IRGenerator::dumpIR(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
        return;
    }
    
    IRPrinter::print(instructions, outFile);
    outFile.close();
}

void IRGenerator::optimize() {
    // 执行各种优化
    constantFolding();
    constantPropagation();
    deadCodeElimination();
    controlFlowOptimization();
}

void IRGenerator::constantFolding() {
    // 常量折叠实现
    // 遍历所有指令，识别可以在编译时计算的常量表达式
    for (size_t i = 0; i < instructions.size(); ++i) {
        auto instr = instructions[i];
        
        // 检查是否是二元操作，且两个操作数都是常量
        if (auto binOp = std::dynamic_pointer_cast<BinaryOpInstr>(instr)) {
            if (binOp->left->type == OperandType::CONSTANT && 
                binOp->right->type == OperandType::CONSTANT) {
                
                int result = 0;
                bool canFold = true;
                
                // 根据操作类型计算结果
                switch (binOp->opcode) {
                    case OpCode::ADD: result = binOp->left->value + binOp->right->value; break;
                    case OpCode::SUB: result = binOp->left->value - binOp->right->value; break;
                    case OpCode::MUL: result = binOp->left->value * binOp->right->value; break;
                    case OpCode::DIV: 
                        if (binOp->right->value == 0) {
                            canFold = false; // 避免除以零
                        } else {
                            result = binOp->left->value / binOp->right->value;
                        }
                        break;
                    case OpCode::MOD: 
                        if (binOp->right->value == 0) {
                            canFold = false; // 避免除以零
                        } else {
                            result = binOp->left->value % binOp->right->value;
                        }
                        break;
                    case OpCode::LT: result = binOp->left->value < binOp->right->value ? 1 : 0; break;
                    case OpCode::GT: result = binOp->left->value > binOp->right->value ? 1 : 0; break;
                    case OpCode::LE: result = binOp->left->value <= binOp->right->value ? 1 : 0; break;
                    case OpCode::GE: result = binOp->left->value >= binOp->right->value ? 1 : 0; break;
                    case OpCode::EQ: result = binOp->left->value == binOp->right->value ? 1 : 0; break;
                    case OpCode::NE: result = binOp->left->value != binOp->right->value ? 1 : 0; break;
                    case OpCode::AND: result = (binOp->left->value && binOp->right->value) ? 1 : 0; break;
                    case OpCode::OR: result = (binOp->left->value || binOp->right->value) ? 1 : 0; break;
                    default: canFold = false; break;
                }
                
                if (canFold) {
                    // 用赋值指令替换原二元操作指令
                    auto constResult = std::make_shared<Operand>(result);
                    auto assignInstr = std::make_shared<AssignInstr>(binOp->result, constResult);
                    instructions[i] = assignInstr;
                }
            }
        }
        // 检查是否是一元操作，且操作数是常量
        else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpInstr>(instr)) {
            if (unaryOp->operand->type == OperandType::CONSTANT) {
                int result = 0;
                bool canFold = true;
                
                // 根据操作类型计算结果
                switch (unaryOp->opcode) {
                    case OpCode::NEG: result = -unaryOp->operand->value; break;
                    case OpCode::NOT: result = !unaryOp->operand->value; break;
                    default: canFold = false; break;
                }
                
                if (canFold) {
                    // 用赋值指令替换原一元操作指令
                    auto constResult = std::make_shared<Operand>(result);
                    auto assignInstr = std::make_shared<AssignInstr>(unaryOp->result, constResult);
                    instructions[i] = assignInstr;
                }
            }
        }
    }
}

void IRGenerator::constantPropagation() {
    // 常量传播实现 - 简化版
    std::map<std::string, std::shared_ptr<Operand>> constants;
    
    for (size_t i = 0; i < instructions.size(); ++i) {
        auto instr = instructions[i];
        
        // 检查赋值指令，如果源操作数是常量，记录目标变量与常量的映射
        if (auto assignInstr = std::dynamic_pointer_cast<AssignInstr>(instr)) {
            if (assignInstr->source->type == OperandType::CONSTANT) {
                constants[assignInstr->target->name] = assignInstr->source;
            } else {
                // 如果不是常量赋值，从映射中移除目标变量
                constants.erase(assignInstr->target->name);
            }
        }
        // 对于其他可能修改变量的指令，从映射中移除相关变量
        else if (auto binOp = std::dynamic_pointer_cast<BinaryOpInstr>(instr)) {
            constants.erase(binOp->result->name);
        }
        else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpInstr>(instr)) {
            constants.erase(unaryOp->result->name);
        }
        else if (auto callInstr = std::dynamic_pointer_cast<CallInstr>(instr)) {
            if (callInstr->result) {
                constants.erase(callInstr->result->name);
            }
        }
    }
}

void IRGenerator::deadCodeElimination() {
    // 死代码删除实现 - 简化版
    // 标记所有可能产生副作用的指令和控制流指令
    std::vector<bool> isLive(instructions.size(), false);
    
    // 第一遍：标记所有产生副作用的指令
    for (size_t i = 0; i < instructions.size(); ++i) {
        auto instr = instructions[i];
        
        // 函数调用、返回、跳转等指令都有副作用
        if (std::dynamic_pointer_cast<CallInstr>(instr) ||
            std::dynamic_pointer_cast<ReturnInstr>(instr) ||
            std::dynamic_pointer_cast<GotoInstr>(instr) ||
            std::dynamic_pointer_cast<IfGotoInstr>(instr) ||
            std::dynamic_pointer_cast<LabelInstr>(instr) ||
            std::dynamic_pointer_cast<FunctionBeginInstr>(instr) ||
            std::dynamic_pointer_cast<FunctionEndInstr>(instr)) {
            isLive[i] = true;
        }
    }
    
    // 第二遍：保留所有标记为活跃的指令
    std::vector<std::shared_ptr<IRInstr>> newInstructions;
    for (size_t i = 0; i < instructions.size(); ++i) {
        if (isLive[i]) {
            newInstructions.push_back(instructions[i]);
        }
    }
    
    instructions = newInstructions;
}

void IRGenerator::controlFlowOptimization() {
    // 控制流优化实现 - 简化版
    // 例如：删除无法到达的代码块、合并基本块等
    // 此处仅作为框架，不实现具体逻辑
}

bool IRGenerator::isControlFlowInstruction(const std::shared_ptr<IRInstr>& instr) const {
    return std::dynamic_pointer_cast<GotoInstr>(instr) != nullptr ||
           std::dynamic_pointer_cast<IfGotoInstr>(instr) != nullptr ||
           std::dynamic_pointer_cast<ReturnInstr>(instr) != nullptr;
}

std::vector<std::string> IRGenerator::getControlFlowTargets(const std::shared_ptr<IRInstr>& instr) const {
    std::vector<std::string> targets;
    
    if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(instr)) {
        targets.push_back(gotoInstr->target->name);
    }
    else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
        targets.push_back(ifGotoInstr->target->name);
    }
    
    return targets;
}

std::map<std::string, IRGenerator::BasicBlock> IRGenerator::buildControlFlowGraph() {
    std::map<std::string, BasicBlock> blocks;
    
    // 寻找所有标签和控制流指令
    std::vector<int> leaders; // 基本块的起始指令索引
    
    // 第一条指令总是一个leader
    if (!instructions.empty()) {
        leaders.push_back(0);
    }
    
    // 标记所有标签指令和控制流指令之后的指令为leader
    for (size_t i = 0; i < instructions.size(); ++i) {
        // 如果是标签，则标签本身是leader
        if (std::dynamic_pointer_cast<LabelInstr>(instructions[i])) {
            leaders.push_back(i);
        }
        // 如果是控制流指令，则下一条指令是leader
        else if (isControlFlowInstruction(instructions[i]) && i + 1 < instructions.size()) {
            leaders.push_back(i + 1);
        }
    }
    
    // 去重并排序leaders
    std::sort(leaders.begin(), leaders.end());
    leaders.erase(std::unique(leaders.begin(), leaders.end()), leaders.end());
    
    // 创建基本块
    for (size_t i = 0; i < leaders.size(); ++i) {
        int start = leaders[i];
        int end = (i + 1 < leaders.size()) ? leaders[i + 1] - 1 : instructions.size() - 1;
        
        // 找到块的标签
        std::string blockLabel;
        if (std::dynamic_pointer_cast<LabelInstr>(instructions[start])) {
            blockLabel = std::dynamic_pointer_cast<LabelInstr>(instructions[start])->label;
        } else {
            blockLabel = "block_" + std::to_string(start);
        }
        
        // 创建基本块
        BasicBlock block;
        block.label = blockLabel;
        
        // 添加指令索引
        for (int j = start; j <= end; ++j) {
            block.instructionIndices.push_back(j);
        }
        
        blocks[blockLabel] = block;
    }
    
    // 添加基本块之间的边
    for (auto& [label, block] : blocks) {
        // 获取块的最后一条指令
        if (!block.instructionIndices.empty()) {
            int lastInstrIndex = block.instructionIndices.back();
            auto lastInstr = instructions[lastInstrIndex];
            
            // 如果是控制流指令，添加相应的边
            if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(lastInstr)) {
                block.successors.push_back(gotoInstr->target->name);
            }
            else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(lastInstr)) {
                // 条件为真时跳转
                block.successors.push_back(ifGotoInstr->target->name);
                
                // 条件为假时继续执行下一个基本块
                if (lastInstrIndex + 1 < instructions.size()) {
                    // 找到下一个基本块
                    for (auto& [nextLabel, nextBlock] : blocks) {
                        if (!nextBlock.instructionIndices.empty() && 
                            nextBlock.instructionIndices.front() == lastInstrIndex + 1) {
                            block.successors.push_back(nextLabel);
                            break;
                        }
                    }
                }
            }
            else if (std::dynamic_pointer_cast<ReturnInstr>(lastInstr)) {
                // 返回指令没有后继
            }
            else if (lastInstrIndex + 1 < instructions.size()) {
                // 非控制流指令，继续执行下一个基本块
                for (auto& [nextLabel, nextBlock] : blocks) {
                    if (!nextBlock.instructionIndices.empty() && 
                        nextBlock.instructionIndices.front() == lastInstrIndex + 1) {
                        block.successors.push_back(nextLabel);
                        break;
                    }
                }
            }
        }
    }
    
    // 计算前驱
    for (auto& [label, block] : blocks) {
        for (const auto& succ : block.successors) {
            if (blocks.find(succ) != blocks.end()) {
                blocks[succ].predecessors.push_back(label);
            }
        }
    }
    
    return blocks;
}


// 访问方法实现
void IRGenerator::visit(NumberExpr& expr) {
    std::shared_ptr<Operand> constant = std::make_shared<Operand>(expr.value);
    operandStack.push_back(constant);
}

void IRGenerator::visit(VariableExpr& expr) {
    std::shared_ptr<Operand> var = getVariable(expr.name);
    operandStack.push_back(var);
}

void IRGenerator::visit(BinaryExpr& expr) {
    expr.right->accept(*this);
    std::shared_ptr<Operand> right = getTopOperand();
    
    expr.left->accept(*this);
    std::shared_ptr<Operand> left = getTopOperand();
    
    std::shared_ptr<Operand> result = createTemp();
    
    OpCode opcode;
    if (expr.op == "+") opcode = OpCode::ADD;
    else if (expr.op == "-") opcode = OpCode::SUB;
    else if (expr.op == "*") opcode = OpCode::MUL;
    else if (expr.op == "/") opcode = OpCode::DIV;
    else if (expr.op == "%") opcode = OpCode::MOD;
    else if (expr.op == "<") opcode = OpCode::LT;
    else if (expr.op == ">") opcode = OpCode::GT;
    else if (expr.op == "<=") opcode = OpCode::LE;
    else if (expr.op == ">=") opcode = OpCode::GE;
    else if (expr.op == "==") opcode = OpCode::EQ;
    else if (expr.op == "!=") opcode = OpCode::NE;
    else if (expr.op == "&&") opcode = OpCode::AND;
    else if (expr.op == "||") opcode = OpCode::OR;
    else {
        std::cerr << "Error: Unknown binary operator: " << expr.op << std::endl;
        opcode = OpCode::ADD; // 默认使用加法
    }
    
    addInstruction(std::make_shared<BinaryOpInstr>(opcode, result, left, right));
    operandStack.push_back(result);
}

std::shared_ptr<Operand> IRGenerator::generateShortCircuitAnd(BinaryExpr& expr) {
    expr.left->accept(*this);
    std::shared_ptr<Operand> left = getTopOperand();
    
    // 创建结果临时变量和短路标签
    std::shared_ptr<Operand> result = createTemp();
    std::shared_ptr<Operand> shortCircuitLabel = createLabel();
    std::shared_ptr<Operand> endLabel = createLabel();
    
    // 如果左操作数为假，短路并返回0
    addInstruction(std::make_shared<IfGotoInstr>(left, shortCircuitLabel));
    
    // 否则，计算右操作数
    expr.right->accept(*this);
    std::shared_ptr<Operand> right = getTopOperand();
    
    // 结果等于右操作数
    addInstruction(std::make_shared<AssignInstr>(result, right));
    addInstruction(std::make_shared<GotoInstr>(endLabel));
    
    // 短路处理：结果为0
    addInstruction(std::make_shared<LabelInstr>(shortCircuitLabel->name));
    addInstruction(std::make_shared<AssignInstr>(result, std::make_shared<Operand>(0)));
    
    // 结束标签
    addInstruction(std::make_shared<LabelInstr>(endLabel->name));
    
    return result;
}

std::shared_ptr<Operand> IRGenerator::generateShortCircuitOr(BinaryExpr& expr) {
    expr.left->accept(*this);
    std::shared_ptr<Operand> left = getTopOperand();
    
    // 创建结果临时变量和短路标签
    std::shared_ptr<Operand> result = createTemp();
    std::shared_ptr<Operand> shortCircuitLabel = createLabel();
    std::shared_ptr<Operand> endLabel = createLabel();
    
    // 如果左操作数为真，短路并返回1
    addInstruction(std::make_shared<IfGotoInstr>(left, shortCircuitLabel));
    
    // 否则，计算右操作数
    expr.right->accept(*this);
    std::shared_ptr<Operand> right = getTopOperand();
    
    // 结果等于右操作数
    addInstruction(std::make_shared<AssignInstr>(result, right));
    addInstruction(std::make_shared<GotoInstr>(endLabel));
    
    // 短路处理：结果为1
    addInstruction(std::make_shared<LabelInstr>(shortCircuitLabel->name));
    addInstruction(std::make_shared<AssignInstr>(result, std::make_shared<Operand>(1)));
    
    // 结束标签
    addInstruction(std::make_shared<LabelInstr>(endLabel->name));
    
    return result;
}

void IRGenerator::visit(UnaryExpr& expr) {
    expr.operand->accept(*this);
    std::shared_ptr<Operand> operand = getTopOperand();
    
    std::shared_ptr<Operand> result = createTemp();
    
    OpCode opcode;
    if (expr.op == "-") opcode = OpCode::NEG;
    else if (expr.op == "!") opcode = OpCode::NOT;
    else if (expr.op == "+") {
        // 一元加法不改变值，直接赋值
        addInstruction(std::make_shared<AssignInstr>(result, operand));
        operandStack.push_back(result);
        return;
    }
    else {
        std::cerr << "Error: Unknown unary operator: " << expr.op << std::endl;
        opcode = OpCode::NEG; // 默认使用取负
    }
    
    addInstruction(std::make_shared<UnaryOpInstr>(opcode, result, operand));
    operandStack.push_back(result);
}

void IRGenerator::visit(CallExpr& expr) {
    // 处理参数
    std::vector<std::shared_ptr<Operand>> args;
    for (const auto& arg : expr.arguments) {
        arg->accept(*this);
        args.push_back(getTopOperand());
    }
    
    // 倒序添加参数指令（参数从右到左压栈）
    for (auto it = args.rbegin(); it != args.rend(); ++it) {
        addInstruction(std::make_shared<ParamInstr>(*it));
    }
    
    // 调用函数
    std::shared_ptr<Operand> result = createTemp();
    addInstruction(std::make_shared<CallInstr>(
        result, expr.callee, expr.arguments.size()));
    
    operandStack.push_back(result);
}

void IRGenerator::visit(ExprStmt& stmt) {
    if (stmt.expression) {
        stmt.expression->accept(*this);
        // 表达式语句的结果会被丢弃
        if (!operandStack.empty()) {
            operandStack.pop_back();
        }
    }
}

void IRGenerator::visit(VarDeclStmt& stmt) {
    std::shared_ptr<Operand> var = getVariable(stmt.name);
    
    if (stmt.initializer) {
        stmt.initializer->accept(*this);
        std::shared_ptr<Operand> value = getTopOperand();
        
        addInstruction(std::make_shared<AssignInstr>(var, value));
    }
}

void IRGenerator::visit(AssignStmt& stmt) {
    stmt.value->accept(*this);
    std::shared_ptr<Operand> value = getTopOperand();
    
    std::shared_ptr<Operand> var = getVariable(stmt.name);
    
    addInstruction(std::make_shared<AssignInstr>(var, value));
}

void IRGenerator::visit(BlockStmt& stmt) {
    // 进入新的作用域
    enterScope();
    
    for (const auto& s : stmt.statements) {
        s->accept(*this);
    }
    
    // 离开作用域
    exitScope();
}

void IRGenerator::visit(IfStmt& stmt) {
    std::shared_ptr<Operand> elseLabel = createLabel();
    std::shared_ptr<Operand> endLabel = stmt.elseBranch ? createLabel() : elseLabel;
    
    // 条件表达式
    stmt.condition->accept(*this);
    std::shared_ptr<Operand> condition = getTopOperand();
    
    // 条件为假时跳转到else分支
    addInstruction(std::make_shared<IfGotoInstr>(condition, elseLabel));
    
    // then分支
    stmt.thenBranch->accept(*this);
    
    if (stmt.elseBranch) {
        // 执行完then分支后跳过else分支
        addInstruction(std::make_shared<GotoInstr>(endLabel));
        
        // else分支标签
        addInstruction(std::make_shared<LabelInstr>(elseLabel->name));
        
        // else分支
        stmt.elseBranch->accept(*this);
        
        // 结束标签
        addInstruction(std::make_shared<LabelInstr>(endLabel->name));
    } else {
        // 结束标签 (等同于else标签)
        addInstruction(std::make_shared<LabelInstr>(elseLabel->name));
    }
}

void IRGenerator::visit(WhileStmt& stmt) {
    std::shared_ptr<Operand> startLabel = createLabel();
    std::shared_ptr<Operand> condLabel = createLabel();
    std::shared_ptr<Operand> endLabel = createLabel();
    
    // 保存之前的break和continue标签
    breakLabels.push_back(endLabel->name);
    continueLabels.push_back(condLabel->name);
    
    // 跳转到条件判断
    addInstruction(std::make_shared<GotoInstr>(condLabel));
    
    // 循环体开始标签
    addInstruction(std::make_shared<LabelInstr>(startLabel->name));
    
    // 循环体
    stmt.body->accept(*this);
    
    // 条件判断标签
    addInstruction(std::make_shared<LabelInstr>(condLabel->name));
    
    // 条件表达式
    stmt.condition->accept(*this);
    std::shared_ptr<Operand> condition = getTopOperand();
    
    // 条件为真时跳转到循环体开始
    addInstruction(std::make_shared<IfGotoInstr>(condition, startLabel));
    
    // 循环结束标签
    addInstruction(std::make_shared<LabelInstr>(endLabel->name));
    
    // 恢复之前的break和continue标签
    breakLabels.pop_back();
    continueLabels.pop_back();
}

void IRGenerator::visit(BreakStmt&) {
    if (breakLabels.empty()) {
        std::cerr << "Error: Break statement outside of loop" << std::endl;
        return;
    }
    
    std::shared_ptr<Operand> target = std::make_shared<Operand>(OperandType::LABEL, breakLabels.back());
    addInstruction(std::make_shared<GotoInstr>(target));
}

void IRGenerator::visit(ContinueStmt&) {
    if (continueLabels.empty()) {
        std::cerr << "Error: Continue statement outside of loop" << std::endl;
        return;
    }
    
    std::shared_ptr<Operand> target = std::make_shared<Operand>(OperandType::LABEL, continueLabels.back());
    addInstruction(std::make_shared<GotoInstr>(target));
}

void IRGenerator::visit(ReturnStmt& stmt) {
    if (stmt.value) {
        stmt.value->accept(*this);
        std::shared_ptr<Operand> value = getTopOperand();
        
        addInstruction(std::make_shared<ReturnInstr>(value));
    } else {
        addInstruction(std::make_shared<ReturnInstr>());
    }
}

void IRGenerator::visit(FunctionDef& funcDef) {
    currentFunction = funcDef.name;
    
    // 函数开始
    addInstruction(std::make_shared<FunctionBeginInstr>(funcDef.name));
    
    // 进入新的作用域
    enterScope();

    // 函数参数
    for (const auto& param : funcDef.params) {
        getVariable(param.name); // 确保参数变量被创建
    }
    
    // 函数体
    funcDef.body->accept(*this);
    
    // 确保有返回指令
    if (funcDef.returnType == "void") {
        addInstruction(std::make_shared<ReturnInstr>());
    }
    
    // 离开作用域
    exitScope();

    // 函数结束
    addInstruction(std::make_shared<FunctionEndInstr>(funcDef.name));
}

void IRGenerator::visit(CompUnit& compUnit) {
    for (const auto& func : compUnit.functions) {
        func->accept(*this);
    }
}

// IR输出器实现
void IRPrinter::print(const std::vector<std::shared_ptr<IRInstr>>& instructions, std::ostream& out) {
    out << "# Intermediate Representation\n";
    
    for (const auto& instr : instructions) {
        out << instr->toString() << "\n";
    }
}