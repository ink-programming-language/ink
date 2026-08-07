# Parser 议题 34：枚举声明

> 状态：已确认
> 确认日期：2026-08-05

## 1. 声明头

命名枚举复用 Parser 议题 33 的类型声明前缀和泛型参数子句：

```ebnf
enum_declaration =
    type_declaration_prefix,
    "enum", identifier,
    [ generic_parameter_clause ],
    enum_body ;

enum_body =
    enum_member_block ;
```

```ink
[reflect]
public enum Result<T: type, E: type> {
    ok(T),
    error(E)
}
```

`enum` 声明头没有继承子句。成员块必须存在，因此不提供 `enum E;` 形式；闭合 `}` 后也不写分号。访问修饰符可以省略，其默认含义不由 Parser 决定。

## 2. 枚举分支

```ebnf
enum_branch =
    { attribute_list },
    enum_branch_core ;

enum_branch_core =
    identifier,
    [ enum_payload_clause ],
    [ enum_discriminant_clause ] ;

enum_payload_clause =
    "(", type, { ",", type }, ")" ;

enum_discriminant_clause =
    "=", expression ;
```

分支载荷是至少含一个元素的位置类型列表，不接受名称、默认值或尾随逗号。无载荷分支直接写名称，不写空载荷子句：

```ink
none
some(T)
node(T, Node*)
red = 1
special(T) = 7
```

分支可以有零组或多组 attribute，且 attribute 必须位于分支名称之前：

```ink
enum Status {
    [reflect(name = "success")]
    ok,

    [deprecated(message = "use failed")]
    error
}
```

枚举分支不接受 decorator、访问修饰符或函数 modifier。

## 3. 成员块

枚举成员使用不接受尾随逗号的逗号列表。空成员块合法：

```ebnf
enum_member_block =
    "{",
    [
        enum_member_item,
        { ",", enum_member_item }
    ],
    "}" ;

enum_member_item =
      enum_branch
    | enum_member_comptime_item ;
```

```ink
enum Empty {}

enum Color {
    red,
    green,
    blue
}
```

因此下列尾随逗号不属于该文法：

```ink
enum Color {
    red,
    green,
    blue,
}
```

## 4. 枚举成员区域的 `comptime`

枚举成员区域复用 Parser 议题 32 的五种结构化控制形式，控制头继续使用固定括号：

```ebnf
enum_member_comptime_item =
    "comptime",
    (
        enum_member_block
      | enum_member_if_tail
      | enum_member_match_tail
      | enum_member_for_tail
      | enum_member_while_tail
    ) ;

enum_member_if_tail =
    "if", "(", if_condition, ")", enum_member_block,
    [
        "else",
        ( enum_member_block | enum_member_if_tail )
    ] ;

enum_member_match_tail =
    "match", "(", expression, ")", "{",
    enum_member_match_arm,
    { enum_member_match_arm },
    "}" ;

enum_member_match_arm =
    match_arm_pattern, "=>", enum_member_block ;

enum_member_for_tail =
    "for", "(",
    for_binding_mode, for_pattern,
    "in", for_source, ")",
    enum_member_block ;

enum_member_while_tail =
    "while", "(", while_condition, ")",
    enum_member_block ;
```

`enum_member_comptime_item` 是外层逗号列表中的一个完整成员，因此它与相邻分支之间仍必须写逗号：

```ink
enum TokenKind {
    identifier,

    comptime if (BuildDebugKinds) {
        debug_begin,
        debug_end
    },

    eof
}
```

`comptime { ... }`、`comptime match (...)`、`comptime for (...)` 和 `comptime while (...)` 使用同一成员列表规则。

## 5. v0 成员边界

Ink v0 的枚举成员块只接受枚举分支和上述结构化 `comptime` 项，不接受字段、函数、嵌套类型或普通表达式语句：

```ink
enum Invalid {
    var value: int;
}
```

这条限制直接由 `enum_member_item` 的两个候选产生式表达，不为枚举引入第二套 `union` 或 `variant` 声明语法。

## 6. 结论

枚举由必需的命名声明头和必需的成员块组成，可以为空或包含无尾随逗号的成员列表。分支支持前置 attribute、非空位置类型载荷和可选判别表达式；枚举成员区域同时支持统一的五种 `comptime` 结构。
