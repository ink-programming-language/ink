# Parser 议题 25：`while (...)`、`while (match ...)`、`for (...)` 与循环跳转

> 状态：已确认；2026-08-05 增加 `comptime for` 与 `comptime while`，Parser 议题 32 统一区域控制并统一要求循环头括号
> 确认日期：2026-08-04

## 1. 基本产生式

Ink 的普通循环使用以下 EBNF：

```ebnf
while_statement =
    "while", "(", while_condition, ")", statement_block ;

while_condition =
      expression
    | while_match_condition ;

while_match_condition =
    "match", conditional_match_pattern, "=", expression ;

for_statement =
    "for", "(", for_binding_mode, for_pattern,
    "in", for_source, ")", statement_block ;

for_binding_mode =
      "var"
    | "const" ;

for_pattern =
      binding_pattern
    | wildcard_pattern ;

for_source =
    expression, [ "..", expression ] ;

comptime_while_statement =
    "comptime", while_statement ;

comptime_for_statement =
    "comptime", for_statement ;

break_statement =
    "break", ";" ;

continue_statement =
    "continue", ";" ;
```

`conditional_match_pattern`、`binding_pattern` 和 `wildcard_pattern` 由议题 23 定义。`for_binding_mode` 复用议题 10 已确认的 `var`/`const` 绑定含义。所有循环体都必须是议题 09 的完整 `statement_block`；循环结构本身不以分号结束。

## 2. 普通 `while`

`while` 在每次尝试进入循环体前求值一次条件：

```ink
while (has_more_work()) {
    process_next();
}
```

外层括号是 `while_statement` 的固定组成部分；条件内部仍可使用普通加括号表达式：

```ink
while ((ready && enabled) || forced) {
    run();
}
```

条件必须在语义检查后具有 `bool` 类型。条件为 `false` 时循环结束；条件求值产生异常、trap 或其他控制流时，不进入本轮循环体。

循环体不能省略花括号，也不支持 `else`：

```ink
while (ready) run(); // 非法

while (ready) {
    run();
} else {           // 非法：while 没有 else
    wait();
}
```

## 3. `while (match ...)`

可反驳枚举模式循环使用与 `if (match ...)` 一致的显式形状：

```ink
while (match .some(value) = next_value()) {
    consume(value);
}
```

每次迭代尝试按以下顺序执行：

1. 求值右侧表达式一次；
2. 检查活动枚举分支；
3. 匹配成功时建立载荷绑定并执行循环体；
4. 匹配失败时结束循环。

顶层只接受议题 23 的 `conditional_match_pattern`，因此必须是枚举 `variant_pattern`：

```ink
while (match .item(value) = iterator.next()) {
    use(value);
}
```

以下形式非法：

```ink
while (match value = expression) {
    use(value);
}

while (match _ = expression) {
    run();
}
```

两者都是永远成功的模式，不属于条件匹配语法。

如果右侧产生临时枚举值，该临时值存活到当前迭代的循环体结束。`continue`、`break`、`return` 或异常离开本轮时，先按普通规则清理本轮局部对象并结束该临时值，再决定是否开始下一次条件求值。

载荷绑定继续遵守议题 23 的访问传播：可写枚举 place 产生 `T&`，只读 place 或临时枚举产生 `const T&`。pattern 内不写 `var` 或 `const`。

## 4. `while (match ...)` 与 `match_expression` 的消歧

`while (` 后若下一个显著 Token 是 `match`，且其后是 `.`，Parser 进入 `while_match_condition`。普通 `match_expression` 必须把被匹配表达式放在自己的括号内，不能以 `.` 开始，因此普通表达式条件仍然可以是返回 `bool` 的 `match_expression`：

```ink
while (match (state) {
    .ready => true,
    _ => false,
}) {
    run();
}
```

上例是 `while` 加一个普通 `match_expression` 条件，不是 `while (match pattern = expression)`。分支逗号、match 右花括号、循环头右括号和循环体右花括号分别结束对应结构。

## 5. 普通 `for (... in ...)`

普通迭代循环必须先为一个名称或 `_` 显式选择 `var` 或 `const`：

```ink
for (const value in values) {
    consume(value);
}

for (var value in generated_values()) {
    value = normalize(value);
    consume(value);
}

for (const _ in events) {
    count += 1;
}
```

`for_binding_mode` 与议题 10 的普通绑定声明含义一致，但整个 `for` 头不是带初始化器和分号的 `binding_declaration`。关键字不可省略，也不接受类型标注：

```ink
for (value in values) {}             // 非法：缺少 var 或 const
for (const value: Data in values) {} // 非法：for 绑定不写类型标注
```

v0 不在普通 `for` 头中接受元组解构、多名称或枚举分支模式：

```ink
for (const (key, value) in entries) {} // 非法
for (const index, value in values) {}  // 非法
```

需要解构时先绑定当前元素，再在循环体中使用议题 10 的普通解构声明。

## 6. 迭代绑定与元素访问能力

`for` 名称是每轮新建立的显式绑定。`var` 或 `const` 写在整个 `for_pattern` 前，不写进 pattern 内部。循环语法本身不会为了绑定额外复制元素：

- 迭代源产生普通值 `T` 时，本轮名称保存该值；`var` 允许重新赋值该循环局部值，`const` 不允许；
- 迭代源产生 `T&` 时，名称保持可写目标引用；外层 `var`/`const` 不会把它改写成 `const T&`，引用本身仍不能重新指向；
- 迭代源产生 `const T&` 时，名称保持只读目标引用；外层 `var` 不能去掉目标类型中的 `const`；
- `_` 不建立名称，也不额外复制当前元素；`var` 与 `const` 在该形状下没有语义差异，但语法上仍必须选择一个；
- 需要独立可修改副本时，在循环体中显式写 `var copy: T = value;`。

因此 `var`/`const` 只控制循环绑定的顶层可重新赋值能力，元素目标是否可写仍由 `T`、`T&`、`const T&` 等实际迭代结果类型决定，与议题 10 的普通声明规则一致。

数组和安全切片按从低索引到高索引的顺序迭代。可写元素访问可以产生 `T&`，只读元素访问产生 `const T&`。其他类型通过何种迭代协议产生值或引用属于后续迭代协议议题；Parser 对 `for_source` 只要求一个表达式形状。

迭代源表达式在进入循环前只求值一次。每轮绑定只在对应循环体中可见，并在本轮结束、`continue`、`break`、`return` 或异常离开时按照普通生命周期规则结束。

## 7. `for` 专用半开区间

为兼容既有索引循环，`for_source` 可以在两个表达式之间使用直接相邻的 `..`：

```ink
for (const index in 0 .. values.length) {
    use(values[index]);
}
```

该形式表示从起点开始、到终点之前结束的升序半开区间。两个边界在进入循环前按照源码从左到右各求值一次；每轮 `index` 是当前整数值。`const index` 不能重新赋值；改用 `var index` 可以修改本轮局部值，但不会改变循环内部保存的下一迭代值或边界。起点不小于终点时执行零次。

`..` 只在 `for_source` 中是两个边界之间的专用定界符，不建立普通范围值，也不进入表达式运算符优先级：

```ink
const range = 0 .. length; // 非法：没有普通范围表达式
consume(0 .. length);      // 非法
```

两个点必须直接相邻；点号两侧可以有 Trivia：

```ink
for (const index in 0..length) {}
for (const index in 0 .. length) {}

for (const index in 0 . . length) {} // 非法
```

解析 `for_source` 的第一个表达式时，位于当前定界深度的两个直接相邻点号是专用停止边界；Parser 不能先把第一个点号作为成员访问消费。单个 `.` 后接 Identifier 时仍属于普通成员后缀，因此 `object.start .. object.end` 可以确定性解析。

v0 不提供包含终点、反向或步长语法：

```ink
for (const index in 0 ..= length) {}        // 非法
for (const index in length .. 0 step -1) {} // 非法
```

这些遍历使用普通 `while` 或显式库迭代视图表达。

## 8. `break` 与 `continue`

`break;` 结束最内层 `while_statement` 或普通 `for_statement`，`continue;` 结束当前迭代并开始该循环的下一次条件或元素获取：

```ink
while (ready) {
    if (should_stop()) {
        break;
    }

    if (should_skip()) {
        continue;
    }

    process();
}
```

两者都必须带分号，不接受值或标签：

```ink
break value;      // 非法
break outer;      // 非法
continue outer;   // 非法
```

在任何 `while_statement` 或普通 `for_statement` 之外使用它们属于语义错误。即使 Token 位于嵌套 `if`、`match` 或普通语句块内，也仍作用于词法上最内层包含它的循环。循环最终在运行时还是编译期执行不改变该词法归属。

离开当前迭代前，已经成功构造的局部对象和 `defer` 按构造逆序清理。`continue` 完成清理后重新进入 `while (...)` 条件、`while (match ...)` 右侧求值或 `for (...)` 的下一元素步骤；`break` 完成清理后离开整个循环。

## 9. 循环不是表达式

`while_statement` 和 `for_statement` 都不产生值，不能作为初始化器、实参或其他表达式的操作数，也不支持带值 `break`：

```ink
const value = while (condition) { // 非法
    break;
};

consume(for (const item in values) { // 非法
    use(item);
});
```

循环的结束 `}` 自行结束结构，后面不写额外分号。Ink v0 也不提供 C 风格 `for (init; condition; step)`、`do ... while`、无限 `loop` 关键字、循环标签或循环 `else`。

无限循环可以显式写为：

```ink
while (true) {
    run_once();
}
```

## 10. `comptime for` 与 `comptime while`

`comptime` 可以直接修饰完整的普通循环结构，并复用相同的条件、绑定和 body 文法：

```ink
comptime for (const T in Types) {
    print(reflect(T).name);
}

comptime while (has_pending_work()) {
    process_next();
}
```

因此 `comptime for` 仍然必须写 `var` 或 `const`，也不增加逗号多绑定：

```ink
comptime for (const T in Types) { ... } // 合法
comptime for (T in Types) { ... }       // 缺少 var/const
comptime for (index, T in Types) { ... } // 非法多绑定
```

在普通语句上下文中，Parser 分别建立议题 32 的统一 `ComptimeForControl` 或 `ComptimeWhileControl`，并使用 `RegionKind::Statement`。循环源或每轮条件能否在编译期求值、循环是否收敛以及展开后的 body 是否残留运行时代码，由 Partial Evaluation 和资源预算检查。

同样的源码前缀还可以出现在 module、类型或其他 declaration region 中逐项产生声明。它仍是议题 32 的同一个区域控制，只把 `RegionRules` 和输出 sink 换成当前声明区域。header 继续复用 `for_binding_mode`、单一 `for_pattern`、`for_source` 与 `while_condition`，花括号内部则是当前 region items，而不是 `statement_block`。Parser 议题 05 已给出 module 顶层的具体 EBNF 展开；class、interface、enum 使用对应 member block。它不会恢复旧的无 `var`/`const` 或逗号多绑定形式。

## 11. CST 与错误恢复

CST 至少使用以下节点：

```text
WhileStatement
WhileMatchCondition
ForStatement
ComptimeWhileControl
ComptimeForControl
ForBindingMode
ForBindingPattern
ForWildcardPattern
ForRangeSource
BreakStatement
ContinueStatement
```

节点按源码顺序保留 `for` 或 `while`、固定左右括号、`var`/`const`、pattern、`in`、表达式、组成 `..` 的两个 Symbol、花括号、分号和全部 Trivia。`..` 不合并成虚构 Token。

缺少条件、`var`/`const` 绑定模式关键字、pattern、`in`、范围终点或循环体时，Parser 按议题 03 使用 `MissingToken` 和 `ErrorNode`。例如 `for (value in values) {}` 可以保留 `value` 并在它之前插入缺失的绑定模式关键字 Token。Parser 可以在循环体起始 `{`、结束 `}`、`in` 或后续明确语句起始处同步；真实的逗号、点号和分号不能被删除。`break` 或 `continue` 缺少分号时继续使用议题 08 的零宽度缺失 Token 恢复。

## 12. 确认结论

Ink 的普通循环包括 `while (expression) { ... }`、`while (match .variant(...) = expression) { ... }` 和 `for (binding_mode pattern in source) { ... }`。固定括号包围完整循环头，但不引入 C 风格三段式 `for`。所有循环体必须使用花括号，循环本身无值且不写结尾分号。普通 `for` 的绑定关键字不可省略，pattern 只接受一个名称或 `_`；`var`/`const` 控制顶层绑定，元素目标访问能力由迭代源产生的值、`T&` 或 `const T&` 决定。`start .. end` 只是在 `for` 头中的半开区间定界形式。`break;` 与 `continue;` 仅作用于最内层 `while_statement` 或普通 `for_statement`。`comptime` 可以直接修饰完整 `for_statement` 或 `while_statement`，且不改变 header 文法；module、语句和成员区域共享同一个控制节点，只替换 body 的 region item 类别。
