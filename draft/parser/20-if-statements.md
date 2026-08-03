# Parser 议题 20：`if` 语句

> 状态：已确认  
> 确认日期：2026-08-03

## 1. 产生式

普通运行时 `if` 语句使用以下 EBNF：

```ebnf
if_statement =
    "if", if_condition, statement_block,
    [ "else", ( statement_block | if_statement ) ] ;

if_condition =
      expression
    | let_condition ;

let_condition =
    "let", pattern, "=", expression ;
```

`statement_block` 由议题 09 定义，`expression` 使用已经确认的表达式文法。`pattern` 是模式语法的非终结符，其完整产生式由后续模式议题定义；本议题只确定 `if let` 中的位置。

## 2. 条件不使用强制括号

条件直接写在 `if` 与语句块之间：

```ink
if ready {
    run();
}
```

外层不要求也不引入专用条件括号。普通圆括号表达式仍然可以显式使用：

```ink
if (left || right) {
    run();
}
```

圆括号在这里属于普通 `parenthesized_expression`，不是 `if_statement` 自身的固定定界符。

## 3. 执行体必须是语句块

每个普通执行体都必须使用 `{}`：

```ink
if ready {
    run();
}
```

不能省略花括号并直接跟随单条语句：

```ink
if ready run();
```

`statement_block` 是无值的普通语句块，不会因为出现在 `if` 后面而变成 block expression。议题 21 的有值 `if_expression` 使用 `if ... then ... else ...`，不使用花括号分支。

## 4. `else` 与 `else if`

`else` 可以省略，也可以跟随一个普通语句块：

```ink
if ready {
    run();
} else {
    wait();
}
```

`else if` 在文法上是 `else` 后嵌套另一个完整 `if_statement`：

```ink
if first {
    a();
} else if second {
    b();
} else {
    c();
}
```

由于每个执行体都必须使用花括号，文法不存在悬空 `else`。每个 `else` 归属于直接包含它的 `if_statement`。

## 5. `if let`

模式条件使用显式的 `let`：

```ink
if let .some(value) = optional {
    use(value);
}
```

`let_condition` 在语法上由一个 `pattern`、单字符 `=` Symbol Token 和一个完整 `expression` 组成。它不是议题 10 的局部绑定声明，不以分号结束，也不能脱离条件位置成为普通语句。

是否匹配成功、绑定哪些名称、模式是否适用于右侧值以及绑定的类型与可变性，都不由 Parser 判断。

## 6. 语句结尾

`if_statement` 由最后一个 `statement_block` 的 `}` 自行结束，末尾不写分号：

```ink
if ready {
    run();
}
```

```ink
if ready {
    run();
};
```

第二种写法中的额外 `;` 不属于 `if_statement`。Ink 不存在空语句，Parser 按既有规则处理该多余 Token。

## 7. Parser 与 CST

CST 使用专用 `IfStatement` 节点，并完整保留：

- `if`、`else` 和可选 `let` Token；
- 条件、模式和 `=` Token；
- 每个 `StatementBlock`；
- 块之间及内部的全部 Trivia；
- 嵌套 `else if` 对应的子 `IfStatement`。

Parser 只按产生式识别结构，不在 CST 中记录条件是否为 `bool`、模式是否可匹配或分支是否可达等语义结论。

## 8. 确认结论

Ink 的普通 `if_statement` 不强制条件括号，所有执行体必须使用无值 `statement_block`。`else` 可省略，`else if` 表示递归嵌套的 `if_statement`，整个结构不写结尾分号。条件可以是普通表达式，也可以是 `if let pattern = expression` 形式；模式的完整语法另行定义。
