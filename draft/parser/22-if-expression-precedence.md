# Parser 议题 22：`if` 表达式优先级

> 状态：已确认；2026-08-05 统一要求条件括号；2026-08-06 移除 `then`
> 确认日期：2026-08-03

## 1. 位于表达式最低层

`if_expression` 位于普通表达式优先级的最低层：

```ebnf
expression =
      if_expression
    | logical_or_expression ;

if_expression =
    "if", "(", logical_or_expression, ")",
    expression, "else", expression ;
```

议题 12 定义的逻辑或表达式仍是普通中缀运算中的最低一层；本议题在它之外增加前缀条件结构。

## 2. 分支包含完整表达式

真分支和假分支都使用完整 `expression`：

```ink
const value = if (condition) a + b else c * d;
```

按照已有运算符优先级，它等价于：

```ink
const value = if (condition) (a + b) else (c * d);
```

条件右括号是条件与真分支的定界 Token；与当前 `if` 配对的 `else` 是真、假分支的定界 Token，不属于任一分支表达式。

## 3. 条件使用 `logical_or_expression`

条件位置直接接受从逻辑或层开始的普通表达式：

```ink
const value = if (ready && enabled || forced) first else second;
```

另一个无块 `if_expression` 不能不加括号直接成为条件。需要这种结构时使用普通加括号表达式：

```ink
const value =
    if ((if (choose_left) left_ready else right_ready))
    first
    else second;
```

括号中的完整 `expression` 可以再次是 `if_expression`。

## 4. 作为其他运算的操作数

由于 `if_expression` 低于所有中缀和后缀运算，它作为其他运算的操作数时必须使用括号：

```ink
const total = value + (if (condition) a else b);
const field = (if (condition) first else second).field;
const item = (if (condition) left else right)[index];
```

以下写法不会把条件结构自动提升为加法右操作数：

```ink
value + if (condition) a else b
```

Parser 不为 `if` 提供中缀运算内部的特殊插入规则。

## 5. 假分支自然形成条件链

假分支是完整 `expression`，因此可以直接包含另一个 `if_expression`：

```ink
const value =
    if (first) a
    else if (second) b
    else c;
```

其结构为：

```text
if (first) a else (if (second) b else c)
```

不需要单独的 `else_if_expression` 节点。CST 中外层 `IfExpression` 的假分支直接包含下一层 `IfExpression`。

## 6. 真分支嵌套

真分支同样接受完整 `expression`：

```ink
const value =
    if (outer)
    if (inner) a else b
    else c;
```

`else` 配对由递归产生式确定：内层 `if` 消费自己的 `else b`，之后的 `else c` 与外层 `if` 配对。为了提高阅读清晰度可以加括号，但语法不强制。

## 7. 与普通 `if_statement` 的区分

在语句位置遇到 `if` 后，Parser 根据条件之后的结构区分两种节点：

```text
if (condition) { ... }       → IfStatement
if (condition) a else b      → IfExpression
```

当第二种形式作为表达式语句使用时，仍须按照议题 18 在末尾写分号：

```ink
if (condition) perform_a() else perform_b();
```

条件右括号后的 `{` 只开始 `if_statement` 的 `statement_block`；其他合法表达式起点进入 `if_expression` 的真分支，并要求后续 `else`。该分界不需要类型信息参与消歧。

## 8. 确认结论

Ink 的 `if_expression` 位于整个表达式优先级的最低层。条件使用 `logical_or_expression`，真、假分支使用完整 `expression`；假分支嵌套自然形成右侧条件链。条件表达式作为中缀运算的操作数或后缀操作的基础时必须加括号。
