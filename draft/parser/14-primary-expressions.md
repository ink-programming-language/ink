# Parser 议题 14：基础表达式

> 状态：已确认，议题 21、22 定义无块 `if_expression`，议题 24 完成 `match_expression`，议题 29 补充元组列表展开，议题 30 补充复合类型值并把直接函数类型改为封闭表达式，议题 35 加入 postfix 聚合初始化，议题 40 加入复用普通 class 结构的类型表达式；2026-08-08 同步 statement entry 的 `match` 上下文分派
> 确认日期：2026-08-03

## 1. 定义

可后缀基础表达式能够独立产生值或编译期实体，并能够作为调用、索引、成员访问等后缀操作起点。直接写出的函数类型是独立的封闭 postfix expression；要对整个函数类型继续应用后缀，必须先加括号。

```ebnf
postfixable_primary_expression =
      literal_expression
    | identifier_expression
    | builtin_type_expression
    | this_expression
    | parenthesized_expression
    | parenthesized_comma_list
    | array_expression
    | structured_expression
    | const_type_value_expression ;

direct_function_type_expression =
    [ "const" ], function_type_expression ;
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

普通值表达式不能由 `.` 开始。枚举值必须通过现有成员后缀写出类型限定名：

```ink
BuildMode.debug
Optional::<int>.some(10)
```

因此 `build.mode == .debug` 和值位置的 `.some(10)` 都不是合法表达式。前导点只属于议题 23 的上下文枚举分支模式，例如 `match (mode) { .debug => ... }`；模式不产生枚举值，也不会向 `postfixable_primary_expression` 增加分支。

## 4. 内建类型表达式

```ebnf
builtin_type_expression = ? BuiltinType Token ? ;
```

`i32`、`bool`、`type` 等内建类型可以作为一等编译期 `type` 值出现：

```ink
reflect(i32)
Buffer::<i32>
```

因此 Parser 允许 `BuiltinType` Token 出现在基础表达式位置。它能否在具体位置物化为运行时值属于后续编译期阶段和类型检查，不由语法限制。

### 4.1 具有独特开头的类型值

议题 30 不把完整 `type` 作为另一个基础表达式备选，只加入普通表达式原先没有的两个确定入口：

```ebnf
const_type_value_expression =
    "const", postfixable_type_primary ;

function_type_expression =
    function_type ;
```

例如：

```ink
const Data*
func(i32) -> bool
async func(Path&) -> Data
```

表达式中的 `const` 只表示前置类型限定且最多出现一次；它不是通用一元运算符。函数类型直接复用议题 29 的完整参数和结果类型文法，但直接函数类型不进入普通 postfix 循环；`(func(i32) -> bool)*` 才能让 `*` 作用于整个函数类型。`*`、`&` 的表达式消歧以及空 `[]` 由议题 30 的后缀规则完成。

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

## 7. 圆括号逗号列表

```ebnf
parenthesized_comma_list =
      "(", ")"
    | "(", expression, ",", ")"
    | "(", parenthesized_comma_item, ",", parenthesized_comma_item,
      { ",", parenthesized_comma_item }, ")"
    | "(", list_expansion, ")" ;

parenthesized_comma_item =
      expression
    | list_expansion ;

list_expansion =
    "...", expression ;
```

三种形状分别是：

```ink
()          // 空元组
(value,)    // 单元素元组
(left, right)
(...values) // 展开运行时参数包形成元组
```

单元素元组必须使用唯一允许的尾随逗号来区别于 `(value)`。两个及以上元素的元组不能使用尾随逗号：

```ink
(left, right,) // 非法
```

单独的 `list_expansion` 不需要尾随逗号，因为 `...` 已经明确选择元组结构。它也可以和普通元素组合：

```ink
(header, ...values, footer)
```

展开是不是合法运行时参数包以及展开后的准确元素类型由议题 68、69 的语义规则检查。Parser 先验证源码列表结构，因此空展开不会让前导、连续或尾随逗号变成合法。元组的普通元素和展开结果按照议题 13 的源码顺序求值并构造。

议题 30 规定该 Token 形状在普通表达式和明确类型入口中共享中性 `ParenthesizedCommaList` CST。期望结果为 `type` 时，语义 elaboration 把它解释为元组类型；期望普通元组或没有期望类型时，它是元组值。Parser 不根据元素是否碰巧都是类型名称选择节点种类。

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
structured_expression =
      match_expression
    | class_type_expression ;
```

`match_expression` 能在需要值的表达式位置出现：

```ink
const result = match (optional) {
    .none => fallback,
    .some(value) => value,
};
```

议题 24 定义其完整 EBNF：正常完成的分支产生表达式值并以逗号结束，不正常完成的分支可以使用同样以逗号结束的 `statement_block`。将它列入基础表达式确定整个结构可以成为完整表达式或后缀操作的基础。

Parser 议题 35 的聚合初始化不再属于结构化基础表达式，而是在普通 postfix 层用真实 `{` 包装已经完成的当前操作数：

```ink
const x = Point { x: 10, y: 20 }.x;
```

Parser 消费聚合体后仍可继续普通后缀链，因此完整聚合初始化结果不要求先增加额外圆括号。把它放在 postfix 层还保证 `{` 只绑定紧邻左侧操作数，不能跨过中缀运算符形成另一棵 CST。

Parser 议题 40 的 `class_type_expression` 复用普通 class 的声明前缀、泛型参数、继承列表和成员块，只在表达式上下文把类名改为可选：

```ink
const Generated: type = class {
    var value: i32;
};

return class Node {
    var next: Node*;
};
```

它产生编译期类型值，不在外层声明区域建立类名。带局部名称的形式只为类体内部的递归引用提供名称。

在调用者已经要求表达式的上下文中，`match_expression` 继续接后缀操作时只为提高可读性而建议使用括号，后缀语法本身不强制：

```ink
(match (optional) {
    .none => fallback,
    .some(value) => value,
}).method()
```

如果完整后缀表达式直接位于 statement entry，裸 `match` 已为 `MatchStatement` 保留，因此必须像上例一样把整个 `match_expression` 括起来；这项限制来自议题 18 的 `statement_expression` 起点守卫，不改变表达式内部的 postfix 产生式。

议题 21 已独立确认无块形式 `if (condition) true_expression else false_expression`，议题 22 将它放在完整表达式最低层。它不属于最高优先级的 `postfixable_primary_expression`；作为其他运算的操作数或后缀基础时必须先使用圆括号。

## 10. 与后缀表达式的关系

所有可后缀基础表达式都可以作为后续调用、索引或成员访问的起点：

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
ParenthesizedCommaList
ArrayExpression
ListExpansion
ConstTypeValueExpression
FunctionTypeExpression
ClassTypeExpression
```

`match` 使用议题 24 的 `MatchExpression` 结构化节点，class 类型表达式使用议题 40 的 `ClassTypeExpression`；聚合初始化由议题 35 的 postfix accumulator 建立 `AggregateInitializationExpression`。议题 21、22 的 `if_expression` 使用独立的最低优先级 `IfExpression` 节点，不归入基础表达式。`ParenthesizedExpression` 与带逗号的 `ParenthesizedCommaList` 必须保持不同节点类别；后者自身作为元组类型还是元组值才由期望类型决定。

缺少表达式、逗号或右定界符时，Parser 按议题 03 使用 `MissingToken` 和 `ErrorNode` 恢复。所有实际 Token 和 Trivia 仍准确保留一次。

## 12. 确认结论

Ink 的可后缀基础表达式包括字面量、标识符、内建类型、`this`、加括号表达式、圆括号逗号列表、数组、非函数 `const` 类型值、class 类型表达式以及其他结构化表达式；同步或异步的直接函数类型是封闭 postfix expression，必须加括号后才能对整个类型继续应用后缀。类型作为一等编译期值在语法上允许出现在表达式位置；`()`、`(value)`、`(value,)`、`(...values)` 和多元素元组具有明确不同的 Token 形状，圆括号逗号列表的类型或值解释由议题 30 的期望类型规则决定。普通数组和多元素元组均不允许尾随逗号。`...expression` 只作为议题 29 确认的列表展开节点出现。无块 `if_expression` 不是基础表达式，而是议题 22 定义的完整表达式最低层。
