# Ink Parser 语法设计

本目录用于逐项讨论并记录 Ink 从 full-fidelity Token 列表到语法树的 parser 规则。只有已经确认的规则才创建独立议题文件；讨论中的方案不提前写入规范。

## 议题状态

| 编号 | 议题 | 状态 | 文件 |
| --- | --- | --- | --- |
| 01 | 无损 CST | 已确认 | [01-full-fidelity-cst.md](01-full-fidelity-cst.md) |
| 02 | Token 游标、Trivia 与连续符号 | 已确认 | [02-token-cursor-trivia-and-symbol-sequences.md](02-token-cursor-trivia-and-symbol-sequences.md) |
| 03 | 语法错误恢复 | 已确认 | [03-syntax-error-recovery.md](03-syntax-error-recovery.md) |
| 04 | 标准 EBNF 记法 | 已确认 | [04-standard-ebnf-notation.md](04-standard-ebnf-notation.md) |
| 05 | 源文件与条件导入 | 已确认 | [05-source-file-and-conditional-imports.md](05-source-file-and-conditional-imports.md) |
| 06 | Import 语法 | 已确认 | [06-import-syntax.md](06-import-syntax.md) |
| 07 | 逗号分隔列表 | 已确认 | [07-comma-separated-lists.md](07-comma-separated-lists.md) |
| 08 | 分号与语句结束 | 已确认 | [08-semicolons-and-statement-termination.md](08-semicolons-and-statement-termination.md) |
| 09 | 语句块与花括号 | 已确认 | [09-statement-blocks.md](09-statement-blocks.md) |
| 10 | `let`、`var` 与 `const` 绑定声明 | 已确认 | [10-binding-declarations.md](10-binding-declarations.md) |
| 11 | 赋值语句 | 已确认 | [11-assignment-statements.md](11-assignment-statements.md) |
| 12 | 表达式运算符优先级 | 已确认 | [12-expression-operator-precedence.md](12-expression-operator-precedence.md) |
| 13 | 表达式求值顺序 | 已确认 | [13-expression-evaluation-order.md](13-expression-evaluation-order.md) |
| 14 | 基础表达式 | 已确认 | [14-primary-expressions.md](14-primary-expressions.md) |
| 15 | 普通后缀表达式 | 已确认 | [15-ordinary-postfix-expressions.md](15-ordinary-postfix-expressions.md) |
| 16 | 泛型实参后缀与尖括号消歧 | 已确认 | [16-generic-argument-postfix.md](16-generic-argument-postfix.md) |
| 17 | 切片后缀 | 已确认 | [17-slice-postfix.md](17-slice-postfix.md) |
| 18 | 表达式语句与丢弃结果 | 已确认 | [18-expression-statements-and-discarded-results.md](18-expression-statements-and-discarded-results.md) |
| 19 | 一元表达式 | 已确认 | [19-unary-expressions.md](19-unary-expressions.md) |
| 20 | `if` 语句 | 已确认 | [20-if-statements.md](20-if-statements.md) |
| 21 | 无块 `if` 表达式 | 已确认 | [21-if-expressions.md](21-if-expressions.md) |
| 22 | `if` 表达式优先级 | 已确认 | [22-if-expression-precedence.md](22-if-expression-precedence.md) |

Parser 议题在本目录内独立编号，从 `01` 开始。Parser 消费 [`../tokenizer/README.md`](../tokenizer/README.md) 定义的完整 Token 流；语法树 lowering、名称绑定、类型检查和 InkIR 生成不属于本目录。
