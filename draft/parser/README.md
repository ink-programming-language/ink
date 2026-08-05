# Ink Parser 语法设计

本目录用于逐项讨论并记录 Ink 从 full-fidelity Token 列表到语法树的 parser 规则。只有已经确认的规则才创建独立议题文件；讨论中的方案不提前写入规范。

## 议题状态

| 编号 | 议题 | 状态 | 文件 |
| --- | --- | --- | --- |
| 01 | 无损 CST | 已确认 | [01-full-fidelity-cst.md](01-full-fidelity-cst.md) |
| 02 | Token 游标、Trivia 与连续符号 | 已确认 | [02-token-cursor-trivia-and-symbol-sequences.md](02-token-cursor-trivia-and-symbol-sequences.md) |
| 03 | 语法错误恢复 | 已确认 | [03-syntax-error-recovery.md](03-syntax-error-recovery.md) |
| 04 | 标准 EBNF 记法 | 已确认 | [04-standard-ebnf-notation.md](04-standard-ebnf-notation.md) |
| 05 | 源文件与条件导入 | 已确认，顶层声明集合完整定义；声明区支持五种固定控制头括号的 `comptime` 结构并作为议题 32 的 `TopLevelRegion` 实例 | [05-source-file-and-conditional-imports.md](05-source-file-and-conditional-imports.md) |
| 06 | Import 语法 | 已确认 | [06-import-syntax.md](06-import-syntax.md) |
| 07 | 逗号分隔列表 | 已确认，枚举分支同样禁止尾随逗号 | [07-comma-separated-lists.md](07-comma-separated-lists.md) |
| 08 | 分号与语句结束 | 已确认 | [08-semicolons-and-statement-termination.md](08-semicolons-and-statement-termination.md) |
| 09 | 语句块与花括号 | 已确认，增加 `comptime { ... }` 并接入统一区域控制 | [09-statement-blocks.md](09-statement-blocks.md) |
| 10 | `var`、`const` 与元组解构声明 | 已确认，字段同样必须显式写 `var` 或 `const`；完整字段语法见议题 33 | [10-binding-declarations.md](10-binding-declarations.md) |
| 11 | 赋值语句 | 已确认 | [11-assignment-statements.md](11-assignment-statements.md) |
| 12 | 表达式运算符优先级 | 已确认 | [12-expression-operator-precedence.md](12-expression-operator-precedence.md) |
| 13 | 表达式求值顺序 | 已确认，议题 15 补充命名实参与默认值求值顺序 | [13-expression-evaluation-order.md](13-expression-evaluation-order.md) |
| 14 | 基础表达式 | 已确认 | [14-primary-expressions.md](14-primary-expressions.md) |
| 15 | 普通后缀表达式 | 已确认，增加普通调用、attribute 与 decorator 共用的命名实参 | [15-ordinary-postfix-expressions.md](15-ordinary-postfix-expressions.md) |
| 16 | 泛型实参后缀与尖括号消歧 | 已确认 | [16-generic-argument-postfix.md](16-generic-argument-postfix.md) |
| 17 | 切片后缀 | 已确认 | [17-slice-postfix.md](17-slice-postfix.md) |
| 18 | 表达式语句与丢弃结果 | 已确认 | [18-expression-statements-and-discarded-results.md](18-expression-statements-and-discarded-results.md) |
| 19 | 一元表达式 | 已确认，增加 `comptime` 表达式前缀并作为议题 32 的 `ValueRegion` 形式 | [19-unary-expressions.md](19-unary-expressions.md) |
| 20 | `if` 语句 | 已确认，增加 `comptime if` 并接入统一区域控制；条件使用固定括号 | [20-if-statements.md](20-if-statements.md) |
| 21 | 无块 `if` 表达式 | 已确认，条件使用固定括号 | [21-if-expressions.md](21-if-expressions.md) |
| 22 | `if` 表达式优先级 | 已确认，固定括号内仍使用逻辑或层 | [22-if-expression-precedence.md](22-if-expression-precedence.md) |
| 23 | 模式语法与访问能力传播 | 已确认 | [23-pattern-syntax-and-access-propagation.md](23-pattern-syntax-and-access-propagation.md) |
| 24 | `match` 语句与表达式 | 已确认，增加 `comptime match` 并接入统一区域控制；被匹配表达式使用固定括号 | [24-match-statements-and-expressions.md](24-match-statements-and-expressions.md) |
| 25 | `while (...)`、`while (match ...)`、`for (...)` 与循环跳转 | 已确认，增加 `comptime while` 与 `comptime for` 并接入统一区域控制；循环头使用固定括号 | [25-runtime-loops-and-loop-jumps.md](25-runtime-loops-and-loop-jumps.md) |
| 26 | `return` 与 `defer` 语句 | 已确认，省略函数返回类型固定为 `void`，不做返回类型推导 | [26-return-and-defer-statements.md](26-return-and-defer-statements.md) |
| 27 | `throw`、重新抛出与显式原因 | 已确认 | [27-throw-and-rethrow-statements.md](27-throw-and-rethrow-statements.md) |
| 28 | `try` 与 `catch` 语句 | 已确认 | [28-try-and-catch-statements.md](28-try-and-catch-statements.md) |
| 29 | 统一类型语法与函数类型 | 已确认，省略结果固定为 `void`；议题 15 的命名实参规则同步到列表展开 | [29-type-syntax-and-function-types.md](29-type-syntax-and-function-types.md) |
| 30 | 普通表达式中的复合类型值 | 已确认 | [30-compound-type-values-in-expressions.md](30-compound-type-values-in-expressions.md) |
| 31 | 函数声明 | 已确认，统一 annotation、修饰符、泛型与运行时参数、只读接收者、返回子句和 body | [31-function-declarations.md](31-function-declarations.md) |
| 32 | 统一的 `comptime` 区域控制 | 已确认，表达式与 module、语句、类、接口、枚举区域共享同一阶段模型；结构化控制头使用固定括号 | [32-unified-comptime-regions.md](32-unified-comptime-regions.md) |
| 33 | 类与接口声明 | 已确认，统一类型前缀、泛型、继承、字段、成员块、嵌套类型和接口 Parser 边界；成员区域同步固定控制头括号 | [33-class-and-interface-declarations.md](33-class-and-interface-declarations.md) |
| 34 | 枚举声明 | 已确认，统一类型前缀、泛型、分支载荷与判别式、无尾随逗号成员列表和枚举成员 `comptime` 区域 | [34-enum-declarations.md](34-enum-declarations.md) |
| 35 | 聚合初始化表达式 | 已确认，显式类型、命名字段、无尾随逗号，并可直接作为普通后缀链起点 | [35-aggregate-initialization-expressions.md](35-aggregate-initialization-expressions.md) |
| 36 | 装饰器声明 | 已确认，仅允许顶层声明；除前缀不接受 decorator application 外复用完整函数声明骨架；`function` continuation 不占形参 | [36-decorator-declarations.md](36-decorator-declarations.md) |
| 37 | 调用中的裸 `...` 全参数转发 | 已确认，裸 `...` 独占完整实参序列并与 `...expression` 列表展开区分 | [37-forward-all-call-arguments.md](37-forward-all-call-arguments.md) |

Parser 议题在本目录内独立编号，从 `01` 开始。Parser 消费 [`../tokenizer/README.md`](../tokenizer/README.md) 定义的完整 Token 流；语法树 lowering、名称绑定、类型检查和 InkIR 生成不属于本目录。
