// CodeGenerator.h
#pragma once
#include "parser/astVisitor.h"
#include "ir/ir.h"
#include <vector>
#include <string>
#include <map>
#include <fstream>

class CodeGenerator {
private:
    std::ofstream output;
    std::map<std::string, int> localVars;
    int labelCount = 0;
    int stackSize = 0;
    
    // 当前函数的信息
    std::string currentFunction;
    bool isInLoop = false;
    
    // IR指令列表
    const std::vector<std::shared_ptr<IRInstr>>& instructions;

public:
    CodeGenerator(const std::string& outputFile, const std::vector<std::shared_ptr<IRInstr>>& instructions);
    ~CodeGenerator();
    
    // 生成代码
    void generate();

private:
    std::string genLabel();
    void emitComment(const std::string& comment);
    void emitInstruction(const std::string& instr);
    
    // 处理IR指令
    void processInstruction(const std::shared_ptr<IRInstr>& instr);
    
    // 处理各种IR指令类型
    void processBinaryOp(const std::shared_ptr<BinaryOpInstr>& instr);
    void processUnaryOp(const std::shared_ptr<UnaryOpInstr>& instr);
    void processAssign(const std::shared_ptr<AssignInstr>& instr);
    void processGoto(const std::shared_ptr<GotoInstr>& instr);
    void processIfGoto(const std::shared_ptr<IfGotoInstr>& instr);
    void processParam(const std::shared_ptr<ParamInstr>& instr);
    void processCall(const std::shared_ptr<CallInstr>& instr);
    void processReturn(const std::shared_ptr<ReturnInstr>& instr);
    void processLabel(const std::shared_ptr<LabelInstr>& instr);
    void processFunctionBegin(const std::shared_ptr<FunctionBeginInstr>& instr);
    void processFunctionEnd(const std::shared_ptr<FunctionEndInstr>& instr);
    
    // 加载操作数到寄存器
    void loadOperand(const std::shared_ptr<Operand>& op, const std::string& reg);
    
    // 存储寄存器到操作数
    void storeRegister(const std::string& reg, const std::shared_ptr<Operand>& op);
    
    // 获取操作数的存储位置
    int getOperandOffset(const std::shared_ptr<Operand>& op);
};