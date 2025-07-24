// irgen.cpp - 实现IR生成器和优化器
#include "irgen.h"
#include "ir.h"
#include <set>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
/**
 * IR生成和优化的实现
 * 
 * 本文件实现了irgen.h和ir.h中定义的方法，用于从抽象语法树(AST)生成
 * 中间表示(IR)代码，并对生成的IR进行优化。
 * 
 * 主要组件:
 * 1. IR指令的字符串表示方法
 * 2. 从AST节点生成IR
 * 3. 优化技术(常量折叠、常量传播等)
 * 4. 控制流分析
 * 5. 变量作用域管理
 */

//------------------------------------------------------------------------------
// 临时变量分析的方法
//------------------------------------------------------------------------------
// 判断操作数是否需要作为寄存器处理（临时变量或命名变量）
bool isProcessableReg(const Operand& op) 
{
    return op.type == OperandType::TEMP || op.type == OperandType::VARIABLE;
}

// 从单个操作数提取寄存器名（若非寄存器类型返回空）
std::vector<std::string> extractReg(const std::shared_ptr<Operand>& op) 
{
    if (op && isProcessableReg(*op)) {
        return {op->name};  // 返回变量名（无论VARIABLE还是TEMP）
    }
    return {};  // 忽略常量(CONSTANT)和标签(LABEL)
}

//多操作数合并
std::vector<std::string> collectRegs(
    const std::initializer_list<std::shared_ptr<Operand>>& ops) 
{
    std::vector<std::string> regs;
    for (const auto& op : ops) {
        auto r = extractReg(op);
        regs.insert(regs.end(), r.begin(), r.end());
    }
    return regs;
}

IRInstr::~IRInstr() = default;

std::vector<std::string> IRInstr::getDefRegisters() {
    return {};
}
std::vector<std::string> IRInstr::getUseRegisters() {
    return {};
}

//------------------------------------------------------------------------------
// IR指令字符串表示方法
//------------------------------------------------------------------------------

// Operand toString方法 - 将操作数转换为字符串表示
std::string Operand::toString() const {
    switch (type) {
        case OperandType::VARIABLE:
            return name;  // 变量名
        case OperandType::TEMP:
            return name;  // 临时变量名(如t0, t1等)
        case OperandType::CONSTANT:
            return std::to_string(value);  // 字面常量值
        case OperandType::LABEL:
            return name;  // 标签名
        default:
            return "unknown";  // 不应该发生
    }
}

// BinaryOpInstr toString方法  - 表示二元操作，如a = b + c
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
    // 格式: result = left op right
    return result->toString() + " = " + left->toString() + " " + opStr + " " + right->toString();
}

// UnaryOpInstr toString方法- 表示一元操作，如a = -b
std::string UnaryOpInstr::toString() const {
    std::string opStr;
    // 将操作码枚举转换为字符串表示
    switch (opcode) {
        case OpCode::NEG: opStr = "-"; break;  // 取负
        case OpCode::NOT: opStr = "!"; break;  // 逻辑非
        default: opStr = "unknown"; break;
    }
    
    // 格式: result = op operand
    return result->toString() + " = " + opStr + operand->toString();
}

// AssignInstr toString方法 - 表示赋值，如a = b
std::string AssignInstr::toString() const {
    return target->toString() + " = " + source->toString();
}

// GotoInstr toString方法 - 表示无条件跳转
std::string GotoInstr::toString() const {
    return "goto " + target->toString();
}

// IfGotoInstr toString方法 - 表示条件跳转
std::string IfGotoInstr::toString() const {
    return "if " + condition->toString() + " goto " + target->toString();
}

// ParamInstr toString方法 - 表示函数参数
std::string ParamInstr::toString() const {
    return "param " + param->toString();
}

// CallInstr toString方法 - 表示函数调用
std::string CallInstr::toString() const {
    if (result) {
        // 有返回值的函数调用: result = call func, paramCount
        return result->toString() + " = call " + funcName + ", " + std::to_string(paramCount);
    } else {
        // 无返回值的函数调用: call func, paramCount
        return "call " + funcName + ", " + std::to_string(paramCount);
    }
}

// ReturnInstr toString方法 - 表示返回语句
std::string ReturnInstr::toString() const {
    if (value) {
        // 有返回值: return value
        return "return " + value->toString();
    } else {
         // 无返回值: return
        return "return";
    }
}

// LabelInstr toString方法 - 表示标签
std::string LabelInstr::toString() const {
    return label + ":";
}

// FunctionBeginInstr toString方法 - 表示函数定义开始
std::string FunctionBeginInstr::toString() const {
    return "function " + funcName + " begin";
}

// FunctionEndInstr toString方法 - 表示函数定义结束
std::string FunctionEndInstr::toString() const {
    return "function " + funcName + " end";
}

//------------------------------------------------------------------------------
// IR生成器核心方法
//------------------------------------------------------------------------------

/**
 * 从AST生成IR。
 * 
 * 这是IR生成的主入口点。它遍历AST并生成相应的IR指令。
 * 如果启用了优化，还会对生成的IR应用各种优化技术。
 * 
 * @param ast AST的根节点
 */
void IRGenerator::generate(std::shared_ptr<CompUnit> ast) {
    if (ast) {
        // 遍历AST生成IR
        ast->accept(*this);

        // 如果启用了优化，则优化IR
        if (config.enableOptimizations) {
            optimize();
        }
    }
}

/**
 * 创建一个新的临时变量。
 * 
 * 临时变量用于存储表达式求值过程中的中间结果。
 * 
 * @return 新临时变量操作数的共享指针
 */
std::shared_ptr<Operand> IRGenerator::createTemp() {
    std::string name = "t" + std::to_string(tempCount++);
    return std::make_shared<Operand>(OperandType::TEMP, name);
}

/**
 * 创建一个新的标签。
 * 
 * 标签用作控制流指令中的跳转目标。
 * 
 * @return 新标签操作数的共享指针
 */
std::shared_ptr<Operand> IRGenerator::createLabel() {
    std::string name = "L" + std::to_string(labelCount++);
    return std::make_shared<Operand>(OperandType::LABEL, name);
}

/**
 * 将指令添加到IR指令列表。
 * 
 * @param instr 要添加的指令
 */
void IRGenerator::addInstruction(std::shared_ptr<IRInstr> instr) {
    instructions.push_back(instr);
}

/**
 * 从操作数栈获取顶部操作数。
 * 
 * 这在求值表达式时使用。表达式访问方法将其结果压入操作数栈，
 * 此方法用于获取这些结果。
 * 
 * @return 顶部操作数的共享指针
 */
std::shared_ptr<Operand> IRGenerator::getTopOperand() {
    if (operandStack.empty()) {
        std::cerr << "Error: Operand stack is empty" << std::endl;
        return std::make_shared<Operand>(0); // 默认返回常量0
    }
    
    std::shared_ptr<Operand> result = operandStack.back();
    operandStack.pop_back();
    return result;
}

//------------------------------------------------------------------------------
// 作用域管理方法
//------------------------------------------------------------------------------

/**
 * 进入新的变量作用域。
 * 
 * 在进入新的代码块、函数或源代码中的其他作用域定义结构时调用。
 */
void IRGenerator::enterScope() {
//------------------------修改7-------------------------------
    scopeDepth++;
//------------------------修改7-------------------------------
    scopeStack.push_back(std::map<std::string, std::shared_ptr<Operand>>());
}


/**
 * 退出当前变量作用域。
 * 
 * 在退出代码块、函数或其他作用域定义结构时调用。
 */
void IRGenerator::exitScope() {
//------------------------修改7-------------------------------
    scopeDepth--;
//------------------------修改7-------------------------------
    if (!scopeStack.empty()) {
        scopeStack.pop_back();
    }
}

/**
 * 在当前作用域中查找变量。
 * 
 * 这只在最内层作用域中查找，不在外层作用域中查找。
 * 
 * @param name 要查找的变量名
 * @return 变量操作数的共享指针，如果未找到则为nullptr
 */
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

/**
 * 在任何作用域中查找变量。
 * 
 * 按照词法作用域规则，从最内层到最外层作用域搜索。
 * 
 * @param name 要查找的变量名
 * @return 变量操作数的共享指针，如果未找到则为nullptr
 */
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

/**
 * 在当前作用域中定义变量。
 * 
 * @param name 要定义的变量名
 * @param var 变量操作数的共享指针
 */
void IRGenerator::defineVariable(const std::string& name, std::shared_ptr<Operand> var) {
    if (scopeStack.empty()) {
        enterScope();
    }
    
    scopeStack.back()[name] = var;
}

/**
 * 按名称获取或创建变量。
 * 
 * 如果变量存在于任何可访问的作用域中，则返回它。
 * 否则，创建一个新变量并添加到当前作用域。
 * 
 * @param name 要获取或创建的变量名
 * @return 变量操作数的共享指针
 */
/*std::shared_ptr<Operand> IRGenerator::getVariable(const std::string& name) {
    // 首先在现有作用域中查找变量
    std::shared_ptr<Operand> var = findVariable(name);
    if (var) {
        return var;
    }
    
    // 如果变量不存在，创建一个新的并添加到当前作用域
    var = std::make_shared<Operand>(OperandType::VARIABLE, name);
    defineVariable(name, var);
    return var;
}*/
//--------------------------修改7------------------------------
std::shared_ptr<Operand> IRGenerator::getVariable(const std::string& name, bool createInCurrentScope) {
    if (createInCurrentScope) {
        // 为变量声明：使用带作用域信息的唯一名称创建新变量
        std::string scopedName = getScopedVariableName(name);
        std::shared_ptr<Operand> var = std::make_shared<Operand>(OperandType::VARIABLE, scopedName);
        defineVariable(name, var);  // 在符号表中仍使用原始名称作为键
        return var;
    }
    
    // 为变量使用：在所有作用域中查找
    std::shared_ptr<Operand> var = findVariable(name);
    if (var) {
        return var;
    }
    
    // 变量不存在，创建新的（通常发生在函数参数）
    // 对于函数参数，使用原始名称，不生成唯一标识符
    var = std::make_shared<Operand>(OperandType::VARIABLE, name);  // 使用原始名称
    defineVariable(name, var);
    return var;
}
//--------------------------修改7------------------------------

/**
 * 将IR指令写入文件。
 * 
 * @param filename 要写入的文件名
 */
void IRGenerator::dumpIR(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
        return;
    }
    
    IRPrinter::print(instructions, outFile);
    outFile.close();
}

//------------------------------------------------------------------------------
// 优化方法
//------------------------------------------------------------------------------

/**
 * 优化生成的IR。
 * 
 * 对IR指令应用各种优化技术。
 */
void IRGenerator::optimize() {
    // 按顺序应用每种优化技术
    constantFolding();        // 在编译时评估常量表达式
    //constantPropagation();    // 在代码中传播常量值
    deadCodeElimination();    // 删除无效果的代码
    //controlFlowOptimization(); // 优化控制流（跳转、分支等）
}

/**
 * 常量折叠优化。
 * 
 * 在编译时评估常量表达式，用结果替换它们。
 * 例如，2 + 3 变成 5。
 */
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

/**
 * 常量传播优化。
 * 
 * 跟踪被赋予常量值的变量，并用常量替换这些变量的使用。
 * 这使得进一步的优化机会，如常量折叠成为可能。
 */
void IRGenerator::constantPropagation() {
    // 从变量名到其常量值的映射（如果已知）
    std::unordered_map<std::string, std::shared_ptr<Operand>> constants;
    bool changed = true;
    int maxIterations = 2; // 限制最大迭代次数
    int iteration = 0;
    
    // 重复直到没有更多变化
    while (changed) {
        changed = false;
        iteration++;
        
        // 遍历所有指令，查找常量赋值
        for (size_t i = 0; i < instructions.size(); ++i) {
            auto instr = instructions[i];
            
            // 如果是赋值语句
            if (auto assignInstr = std::dynamic_pointer_cast<AssignInstr>(instr)) {
                // 如果源操作数是常量，记录它
                if (assignInstr->source->type == OperandType::CONSTANT) {
                    if (assignInstr->target->type == OperandType::VARIABLE || 
                        assignInstr->target->type == OperandType::TEMP) {
                        constants[assignInstr->target->name] = assignInstr->source;
                    }
                } 
                // 如果源操作数是变量或临时变量，尝试替换为已知常量
                else if (assignInstr->source->type == OperandType::VARIABLE || 
                          assignInstr->source->type == OperandType::TEMP) {
                    // 如果从另一个已知具有常量值的变量赋值，传播它
                    //auto it = constants.find(assignInstr->source->name);
                    auto resolved = resolveConstant(assignInstr->source->name, constants);
                    if (resolved) {
                        // 替换为直接从常量赋值
                        assignInstr->source = resolved;
                        changed = true;
                    }
                }
                
                // 目标变量可能不再是常量，除非我们刚刚给它赋了常量值
                if (assignInstr->source->type != OperandType::CONSTANT) {
                    constants.erase(assignInstr->target->name);
                }
            }
            // 对于二元操作，用已知常量替换变量
            else if (auto binOp = std::dynamic_pointer_cast<BinaryOpInstr>(instr)) {
                // 尝试替换左操作数
                if (binOp->left->type == OperandType::VARIABLE || 
                    binOp->left->type == OperandType::TEMP) {
                    //auto it = constants.find(binOp->left->name);
                    auto resolved = resolveConstant(binOp->left->name, constants);
                    if (resolved) {
                        binOp->left = resolved;
                        changed = true;
                    }
                }
                
                // 尝试替换右操作数
                if (binOp->right->type == OperandType::VARIABLE || 
                    binOp->right->type == OperandType::TEMP) {
                    //auto it = constants.find(binOp->right->name);
                    auto resolved = resolveConstant(binOp->right->name, constants);
                    if (resolved) {
                        binOp->right = resolved;
                        changed = true;
                    }
                }
                
                // 结果变量不再是常量
                constants.erase(binOp->result->name);
            }
            // 对于一元操作，用已知常量替换变量
            else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpInstr>(instr)) {
                if (unaryOp->operand->type == OperandType::VARIABLE || 
                    unaryOp->operand->type == OperandType::TEMP) {
                    //auto it = constants.find(unaryOp->operand->name);
                    auto resolved = resolveConstant(unaryOp->operand->name, constants);
                    if (resolved) {
                        unaryOp->operand = resolved;
                        changed = true;
                    }
                }
                
                // 结果变量不再是常量
                constants.erase(unaryOp->result->name);
            }
            // 对于函数调用、参数和返回，如果已知则使用常量
            else if (auto callInstr = std::dynamic_pointer_cast<CallInstr>(instr)) {
                // 结果变量不再是常量
                if (callInstr->result) {
                    constants.erase(callInstr->result->name);
                }
            }
            else if (auto paramInstr = std::dynamic_pointer_cast<ParamInstr>(instr)) {
                if (paramInstr->param->type == OperandType::VARIABLE || 
                    paramInstr->param->type == OperandType::TEMP) {
                    //auto it = constants.find(paramInstr->param->name);
                    auto resolved = resolveConstant(paramInstr->param->name, constants);
                    if (resolved) {
                        paramInstr->param = resolved;
                        changed = true;
                    }
                }
            }
            else if (auto returnInstr = std::dynamic_pointer_cast<ReturnInstr>(instr)) {
                if (returnInstr->value && 
                   (returnInstr->value->type == OperandType::VARIABLE || 
                    returnInstr->value->type == OperandType::TEMP)) {
                    auto resolved = resolveConstant(returnInstr->value->name, constants);
                    if (resolved) {
                        returnInstr->value = resolved;
                        changed = true;
                    }
                }
            }
            else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
                if (ifGotoInstr->condition->type == OperandType::VARIABLE || 
                    ifGotoInstr->condition->type == OperandType::TEMP) {
                    //auto it = constants.find(ifGotoInstr->condition->name);
                    auto resolved = resolveConstant(ifGotoInstr->condition->name, constants);
                    if (resolved) {
                        ifGotoInstr->condition = resolved;
                        changed = true;
                    }
                }
            }
        }
        
        // 如果我们做了更改，再次运行常量折叠
        if (changed) {
            constantFolding();
        }

        if (iteration >= maxIterations)  break;
        
    }
}

const int MAX_PROPAGATION_DEPTH = 20;
std::shared_ptr<Operand> IRGenerator::resolveConstant(
    const std::string& name,
    std::unordered_map<std::string, std::shared_ptr<Operand>>& constants,
    int depth
) {
    if (depth > MAX_PROPAGATION_DEPTH) return nullptr; // 防止无限传播
    auto it = constants.find(name);
    if (it == constants.end()) return nullptr;
    auto operand = it->second;
    if (operand->type == OperandType::CONSTANT) return operand;
    if (operand->type == OperandType::VARIABLE || operand->type == OperandType::TEMP) {
        return resolveConstant(operand->name, constants, depth + 1);
    }
    return nullptr;
}



/**
 * 死代码消除优化。
 * 
 * 删除对程序输出没有影响的指令。
 * 例如，对从未使用的变量的赋值。
 */
void IRGenerator::deadCodeElimination() {
    // 首先，识别使用的变量
    std::unordered_set<std::string> usedVars;
    
    // 第一遍：识别所有使用的变量
    for (const auto& instr : instructions) {
        auto usedInInstr = IRAnalyzer::getUsedVariables(instr);
        for (const auto& var : usedInInstr) {
            usedVars.insert(var);
        }
    }
    
    // 现在标记所有具有副作用（调用、返回、跳转等）的指令
    // 或定义被使用的变量
    std::vector<bool> isLive(instructions.size(), false);
    
    // 第二遍：标记活跃指令
    for (size_t i = 0; i < instructions.size(); ++i) {
        auto instr = instructions[i];
        
        // 具有副作用的指令总是活跃的
        if (std::dynamic_pointer_cast<CallInstr>(instr) ||
            std::dynamic_pointer_cast<ReturnInstr>(instr) ||
            std::dynamic_pointer_cast<GotoInstr>(instr) ||
            std::dynamic_pointer_cast<IfGotoInstr>(instr) ||
            std::dynamic_pointer_cast<LabelInstr>(instr) ||
            std::dynamic_pointer_cast<FunctionBeginInstr>(instr) ||
            std::dynamic_pointer_cast<FunctionEndInstr>(instr) ||
            std::dynamic_pointer_cast<ParamInstr>(instr)) {
            isLive[i] = true;
        }
        // 定义被使用变量的指令是活跃的
        else {
            auto definedVars = IRAnalyzer::getDefinedVariables(instr);
            for (const auto& var : definedVars) {
                if (usedVars.find(var) != usedVars.end()) {
                    isLive[i] = true;
                    break;
                }
            }
        }
    }
    
    // 第三遍：只保留活跃指令
    std::vector<std::shared_ptr<IRInstr>> newInstructions;
    for (size_t i = 0; i < instructions.size(); ++i) {
        if (isLive[i]) {
            newInstructions.push_back(instructions[i]);
        }
    }
    
    instructions = newInstructions;
}

/**
 * 控制流优化。
 * 
 * 优化程序的控制流，包括跳转和分支。
 * 例如，消除冗余跳转，优化分支条件，
 * 简化控制流模式。
 */
void IRGenerator::controlFlowOptimization() {
    bool changed = true;
    
    while (changed) {
        changed = false;
        
        // 构建控制流图
        auto cfg = buildControlFlowGraph();
        
        // 优化跳转到跳转（跳转链）
        for (size_t i = 0; i < instructions.size(); ++i) {
            auto instr = instructions[i];
            
            // 如果这是一个跳转指令
            if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(instr)) {
                std::string targetLabel = gotoInstr->target->name;
                
                // 查找目标块
                if (cfg.find(targetLabel) != cfg.end()) {
                    auto& targetBlock = cfg[targetLabel];
                    
                    // 如果目标块只包含另一个跳转，绕过它
                    if (targetBlock.instructionIndices.size() == 1) {
                        auto targetInstr = instructions[targetBlock.instructionIndices[0]];
                        if (auto targetGoto = std::dynamic_pointer_cast<GotoInstr>(targetInstr)) {
                            // 将此跳转替换为跳转到最终目标
                            gotoInstr->target = targetGoto->target;
                            changed = true;
                        }
                    }
                }
            }
            
            // 如果这是一个条件跳转指令
            else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
                // 如果条件是常量，我们可以简化
                if (ifGotoInstr->condition->type == OperandType::CONSTANT) {
                    if (ifGotoInstr->condition->value != 0) {
                        // 始终为真条件 - 替换为无条件跳转
                        instructions[i] = std::make_shared<GotoInstr>(ifGotoInstr->target);
                        changed = true;
                    } else {
                        // 始终为假条件 - 删除跳转
                        // 我们将在死代码消除中处理
                        // 现在，只是替换为空操作（自我赋值）
                        auto noop = std::make_shared<AssignInstr>(
                            std::make_shared<Operand>(OperandType::TEMP, "noop"),
                            std::make_shared<Operand>(OperandType::TEMP, "noop")
                        );
                        instructions[i] = noop;
                        changed = true;
                    }
                }
            }
        }
        
        // 如果我们做了更改，再次运行死代码消除
        if (changed) {
            deadCodeElimination();
        }
    }
}

/**
 * 检查指令是否是控制流指令。
 * 
 * 控制流指令改变执行流程，如跳转和返回。
 * 
 * @param instr 要检查的指令
 * @return 如果指令是控制流指令，则为true
 */
bool IRGenerator::isControlFlowInstruction(const std::shared_ptr<IRInstr>& instr) const {
    return std::dynamic_pointer_cast<GotoInstr>(instr) != nullptr ||
           std::dynamic_pointer_cast<IfGotoInstr>(instr) != nullptr ||
           std::dynamic_pointer_cast<ReturnInstr>(instr) != nullptr;
}

/**
 * 获取控制流指令的目标标签。
 * 
 * @param instr 控制流指令
 * @return 目标标签名的向量
 */
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

/**
 * 从IR指令构建控制流图。
 * 
 * 控制流图表示程序中的执行流程。
 * 每个节点（基本块）是按顺序执行的一系列指令，
 * 边表示块之间可能的控制流。
 * 
 * @return 从标签名到基本块的映射
 */
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

//------------------------------------------------------------------------------
// AST访问者方法（表达式节点）
//------------------------------------------------------------------------------

/**
 * 访问数字表达式。
 * 
 * 创建一个常量操作数并将其推入操作数栈。
 * 
 * @param expr 数字表达式
 */
void IRGenerator::visit(NumberExpr& expr) {
    std::shared_ptr<Operand> constant = std::make_shared<Operand>(expr.value);
    operandStack.push_back(constant);
}

/**
 * 访问变量表达式。
 * 
 * 获取变量操作数并将其推入操作数栈。
 * 
 * @param expr 变量表达式
 */
void IRGenerator::visit(VariableExpr& expr) {
    std::shared_ptr<Operand> var = getVariable(expr.name);
    if (!var) return; // 错误已输出
    operandStack.push_back(var);
}

/**
 * 访问二元表达式。
 * 
 * 评估两个操作数，执行二元操作，并将结果推入操作数栈。
 * 
 * @param expr 二元表达式
 */
void IRGenerator::visit(BinaryExpr& expr) {
    // 处理逻辑运算符的短路求值
    if (expr.op == "&&") {
        auto result = generateShortCircuitAnd(expr);
        operandStack.push_back(result);
        return;
    } else if (expr.op == "||") {
        auto result = generateShortCircuitOr(expr);
        operandStack.push_back(result);
        return;
    }
    // 正常二元表达式求值
    expr.right->accept(*this);
    std::shared_ptr<Operand> right = getTopOperand();
    
    expr.left->accept(*this);
    std::shared_ptr<Operand> left = getTopOperand();
    
    std::shared_ptr<Operand> result = createTemp();
    // 将运算符字符串映射到操作码
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

/**
 * 为逻辑AND生成短路求值。
 * 
 * 首先评估左操作数。如果为假，结果为假，
 * 无需评估右操作数。否则，结果为右操作数的值。
 * 
 * @param expr 二元表达式
 * @return 结果操作数
 */
std::shared_ptr<Operand> IRGenerator::generateShortCircuitAnd(BinaryExpr& expr) {
    // 评估左操作数
    expr.left->accept(*this);
    std::shared_ptr<Operand> left = getTopOperand();

    // 创建结果临时变量和短路标签
    std::shared_ptr<Operand> result = createTemp();
    std::shared_ptr<Operand> shortCircuitLabel = createLabel();
    std::shared_ptr<Operand> endLabel = createLabel();

    // 如果左操作数为假（0），短路
    // 因为IfGotoInstr在条件为真时跳转，所以需要翻转条件
    //addInstruction(std::make_shared<IfGotoInstr>(left, shortCircuitLabel));
    // 创建左操作数的否定
    std::shared_ptr<Operand> notLeft = createTemp();
    addInstruction(std::make_shared<UnaryOpInstr>(OpCode::NOT, notLeft, left));

    // 如果左操作数为假（0），短路
    addInstruction(std::make_shared<IfGotoInstr>(notLeft, shortCircuitLabel));

    // 左操作数为真，评估右操作数
    expr.right->accept(*this);
    std::shared_ptr<Operand> right = getTopOperand();

    // 结果为右操作数
    addInstruction(std::make_shared<AssignInstr>(result, right));
    addInstruction(std::make_shared<GotoInstr>(endLabel));

    // 短路：结果为假（0）
    addInstruction(std::make_shared<LabelInstr>(shortCircuitLabel->name));
    addInstruction(std::make_shared<AssignInstr>(result, std::make_shared<Operand>(0)));

    // 结束
    addInstruction(std::make_shared<LabelInstr>(endLabel->name));
    return result;
}

/**
 * 为逻辑OR生成短路求值。
 * 
 * 首先评估左操作数。如果为真，结果为真，
 * 无需评估右操作数。否则，结果为右操作数的值。
 * 
 * @param expr 二元表达式
 * @return 结果操作数
 */
std::shared_ptr<Operand> IRGenerator::generateShortCircuitOr(BinaryExpr& expr) {
    // 评估左操作数
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

/**
 * 访问一元表达式。
 * 
 * 评估操作数，执行一元操作，并将结果推入操作数栈。
 * 
 * @param expr 一元表达式
 */
void IRGenerator::visit(UnaryExpr& expr) {
    expr.operand->accept(*this);
    std::shared_ptr<Operand> operand = getTopOperand();
    
    std::shared_ptr<Operand> result = createTemp();
    
    // 处理不同的一元运算符
    if (expr.op == "-") {
        // 取负
        addInstruction(std::make_shared<UnaryOpInstr>(OpCode::NEG, result, operand));
    } else if (expr.op == "!") {
        // 逻辑非
        addInstruction(std::make_shared<UnaryOpInstr>(OpCode::NOT, result, operand));
    } else if (expr.op == "+") {
        // 一元加（无效果）
        addInstruction(std::make_shared<AssignInstr>(result, operand));
    } else {
        std::cerr << "错误: 未知的一元运算符: " << expr.op << std::endl;
        // 默认为取负
        addInstruction(std::make_shared<UnaryOpInstr>(OpCode::NEG, result, operand));
    }
    
    operandStack.push_back(result);
}

/**
 * 访问函数调用表达式。
 * 
 * 评估参数，将其传递给函数，调用函数，
 * 并将结果推入操作数栈。
 * 
 * @param expr 函数调用表达式
 */
void IRGenerator::visit(CallExpr& expr) {
    // 处理参数
    std::vector<std::shared_ptr<Operand>> args;
    for (const auto& arg : expr.arguments) {
        arg->accept(*this);
        args.push_back(getTopOperand());
    }
    
    // 从左到右添加参数指令（与RISC-V调用约定一致）
    for (const auto& arg : args) {
        addInstruction(std::make_shared<ParamInstr>(arg));
    }
    
    // 为结果创建临时变量
    std::shared_ptr<Operand> result = createTemp();
    
    // 创建调用指令
    auto callInstr = std::make_shared<CallInstr>(
        result, expr.callee, expr.arguments.size());
    
    // 存储参数列表，便于代码生成
    callInstr->params = args;
    
    // 调用函数
    addInstruction(callInstr);
    
    // 记录函数被使用
    markFunctionAsUsed(expr.callee);
    
    // 将结果推入操作数栈
    operandStack.push_back(result);
}

//------------------------------------------------------------------------------
// AST访问者方法（语句节点）
//------------------------------------------------------------------------------

/**
 * 访问表达式语句。
 * 
 * 评估表达式并丢弃结果。
 * 
 * @param stmt 表达式语句
 */
void IRGenerator::visit(ExprStmt& stmt) {
    if (stmt.expression) {
        stmt.expression->accept(*this);
        // 表达式语句的结果会被丢弃
        if (!operandStack.empty()) {
            operandStack.pop_back();
        }
    }
}

/**
 * 访问变量声明语句。
 * 
 * 创建新变量，如果提供了初始化器，则进行初始化。
 * 
 * @param stmt 变量声明语句
 */
/*void IRGenerator::visit(VarDeclStmt& stmt) {
    std::shared_ptr<Operand> var = getVariable(stmt.name);
    
    if (stmt.initializer) {
        stmt.initializer->accept(*this);
        std::shared_ptr<Operand> value = getTopOperand();
        
        addInstruction(std::make_shared<AssignInstr>(var, value));
    }
}*/
//---------------------------修改7-----------------------------
void IRGenerator::visit(VarDeclStmt& stmt) {
    // 关键修改：使用 createInCurrentScope = true，强制在当前作用域创建新变量
    std::shared_ptr<Operand> var = getVariable(stmt.name, true);
    
    if (stmt.initializer) {
        stmt.initializer->accept(*this);
        std::shared_ptr<Operand> value = getTopOperand();
        
        addInstruction(std::make_shared<AssignInstr>(var, value));
    }
}
//---------------------------修改7-----------------------------
/**
 * 访问赋值语句。
 * 
 * 评估右侧并将其赋值给变量。
 * 
 * @param stmt 赋值语句
 */
void IRGenerator::visit(AssignStmt& stmt) {
    // 评估右侧
    stmt.value->accept(*this);
    std::shared_ptr<Operand> value = getTopOperand();
    
    // 获取变量
    std::shared_ptr<Operand> var = getVariable(stmt.name);
    
    // 将值赋给变量
    addInstruction(std::make_shared<AssignInstr>(var, value));
}

/**
 * 访问块语句。
 * 
 * 创建新作用域并执行块中的语句。
 * 
 * @param stmt 块语句
 */
void IRGenerator::visit(BlockStmt& stmt) {
    // 进入新的作用域
    enterScope();
    
    for (const auto& s : stmt.statements) {
        s->accept(*this);
    }
    
    // 离开作用域
    exitScope();
}

/**
 * 访问if语句。
 * 
 * 评估条件并执行then分支或else分支（如果存在）。
 * 
 * @param stmt if语句
 */
void IRGenerator::visit(IfStmt& stmt) {
    // 为else分支和结束创建标签
    std::shared_ptr<Operand> elseLabel = createLabel();
    std::shared_ptr<Operand> endLabel = stmt.elseBranch ? createLabel() : elseLabel;
    
    // 评估条件
    stmt.condition->accept(*this);
    std::shared_ptr<Operand> condition = getTopOperand();
    
    // 如果条件为假，跳转到else分支
    // 注意：IfGotoInstr在条件为真时跳转，所以我们需要翻转逻辑
    // 我们想要的是：if (!condition) goto elseLabel
    // 但IfGotoInstr是：if (condition) goto target
    // 所以我们需要使用一个临时变量来存储!condition
    std::shared_ptr<Operand> notCondition = createTemp();
    addInstruction(std::make_shared<UnaryOpInstr>(OpCode::NOT, notCondition, condition));
    addInstruction(std::make_shared<IfGotoInstr>(notCondition, elseLabel));
    
    // 为then分支生成代码
    stmt.thenBranch->accept(*this);
    
    // 如果有else分支，在then分支之后添加跳转到结束
    if (stmt.elseBranch) {
        addInstruction(std::make_shared<GotoInstr>(endLabel));
        
        // 添加else标签
        addInstruction(std::make_shared<LabelInstr>(elseLabel->name));
        
        // 为else分支生成代码
        stmt.elseBranch->accept(*this);
        
        // 添加结束标签
        addInstruction(std::make_shared<LabelInstr>(endLabel->name));
    } else {
        // 没有else分支，只添加else/end标签
        addInstruction(std::make_shared<LabelInstr>(elseLabel->name));
    }
}

/**
 * 访问while语句。
 * 
 * 评估条件并重复执行主体，只要条件为真。
 * 
 * @param stmt while语句
 */
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

/**
 * 访问break语句。
 * 
 * 生成跳转到当前循环的结束。
 * 
 * @param stmt break语句
 */
void IRGenerator::visit(BreakStmt&) {
    if (breakLabels.empty()) {
        std::cerr << "Error: Break statement outside of loop" << std::endl;
        return;
    }
    
    std::shared_ptr<Operand> target = std::make_shared<Operand>(OperandType::LABEL, breakLabels.back());
    addInstruction(std::make_shared<GotoInstr>(target));
}

/**
 * 访问continue语句。
 * 
 * 生成跳转到当前循环的条件检查。
 * 
 * @param stmt continue语句
 */
void IRGenerator::visit(ContinueStmt&) {
    if (continueLabels.empty()) {
        std::cerr << "Error: Continue statement outside of loop" << std::endl;
        return;
    }
    
    std::shared_ptr<Operand> target = std::make_shared<Operand>(OperandType::LABEL, continueLabels.back());
    addInstruction(std::make_shared<GotoInstr>(target));
}

/**
 * 访问return语句。
 * 
 * 评估返回值（如果有）并生成返回指令。
 * 
 * @param stmt return语句
 */
void IRGenerator::visit(ReturnStmt& stmt) {
    if (stmt.value) {
        stmt.value->accept(*this);
        std::shared_ptr<Operand> value = getTopOperand();
        
        addInstruction(std::make_shared<ReturnInstr>(value));
    } else {
        addInstruction(std::make_shared<ReturnInstr>());
    }
}

//------------------------------------------------------------------------------
// AST访问者方法（声明节点）
//------------------------------------------------------------------------------

/**
 * 访问函数定义。
 * 
 * 为函数生成代码，包括其参数和主体。
 * 
 * @param funcDef 函数定义
 */
void IRGenerator::visit(FunctionDef& funcDef) {
    currentFunction = funcDef.name;
    currentFunctionReturnType = funcDef.returnType;

    // 函数开始
    auto funcBeginInstr = std::make_shared<FunctionBeginInstr>(funcDef.name, funcDef.returnType);


    // 添加参数名列表
    for (const auto& param : funcDef.params) {
        funcBeginInstr->paramNames.push_back(param.name);
    }
    
    addInstruction(funcBeginInstr);
  
    // 进入新的作用域
    enterScope();

    // 函数参数
    // for (const auto& param : funcDef.params) {
    //    getVariable(param.name); // 确保参数变量被创建
    // }
    
//--------------------------修改7----------------------------
    // 函数参数处理 - 关键修改：使用 createInCurrentScope = false
    for (const auto& param : funcDef.params) {
        // 对于函数参数，使用 createInCurrentScope = false，
        // 这样会使用原始名称，不会生成唯一标识符
        getVariable(param.name, false);  // 改为 false！
    }
//--------------------------修改7-----------------------------

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

/**
 * 访问编译单元。
 * 
 * 为编译单元中的所有函数生成代码。
 * 
 * @param compUnit 编译单元
 */
void IRGenerator::visit(CompUnit& compUnit) {
    for (const auto& func : compUnit.functions) {
        func->accept(*this);
    }
}

//------------------------------------------------------------------------------
// IR打印器实现
//------------------------------------------------------------------------------

/**
 * 将IR指令打印到流。
 * 
 * @param instructions 要打印的IR指令
 * @param out 输出流
 */
void IRPrinter::print(const std::vector<std::shared_ptr<IRInstr>>& instructions, std::ostream& out) {
    out << "# Intermediate Representation\n";
    
    for (const auto& instr : instructions) {
        out << instr->toString() << "\n";
    }
}

//------------------------------------------------------------------------------
// IR分析器实现
//------------------------------------------------------------------------------

/**
 * 查找定义变量的指令。
 * 
 * @param instructions 要搜索的IR指令
 * @param operandName 要查找的变量名
 * @return 定义指令的索引，如果未找到则为-1
 */
int IRAnalyzer::findDefinition(const std::vector<std::shared_ptr<IRInstr>>& instructions, 
                              const std::string& operandName) {
    for (int i = 0; i < instructions.size(); ++i) {
        auto instr = instructions[i];
        
        // 检查是否定义了该操作数
        auto definedVars = getDefinedVariables(instr);
        if (std::find(definedVars.begin(), definedVars.end(), operandName) != definedVars.end()) {
            return i;
        }
    }
    
    return -1; // 未找到定义
}

/**
 * 查找使用变量的所有指令。
 * 
 * @param instructions 要搜索的IR指令
 * @param operandName 要查找的变量名
 * @return 使用变量的指令索引向量
 */
std::vector<int> IRAnalyzer::findUses(const std::vector<std::shared_ptr<IRInstr>>& instructions, 
                                    const std::string& operandName) {
    std::vector<int> uses;
    
    for (int i = 0; i < instructions.size(); ++i) {
        auto instr = instructions[i];
        
        // 检查是否使用了该操作数
        auto usedVars = getUsedVariables(instr);
        if (std::find(usedVars.begin(), usedVars.end(), operandName) != usedVars.end()) {
            uses.push_back(i);
        }
    }
    
    return uses;
}

/**
 * 检查变量在给定位置是否活跃。
 * 
 * 如果变量在位置之后使用且未重新定义，则它是活跃的。
 * 
 * @param instructions 要搜索的IR指令
 * @param varName 要检查的变量名
 * @param position 要检查的位置
 * @return 如果变量在位置处活跃则为true
 */
bool IRAnalyzer::isVariableLive(const std::vector<std::shared_ptr<IRInstr>>& instructions,
                               const std::string& varName,
                               int position) {
    // 如果变量在position之后被使用，则认为它是活跃的
    for (int i = position + 1; i < instructions.size(); ++i) {
        auto usedVars = getUsedVariables(instructions[i]);
        if (std::find(usedVars.begin(), usedVars.end(), varName) != usedVars.end()) {
            return true;
        }
        
        // 如果变量在这条指令中被重新定义，则之前的值不再活跃
        auto definedVars = getDefinedVariables(instructions[i]);
        if (std::find(definedVars.begin(), definedVars.end(), varName) != definedVars.end()) {
            return false;
        }
    }
    
    return false; // 变量不再被使用
}

/**
 * 获取指令定义的所有变量。
 * 
 * @param instr 要检查的指令
 * @return 指令定义的变量名向量
 */
std::vector<std::string> IRAnalyzer::getDefinedVariables(const std::shared_ptr<IRInstr>& instr) {
    std::vector<std::string> definedVars;
    
    if (auto binaryOp = std::dynamic_pointer_cast<BinaryOpInstr>(instr)) {
        if (binaryOp->result) {
            definedVars.push_back(binaryOp->result->name);
        }
    }
    else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpInstr>(instr)) {
        if (unaryOp->result) {
            definedVars.push_back(unaryOp->result->name);
        }
    }
    else if (auto assignInstr = std::dynamic_pointer_cast<AssignInstr>(instr)) {
        if (assignInstr->target) {
            definedVars.push_back(assignInstr->target->name);
        }
    }
    else if (auto callInstr = std::dynamic_pointer_cast<CallInstr>(instr)) {
        if (callInstr->result) {
            definedVars.push_back(callInstr->result->name);
        }
    }
    
    return definedVars;
}

/**
 * 获取指令使用的所有变量。
 * 
 * @param instr 要检查的指令
 * @return 指令使用的变量名向量
 */
std::vector<std::string> IRAnalyzer::getUsedVariables(const std::shared_ptr<IRInstr>& instr) {
    std::vector<std::string> usedVars;
    
    if (auto binaryOp = std::dynamic_pointer_cast<BinaryOpInstr>(instr)) {
        if (binaryOp->left && binaryOp->left->type != OperandType::CONSTANT) {
            usedVars.push_back(binaryOp->left->name);
        }
        if (binaryOp->right && binaryOp->right->type != OperandType::CONSTANT) {
            usedVars.push_back(binaryOp->right->name);
        }
    }
    else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpInstr>(instr)) {
        if (unaryOp->operand && unaryOp->operand->type != OperandType::CONSTANT) {
            usedVars.push_back(unaryOp->operand->name);
        }
    }
    else if (auto assignInstr = std::dynamic_pointer_cast<AssignInstr>(instr)) {
        if (assignInstr->source && assignInstr->source->type != OperandType::CONSTANT) {
            usedVars.push_back(assignInstr->source->name);
        }
    }
    else if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(instr)) {
        // 标签不算变量使用
    }
    else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
        if (ifGotoInstr->condition && ifGotoInstr->condition->type != OperandType::CONSTANT) {
            usedVars.push_back(ifGotoInstr->condition->name);
        }
    }
    else if (auto paramInstr = std::dynamic_pointer_cast<ParamInstr>(instr)) {
        if (paramInstr->param && paramInstr->param->type != OperandType::CONSTANT) {
            usedVars.push_back(paramInstr->param->name);
        }
    }
    else if (auto returnInstr = std::dynamic_pointer_cast<ReturnInstr>(instr)) {
        if (returnInstr->value && returnInstr->value->type != OperandType::CONSTANT) {
            usedVars.push_back(returnInstr->value->name);
        }
    }
    
    return usedVars;
}
/**
 * 检查函数是否被使用
 * 
 * @param instructions 要搜索的IR指令
 * @param funcName 要检查的函数名
 * @return 如果函数被使用则为true
 */
bool IRAnalyzer::isFunctionUsed(const std::vector<std::shared_ptr<IRInstr>>& instructions,
                              const std::string& funcName) {
    // 如果是main函数，总是被使用
    if (funcName == "main") {
        return true;
    }
    
    for (const auto& instr : instructions) {
        if (auto callInstr = std::dynamic_pointer_cast<CallInstr>(instr)) {
            if (callInstr->funcName == funcName) {
                return true;
            }
        }
    }
    
    return false;
}

void IRGenerator::markFunctionAsUsed(const std::string& funcName) {
    usedFunctions.insert(funcName);
}