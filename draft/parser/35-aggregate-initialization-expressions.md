# Parser 议题 35：聚合初始化表达式

> 状态：已确认；2026-08-08 增加命名字段简写，并把聚合初始化改为普通表达式专用 postfix 后缀以消除完整 `type` 与中缀表达式的重叠
> 确认日期：2026-08-05

## 1. 基本产生式

聚合初始化对已经按照普通表达式规则解析完成的当前 postfix 操作数应用命名字段初始化体；字段项可以显式给出值，也可以使用同名简写：

```ebnf
postfix_expression =
      direct_function_type_expression
    | postfixable_primary_expression,
      { expression_postfix_suffix },
      terminal_type_constructor_tail_decision ;

expression_postfix_suffix =
      postfix_suffix
    | aggregate_initialization_suffix ;

aggregate_initialization_suffix =
    aggregate_initializer_body ;

aggregate_initializer_body =
    "{",
    [
        aggregate_field_initializer,
        { ",", aggregate_field_initializer }
    ],
    "}" ;

aggregate_field_initializer =
    identifier, [ ":", expression ] ;
```

```ink
Point {
    x,
    y: 20
}
```

左侧操作数不能省略，也不引入 `construct` 关键字。Parser 不使用完整 `type` 非终结符重新解析左侧，也不查询它是否表示类型；语义分析要求左侧表达式在编译期准确产生支持命名聚合初始化的 `type`。`x` 这样的 `aggregate_field_shorthand` 在语义上等价于 `x: x`：字段名和用作初始值的名称均取自同一个源码 Identifier Token。这不增加位置初始化、spread 或 update 语法。

## 2. 空初始化体与逗号

初始化体可以为空：

```ink
Empty {}
```

非空初始化体使用不接受尾随逗号的普通逗号列表：

```ink
Point {
    x: 10,
    y: 20,
}
```

上例不属于该文法。

显式字段项和简写项共享同一普通列表，可以任意交错，但不会因简写而改变议题 07 的逗号规则：

```ink
Point { x, y: 20 }  // 合法
Point { x, y: 20, } // 非法：尾随逗号
```

## 3. Postfix 绑定与唯一解析

聚合初始化不是 `structured_expression` 的基础分支，而是只存在于普通表达式 `expression_postfix_suffix` 循环中的后缀。共享的 `postfix_suffix` 本身保持不变，因为它还会被明确 `type` 文法复用；把聚合后缀加入共享集合会让类型 Parser 错把声明体或其他花括号块吞入类型。末尾的 `terminal_type_constructor_tail_decision` 由议题 30 定义为正尾链与零宽度否定守卫的互斥选择，不是无条件可省略的尾链。

```ebnf
structured_expression =
      match_expression
    | class_type_expression ;
```

聚合后缀与调用、索引和成员访问处于同一最高绑定层，只包装紧邻左侧已经完成的 postfix 操作数，不能跨过中缀运算符抢占更大的表达式：

```ink
T * (U) {}       // T * AggregateInitialization((U), {})
(T * (U)) {}     // AggregateInitialization((T * (U)), {})
A + B { value }  // A + AggregateInitialization(B, { value })
(A + B) { value } // AggregateInitialization((A + B), { value })
```

这条统一优先级规则覆盖所有中缀运算，不需要为 `*`、`&` 或其他运算符逐个规定聚合初始化特例。特别是第一行继续满足 Parser 议题 30 已确认的 `T*(value)` 乘法解释，同一 Token 流不再能由完整 `type` 分支吞成另一棵树。

普通类型值表达式可以直接作为左侧操作数：

```ink
const point = Point { x: 10, y: 20 };
consume(Point { x: 10, y: 20 });
return Point { x: 10, y: 20 };
return select_type() { value };
return class Box { var value: i32; } { value: 1 };
```

后缀循环不限制聚合初始化只能出现一次。前一次聚合的结果经过成员访问或调用后仍可能在编译期产生另一个 `type`，Parser 继续只保存中性结构并让语义阶段判断每个聚合目标：

```ink
Factory { config }.result_type() { value }
```

复杂中缀表达式若要整体计算类型，必须先分组。Parser 议题 30 的终止型 `*`、`&` 和空 `[]` 类型构造尾链之后继续施加操作时也必须先分组；聚合初始化不把 `{` 加入该尾链的 `EndSet`：

```ink
T* {}       // 语法错误：终止型类型尾链后不能直接继续聚合后缀
(T*) {}     // 语法成立；指针类型不支持聚合初始化时由语义分析拒绝
T*(U) {}    // T * AggregateInitialization((U), {})
(T*)(U) {}  // 对 T* 调用后，以调用结果作为聚合目标
```

直接函数类型继续是封闭的 `postfix_expression` 分支，因此同样需要先分组：

```ink
(func() -> Data) {}
```

## 4. 聚合结果的直接后缀

`expression_postfix_suffix` 循环允许完整聚合初始化结果直接继续现有后缀链，不要求额外加括号：

```ink
Point { x: 10, y: 20 }.x
Point { x: 10, y: 20 }.length()
Matrix { data: values }[row]
```

这些后缀继续复用 Parser 议题 15—17 的普通调用、索引、成员访问、泛型实参和切片产生式，不为聚合初始化建立专用版本。

## 5. 与控制结构的边界

Parser 议题 20、21、24、25 和 32 已统一要求控制头使用圆括号：

```ink
if (condition) {}
while (condition) {}
for (const item in items) {}
match (value) {}
```

因此控制条件后的语句块或分支块具有固定边界，不会与 `expression { ... }` 的聚合初始化后缀共用同一无括号控制头形状。

`aggregate_initialization_suffix` 只接入普通 `postfix_expression`，不进入被明确 `type` 和各类声明复用的 `postfix_suffix`。因此声明文法中紧随类型或初始化列表的花括号继续固定属于对应 body：

```ink
func make() -> Result {}
class Derived : Base {}
catch Error {}
func Derived() : Base() {}
```

上述花括号分别属于函数体、类成员块、处理器语句块和构造函数体，不会被前面的 `Result`、`Base`、`Error` 或 `Base()` 吞成聚合初始化。

## 6. CST 与恢复

Parser 在 postfix 循环中读到真实左花括号时，把当前 accumulator 原子包装为 `AggregateInitializationExpression`，然后解析 body 并继续后续普通后缀。CST 分别保留目标表达式、显式字段项和简写项：

```text
AggregateInitializationExpression
├─ TargetExpression
└─ AggregateInitializerBody
   └─ AggregateFieldInitializer | AggregateFieldShorthand
```

`TargetExpression` 位置保存消费左花括号前已经完成的普通表达式节点，不表示 Parser 使用 `type` 非终结符重新解析或改写它。实现也可以在 full-fidelity CST 中保存基础节点加有序 suffix 列表，再在 lowering 时建立相同的包装关系。

只有源码中真实存在的 `{` 才能提交聚合初始化。未看到 `{` 时没有聚合候选，不为试探合成缺失左花括号；一旦消费 `{`，字段或右花括号错误都只能在当前聚合体内恢复，不得回滚左侧表达式或改按语句块解释。

`aggregate_field_initializer` 中的可选部分存在时，Parser 建立 `AggregateFieldInitializer`，保存字段名、真实冒号和完整表达式；可选部分缺席时建立 `AggregateFieldShorthand`，只保存源码中唯一的 Identifier Token 及其 Trivia。Parser 不伪造冒号、表达式节点或第二个标识符。简写的同名展开仅存在于语义分析中。

显式项在冒号后缺少表达式时，Parser 以逗号、结束花括号和调用者的外层 StopSet 作为字段表达式的 `EndSet`，按议题 03 建立缺失表达式并恢复。初始化体的项级同步集合是逗号、结束花括号和调用者的外层 StopSet；非法字段起始 Token、前导逗号和连续逗号中的真实 Token 必须进入 `ErrorNode` 并保证游标前进，不能在 `ErrorNode` 与缺失字段两种树之间任意选择。

如果在调用者 StopSet 或批处理 EOF 前仍未看到 `}`，Parser 插入 `MissingToken('}')` 并把该外层 Token 留给调用者；已经消费的真实 `{` 保证当前 `AggregateInitializationExpression` 不回滚。交互式 REPL 在 EOF 处只有聚合体或其内部定界结构尚未闭合时返回 `Incomplete`，而不是立即固化缺失右花括号。

非空列表的最后一项后出现逗号时，该真实 Token 必须保留在 `ErrorNode` 中，不能静默接受为尾随逗号。

## 7. 结论

聚合初始化是对当前普通 postfix 操作数施加的专用后缀，语义阶段要求该操作数产生支持命名聚合初始化的 `type`。它不再把完整 `type` 作为重叠的基础表达式分支，因此中缀表达式只遵守统一优先级并形成唯一 CST。字段可以写为 `identifier: expression` 或保留独立 CST 的同名 `identifier` 简写；初始化体允许为空，不接受尾随逗号；完整结果可以直接继续普通后缀链。
