# Parser 议题 08：分号与语句结束

> 状态：已确认，议题 18、24—28 补充表达式、控制流、清理与异常语句边界
> 确认日期：2026-08-03

## 1. 显式分号

Ink 不提供自动分号插入。所有由对应语法定义为简单语句的结构都必须显式以 `;` 结束。

代表性产生式为：

```ebnf
expression_statement =
    expression, ";" ;

defer_statement =
    "defer",
    ( expression, ";"
    | statement_block ) ;

return_statement =
    "return", [ expression ], ";" ;

break_statement =
    "break", ";" ;

continue_statement =
    "continue", ";" ;

throw_statement =
    "throw",
    ( ";"
    | expression, [ throw_cause_clause ], ";" ) ;

throw_cause_clause =
    "from", identifier ;
```

这些产生式确认分号规则；议题 18 进一步规定表达式语句允许丢弃非 `void` 结果，并以分号作为未消费临时结果的销毁边界。议题 25 确认 `break;` 与 `continue;` 不接受值或标签，并只作用于最内层普通循环。议题 26 确认 `return` 的可选结果表达式，以及表达式和 block 两种 `defer`。议题 27 确认新异常、显式原因和 `throw;` 重新抛出都由真实分号结束；其中 `"from"` 直接匹配硬关键字 Token。

## 2. 换行没有终止作用

换行是 Trivia，不参与普通语句结束判断：

```ink
const result =
    first_value
    + second_value;
```

Parser 只在读到语法要求的 `Symbol(';')` 时结束该简单语句。换行数量、缩进和前一行末尾 Token 都不能触发隐式分号。

因此：

```ink
const value = calculate()
process(value)
```

不会被解释为两个完整语句；它缺少必要分号并进入普通语法错误恢复。

## 3. 需要分号的结构

凡具体产生式属于简单声明或简单语句，都在末尾直接写出 `";"`。这至少包括：

- `import` 与 `from ... import ...`；
- 局部或 module 级简单绑定声明；
- 表达式语句；
- `return`、`break`、`continue`、`throw` 等跳转语句；
- 表达式式 `defer expression;` 延迟清理语句；
- 以后定义为简单语句的其他结构。

初始化表达式最后一个字符是 `}` 时，也不会替代外层声明分号：

```ink
const value = Value {
    field: 10
};
```

聚合初始化使用已经确认的显式 `type { field: expression }` 形状；此例只说明外层绑定仍由 `;` 终止。

## 4. 不使用结尾分号的结构

由花括号声明体或语句体自身闭合的结构，结束 `}` 后不再增加普通结尾分号。例如：

```ink
func calculate() -> i32 {
    return 42;
}
```

函数、类、接口、枚举、条件、循环以及其他带完整花括号体的结构是否属于此类，由对应 EBNF 明确写出；不能仅凭视觉上出现 `}` 猜测。

议题 24 的 `match_statement` 由自己的 `}` 结束，不写分号；`match_expression` 自身不包含外层分号，由包含它的绑定声明或表达式语句提供：

```ink
match (state) {
    .ready => run();
    _ => wait();
}

const code = match (state) {
    .ready => 1,
    _ => 0,
};
```

议题 26 的 `defer { ... }` 同样由自身 `statement_block` 的 `}` 结束，后面不写分号；`defer expression;` 则继续要求分号。

议题 28 的 `try_statement` 由最后一个 `catch` 的 `statement_block` 结束，整个结构后不写分号。`try` body 和每个处理器 body 的结束 `}` 只结束各自 block；至少一个紧随其后的 `catch` 是 `try_statement` 自身的必需部分。

没有源码体的函数、接口方法、`extern` 声明或其他特殊声明是否以 `;` 结束，由其具体产生式规定。它们不依赖自动分号规则。

## 5. 不存在空语句

单独的 `;` 不构成合法语句或顶层项：

```ink
;
;;;;
```

这些分号作为无法归类的真实 Token 进入 Parser 错误恢复，不能静默忽略。

因此带花括号体的结构后多写一个分号也非法：

```ink
if (condition) {
    run();
};
```

结束 `}` 已经结束条件结构，后面的 `;` 不是合法空语句。

## 6. CST 表示

实际存在的分号是独立 `Symbol(';')` Token，并作为对应 CST 节点的最后一个真实子元素保留：

```text
ReturnStatement
├─ Keyword(return)
├─ Expression
└─ Symbol(';')
```

如果分号缺失且当前 Token 能够明确开始下一结构，Parser 可以按议题 03 在语句末尾加入零宽度 `MissingToken(';')`，但不能把换行伪装成真实分号，也不能修改 Tokenizer Token 流。

## 7. 格式化与代码生成

格式化器可以重新排列语句内部 Trivia 和换行，但必须保留或生成语法要求的真实分号。保留源码的工具不能因为某行看起来已经结束而删除分号。

结构化代码生成必须根据产生式生成分号，而不是根据生成文本的最后字符或换行状态进行自动插入。

## 8. 确认结论

Ink 只使用显式 `;` 终止简单语句和简单声明，不提供自动分号插入，换行始终只是 Trivia。`return`、表达式式 `defer`、`break`、`continue` 以及三种 `throw` 形态保留自己的结尾分号；block 式 `defer` 和完整 `try_statement` 由最终 `}` 自行结束。其他花括号体是否自终止也由对应产生式决定；`match_statement` 自行结束，`match_expression` 由外层消费者提供分号。单独分号不形成空语句，多余或缺失分号都由普通 CST 错误恢复处理。
