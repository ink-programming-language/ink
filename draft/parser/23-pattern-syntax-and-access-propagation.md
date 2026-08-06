# Parser 议题 23：模式语法与访问能力传播

> 状态：已确认，Parser 议题 10、24、25 已接入解构、`match`、循环条件与普通 `for` 绑定
> 确认日期：2026-08-04

## 1. v0 模式集合

Ink v0 的基础模式只包含名称绑定、通配、元组和枚举分支：

```ebnf
pattern =
      variant_pattern
    | payload_pattern ;

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

variant_pattern =
    ".", identifier,
    [ "(", payload_pattern,
      { ",", payload_pattern }, ")" ] ;
```

本议题不加入字面量、范围、`|`、字段、数组、guard 或用户定义解构模式，也不允许 `variant_pattern` 递归出现在另一个分支载荷模式中。嵌套元组模式仍然可以递归出现。

## 2. 名称绑定与通配

普通名称模式在所在语法上下文中建立一个局部绑定：

```ink
.some(value)
.point(x, y)
```

单独的 `_` 是通配模式，匹配对应位置但不建立名称：

```ink
.some(_)
.point(x, _)
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

## 4. 枚举分支模式

无载荷分支只写上下文限定的分支名：

```ink
.none
```

存在载荷括号时至少包含一个载荷模式：

```ink
.some(value)
.point(x, y)
.entry(name, _)
```

因此 `.none()` 不符合本议题的产生式。一个元组载荷与多个独立载荷在语法上保持不同：

```ink
.pair((left, right)) // 一个二元素元组载荷
.pair(left, right)   // 两个独立载荷
```

分支是否存在、载荷数量是否匹配以及载荷类型是否支持对应模式属于语义检查，不由 Parser 查询枚举声明决定。

这里的前导点只存在于模式上下文，不是普通值表达式简写。构造或引用枚举值时必须写出类型限定名，例如 `Color.red`、`Optional<int>.some(value)`；普通表达式不能写 `.red` 或 `.some(value)`。

## 5. 顶层模式上下文

`if (match ...)` 和议题 25 的 `while (match ...)` 顶层模式必须是 `variant_pattern`：

```ebnf
conditional_match_pattern = variant_pattern ;
```

```ink
if (match .some(value) = optional) {
    use(value);
}
```

这样不会接受永远成功的 `if (match value = expression)` 或 `while (match value = expression)`。

议题 24 的 `match` 顶层分支模式只接受枚举分支或通配：

```ebnf
match_arm_pattern =
      variant_pattern
    | wildcard_pattern ;
```

裸 Identifier 不会在 `match` 顶层意外成为全匹配绑定；剩余分支必须显式写 `_`。

名称、通配和元组组成的不可反驳模式为普通解构声明提供基础形状：

```ebnf
irrefutable_pattern = payload_pattern ;
```

议题 10 在 `var` 或 `const` 后接受顶层 `tuple_pattern`，并继续以普通 `identifier` 处理非解构名称声明。元组内部可以递归使用名称、`_` 和嵌套元组；顶层 `variant_pattern` 不可用于普通声明。

议题 25 的普通 `for` 在显式 `var` 或 `const` 后只接受单个 `binding_pattern` 或 `wildcard_pattern`。关键字属于整个循环绑定，不进入 pattern 内部；v0 的普通 `for` 暂不接受 `tuple_pattern`。

## 6. 匹配绑定始终借用载荷

`if (match ...)` 和 `match (...)` 中的名称模式不会复制、移动或取得枚举载荷所有权。绑定是指向活动载荷存储的非拥有引用，其生命周期不会超过被匹配对象实际存储的生命周期。

不在 pattern 中使用 `var` 修饰符，也不建立 `MutableBindingPattern`：

```ink
.some(var value) // 非法
```

普通声明中的 `var value = expression;` 与 `var (left, right) = expression;` 创建可变绑定，`const` 形式创建不可重新赋值的绑定；两种关键字都只能出现在整个声明的起始位置，不是单个 pattern 元素的修饰符，也都不同于匹配产生的非拥有引用。

## 7. 访问能力从被匹配对象传播

载荷绑定是否为 `T&` 或 `const T&`，由被匹配表达式对枚举存储的访问能力决定，而不是由 pattern 关键字决定：

```ink
func inspect(optional: const Optional<Data>&) {
    if (match .some(value) = optional) {
        // value: const Data&
        value.inspect();
    }
}

func update(optional: Optional<Data>&) {
    if (match .some(value) = optional) {
        // value: Data&
        value.update();
    }
}
```

规则为：

- 可写枚举 place，包括通过 `Enum&` 获得的对象，产生可写载荷绑定；
- 只读枚举 place，包括通过 `const Enum&` 获得的对象，产生只读载荷绑定；
- 按值临时匹配对象的载荷只允许只读绑定；
- Parser 只建立模式 CST，不计算 place 类别或引用类型。

这与普通字段和索引访问一致：是否可写由到达目标存储的访问路径决定，不需要在每个成员或模式名称前重复可变性修饰符。

## 8. 引用和指针载荷

访问能力传播只约束枚举中的载荷槽，不递归削弱载荷本身携带的访问能力：

- `Optional<T&>` 的绑定保持为 `T&`，不会形成引用的引用；
- `Optional<const T&>` 的绑定保持为 `const T&`；
- 对指针载荷只读访问时不能改写枚举中保存的指针值，但指针目标是否可写仍由 `T*` 或 `const T*` 决定；
- 对可写指针载荷槽的引用允许按照普通赋值规则改写该槽。

模式不会通过访问能力传播去掉底层目标已有的 `const`，也不会延长引用或指针目标的生命周期。

## 9. CST 与错误恢复

CST 使用独立节点保存：

- `BindingPattern`；
- `WildcardPattern`；
- `TuplePattern`；
- `VariantPattern`。

所有点号、括号、逗号、Identifier 和 Trivia 都按源码顺序准确保存一次。不存在 `MutableBindingPattern`。

缺少模式、逗号或右括号时，Parser 按议题 03 使用 `MissingToken` 和 `ErrorNode` 恢复。枚举载荷模式可以在 `,`、`)`、`=`、连续 `=>` 或后续语句块起始 `{` 等外层边界同步，但不能删除真实 Token。

## 10. 确认结论

Ink v0 的模式由名称绑定、`_` 通配、递归元组和上下文限定的枚举分支组成。普通 `var`/`const` 声明接受不可反驳的顶层元组模式；普通 `for` 必须显式写 `var`/`const`，随后只接受单个名称或 `_`。`if (match ...)` 与 `while (match ...)` 顶层只接受枚举分支模式，`match (...)` 顶层接受枚举分支或 `_`。匹配绑定始终借用载荷，不在 pattern 内使用 `var` 或 `const`；载荷绑定的可写性由被匹配对象的 `Enum&`、`const Enum&` 或 place 访问能力传播。
