# ToyC IR 模块

本模块负责将 AST（抽象语法树）转换为 IR（中间表示）指令序列，并支持 IR 的打印与优化。

## 主要功能

- 遍历 AST，生成三地址码风格的 IR 指令
- 支持表达式、赋值、条件、循环、函数定义与调用等
- 自动生成临时变量和标签
- 支持作用域和变量管理
- 支持 IR 的打印输出
- ⚠ 尚未实现：IR 优化（如常量折叠、死代码删除等）

## 主要文件

- `ir.h`：IR 指令、操作数、IRGenerator 等类声明
- `irgen.h`：IRGenerator 类声明及相关优化器接口
- `irgen.cpp`：IRGenerator 类实现，AST 到 IR 的转换逻辑

## 关键类与方法

- `Operand`：IR 操作数（变量、常量、临时变量、标签）
- `IRInstr` 及其子类：各种 IR 指令（`BinaryOpInstr`、`AssignInstr`、`CallInstr` 等）
- `IRGenerator`
  - `generate()`：主入口，AST 到 IR 的转换
  - `visit(BinaryExpr&)` 等：递归遍历 AST，生成三地址码
  - `createTemp()`：自动生成临时变量
  - `addInstruction()`：添加 IR 指令
- `IRPrinter`：IR 指令的文本输出

## 代码结构说明

- 复杂表达式会自动拆分为多条三地址码，每步结果存入临时变量
- 参数、作用域、变量管理自动处理
- 支持 IR 优化接口，可扩展更多优化器

## 待优化部分

- **IR 优化尚未实现**：如常量折叠、死代码删除、公共子表达式消除等优化功能暂未集成。
- **更丰富的控制流优化**：如基本块划分、跳转优化等可进一步完善。
- **SSA 形式与寄存器分配接口**：如需支持更高级的后端优化，可考虑引入 SSA 形式和寄存器分配相关接口。

## 典型用法

```cpp
IRGenerator irgen;
irgen.generate(ast);
const auto& ir = irgen.getInstructions();
IRPrinter printer;
printer.print(ir);
```

