# Parser 议题 14：基础表达式

> 状态：已确认，议题 21、22 将无块 `if_expression` 放在完整表达式最低层  
> 确认日期：2026-08-03

## 1. 定义

基础表达式是能够独立产生值或编译期实体，并能够作为调用、索引、成员访问等后缀操作起点的表达式。

```ebnf
primary_expression =
      literal_expression
    | identifier_expression
    | builtin_type_expression
    | this_expression
    | parenthesized_expression
    | tuple_expression
    | array_expression
    | structured_expression ;
```

名称解析、类型和值类别判断不由 Parser 完成。Parser 只根据 Token 形状构造对应 CST。

## 2. 字面量表达式

```ebnf
literal_expression =
      ? IntegerLiteral Token ?
    | ? FloatLiteral Token ?
    | ? ScalarLiteral Token ?
    | ? StringLiteral Token ?
    | ? BoolLiteral Token ?
    | ? NullLiteral Token ? ;
```

这覆盖 Tokenizer 已确认的普通、Raw、多行字符串形式以及带类型后缀的数值字面量：

```ink
42
255u8
3.14f64
'中'
"hello"
r"raw"
true
false
null
```

Parser 不重新扫描转义、数字进制或后缀；这些信息由完整字面量 Token 提供。

## 3. 标识符表达式

```ebnf
identifier_expression = ? Identifier Token ? ;
```

标识符在语法阶段不区分变量、常量、函数、类型、泛型声明、模块或编译期声明值：

```ink
value
make_value
Buffer
network
```

同一个 `Identifier` CST 节点的具体含义由名称绑定和使用上下文决定。Parser 不根据命名风格或当前符号表改变表达式形状。

## 4. 内建类型表达式

```ebnf
builtin_type_expression = ? BuiltinType Token ? ;
```

`i32`、`bool`、`type` 等内建类型可以作为一等编译期 `type` 值出现：

```ink
reflect(i32)
Buffer<i32>
```

因此 Parser 允许 `BuiltinType` Token 出现在基础表达式位置。它能否在具体位置物化为运行时值属于后续编译期阶段和类型检查，不由语法限制。

## 5. `this` 表达式

```ebnf
this_expression = "this" ;
```

`this` 是一个硬关键字表达式：

```ink
this.field
this->field
```

它只在具有实例接收者的合法上下文中可用。普通函数、静态上下文或其他位置出现 `this` 时，Parser 仍可建立节点，再由语义检查报告非法上下文。

## 6. 加括号表达式

```ebnf
parenthesized_expression = "(", expression, ")" ;
```

圆括号显式改变默认优先级和结合方式：

```ink
(a + b) * c
```

`(value)` 只是加括号表达式，不是单元素元组。CST 必须保留左右括号和内部 Trivia；lowering 可以在不影响源码工具的 AST 中消除纯分组节点。

## 7. 元组表达式

```ebnf
tuple_expression =
      "(", ")"
    | "(", expression, ",", ")"
    | "(", expression, ",", expression,
      { ",", expression }, ")" ;
```

三种形状分别是：

```ink
()          // 空元组
(value,)    // 单元素元组
(left, right)
```

单元素元组必须使用唯一允许的尾随逗号来区别于 `(value)`。两个及以上元素的元组不能使用尾随逗号：

```ink
(left, right,) // 非法
```

元组元素按照议题 13 从左到右求值。

## 8. 数组表达式

```ebnf
array_expression =
    "[", [ expression, { ",", expression } ], "]" ;
```

数组表达式可以为空或包含一个以上元素：

```ink
[]
[value]
[first(), second(), third()]
```

普通逗号列表规则继续适用，因此数组表达式不允许尾随逗号：

```ink
[first, second,] // 非法
```

元素从左到右求值。空数组以及不同元素类型如何确定最终数组类型属于类型检查和初始化语义，不改变本节文法。

## 9. 结构化表达式

```ebnf
structured_expression = match_expression ;
```

`match_expression` 能在需要值的表达式位置出现：

```ink
let result = match input { /* arms */ };
```

其模式、分支、结果类型和完整 EBNF 由后续议题定义。将它列入基础表达式只确定它可以成为完整表达式或后缀操作的基础。

为提高可读性，`match_expression` 继续接后缀操作时建议使用括号，但语法不强制：

```ink
(match input { /* arms */ }).method()
```

议题 21 已独立确认无块形式 `if condition then true_expression else false_expression`，议题 22 将它放在完整表达式最低层。它不属于最高优先级的 `primary_expression`；作为其他运算的操作数或后缀基础时必须先使用圆括号。

## 10. 与后缀表达式的关系

所有基础表达式都可以作为后续调用、索引或成员访问的起点：

```ink
make_object().field
get_array()[index]
(left, right).0
"text".length
```

后缀操作的完整组合顺序和 EBNF 由下一议题定义。本节不把 `.`、`[]` 或 `()` 合并进基础表达式自身。

## 11. CST 与恢复

建议为以下语法形状建立不同 CST 节点：

```text
LiteralExpression
IdentifierExpression
BuiltinTypeExpression
ThisExpression
ParenthesizedExpression
TupleExpression
ArrayExpression
```

`match` 使用自己的结构化节点；议题 21、22 的 `if_expression` 使用独立的最低优先级 `IfExpression` 节点，不归入基础表达式。`ParenthesizedExpression` 与单元素 `TupleExpression` 必须保持不同节点类别，不能只通过后续类型推断区分。

缺少表达式、逗号或右定界符时，Parser 按议题 03 使用 `MissingToken` 和 `ErrorNode` 恢复。所有实际 Token 和 Trivia 仍准确保留一次。

## 12. 确认结论

Ink 的基础表达式包括字面量、标识符、内建类型、`this`、加括号表达式、元组、数组以及 `match` 结构化表达式。类型作为一等编译期值在语法上允许出现在表达式位置；`()`、`(value)`、`(value,)` 和多元素元组具有明确不同的语法；普通数组和多元素元组均不允许尾随逗号。无块 `if_expression` 不是基础表达式，而是议题 22 定义的完整表达式最低层。
