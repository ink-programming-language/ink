# Parser 议题 23：模式语法

> 状态：已确认，Parser 议题 10、25 已接入解构与普通 `for` 绑定
> 确认日期：2026-08-04

## 1. v0 模式集合

Ink v0 的基础模式只包含名称绑定、通配和元组：

```ebnf
pattern =
    payload_pattern ;

payload_pattern =
      binding_pattern
    | wildcard_pattern
    | tuple_pattern ;

binding_pattern =
    ? Identifier Token whose spelling is not "_" ? ;

wildcard_pattern =
    ? Identifier Token whose spelling is exactly "_" ? ;

tuple_pattern =
      "(", ")"
    | "(", payload_pattern, ",", ")"
    | "(", payload_pattern, ",", payload_pattern,
      { ",", payload_pattern }, ")" ;
```

本议题不加入字面量、范围、`|`、字段、数组或用户定义解构模式。嵌套元组模式可以递归出现。

## 2. 名称绑定与通配

普通名称模式在所在语法上下文中建立一个局部绑定：

```ink
const (left, right) = pair;
for (const value in values) {}
```

单独的 `_` 是通配模式，匹配对应位置但不建立名称：

```ink
const (first, _) = pair;
for (const _ in values) {}
```

Tokenizer 仍按照既有规则为 `_` 产生普通 `Identifier` Token。Parser 只在模式位置检查其准确拼写并建立 `WildcardPattern`；`_value`、`__internal` 等其他 Identifier 仍是普通 `BindingPattern`。

## 3. 元组模式

元组模式与元组表达式使用相同的形状消歧：

```ink
()                      // 空元组模式
(value,)                // 单元素元组模式
(left, right)           // 多元素元组模式
(header, (left, right)) // 嵌套元组模式
```

`(value)` 不是分组模式，两个及以上元素的元组模式也不允许尾随逗号：

```ink
(value)        // 非法
(left, right,) // 非法
```

这与议题 07 的逗号规则和议题 14 的元组表达式形状保持一致。

## 4. 模式使用上下文

名称、通配和元组组成的不可反驳模式为普通解构声明提供基础形状：

```ebnf
irrefutable_pattern = payload_pattern ;
```

议题 10 在 `var` 或 `const` 后接受顶层 `tuple_pattern`，并继续以普通 `identifier` 处理非解构名称声明。元组内部可以递归使用名称、`_` 和嵌套元组。

议题 25 的普通 `for` 在显式 `var` 或 `const` 后只接受单个 `binding_pattern` 或 `wildcard_pattern`。关键字属于整个循环绑定，不进入 pattern 内部；v0 的普通 `for` 暂不接受 `tuple_pattern`。

## 9. CST 与错误恢复

CST 使用独立节点保存：

- `BindingPattern`；
- `WildcardPattern`；
- `TuplePattern`。

所有括号、逗号、Identifier 和 Trivia 都按源码顺序准确保存一次。

缺少模式、逗号或右括号时，Parser 按议题 03 使用 `MissingToken` 和 `ErrorNode` 恢复，并在调用者提供的 `,`、`)`、`=`、`in` 或语句边界同步，但不能删除真实 Token。

## 10. 确认结论

Ink v0 的模式由名称绑定、`_` 通配和递归元组组成。普通 `var`/`const` 声明接受不可反驳的顶层元组模式；普通 `for` 必须显式写 `var`/`const`，随后只接受单个名称或 `_`。声明关键字属于整个绑定结构，不进入单个 pattern 元素。
