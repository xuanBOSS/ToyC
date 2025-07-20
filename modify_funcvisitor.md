# analyzeVisitor::visit(FunctionDef) 方法更新说明

本文档记录了 `analyzeVisitor::visit(FunctionDef &funcDef)` 方法从旧版本到新版本的主要改动和优化点。

原先代码存在的问题：定义递归函数时，函数信息注册太晚导致函数体内对自身的调用解析出现错误。

解决方式：提前注册信息。

## 代码对比

### 旧版本代码

```cpp
void analyzeVisitor::visit(FunctionDef &funcDef)
{
    // 构建当前函数的函数信息结构体，以放进函数表
    FunctionInfo info;
    info.line = funcDef.line;
    info.column = funcDef.column;

    // 函数名称在全局的唯一性
    if (functionTable.find(funcDef.name) != functionTable.end())
    {
        helper.error("Duplicate function name", info.line, info.column);
    }

    // 续函数信息
    info.returnType = funcDef.returnType;
    for (const auto &param : funcDef.params)
    {
        info.paramTypes.push_back("int"); // ToyC中所有参数都是int
        info.paramNames.push_back(param.name);
    }

    // 如果是main函数，检查其合法性
    if (funcDef.name == "main" && !helper.isValidMainFunction(funcDef))
    {
        helper.error("Invalid main function declaration", info.line, info.column);
    }

    // 设置当前函数的（全局）上下文
    currentFunction = funcDef.name;
    currentFunctionReturnType = funcDef.returnType;
    hasReturn = false;

    // 进入新作用域(即创建新的符号表压入栈)
    helper.enterScope();

    // 添加函数名到当前作用域的符号表
    Symbol symbol(Symbol::Kind::FUNCTION, funcDef.returnType, info.line, info.column);
    symbol.used = (funcDef.name == "main"); // main函数默认为已使用
    helper.declareSymbol(funcDef.name, symbol);

    // 添加参数到当前作用域的符号表
    for (size_t i = 0; i < funcDef.params.size(); i++)
    {
        const auto &param = funcDef.params[i];
        Symbol symbol(Symbol::Kind::PARAMETER, "int", info.line, info.column, i);
        symbol.used = false; // 初始设置为未使用
        if (!helper.declareSymbol(param.name, symbol))
        {
            helper.error("Parameter '" + param.name + "' already declared", info.line, info.column);
        }
    }

    // 分析函数体
    funcDef.body->accept(*this);

    // 函数体内无return关键字的情况
    if (funcDef.returnType != "void" && !hasReturn)
    {
        helper.error("Function '" + funcDef.name + "' has no return statement", info.line, info.column);
    }

    // 检查未使用的参数和局部变量
    checkUnusedVariables();

    // 当前函数加入函数表
    functionTable[currentFunction] = info;

    // 离开作用域
    helper.exitScope();

    // 清除当前函数上下文
    currentFunction = "";
    currentFunctionReturnType = "";
    hasReturn = false;
}
```

## 新版本代码

```cpp
void analyzeVisitor::visit(FunctionDef &funcDef)
{
    int line = funcDef.line;
    int column = funcDef.column;
    std::string name = funcDef.name;

    // 函数名不能重复
    if (functionTable.count(name)) {
        helper.error("Duplicate function name", line, column);
    }

    // 构建函数信息（完整）
    FunctionInfo info;
    info.returnType = funcDef.returnType;
    info.line = line;
    info.column = column;
    for (const auto &param : funcDef.params) {
        info.paramTypes.push_back("int");
        info.paramNames.push_back(param.name);
    }

    // 提前注册函数信息（支持递归）
    functionTable[name] = info;

    // 检查 main 函数合法性
    if (name == "main" && !helper.isValidMainFunction(funcDef)) {
        helper.error("Invalid main function declaration", line, column);
    }

    // 设置当前上下文
    currentFunction = name;
    currentFunctionReturnType = funcDef.returnType;
    hasReturn = false;

    // 进入新作用域
    helper.enterScope();

    // 注册函数符号
    Symbol funcSymbol(Symbol::Kind::FUNCTION, funcDef.returnType, line, column);
    funcSymbol.used = (name == "main");
    helper.declareSymbol(name, funcSymbol);

    // 注册参数符号
    for (size_t i = 0; i < funcDef.params.size(); i++) {
        const auto &param = funcDef.params[i];
        Symbol paramSymbol(Symbol::Kind::PARAMETER, "int", line, column, i);
        paramSymbol.used = false;
        if (!helper.declareSymbol(param.name, paramSymbol)) {
            helper.error("Parameter '" + param.name + "' already declared", line, column);
        }
    }

    // 访问函数体
    funcDef.body->accept(*this);

    // 检查 return 语句是否遗漏
    if (funcDef.returnType != "void" && !hasReturn) {
        helper.error("Function '" + name + "' has no return statement", line, column);
    }

    // 检查未使用的局部变量和参数
    checkUnusedVariables();

    // 离开作用域
    helper.exitScope();

    // 清除上下文
    currentFunction = "";
    currentFunctionReturnType = "";
    hasReturn = false;
}
```
## 主要修改点

| 修改内容 | 说明 |
|---------|------|
| **提前注册函数信息** | 新版本中将函数信息 info 在函数体分析前就加入了 functionTable，支持递归调用时函数信息的查找 |
| **更简洁的变量定义** | 将 line, column, name 提取为局部变量，代码更清晰 |
| **避免重复赋值** | 取消了旧版本中多次对 currentFunction 和 currentFunctionReturnType 的赋值 |
| **删除冗余操作** | 移除了旧版本中函数信息重复写入 functionTable 的操作，防止覆盖 |
| **错误检测顺序调整** | 先检测函数名是否重复，再注册信息，保证逻辑合理 |

## 优化效果

- **支持递归函数的正确分析**：防止因函数信息未提前注册而导致的查找失败
- **代码结构更清晰**：易读性提高  
- **减少冗余赋值**：提升性能及维护便利性

## 使用建议

1. 在处理函数定义的分析时，务必确保函数签名（参数和返回类型等）提前注册，方便递归函数的正确识别
2. 错误检测尽量提前，避免无意义的后续操作
3. 保持代码简洁，避免重复状态更新