# Parser 议题 07：逗号分隔列表

> 状态：已确认，议题 24 补充 `match_expression` 分支结束逗号；议题 27 同步上下文 `from`；议题 29 补充列表展开；2026-08-05 确认枚举分支继续禁止尾随逗号
> 确认日期：2026-08-03

## 1. 普通列表规则

Ink 的普通逗号分隔列表不允许尾随逗号。每种实际列表在自己的产生式中使用标准 EBNF 展开：

```ebnf
parameter_list =
    parameter, { ",", parameter } ;

argument_list =
    argument, { ",", argument } ;

imported_member_list =
    imported_member, { ",", imported_member } ;
```

本议题不引入可参数化的自定义 `List<T>` 元语法；后续正式产生式应直接写出对应元素非终结符。

## 2. 空列表

列表本身表示至少一个元素。某个定界结构是否允许空列表，由外层产生式使用标准可选序列决定：

```ebnf
parameter_clause =
    "(", [ parameter_list ], ")" ;

argument_clause =
    "(", [ argument_list ], ")" ;
```

因此 `()` 可以表示空参数或空实参结构，但单独的逗号不能表示空元素。

## 3. 禁止形式

普通列表禁止：

- 第一个元素之前的前导逗号；
- 两个相邻逗号形成的空元素；
- 最后一个元素之后的尾随逗号；
- 用一个逗号表示单元素普通列表。

例如：

```ink
func f(a: i32, b: i32) {}  // 合法
func f(, a: i32) {}        // 非法：前导逗号
func f(a: i32,, b: i32) {} // 非法：空元素
func f(a: i32, b: i32,) {} // 非法：尾随逗号
```

尾随逗号不会被解释为隐藏的空元素，也不会由 Parser 静默忽略。

## 4. 多行列表

换行属于 Trivia，因此普通列表可以跨行，但最后一个元素后仍不写逗号：

```ink
func run(
    path: String,
    retry: i32
) {
    process(
        path,
        retry
    );
}
```

格式化器可以选择单行或多行布局，但不能为了多行格式插入语言不接受的尾随逗号。

## 5. 适用范围

该规则统一适用于所有被对应语法定义为逗号分隔的普通列表，包括：

- 函数参数和调用实参；
- 泛型参数和泛型实参；
- 数组或其他集合字面量元素；
- `from ... import ...` 的成员；
- 以后明确使用逗号分隔的字段、枚举分支或其他结构。

某项语法如果使用分号或其他分隔符，不受本议题自动影响。未来增加的新逗号列表默认遵守本议题，除非其议题明确说明它属于元组消歧例外。

枚举分支已经确认继续使用本规则：

```ink
enum Color {
    red,
    green,
    blue
}
```

`blue` 后不能添加尾随逗号。带载荷分支和显式判别值分支遵守同一分隔规则。

枚举的外层 body 可以省略整个 `enum_branch_list` 形成 `{}`；这表示列表不存在，不是由空元素或尾随逗号表示空列表。

## 6. `from` 成员导入

成员导入直接使用普通列表规则：

```ebnf
member_import_declaration =
    contextual_from, module_path, "import", imported_member,
    { ",", imported_member }, ";" ;
```

`contextual_from` 是 Parser 议题 04 定义的准确拼写为 `from` 的 Identifier Token；它不改变本节的逗号列表规则。

单行和多行均可：

```ink
from math.scalar import sin, cos;

from math.scalar import
    sin,
    cos;
```

不增加仅为容纳尾随逗号而存在的括号导入形式。

## 7. 单元素元组例外

单元素元组必须使用逗号与普通括号表达式或括号类型消歧，这是唯一确认的尾随逗号例外：

```ink
(value)  // 括号表达式
(value,) // 单元素元组值

(i32)    // 括号类型
(i32,)   // 单元素元组类型
```

元组语法明确区分：

```ebnf
empty_tuple =
    "(", ")" ;

single_element_tuple =
    "(", tuple_element, ",", ")" ;

multiple_element_tuple =
    "(", tuple_element, ",", tuple_element,
    { ",", tuple_element }, ")" ;
```

多元素元组不允许尾随逗号：

```ink
(a, b)  // 合法
(a, b,) // 非法
```

单元素元组中的逗号承担语法消歧作用，不是普通列表的可选格式符号。

## 8. 列表展开不改变源码逗号规则

议题 29 使用前置 `...expression` 作为调用、泛型、元组类型和元组值列表中的一个源码元素：

```ink
target(prefix, ...values)
Other<Header, ...Types, Footer>
(header, ...values, footer)
(Header, ...Types, Footer)
```

Parser 先按照源码元素验证逗号，不提前展开元素数量。展开空序列不会使前导、连续或尾随逗号合法。单独的 `(...values)` 和 `(...Types)` 由 `...` 明确选择元组结构，不需要单元素尾随逗号；`(...values,)` 和 `(...Types,)` 仍然非法。

## 9. `match_expression` 的分支结束逗号

议题 24 要求每个 `match_expression_arm` 都以逗号结束，包括最后一个分支：

```ink
const value = match (optional) {
    .none => 0,
    .some(item) => item.value,
};
```

这里的每个逗号都是对应 arm 产生式的必需结束 Token，不是普通逗号列表中的可选尾随逗号。因此最后一个逗号不能省略，`match_statement` 分支则完全不使用逗号。

## 10. CST 与恢复

每个逗号仍是独立的 `Symbol(',')` Token，并作为列表 CST 的真实叶节点保留。Parser 遇到连续、前导或尾随逗号时，按照议题 03 形成 `ErrorNode` 或缺失元素恢复结构，不能删除真实逗号。

具体错误信息由未来独立 diagnostics draft 规定。

## 11. 确认结论

Ink 的普通逗号列表由一个元素和零个或多个“逗号加元素”组成，不接受尾随逗号。多行布局和语义阶段的列表展开不改变源码逗号规则。`(element,)` 的逗号用于单元素元组消歧；单独的 `(...expression)` 由展开标记完成消歧；`match_expression_arm` 的逗号用于结束每个分支。这些都由各自结构的专用产生式要求，不是普通列表的可选尾随格式。
