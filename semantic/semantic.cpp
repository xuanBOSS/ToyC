
#include "semantic.h"
#include <iostream>

SemanticAnalyzer::SemanticAnalyzer() {
    // 初始全局作用域
    enterScope();
}

bool SemanticAnalyzer::analyze(std::shared_ptr<CompUnit> ast) {
    try {
        ast->accept(*this);
        return success;
    } catch (const SemanticError& e) {
        std::cerr << "Semantic error: " << e.what() << std::endl;
        return false;
    }
}

void SemanticAnalyzer::enterScope() {
    symbolTables.push_back(std::unordered_map<std::string, Symbol>());
}

void SemanticAnalyzer::exitScope() {
    if (!symbolTables.empty()) {
        symbolTables.pop_back();
    }
}

bool SemanticAnalyzer::declareSymbol(const std::string& name, Symbol symbol) {
    if (symbolTables.back().find(name) != symbolTables.back().end()) {
        return false; // 已在当前作用域声明
    }
    
    symbolTables.back().insert(std::make_pair(name, symbol));
    // 或者使用 emplace
    // symbolTables.back().emplace(name, symbol);
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

Symbol* SemanticAnalyzer::findSymbol(const std::string& name) {
    for (auto it = symbolTables.rbegin(); it != symbolTables.rend(); ++it) {
        auto symIt = it->find(name);
        if (symIt != it->end()) {
            return &symIt->second;
        }
    }
    return nullptr;
}

void SemanticAnalyzer::error(const std::string& message) {
    success = false;
    throw SemanticError(message);
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

// 语句分析方法
void SemanticAnalyzer::visit(VarDeclStmt& stmt) {
    if (isDeclared(stmt.name)) {
        error("Variable " + stmt.name + " is already declared in this scope");
    }
    
    if (stmt.initializer) {
        std::string initType = getExprType(*stmt.initializer);
        if (initType != "int") {
            error("Cannot initialize int variable with non-int expression");
        }
    }
    
    declareSymbol(stmt.name, Symbol(Symbol::Kind::VARIABLE, "int", stmt.initializer != nullptr));
}

void SemanticAnalyzer::visit(AssignStmt& stmt) {
    Symbol* symbol = findSymbol(stmt.name);
    if (!symbol) {
        error("Undefined variable: " + stmt.name);
    }
    
    if (symbol->kind != Symbol::Kind::VARIABLE && symbol->kind != Symbol::Kind::PARAMETER) {
        error("Cannot assign to " + stmt.name + " (not a variable)");
    }
    
    std::string valueType = getExprType(*stmt.value);
    if (valueType != symbol->type) {
        error("Type mismatch in assignment to " + stmt.name);
    }
    
    symbol->initialized = true;
}

void SemanticAnalyzer::visit(ReturnStmt& stmt) {
    if (stmt.value) {
        std::string returnType = getExprType(*stmt.value);
        if (currentFunctionReturnType == "void") {
            error("Cannot return a value from void function");
        } else if (returnType != currentFunctionReturnType) {
            error("Return type mismatch");
        }
    } else if (currentFunctionReturnType != "void") {
        error("Function with return type " + currentFunctionReturnType + " must return a value");
    }
}

void SemanticAnalyzer::visit(BreakStmt&) {
    if (!inLoop) {
        error("Break statement outside of loop");
    }
}

void SemanticAnalyzer::visit(ContinueStmt&) {
    if (!inLoop) {
        error("Continue statement outside of loop");
    }
}

void SemanticAnalyzer::visit(WhileStmt& stmt) {
    std::string condType = getExprType(*stmt.condition);
    if (condType != "int") {
        error("While condition must be of type int");
    }
    
    bool oldInLoop = inLoop;
    inLoop = true;
    stmt.body->accept(*this);
    inLoop = oldInLoop;
}

void SemanticAnalyzer::visit(IfStmt& stmt) {
    std::string condType = getExprType(*stmt.condition);
    if (condType != "int") {
        error("If condition must be of type int");
    }
    
    stmt.thenBranch->accept(*this);
    if (stmt.elseBranch) {
        stmt.elseBranch->accept(*this);
    }
}

void SemanticAnalyzer::visit(FunctionDef& funcDef) {
    // 登记函数
    FunctionInfo info;
    info.returnType = funcDef.returnType;
    
    for (const auto& param : funcDef.params) {
        info.paramTypes.push_back("int"); // ToyC中所有参数都是int
    }
    
    functionTable[funcDef.name] = info;
    
    // 进入新作用域
    enterScope();
    
    currentFunctionReturnType = funcDef.returnType;
    
    // 添加参数到符号表
    for (const auto& param : funcDef.params) {
        declareSymbol(param.name, Symbol(Symbol::Kind::PARAMETER, "int", true));
    }
    
    // 分析函数体
    funcDef.body->accept(*this);
    
    // 离开作用域
    exitScope();
}

void SemanticAnalyzer::visit(CompUnit& compUnit) {
    // 检查是否有main函数
    bool hasMain = false;
    
    for (const auto& func : compUnit.functions) {
        func->accept(*this);
        if (func->name == "main") {
            hasMain = true;
            if (func->returnType != "int") {
                error("Main function must have return type int");
            }
            if (!func->params.empty()) {
                error("Main function should have no parameters in ToyC");
            }
        }
    }
    
    if (!hasMain) {
        error("Program must have a main function");
    }
}
