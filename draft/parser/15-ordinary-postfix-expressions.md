# Parser 议题 15：普通后缀表达式

> 状态：已确认，议题 16、17 补充泛型实参和切片后缀  
> 确认日期：2026-08-03

## 1. 重复后缀结构

普通后缀表达式从一个基础表达式开始，并允许依次附加零个或多个调用、索引、成员访问或指针成员访问后缀：

```ebnf
postfix_expression =
    primary_expression, { ordinary_postfix_suffix } ;

ordinary_postfix_suffix =
      call_suffix
    | index_suffix
    | member_suffix
    | pointer_member_suffix ;
```

重复结构允许任意合法链式组合：

```ink
make_object().field
make_array()[index]
get_factory()(argument)
objects[index].method(argument).field
```

本议题中的“链式”仅指对前一表达式结果继续应用后缀，不是议题 11 已禁止的链式赋值。

## 2. 调用后缀

```ebnf
call_suffix = "(", [ argument_list ], ")" ;

argument_list =
    expression, { ",", expression } ;
```

调用可以没有实参，也可以包含一个以上位置实参：

```ink
run()
consume(value)
combine(first(), second(), third())
```

议题 07 的普通逗号列表规则适用，因此调用实参不允许尾随逗号：

```ink
consume(first, second,) // 非法
```

Ink v0 没有普通命名实参。默认实参的省略和补全继续遵守议题 65；它不改变调用后缀的源码实参列表结构。

## 3. 普通索引后缀

```ebnf
index_suffix = "[", expression, "]" ;
```

普通索引必须准确包含一个表达式：

```ink
array[index]
pointer[offset]
matrix[row][column]
```

空索引非法：

```ink
array[]
```

切片或子范围不是空索引的特殊解释。议题 17 已将 `[:]`、`[start:end]` 定义为独立的 `slice_suffix`，并作为另一种后缀加入完整文法。

## 4. 普通成员访问

```ebnf
member_suffix = ".", member_selector ;

member_selector = identifier | tuple_position_selector ;

tuple_position_selector =
    ? Unsuffixed decimal IntegerLiteral Token ? ;
```

普通具名成员使用 Identifier：

```ink
object.field
object.method
module.declaration
```

元组位置成员使用不带类型后缀的十进制整数 Token：

```ink
tuple.0
tuple.1
```

Parser 保留完整整数 Token；必须是规范位置拼写以及位置是否越界由语义检查决定。`.0` 不是浮点字面量的一部分，因为 Tokenizer 议题 05 已确认 `.5` 不构成 `FloatLiteral`。

## 5. 指针成员访问

```ebnf
pointer_member_suffix = "->", member_selector ;
```

`->` 显式表示通过原始指针访问目标成员：

```ink
pointer->field
pointer->method()
tuple_pointer->0
```

Ink 不把 `pointer.field` 自动解释为隐式解引用。`.` 和 `->` 的接收类型要求由类型检查决定；Parser 只建立不同的后缀节点。

Tokenizer 把 `-` 和 `>` 分别输出为单字符 Symbol Token。只有两个 Token 直接相邻时，Parser 才能识别 `->`：

```ink
pointer->field  // 合法的指针成员后缀
pointer- >field // 不是 "->"
```

## 6. 组合与结合方向

后缀操作具有议题 12 确认的最高表达式优先级，并从左到右逐层包裹前一结果：

```ink
objects[index].method(argument).field
```

概念结构为：

```text
MemberAccess(
    Call(
        MemberAccess(
            Index(objects, index),
            method
        ),
        argument
    ),
    field
)
```

调用结果可以再次调用，索引结果可以继续访问成员，成员也可以保存可调用值。Parser 不根据中间结果类型限制后缀组合；不合法的组合由类型检查拒绝。

## 7. 求值顺序

每个后缀按照议题 13 先计算已有基础，再计算当前后缀引入的子表达式：

```ink
make_container().items[next_index()].process(first(), second())
```

顺序为：

```text
make_container()
→ 选择 items
→ next_index()
→ 索引
→ 选择 process
→ first()
→ second()
→ 调用
```

任何较早步骤抛出异常时，尚未开始的后续步骤不执行。

## 8. 不提供可选链

Ink v0 不定义 `?.`、`?->` 或其他可选链后缀。原始指针需要显式检查 `null`，`Optional<T>` 等枚举值使用模式匹配或对应标准库 API 处理：

```ink
if pointer != null {
    pointer->process();
}
```

Tokenizer 仍会分别产生源码中的单字符 Symbol Token，但 Parser 不把这些序列识别成可选链。

## 9. 泛型与切片扩展

议题 16 已经定义显式泛型实例化 `<...>` 及其与比较 `<`、`>` 和移位 `>>` 的消歧规则。它与本议题的普通后缀共同构成完整后缀链。

议题 17 已经使用冒号定义切片后缀。它是不同于普通单表达式索引的另一种后缀，并共同扩展完整的 `postfix_suffix` 集合。

## 10. CST 与恢复

CST 应保存基础表达式以及有序后缀节点。每个调用括号、索引方括号、成员点、`->` 的两个单字符 Token、分隔逗号和全部 Trivia 均准确保留。

Parser 可以使用嵌套的 `CallExpression`、`IndexExpression`、`MemberExpression` 节点，也可以在 full-fidelity CST 中保存一个基础节点加有序 suffix 列表，再在 lowering 时建立嵌套 AST。两种表示必须产生相同结合方向和源码范围。

缺少实参、索引、成员名或右定界符时，按照议题 03 插入 `MissingToken` 或建立 `ErrorNode`，并保证游标取得进展。

## 11. 确认结论

Ink 的普通后缀表达式从基础表达式开始，可重复附加调用、单表达式索引、`.` 成员访问和 `->` 原始指针成员访问。后缀具有最高优先级并从左到右组合；调用参数不允许尾随逗号，索引不能为空，指针成员不能通过 `.` 隐式解引用，v0 不提供可选链。议题 16、17 分别以独立规则加入泛型实例化和切片。
