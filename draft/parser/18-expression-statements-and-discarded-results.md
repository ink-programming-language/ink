# Parser 议题 18：表达式语句与丢弃结果

> 状态：已确认，议题 24—26 完成 `match`、循环、`defer` 与表达式语句的边界；2026-08-08 排除以声明关键字或保留 `match` 结构起点直接开始的 statement expression
> 确认日期：2026-08-03

## 1. 表达式语句

议题 08 已经确认表达式语句使用显式分号：

```ebnf
expression_statement = statement_expression, ";" ;

statement_expression =
    ? next significant Token is neither Keyword(Var), Keyword(Const), nor Keyword(Match), and the next two significant Tokens are not Keyword(Comptime) followed by Keyword(Match) ?,
    expression ;
```

除直接以 `var`、`const`、`match` 开始的形状，以及以显著 Token 序列 `comptime match` 开始的形状外，任何能够按照表达式文法完成解析的表达式都可以在允许普通语句的位置形成表达式语句：

```ink
log("started");
container.insert(value);
make_temporary();
```

Parser 不根据表达式的结果类型、纯度或副作用判断它能否成为表达式语句。

该入口限制只负责确定性语法分派，不做名称或类型查询。`var` 不能开始普通表达式；`const` 在其他表达式位置仍可开始前置限定类型值，但在 statement 起点固定保留给绑定声明。确实需要丢弃这一类型值时必须先分组，例如 `(const Data*);`。声明解析一旦由关键字选中，即使声明残缺也不回退到表达式语句。

同理，语句入口的裸 `match` 固定保留给 `MatchStatement`，显著 Token 序列 `comptime match` 固定保留给 `ComptimeMatchControl`。这个守卫只检查整个 statement expression 的外层起点，不会递归屏蔽已经进入表达式上下文后的 `match`，也不会屏蔽 `MatchExpression` arm 中合法的 `const` 类型值。圆括号、初始化器、`return` 操作数和调用实参等位置都能先建立明确的表达式上下文。

## 2. 允许丢弃非 `void` 结果

表达式语句会完整计算表达式。表达式产生非 `void` 结果但没有其他消费者时，该结果被丢弃：

```ink
container.insert(value); // 即使返回 bool，也允许直接忽略
calculate();             // 即使返回整数，也形成合法表达式语句
```

丢弃结果不等于跳过表达式求值。函数调用、副作用、边界检查、trap、异常以及其他语言规定的可观察行为仍必须发生。

编译器不能仅因最终值未使用，就删除一个可能产生副作用、异常、trap 或编译期效果的表达式。

## 3. `void` 表达式

结果为 `void` 的表达式没有待丢弃的值，但使用完全相同的表达式语句语法：

```ink
log("finished");
```

Parser 不需要为 `void` 调用建立另一种语句节点。结果是否为 `void` 在类型检查后才确定。

## 4. 完整表达式和临时对象

表达式语句中的未消费临时结果活到该完整表达式结束，并在结尾分号处按照普通 RAII 规则销毁：

```ink
make_resource();
continue_work();
```

概念顺序为：

```text
调用 make_resource()
→ 成功构造返回临时对象
→ 到达表达式语句分号
→ 销毁该临时对象
→ 调用 continue_work()
```

如果同一表达式产生多个需要清理的临时对象，它们按照成功构造顺序的逆序销毁。较早求值抛出异常时，只清理已经成功构造的临时对象，尚未开始的子表达式不执行。

返回值优化或直接构造可以消除不具有独立语义身份的中间临时对象，但不能把表达式语句最终丢弃结果的析构推迟到后续语句或外层作用域结束。

## 5. 明显无效果的表达式

以下形式语法上仍然是表达式语句：

```ink
42;
value;
a + b;
object.field;
```

它们是否应作为明显无效果代码产生 warning 或 error，不属于 Parser，也不在本议题中确定。该策略连同诊断编号、严重级别和抑制方式留给未来独立的 diagnostics draft。

优化器可以删除能够证明没有任何可观察效果的计算，但这种优化不改变源码在语法上是合法表达式语句。

## 6. 不增加显式丢弃语法

Ink v0 不增加 `discard` 关键字，也不把 `_ = expression;` 定义成特殊丢弃语句：

```ink
discard calculate(); // 没有这种关键字语句
_ = calculate();     // 不是内建丢弃形式
```

按照 Tokenizer 议题 03，`_` 词法上仍是普通 Identifier。它只在具体模式语法明确规定时作为忽略占位符；本议题不把它推广成赋值目标或通用结果黑洞。

调用者希望忽略普通结果时，直接使用表达式语句即可。

## 7. 专用类型规则

某些类型可以具有独立于通用表达式语句的使用检查。例如既有异步规则允许对明显创建后立即丢弃的惰性 `Task::<T>` 产生专用 warning。

这类规则不禁止 Parser 建立表达式语句，也不改变通用的结果丢弃和临时对象析构边界。未来如果引入通用 must-use 契约，应作为单独语义与 diagnostics 议题设计，而不能悄悄改变本节文法。

## 8. 结构化语句边界

本议题不改变以花括号自行结束的专用语句。议题 24 的 `match_statement` 由自己的 `}` 结束，不需要结尾分号。

议题 25 的 `while_statement` 和 `for_statement` 同样由循环体的 `}` 结束，并且不是可以丢弃结果的表达式；在循环后增加分号会形成非法空语句 Token。

议题 26 的 `defer expression;` 虽然包含表达式并丢弃其最终结果，但它是独立 `DeferStatement`：表达式在作用域清理时才求值，不是在注册位置立即执行的普通 `ExpressionStatement`。

同一议题的 `defer { ... }` 不包含待丢弃结果，是由 `StatementBlock` 的 `}` 自行结束的结构化语句，后面不写分号。

如果要把整个 `match_expression` 作为表达式语句使用并丢弃其结果，必须先用括号使 statement entry 不再直接以 `match` 开始；外层仍以 `;` 结束：

```ink
(match (optional) {
    .none => 0,
    .some(value) => value,
});
```

在初始化器等已经要求表达式的位置不需要这层消歧括号，arm 中也可以合法使用以 `const` 开始的类型值：

```ink
const selected: type = match (mode) {
    .readonly => const Data*,
    _ => Data*,
};
```

分支逗号只验证已经选定的 `MatchExpression`，不参与选择节点种类。未加外层括号的 `match (optional) { ... };` 在语句入口始终先成为 `MatchStatement`，其中的真实逗号和结构后的分号都是语法错误，Parser 不得回退重建为表达式语句。

## 9. CST 与恢复

`ExpressionStatement` CST 保存完整表达式、结尾 `Symbol(';')` 和全部 Trivia。结果类型和“结果已丢弃”不是额外源码 Token，也不要求 Parser 插入特殊 CST 子节点。

缺少分号时继续采用议题 08、03 的 `MissingToken(';')` 恢复。Parser 不能因为表达式看起来无效果就删除节点或跳过真实 Token。

## 10. 确认结论

Ink 允许未命中声明或保留结构起点守卫的语法正确表达式形成以分号结束的表达式语句，并允许直接丢弃非 `void` 结果；以 `const` 开始的类型值和作为完整表达式语句的 `match_expression` 需要先加括号，裸 `match` 与 `comptime match` 在语句入口固定提交到结构控制。普通表达式语句立即完整求值，未消费临时结果在分号处析构；`defer expression;` 使用独立节点和延迟求值时点，`defer { ... }` 则延迟执行完整语句块。v0 不增加 `discard` 或 `_ =` 特殊语法；明显无效果表达式和专用 must-use 检查留给 diagnostics 与后续语义议题。
