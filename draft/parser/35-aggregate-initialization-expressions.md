# Parser 议题 35：聚合初始化表达式

> 状态：已确认
> 确认日期：2026-08-05

## 1. 基本产生式

聚合初始化使用显式类型和命名字段初始化项：

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
    identifier, ":", expression ;
```

```ink
Point {
    x: 10,
    y: 20
}
```

类型不能省略，也不引入 `construct` 关键字。字段项只提供 `identifier: expression` 形式，不增加位置初始化、字段名简写、spread 或 update 语法。

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

## 3. 接入基础表达式

聚合初始化属于结构化基础表达式：

```ebnf
structured_expression =
      match_expression
    | aggregate_initialization_expression ;
```

因此它可以出现在任何普通表达式位置：

```ink
const point = Point { x: 10, y: 20 };
consume(Point { x: 10, y: 20 });
return Point { x: 10, y: 20 };
```

## 4. 直接后缀

`primary_expression` 已包含 `structured_expression`，所以完整聚合初始化结果可以直接作为现有后缀链的起点，不要求额外加括号：

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

## 6. 结论

聚合初始化是使用显式 `type` 和命名字段列表形成的基础表达式。初始化体允许为空，不接受尾随逗号；完整结果可以直接继续普通后缀链。
