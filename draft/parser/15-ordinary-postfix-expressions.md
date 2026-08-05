# Parser 议题 15：普通后缀表达式

> 状态：已确认，议题 16、17 补充泛型实参和切片后缀；议题 29 补充列表展开；议题 30 补充类型值后缀和构造调用的中性解释；议题 35 允许聚合初始化直接作为后缀起点；议题 37 增加裸 `...` 全参数转发；2026-08-05 增加统一命名实参
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

议题 30 在完整 `postfix_expression` 外层进一步加入空 `[]` 以及受当前子表达式结束位置约束的 `*`、`&` 类型构造尾链。它们不改变本节四种普通后缀的内部结构。

## 2. 调用后缀

```ebnf
call_suffix =
    "(", [ call_argument_sequence ], ")" ;

call_argument_sequence =
      argument_list
    | forward_all_arguments ;

forward_all_arguments =
    "..." ;

argument_list =
      positional_argument_list, [ ",", named_argument_list ]
    | named_argument_list ;

positional_argument_list =
    positional_argument, { ",", positional_argument } ;

positional_argument =
      expression
    | list_expansion ;

named_argument_list =
    named_argument, { ",", named_argument } ;

named_argument =
    identifier, "=", expression ;

list_expansion =
    "...", expression ;
```

调用可以没有实参，也可以包含位置实参、列表展开和命名实参：

```ink
run()
consume(value)
combine(first(), second(), third())
target(prefix, ...values)
connect(host, timeout = 30)
Point(x = 10, y = 20)
function(...)
```

`list_expansion` 由议题 29 确认为列表元素，不是普通一元表达式。该位置最终是否允许展开、表达式是否表示可展开运行时参数包以及展开后的实参数量，由议题 68 的语义规则检查。`"..."` 的三个点必须直接相邻，后续表达式前可以有普通 Trivia。

`forward_all_arguments` 与 `list_expansion` 不同：裸 `...` 必须单独构成整个实参序列，不能与位置实参、命名实参或另一个展开混写。`target(...)` 和 `target(...values)` 分别形成全参数转发与普通列表展开。调用目标是否允许使用全参数转发由后续阶段判断，不改变调用后缀的中性 Parser 结构。

`named_argument` 使用 `identifier = expression`。赋值不是表达式，因此 Parser 在实参起始位置看到 Identifier 后跟 `=` 时可以直接建立 `NamedArgument`，不需要名称绑定或任意回溯。普通位置实参和 `list_expansion` 必须全部位于命名实参之前；进入 `named_argument_list` 后不能返回位置阶段：

```ink
connect(host, 443, timeout = 30)  // 合法
connect(host = host, timeout = 30) // 合法
target(prefix, ...values, mode = fast) // 合法

connect(timeout = 30, host)       // 语法错误
target(mode = fast, ...values)    // 语法错误
```

命名实参自身可以按任意源码顺序出现。名称是否存在、是否重复绑定参数以及目标 callable 是否保留可用于命名调用的参数名称，由语义分析检查。所有显式实参仍按源码顺序从左到右求值。

议题 07 的普通逗号列表规则适用，因此调用实参不允许尾随逗号：

```ink
consume(first, second,) // 非法
```

attribute application 与 decorator application 复用同一 `argument_list`，不建立独立的属性或装饰器实参文法。它们不复用外层 `call_argument_sequence`，因此不接受裸 `...`。默认实参的省略和补全继续遵守议题 65；省略值不会在 CST 中伪造实参节点。

调用目标在 Parser 阶段保持中性。`Point(10, 20)`、`factory(10, 20)` 和 `SelectedType(10, 20)` 都建立相同的 `CallExpression`；语义分析在解析 callee 后区分普通函数调用和议题 06 的显式构造调用。Parser 不建立单独的 `ConstructorExpression`。

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

空方括号不是普通索引，因为其中没有 `expression`。议题 30 将它定义为独立的类型值后缀：

```ink
Data[]   // 安全切片类型
array[]  // 语法结构成立；array 不产生 type 时语义错误
```

因此 `expression[]` 现在可以进入 CST，但空 `[]` 的左侧必须在语义阶段产生 `type`。它不能替代 `array[index]`、`vector[index]` 或 `map[key]`；这些带一个表达式的形式仍是本节的普通中性索引后缀。

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
if (pointer != null) {
    pointer->process();
}
```

Tokenizer 仍会分别产生源码中的单字符 Symbol Token，但 Parser 不把这些序列识别成可选链。

## 9. 泛型与切片扩展

议题 16 已经定义显式泛型实例化 `<...>` 及其与比较 `<`、`>` 和移位 `>>` 的消歧规则。它与本议题的普通后缀共同构成完整后缀链。

议题 17 已经使用冒号定义切片后缀。它是不同于普通单表达式索引的另一种后缀，并共同扩展完整的 `postfix_suffix` 集合。

议题 30 的空 `[]` 构造安全切片类型；非空 `[expression]` 则继续保持中性，由语义分析区分固定数组类型构造、编译期集合索引和普通运行时容器索引。

## 10. CST 与恢复

CST 应保存基础表达式以及有序后缀节点。每个调用括号、索引方括号、成员点、`->` 的两个单字符 Token、列表展开的三个点、分隔逗号和全部 Trivia 均准确保留。

Parser 可以使用嵌套的 `CallExpression`、`IndexExpression`、`MemberExpression` 节点，也可以在 full-fidelity CST 中保存一个基础节点加有序 suffix 列表，再在 lowering 时建立嵌套 AST。两种表示必须产生相同结合方向和源码范围。

缺少实参、展开表达式、索引、成员名或右定界符时，按照议题 03 插入 `MissingToken` 或建立 `ErrorNode`，并保证游标取得进展。

## 11. 确认结论

Ink 的普通后缀表达式从基础表达式开始，可重复附加调用、单表达式索引、`.` 成员访问和 `->` 原始指针成员访问。后缀具有最高优先级并从左到右组合；调用参数不允许尾随逗号，可以使用前置 `...expression` 作为展开元素；带索引的方括号必须准确包含一个表达式，空 `[]` 由议题 30 单独作为类型构造后缀；指针成员不能通过 `.` 隐式解引用，v0 不提供可选链。议题 16、17 分别以独立规则加入泛型实例化和切片，议题 30 进一步加入复合类型值尾链并让类型构造调用复用普通 `CallExpression`。
