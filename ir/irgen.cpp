// ==== ir/irgen.cpp ====
#include "ir.h"
#include <iostream>
#include <sstream>

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

std::shared_ptr<Operand> IRGenerator::getVariable(const std::string& name) {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second;
    }
    
    // 如果变量不存在，创建一个新的
    std::shared_ptr<Operand> var = std::make_shared<Operand>(OperandType::VARIABLE, name);
    variables[name] = var;
    return var;
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

void IRGenerator::visit(UnaryExpr& expr) {
    expr.operand->accept(*this);
    std::shared_ptr<Operand> operand = getTopOperand();
    
    std::shared_ptr<Operand> result = createTemp();
    
    OpCode opcode;
    if (expr.op == "-") opcode = OpCode::NEG;
    else if (expr.op == "!") opcode = OpCode::NOT;
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
    for (const auto& s : stmt.statements) {
        s->accept(*this);
    }
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