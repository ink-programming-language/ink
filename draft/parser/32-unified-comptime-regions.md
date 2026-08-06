# Parser 议题 32：统一的 `comptime` 区域控制

> 状态：已确认，所有结构化控制头使用固定括号
> 确认日期：2026-08-05

## 1. 目标

Ink 只有一个 `comptime` 阶段前缀，不为 module、函数、类、接口或枚举分别定义不同的编译期语言。`comptime` 始终要求其直接拥有的值或控制决定在编译期完成；调用位置只决定选中结果必须输出到哪一种语法区域。

代表性用法为：

```ink
class Config {
    var capacity: int = comptime GetCapacity();

    comptime if (build.mode == BuildMode.debug) {
        var debug_id: u64;
    }
}

func run() {
    var capacity: int = comptime GetCapacity();

    comptime if (build.mode == BuildMode.debug) {
        log_debug();
    }
}
```

两个 `comptime GetCapacity()` 都是 Parser 议题 19 的同一种 `comptime_expression`。两个 `comptime if` 也共享同一种控制结构和 Partial Evaluation 规则；前者选出的 body 输出类成员，后者选出的 body 输出语句。类中不存在另一种“生成字段的 `comptime`”。

## 2. 区域种类

Parser 至少区分以下区域：

```text
ValueRegion             一个表达式值
StatementRegion         函数或普通语句块中的 block item
TopLevelRegion          module 顶层 item
ClassMemberRegion       class 成员 item
InterfaceMemberRegion   interface 成员 item
EnumMemberRegion        enum 成员 item
```

区域种类决定两件事：

1. 花括号内调用哪个 item Parser；
2. 编译期选择或展开完成后，结果交给哪一种输出 sink。

它不改变 `comptime` 的求值规则。`ClassMemberRegion` 允许字段、函数和嵌套类型，`StatementRegion` 允许局部声明和语句，`TopLevelRegion` 允许导入和顶层声明；不能为了复用花括号而把它们都解析成 `statement_block`。

## 3. 统一语法模式

概念上的语法族为：

```text
comptime_form<R> :=
    "comptime" comptime_operand<R>

comptime_operand<ValueRegion> :=
    unary_expression

comptime_operand<ItemRegion R> :=
      region_block<R>
    | region_if_tail<R>
    | region_match_tail<R>
    | region_for_tail<R>
    | region_while_tail<R>

region_block<R> :=
    "{" { region_item<R> } "}"
```

这一段是用于说明产生式家族的 schema，不是 Parser 议题 04 所定义的标准 EBNF，也没有向 Ink 源码增加泛型语法。正式 EBNF 必须为每一种区域把 `R` 机械替换为准确的 block 和 item 非终结符。所有展开实例都来自同一个 schema，并降低到同一种 CST 与语义节点。

## 4. 标准 EBNF 的具体实例

以类成员区域为例，标准 EBNF 展开为：

```ebnf
class_member_comptime_item =
    "comptime",
    (
        class_member_block
      | class_member_if_tail
      | class_member_match_tail
      | class_member_for_tail
      | class_member_while_tail
    ) ;

class_member_if_tail =
    "if", "(", if_condition, ")", class_member_block,
    [
        "else",
        ( class_member_block | class_member_if_tail )
    ] ;

class_member_match_tail =
    "match", "(", expression, ")", "{",
    class_member_match_arm, { class_member_match_arm },
    "}" ;

class_member_match_arm =
    match_arm_pattern, "=>", class_member_block ;

class_member_for_tail =
    "for", "(", for_binding_mode, for_pattern,
    "in", for_source, ")", class_member_block ;

class_member_while_tail =
    "while", "(", while_condition, ")", class_member_block ;
```

`class_member_comptime_item` 只是标准 EBNF 中对统一 schema 的区域适配名称，不建立类专用关键字、AST 语义或执行器。接口实例把其中的 `class_member_block` 换成 `interface_member_block`；enum 实例把它换成使用普通逗号列表的 `enum_member_block`，并把整个控制项作为一个 `enum_member_item` 参与外层列表分隔；module 顶层实例由 Parser 议题 05 的 `top_level_block` 展开；statement 实例使用 `statement_block` 和既有 statement 产生式。

具体展开继续复用同一组 header 非终结符：

```text
if_condition
match_arm_pattern
for_binding_mode
for_pattern
for_source
while_condition
```

因此区域变化只替换 body 的 item 类别，不产生另一套条件、模式、循环绑定或标点规则。

## 5. 唯一阶段含义

不同直接操作数拥有不同的阶段完成条件，但该条件不随区域变化：

| 直接操作数 | 编译期要求 | 输出 |
| --- | --- | --- |
| expression | 整个表达式产生 `Known(value)` | `ValueRegion` 接收一个值 |
| block | 整个 block 在编译期区域中执行 | 当前区域接收执行产生的结果 |
| `if` | 条件在编译期已知 | 当前区域接收选中分支 |
| `match` | 被匹配值和 arm 选择在编译期已知 | 当前区域接收选中 arm |
| `for` | 迭代源和展开次数在编译期已知 | 当前区域按迭代顺序接收展开结果 |
| `while` | 每次条件和展开过程均在编译期决定 | 当前区域接收每轮结果 |

例如同一个 `comptime if`：

```text
StatementRegion
→ 选中 statement_block
→ 执行编译期部分并残留允许的运行时 InkIR

ClassMemberRegion
→ 选中 class_member_block
→ 贡献选中的字段、函数和嵌套类型

TopLevelRegion
→ 选中 top_level_block
→ 贡献选中的导入和顶层声明
```

这是同一个控制决定投影到不同的类型化输出区域，不是三种 `comptime` 语义。

## 6. 表达式位置

表达式位置继续使用 Parser 议题 19：

```ebnf
comptime_expression =
    "comptime", unary_expression ;
```

函数调用、构造调用和成员调用都是表达式，不需要专用编译期调用语法：

```ink
var first: int = comptime GetXXX();
var second: int = comptime (GetXXX() + 1);
```

第一种只把调用作为直接一元操作数；第二种通过括号要求整个加法表达式产生编译期已知结果。字段初始化器、局部初始化器、module 绑定初始化器、实参和其他表达式位置全部复用相同节点。

位于字段声明中的 `comptime_expression` 不会条件性地增加或删除字段。字段始终存在，只有初始化值被强制在编译期求出。

## 7. Item 起始位置

在 item 起始位置，外层 Parser 已经知道当前区域。若 `comptime` 后的显著 Token 是 `{`、`if`、`match`、`for` 或 `while`，进入统一的区域控制 Parser。

普通 statement context 还允许 `comptime_expression` 成为表达式语句的一部分：

```ink
comptime validate_configuration();
```

此时 statement Parser 在结构化区域前缀试探失败后回到普通表达式语句入口，由表达式 Parser 消费 `comptime`。module、class、interface 和 enum item 区域不允许普通表达式语句，所以 `comptime generate();` 不能仅因带有前缀就成为声明；声明区域只能选择或重复其 `RegionRules` 已经允许、并且静态写在源码中的普通声明。Ink v0 不提供 `field(...)` 或其他声明构造表达式。

在 `=` 之后不存在上述 item 歧义。字段初始化器调用普通表达式 Parser，因此：

```ink
var capacity: int = comptime GetCapacity();
```

确定进入 `ComptimeExpression`。

## 8. 递归下降实现

实现使用一个区域描述对象，而不是为每个区域复制控制结构 Parser：

```cpp
enum class RegionKind {
    Statement,
    TopLevel,
    ClassMember,
    InterfaceMember,
    EnumMember,
};

struct RegionRules {
    RegionKind kind;
    SyntaxNode (*parse_item)(Parser&);
    TokenSet item_starters;
    TokenSet recovery_set;
    SyntaxKind block_kind;
};
```

统一入口为：

```cpp
SyntaxNode Parser::parse_comptime_region_control(
    const RegionRules& region) {
    auto comptime = consume(TokenKind::Comptime);

    switch (peek_significant().kind) {
    case TokenKind::LeftBrace:
        return parse_comptime_block(comptime, region);
    case TokenKind::If:
        return parse_comptime_if(comptime, region);
    case TokenKind::Match:
        return parse_comptime_match(comptime, region);
    case TokenKind::For:
        return parse_comptime_for(comptime, region);
    case TokenKind::While:
        return parse_comptime_while(comptime, region);
    default:
        return recover_comptime_region_operand(comptime, region);
    }
}
```

所有 body 通过同一个区域 block helper 解析：

```cpp
SyntaxNode Parser::parse_region_block(const RegionRules& region) {
    auto left = expect(TokenKind::LeftBrace);
    NodeList items;

    while (!at(TokenKind::RightBrace) && !at_end()) {
        items.push_back(region.parse_item(*this));
    }

    auto right = expect(TokenKind::RightBrace);
    return make_region_block(region.block_kind, left, items, right);
}
```

`parse_comptime_if`、`parse_comptime_match`、`parse_comptime_for` 和 `parse_comptime_while` 只接收 `RegionRules` 并把它传给每个 body；header Parser 与普通控制结构完全复用。

## 9. CST

CST 使用统一的结构节点：

```text
ComptimeExpression
ComptimeBlockControl
ComptimeIfControl
ComptimeMatchControl
ComptimeForControl
ComptimeWhileControl
```

每个结构化节点的父区域或显式 `RegionKind` 决定 body 节点种类：

```text
ComptimeIfControl
├─ Keyword(comptime)
├─ IfCondition
├─ ClassMemberBlock
└─ ClassMemberBlock
```

不建立具有独立语义的 `ComptimeClassIf`、`ComptimeInterfaceIf` 或 `ComptimeTopLevelIf`。实现可以为了 visitor 分派提供区域适配 wrapper，但 lowering 前必须规范化为同一种控制节点，并且不能让 wrapper 产生不同的语言行为。

## 10. Partial Evaluation 与输出 sink

lowering 后的统一节点接收一个类型化输出 sink：

```text
ValueSink
StatementSink
TopLevelDeclarationSink
ClassMemberSink
InterfaceMemberSink
EnumMemberSink
```

执行器首先满足直接操作数的阶段要求，再把结果交给 sink：

```text
evaluate selector in ComptimeWorld
→ choose or expand regions
→ emit selected items to current sink
```

sink 拒绝错误种类的输出。例如 `ClassMemberSink` 不接受普通 `return`，`StatementSink` 不接受 `import`。这些形状通常已经由对应 region Parser 排除；lowering verifier 仍必须防止结构化生成或内部错误绕过区域约束。

未选中分支仍必须词法和语法正确，但不进入后续名称绑定、类型检查、布局或最终代码生成。选中类成员改变对象布局时，控制输入必须进入实例和构建缓存键；这属于 Partial Evaluation 与 ABI 规则，不改变 Parser。

## 11. 错误恢复与 REPL

消费 `comptime` 后缺少直接操作数时，恢复集合来自当前区域：

- `StatementRegion` 同步到语句或局部声明起始 Token；
- `TopLevelRegion` 同步到导入、顶层声明、`comptime` 或 EOF；
- 类型成员区域同步到字段、函数、嵌套类型、`comptime` 或外层 `}`。

每个 `region_block` 缺少 `}` 时使用当前区域的 block kind 插入对应 `MissingToken('}')`。不能先建立 `StatementBlock` 再根据其中内容改判，也不能因为未选中分支而忽略其语法错误。

REPL 在未闭合的 condition、match arm、loop header 或 region block 末尾返回 `Incomplete`。条件不能编译期求值、循环不收敛或选中成员语义非法都发生在 Parser 之后，不改变结构完整状态。

## 12. 确认结论

Ink 的 `comptime` 是统一阶段前缀。表达式位置要求产生编译期已知值；结构化 block、`if`、`match`、`for` 和 `while` 使用同一个区域控制 schema。module、函数、类、接口和枚举只提供不同的 `RegionRules` 与输出 sink，不获得各自的 `comptime` 关键字或语义。标准 EBNF 可以机械展开区域适配非终结符，Parser、CST、Partial Evaluation 和诊断则共享同一套核心实现。
