# Lexer 模块分析



## 设计逻辑



词法分析器(Lexer)是编译器的第一阶段，负责将源代码转换为一系列标记(Token)。该模块采用了以下设计逻辑：

1. **标记定义**：在`Token.h`中定义了`TokenType`枚举类和`Token`结构体，枚举类包含了ToyC语言中所有类型的标记，包括关键字、标识符、运算符、分隔符等。
2. **状态管理**：Lexer类维护了当前扫描位置、行号和列号，用于跟踪源代码中的位置信息。
3. **查表法**：使用两个哈希表`keywords`和`operators`来快速识别关键字和运算符，提高了效率。
4. **递进式扫描**：通过不断前进并检查字符来识别不同类型的标记，如标识符、数字、运算符等。
5. **预读机制**：使用`peek()`函数来预读字符，不改变当前位置，有助于识别多字符标记（如`==`、`!=`等）。

## 功能实现





### 核心功能



1. 构造函数

   ：

   - 初始化内部状态（源码、位置、行列信息）
   - 建立关键字和运算符的映射表

2. 标记化

   ：

   - `tokenize()`方法：将整个源代码字符串转换为标记序列
   - `nextToken()`方法：获取下一个标记
   - `peekToken()`方法：预览下一个标记但不消耗它

3. 扫描特定类型

   ：

   - `scanIdentifier()`：扫描标识符和关键字
   - `scanNumber()`：扫描数字字面量
   - `readOperatorOrPunctuator()`：读取运算符或标点符号

4. 辅助功能

   ：

   - `skipWhitespace()`：跳过空白字符
   - `skipComment()`：跳过单行和多行注释
   - `advance()`：前进一个字符并更新位置信息
   - `isAtEnd()`：检查是否到达源码末尾

### 特殊处理



1. **注释处理**：支持跳过单行注释(`//`)和多行注释(`/* */`)。
2. **位置跟踪**：精确跟踪每个标记的行号和列号，方便后续错误报告。
3. **多字符运算符**：能够正确识别双字符运算符，如`==`、`!=`、`<=`、`>=`、`&&`、`||`等。

## 完整性评估



涵盖了ToyC语言定义的所有标记类型，并包含了必要的错误处理机制。主要优点包括：

1. **接口丰富**：除了基本的标记化功能外，还提供了灵活的查看和获取下一个标记的方法。
2. **位置追踪**：精确记录每个标记的位置信息，有助于生成准确的错误信息。
3. **注释处理**：正确处理了单行和多行注释。
4. **可重用性**：提供了多种构造函数和方法重载，增强了模块的可重用性。

# Parser 模块分析



## 设计逻辑



Parser（解析器）模块是编译器的第二个重要阶段，它将词法分析器生成的标记序列转换为抽象语法树（AST）。采用了以下设计逻辑：

1. 抽象语法树设计

   ：

   - 定义了完整的AST节点层次结构，以反映ToyC语言的语法结构
   - 使用了访问者模式（Visitor Pattern）设计，便于后续各种分析和代码生成阶段访问AST节点

2. 递归下降解析

   ：

   - 实现了自顶向下的递归下降解析方法，每个语法规则对应一个解析函数
   - 函数调用层次反映了语法的嵌套结构和优先级关系

3. 错误处理与恢复

   ：

   - 采用异常机制处理语法错误
   - 实现了错误恢复机制，允许在出错后继续解析，而不是立即终止

4. 位置跟踪

   ：

   - 每个AST节点都记录其在源代码中的位置信息（行号和列号）
   - 有助于在后续阶段生成准确的错误信息

## 功能实现



### AST节点设计



1. 基础节点类

   ：

   - `ASTNode`：所有节点的基类，包含位置信息和访问者接口
   - `Expr`：表达式节点的基类
   - `Stmt`：语句节点的基类

2. 表达式节点

   ：

   - `NumberExpr`：数字字面量
   - `VariableExpr`：变量引用
   - `BinaryExpr`：二元操作表达式
   - `UnaryExpr`：一元操作表达式
   - `CallExpr`：函数调用表达式

3. 语句节点

   ：

   - `ExprStmt`：表达式语句
   - `VarDeclStmt`：变量声明语句
   - `AssignStmt`：赋值语句
   - `BlockStmt`：语句块
   - `IfStmt`：条件语句
   - `WhileStmt`：循环语句
   - `BreakStmt`：中断语句
   - `ContinueStmt`：继续语句
   - `ReturnStmt`：返回语句

4. 顶层节点

   ：

   - `FunctionDef`：函数定义
   - `CompUnit`：编译单元（整个程序）

### 解析器实现



1. Token处理功能

   ：

   - `advance()`：移动到下一个标记
   - `peek()`：预览后续标记
   - `match()`：尝试匹配指定类型的标记
   - `consume()`：消费期望的标记类型，否则报错

2. 错误处理

   ：

   - `error()`：记录语法错误
   - `synchronize()`：错误恢复机制，寻找下一个合适的继续点
   - 使用异常机制传播错误信息

3. 递归下降解析函数

   ：

   - 为每个语法规则实现相应的解析函数
   - 从顶层的`compUnit()`开始，层层调用子规则的解析函数
   - 表达式解析函数的调用顺序反映了运算符的优先级

4. 特殊处理

   ：

   - 处理空语句
   - 区分变量声明和赋值语句
   - 处理可选的else分支和返回值

## 完整性评估



有效地解析ToyC语言的所有语法结构。主要优点包括：

1. **严格遵循语法规则**：解析器的实现严格按照ToyC语言的语法规则进行设计，确保了解析的准确性。

2. 健壮的错误处理

   ：

   - 实现了详细的错误报告机制，提供了行列号信息
   - 错误恢复机制允许检测多个错误，而不是在第一个错误处停止

3. 完整的AST表示

   ：

   - AST节点设计全面覆盖了语言的所有语法结构
   - 节点之间的关系清晰地反映了源代码的结构

4. **访问者模式设计**：通过`ASTVisitor`接口，为后续的语义分析和代码生成阶段提供了便捷的访问方式。

# Semantic 模块分析



## 设计逻辑



语义分析（Semantic Analysis）是编译器的第三个主要阶段，它在语法分析生成AST之后进行，主要负责检查程序的语义正确性。语义分析模块采用了以下设计逻辑：

1. **访问者模式**：继续使用访问者模式遍历AST，进行各种语义检查。

2. 职责分离

   ：

   - `analyzeVisitor`：主要进行语义规则检查
   - `typeVisitor`：专门负责类型检查
   - `analyzeHelper`：提供辅助功能，如作用域管理、符号查找等

3. 符号表管理

   ：

   - 使用栈式结构（vector）实现嵌套作用域
   - 每进入一个新的块级作用域，就创建一个新的符号表
   - 查找标识符时从内层作用域向外层查找

4. 函数表

   ：

   - 单独维护函数信息表，存储函数的返回类型、参数列表等信息
   - 用于函数调用检查和返回值类型验证

5. 多种语义检查

   ：

   - 类型检查
   - 变量声明和使用检查
   - 函数调用参数匹配检查
   - 控制流检查（break/continue在循环中使用）
   - 死代码检测
   - 未使用变量检查

## 功能实现



### 1. `analyzeHelper`类



这是一个辅助类，提供各种工具函数：

- 作用域管理

  ：

  - `enterScope()`：进入新作用域
  - `exitScope()`：退出作用域

- 符号管理

  ：

  - `declareSymbol()`：在当前作用域声明符号
  - `findSymbol()`：从当前作用域向上查找符号

- 循环控制检查

  ：

  - `enterLoop()`/`exitLoop()`：追踪循环嵌套层次
  - `isInLoop()`：检查是否在循环内部

- 常量表达式求值

  ：

  - `evaluateConstant()`：计算常量表达式的值（用于检测除零错误、恒为真/假的条件等）

- 错误和警告处理

  ：

  - `error()`：记录错误信息
  - `warning()`：记录警告信息

- 死代码和未使用变量检测

  ：

  - `detectDeadCode()`
  - `checkUnusedVariables()`

### 2. `analyzeVisitor`类



主要的语义分析访问者，负责遍历AST并进行语义检查：

- 表达式检查

  ：

  - 变量引用：检查变量是否声明
  - 二元表达式：检查操作数类型、除零错误等
  - 函数调用：检查函数是否存在、参数数量和类型是否匹配

- 语句检查

  ：

  - 变量声明：检查重复声明、初始化表达式类型
  - 赋值语句：检查变量是否存在、类型兼容性
  - 控制语句：检查条件表达式类型、break/continue的合法性
  - 返回语句：检查返回值类型是否与函数声明匹配

- 函数定义检查

  ：

  - 函数重复定义检查
  - main函数的特殊检查
  - 返回语句的存在性检查（非void函数必须有返回值）

- 额外静态分析

  ：

  - 未使用变量/函数的检测
  - 恒为真/假的条件表达式检测

### 3. `typeVisitor`类



专门负责类型相关的检查：

- 计算表达式的类型
- 检查表达式类型与期望类型的兼容性
- 函数调用参数类型检查

### 4. `SemanticAnalyzer`类



对外提供统一的语义分析接口：

- `analyze()`：分析整个AST
- `getErrors()`/`getWarnings()`：获取分析过程中收集的错误和警告
- `checkUnusedVariables()`/`detectDeadCode()`：执行额外的静态分析

## 完整性评估



涵盖了ToyC语言所需的所有语义检查。主要优点包括：

1. 清晰的模块化设计

   ：

   - 职责分离明确，每个类都有特定的职责
   - 使用辅助类分离核心逻辑和工具函数

2. 全面的语义检查

   ：

   - 类型检查
   - 变量和函数声明/引用检查
   - 控制流检查
   - 函数返回值检查
   - 运行时错误检测（如除零）

3. 增强的静态分析能力

   ：

   - 未使用变量/函数检测
   - 死代码检测
   - 恒为真/假条件表达式检测

4. 良好的错误处理机制

   ：

   - 收集错误和警告而不是立即终止
   - 提供详细的错误位置信息

5. 作用域管理

   ：

   - 正确实现了嵌套作用域
   - 变量查找遵循从内到外的原则

# IR生成器 模块分析



## 1. 总体设计



IR生成器是编译器前端和后端之间的桥梁，它将抽象语法树(AST)转换为一种更接近目标代码的中间表示形式。IR生成器采用了以下设计：

### 1.1 核心组件



- **IR指令体系**：定义了各种IR指令类型（二元运算、一元运算、赋值、跳转等）
- **操作数系统**：支持变量、临时变量、常量和标签四种操作数类型
- **IR生成器**：继承自ASTVisitor，通过访问者模式遍历AST并生成IR
- **IR优化器**：包括常量折叠、常量传播、死代码消除和控制流优化
- **作用域管理**：支持嵌套作用域和变量查找

### 1.2 设计思路



- **访问者模式**：利用ASTVisitor接口访问AST的每个节点
- **操作数栈**：使用栈来存储表达式求值的中间结果
- **三地址码**：大多数IR指令采用三地址码形式（result = left op right）
- **控制流图**：用于分析和优化程序的执行流程
- **短路求值**：对逻辑运算符(&&, ||)实现短路求值

## 2. IR指令设计



您的IR指令体系设计得非常完善，涵盖了以下几种类型：

### 2.1 算术和逻辑运算指令



- **二元运算**：加、减、乘、除、取模、比较、逻辑运算等
- **一元运算**：取负、逻辑非

### 2.2 控制流指令



- **无条件跳转**：`goto label`
- **条件跳转**：`if condition goto label`
- **函数调用**：`result = call func, paramCount`
- **返回**：`return value` 或 `return`

### 2.3 函数和作用域指令



- **函数开始和结束**：标记函数定义的边界
- **标签定义**：作为跳转目标的位置标记

## 3. IR生成过程详解



### 3.1 表达式处理



表达式处理是IR生成的核心部分，采用了后序遍历方式：

1. **递归处理子表达式**：先访问操作数节点
2. **生成中间代码**：为表达式操作创建IR指令
3. **结果存储**：将结果存入临时变量，并压入操作数栈

特别之处在于对短路求值的支持：

```
// 短路逻辑与(&&)的实现
std::shared_ptr<Operand> IRGenerator::generateShortCircuitAnd(BinaryExpr& expr) {
    // 评估左操作数
    expr.left->accept(*this);
    std::shared_ptr<Operand> left = getTopOperand();
    
    // 创建临时变量和标签
    std::shared_ptr<Operand> result = createTemp();
    std::shared_ptr<Operand> shortCircuitLabel = createLabel();
    std::shared_ptr<Operand> endLabel = createLabel();
    
    // 如果左操作数为假，短路
    addInstruction(std::make_shared<IfGotoInstr>(left, shortCircuitLabel));
    
    // 左操作数为真，评估右操作数
    expr.right->accept(*this);
    std::shared_ptr<Operand> right = getTopOperand();
    
    // 结果为右操作数
    addInstruction(std::make_shared<AssignInstr>(result, right));
    addInstruction(std::make_shared<GotoInstr>(endLabel));
    
    // 短路：结果为假
    addInstruction(std::make_shared<LabelInstr>(shortCircuitLabel->name));
    addInstruction(std::make_shared<AssignInstr>(result, std::make_shared<Operand>(0)));
    
    // 结束
    addInstruction(std::make_shared<LabelInstr>(endLabel->name));
    
    return result;
}
```



### 3.2 语句处理



语句处理相对复杂，特别是控制流语句：

1. **条件语句(if-else)**：使用条件跳转和标签实现分支
2. **循环语句(while)**：使用标签和跳转指令创建循环结构
3. **break/continue**：跳转到当前循环的结束/条件检查点
4. **变量声明**：创建变量并可选地进行初始化
5. **返回语句**：生成返回指令，可能带返回值

### 3.3 函数处理



函数处理涉及以下步骤：

1. **函数开始标记**：添加函数开始指令
2. **参数处理**：在作用域中创建参数变量
3. **函数体生成**：为函数体生成IR
4. **确保返回**：为void函数添加默认返回
5. **函数结束标记**：添加函数结束指令

## 4. 作用域管理



作用域管理实现了词法作用域规则：

```
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
```



## 5. 优化实现



您实现了多种优化技术：

### 5.1 常量折叠



常量折叠将编译时可知的常量表达式计算出结果：

```
void IRGenerator::constantFolding() {
    for (size_t i = 0; i < instructions.size(); ++i) {
        auto instr = instructions[i];
        
        if (auto binOp = std::dynamic_pointer_cast<BinaryOpInstr>(instr)) {
            if (binOp->left->type == OperandType::CONSTANT && 
                binOp->right->type == OperandType::CONSTANT) {
                
                int result = 0;
                bool canFold = true;
                
                // 根据操作类型计算结果
                switch (binOp->opcode) {
                    case OpCode::ADD: result = binOp->left->value + binOp->right->value; break;
                    // 其他操作...
                }
                
                if (canFold) {
                    // 用赋值指令替换原二元操作指令
                    auto constResult = std::make_shared<Operand>(result);
                    auto assignInstr = std::make_shared<AssignInstr>(binOp->result, constResult);
                    instructions[i] = assignInstr;
                }
            }
        }
        // 一元操作处理...
    }
}
```



### 5.2 常量传播



常量传播跟踪变量的常量值并传播它们：

```
void IRGenerator::constantPropagation() {
    std::unordered_map<std::string, std::shared_ptr<Operand>> constants;
    bool changed = true;
    
    while (changed) {
        changed = false;
        
        // 遍历指令，找出并传播常量赋值
        for (size_t i = 0; i < instructions.size(); ++i) {
            auto instr = instructions[i];
            
            if (auto assignInstr = std::dynamic_pointer_cast<AssignInstr>(instr)) {
                if (assignInstr->source->type == OperandType::CONSTANT) {
                    // 记录常量赋值
                    constants[assignInstr->target->name] = assignInstr->source;
                } 
                // 更多处理...
            }
            // 其他指令类型处理...
        }
    }
}
```



### 5.3 死代码消除



死代码消除删除不会影响程序结果的指令：

```
void IRGenerator::deadCodeElimination() {
    // 找出所有使用的变量
    std::unordered_set<std::string> usedVars;
    
    // 标记活跃指令
    std::vector<bool> isLive(instructions.size(), false);
    
    // 只保留活跃指令
    std::vector<std::shared_ptr<IRInstr>> newInstructions;
    for (size_t i = 0; i < instructions.size(); ++i) {
        if (isLive[i]) {
            newInstructions.push_back(instructions[i]);
        }
    }
    
    instructions = newInstructions;
}
```



### 5.4 控制流优化



控制流优化简化程序的控制流结构：

```
void IRGenerator::controlFlowOptimization() {
    bool changed = true;
    
    while (changed) {
        changed = false;
        
        // 构建控制流图
        auto cfg = buildControlFlowGraph();
        
        // 优化跳转链和常量条件
        // ...
        
        if (changed) {
            deadCodeElimination();
        }
    }
}
```



## 6. 控制流图构建



控制流图(CFG)是许多优化的基础：

```
std::map<std::string, IRGenerator::BasicBlock> IRGenerator::buildControlFlowGraph() {
    std::map<std::string, BasicBlock> blocks;
    
    // 找出基本块的起始点（leaders）
    std::vector<int> leaders;
    
    // 创建基本块
    for (size_t i = 0; i < leaders.size(); ++i) {
        // ...创建基本块逻辑
    }
    
    // 添加基本块之间的边
    for (auto& [label, block] : blocks) {
        // ...添加边的逻辑
    }
    
    // 计算前驱
    for (auto& [label, block] : blocks) {
        // ...计算前驱逻辑
    }
    
    return blocks;
}
```



## 7. IR分析器功能



IR分析器提供了分析IR的工具：

1. **定义查找**：找出定义变量的指令
2. **使用查找**：找出使用变量的指令
3. **活跃变量检查**：检查变量是否在某点活跃
4. **变量访问分析**：获取指令定义和使用的变量

## 8. 总结与建议



可能的改进：

1. **类型系统支持**：目前IR似乎只支持整数类型，可以扩展以支持更多数据类型
2. **优化通道框架**：创建通用的优化通道框架，使添加新优化更容易
3. **数据流分析**：增加更多数据流分析算法，如可达性定义分析、活跃变量分析等
4. **指令特化**：为常见模式添加特殊指令，如自增/自减
5. **内存模型**：增加对数组和指针的支持

# RISC-V代码生成器 模块分析



## 1. 总体设计架构



代码生成器将IR(中间表示)转换为RISC-V汇编代码，采用了模块化设计，支持多种寄存器分配策略和优化技术。整体架构包含以下几个主要组件：

1. **代码生成器主体(CodeGenerator)**：负责将IR指令转换为RISC-V汇编指令
2. **寄存器分配策略**：包括朴素分配、线性扫描和图着色三种算法
3. **优化组件**：包括栈布局优化和窥孔优化
4. **代码生成辅助功能**：包括指令输出、寄存器管理和变量访问

这种设计使代码生成器具有很好的可扩展性和灵活性，能够根据配置选择不同的优化策略。

## 2. 寄存器分配策略



代码生成器实现了三种寄存器分配策略：

### 2.1 朴素寄存器分配 (NaiveRegisterAllocator)



简单地将变量按顺序分配给可用寄存器，直到寄存器用完。这种方法实现简单，但效率较低，无法处理复杂程序的变量生命周期问题。主要逻辑是：

1. 收集所有变量
2. 按顺序分配寄存器，优先使用被调用者保存的寄存器
3. 寄存器用完后，剩余变量使用栈存储

### 2.2 线性扫描寄存器分配 (LinearScanRegisterAllocator)



这是一种更高效的分配算法，考虑了变量的生命周期：

1. 计算每个变量的生命周期区间（从定义到最后使用的指令范围）
2. 按开始位置排序区间
3. 线性扫描这些区间，当一个变量的生命周期结束时，释放其寄存器给其他变量使用
4. 当没有可用寄存器时，选择溢出策略（通常是选择最晚结束的区间）

代码中的实现考虑了变量的生命周期重叠问题，是一个经典的线性扫描算法实现。

### 2.3 图着色寄存器分配 (GraphColoringRegisterAllocator)



这是最复杂但也最优的分配策略，基于图论：

1. 构建变量冲突图（如果两个变量的生命周期重叠，则它们之间有边）
2. 使用图简化算法，反复移除度数小于可用寄存器数量的节点
3. 按照简化的逆序为变量分配寄存器（颜色），确保相邻节点使用不同的寄存器

这种方法能够处理复杂的变量依赖关系，是编译器常用的寄存器分配算法。

## 3. 代码生成流程



代码生成的主要流程包括：

1. **初始化**：设置输出文件、初始化寄存器信息
2. **寄存器分配**：根据配置选择寄存器分配策略
3. **生成代码**：遍历IR指令，为每条指令生成相应的RISC-V汇编代码
4. **优化**：应用窥孔优化和栈布局优化
5. **输出**：将生成的汇编代码写入文件

### 3.1 IR指令处理



代码生成器为每种IR指令类型实现了专门的处理方法：

```
void CodeGenerator::processInstruction(const std::shared_ptr<IRInstr>& instr) {
    switch (instr->opcode) {
        case OpCode::ADD:
        case OpCode::SUB:
            // 处理二元操作
            processBinaryOp(std::dynamic_pointer_cast<BinaryOpInstr>(instr));
            break;
        case OpCode::NEG:
        case OpCode::NOT:
            // 处理一元操作
            processUnaryOp(std::dynamic_pointer_cast<UnaryOpInstr>(instr));
            break;
        // ... 其他指令类型
    }
}
```



### 3.2 二元操作处理示例



以二元操作为例，处理流程为：

1. 为源操作数和结果分配临时寄存器
2. 加载源操作数到寄存器
3. 根据操作类型生成相应的RISC-V指令
4. 将结果存储到目标位置
5. 释放临时寄存器

```
void CodeGenerator::processBinaryOp(const std::shared_ptr<BinaryOpInstr>& instr) {
    // 获取临时寄存器
    std::string resultReg = allocTempReg();
    std::string leftReg = allocTempReg();
    std::string rightReg = allocTempReg();

    // 加载操作数
    loadOperand(instr->left, leftReg);
    loadOperand(instr->right, rightReg);
    
    // 根据操作类型生成指令
    switch (instr->opcode) {
        case OpCode::ADD:
            emitInstruction("add " + resultReg + ", " + leftReg + ", " + rightReg);
            break;
        // ... 其他操作类型
    }
    
    // 存储结果
    storeRegister(resultReg, instr->result);
    
    // 释放临时寄存器
    freeTempReg(rightReg);
    freeTempReg(leftReg);
    freeTempReg(resultReg);
}
```



## 4. 函数调用约定实现



代码生成器遵循RISC-V的函数调用约定：

1. **参数传递**：前8个参数通过a0-a7寄存器传递，多余的参数通过栈传递

2. **返回值**：通过a0寄存器返回

3. 寄存器保存

   ：

   - 调用者保存的寄存器：ra, t0-t6, a0-a7
   - 被调用者保存的寄存器：s0-s11, sp, fp

### 4.1 函数序言与后记



函数序言负责设置栈帧：

```
void CodeGenerator::emitPrologue(const std::string& funcName) {
    // 计算帧大小
    int frameSize = 8 + stackSize;
    
    // 保存返回地址和帧指针
    emitInstruction("addi sp, sp, -" + std::to_string(frameSize));
    emitInstruction("sw ra, " + std::to_string(frameSize - 4) + "(sp)");
    emitInstruction("sw fp, " + std::to_string(frameSize - 8) + "(sp)");
    
    // 设置新的帧指针
    emitInstruction("addi fp, sp, " + std::to_string(frameSize));
    
    // 保存被调用者保存的寄存器
    saveCalleeSavedRegs();
}
```



函数后记负责恢复栈帧并返回：

```
void CodeGenerator::emitEpilogue(const std::string& funcName) {
    // 恢复被调用者保存的寄存器
    restoreCalleeSavedRegs();
    
    // 恢复帧指针和返回地址
    emitInstruction("lw fp, " + std::to_string(frameSize - 8) + "(sp)");
    emitInstruction("lw ra, " + std::to_string(frameSize - 4) + "(sp)");
    
    // 调整栈指针
    emitInstruction("addi sp, sp, " + std::to_string(frameSize));
    
    // 返回
    emitInstruction("ret");
}
```



## 5. 优化技术



代码生成器实现了两种主要优化技术：

### 5.1 栈布局优化



这种优化通过分析变量的生命周期，重用栈空间以减少函数栈帧大小：

1. 分析变量的生命周期（从定义到最后使用）
2. 按生命周期结束时间排序变量
3. 使用贪心算法重新分配栈空间，允许不重叠的变量共享同一栈位置
4. 更新变量到栈偏移的映射

### 5.2 窥孔优化



窥孔优化查看相邻的几条指令，识别并替换可优化的模式：

1. **加载后立即存储相同位置**：消除冗余的内存操作
2. **加载立即数后立即比较**：合并为立即数比较指令
3. **移动到同一寄存器**：消除冗余的寄存器移动

窥孔优化的设计非常灵活，使用模式匹配加处理函数的方式，便于添加新的优化规则。

## 6. 变量访问与寄存器管理



### 6.1 变量访问



代码生成器通过两个主要方法处理变量访问：

1. loadOperand

   ：将操作数加载到寄存器

   - 对于常量，使用`li`指令直接加载
   - 对于变量，检查是否已分配寄存器，如果是则直接移动，否则从栈中加载

2. storeRegister

   ：将寄存器内容存储到变量

   - 如果变量已分配寄存器，直接移动
   - 否则存储到栈中

### 6.2 寄存器管理



代码生成器使用一组临时寄存器(t0-t6)进行指令内部的计算：

1. **allocTempReg**：分配临时寄存器（简单循环使用）
2. **freeTempReg**：释放临时寄存器（目前为空实现）
3. **saveCallerSavedRegs/restoreCallerSavedRegs**：在函数调用前后保存/恢复调用者保存的寄存器
4. **saveCalleeSavedRegs/restoreCalleeSavedRegs**：在函数开始/结束时保存/恢复被调用者保存的寄存器

## 7. 代码优势与可能改进点



### 7.1 代码优势



1. **模块化设计**：各组件职责明确，便于维护和扩展
2. **多种寄存器分配策略**：支持不同复杂度的寄存器分配算法
3. **优化支持**：实现了栈布局优化和窥孔优化等重要优化
4. **符合标准调用约定**：遵循RISC-V调用约定，保证兼容性
5. **灵活的配置选项**：通过配置可以启用/禁用各种优化

### 7.2 可能的改进点



1. 寄存器分配的改进

   ：

   - 实现更精细的溢出决策，考虑变量使用频率
   - 支持寄存器合并(coalescing)，减少不必要的移动指令

2. 临时寄存器管理

   ：

   - 当前临时寄存器分配较简单，可以实现更智能的分配机制
   - `freeTempReg`方法目前为空实现，可以改进为真正释放寄存器

3. 指令选择优化

   ：

   - 针对特定模式生成更优的指令序列
   - 利用RISC-V的特殊指令（如立即数操作）减少指令数量

4. 函数内联

   ：

   - 为小函数实现内联展开，减少函数调用开销

5. 循环优化

   ：

   - 添加循环优化，如循环不变量外提、循环展开等