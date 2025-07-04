# ToyC CodeGen 模块

本模块负责将 IR（中间表示）指令序列翻译为 RISC-V 汇编代码。

## 主要功能

- 遍历 IR 指令，生成对应的 RISC-V 汇编
- 支持函数调用、参数传递、返回值、条件跳转、循环、变量/临时变量管理等
- 动态分配和回填栈帧大小，保证局部变量和参数的正确访问
- 支持参数缓存与统一分配，符合 RISC-V 调用约定
- 支持函数序言/后记自动生成和回填
- 支持多函数代码生成

## 主要文件

- `codegen.h`：CodeGenerator 类声明，接口定义
- `codegen.cpp`：CodeGenerator 类实现，核心逻辑

## 关键类与方法

- `CodeGenerator`
  - `generate()`：主入口，遍历 IR 指令并生成汇编
  - `processInstruction()`：分发各类 IR 指令到对应处理函数
  - `processBinaryOp`/`processUnaryOp`/`processAssign`/`processCall` 等：各类 IR 指令的 RISC-V 生成
  - `emitInstruction`/`emitComment`：汇编输出缓冲
  - `getOperandOffset()`：为变量/临时变量分配栈空间
  - `processFunctionBegin`/`processFunctionEnd`：函数序言/后记生成与栈帧回填

## 代码结构说明

- 所有汇编代码先写入 `codeBuffer`，最后统一输出到文件，便于回填和多函数支持
- 支持多函数，每个函数的栈帧大小动态计算并回填
- 参数传递严格遵循 RISC-V 调用约定（前8个参数用 a0~a7，更多参数用栈）


## 典型用法

```cpp
CodeGenerator codegen("output.s", irInstructions);
codegen.generate();