# ToyC 编译器栈分配策略

## 概述

ToyC 编译器采用基于 RISC-V ABI 的分层栈分配策略，通过智能的寄存器管理和栈空间优化，实现高效的内存管理。

## 栈帧布局设计

### 整体布局

栈帧从高地址到低地址的完整布局：

```
+-------------------+ 
| 调用者的栈帧       |
+-------------------+ <- fp (帧指针)
| 返回地址 (ra)     | <- fp-4  
+-------------------+
| 保存的帧指针 (fp) | <- fp-8
+-------------------+
| 被调用者保存寄存器 | <- calleeRegsSize
+-------------------+
| 调用者保存寄存器   | <- callerRegsSize  
+-------------------+
| 局部变量和临时变量 | <- localsAndPadding
+-------------------+ <- sp (栈指针)
```

### 各区域详细说明

#### 1. 基础帧信息（8字节固定）
- **返回地址（ra）**：保存函数调用的返回地址
- **帧指针（fp）**：保存调用者的帧指针，用于栈帧链接

#### 2. 寄存器保存区（可变大小）
- **被调用者保存寄存器**：`s0-s11`（除 `s0/fp` 外）
  - 函数必须保证这些寄存器在返回时值不变
  - 只为实际使用的寄存器分配空间
  
- **调用者保存寄存器**：`t0-t6`, `a0-a7`, `ra`
  - 在函数调用前需要保存的寄存器
  - 动态分析实际使用情况

#### 3. 变量存储区（可变大小）
- **局部变量**：函数内部声明的变量
- **临时变量**：编译器生成的中间计算结果
- **函数参数**：所有参数的栈存储副本

## 参数传递策略

### 混合传递机制

#### 寄存器传递（前8个参数）
```cpp
if (i < 8) {
    // 通过 a0-a7 寄存器传递
    regAlloc[currentFunctionParams[i]] = "a" + std::to_string(i);
}
```

#### 栈传递（第9个及以后参数）
```cpp
else {
    // 直接在栈上传递
    localVars[currentFunctionParams[i]] = -12 - (i-8)*4;
    localVarsSize += 4;
}
```

### 统一栈存储策略

所有参数（包括寄存器传递的）都会在被调用函数的栈帧中保存副本：

```cpp
// 保存寄存器参数到栈中，便于后续统一访问
emitInstruction("sw " + argReg + ", " + std::to_string(offset) + "(fp)");
```

**优势**：
- 简化变量访问逻辑
- 统一通过 `fp + offset` 访问所有变量
- 便于调试和错误处理

## 栈内容分类

### 会被压入栈的内容

#### 1. 函数调用上下文
- **返回地址（ra）**：每个函数调用必须保存
- **帧指针（fp）**：维护栈帧链接
- **被调用者保存寄存器**：函数使用时需要保存

#### 2. 函数调用时的寄存器状态
- **调用者保存寄存器**：调用前保存，调用后恢复
- **参数寄存器（a0-a7）**：可能需要保存以便重复使用

#### 3. 变量和数据
- **局部变量**：函数内声明的所有变量
- **临时变量**：表达式计算的中间结果
- **函数参数**：所有参数的栈存储副本
- **超过8个的参数**：直接通过栈传递

#### 4. 编译器生成的临时数据
- **寄存器溢出**：寄存器不够用时的临时存储
- **复杂表达式的中间值**
- **类型转换的临时结果**

## 动态分配机制

### 按需分配策略

```cpp
int CodeGenerator::getOperandOffset(const std::shared_ptr<Operand>& op) {
    // 检查是否已分配
    auto it = localVars.find(op->name);
    if (it != localVars.end()) {
        return it->second;
    }
    
    // 分配新的栈空间（向下增长）
    int offset = currentStackOffset;
    currentStackOffset -= 4;
    localVars[op->name] = offset;
    return offset;
}
```

### 智能寄存器分析

#### 使用情况检测
```cpp
void analyzeUsedCalleeSavedRegs();  // 分析被调用者保存寄存器使用
void analyzeUsedCallerSavedRegs();  // 分析调用者保存寄存器使用
int analyzeTempVars();              // 分析临时变量需求
```

#### 优势
- 避免为未使用的寄存器分配栈空间
- 减少不必要的保存/恢复操作
- 提高栈空间利用效率

## 栈空间优化

### 内存对齐

#### 16字节对齐策略
```cpp
int totalFrameSize = calleeRegsSize + callerRegsSize + localsAndPadding + 8;
totalFrameSize = (totalFrameSize + 15) & ~15;  // 强制16字节对齐
```

**原因**：
- RISC-V ABI 要求
- 提高内存访问性能
- 确保某些指令的正确执行


### 生命周期分析
这个函数的核心是通过分析指令序列，追踪临时变量的生命周期，从而优化栈空间的分配。
```cpp
int analyzeTempVars();      //遍历指令序列，记录临时变量的生命周期和冲突关系
```
#### 1. 初始化：
- 使用 activeTemps 集合记录当前活跃的临时变量。
- 初始化 maxTempSize 为 0，用于记录栈中临时变量的最大空间需求。
#### 2. 遍历指令：
- 对每条指令，获取其定义的寄存器（defRegs）和使用的寄存器（useRegs）。
#### 3. 释放失效的临时变量：
- 如果某个寄存器在当前指令中被定义（defRegs），说明旧值失效，从 activeTemps 中移除。

#### 4. 记录需要栈存储的临时变量：
- 如果某个寄存器在 useRegs 中且是临时变量，并且没有分配到寄存器，则将其加入 activeTemps。

#### 5. 更新最大栈空间需求：
- 根据 activeTemps 的大小，计算当前需要的栈空间，并更新 maxTempSize。
#### 6. 返回结果：
- 返回 maxTempSize，即临时变量在栈中所需的最大空间。


## 关键特性

### 1. 统一访问模式
- 所有变量通过 `fp + offset` 访问
- 简化了代码生成逻辑
- 便于调试和分析

### 2. 灵活的寄存器管理
- 寄存器不足时自动溢出到栈
- 智能分析寄存器使用情况

### 3. 空间效率优化
- 只为实际使用的寄存器分配保存空间
- 变量生命周期分析

## 实现细节

### 栈指针管理
```cpp
void CodeGenerator::emitPrologue(const std::string& funcName) {
    // 调整栈指针，分配整个栈帧
    emitInstruction("addi sp, sp, -" + std::to_string(totalFrameSize));
    
    // 保存返回地址和帧指针
    emitInstruction("sw ra, " + std::to_string(totalFrameSize - 4) + "(sp)");
    emitInstruction("sw fp, " + std::to_string(totalFrameSize - 8) + "(sp)");
    
    // 设置新的帧指针
    emitInstruction("addi fp, sp, " + std::to_string(totalFrameSize));
}
```

### 变量访问
```cpp
// 所有变量访问都通过帧指针相对偏移
std::string addr = std::to_string(offset) + "(fp)";
emitInstruction("lw " + reg + ", " + addr);  // 加载
emitInstruction("sw " + reg + ", " + addr);  // 存储
```
