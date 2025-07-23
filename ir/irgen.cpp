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
    constantPropagation();    // 在代码中传播常量值
    enhancedCommonSubexpressionElimination(); // 消除公共子表达式
    deadCodeElimination();    // 删除无效果的代码
    controlFlowOptimization(); // 优化控制流（跳转、分支等）
}

 //--------------------增强常量折叠-----------------------   
/**
 * 常量折叠优化。
 * 
 * 在编译时评估常量表达式，用结果替换它们。
 * 例如，2 + 3 变成 5。
 */
/*void IRGenerator::constantFolding() {
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
}*/
/**
 * 增强版常量折叠优化
 * 
 * 改进包括：
 * 1. 迭代优化直到收敛
 * 2. 支持更多表达式类型
 * 3. 更好的错误处理
 * 4. 统计优化效果
 */
void IRGenerator::constantFolding() {
    bool changed = true;
    int optimizationRound = 0;
    int totalOptimizations = 0;
    
    // 迭代优化直到没有更多改变
    while (changed && optimizationRound < 10) { // 限制最大迭代次数防止无限循环
        changed = false;
        optimizationRound++;
        int roundOptimizations = 0;
        
        // 遍历所有指令进行常量折叠
        for (size_t i = 0; i < instructions.size(); ++i) {
            auto instr = instructions[i];
            
            // 处理二元运算
            if (auto binOp = std::dynamic_pointer_cast<BinaryOpInstr>(instr)) {
                if (binOp->left->type == OperandType::CONSTANT && 
                    binOp->right->type == OperandType::CONSTANT) {
                    
                    int result = 0;
                    bool canFold = true;
                    
                    // 根据操作类型计算结果
                    switch (binOp->opcode) {
                        case OpCode::ADD: 
                            result = binOp->left->value + binOp->right->value; 
                            break;
                        case OpCode::SUB: 
                            result = binOp->left->value - binOp->right->value; 
                            break;
                        case OpCode::MUL: 
                            result = binOp->left->value * binOp->right->value; 
                            break;
                        case OpCode::DIV: 
                            if (binOp->right->value == 0) {
                                canFold = false; // 避免除以零
                                std::cerr << "Warning: Division by zero in constant expression" << std::endl;
                            } else {
                                result = binOp->left->value / binOp->right->value;
                            }
                            break;
                        case OpCode::MOD: 
                            if (binOp->right->value == 0) {
                                canFold = false; // 避免模零
                                std::cerr << "Warning: Modulo by zero in constant expression" << std::endl;
                            } else {
                                result = binOp->left->value % binOp->right->value;
                            }
                            break;
                        case OpCode::LT: 
                            result = binOp->left->value < binOp->right->value ? 1 : 0; 
                            break;
                        case OpCode::GT: 
                            result = binOp->left->value > binOp->right->value ? 1 : 0; 
                            break;
                        case OpCode::LE: 
                            result = binOp->left->value <= binOp->right->value ? 1 : 0; 
                            break;
                        case OpCode::GE: 
                            result = binOp->left->value >= binOp->right->value ? 1 : 0; 
                            break;
                        case OpCode::EQ: 
                            result = binOp->left->value == binOp->right->value ? 1 : 0; 
                            break;
                        case OpCode::NE: 
                            result = binOp->left->value != binOp->right->value ? 1 : 0; 
                            break;
                        case OpCode::AND: 
                            result = (binOp->left->value && binOp->right->value) ? 1 : 0; 
                            break;
                        case OpCode::OR: 
                            result = (binOp->left->value || binOp->right->value) ? 1 : 0; 
                            break;
                        default: 
                            canFold = false; 
                            break;
                    }
                    
                    if (canFold) {
                        // 用赋值指令替换原二元操作指令
                        auto constResult = std::make_shared<Operand>(result);
                        auto assignInstr = std::make_shared<AssignInstr>(binOp->result, constResult);
                        instructions[i] = assignInstr;
                        changed = true;
                        roundOptimizations++;
                    }
                }
            }
            // 处理一元运算
            else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpInstr>(instr)) {
                if (unaryOp->operand->type == OperandType::CONSTANT) {
                    int result = 0;
                    bool canFold = true;
                    
                    // 根据操作类型计算结果
                    switch (unaryOp->opcode) {
                        case OpCode::NEG: 
                            result = -unaryOp->operand->value; 
                            break;
                        case OpCode::NOT: 
                            result = !unaryOp->operand->value ? 1 : 0; 
                            break;
                        default: 
                            canFold = false; 
                            break;
                    }
                    
                    if (canFold) {
                        // 用赋值指令替换原一元操作指令
                        auto constResult = std::make_shared<Operand>(result);
                        auto assignInstr = std::make_shared<AssignInstr>(unaryOp->result, constResult);
                        instructions[i] = assignInstr;
                        changed = true;
                        roundOptimizations++;
                    }
                }
            }
            // 处理赋值指令中的冗余（常量到常量的赋值）
            else if (auto assignInstr = std::dynamic_pointer_cast<AssignInstr>(instr)) {
                if (assignInstr->source->type == OperandType::CONSTANT && 
                    assignInstr->target->type == OperandType::CONSTANT) {
                    // 这种情况通常不应该发生，但如果发生了，我们可以标记为优化机会
                    // 暂时不做处理，因为这可能表示代码生成有问题
                }
            }
            // 处理条件跳转中的常量条件
            else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
                if (ifGotoInstr->condition->type == OperandType::CONSTANT) {
                    if (ifGotoInstr->condition->value != 0) {
                        // 条件始终为真 - 替换为无条件跳转
                        instructions[i] = std::make_shared<GotoInstr>(ifGotoInstr->target);
                        changed = true;
                        roundOptimizations++;
                    } else {
                        // 条件始终为假 - 创建一个无操作指令（将在死代码消除中移除）
                        auto noop = std::make_shared<AssignInstr>(
                            std::make_shared<Operand>(OperandType::TEMP, "noop_" + std::to_string(i)),
                            std::make_shared<Operand>(0)
                        );
                        instructions[i] = noop;
                        changed = true;
                        roundOptimizations++;
                    }
                }
            }
        }
        
        totalOptimizations += roundOptimizations;
        
        // 输出优化统计信息（在调试模式下）
        if (config.generateDebugInfo && roundOptimizations > 0) {
            std::cout << "Constant folding round " << optimizationRound 
                     << ": " << roundOptimizations << " optimizations applied" << std::endl;
        }
    }
    
    // 输出总优化统计
    if (config.generateDebugInfo && totalOptimizations > 0) {
        std::cout << "Total constant folding optimizations: " << totalOptimizations 
                 << " (converged in " << optimizationRound << " rounds)" << std::endl;
    }
}
/**
 * 立即常量折叠 - 在生成IR指令时立即应用简单的常量折叠
 * 这可以避免生成不必要的中间指令
 */
std::shared_ptr<Operand> IRGenerator::foldConstantExpression(OpCode opcode, 
                                                           std::shared_ptr<Operand> left, 
                                                           std::shared_ptr<Operand> right) {
    // 只有当两个操作数都是常量时才能折叠
    if (!left || !right || 
        left->type != OperandType::CONSTANT || 
        right->type != OperandType::CONSTANT) {
        return nullptr;
    }
    
    int result = 0;
    bool canFold = true;
    
    switch (opcode) {
        case OpCode::ADD: result = left->value + right->value; break;
        case OpCode::SUB: result = left->value - right->value; break;
        case OpCode::MUL: result = left->value * right->value; break;
        case OpCode::DIV: 
            if (right->value == 0) {
                canFold = false;
            } else {
                result = left->value / right->value;
            }
            break;
        case OpCode::MOD: 
            if (right->value == 0) {
                canFold = false;
            } else {
                result = left->value % right->value;
            }
            break;
        case OpCode::LT: result = left->value < right->value ? 1 : 0; break;
        case OpCode::GT: result = left->value > right->value ? 1 : 0; break;
        case OpCode::LE: result = left->value <= right->value ? 1 : 0; break;
        case OpCode::GE: result = left->value >= right->value ? 1 : 0; break;
        case OpCode::EQ: result = left->value == right->value ? 1 : 0; break;
        case OpCode::NE: result = left->value != right->value ? 1 : 0; break;
        case OpCode::AND: result = (left->value && right->value) ? 1 : 0; break;
        case OpCode::OR: result = (left->value || right->value) ? 1 : 0; break;
        default: canFold = false; break;
    }
    
    if (canFold) {
        return std::make_shared<Operand>(result);
    }
    
    return nullptr;
}
 //--------------------增强常量折叠-----------------------   

/**
 * 常量传播优化。
 * 
 * 跟踪被赋予常量值的变量，并用常量替换这些变量的使用。
 * 这使得进一步的优化机会，如常量折叠成为可能。
 */
void IRGenerator::constantPropagation() {
//-----------------增强常量传播--------------------------
    /*// 从变量名到其常量值的映射（如果已知）
    std::unordered_map<std::string, std::shared_ptr<Operand>> constants;
    bool changed = true;
    
    // 重复直到没有更多变化
    while (changed) {
        changed = false;
        
        // 遍历所有指令，查找常量赋值
        for (size_t i = 0; i < instructions.size(); ++i) {
            auto instr = instructions[i];
            
            // 如果是从常量赋值，记录它
            if (auto assignInstr = std::dynamic_pointer_cast<AssignInstr>(instr)) {
                if (assignInstr->source->type == OperandType::CONSTANT) {
                    if (assignInstr->target->type == OperandType::VARIABLE || 
                        assignInstr->target->type == OperandType::TEMP) {
                        constants[assignInstr->target->name] = assignInstr->source;
                    }
                } else if (assignInstr->source->type == OperandType::VARIABLE || 
                          assignInstr->source->type == OperandType::TEMP) {
                    // 如果从另一个已知具有常量值的变量赋值，传播它
                    auto it = constants.find(assignInstr->source->name);
                    if (it != constants.end()) {
                        // 替换为直接从常量赋值
                        assignInstr->source = it->second;
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
                    auto it = constants.find(binOp->left->name);
                    if (it != constants.end()) {
                        binOp->left = it->second;
                        changed = true;
                    }
                }
                
                // 尝试替换右操作数
                if (binOp->right->type == OperandType::VARIABLE || 
                    binOp->right->type == OperandType::TEMP) {
                    auto it = constants.find(binOp->right->name);
                    if (it != constants.end()) {
                        binOp->right = it->second;
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
                    auto it = constants.find(unaryOp->operand->name);
                    if (it != constants.end()) {
                        unaryOp->operand = it->second;
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
                    auto it = constants.find(paramInstr->param->name);
                    if (it != constants.end()) {
                        paramInstr->param = it->second;
                        changed = true;
                    }
                }
            }
            else if (auto returnInstr = std::dynamic_pointer_cast<ReturnInstr>(instr)) {
                if (returnInstr->value && 
                   (returnInstr->value->type == OperandType::VARIABLE || 
                    returnInstr->value->type == OperandType::TEMP)) {
                    auto it = constants.find(returnInstr->value->name);
                    if (it != constants.end()) {
                        returnInstr->value = it->second;
                        changed = true;
                    }
                }
            }
            else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
                if (ifGotoInstr->condition->type == OperandType::VARIABLE || 
                    ifGotoInstr->condition->type == OperandType::TEMP) {
                    auto it = constants.find(ifGotoInstr->condition->name);
                    if (it != constants.end()) {
                        ifGotoInstr->condition = it->second;
                        changed = true;
                    }
                }
            }
        }
        
        // 如果我们做了更改，再次运行常量折叠
        if (changed) {
            constantFolding();
        }
    }*/
    enhancedConstantPropagation();
//-----------------增强常量传播--------------------------
}


/**
 * 死代码消除优化。
 * 
 * 删除对程序输出没有影响的指令。
 * 例如，对从未使用的变量的赋值。
 */
void IRGenerator::deadCodeElimination() {
//----------------------增强死代码消除------------------------    
    /*// 首先，识别使用的变量
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
    
    instructions = newInstructions;*/
    enhancedDeadCodeElimination();
//----------------------增强死代码消除------------------------    
}

/**
 * 控制流优化。
 * 
 * 优化程序的控制流，包括跳转和分支。
 * 例如，消除冗余跳转，优化分支条件，
 * 简化控制流模式。
 */
void IRGenerator::controlFlowOptimization() {
//----------------------增强控制流优化------------------------ 
    /*bool changed = true;
    
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
    }*/
    enhancedControlFlowOptimization();
//----------------------增强控制流优化---------------------------
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

//---------------------增强常量折叠-----------------------
/**
 * 访问二元表达式。
 * 
 * 评估两个操作数，执行二元操作，并将结果推入操作数栈。
 * 
 * @param expr 二元表达式
 */
/*void IRGenerator::visit(BinaryExpr& expr) {
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
}*/
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
    
    // 尝试立即常量折叠
    auto foldedResult = foldConstantExpression(opcode, left, right);
    if (foldedResult) {
        // 可以折叠 - 直接使用折叠后的常量
        operandStack.push_back(foldedResult);
    } else {
        // 不能折叠 - 生成普通的二元运算指令
        std::shared_ptr<Operand> result = createTemp();
        addInstruction(std::make_shared<BinaryOpInstr>(opcode, result, left, right));
        operandStack.push_back(result);
    }
}
//-----------------------增强常量折叠-----------------------


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

//----------------------增强常量传播-----------------------------
/**
 * 增强版常量传播优化
 * 
 * 基于到达定义分析的全局常量传播，支持跨基本块的常量传播
 * 改进包括：
 * 1. 完整的到达定义分析
 * 2. 工作列表算法进行迭代分析
 * 3. 更精确的传播条件判断
 * 4. 与常量折叠的紧密协同
 */
void IRGenerator::enhancedConstantPropagation() {
    if (config.generateDebugInfo) {
        std::cout << "Enhanced constant propagation started..." << std::endl;
    }
    
    bool globalChanged = true;
    int propagationRound = 0;
    int totalPropagations = 0;
    
    // 迭代进行常量传播直到收敛
    while (globalChanged && propagationRound < 20) {
        globalChanged = false;
        propagationRound++;
        int roundPropagations = 0;
        
        // 计算到达定义
        auto reachingDefs = computeReachingDefinitions();
        
        // 为每个指令进行常量传播分析
        for (size_t i = 0; i < instructions.size(); ++i) {
            auto instr = instructions[i];
            bool instrChanged = false;
            
            // 处理二元运算指令
            if (auto binOp = std::dynamic_pointer_cast<BinaryOpInstr>(instr)) {
                // 检查左操作数是否可以传播
                if (binOp->left->type == OperandType::VARIABLE || 
                    binOp->left->type == OperandType::TEMP) {
                    
                    std::shared_ptr<Operand> constantValue = nullptr;
                    if (canPropagateConstant(reachingDefs[i], binOp->left->name, constantValue)) {
                        binOp->left = constantValue;
                        instrChanged = true;
                        roundPropagations++;
                    }
                }
                
                // 检查右操作数是否可以传播
                if (binOp->right->type == OperandType::VARIABLE || 
                    binOp->right->type == OperandType::TEMP) {
                    
                    std::shared_ptr<Operand> constantValue = nullptr;
                    if (canPropagateConstant(reachingDefs[i], binOp->right->name, constantValue)) {
                        binOp->right = constantValue;
                        instrChanged = true;
                        roundPropagations++;
                    }
                }
            }
            // 处理一元运算指令
            else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpInstr>(instr)) {
                if (unaryOp->operand->type == OperandType::VARIABLE || 
                    unaryOp->operand->type == OperandType::TEMP) {
                    
                    std::shared_ptr<Operand> constantValue = nullptr;
                    if (canPropagateConstant(reachingDefs[i], unaryOp->operand->name, constantValue)) {
                        unaryOp->operand = constantValue;
                        instrChanged = true;
                        roundPropagations++;
                    }
                }
            }
            // 处理赋值指令
            else if (auto assignInstr = std::dynamic_pointer_cast<AssignInstr>(instr)) {
                if (assignInstr->source->type == OperandType::VARIABLE || 
                    assignInstr->source->type == OperandType::TEMP) {
                    
                    std::shared_ptr<Operand> constantValue = nullptr;
                    if (canPropagateConstant(reachingDefs[i], assignInstr->source->name, constantValue)) {
                        assignInstr->source = constantValue;
                        instrChanged = true;
                        roundPropagations++;
                    }
                }
            }
            // 处理条件跳转指令
            else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
                if (ifGotoInstr->condition->type == OperandType::VARIABLE || 
                    ifGotoInstr->condition->type == OperandType::TEMP) {
                    
                    std::shared_ptr<Operand> constantValue = nullptr;
                    if (canPropagateConstant(reachingDefs[i], ifGotoInstr->condition->name, constantValue)) {
                        ifGotoInstr->condition = constantValue;
                        instrChanged = true;
                        roundPropagations++;
                    }
                }
            }
            // 处理参数指令
            else if (auto paramInstr = std::dynamic_pointer_cast<ParamInstr>(instr)) {
                if (paramInstr->param->type == OperandType::VARIABLE || 
                    paramInstr->param->type == OperandType::TEMP) {
                    
                    std::shared_ptr<Operand> constantValue = nullptr;
                    if (canPropagateConstant(reachingDefs[i], paramInstr->param->name, constantValue)) {
                        paramInstr->param = constantValue;
                        instrChanged = true;
                        roundPropagations++;
                    }
                }
            }
            // 处理返回指令
            else if (auto returnInstr = std::dynamic_pointer_cast<ReturnInstr>(instr)) {
                if (returnInstr->value && 
                   (returnInstr->value->type == OperandType::VARIABLE || 
                    returnInstr->value->type == OperandType::TEMP)) {
                    
                    std::shared_ptr<Operand> constantValue = nullptr;
                    if (canPropagateConstant(reachingDefs[i], returnInstr->value->name, constantValue)) {
                        returnInstr->value = constantValue;
                        instrChanged = true;
                        roundPropagations++;
                    }
                }
            }
            
            if (instrChanged) {
                globalChanged = true;
            }
        }
        
        totalPropagations += roundPropagations;
        
        // 在每轮传播后立即应用常量折叠
        if (roundPropagations > 0) {
            constantFolding();
        }
        
        // 输出调试信息
        if (config.generateDebugInfo && roundPropagations > 0) {
            std::cout << "Constant propagation round " << propagationRound 
                     << ": " << roundPropagations << " propagations applied" << std::endl;
        }
    }
    
    // 输出总优化统计
    if (config.generateDebugInfo && totalPropagations > 0) {
        std::cout << "Total constant propagations: " << totalPropagations 
                 << " (converged in " << propagationRound << " rounds)" << std::endl;
    }
}

/**
 * 计算到达定义分析
 * 
 * 使用工作列表算法计算每个程序点的到达定义集合
 * 
 * @return 从指令索引到到达定义集合的映射
 */
std::map<int, std::set<IRGenerator::ReachingDefinition>> IRGenerator::computeReachingDefinitions() {
    std::map<int, std::set<ReachingDefinition>> reachingDefs;
    std::map<int, std::set<ReachingDefinition>> genSet;  // 每条指令生成的定义
    std::map<int, std::set<std::string>> killSet; // 每条指令杀死的变量名
    
    // 初始化gen和kill集合
    for (size_t i = 0; i < instructions.size(); ++i) {
        auto instr = instructions[i];
        
        // 计算gen集合（该指令生成的定义）
        auto definedVars = IRAnalyzer::getDefinedVariables(instr);
        for (const auto& varName : definedVars) {
            ReachingDefinition def;
            def.instructionIndex = i;
            def.varName = varName;
            
            // 检查是否是常量定义
            if (isConstantDefinition(instr, varName, def.value)) {
                genSet[i].insert(def);
            } else {
                def.value = nullptr;
                genSet[i].insert(def);
            }
            
            // 该指令杀死该变量的所有之前定义
            killSet[i].insert(varName);
        }
        
        // 初始化到达定义为空集
        reachingDefs[i] = std::set<ReachingDefinition>();
    }
    
    // 工作列表算法
    std::vector<bool> changed(instructions.size(), true);
    bool globalChanged = true;
    
    while (globalChanged) {
        globalChanged = false;
        
        for (size_t i = 0; i < instructions.size(); ++i) {
            if (!changed[i]) continue;
            
            std::set<ReachingDefinition> newReachingDefs;
            
            // 合并所有前驱的输出
            if (i == 0) {
                // 第一条指令，没有前驱
                newReachingDefs = genSet[i];
            } else {
                // 从前驱收集到达定义
                newReachingDefs = reachingDefs[i-1];
                
                // 移除被当前指令杀死的定义
                for (auto it = newReachingDefs.begin(); it != newReachingDefs.end(); ) {
                    if (killSet[i].count(it->varName)) {
                        it = newReachingDefs.erase(it);
                    } else {
                        ++it;
                    }
                }
                
                // 添加当前指令生成的定义
                newReachingDefs.insert(genSet[i].begin(), genSet[i].end());
            }
            
            // 检查是否有变化
            if (newReachingDefs != reachingDefs[i]) {
                reachingDefs[i] = newReachingDefs;
                changed[i] = false;
                globalChanged = true;
                
                // 标记后继需要重新计算
                if (i + 1 < instructions.size()) {
                    changed[i + 1] = true;
                }
            } else {
                changed[i] = false;
            }
        }
    }
    
    return reachingDefs;
}

/**
 * 检查指令是否为常量定义
 * 
 * @param instr 要检查的指令
 * @param varName 被定义的变量名
 * @param constantValue 输出参数，存储常量值（如果是常量定义）
 * @return 如果是常量定义则返回true
 */
bool IRGenerator::isConstantDefinition(const std::shared_ptr<IRInstr>& instr, 
                                     const std::string& varName, 
                                     std::shared_ptr<Operand>& constantValue) {
    // 检查赋值指令
    if (auto assignInstr = std::dynamic_pointer_cast<AssignInstr>(instr)) {
        if (assignInstr->target->name == varName && 
            assignInstr->source->type == OperandType::CONSTANT) {
            constantValue = assignInstr->source;
            return true;
        }
    }
    
    // 其他类型的指令不是常量定义
    constantValue = nullptr;
    return false;
}

/**
 * 检查变量是否可以进行常量传播
 * 
 * @param reachingDefs 到达该点的定义集合
 * @param varName 要检查的变量名
 * @param constantValue 输出参数，存储常量值
 * @return 如果可以传播则返回true
 */
bool IRGenerator::canPropagateConstant(const std::set<ReachingDefinition>& reachingDefs,
                                      const std::string& varName,
                                      std::shared_ptr<Operand>& constantValue) {
    std::vector<ReachingDefinition> varDefs;
    
    // 收集该变量的所有到达定义
    for (const auto& def : reachingDefs) {
        if (def.varName == varName) {
            varDefs.push_back(def);
        }
    }
    
    // 如果没有定义，不能传播
    if (varDefs.empty()) {
        return false;
    }
    
    // 检查所有定义是否都是相同的常量
    std::shared_ptr<Operand> firstConstant = nullptr;
    for (const auto& def : varDefs) {
        if (!def.value || def.value->type != OperandType::CONSTANT) {
            // 存在非常量定义，不能传播
            return false;
        }
        
        if (!firstConstant) {
            firstConstant = def.value;
        } else if (firstConstant->value != def.value->value) {
            // 存在不同的常量值，不能传播
            return false;
        }
    }
    
    // 所有定义都是相同的常量
    constantValue = firstConstant;
    return true;
}
//-----------------------增强常量传播---------------------------
//-----------------------增强死代码消除-------------------------
/**
 * 增强版死代码消除优化
 * 
 * 基于活跃变量分析的精确死代码消除
 * 改进包括：
 * 1. 完整的活跃变量分析
 * 2. 迭代消除直到收敛
 * 3. 精确的副作用判断
 * 4. 无用函数消除
 */
void IRGenerator::enhancedDeadCodeElimination() {
    if (config.generateDebugInfo) {
        std::cout << "Enhanced dead code elimination started..." << std::endl;
    }
    
    bool globalChanged = true;
    int eliminationRound = 0;
    int totalEliminations = 0;
    
    // 迭代进行死代码消除直到收敛
    while (globalChanged && eliminationRound < 10) {
        globalChanged = false;
        eliminationRound++;
        int roundEliminations = 0;
        
        // 计算活跃变量分析
        auto livenessInfo = computeLivenessAnalysis();
        
        // 标记死代码
        std::vector<bool> isDead(instructions.size(), false);
        
        for (size_t i = 0; i < instructions.size(); ++i) {
            auto instr = instructions[i];
            
            // 检查指令是否有重要的副作用
            if (hasEssentialSideEffects(instr)) {
                continue; // 不能删除有副作用的指令
            }
            
            // 检查指令定义的变量是否在指令执行后不再活跃
            auto definedVars = IRAnalyzer::getDefinedVariables(instr);
            bool allDeadDefs = true;
            
            for (const auto& varName : definedVars) {
                if (livenessInfo[i].liveOut.count(varName)) {
                    allDeadDefs = false;
                    break;
                }
            }
            
            // 如果所有定义的变量都不再活跃，标记为死代码
            if (allDeadDefs && !definedVars.empty()) {
                isDead[i] = true;
                roundEliminations++;
                globalChanged = true;
            }
        }
        
        // 删除死代码
        std::vector<std::shared_ptr<IRInstr>> newInstructions;
        for (size_t i = 0; i < instructions.size(); ++i) {
            if (!isDead[i]) {
                newInstructions.push_back(instructions[i]);
            }
        }
        
        instructions = newInstructions;
        totalEliminations += roundEliminations;
        
        // 输出调试信息
        if (config.generateDebugInfo && roundEliminations > 0) {
            std::cout << "Dead code elimination round " << eliminationRound 
                     << ": " << roundEliminations << " instructions eliminated" << std::endl;
        }
    }
    
    // 消除未使用的函数
    eliminateUnusedFunctions();
    
    // 输出总优化统计
    if (config.generateDebugInfo && totalEliminations > 0) {
        std::cout << "Total dead code eliminations: " << totalEliminations 
                 << " (converged in " << eliminationRound << " rounds)" << std::endl;
    }
}

/**
 * 计算活跃变量分析
 * 
 * 使用逆向数据流分析计算每个指令的活跃变量信息
 * 
 * @return 每个指令的活跃变量信息
 */
std::vector<IRGenerator::LivenessInfo> IRGenerator::computeLivenessAnalysis() {
    std::vector<LivenessInfo> livenessInfo(instructions.size());
    
    // 初始化use和def集合
    for (size_t i = 0; i < instructions.size(); ++i) {
        auto instr = instructions[i];
        
        // 计算use集合（指令使用的变量）
        auto usedVars = IRAnalyzer::getUsedVariables(instr);
        for (const auto& varName : usedVars) {
            livenessInfo[i].use.insert(varName);
        }
        
        // 计算def集合（指令定义的变量）
        auto definedVars = IRAnalyzer::getDefinedVariables(instr);
        for (const auto& varName : definedVars) {
            livenessInfo[i].def.insert(varName);
        }
    }
    
    // 工作列表算法进行逆向数据流分析
    std::vector<bool> changed(instructions.size(), true);
    bool globalChanged = true;
    int iterations = 0;
    
    while (globalChanged && iterations < 100) { // 防止无限循环
        globalChanged = false;
        iterations++;
        
        // 逆向遍历（从后往前）
        for (int i = instructions.size() - 1; i >= 0; --i) {
            if (!changed[i]) continue;
            
            std::set<std::string> newLiveOut;
            
            // 计算liveOut[i] = ∪ liveIn[s] for all successors s
            auto successors = findSuccessors(i);
            for (int successor : successors) {
                newLiveOut.insert(livenessInfo[successor].liveIn.begin(), 
                                livenessInfo[successor].liveIn.end());
            }
            
            // 计算liveIn[i] = use[i] ∪ (liveOut[i] - def[i])
            std::set<std::string> newLiveIn = livenessInfo[i].use;
            
            // 添加liveOut[i] - def[i]
            for (const auto& var : newLiveOut) {
                if (livenessInfo[i].def.find(var) == livenessInfo[i].def.end()) {
                    newLiveIn.insert(var);
                }
            }
            
            // 检查是否有变化
            if (newLiveIn != livenessInfo[i].liveIn || newLiveOut != livenessInfo[i].liveOut) {
                livenessInfo[i].liveIn = newLiveIn;
                livenessInfo[i].liveOut = newLiveOut;
                changed[i] = false;
                globalChanged = true;
                
                // 标记前驱需要重新计算
                auto predecessors = findPredecessors(i);
                for (int pred : predecessors) {
                    changed[pred] = true;
                }
            } else {
                changed[i] = false;
            }
        }
    }
    
    if (config.generateDebugInfo) {
        std::cout << "Liveness analysis converged in " << iterations << " iterations" << std::endl;
    }
    
    return livenessInfo;
}

/**
 * 检查指令是否有重要的副作用
 * 
 * 有副作用的指令不能被删除，即使它们的结果没有被使用
 * 
 * @param instr 要检查的指令
 * @return 如果有重要副作用则返回true
 */
bool IRGenerator::hasEssentialSideEffects(const std::shared_ptr<IRInstr>& instr) {
    // 函数调用有副作用
    if (std::dynamic_pointer_cast<CallInstr>(instr)) {
        return true;
    }
    
    // 控制流指令有副作用
    if (std::dynamic_pointer_cast<GotoInstr>(instr) ||
        std::dynamic_pointer_cast<IfGotoInstr>(instr) ||
        std::dynamic_pointer_cast<LabelInstr>(instr)) {
        return true;
    }
    
    // 函数相关指令有副作用
    if (std::dynamic_pointer_cast<ReturnInstr>(instr) ||
        std::dynamic_pointer_cast<ParamInstr>(instr) ||
        std::dynamic_pointer_cast<FunctionBeginInstr>(instr) ||
        std::dynamic_pointer_cast<FunctionEndInstr>(instr)) {
        return true;
    }
    
    // 其他指令（算术运算、赋值等）通常没有副作用
    return false;
}

/**
 * 消除未使用的函数
 * 
 * 删除从未被调用的函数（除了main函数）
 */
void IRGenerator::eliminateUnusedFunctions() {
    std::set<std::string> usedFunctions;
    std::set<std::string> definedFunctions;
    
    // 收集所有函数调用和函数定义
    for (const auto& instr : instructions) {
        if (auto callInstr = std::dynamic_pointer_cast<CallInstr>(instr)) {
            usedFunctions.insert(callInstr->funcName);
        }
        if (auto funcBegin = std::dynamic_pointer_cast<FunctionBeginInstr>(instr)) {
            definedFunctions.insert(funcBegin->funcName);
        }
    }
    
    // main函数总是被使用
    usedFunctions.insert("main");
    
    // 找出未使用的函数
    std::set<std::string> unusedFunctions;
    for (const auto& funcName : definedFunctions) {
        if (usedFunctions.find(funcName) == usedFunctions.end()) {
            unusedFunctions.insert(funcName);
        }
    }
    
    if (unusedFunctions.empty()) {
        return;
    }
    
    // 删除未使用的函数
    std::vector<std::shared_ptr<IRInstr>> newInstructions;
    bool inUnusedFunction = false;
    std::string currentFunction;
    
    for (const auto& instr : instructions) {
        if (auto funcBegin = std::dynamic_pointer_cast<FunctionBeginInstr>(instr)) {
            currentFunction = funcBegin->funcName;
            inUnusedFunction = unusedFunctions.count(currentFunction) > 0;
        }
        
        if (!inUnusedFunction) {
            newInstructions.push_back(instr);
        }
        
        if (auto funcEnd = std::dynamic_pointer_cast<FunctionEndInstr>(instr)) {
            inUnusedFunction = false;
        }
    }
    
    instructions = newInstructions;
    
    if (config.generateDebugInfo && !unusedFunctions.empty()) {
        std::cout << "Eliminated " << unusedFunctions.size() << " unused functions: ";
        for (const auto& funcName : unusedFunctions) {
            std::cout << funcName << " ";
        }
        std::cout << std::endl;
    }
}

/**
 * 查找指令的后继
 * 
 * @param instructionIndex 指令索引
 * @return 后继指令的索引列表
 */
std::vector<int> IRGenerator::findSuccessors(int instructionIndex) {
    std::vector<int> successors;
    
    if (instructionIndex + 1 < instructions.size()) {
        auto instr = instructions[instructionIndex];
        
        // 无条件跳转只有跳转目标作为后继
        if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(instr)) {
            // 查找目标标签
            for (size_t i = 0; i < instructions.size(); ++i) {
                if (auto labelInstr = std::dynamic_pointer_cast<LabelInstr>(instructions[i])) {
                    if (labelInstr->label == gotoInstr->target->name) {
                        successors.push_back(i);
                        break;
                    }
                }
            }
        }
        // 条件跳转有两个后继：跳转目标和下一条指令
        else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
            // 跳转目标
            for (size_t i = 0; i < instructions.size(); ++i) {
                if (auto labelInstr = std::dynamic_pointer_cast<LabelInstr>(instructions[i])) {
                    if (labelInstr->label == ifGotoInstr->target->name) {
                        successors.push_back(i);
                        break;
                    }
                }
            }
            // 下一条指令
            successors.push_back(instructionIndex + 1);
        }
        // 返回指令没有后继
        else if (std::dynamic_pointer_cast<ReturnInstr>(instr)) {
            // 没有后继
        }
        // 其他指令的后继是下一条指令
        else {
            successors.push_back(instructionIndex + 1);
        }
    }
    
    return successors;
}

/**
 * 查找指令的前驱
 * 
 * @param instructionIndex 指令索引
 * @return 前驱指令的索引列表
 */
std::vector<int> IRGenerator::findPredecessors(int instructionIndex) {
    std::vector<int> predecessors;
    
    // 简化实现：假设前驱就是前一条指令（除非是跳转目标）
    if (instructionIndex > 0) {
        predecessors.push_back(instructionIndex - 1);
    }
    
    // 检查是否有跳转到这条指令的
    if (auto labelInstr = std::dynamic_pointer_cast<LabelInstr>(instructions[instructionIndex])) {
        for (size_t i = 0; i < instructions.size(); ++i) {
            if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(instructions[i])) {
                if (gotoInstr->target->name == labelInstr->label) {
                    predecessors.push_back(i);
                }
            }
            else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instructions[i])) {
                if (ifGotoInstr->target->name == labelInstr->label) {
                    predecessors.push_back(i);
                }
            }
        }
    }
    
    return predecessors;
}
//----------------------增强死代码消除------------------------
//----------------------公共子表达式消除----------------------
/**
 * 表达式哈希计算
 * 
 * 为表达式计算哈希值以便快速比较和查找
 */
size_t IRGenerator::Expression::getHash() const {
    size_t hash = std::hash<int>{}(static_cast<int>(opcode));
    
    if (left && left->type == OperandType::VARIABLE) {
        hash ^= std::hash<std::string>{}(left->name) << 1;
    } else if (left && left->type == OperandType::CONSTANT) {
        hash ^= std::hash<int>{}(left->value) << 1;
    }
    
    if (right && right->type == OperandType::VARIABLE) {
        hash ^= std::hash<std::string>{}(right->name) << 2;
    } else if (right && right->type == OperandType::CONSTANT) {
        hash ^= std::hash<int>{}(right->value) << 2;
    }
    
    return hash;
}

/**
 * 表达式相等比较
 */
bool IRGenerator::Expression::operator==(const Expression& other) const {
    if (opcode != other.opcode) {
        return false;
    }
    
    // 比较左操作数
    if ((left == nullptr) != (other.left == nullptr)) {
        return false;
    }
    if (left && other.left) {
        if (left->type != other.left->type) {
            return false;
        }
        if (left->type == OperandType::VARIABLE || left->type == OperandType::TEMP) {
            if (left->name != other.left->name) {
                return false;
            }
        } else if (left->type == OperandType::CONSTANT) {
            if (left->value != other.left->value) {
                return false;
            }
        }
    }
    
    // 比较右操作数
    if ((right == nullptr) != (other.right == nullptr)) {
        return false;
    }
    if (right && other.right) {
        if (right->type != other.right->type) {
            return false;
        }
        if (right->type == OperandType::VARIABLE || right->type == OperandType::TEMP) {
            if (right->name != other.right->name) {
                return false;
            }
        } else if (right->type == OperandType::CONSTANT) {
            if (right->value != other.right->value) {
                return false;
            }
        }
    }
    
    return true;
}

/**
 * 表达式小于比较（用于在set中排序）
 */
bool IRGenerator::Expression::operator<(const Expression& other) const {
    if (opcode != other.opcode) {
        return opcode < other.opcode;
    }
    
    // 比较左操作数
    if (!left && other.left) return true;
    if (left && !other.left) return false;
    if (left && other.left) {
        if (left->type != other.left->type) {
            return left->type < other.left->type;
        }
        if (left->type == OperandType::VARIABLE || left->type == OperandType::TEMP) {
            if (left->name != other.left->name) {
                return left->name < other.left->name;
            }
        } else if (left->type == OperandType::CONSTANT) {
            if (left->value != other.left->value) {
                return left->value < other.left->value;
            }
        }
    }
    
    // 比较右操作数
    if (!right && other.right) return true;
    if (right && !other.right) return false;
    if (right && other.right) {
        if (right->type != other.right->type) {
            return right->type < other.right->type;
        }
        if (right->type == OperandType::VARIABLE || right->type == OperandType::TEMP) {
            if (right->name != other.right->name) {
                return right->name < other.right->name;
            }
        } else if (right->type == OperandType::CONSTANT) {
            if (right->value != other.right->value) {
                return right->value < other.right->value;
            }
        }
    }
    
    return false;
}

/**
 * 增强版公共子表达式消除优化
 * 
 * 基于可用表达式分析的全局公共子表达式消除
 * 改进包括：
 * 1. 完整的可用表达式分析
 * 2. 局部和全局CSE
 * 3. 智能临时变量管理
 * 4. 代价效益分析
 */
void IRGenerator::enhancedCommonSubexpressionElimination() {
    if (config.generateDebugInfo) {
        std::cout << "Enhanced common subexpression elimination started..." << std::endl;
    }
    
    int totalEliminations = 0;
    
    // 计算可用表达式分析
    auto availInfo = computeAvailableExpressionAnalysis();
    
    // 遍历所有指令，查找可以消除的公共子表达式
    for (size_t i = 0; i < instructions.size(); ++i) {
        auto instr = instructions[i];
        
        // 提取当前指令的表达式
        Expression* exprPtr = extractExpression(instr);
        if (!exprPtr) {
            continue;
        }
        
        Expression expr = *exprPtr;
        
        // 检查这个表达式是否在当前点可用（即之前已经计算过）
        if (canEliminateExpression(expr, i, availInfo)) {
            // 查找或创建存储该表达式值的临时变量
            auto tempVar = findOrCreateTempForExpression(expr);
            
            // 将当前指令替换为赋值指令
            if (auto binOp = std::dynamic_pointer_cast<BinaryOpInstr>(instr)) {
                instructions[i] = std::make_shared<AssignInstr>(binOp->result, tempVar);
                totalEliminations++;
            } else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpInstr>(instr)) {
                instructions[i] = std::make_shared<AssignInstr>(unaryOp->result, tempVar);
                totalEliminations++;
            }
        } else {
            // 如果这是表达式第一次出现，记录它
            if (auto binOp = std::dynamic_pointer_cast<BinaryOpInstr>(instr)) {
                expressionToTemp[expr] = binOp->result;
            } else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpInstr>(instr)) {
                expressionToTemp[expr] = unaryOp->result;
            }
        }
    }
    
    // 输出优化统计
    if (config.generateDebugInfo && totalEliminations > 0) {
        std::cout << "Total common subexpression eliminations: " << totalEliminations << std::endl;
    }
}

/**
 * 计算可用表达式分析
 * 
 * 使用数据流分析计算每个指令的可用表达式信息
 */
std::vector<IRGenerator::AvailableExprInfo> IRGenerator::computeAvailableExpressionAnalysis() {
    std::vector<AvailableExprInfo> availInfo(instructions.size());
    
    // 收集程序中所有的表达式 
    std::set<Expression> allExpressions;
    
    // 初始化每个指令的gen和kill集合
    for (size_t i = 0; i < instructions.size(); ++i) {
        auto instr = instructions[i];
        
        // 计算gen集合（该指令生成的表达式）
        Expression* exprPtr = extractExpression(instr);
        if (!exprPtr) {
            Expression expr = *exprPtr;
            availInfo[i].gen.insert(expr);
            allExpressions.insert(expr);
        }
        
        // 计算kill集合（该指令杀死的表达式）
        auto killedExprs = getKilledExpressions(instr);
        availInfo[i].kill = killedExprs;
        allExpressions.insert(killedExprs.begin(), killedExprs.end());
    }
    
    // 初始化可用表达式集合 - 开始时所有表达式都可用（保守估计）
    for (size_t i = 0; i < instructions.size(); ++i) {
        if (i == 0) {
            // 程序开始处没有可用表达式
            availInfo[i].availIn = std::set<Expression>();
        } else {
            // 其他位置初始化为所有表达式都可用
            availInfo[i].availIn = allExpressions;
        }
        availInfo[i].availOut = allExpressions;
    }
    
    // 工作列表算法进行前向数据流分析
    bool changed = true;
    int iterations = 0;
    
    while (changed && iterations < 100) {
        changed = false;
        iterations++;
        
        for (size_t i = 0; i < instructions.size(); ++i) {
            // 计算availOut[i] = (availIn[i] - kill[i]) ∪ gen[i]
            std::set<Expression> newAvailOut;
            
            // 添加availIn[i] - kill[i]  
            for (const auto& expr : availInfo[i].availIn) {
                if (availInfo[i].kill.find(expr) == availInfo[i].kill.end()) {
                    newAvailOut.insert(expr);
                }
            }
            
            // 添加gen[i]
            newAvailOut.insert(availInfo[i].gen.begin(), availInfo[i].gen.end());
            
            // 检查是否有变化
            if (newAvailOut != availInfo[i].availOut) {
                availInfo[i].availOut = newAvailOut;
                changed = true;
            }
            
            // 计算后继的availIn
            if (i + 1 < instructions.size()) {
                // 简化处理：假设下一条指令是唯一后继
                // availIn[i+1] = availOut[i] ∩ availIn[i+1]（交集）
                std::set<Expression> newAvailIn;
                std::set_intersection(
                    availInfo[i].availOut.begin(), availInfo[i].availOut.end(),
                    availInfo[i+1].availIn.begin(), availInfo[i+1].availIn.end(),
                    std::inserter(newAvailIn, newAvailIn.begin())
                );
                
                if (newAvailIn != availInfo[i+1].availIn) {
                    availInfo[i+1].availIn = newAvailIn;
                    changed = true;
                }
            }
        }
    }
    
    if (config.generateDebugInfo) {
        std::cout << "Available expression analysis converged in " << iterations << " iterations" << std::endl;
    }
    
    return availInfo;
}

/**
 * 从指令中提取表达式
 * 
 * @param instr 要分析的指令
 * @return 表达式（如果指令包含可消除的表达式）
 */
IRGenerator::Expression* IRGenerator::extractExpression(const std::shared_ptr<IRInstr>& instr) {
    static Expression expr; // 使用静态变量存储结果
    
    if (auto binOp = std::dynamic_pointer_cast<BinaryOpInstr>(instr)) {
        // 只有当操作数不是常量或者不是简单的变量赋值时才考虑CSE
        if (binOp->opcode != OpCode::ASSIGN) {
            expr.opcode = binOp->opcode;
            expr.left = binOp->left;
            expr.right = binOp->right;
            return &expr;
        }
    } else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpInstr>(instr)) {
        expr.opcode = unaryOp->opcode;
        expr.left = nullptr;
        expr.right = unaryOp->operand;
        return &expr;
    }
    
    return nullptr;
}

/**
 * 获取指令杀死的表达式集合
 * 
 * @param instr 要分析的指令
 * @return 被杀死的表达式集合
 */
std::set<IRGenerator::Expression> IRGenerator::getKilledExpressions(const std::shared_ptr<IRInstr>& instr) {
    std::set<Expression> killedExprs;
    
    // 获取指令定义的变量
    auto definedVars = IRAnalyzer::getDefinedVariables(instr);
    
    // 对于程序中的每个表达式，如果它使用了被重新定义的变量，则被杀死
    for (const auto& [expr, temp] : expressionToTemp) {
        bool isKilled = false;
        
        // 检查表达式的左操作数
        if (expr.left && (expr.left->type == OperandType::VARIABLE || expr.left->type == OperandType::TEMP)) {
            for (const auto& defVar : definedVars) {
                if (expr.left->name == defVar) {
                    isKilled = true;
                    break;
                }
            }
        }
        
        // 检查表达式的右操作数  
        if (!isKilled && expr.right && (expr.right->type == OperandType::VARIABLE || expr.right->type == OperandType::TEMP)) {
            for (const auto& defVar : definedVars) {
                if (expr.right->name == defVar) {
                    isKilled = true;
                    break;
                }
            }
        }
        
        if (isKilled) {
            killedExprs.insert(expr);
        }
    }
    
    return killedExprs;
}

/**
 * 检查表达式是否可以消除
 * 
 * @param expr 要检查的表达式
 * @param instructionIndex 指令索引
 * @param availInfo 可用表达式信息
 * @return 如果可以消除则返回true
 */
bool IRGenerator::canEliminateExpression(const Expression& expr, int instructionIndex, 
                                       const std::vector<AvailableExprInfo>& availInfo) {
    // 检查表达式是否在当前点可用
    if (availInfo[instructionIndex].availIn.find(expr) != availInfo[instructionIndex].availIn.end()) {
        // 还需要检查是否已经有临时变量存储了这个表达式的值
        return expressionToTemp.find(expr) != expressionToTemp.end();
    }
    
    return false;
}

/**
 * 查找或创建存储表达式的临时变量
 * 
 * @param expr 表达式
 * @return 存储表达式值的临时变量
 */
std::shared_ptr<Operand> IRGenerator::findOrCreateTempForExpression(const Expression& expr) {
    auto it = expressionToTemp.find(expr);
    if (it != expressionToTemp.end()) {
        return it->second;
    }
    
    // 如果没有找到，创建新的临时变量
    auto tempVar = createTemp();
    expressionToTemp[expr] = tempVar;
    return tempVar;
}
//----------------------公共子表达式消除----------------------
//----------------------增强控制流优化-----------------------
/**
 * 增强版控制流优化
 * 
 * 全面的控制流结构优化，包括跳转链优化、分支简化、
 * 无用标签消除和不可达代码消除
 */
void IRGenerator::enhancedControlFlowOptimization() {
    if (config.generateDebugInfo) {
        std::cout << "Enhanced control flow optimization started..." << std::endl;
    }
    
    bool globalChanged = true;
    int optimizationRound = 0;
    int totalOptimizations = 0;
    
    // 迭代进行控制流优化直到收敛
    while (globalChanged && optimizationRound < 10) {
        globalChanged = false;
        optimizationRound++;
        int roundOptimizations = 0;
        
        // 分析当前的控制流
        auto cfInfo = analyzeControlFlow();
        
        // 统计优化前的指令数量
        int instructionsBefore = instructions.size();
        
        // 1. 优化跳转链
        optimizeJumpChains(cfInfo);
        
        // 2. 简化分支
        simplifyBranches(cfInfo);
        
        // 3. 消除无用标签 
        eliminateDeadLabels(cfInfo);
        
        // 4. 消除不可达代码
        eliminateUnreachableCode(cfInfo);
        
        // 统计优化后的指令数量
        int instructionsAfter = instructions.size();
        int eliminatedInstructions = instructionsBefore - instructionsAfter;
        
        if (eliminatedInstructions > 0) {
            globalChanged = true;
            roundOptimizations += eliminatedInstructions;
        }
        
        totalOptimizations += roundOptimizations;
        
        // 输出调试信息
        if (config.generateDebugInfo && roundOptimizations > 0) {
            std::cout << "Control flow optimization round " << optimizationRound 
                     << ": " << roundOptimizations << " optimizations applied" << std::endl;
        }
    }
    
    // 输出总优化统计
    if (config.generateDebugInfo && totalOptimizations > 0) {
        std::cout << "Total control flow optimizations: " << totalOptimizations 
                 << " (converged in " << optimizationRound << " rounds)" << std::endl;
    }
}

/**
 * 分析控制流结构
 * 
 * 构建控制流图，识别标签、跳转关系和可达性
 */
IRGenerator::ControlFlowInfo IRGenerator::analyzeControlFlow() {
    ControlFlowInfo cfInfo;
    
    // 1. 构建标签到索引的映射
    for (size_t i = 0; i < instructions.size(); ++i) {
        if (auto labelInstr = std::dynamic_pointer_cast<LabelInstr>(instructions[i])) {
            cfInfo.labelToIndex[labelInstr->label] = i;
        }
    }
    
    // 2. 分析跳转关系和收集被引用的标签
    for (size_t i = 0; i < instructions.size(); ++i) {
        auto instr = instructions[i];
        
        // 处理无条件跳转
        if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(instr)) {
            std::string targetLabel = gotoInstr->target->name;
            cfInfo.referencedLabels.insert(targetLabel);
            
            if (cfInfo.labelToIndex.find(targetLabel) != cfInfo.labelToIndex.end()) {
                int targetIndex = cfInfo.labelToIndex[targetLabel];
                cfInfo.successors[i].insert(targetIndex);
                cfInfo.predecessors[targetIndex].insert(i);
            }
        }
        // 处理条件跳转
        else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
            std::string targetLabel = ifGotoInstr->target->name;
            cfInfo.referencedLabels.insert(targetLabel);
            
            if (cfInfo.labelToIndex.find(targetLabel) != cfInfo.labelToIndex.end()) {
                int targetIndex = cfInfo.labelToIndex[targetLabel];
                cfInfo.successors[i].insert(targetIndex);
                cfInfo.predecessors[targetIndex].insert(i);
            }
            
            // 条件跳转还有一个fallthrough后继
            if (i + 1 < instructions.size()) {
                cfInfo.successors[i].insert(i + 1);
                cfInfo.predecessors[i + 1].insert(i);
            }
        }
        // 处理非跳转指令（除了return）
        else if (!std::dynamic_pointer_cast<ReturnInstr>(instr)) {
            if (i + 1 < instructions.size()) {
                cfInfo.successors[i].insert(i + 1);
                cfInfo.predecessors[i + 1].insert(i);
            }
        }
    }
    
    // 3. 计算可达指令集合
    std::stack<int> workStack;
    cfInfo.reachableInstructions.insert(0); // 第一条指令总是可达的
    workStack.push(0);
    
    while (!workStack.empty()) {
        int current = workStack.top();
        workStack.pop();
        
        for (int successor : cfInfo.successors[current]) {
            if (cfInfo.reachableInstructions.find(successor) == cfInfo.reachableInstructions.end()) {
                cfInfo.reachableInstructions.insert(successor);
                workStack.push(successor);
            }
        }
    }
    
    return cfInfo;
}

/**
 * 优化跳转链
 * 
 * 消除"跳转到跳转"的情况，将多级跳转简化为单级跳转
 */
void IRGenerator::optimizeJumpChains(ControlFlowInfo& cfInfo) {
    bool changed = true;
    int chainOptimizations = 0;
    
    while (changed) {
        changed = false;
        
        for (size_t i = 0; i < instructions.size(); ++i) {
            auto instr = instructions[i];
            
            // 处理无条件跳转
            if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(instr)) {
                std::string currentTarget = gotoInstr->target->name;
                std::string finalTarget = currentTarget;
                
                // 跟踪跳转链，最多跟踪10级以防无限循环
                for (int depth = 0; depth < 10; ++depth) {
                    auto targetIt = cfInfo.labelToIndex.find(finalTarget);
                    if (targetIt == cfInfo.labelToIndex.end()) {
                        break;
                    }
                    
                    int targetIndex = targetIt->second;
                    if (targetIndex + 1 >= instructions.size()) {
                        break;
                    }
                    
                    // 检查目标标签后的下一条指令是否是无条件跳转
                    auto nextInstr = instructions[targetIndex + 1];
                    if (auto nextGoto = std::dynamic_pointer_cast<GotoInstr>(nextInstr)) {
                        finalTarget = nextGoto->target->name;
                    } else {
                        break;
                    }
                }
                
                // 如果找到了更短的跳转路径，更新跳转目标
                if (finalTarget != currentTarget) {
                    gotoInstr->target->name = finalTarget;
                    changed = true;
                    chainOptimizations++;
                }
            }
            // 处理条件跳转
            else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
                std::string currentTarget = ifGotoInstr->target->name;
                std::string finalTarget = currentTarget;
                
                // 跟踪跳转链
                for (int depth = 0; depth < 10; ++depth) {
                    auto targetIt = cfInfo.labelToIndex.find(finalTarget);
                    if (targetIt == cfInfo.labelToIndex.end()) {
                        break;
                    }
                    
                    int targetIndex = targetIt->second;
                    if (targetIndex + 1 >= instructions.size()) {
                        break;
                    }
                    
                    auto nextInstr = instructions[targetIndex + 1];
                    if (auto nextGoto = std::dynamic_pointer_cast<GotoInstr>(nextInstr)) {
                        finalTarget = nextGoto->target->name;
                    } else {
                        break;
                    }
                }
                
                if (finalTarget != currentTarget) {
                    ifGotoInstr->target->name = finalTarget;
                    changed = true;
                    chainOptimizations++;
                }
            }
        }
    }
    
    if (config.generateDebugInfo && chainOptimizations > 0) {
        std::cout << "Jump chain optimizations: " << chainOptimizations << std::endl;
    }
}

/**
 * 简化分支
 * 
 * 检测和优化冗余的条件分支
 */
void IRGenerator::simplifyBranches(ControlFlowInfo& cfInfo) {
    int branchSimplifications = 0;
    
    for (size_t i = 0; i < instructions.size(); ++i) {
        auto instr = instructions[i];
        
        // 寻找条件跳转到下一条指令的情况
        if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
            std::string targetLabel = ifGotoInstr->target->name;
            
            // 检查跳转目标是否就是下一条指令
            if (i + 1 < instructions.size()) {
                if (auto nextLabel = std::dynamic_pointer_cast<LabelInstr>(instructions[i + 1])) {
                    if (nextLabel->label == targetLabel) {
                        // 条件跳转到下一条指令，可以删除这个跳转
                        instructions.erase(instructions.begin() + i);
                        branchSimplifications++;
                        break; // 需要重新开始，因为索引改变了
                    }
                }
            }
        }
    }
    
    if (config.generateDebugInfo && branchSimplifications > 0) {
        std::cout << "Branch simplifications: " << branchSimplifications << std::endl;
    }
}

/**
 * 消除无用标签
 * 
 * 删除不再被任何跳转引用的标签
 */
void IRGenerator::eliminateDeadLabels(const ControlFlowInfo& cfInfo) {
    std::vector<std::shared_ptr<IRInstr>> newInstructions;
    int eliminatedLabels = 0;
    
    for (const auto& instr : instructions) {
        if (auto labelInstr = std::dynamic_pointer_cast<LabelInstr>(instr)) {
            // 检查标签是否被引用
            if (cfInfo.referencedLabels.find(labelInstr->label) != cfInfo.referencedLabels.end()) {
                newInstructions.push_back(instr);
            } else {
                eliminatedLabels++;
            }
        } else {
            newInstructions.push_back(instr);
        }
    }
    
    instructions = newInstructions;
    
    if (config.generateDebugInfo && eliminatedLabels > 0) {
        std::cout << "Dead labels eliminated: " << eliminatedLabels << std::endl;
    }
}

/**
 * 消除不可达代码
 * 
 * 删除永远无法执行到的指令
 */
void IRGenerator::eliminateUnreachableCode(const ControlFlowInfo& cfInfo) {
    std::vector<std::shared_ptr<IRInstr>> newInstructions;
    int eliminatedInstructions = 0;
    
    for (size_t i = 0; i < instructions.size(); ++i) {
        if (cfInfo.reachableInstructions.find(i) != cfInfo.reachableInstructions.end()) {
            newInstructions.push_back(instructions[i]);
        } else {
            eliminatedInstructions++;
        }
    }
    
    instructions = newInstructions;
    
    if (config.generateDebugInfo && eliminatedInstructions > 0) {
        std::cout << "Unreachable instructions eliminated: " << eliminatedInstructions << std::endl;
    }
}

/**
 * 获取跳转指令的目标标签
 */
std::string IRGenerator::getJumpTarget(int instructionIndex) {
    if (instructionIndex >= instructions.size()) {
        return "";
    }
    
    auto instr = instructions[instructionIndex];
    if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(instr)) {
        return gotoInstr->target->name;
    } else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
        return ifGotoInstr->target->name;
    }
    
    return "";
}

/**
 * 检查指令是否是无条件跳转
 */
bool IRGenerator::isUnconditionalJump(int instructionIndex) {
    if (instructionIndex >= instructions.size()) {
        return false;
    }
    
    return std::dynamic_pointer_cast<GotoInstr>(instructions[instructionIndex]) != nullptr;
}

/**
 * 检查指令是否是条件跳转
 */
bool IRGenerator::isConditionalJump(int instructionIndex) {
    if (instructionIndex >= instructions.size()) {
        return false;
    }
    
    return std::dynamic_pointer_cast<IfGotoInstr>(instructions[instructionIndex]) != nullptr;
}

/**
 * 更新跳转指令的目标标签
 */
void IRGenerator::updateJumpTarget(int instructionIndex, const std::string& newTarget) {
    if (instructionIndex >= instructions.size()) {
        return;
    }
    
    auto instr = instructions[instructionIndex];
    if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(instr)) {
        gotoInstr->target->name = newTarget;
    } else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
        ifGotoInstr->target->name = newTarget;
    }
}
//------------------------增强控制流优化------------------------