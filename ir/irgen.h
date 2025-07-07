#pragma once
#include "ir.h"
#include "parser/ast.h"
#include "parser/astVisitor.h"
#include "semantic/semantic.h"
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <stack>
#include <functional>

// IR生成异常类
class IRGenError : public std::runtime_error {
public:
    IRGenError(const std::string& message) : std::runtime_error(message) {}
};

// IR生成器配置
struct IRGenConfig {
    bool enableOptimizations = false;  // 是否启用优化
    bool generateDebugInfo = false;    // 是否生成调试信息
    bool inlineSmallFunctions = false; // 是否内联小函数
};

// IR生成器
class IRGenerator : public ASTVisitor {
private:
    std::vector<std::shared_ptr<IRInstr>> instructions;
    int tempCount = 0;
    int labelCount = 0;
    std::map<std::string, std::shared_ptr<Operand>> variables;
    std::string currentFunction;
    std::string currentFunctionReturnType; // 当前函数的返回类型，用于检查 return 语句
    
    // 操作数栈，用于表达式计算
    std::vector<std::shared_ptr<Operand>> operandStack;

    // 用于break和continue语句的标签栈
    std::vector<std::string> breakLabels;
    std::vector<std::string> continueLabels;

    // 生成器配置
    IRGenConfig config;
    
    // 变量作用域管理
    std::vector<std::map<std::string, std::shared_ptr<Operand>>> scopeStack;

public:
    IRGenerator(const IRGenConfig& config = IRGenConfig()) : config(config) {
        // 初始化作用域栈
        enterScope();
    }
    
    // 获取生成的IR指令序列
    const std::vector<std::shared_ptr<IRInstr>>& getInstructions() const { 
        return instructions; 
    }
    
    // 生成IR
    void generate(std::shared_ptr<CompUnit> ast);
    
    // 导出IR到文件
    void dumpIR(const std::string& filename) const;
    
    // 优化IR
    void optimize();

    // 辅助方法
    std::shared_ptr<Operand> createTemp();
    std::shared_ptr<Operand> createLabel();
    void addInstruction(std::shared_ptr<IRInstr> instr);
    
    // 返回栈顶操作数
    std::shared_ptr<Operand> getTopOperand();
    
    // 实现ASTVisitor接口
    void visit(NumberExpr& expr) override;
    void visit(VariableExpr& expr) override;
    void visit(BinaryExpr& expr) override;
    void visit(UnaryExpr& expr) override;
    void visit(CallExpr& expr) override;
    
    void visit(ExprStmt& stmt) override;
    void visit(VarDeclStmt& stmt) override;
    void visit(AssignStmt& stmt) override;
    void visit(BlockStmt& stmt) override;
    void visit(IfStmt& stmt) override;
    void visit(WhileStmt& stmt) override;
    void visit(BreakStmt& stmt) override;
    void visit(ContinueStmt& stmt) override;
    void visit(ReturnStmt& stmt) override;
    
    void visit(FunctionDef& funcDef) override;
    void visit(CompUnit& compUnit) override;
    
private:
    // 获取或创建变量操作数
    std::shared_ptr<Operand> getVariable(const std::string& name);
    
    // 作用域管理
    void enterScope();
    void exitScope();
    
    // 在当前作用域中查找变量
    std::shared_ptr<Operand> findVariableInCurrentScope(const std::string& name);
    
    // 在所有作用域中查找变量
    std::shared_ptr<Operand> findVariable(const std::string& name);
    
    // 在当前作用域中定义变量
    void defineVariable(const std::string& name, std::shared_ptr<Operand> var);
    
    // 优化相关方法
    void constantFolding();        // 常量折叠
    void constantPropagation();    // 常量传播
    void deadCodeElimination();    // 死代码删除
    void controlFlowOptimization();// 控制流优化
    
    // 短路求值支持
    std::shared_ptr<Operand> generateShortCircuitAnd(BinaryExpr& expr);
    std::shared_ptr<Operand> generateShortCircuitOr(BinaryExpr& expr);
    
    // 控制流分析
    struct BasicBlock {
        std::string label;
        std::vector<int> instructionIndices;  // 指令索引
        std::vector<std::string> successors;  // 后继基本块
        std::vector<std::string> predecessors;// 前驱基本块
    };
    
    // 构建控制流图
    std::map<std::string, BasicBlock> buildControlFlowGraph();
    
    // 检查指令是否是控制流指令
    bool isControlFlowInstruction(const std::shared_ptr<IRInstr>& instr) const;
    
    // 获取控制流指令的目标标签
    std::vector<std::string> getControlFlowTargets(const std::shared_ptr<IRInstr>& instr) const;

    // 辅助函数，递归判断一个语句是否所有路径都 return
    bool allPathsReturn(const std::shared_ptr<Stmt>& stmt);
};

// IR优化器接口
class IROptimizer {
public:
    virtual ~IROptimizer() = default;
    virtual void optimize(std::vector<std::shared_ptr<IRInstr>>& instructions) = 0;
};

// 常量折叠优化器
class ConstantFoldingOptimizer : public IROptimizer {
public:
    void optimize(std::vector<std::shared_ptr<IRInstr>>& instructions) override;
private:
    bool evaluateConstantExpression(OpCode op, int left, int right, int& result);
};

// 死代码消除优化器
class DeadCodeOptimizer : public IROptimizer {
public:
    void optimize(std::vector<std::shared_ptr<IRInstr>>& instructions) override;
private:
    std::vector<bool> findLiveInstructions(const std::vector<std::shared_ptr<IRInstr>>& instructions);
    bool isInstructionLive(const std::shared_ptr<IRInstr>& instr, 
                          const std::map<std::string, bool>& liveVars);
};

// IR到RISC-V汇编代码生成器接口
class IRToRISCVGenerator {
public:
    virtual ~IRToRISCVGenerator() = default;
    
    // 将IR转换为RISC-V汇编代码并写入文件
    virtual void generate(const std::vector<std::shared_ptr<IRInstr>>& instructions, 
                         const std::string& outputFile) = 0;
                         
    // 将单个IR指令转换为RISC-V汇编代码
    virtual std::vector<std::string> translateInstruction(const std::shared_ptr<IRInstr>& instr) = 0;
};
