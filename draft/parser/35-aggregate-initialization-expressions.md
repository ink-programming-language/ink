# Parser 议题 35：聚合初始化表达式

> 状态：已确认；Parser 议题 40 同步扩展 `structured_expression`；2026-08-08 增加命名字段简写
> 确认日期：2026-08-05

## 1. 基本产生式

聚合初始化使用显式类型和命名字段初始化项；字段项可以显式给出值，也可以使用同名简写：

```ebnf
aggregate_initialization_expression =
    type, aggregate_initializer_body ;

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

类型不能省略，也不引入 `construct` 关键字。`x` 这样的 `aggregate_field_shorthand` 在语义上等价于 `x: x`：字段名和用作初始值的名称均取自同一个源码 Identifier Token。这不增加位置初始化、spread 或 update 语法。

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

## 3. 接入基础表达式

聚合初始化属于结构化基础表达式：

```ebnf
structured_expression =
      match_expression
    | aggregate_initialization_expression
    | class_type_expression ;
```

因此它可以出现在任何普通表达式位置：

```ink
const point = Point { x: 10, y: 20 };
consume(Point { x: 10, y: 20 });
return Point { x: 10, y: 20 };
```

## 4. 直接后缀

`postfixable_primary_expression` 已包含 `structured_expression`，所以完整聚合初始化结果可以直接作为现有后缀链的起点，不要求额外加括号：

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

因此控制条件后的语句块或分支块具有固定边界，不会与 `type { ... }` 的聚合初始化体共用同一无括号控制头形状。

## 6. CST 与恢复

CST 分别保留显式字段项和简写项：

```text
AggregateInitializationExpression
AggregateInitializerBody
AggregateFieldInitializer
AggregateFieldShorthand
```

`aggregate_field_initializer` 中的可选部分存在时，Parser 建立 `AggregateFieldInitializer`，保存字段名、真实冒号和完整表达式；可选部分缺席时建立 `AggregateFieldShorthand`，只保存源码中唯一的 Identifier Token 及其 Trivia。Parser 不伪造冒号、表达式节点或第二个标识符。简写的同名展开仅存在于语义分析中。

显式项在冒号后缺少表达式时，Parser 按议题 03 建立缺失表达式并向下一个逗号或结束花括号恢复。非空列表的最后一项后出现逗号时，该真实 Token 必须保留在 `ErrorNode` 中，不能静默接受为尾随逗号。

## 7. 结论

聚合初始化是使用显式 `type` 和命名字段列表形成的基础表达式。字段可以写为 `identifier: expression` 或保留独立 CST 的同名 `identifier` 简写。初始化体允许为空，不接受尾随逗号；完整结果可以直接继续普通后缀链。
