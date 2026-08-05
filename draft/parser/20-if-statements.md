# Parser 议题 20：`if` 语句

> 状态：已确认，议题 23、25 补充条件模式并由 `while (match ...)` 复用；2026-08-05 增加 `comptime if`，Parser 议题 32 统一区域控制；2026-08-05 统一要求控制头括号
> 确认日期：2026-08-04

## 1. 产生式

普通运行时 `if` 语句使用以下 EBNF：

```ebnf
if_statement =
    "if", "(", if_condition, ")", statement_block,
    [ "else", ( statement_block | if_statement ) ] ;

if_condition =
      expression
    | match_condition ;

match_condition =
    "match", conditional_match_pattern, "=", expression ;

comptime_if_statement =
    "comptime", if_statement ;
```

`statement_block` 由议题 09 定义，`expression` 使用已经确认的表达式文法。`conditional_match_pattern` 由议题 23 定义为顶层 `variant_pattern`，并由议题 25 的 `while (match ...)` 复用。

## 2. 条件必须使用括号

完整条件必须写在 `if (` 与 `)` 之间：

```ink
if (ready) {
    run();
}
```

这对普通表达式和 `match_condition` 使用同一组固定定界符。条件内部若要额外分组，继续使用普通加括号表达式：

```ink
if ((left || right) && enabled) {
    run();
}
```

最外层圆括号属于 `if_statement`，内层圆括号才是 `parenthesized_expression`。该固定形状也避免紧随条件的 `{` 与聚合初始化体产生语法歧义。

## 3. 执行体必须是语句块

每个普通执行体都必须使用 `{}`：

```ink
if (ready) {
    run();
}
```

不能省略花括号并直接跟随单条语句：

```ink
if (ready) run();
```

`statement_block` 是无值的普通语句块，不会因为出现在 `if` 后面而变成 block expression。议题 21 的有值 `if_expression` 使用 `if (...) then ... else ...`，不使用花括号分支。

## 4. `else` 与 `else if`

`else` 可以省略，也可以跟随一个普通语句块：

```ink
if (ready) {
    run();
} else {
    wait();
}
```

`else if` 在文法上是 `else` 后嵌套另一个完整 `if_statement`：

```ink
if (first) {
    a();
} else if (second) {
    b();
} else {
    c();
}
```

由于每个执行体都必须使用花括号，文法不存在悬空 `else`。每个 `else` 归属于直接包含它的 `if_statement`。

## 5. `if (match ...)`

模式条件使用显式的 `match`：

```ink
if (match .some(value) = optional) {
    use(value);
}
```

`match_condition` 在语法上由 `match`、一个 `conditional_match_pattern`、单字符 `=` Symbol Token 和一个完整 `expression` 组成。它不是议题 10 的局部绑定声明，不以分号结束，也不能脱离条件位置成为普通语句。

`if (` 后若下一个显著 Token 是 `match`，且其后是 `.`，Parser 进入 `match_condition`；普通 `match_expression` 的被匹配表达式位于自己的括号内，不能以 `.` 开始，因此两种结构可以确定性区分。

是否匹配成功、绑定哪些名称、模式是否适用于右侧值以及绑定的类型与可变性，都不由 Parser 判断。

## 6. `comptime if`

编译期条件结构直接在完整 `if_statement` 前添加 `comptime`：

```ink
comptime if (target.os == Os.windows) {
    use_windows_backend();
} else if (target.os == Os.linux) {
    use_linux_backend();
} else {
    compile_error("unsupported target");
}
```

`comptime` 修饰整条条件链，因此 `else if` 不重复书写 `comptime`。Parser 建立议题 32 的统一 `ComptimeIfControl`，在普通函数体中使用 `RegionKind::Statement` 并保留完整条件链和 `StatementBlock`。条件能否在编译期求值、未选择分支是否参与后续语义分析以及分支如何残留，均属于编译期执行语义。

上述产生式是统一 schema 的 `StatementRegion` 标准 EBNF 展开。在 module 或类型成员 declaration region 中，表面语法仍是同一个 `comptime if (...) ...`，Parser 议题 32 只把 `RegionRules` 换成 `TopLevelRegion` 或对应 member region；Parser 议题 05 给出顶层具体展开。不同区域共享控制 Parser 和 CST 节点，但 body 保持准确的 region block，不能事后把 `StatementBlock` 改判为声明块。

## 7. 语句结尾

`if_statement` 由最后一个 `statement_block` 的 `}` 自行结束，末尾不写分号：

```ink
if (ready) {
    run();
}
```

```ink
if (ready) {
    run();
};
```

第二种写法中的额外 `;` 不属于 `if_statement`。Ink 不存在空语句，Parser 按既有规则处理该多余 Token。

## 8. Parser 与 CST

CST 使用专用 `IfStatement` 节点，并完整保留：

- `if`、固定左右括号、`else` 和可选 `match` Token；
- 条件、模式和 `=` Token；
- 每个 `StatementBlock`；
- 块之间及内部的全部 Trivia；
- 嵌套 `else if` 对应的子 `IfStatement`。

Parser 只按产生式识别结构，不在 CST 中记录条件是否为 `bool`、模式是否可匹配或分支是否可达等语义结论。

## 9. 确认结论

Ink 的普通 `if_statement` 强制使用 `if (condition) statement_block`，所有执行体必须是无值 `statement_block`。`else` 可省略，`else if` 表示递归嵌套的 `if_statement`，整个结构不写结尾分号。括号内条件可以是普通表达式，也可以是 `match variant_pattern = expression` 形式；模式语法和访问能力传播由议题 23 定义。`comptime if` 直接要求整条条件链在编译期选择分支，链中的 `else if` 不重复 `comptime`；module、语句和类型成员位置都降低为同一个区域控制节点。
