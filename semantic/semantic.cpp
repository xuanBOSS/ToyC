
#include "semantic.h"
#include <iostream>
#include <sstream>

SemanticAnalyzer::SemanticAnalyzer() {
    // 初始全局作用域
    enterScope();
    // 添加内置函数
    addBuiltinFunctions();
}

SemanticAnalyzer::~SemanticAnalyzer() {
    // 清理所有作用域
    while (!symbolTables.empty()) {
        exitScope();
    }
}

void SemanticAnalyzer::addBuiltinFunctions() {
    // 添加内置函数，例如 getint, putint 等
    FunctionInfo getintInfo("int", true);
    functionTable["getint"] = getintInfo;
    
    FunctionInfo putintInfo("void", true);
    putintInfo.paramTypes.push_back("int");
    putintInfo.paramNames.push_back("value");
    functionTable["putint"] = putintInfo;
    
    // ----------------------根据需要添加更多内置函数-------------------------------
}

bool SemanticAnalyzer::analyze(std::shared_ptr<CompUnit> ast) {
    try {
        ast->accept(*this);
        return success;
    } catch (const SemanticError& e) {
        std::string location = e.line > 0 ? 
            " at line " + std::to_string(e.line) + 
            (e.column > 0 ? ", column " + std::to_string(e.column) : "") : "";
        
        std::cerr << "Semantic error" << location << ": " << e.what() << std::endl;
        errorMessages.push_back(std::string(e.what()) + location);
        return false;
    }
}

void SemanticAnalyzer::enterScope() {
    symbolTables.push_back(std::unordered_map<std::string, Symbol>());
}

void SemanticAnalyzer::exitScope() {
    if (!symbolTables.empty()) {
        // 检查未使用的变量
        checkUnusedVariables();
        symbolTables.pop_back();
    }
}

bool SemanticAnalyzer::declareSymbol(const std::string& name, Symbol symbol) {
    if (symbolTables.back().find(name) != symbolTables.back().end()) {
        return false; // 已在当前作用域声明
    }
    
    symbolTables.back()[name] = symbol;
    return true;
}

bool SemanticAnalyzer::isDeclared(const std::string& name) {
    for (auto it = symbolTables.rbegin(); it != symbolTables.rend(); ++it) {
        if (it->find(name) != it->end()) {
            return true;
        }
    }
    return false;
}

bool SemanticAnalyzer::isLocallyDeclared(const std::string& name) {
    return symbolTables.back().find(name) != symbolTables.back().end();
}

Symbol* SemanticAnalyzer::findSymbol(const std::string& name) {
    for (auto it = symbolTables.rbegin(); it != symbolTables.rend(); ++it) {
        auto symIt = it->find(name);
        if (symIt != it->end()) {
            return &symIt->second;
        }
    }
    return nullptr;
}

bool SemanticAnalyzer::declareFunction(const std::string& name, const FunctionInfo& info) {
    if (functionTable.find(name) != functionTable.end()) {
        // 函数已声明，检查是否一致
        FunctionInfo& existingInfo = functionTable[name];
        if (existingInfo.defined && info.defined) {
            return false; // 重复定义
        }
        if (existingInfo.returnType != info.returnType) {
            return false; // 返回类型不匹配
        }
        if (existingInfo.paramTypes.size() != info.paramTypes.size()) {
            return false; // 参数数量不匹配
        }
        for (size_t i = 0; i < info.paramTypes.size(); i++) {
            if (existingInfo.paramTypes[i] != info.paramTypes[i]) {
                return false; // 参数类型不匹配
            }
        }
        // 更新为已定义
        if (info.defined) {
            existingInfo.defined = true;
        }
        return true;
    }
    
    functionTable[name] = info;
    return true;
}

bool SemanticAnalyzer::isFunctionDeclared(const std::string& name) {
    return functionTable.find(name) != functionTable.end();
}

FunctionInfo* SemanticAnalyzer::findFunction(const std::string& name) {
    auto it = functionTable.find(name);
    if (it != functionTable.end()) {
        return &it->second;
    }
    return nullptr;
}

void SemanticAnalyzer::error(const std::string& message, int line, int column) {
    success = false;
    std::string fullMessage = message;
    if (line > 0) {
        fullMessage += " at line " + std::to_string(line);
        if (column > 0) {
            fullMessage += ", column " + std::to_string(column);
        }
    }
    errorMessages.push_back(fullMessage);
    throw SemanticError(message, line, column);
}

void SemanticAnalyzer::warning(const std::string& message, int line, int column) {
    std::string fullMessage = message;
    if (line > 0) {
        fullMessage += " at line " + std::to_string(line);
        if (column > 0) {
            fullMessage += ", column " + std::to_string(column);
        }
    }
    warningMessages.push_back(fullMessage);
    std::cerr << "Warning: " << fullMessage << std::endl;
}

std::string SemanticAnalyzer::getExprType(Expr& expr) {
    // 递归求表达式类型
    struct TypeVisitor : public ASTVisitor {
    std::string type;
    SemanticAnalyzer& analyzer;
    
    TypeVisitor(SemanticAnalyzer& analyzer) : analyzer(analyzer) {}
    
    void visit(NumberExpr&) override { type = "int"; }
    
    void visit(VariableExpr& expr) override {
        Symbol* symbol = analyzer.findSymbol(expr.name);
        if (!symbol) {
            analyzer.error("Undefined variable: " + expr.name);
        }
        type = symbol->type;
    }
    
    void visit(BinaryExpr& expr) override {
        expr.left->accept(*this);
        std::string leftType = type;
        expr.right->accept(*this);
        std::string rightType = type;
        
        if (leftType != "int" || rightType != "int") {
            analyzer.error("Binary operator requires int operands");
        }
        
        // 关系运算符返回int（在ToyC中作为布尔值）
        if (expr.op == "<" || expr.op == ">" || expr.op == "<=" || 
            expr.op == ">=" || expr.op == "==" || expr.op == "!=" || 
            expr.op == "&&" || expr.op == "||") {
            type = "int";  // 布尔结果
        } else {
            type = "int";  // 算术结果
        }
    }
    
    void visit(UnaryExpr& expr) override {
        expr.operand->accept(*this);
        if (type != "int") {
            analyzer.error("Unary operator requires int operand");
        }
        type = "int";
    }
    
    void visit(CallExpr& expr) override {
        auto it = analyzer.functionTable.find(expr.callee);
        if (it == analyzer.functionTable.end()) {
            analyzer.error("Undefined function: " + expr.callee);
        }
        
        if (it->second.paramTypes.size() != expr.arguments.size()) {
            analyzer.error("Function " + expr.callee + " called with wrong number of arguments");
        }
        
        for (size_t i = 0; i < expr.arguments.size(); ++i) {
            expr.arguments[i]->accept(*this);
            if (type != it->second.paramTypes[i]) {
                analyzer.error("Argument type mismatch in call to " + expr.callee);
            }
        }
        
        type = it->second.returnType;
    }
    
    // 以下是必须实现的其他visit方法
    void visit(ExprStmt& stmt) override { 
        if (stmt.expression) stmt.expression->accept(*this); 
    }
    void visit(VarDeclStmt& stmt) override { 
        if (stmt.initializer) stmt.initializer->accept(*this); 
    }
    void visit(AssignStmt& stmt) override { 
        stmt.value->accept(*this); 
    }
    void visit(BlockStmt& stmt) override { 
        for (auto& s : stmt.statements) s->accept(*this); 
    }
    void visit(IfStmt& stmt) override { 
        stmt.condition->accept(*this);
        stmt.thenBranch->accept(*this);
        if (stmt.elseBranch) stmt.elseBranch->accept(*this);
    }
    void visit(WhileStmt& stmt) override { 
        stmt.condition->accept(*this);
        stmt.body->accept(*this);
    }
    void visit(BreakStmt&) override { }
    void visit(ContinueStmt&) override { }
    void visit(ReturnStmt& stmt) override { 
        if (stmt.value) stmt.value->accept(*this); 
    }
    void visit(FunctionDef& funcDef) override { 
        funcDef.body->accept(*this); 
    }
    void visit(CompUnit& compUnit) override { 
        for (auto& func : compUnit.functions) func->accept(*this); 
    }

    };
    
    TypeVisitor visitor(*this);
    expr.accept(visitor);
    return visitor.type;
}

bool SemanticAnalyzer::checkAssignable(const std::string& leftType, const std::string& rightType) {
    // 在ToyC中，只有int类型可以赋值给int类型
    return leftType == rightType && leftType == "int";
}

bool SemanticAnalyzer::checkCallable(CallExpr& expr) {
    // 检查函数调用是否合法
    auto funcInfo = findFunction(expr.callee);
    if (!funcInfo) {
        return false;
    }
    
    if (funcInfo->paramTypes.size() != expr.arguments.size()) {
        return false;
    }
    
    for (size_t i = 0; i < expr.arguments.size(); i++) {
        std::string argType = getExprType(*expr.arguments[i]);
        if (argType != funcInfo->paramTypes[i]) {
            return false;
        }
    }
    
    return true;
}

void SemanticAnalyzer::enterLoop() {
    loopDepth++;
}

void SemanticAnalyzer::exitLoop() {
    loopDepth--;
}

bool SemanticAnalyzer::isInLoop() const {
    return loopDepth > 0;
}

void SemanticAnalyzer::checkReturnPaths(BlockStmt& block, bool isFunction) {
    // 检查所有路径是否都有return语句
    // 这是一个较复杂的分析，根据需要实现
    // 例如: 如果函数返回类型是int，所有的执行路径都必须有return语句
}

void SemanticAnalyzer::checkUnusedVariables() {
    // 检查当前作用域中未使用的变量
    for (const auto& pair : symbolTables.back()) {
        if (pair.second.kind == Symbol::Kind::VARIABLE && !pair.second.initialized) {
            warning("Variable '" + pair.first + "' is declared but never used", 
                    pair.second.line, pair.second.column);
        }
    }
}

bool SemanticAnalyzer::isValidMainFunction(FunctionDef& funcDef) {
    // 检查main函数是否合法
    if (funcDef.returnType != "int") {
        return false;
    }
    
    if (!funcDef.params.empty()) {
        return false;
    }
    
    return true;
}

void SemanticAnalyzer::checkInitialized(const std::string& name) {
    // 检查变量是否已初始化
    Symbol* symbol = findSymbol(name);
    if (symbol && !symbol->initialized) {
        warning("Variable '" + name + "' is used before being initialized", 
                symbol->line, symbol->column);
    }
}

void SemanticAnalyzer::markInitialized(const std::string& name) {
    // 将变量标记为已初始化
    Symbol* symbol = findSymbol(name);
    if (symbol) {
        symbol->initialized = true;
    }
}

int SemanticAnalyzer::getLineNumber(Expr& expr) {
    // 这个方法需要基于AST节点的位置信息实现
    // 为简单起见，这里返回0
    return 0;
}

int SemanticAnalyzer::getLineNumber(Stmt& stmt) {
    // 这个方法需要基于AST节点的位置信息实现
    // 为简单起见，这里返回0
    return 0;
}

// 以下是AST节点的访问方法

void SemanticAnalyzer::visit(NumberExpr& expr) {
    // 数字字面量总是int类型，无需额外检查
}

void SemanticAnalyzer::visit(VariableExpr& expr) {
    // 检查变量是否已声明
    Symbol* symbol = findSymbol(expr.name);
    if (!symbol) {
        error("Undefined variable: " + expr.name, getLineNumber(expr));
    }
    
    // 检查变量是否已初始化
    checkInitialized(expr.name);
}

void SemanticAnalyzer::visit(BinaryExpr& expr) {
    // 检查二元表达式的操作数类型
    expr.left->accept(*this);
    expr.right->accept(*this);
    
    std::string leftType = getExprType(*expr.left);
    std::string rightType = getExprType(*expr.right);
    
    if (leftType != "int" || rightType != "int") {
        error("Binary operator '" + expr.op + "' requires int operands", getLineNumber(expr));
    }
}

void SemanticAnalyzer::visit(UnaryExpr& expr) {
    // 检查一元表达式的操作数类型
    expr.operand->accept(*this);
    
    std::string operandType = getExprType(*expr.operand);
    if (operandType != "int") {
        error("Unary operator '" + expr.op + "' requires int operand", getLineNumber(expr));
    }
}

void SemanticAnalyzer::visit(CallExpr& expr) {
    // 检查函数调用
    if (!isFunctionDeclared(expr.callee)) {
        error("Undefined function: " + expr.callee, getLineNumber(expr));
    }
    
    FunctionInfo* funcInfo = findFunction(expr.callee);
    
    // 检查参数数量
    if (funcInfo->paramTypes.size() != expr.arguments.size()) {
        error("Function '" + expr.callee + "' called with wrong number of arguments", 
              getLineNumber(expr));
    }
    
    // 检查参数类型
    for (size_t i = 0; i < expr.arguments.size(); i++) {
        expr.arguments[i]->accept(*this);
        std::string argType = getExprType(*expr.arguments[i]);
        if (i < funcInfo->paramTypes.size() && argType != funcInfo->paramTypes[i]) {
            error("Argument type mismatch in call to '" + expr.callee + "'", 
                  getLineNumber(expr));
        }
    }
}

void SemanticAnalyzer::visit(ExprStmt& stmt) {
    if (stmt.expression) {
        stmt.expression->accept(*this);
    }
}

// 语句分析方法
void SemanticAnalyzer::visit(VarDeclStmt& stmt) {
    // 检查变量是否已声明
    if (isLocallyDeclared(stmt.name)) {
        error("Variable '" + stmt.name + "' is already declared in this scope", 
              getLineNumber(stmt));
    }
    
    // 检查初始值类型
    if (stmt.initializer) {
        stmt.initializer->accept(*this);
        std::string initType = getExprType(*stmt.initializer);
        if (initType != "int") {
            error("Cannot initialize int variable with non-int expression", 
                  getLineNumber(stmt));
        }
    }
    
    // 声明变量
    Symbol symbol(Symbol::Kind::VARIABLE, "int", stmt.initializer != nullptr, 
                  getLineNumber(stmt));
    declareSymbol(stmt.name, symbol);
}

void SemanticAnalyzer::visit(AssignStmt& stmt) {
    // 检查变量是否已声明
    Symbol* symbol = findSymbol(stmt.name);
    if (!symbol) {
        error("Undefined variable: " + stmt.name, getLineNumber(stmt));
    }
    
    // 检查变量类型
    if (symbol->kind != Symbol::Kind::VARIABLE && symbol->kind != Symbol::Kind::PARAMETER) {
        error("Cannot assign to '" + stmt.name + "' (not a variable)", 
              getLineNumber(stmt));
    }
    
    // 检查赋值表达式类型
    stmt.value->accept(*this);
    std::string valueType = getExprType(*stmt.value);
    
    if (!checkAssignable(symbol->type, valueType)) {
        error("Type mismatch in assignment to '" + stmt.name + "'", 
              getLineNumber(stmt));
    }
    
    // 标记变量为已初始化
    markInitialized(stmt.name);
}

void SemanticAnalyzer::visit(BlockStmt& stmt) {
    // 进入新的作用域
    enterScope();
    
    // 分析块中的每个语句
    for (auto& s : stmt.statements) {
        s->accept(*this);
    }
    
    // 离开作用域
    exitScope();
}

void SemanticAnalyzer::visit(IfStmt& stmt) {
    // 检查条件表达式
    stmt.condition->accept(*this);
    std::string condType = getExprType(*stmt.condition);
    if (condType != "int") {
        error("If condition must be of type int", getLineNumber(stmt));
    }
    
    // 分析then分支
    stmt.thenBranch->accept(*this);
    
    // 分析else分支(如果有)
    if (stmt.elseBranch) {
        stmt.elseBranch->accept(*this);
    }
}

void SemanticAnalyzer::visit(WhileStmt& stmt) {
    // 检查条件表达式
    stmt.condition->accept(*this);
    std::string condType = getExprType(*stmt.condition);
    if (condType != "int") {
        error("While condition must be of type int", getLineNumber(stmt));
    }
    
    // 进入循环
    enterLoop();
    
    // 分析循环体
    stmt.body->accept(*this);
    
    // 离开循环
    exitLoop();
}

void SemanticAnalyzer::visit(BreakStmt& stmt) {
    // 检查break语句是否在循环内
    if (!isInLoop()) {
        error("Break statement outside of loop", getLineNumber(stmt));
    }
}

void SemanticAnalyzer::visit(ContinueStmt& stmt) {
    // 检查continue语句是否在循环内
    if (!isInLoop()) {
        error("Continue statement outside of loop", getLineNumber(stmt));
    }
}

void SemanticAnalyzer::visit(ReturnStmt& stmt) {
    // 检查return语句的表达式类型
    if (stmt.value) {
        stmt.value->accept(*this);
        std::string returnType = getExprType(*stmt.value);
        
        if (currentFunctionReturnType == "void") {
            error("Cannot return a value from void function", getLineNumber(stmt));
        } 
        else if (returnType != currentFunctionReturnType) {
            error("Return type mismatch: expected '" + currentFunctionReturnType + 
                  "', got '" + returnType + "'", getLineNumber(stmt));
        }
    } 
    else if (currentFunctionReturnType != "void") {
        error("Function with return type '" + currentFunctionReturnType + 
              "' must return a value", getLineNumber(stmt));
    }
    
    // 标记当前函数有return语句
    hasReturn = true;
}

void SemanticAnalyzer::visit(FunctionDef& funcDef) {
    // 设置当前函数上下文
    currentFunction = funcDef.name;
    currentFunctionReturnType = funcDef.returnType;
    hasReturn = false;
    
    // 检查函数是否已声明
    FunctionInfo info;
    info.returnType = funcDef.returnType;
    info.defined = true;
    info.line = getLineNumber(funcDef.body);  
    
    for (const auto& param : funcDef.params) {
        info.paramTypes.push_back("int"); // ToyC中所有参数都是int
        info.paramNames.push_back(param.name);
    }
    
    if (!declareFunction(funcDef.name, info)) {
        error("Function '" + funcDef.name + "' is already defined", info.line);
    }
    
    // 如果是main函数，检查其合法性
    if (funcDef.name == "main" && !isValidMainFunction(funcDef)) {
        error("Invalid main function declaration", info.line);
    }
    
    // 进入新作用域
    enterScope();
    
    // 添加参数到符号表
    for (size_t i = 0; i < funcDef.params.size(); i++) {
        const auto& param = funcDef.params[i];
        Symbol symbol(Symbol::Kind::PARAMETER, "int", true, info.line, 0, i);
        if (!declareSymbol(param.name, symbol)) {
            error("Parameter '" + param.name + "' is already declared", info.line);
        }
    }
    
    // 分析函数体
    funcDef.body->accept(*this);
    
    // 检查是否所有路径都有return语句(对于非void函数)
    if (funcDef.returnType != "void" && !hasReturn) {
        // 简单检查: 函数没有return语句
        error("Function '" + funcDef.name + "' has no return statement", info.line);
    } 
    else if (funcDef.returnType != "void") {
        // 更复杂的检查: 检查所有路径是否都有return语句
        checkReturnPaths(*funcDef.body, true);
    }
    
    // 离开作用域
    exitScope();
    
    // 清除当前函数上下文
    currentFunction = "";
    currentFunctionReturnType = "";
}

void SemanticAnalyzer::visit(CompUnit& compUnit) {
    // 检查是否有main函数
    bool hasMain = false;
    
    // 首先收集所有函数声明
    for (const auto& func : compUnit.functions) {
        if (func->name == "main") {
            hasMain = true;
        }
    }
    
    // 然后分析每个函数定义
    for (const auto& func : compUnit.functions) {
        func->accept(*this);
    }
    
    // 确保程序有main函数
    if (!hasMain) {
        error("Program must have a main function");
    }
}