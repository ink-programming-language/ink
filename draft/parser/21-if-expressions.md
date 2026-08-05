# Parser 议题 21：无块 `if` 表达式

> 状态：已确认，议题 22 补充表达式优先级；2026-08-05 统一要求条件括号
> 确认日期：2026-08-03

## 1. 基本形式

有值条件表达式使用 `if (...) then ... else ...`，不使用花括号：

```ebnf
if_expression =
    "if", "(", logical_or_expression, ")",
    "then", expression, "else", expression ;
```

```ink
const value = if (condition) then first else second;
```

三个表达式依次表示条件、条件成立时的结果和条件不成立时的结果。`then` 与 `else` 是该结构的显式分隔 Token。

## 2. 分支恰好是表达式

`then` 和 `else` 后面各接一个完整 `expression`：

```ink
const value = if (ready) then load() else fallback();
```

分支不是 `statement_block`，因此不能在分支位置直接放置局部声明、普通语句序列或无值 `{ ... }`：

```ink
const value = if (ready) then {
    log();
    load();
} else {
    fallback();
};
```

上面的花括号形式不是 `if_expression`。如果某种其他表达式自身使用花括号，例如未来完整定义的聚合初始化或议题 24 的 `match_expression`，它仍可以作为一个分支表达式出现；这不会把普通语句块变成表达式。

## 3. `else` 必须存在

`if_expression` 必须同时包含 `then` 和 `else`：

```ink
const value = if (ready) then load();
```

该 Token 序列不能形成完整 `if_expression`。允许省略 `else` 的控制结构是议题 20 的 `if_statement`，它使用花括号语句块且不产生值。

## 4. `else if` 链

假分支本身可以是另一个 `if_expression`：

```ink
const value =
    if (first) then a
    else if (second) then b
    else c;
```

这不需要专门的 `else_if_expression` 产生式；最后一个 `expression` 递归包含下一层 `if_expression`。每一层都必须具有自己的 `then` 和 `else`。

## 5. 与 `if_statement` 的区别

两种结构共享 `if` 起始 Token，但后续定界符不同：

```ink
if (condition) {
    run();
}

const value = if (condition) then first else second;
```

- `if_statement` 在条件后进入 `statement_block`；
- `if_expression` 在条件后出现 `then`；
- `if_statement` 自身不需要结尾分号；
- `if_expression` 自身不包含结尾分号，外层绑定声明或表达式语句按自己的产生式提供 `;`。

## 6. Parser 与 CST

CST 使用专用 `IfExpression` 节点并按源码顺序保存：

- `if` Token；
- 固定的左右括号；
- 条件表达式；
- `then` Token；
- 真分支表达式；
- `else` Token；
- 假分支表达式；
- 其中的全部 Trivia。

Parser 不把任一分支包装成 `StatementBlock` 或此前预留的 `ConditionalExprArm`。条件类型、两个结果类型是否兼容以及最终值类别都不属于本议题。

## 7. 表达式优先级连接

议题 22 将 `if_expression` 放在完整表达式的最低优先级层。条件使用 `logical_or_expression`，两个分支使用完整 `expression`。条件表达式作为外层二元运算的操作数，或者需要对整个结果继续调用、索引和成员访问时，必须使用圆括号。

## 8. 确认结论

Ink 的有值条件表达式写作 `if (condition) then true_expression else false_expression`。条件括号是结构自身的固定定界符；表达式必须具有 `then` 和 `else`，两个分支各是一个表达式，不使用有值花括号分支，也不把普通语句块提升为表达式。假分支可以递归包含另一个 `if_expression` 形成链式条件。
