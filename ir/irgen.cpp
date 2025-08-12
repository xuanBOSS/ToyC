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
    constantPropagationCFG();    // 在代码中传播常量值
    //deadCodeElimination();    // 删除无效果的代码
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
/*void IRGenerator::constantPropagation() {
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
                    std::unordered_set<std::string> visited;
                    auto resolved = resolveConstant(assignInstr->source->name, constants, visited);
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
                    std::unordered_set<std::string> visited;
                    auto resolved = resolveConstant(binOp->left->name, constants, visited);
                    if (resolved) {
                        binOp->left = resolved;
                        changed = true;
                    }
                }
                
                // 尝试替换右操作数
                if (binOp->right->type == OperandType::VARIABLE || 
                    binOp->right->type == OperandType::TEMP) {
                    //auto it = constants.find(binOp->right->name);
                    std::unordered_set<std::string> visited;
                    auto resolved = resolveConstant(binOp->right->name, constants, visited);
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
                    std::unordered_set<std::string> visited;
                    auto resolved = resolveConstant(unaryOp->operand->name, constants, visited);
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
                    std::unordered_set<std::string> visited;
                    auto resolved = resolveConstant(paramInstr->param->name, constants, visited);
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
                    std::unordered_set<std::string> visited;
                    auto resolved = resolveConstant(returnInstr->value->name, constants, visited);
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
                    std::unordered_set<std::string> visited;
                    auto resolved = resolveConstant(ifGotoInstr->condition->name, constants, visited);
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
    std::unordered_set<std::string>& visited,
    int depth
) {
    // 防止进入无限循环
    if (depth > MAX_PROPAGATION_DEPTH) return nullptr; // 深度检测
    if (visited.count(name)) return nullptr;           // 检测到环，直接返回
    visited.insert(name);

    // 寻找依赖的变量
    auto it = constants.find(name);
    if (it == constants.end()) return nullptr;

    auto operand = it->second;
    if (operand->type == OperandType::CONSTANT) return operand;
    if (operand->type == OperandType::VARIABLE || operand->type == OperandType::TEMP) {
        return resolveConstant(operand->name, constants, visited, depth + 1);
    }
    return nullptr;
}*/

/*
*   常量传播优化
*
*   基于基本块 + 控制流图（CFG）+ 数据流分析 的实现
*/

// --- Lattice / ConstMap ---
enum class LatticeKind { Unknown, Constant, Top };  // 三态：未知 / 常量 / 冲突

struct LatticeValue {
    LatticeKind kind = LatticeKind::Unknown;
    int constantValue = 0;                                  // 仅当 kind == LatticeKind::Constant时有效，表示该值的​​具体常量数值​​。
    bool operator==(const LatticeValue& o) const {                          // ==运算符重载
        if (kind != o.kind) return false;
        if (kind == LatticeKind::Constant) return constantValue == o.constantValue;
        return true;
    }
    bool operator!=(const LatticeValue& o) const { return !(*this == o); }  // != 运算符重载
};

using ConstMap = std::unordered_map<std::string, LatticeValue>;

// ---------- 帮助函数（用于比较两个constMap是否语义等价） ----------
static bool constMapsEqual(const ConstMap& a, const ConstMap& b) {

    // 将缺失的键视为Unknown状态
    if (a.size() == b.size()) {
        for (auto& [k, v] : a) {
            auto it = b.find(k);
            if (it == b.end()) return false;        // 键不一致直接返回false
            if (v != it->second) return false;      // 值不一致直接返回false
        }
        return true;    // 所有键值对完全匹配
    }

    // 大小不同时：仍需检查键的并集
    std::unordered_set<std::string> keys;   // 存储所有键的集合
    for (auto& [k,_] : a) keys.insert(k);   // 收集a的键
    for (auto& [k,_] : b) keys.insert(k);   // 收集b的键
    for (auto& k : keys) {
        auto ita = a.find(k);
        auto itb = b.find(k);
        // 若键不存在则使用默认LatticeValue(即Unknown)
        LatticeValue va = (ita == a.end() ? LatticeValue{} : ita->second);
        LatticeValue vb = (itb == b.end() ? LatticeValue{} : itb->second);
        if (va != vb) return false;     // 存在不匹配的值
    }
    return true;    // 所有键的对应值语义等价
}

// 在循环中对于回边的特殊处理
using BlockID = int;
std::vector<std::pair<BlockID, BlockID>> findBackEdges(
    const std::unordered_map<BlockID, std::vector<BlockID>>& cfg) 
{
    std::vector<std::pair<BlockID, BlockID>> backEdges;     // 存储回边结果
    std::unordered_set<BlockID> visited;                    // 记录已访问的块
    std::vector<BlockID> stack;                             // DFS递归栈，用于检测回边

    // DFS遍历函数（Lambda表达式）
    std::function<void(BlockID)> dfs = [&](BlockID b) {
        visited.insert(b);      // 标记当前块为已访问
        stack.push_back(b);     // 将当前块压入DFS栈

        // 遍历当前块的所有后继
        for (auto succ : cfg.at(b)) {
            // 情况1：后继块在DFS栈中 -> 发现回边
            if (std::find(stack.begin(), stack.end(), succ) != stack.end()) {
                backEdges.emplace_back(b, succ);
            } 
            // 情况2：后继块未访问 -> 递归处理
            else if (!visited.count(succ)) {
                dfs(succ);
            }
        }

        stack.pop_back();   // 回溯：当前块处理完毕，弹出栈
    };

    // 对所有未访问的块启动DFS
    for (auto& kv : cfg) {
        if (!visited.count(kv.first)) {
            dfs(kv.first);
        }
    }
    return backEdges;
}

/**
 * 获取循环体内定义的所有变量集合（循环定义变量分析）。
 * 通过遍历从循环入口到回边的所有基本块，收集其中被赋值的变量名。
 * 
 * @param cfg 控制流图（BlockID -> 后继块列表）
 * @param fromBlk 回边的起始块（循环体出口）
 * @param toBlk 回边的目标块（循环体入口）
 * @param blocks 基本块集合（BlockID -> Block结构体）
 * @return 包含循环体内所有被赋值变量名的集合
 */
/*std::unordered_set<std::string> IRGenerator::getLoopDefs(
    const std::unordered_map<BlockID, std::vector<BlockID>>& cfg,
    BlockID fromBlk, BlockID toBlk,
    const std::unordered_map<BlockID, IRGenerator::BasicBlock>& blocks)
{
    std::unordered_set<BlockID> visited;    // 记录已访问的基本块
    std::unordered_set<std::string> defs;   // 存储发现的变量定义
    std::vector<BlockID> stack = {toBlk};   // 初始化DFS栈（从循环入口开始）

    while (!stack.empty()) {
        BlockID blk = stack.back();
        stack.pop_back();

        // 跳过已处理块
        if (visited.count(blk)) continue;
        visited.insert(blk);

        // 遍历当前块的所有指令，收集赋值语句的目标变量
        for (auto& inst : blocks.at(blk).instructions) {
            if (auto assignInstr = std::dynamic_pointer_cast<AssignInstr>(inst)) {
                defs.insert(assignInstr->target->name); // 记录被赋值的变量
            }
        }

        // 处理后继块（深度优先遍历）
        for (auto succ : cfg.at(blk)) {
            // 关键逻辑：跳过直接回到循环入口的回边（避免无限循环）
            if (succ != fromBlk) { // 避免直接走回边出口
                stack.push_back(succ);
            }
        }
    }
    return defs;
}*/
std::unordered_set<std::string> IRGenerator::getLoopDefs(
    const std::unordered_set<BlockID>& loopBlocks,
    const std::unordered_map<BlockID, IRGenerator::BasicBlock>& blocks)
{
    std::unordered_set<std::string> defs;   // 存储结果：循环内所有被赋值的变量名

    // 遍历循环体内的每一个基本块
    for (auto blkId : loopBlocks) {
        // 查找当前块ID对应的基本块对象
        auto it = blocks.find(blkId);
        if (it == blocks.end()) continue; // 没找到块，跳过

        // 遍历当前块的所有指令
        for (auto& inst : it->second.instructions) {
            // 检查是否为赋值指令
            if (auto assignInstr = std::dynamic_pointer_cast<AssignInstr>(inst)) {
                // 记录被赋值的变量名
                defs.insert(assignInstr->target->name);
            }
        }
    }
    return defs;
}

/**
 * 获取由回边(fromBlk→toBlk)定义的循环体内的所有基本块ID集合
 * 
 * @param cfg 控制流图（BlockID -> 后继列表）
 * @param fromBlk 回边的起始块ID（循环体出口）
 * @param toBlk 回边的目标块ID（循环体入口）
 * @return 包含循环体内所有块ID的无序集合
 */
std::unordered_set<int> IRGenerator::getLoopBlocks(
    const std::unordered_map<int, std::vector<int>>& cfg,
    int fromBlk, int toBlk)
{
    // 构建反向CFG
    std::unordered_map<int, std::vector<int>> predMap;
    for (auto& kv : cfg) {
        int u = kv.first;
        for (int v : kv.second) {
            predMap[v].push_back(u);
        }
    }

    // 反向遍历识别循环体
    // 从 fromBlk 出发沿反向边走
    std::unordered_set<int> loopBlocks;
    std::stack<int> st;

    // 初始化：回边两端一定在循环体内
    st.push(fromBlk);
    loopBlocks.insert(toBlk); // 循环入口一定在循环内
    loopBlocks.insert(fromBlk);

    while (!st.empty()) {
        int cur = st.top();
        st.pop();

        // 遍历当前块的所有前驱
        for (int pred : predMap[cur]) {
            if (loopBlocks.count(pred)) continue;   // 已处理则跳过
            loopBlocks.insert(pred);
            // 关键限制：不越过循环入口（避免包含外部块）
            if (pred != toBlk) {
                st.push(pred);
            }
        }
    }

    return loopBlocks;
}


/**
 * 清除指定基本块在循环中定义的变量的常量状态，将其标记为未知（Unknown）。
 * 用于处理循环体中对变量的重新定义，确保数据流分析的保守性。
 * 
 * @param inMap 当前块的输入常量映射表（将被修改）
 * @param loopDefs 预计算的各循环块中定义的变量集合（BlockID -> 变量名集合）
 * @param block 当前处理的基本块ID
 */
void clearLoopDefs(ConstMap& inMap, 
    const std::unordered_map<BlockID, std::unordered_set<std::string>>& loopDefs,
    BlockID block) 
{
    // 查找当前块是否属于某个循环定义域
    auto it = loopDefs.find(block);
    if (it != loopDefs.end()) {
        // 遍历该循环块中所有被定义的变量
        for (auto& var : it->second) {
            // 强制将变量状态设为未知（覆盖可能的常量传播结果）
            inMap[var] = {LatticeKind::Unknown, 0}; 
        }
    }
}


// meet (合并) 两个 ConstMap：对每个变量，如果两个都是同一常量，则保留，否则 Unknown/Top 规则
static ConstMap meetMaps(const ConstMap& A, const ConstMap& B) {
    ConstMap R;     // 结果映射表

    // 1. 收集所有键的并集
    std::unordered_set<std::string> keys;
    for (auto& p : A) keys.insert(p.first);
    for (auto& p : B) keys.insert(p.first);

    // 2. 对每个键进行合并操作
    for (auto& k : keys) {
        // 获取两个映射表中该键的值（不存在则视为Unknown）
        auto ita = A.find(k);
        auto itb = B.find(k);
        LatticeValue va = (ita == A.end() ? LatticeValue{} : ita->second);
        LatticeValue vb = (itb == B.end() ? LatticeValue{} : itb->second);

        // 根据值的类型进行合并  
        // 情况1：两者都是常量值     
        if (va.kind == LatticeKind::Constant && vb.kind == LatticeKind::Constant) { 
            if (va.constantValue == vb.constantValue) {
                R[k] = va;
            } else {
                R[k] = LatticeValue{LatticeKind::Top, 0};
            }
        } 
        // 情况2：一方是Unknown，直接取另一方的值（Unknown不影响结果）
        else if (va.kind == LatticeKind::Unknown) {
            R[k] = vb;
        } 
        else if (vb.kind == LatticeKind::Unknown) {
            R[k] = va;
        } 
        // 情况3：其他情况（至少一方是Top，或类型组合不明确）
        else {
            if (va.kind == LatticeKind::Top || vb.kind == LatticeKind::Top) {
                R[k] = LatticeValue{LatticeKind::Top, 0};
            } else {
                R[k] = LatticeValue{LatticeKind::Top, 0};
            }
        }
    }
    return R;
}

// 将 Operand 转换为 LatticeValue（使用当前 env）
static LatticeValue valueOfOperand(const std::shared_ptr<Operand>& op, const ConstMap& env) {

    // 1. 处理空指针情况：返回Unknown表示无效操作数
    if (!op) return LatticeValue{LatticeKind::Unknown, 0};

    // 2. 处理常量类型操作数
    if (op->type == OperandType::CONSTANT) {
        return LatticeValue{LatticeKind::Constant, op->value};
    } 
    // 3. 处理变量或临时变量
    else if (op->type == OperandType::VARIABLE || op->type == OperandType::TEMP) {
        auto it = env.find(op->name);
        if (it == env.end()) return LatticeValue{LatticeKind::Unknown, 0};
        return it->second;
    } 
    // 4. 处理其他类型操作数(如函数调用、指针等)
    else {
        // 返回Top表示"可能任何值"(最不精确状态)
        return LatticeValue{LatticeKind::Top, 0};
    }
}

// 尝试计算二元运算的常量（如果两边常量且 op 可计算）
static bool tryEvalBinaryOp(const std::shared_ptr<BinaryOpInstr>& binOp, int lval, int rval, int& out) {
    
    // 匹配对应的二元操作
    switch (binOp->opcode) {
        case OpCode::ADD: out = lval + rval; return true;
        case OpCode::SUB: out = lval - rval; return true;
        case OpCode::MUL: out = lval * rval; return true;
        case OpCode::DIV:
            if (rval == 0) return false;
            out = lval / rval; return true;
        case OpCode::AND: out = lval & rval; return true;
        case OpCode::OR:  out = lval | rval; return true;
        case OpCode::LT:  out = (lval < rval) ? 1 : 0; return true;
        case OpCode::GT:  out = (lval > rval) ? 1 : 0; return true;
        case OpCode::LE:  out = (lval <= rval) ? 1 : 0; return true;
        case OpCode::GE:  out = (lval >= rval) ? 1 : 0; return true;
        case OpCode::EQ:  out = (lval == rval) ? 1 : 0; return true;
        case OpCode::NE:  out = (lval != rval) ? 1 : 0; return true;
        default: return false;
    }
}

// 生成常量操作数
std::shared_ptr<Operand> IRGenerator::makeConstantOperand(int v, std::string name) {
    auto op = std::make_shared<Operand>(OperandType::CONSTANT, name);    
    op->value = v;
    return op;
}

// ---------- 构建基本块 ----------
std::vector<std::shared_ptr<IRGenerator::BasicBlock>> IRGenerator::buildBasicBlocks() {
    std::vector<std::shared_ptr<BasicBlock>> blocks;    // 存储生成的基本块
    const auto& instrs = this->instructions; 

    // 首先扫描得到 label -> index 映射
    std::unordered_map<std::string, int> labelToIndex;
    for (int i = 0; i < (int)instrs.size(); ++i) {
        // 如果是标签指令，记录其位置
        if (auto lbl = std::dynamic_pointer_cast<LabelInstr>(instrs[i])) {
            labelToIndex[lbl->label] = i;
        }
    }

    // 标记 leader（基本块起点）
    std::vector<char> isLeader(instrs.size(), 0);

    // 规则1：第一条指令默认是Leader
    if (!instrs.empty()) isLeader[0] = 1;

    for (int i = 0; i < (int)instrs.size(); ++i) {
        auto ins = instrs[i];

        // 规则2：跳转指令的目标指令是Leader
        if (auto ifg = std::dynamic_pointer_cast<IfGotoInstr>(ins)) {

            // 查找跳转目标标签对应的指令位置
            auto it = labelToIndex.find(ifg->target->name);
            if (it != labelToIndex.end()) isLeader[it->second] = 1;

            // 规则3：跳转指令的下一条指令是Leader（fall-through情况）
            if (i + 1 < (int)instrs.size()) isLeader[i + 1] = 1;
        } 
        // 规则4：无条件跳转指令处理
        else if (auto g = std::dynamic_pointer_cast<GotoInstr>(ins)) {
            auto it = labelToIndex.find(g->target->name);
            if (it != labelToIndex.end()) isLeader[it->second] = 1;
            if (i + 1 < (int)instrs.size()) isLeader[i + 1] = 1; // safe：即使不可达也当 leader
        } 
        // 规则5：返回指令的下一条是Leader
        else if (auto ret = std::dynamic_pointer_cast<ReturnInstr>(ins)) {
            if (i + 1 < (int)instrs.size()) isLeader[i + 1] = 1;
        } 
        // 规则6：标签指令自身是Leader（处理标签在代码中间的情况）
        else if (std::dynamic_pointer_cast<LabelInstr>(ins)) {
            isLeader[i] = 1; // label 自身是 leader（如果 label 在中间）
        }
        // 规则7：函数调用的下一条指令是Leader（保守策略）
        else if (std::dynamic_pointer_cast<CallInstr>(ins)) {
            if (i + 1 < (int)instrs.size()) isLeader[i + 1] = 1;
        }
        // 规则8：函数开始指令是Leader
        else if (std::dynamic_pointer_cast<FunctionBeginInstr>(ins)) {
            isLeader[i] = 1;
        }
    }

    // 收集所有Leader的索引
    std::vector<int> leadersIdx;
    for (int i = 0; i < (int)isLeader.size(); ++i) if (isLeader[i]) leadersIdx.push_back(i);
    
    // 特殊情况处理：如果没有Leader但指令不为空，默认第一条为Leader
    if (leadersIdx.empty() && !instrs.empty()) leadersIdx.push_back(0);

    // 根据Leader划分基本块
    for (int bi = 0; bi < (int)leadersIdx.size(); ++bi) {
        int start = leadersIdx[bi];     // 当前基本块起始指令索引
        // 计算结束指令索引：下一个Leader前一条，或者指令列表末尾
        int end = (bi + 1 < (int)leadersIdx.size()) ? (leadersIdx[bi + 1] - 1) : ((int)instrs.size() - 1);

        // 创建新基本块
        auto block = std::make_shared<BasicBlock>();
        block->id = (int)blocks.size();

        // 将[start..end]范围内的指令加入基本块
        for (int k = start; k <= end; ++k) block->instructions.push_back(instrs[k]);

        // 如果基本块的第一条指令是标签，记录标签名
        /*if (!block->instructions.empty()) {
            if (auto lbl = std::dynamic_pointer_cast<LabelInstr>(block->instructions.front())) {
                block->label = lbl->label;
            }
        }*/

        // 每一个基本块都有各自唯一的标签
        // 如果第一条指令不是标签，生成一个新标签指令
        if (block->instructions.empty() || 
            !std::dynamic_pointer_cast<LabelInstr>(block->instructions.front())) {
            std::string newLabel = "__block" + std::to_string(block->id); // 唯一标签名
            auto lblInstr = std::make_shared<LabelInstr>(newLabel);
            block->instructions.insert(block->instructions.begin(), lblInstr); // 插到最前面
            block->label = newLabel; // 更新块标签
        } else {
            // 第一条是标签
            auto lbl = std::dynamic_pointer_cast<LabelInstr>(block->instructions.front());
            block->label = lbl->label;
        }

        blocks.push_back(block);    // 将基本块加入结果列表
    }

    return blocks;
}

// ---------- 构建 CFG（连接基本块） ----------
void IRGenerator::buildCFG(std::vector<std::shared_ptr<BasicBlock>>& blocks) {
    if (blocks.empty()) return;

    // 建立 label -> block 映射（块以 label 开头）
    std::unordered_map<std::string, std::shared_ptr<BasicBlock>> labelToBlock;
    for (auto& b : blocks) {
        if (!b->label.empty()) labelToBlock[b->label] = b;
    }

    // 建立 funcName -> block 映射
    std::unordered_map<std::string, std::shared_ptr<BasicBlock>> functionLabelToBlock;
    for (auto& block : blocks) {
        if (!block->instructions.empty()) {
            for(auto ins:block->instructions)
            {
                if(auto fbl = std::dynamic_pointer_cast<FunctionBeginInstr>(ins))
                {
                    // 如果是函数入口标签，就加入映射
                    functionLabelToBlock[fbl->funcName] = block;
                    break;
                }
            }
            /*if (auto fbl = std::dynamic_pointer_cast<FunctionBeginInstr>(block->instructions.front())) {
                // 如果是函数入口标签，就加入映射
                functionLabelToBlock[fbl->funcName] = block;
            }*/
        }
    }

    // 被调函数名 -> 该函数所有调用点的返回位置块
    std::unordered_map<std::string,std::vector<std::shared_ptr<BasicBlock>>> callReturnSites;

    // 先把函数入口块的 functionName 设置好
    for (auto& kv : functionLabelToBlock) {
        const std::string& funcName = kv.first;
        auto& entryBlock = kv.second;
        entryBlock->functionName = funcName;
    }

    // 按照顺序遍历 blocks，给块分配 functionName
    std::string currentFuncName;
    for (auto& b : blocks) {
        if (!b->instructions.empty()) {
            /*if (auto fbegin = std::dynamic_pointer_cast<FunctionBeginInstr>(b->instructions.front())) {
                currentFuncName = fbegin->funcName;
            }*/
           if(b->functionName != "") currentFuncName = b->functionName;
        }
        b->functionName = currentFuncName;
    }

    // 用来存储每个函数名对应的所有包含 ReturnInstr 的基本块集合，方便后面把这些函数的 return 块连接回调用点的返回块
    std::unordered_map<std::string, std::vector<std::shared_ptr<BasicBlock>>> functionReturnBlocks;
    for (auto& b : blocks) {
        if (!b->instructions.empty()) {
            auto last = b->instructions.back();
            if (std::dynamic_pointer_cast<ReturnInstr>(last)) {
                functionReturnBlocks[b->functionName].push_back(b);
            }
        }
    }

    // 先清空 predecessors / successors（避免旧数据干扰）
    for (auto& b : blocks) {
        b->successors.clear();
        b->predecessors.clear();
    }

    // 对每个 block，根据尾指令确定 successors（去重）
    for (int i = 0; i < (int)blocks.size(); ++i) {
        auto& b = blocks[i];
        std::unordered_set<std::shared_ptr<BasicBlock>> succSet;

        if (!b->instructions.empty()) {
            auto last = b->instructions.back();

            if (auto ifg = std::dynamic_pointer_cast<IfGotoInstr>(last)) {
                auto it = labelToBlock.find(ifg->target->name);
                if (it != labelToBlock.end()) succSet.insert(it->second);
                if (i + 1 < (int)blocks.size()) succSet.insert(blocks[i + 1]);

            } else if (auto g = std::dynamic_pointer_cast<GotoInstr>(last)) {
                auto it = labelToBlock.find(g->target->name);
                if (it != labelToBlock.end()) succSet.insert(it->second);

            }
            else if (auto call = std::dynamic_pointer_cast<CallInstr>(last)) {
                // 1. 连接到被调函数入口（如果解析到）
                /*auto it = functionLabelToBlock.find(call->funcName);
                if (it != functionLabelToBlock.end()) succSet.insert(it->second);

                // 2. 如果调用会返回，记录返回点（但是不要在此刻把调用块直接连到返回点）
                if (i + 1 < blocks.size()) {
                    // 记录：被调函数名 -> 返回点（调用之后的块）
                    callReturnSites[call->funcName].push_back(blocks[i + 1]);
                }

                // 3. 保证顺序连接存在
                if (i + 1 < blocks.size()) {
                    succSet.insert(blocks[i + 1]); 
                    callReturnSites[call->funcName].push_back(blocks[i + 1]);
                }*/

                // 对于函数内分析，不去连接到被调函数
                // 只顺序连接到下一个块（如果有）
                if (i + 1 < (int)blocks.size()) {
                    succSet.insert(blocks[i + 1]);
                }
            } 
            else if (!std::dynamic_pointer_cast<ReturnInstr>(last)) {
                if (i + 1 < (int)blocks.size()) succSet.insert(blocks[i + 1]);

            } 
        }

        b->successors.assign(succSet.begin(), succSet.end());
    }

    //  在所有函数都处理完后：把被调函数的 return 块统一连回所有调用点的返回点
    /*for (auto &kv : callReturnSites) {
        const std::string &callee = kv.first;
        const auto &returnSites = kv.second; // 所有调用该 callee 的返回点块

        auto it = functionReturnBlocks.find(callee);
        if (it == functionReturnBlocks.end()) continue; // 没有 return（或未解析），跳过

        const auto &calleeReturnBlocks = it->second;
        for (auto &retBlk : calleeReturnBlocks) {
            // 把 retBlk 的后继加入每个 returnSite（去重）
            std::unordered_set<std::shared_ptr<BasicBlock>> succSet(
                retBlk->successors.begin(), retBlk->successors.end());
            for (auto &retSiteBlk : returnSites) {
                succSet.insert(retSiteBlk);
            }
            retBlk->successors.assign(succSet.begin(), succSet.end());
        }
    }*/

    // 填充 predecessors
    for (auto& b : blocks) {
        for (auto& s : b->successors) {
            s->predecessors.push_back(b);
        }
    }
}

// ---------- transfer function：基于当前 env 更新 env（顺序应用 block 内指令） ----------
void applyTransferToEnv(ConstMap& env, const std::shared_ptr<IRInstr>& instr) {

    // AssignInstr
    if (auto assignInstr = std::dynamic_pointer_cast<AssignInstr>(instr)) {
        // 如果 source 是常量，直接写常量
        if (assignInstr->source->type == OperandType::CONSTANT) {
            env[assignInstr->target->name] = LatticeValue{LatticeKind::Constant, assignInstr->source->value};
        } else if (assignInstr->source->type == OperandType::VARIABLE || assignInstr->source->type == OperandType::TEMP) {
            // 如果 source 在 env 中是常量，赋值传播，否则变 Top
            auto it = env.find(assignInstr->source->name);
            if (it != env.end() && it->second.kind == LatticeKind::Constant) {
                env[assignInstr->target->name] = it->second;
            } else if (it != env.end() && it->second.kind == LatticeKind::Unknown) {
                env[assignInstr->target->name] = LatticeValue{LatticeKind::Unknown, 0};
            } else {
                env[assignInstr->target->name] = LatticeValue{LatticeKind::Top, 0};
            }
        } else {
            // 来源复杂（如 memory），置 Top
            env[assignInstr->target->name] = LatticeValue{LatticeKind::Top, 0};
        }
    } 
    // BinaryOpInstr
    else if (auto binOp = std::dynamic_pointer_cast<BinaryOpInstr>(instr)) {
        // 尝试如果左右都是常量，则计算结果
        auto L = valueOfOperand(binOp->left, env);
        auto R = valueOfOperand(binOp->right, env);
        if (L.kind == LatticeKind::Constant && R.kind == LatticeKind::Constant) {
            int outv;
            if (tryEvalBinaryOp(binOp, L.constantValue, R.constantValue, outv)) {
                env[binOp->result->name] = LatticeValue{LatticeKind::Constant, outv};
            } else {
                env[binOp->result->name] = LatticeValue{LatticeKind::Top, 0};
            }
        } else if (L.kind == LatticeKind::Unknown || R.kind == LatticeKind::Unknown) {
            env[binOp->result->name] = LatticeValue{LatticeKind::Unknown, 0};
        } else {
            env[binOp->result->name] = LatticeValue{LatticeKind::Top, 0};
        }
    } 
    // UnaryOpInstr
    else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpInstr>(instr)) {
        auto V = valueOfOperand(unaryOp->operand, env);
        if (V.kind == LatticeKind::Constant) {

            // 按照对应的操作符执行操作
            int outv = V.constantValue;
            if(unaryOp->opcode == OpCode::NEG) outv = -outv;
            else if(unaryOp->opcode == OpCode::NOT) outv = !outv;

            env[unaryOp->result->name] = LatticeValue{LatticeKind::Constant, outv};
        } else if (V.kind == LatticeKind::Unknown) {
            env[unaryOp->result->name] = LatticeValue{LatticeKind::Unknown, 0};
        } else {
            env[unaryOp->result->name] = LatticeValue{LatticeKind::Top, 0};
        }
    } 
    // CallInstr
    else if (auto callInstr = std::dynamic_pointer_cast<CallInstr>(instr)) {
        // 保守处理：函数调用可能有副作用，result 置 Top；如果你能保证调用不影响其他变量，可优化
        if (callInstr->result) env[callInstr->result->name] = LatticeValue{LatticeKind::Top, 0};
    } 
    // 其他指令
    else {
        // GotoInstr / IfGotoInstr / ParamInstr / ReturnInstr / LabelInstr / FunctionBeginInstr / FunctionEndInstr
        // 不会产生新的定义，因此 env 保持不变
    }
}

// ---------- 主分析与替换（CFG 版常量传播） ----------
void IRGenerator::constantPropagationCFG() {
    // 1. 构建 basic blocks 与 CFG
    auto blocks = buildBasicBlocks();
    buildCFG(blocks);

    int n = (int)blocks.size();
    if (n == 0) return;

    // 生成 CFG 映射：BlockID -> 后继列表
    std::unordered_map<int, std::vector<int>> cfg;
    for (auto& b : blocks) {
        std::vector<int> succIds;
        for (auto& s : b->successors) {
            succIds.push_back(s->id);
        }
        cfg[b->id] = std::move(succIds);
    }

    // 2. 找回边
    auto backEdges = findBackEdges(cfg);

    // 3. 针对回边，找循环体内所有定义变量集合
    //    这里需要一个 BlockID -> BasicBlock 映射方便访问
    std::unordered_map<int, IRGenerator::BasicBlock> blocksMap;
    for (auto& b : blocks) blocksMap[b->id] = *b;
    

    // 循环入口块ID -> 循环内所有定义变量集合
    std::unordered_map<int, std::unordered_set<std::string>> loopDefs;
    for (auto& edge : backEdges) {
        int fromBlk = edge.first;
        int toBlk = edge.second;

        //auto defs = getLoopDefs(cfg, fromBlk, toBlk, blocksMap);

        // 用 getLoopBlocks 得到该回边的自然循环块集合
        auto loopBlocks = getLoopBlocks(cfg, fromBlk, toBlk);

        // 收集该循环体内所有定义变量
        auto defs = getLoopDefs(loopBlocks, blocksMap);

        // 合并 defs 到 loopDefs，key用循环入口块ID（toBlk）
        auto& defSet = loopDefs[toBlk];
        defSet.insert(defs.begin(), defs.end());

    }

    // 4. 初始化 in/out map
    std::vector<ConstMap> inMap(n), outMap(n);

    // 5. worklist（初始把入口块放入）
    std::queue<int> q;
    // 假定 blocks[0] 是入口（如果函数有多入口或特殊结构需修改）
    q.push(0);

    while (!q.empty()) {
        int bid = q.front(); q.pop();
        auto blk = blocks[bid];

        // 计算 inMap[bid]
        // in[bid] = meet(out[pred]) for all predecessors
        if (blk->predecessors.empty()) {
            inMap[bid].clear();
        } else {
            ConstMap accum;
            bool first = true;
            for (auto& p : blk->predecessors) {
                if (first) {
                    accum = outMap[p->id];
                    first = false;
                } else {
                    accum = meetMaps(accum, outMap[p->id]);
                }
            }

            // 清除循环定义变量的常量状态（只有循环入口块才清除）
            /*if (loopDefs.count(bid)) {
                clearLoopDefs(accum, loopDefs, bid);
            }*/

            inMap[bid] = accum;
        }


        // 计算 outMap[bid]
        // out = transfer(in, block.instructions)
        ConstMap outEnv = inMap[bid];
        for (auto& instr : blk->instructions) {
            applyTransferToEnv(outEnv, instr);
        }

        // 如果 out 改变，更新并把所有后继放入队列
        if (!constMapsEqual(outEnv, outMap[bid])) {
            outMap[bid] = outEnv;
            for (auto& succ : blk->successors) q.push(succ->id);
        }
    }

    // 6. 用 inMap 替换每个基本块内部可确定为常量的操作数（在替换时顺序应用 transfer）
    for (int bid = 0; bid < n; ++bid) {

        // 获取当前块的常量环境（inMap）和基本块对象
        ConstMap env = inMap[bid];
        auto blk = blocks[bid];

        // 遍历块中的每条指令
        for (auto& instr : blk->instructions) {
            // 处理赋值指令
            if (auto assignInstr = std::dynamic_pointer_cast<AssignInstr>(instr)) {
                // 检查源操作数是否为变量/临时变量
                if (assignInstr->source->type == OperandType::VARIABLE || assignInstr->source->type == OperandType::TEMP) {
                    // 在环境查找变量状态
                    auto it = env.find(assignInstr->source->name);
                    // 如果是常量则替换为常量操作数
                    if (it != env.end() && it->second.kind == LatticeKind::Constant) {
                        assignInstr->source = makeConstantOperand(it->second.constantValue, assignInstr->source->name);
                    }
                }
            } 
            // 处理二元运算指令
            else if (auto binOp = std::dynamic_pointer_cast<BinaryOpInstr>(instr)) {
                // 检查左操作数
                if (binOp->left->type == OperandType::VARIABLE || binOp->left->type == OperandType::TEMP) {
                    auto it = env.find(binOp->left->name);
                    if (it != env.end() && it->second.kind == LatticeKind::Constant) {
                        binOp->left = makeConstantOperand(it->second.constantValue, binOp->left->name);
                    }
                }
                // 检查右操作数
                if (binOp->right->type == OperandType::VARIABLE || binOp->right->type == OperandType::TEMP) {
                    auto it = env.find(binOp->right->name);
                    if (it != env.end() && it->second.kind == LatticeKind::Constant) {
                        binOp->right = makeConstantOperand(it->second.constantValue, binOp->right->name);
                    }
                }
            } 
            // 处理一元运算指令
            else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOpInstr>(instr)) {
                if (unaryOp->operand->type == OperandType::VARIABLE || unaryOp->operand->type == OperandType::TEMP) {
                    auto it = env.find(unaryOp->operand->name);
                    if (it != env.end() && it->second.kind == LatticeKind::Constant) {
                        unaryOp->operand = makeConstantOperand(it->second.constantValue, unaryOp->operand->name);
                    }
                }
            } 
            // 处理参数传递指令
            else if (auto paramInstr = std::dynamic_pointer_cast<ParamInstr>(instr)) {
                if (paramInstr->param->type == OperandType::VARIABLE || paramInstr->param->type == OperandType::TEMP) {
                    auto it = env.find(paramInstr->param->name);
                    if (it != env.end() && it->second.kind == LatticeKind::Constant) {
                        paramInstr->param = makeConstantOperand(it->second.constantValue, paramInstr->param->name);
                    }
                }
            } 
            // 处理返回指令
            else if (auto returnInstr = std::dynamic_pointer_cast<ReturnInstr>(instr)) {
                if (returnInstr->value && (returnInstr->value->type == OperandType::VARIABLE || returnInstr->value->type == OperandType::TEMP)) {
                    auto it = env.find(returnInstr->value->name);
                    if (it != env.end() && it->second.kind == LatticeKind::Constant) {
                        returnInstr->value = makeConstantOperand(it->second.constantValue, returnInstr->value->name);
                    }
                }
            } 
            // 处理条件跳转指令
            else if (auto ifg = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
                if (ifg->condition->type == OperandType::VARIABLE || ifg->condition->type == OperandType::TEMP) {
                    auto it = env.find(ifg->condition->name);
                    if (it != env.end() && it->second.kind == LatticeKind::Constant) {
                        ifg->condition = makeConstantOperand(it->second.constantValue, ifg->condition->name);
                    }
                }
            }
            // 逐条指令应用 transfer，使 env 随 block 内顺序更新（保守，保持一致）
            applyTransferToEnv(env, instr);
        }
    }

    // 7. 再执行常量折叠（已有的函数）
    constantFolding();
}


/**
 * 执行死代码消除优化（Dead Code Elimination, DCE）
 * 算法步骤：
 * 1. 构建基本块和控制流图（CFG）
 * 2. 计算每个基本块的use和def集合
 * 3. 迭代计算live_in和live_out集合（数据流分析）
 * 4. 反向扫描指令，删除未被使用的定义
 */
void IRGenerator::deadCodeElimination() {
     // Step 0: 构建基本块和控制流图
    auto basicBlocks = buildBasicBlocks();
    buildCFG(basicBlocks);

    // Step 1: 收集 use / def
    std::unordered_map<std::shared_ptr<BasicBlock>, std::unordered_set<std::string>> use, def;
    for (auto& block : basicBlocks) {
        for (auto& instr : block->instructions) {
            // 获取当前指令定义和使用的变量
            auto defs = IRAnalyzer::getDefinedVariables(instr);
            auto uses = IRAnalyzer::getUsedVariables(instr);

            // 构建use集合：变量在被定义前被使用
            for (auto& u : uses) {
                if (def[block].find(u) == def[block].end()) {
                    use[block].insert(u);
                }
            }

            // 构建def集合：当前指令定义的所有变量
            for (auto& d : defs) {
                def[block].insert(d);
            }
        }
    }


    // Step 2: 计算活跃变量（live_in和live_out）
    std::unordered_map<std::shared_ptr<BasicBlock>, std::unordered_set<std::string>> live_in, live_out;

    // 初始化 worklist（逆序放入所有块）
    std::queue<std::shared_ptr<BasicBlock>> worklist;
    std::unordered_set<std::shared_ptr<BasicBlock>> inQueue;
    for (auto it = basicBlocks.rbegin(); it != basicBlocks.rend(); ++it) {
        worklist.push(*it);
        inQueue.insert(*it);
    }

    while (!worklist.empty()) {
        auto block = worklist.front();
        worklist.pop();
        inQueue.erase(block);

        // live_out = 后继的 live_in 并集
        std::unordered_set<std::string> new_live_out;
        for (auto succ : block->successors) {
            new_live_out.insert(live_in[succ].begin(), live_in[succ].end());
        }

        // live_in = use ∪ (live_out - def)
        std::unordered_set<std::string> new_live_in = use[block];
        for (auto& var : new_live_out) {
            if (def[block].find(var) == def[block].end()) {
                new_live_in.insert(var);
            }
        }

        // 如果有变化，更新并把前驱加入队列
        if (new_live_in != live_in[block] || new_live_out != live_out[block]) {
            live_in[block] = std::move(new_live_in);
            live_out[block] = std::move(new_live_out);

            for (auto pred : block->predecessors) {
                if (!inQueue.count(pred)) {
                    worklist.push(pred);
                    inQueue.insert(pred);
                }
            }
        }
    }


    // Step 3: 反向删除死代码
    for (auto& block : basicBlocks) {
        auto live = live_out[block];    // 初始化为基本块出口的活跃变量集合

        // 反向遍历指令（从后往前）
        for (auto it = block->instructions.rbegin(); it != block->instructions.rend(); ) {
            auto defs = IRAnalyzer::getDefinedVariables(*it);
            auto uses = IRAnalyzer::getUsedVariables(*it);

            bool hasSideEffect = isSideEffectInstr(*it);    // 检查是否是副作用指令

            // 判断当前指令是否定义了活跃变量
            bool isLive = false;
            for (auto& d : defs) {
                if (live.find(d) != live.end()) {
                    isLive = true;
                    break;
                }
            }

            // 删除条件：1. 未定义活跃变量 2. 无副作用 3. 实际有定义（避免删除空指令）
            if (!isLive && !hasSideEffect && !defs.empty()) {
                // 删除死代码
                it = decltype(it){ block->instructions.erase(std::next(it).base()) };
                continue;
            }

            // 更新 live 集合
            for (auto& d : defs) {
                live.erase(d);      // 定义的变量不再活跃
            }
            for (auto& u : uses) {
                live.insert(u);     // 使用的变量变为活跃
            }

            ++it;
        }
    }
}


/**
 * 判断指令是否具有副作用
 * 有副作用的指令包括：
 * - 函数调用（可能修改外部状态）
 * - 控制流指令（跳转、返回等）
 * - 标签和函数边界指令
 * - 参数传递指令
 */
bool IRGenerator::isSideEffectInstr(const std::shared_ptr<IRInstr>& instr) {
    return
        std::dynamic_pointer_cast<CallInstr>(instr) != nullptr ||               // 函数调用
        std::dynamic_pointer_cast<ReturnInstr>(instr) != nullptr ||             // 返回指令
        std::dynamic_pointer_cast<GotoInstr>(instr) != nullptr ||               // 无条件跳转
        std::dynamic_pointer_cast<IfGotoInstr>(instr) != nullptr ||             // 条件跳转
        std::dynamic_pointer_cast<LabelInstr>(instr) != nullptr ||              // 标签
        std::dynamic_pointer_cast<FunctionBeginInstr>(instr) != nullptr ||      // 函数开始
        std::dynamic_pointer_cast<FunctionEndInstr>(instr) != nullptr ||        // 函数结束
        std::dynamic_pointer_cast<ParamInstr>(instr) != nullptr;                // 参数传递
}

/**
 * 执行控制流优化（Control Flow Optimization）
 * 主要包含四个优化阶段：
 * 1. 删除不可达基本块
 * 2. 合并直连基本块
 * 3. 删除多余跳转
 * 4. 重新线性化指令
 */
/*void IRGenerator::controlFlowOptimization() {

    // 构建基本块和控制流图
    auto blocks = buildBasicBlocks();
    buildCFG(blocks);

    if (blocks.empty()) return;

    // ==== Step 1: 删除不可达基本块 ====
    std::unordered_set<std::shared_ptr<BasicBlock>> reachable;  // 可达基本块集合
    std::function<void(std::shared_ptr<BasicBlock>)> dfs =
        [&](std::shared_ptr<BasicBlock> blk) {
            // 跳过空块或已访问块
            if (!blk || reachable.count(blk)) return;
            reachable.insert(blk);  // 标记当前块为可达
            // 递归遍历所有后继块
            for (auto& succ : blk->successors) {
                dfs(succ);
            }
        };

    // 从入口块开始深度优先遍历
    dfs(blocks.front());    // 假设第一个基本块是入口块

    // 过滤掉不可达的基本块
    std::vector<std::shared_ptr<BasicBlock>> newBlocks;
    for (auto& blk : blocks) {
        if (reachable.count(blk)) {
            newBlocks.push_back(blk);
        }
    }
    blocks.swap(newBlocks); // 更新基本块列表

    // ==== Step 2: 合并直连基本块 ====
    std::unordered_set<std::shared_ptr<BasicBlock>> toRemove;    // 待删除块
    for (auto& blk : blocks) {
        if (blk->instructions.empty()) continue;    // 跳过空块

        // 检查最后一条指令是否为goto
        if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(blk->instructions.back())) {
            // 获取唯一后继块（当前块必须有且仅有一个后继）
            auto target = blk->successors.size() == 1 ? blk->successors[0] : nullptr;
            
            // 满足合并条件：后继块有且仅有一个前驱（即当前块）
            if (target && target->predecessors.size() == 1) {
                blk->instructions.pop_back();  // 删除跳转指令

                // 保留目标块首指令标签（如果有）
                if (!target->instructions.empty()) {
                    auto firstInstr = target->instructions.front();
                    if (auto labelInstr = std::dynamic_pointer_cast<LabelInstr>(firstInstr)) {
                        blk->instructions.push_back(labelInstr);    // 保留标签
                        // 合并剩余指令
                        blk->instructions.insert(
                            blk->instructions.end(),
                            target->instructions.begin() + 1,
                            target->instructions.end()
                        );
                    } else {
                        // 无标签则全合并
                        blk->instructions.insert(
                            blk->instructions.end(),
                            target->instructions.begin(),
                            target->instructions.end()
                        );
                    }
                }

                // 更新 CFG
                blk->successors = target->successors;

                // 更新后继块的前驱指向当前块
                for (auto& succ : target->successors) {
                    for (auto& pred : succ->predecessors) {
                        if (pred == target) {
                            pred = blk;
                        }
                    }
                }
                toRemove.insert(target);
            }
        }
    }

    // 删除合并掉的块
    blocks.erase(
        std::remove_if(blocks.begin(), blocks.end(),
            [&](const std::shared_ptr<BasicBlock>& b) { return toRemove.count(b); }),
        blocks.end()
    );

    // ==== Step 3: 删除多余跳转 ====
    for (size_t i = 0; i + 1 < blocks.size(); ++i) {
        auto& blk = blocks[i];
        if (blk->instructions.empty()) continue;

        // 检查最后一条指令是否为跳转
        if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(blk->instructions.back())) {
            auto& nextBlk = blocks[i + 1];  // 物理相邻的下一个块

            // 确认 nextBlk 的第一个指令是标签
            if (!nextBlk->instructions.empty()) {
                if (auto labelInstr = std::dynamic_pointer_cast<LabelInstr>(nextBlk->instructions.front())) {
                    // 如果跳转目标就是下一个块，则删除冗余跳转
                    if (gotoInstr->target && gotoInstr->target->name == labelInstr->label) {
                        blk->instructions.pop_back();    // 删除跳转指令
                    }
                }
            }
        }
    }

    // ==== Step 4: 重新线性化指令 ====
    instructions.clear();   // 清空原始指令序列
    std::unordered_set<std::shared_ptr<BasicBlock>> visited;

    // 深度优先遍历按控制流顺序重组指令
    std::function<void(std::shared_ptr<BasicBlock>)> dfsLinearize = [&](std::shared_ptr<BasicBlock> blk) {
        if (!blk || visited.count(blk)) return;
        visited.insert(blk);
        // 添加当前块指令
        instructions.insert(instructions.end(), blk->instructions.begin(), blk->instructions.end());
        // 递归处理后继
        for (auto& succ : blk->successors) {
            dfsLinearize(succ);
        }
    };

    dfsLinearize(blocks.front());   // 从入口块开始重组
}*/

// 辅助：更新所有跳转指令目标标签，fromLabel -> toLabel
void IRGenerator::updateJumpTargets(
    std::vector<std::shared_ptr<BasicBlock>>& blocks,
    const std::string& fromLabel,
    const std::string& toLabel)
{
    for (auto& blk : blocks) {
        for (auto& instr : blk->instructions) {
            if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(instr)) {
                if (gotoInstr->target && gotoInstr->target->name == fromLabel) {
                    gotoInstr->target->name = toLabel;
                }
            }
            if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
                if (ifGotoInstr->target && ifGotoInstr->target->name == fromLabel) {
                    ifGotoInstr->target->name = toLabel;
                }
            }
        }
    }
}

// 校验 CFG 有效性，标签唯一且跳转目标存在
bool IRGenerator::validateCFG(const std::vector<std::shared_ptr<BasicBlock>>& blocks) {
    std::unordered_set<std::string> allLabels;
    std::unordered_set<std::string> usedLabels;

    for (const auto& blk : blocks) {
        if (blk->instructions.empty()) continue;
        // 基本块首指令必须是标签
        auto firstInstr = blk->instructions.front();
        auto labelInstr = std::dynamic_pointer_cast<LabelInstr>(firstInstr);
        if (!labelInstr) {
            std::cerr << "Error: BasicBlock missing starting LabelInstr\n";
            return false;
        }
        // 标签唯一
        if (allLabels.count(labelInstr->label)) {
            std::cerr << "Error: Duplicate label: " << labelInstr->label << "\n";
            return false;
        }
        allLabels.insert(labelInstr->label);

        // 收集跳转目标
        for (const auto& instr : blk->instructions) {
            if (auto g = std::dynamic_pointer_cast<GotoInstr>(instr)) {
                if (g->target) usedLabels.insert(g->target->name);
            }
            if (auto ig = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
                if (ig->target) usedLabels.insert(ig->target->name);
            }
        }
    }

    // 检查跳转目标是否都存在
    for (const auto& label : usedLabels) {
        if (!allLabels.count(label)) {
            std::cerr << "Error: Jump target label not found: " << label << "\n";
            return false;
        }
    }
    return true;
}

void IRGenerator::controlFlowOptimization() {
    // 构建基本块和CFG
    auto blocks = buildBasicBlocks();
    buildCFG(blocks);
    if (blocks.empty()) return;

    // Step 1: 删除不可达基本块
    std::unordered_set<std::shared_ptr<BasicBlock>> reachable;      // 存储可达块
    std::unordered_set<std::shared_ptr<BasicBlock>> inProgress;                     // 存储正在处理的块，用于检测循环

    /*std::function<void(std::shared_ptr<BasicBlock>)> dfs =
        [&](std::shared_ptr<BasicBlock> blk) {
            if (!blk || reachable.count(blk)) return;
            reachable.insert(blk);
            for (auto& succ : blk->successors) {
                dfs(succ);
            }
        };*/

    // 使用安全的DFS遍历，避免循环导致的无限递归
    std::function<void(std::shared_ptr<BasicBlock>)> dfs = [&](std::shared_ptr<BasicBlock> blk) {
        if (!blk || reachable.count(blk)) return;  // 已访问的块直接返回
    
        // 检测是否在当前DFS路径上（循环检测）
        if (inProgress.count(blk)) return;         // 如果当前块正在处理中，直接返回
    
        inProgress.insert(blk);                    // 标记当前块为处理中
        reachable.insert(blk);                     // 标记当前块为可达
    
        // 递归处理所有后继块
        for (auto succ : blk->successors) {
            dfs(succ);
        }
    
        inProgress.erase(blk);                     // 处理完成，从处理中集合移除
    };


    // 寻找程序入口
    std::shared_ptr<BasicBlock> entry = nullptr;

    for (auto& blk : blocks) {
        for (auto& ins : blk->instructions) {
            if (auto funcInstr = std::dynamic_pointer_cast<FunctionBeginInstr>(ins)) {
                if (funcInstr->funcName == "main") { 
                    entry = blk;
                    goto found; // 直接跳出双层循环
                }
            }
        }
    }

    found:
    if (!entry) {
        throw std::runtime_error("入口块 'main' 未找到");
    }

    dfs(entry);

    // 过滤不可达块
    std::vector<std::shared_ptr<BasicBlock>> newBlocks;
    for (auto& blk : blocks) {
        if (reachable.count(blk)) newBlocks.push_back(blk);
    }
    blocks.swap(newBlocks);


    // Step 2: 合并直连基本块
    std::unordered_set<std::shared_ptr<BasicBlock>> toRemove;

    for (auto& blk : blocks) {
        if (blk->instructions.empty()) continue;

        // 必须保证第一个指令是标签
        auto firstInstr = blk->instructions.front();
        auto blkLabelInstr = std::dynamic_pointer_cast<LabelInstr>(firstInstr);
        if (!blkLabelInstr) continue; // 【修改点】跳过无标签块，确保标签存在

        // 检查最后一条是否为goto
        if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(blk->instructions.back())) {
            if (blk->successors.size() != 1) continue;
            auto target = blk->successors[0];
            if (!target || target->instructions.empty()) continue;

            // 目标块必须只有一个前驱（即当前块）
            if (target->predecessors.size() != 1 || target->predecessors[0] != blk) continue;

            // 目标块第一个指令必须是标签
            auto targetLabelInstr = std::dynamic_pointer_cast<LabelInstr>(target->instructions.front());
            if (!targetLabelInstr) continue;

            // 【修改点】合并块前先记录标签名
            std::string blkLabel = blkLabelInstr->label;
            std::string targetLabel = targetLabelInstr->label;

            // 【修改点】合并时删除当前块尾部goto
            blk->instructions.pop_back();

            // 【修改点】删除目标块标签指令（保持块唯一标签）
            target->instructions.erase(target->instructions.begin());

            // 合并目标块剩余指令
            blk->instructions.insert(
                blk->instructions.end(),
                target->instructions.begin(),
                target->instructions.end()
            );

            // 更新CFG successors
            blk->successors = target->successors;

            // 更新目标块后继的predecessors指向当前块
            for (auto& succ : target->successors) {
                for (auto& pred : succ->predecessors) {
                    if (pred == target) {
                        pred = blk;
                    }
                }
            }

            // 【修改点】更新所有跳转指令指向目标标签的，改为指向当前块标签
            updateJumpTargets(blocks, targetLabel, blkLabel);

            toRemove.insert(target);
        }
    }

    // 删除合并块
    blocks.erase(
        std::remove_if(blocks.begin(), blocks.end(),
            [&](const std::shared_ptr<BasicBlock>& b) { return toRemove.count(b); }),
        blocks.end()
    );

    // Step 3: 删除多余跳转，且同步更新 successors
    for (size_t i = 0; i + 1 < blocks.size(); ++i) {
        auto& blk = blocks[i];
        if (blk->instructions.empty()) continue;

        if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(blk->instructions.back())) {
            auto& nextBlk = blocks[i + 1];
            if (nextBlk->instructions.empty()) continue;

            auto labelInstr = std::dynamic_pointer_cast<LabelInstr>(nextBlk->instructions.front());
            if (!labelInstr) continue;

            if (gotoInstr->target && gotoInstr->target->name == labelInstr->label) {
                // 删除跳转指令
                blk->instructions.pop_back();

                // 【修改点】同步删除 successors 中的 nextBlk
                blk->successors.erase(
                    std::remove(blk->successors.begin(), blk->successors.end(), nextBlk),
                    blk->successors.end()
                );
            }
        }
    }

    // Step 4: 重新线性化指令，优先fall-through后继
    instructions.clear();   // 清空最终的指令序列，准备重新生成
    for (auto& block : blocks) {
        instructions.insert(instructions.end(), block->instructions.begin(), block->instructions.end());
    }
    /*std::unordered_set<std::shared_ptr<BasicBlock>> visited;

    // 定义深度优先搜索（DFS）函数，用于线性化基本块
    std::function<void(std::shared_ptr<BasicBlock>)> dfsLinearize = [&](std::shared_ptr<BasicBlock> blk) {
        // 如果基本块为空或已访问过，直接返回
        if (!blk || visited.count(blk)) return;

        // 标记当前基本块为已访问
        visited.insert(blk);

        // 将当前基本块的所有指令追加到最终的指令序列中
        instructions.insert(instructions.end(), blk->instructions.begin(), blk->instructions.end());

        // 找fall-through后继（非跳转目标）
        std::shared_ptr<BasicBlock> fallthrough = nullptr;
        if (!blk->instructions.empty()) {
            auto lastInstr = blk->instructions.back();  // 获取最后一条指令

            // 如果是 `Goto` 或 `Return`，则没有 fall-through
            if (std::dynamic_pointer_cast<GotoInstr>(lastInstr) || std::dynamic_pointer_cast<ReturnInstr>(lastInstr)) {
                fallthrough = nullptr; // 无fall-through
            } 
            // 如果是 `IfGoto`（条件跳转），则 fall-through 是第二个后继（false 分支）
            else if (auto ifGoto = std::dynamic_pointer_cast<IfGotoInstr>(lastInstr)) {
                if (blk->successors.size() > 1) fallthrough = blk->successors[1];
            } 
            // 其他情况（普通指令），fall-through 是唯一后继（如果有的话）
            else {
                if (blk->successors.size() == 1) fallthrough = blk->successors[0];
            }
        }

        // 优先处理 fall-through 后继（保证顺序执行）
        if (fallthrough) dfsLinearize(fallthrough);

        // 处理其他后继（跳转目标）
        for (auto& succ : blk->successors) {
            if (succ != fallthrough) dfsLinearize(succ);
        }
    };

    // 从入口基本块开始线性化
    dfsLinearize(entry);*/

    // Step 5: 最后校验CFG有效性，避免标签或跳转错误
    if (!validateCFG(blocks)) {
        std::cerr << "Error: CFG validation failed after controlFlowOptimization\n";
        // 这里可考虑回滚或抛异常
    }
}





/**
 * 控制流优化。
 * 
 * 优化程序的控制流，包括跳转和分支。
 * 例如，消除冗余跳转，优化分支条件，
 * 简化控制流模式。
 */
/*void IRGenerator::controlFlowOptimization() {
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
}*/

/**
 * 检查指令是否是控制流指令。
 * 
 * 控制流指令改变执行流程，如跳转和返回。
 * 
 * @param instr 要检查的指令
 * @return 如果指令是控制流指令，则为true
 */
/*bool IRGenerator::isControlFlowInstruction(const std::shared_ptr<IRInstr>& instr) const {
    return std::dynamic_pointer_cast<GotoInstr>(instr) != nullptr ||
           std::dynamic_pointer_cast<IfGotoInstr>(instr) != nullptr ||
           std::dynamic_pointer_cast<ReturnInstr>(instr) != nullptr;
}*/

/**
 * 获取控制流指令的目标标签。
 * 
 * @param instr 控制流指令
 * @return 目标标签名的向量
 */
/*std::vector<std::string> IRGenerator::getControlFlowTargets(const std::shared_ptr<IRInstr>& instr) const {
    std::vector<std::string> targets;
    
    if (auto gotoInstr = std::dynamic_pointer_cast<GotoInstr>(instr)) {
        targets.push_back(gotoInstr->target->name);
    }
    else if (auto ifGotoInstr = std::dynamic_pointer_cast<IfGotoInstr>(instr)) {
        targets.push_back(ifGotoInstr->target->name);
    }
    
    return targets;
}*/

/**
 * 从IR指令构建控制流图。
 * 
 * 控制流图表示程序中的执行流程。
 * 每个节点（基本块）是按顺序执行的一系列指令，
 * 边表示块之间可能的控制流。
 * 
 * @return 从标签名到基本块的映射
 */
/*std::map<std::string, IRGenerator::BasicBlock> IRGenerator::buildControlFlowGraph() {
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
}*/

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