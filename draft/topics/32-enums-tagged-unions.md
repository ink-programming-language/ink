# 议题 32：枚举、判别联合与通用 niche 优化

> 状态：已确认，议题 33、34、61 补充；Parser 议题 23、24、34 完成模式、`match` 与枚举声明语法
> 确认日期：2026-08-02

## 1. 一个 `enum` 同时覆盖两种枚举

Ink 使用同一个 `enum` 关键字定义无载荷枚举和带载荷的判别联合：

```ink
enum Color {
    red,
    green,
    blue
}

enum Optional<T: type> {
    none,
    some(T)
}

enum Result<T: type, E: type> {
    ok(T),
    error(E)
}
```

枚举分支使用 Parser 议题 07 的普通逗号列表：分支之间必须写逗号，最后一个分支后不接受尾随逗号。多行布局不改变该规则。

Parser 允许分支列表完全省略，因此空成员块可以形成结构完整的枚举声明：

```ink
enum Empty {}
```

`Empty` 是否作为没有任何可构造值的 uninhabited type、能否用于普通类型位置以及其布局和 ABI，属于后续语义规则，不由 Parser 通过强制至少一个分支提前决定。

单个枚举分支的核心语法为：

```ebnf
enum_branch_core =
    identifier,
    [ enum_payload_clause ],
    [ enum_discriminant_clause ] ;

enum_payload_clause =
    "(", type, { ",", type }, ")" ;

enum_discriminant_clause =
    "=", expression ;
```

因此无载荷分支直接写成 `none`，而不是重复提供等价的 `none()`；载荷子句至少包含一个 `type`，多个载荷类型之间使用不接受尾随逗号的普通逗号列表。当前只定义位置载荷，不在该语法中混入命名载荷。

```ink
none
some(T)
node(T, Node*)
red = 1
special(T) = 7
```

最后一种形式对 Parser 而言结构完整。判别表达式是否为合法编译期整数、判别值是否重复，以及带载荷分支能否同时指定显式判别值，均由语义分析决定。

完整的枚举分支允许在核心语法之前书写零组或多组 attribute：

```ebnf
enum_branch =
    { attribute_list },
    enum_branch_core ;
```

```ink
enum Result<T: type, E: type> {
    [reflect(name = "success")]
    ok(T),

    [deprecated(message = "use error")]
    failure(E),

    error(E)
}
```

attribute 必须位于分支最前面，其参数继续复用函数调用实参规则，包括命名实参。枚举分支不是函数声明，因此不接受 decorator、访问修饰符或函数 modifier。列表逗号位于完整 `enum_branch` 之后，分支 attribute 不改变逗号分隔规则。

枚举成员块只接受分支和统一的 enum 区域 `comptime` 控制项，并继续使用不接受尾随逗号的普通逗号列表：

```ebnf
enum_body =
    enum_member_block ;

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

`enum_member_comptime_item` 本身也是一个外层 `enum_member_item`，所以它和相邻分支之间必须写逗号；其每个 body 又递归使用完整的 `enum_member_block`：

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

同一个结构也允许 `comptime { ... }`、`comptime match`、`comptime for` 和 `comptime while`。`match` 至少包含一个 arm，arm 之间不写逗号；`for` 绑定继续强制写 `var` 或 `const`。这些名称只是统一 `ComptimeRegionControl` 对 `EnumMemberRegion` 的标准 EBNF 展开，不建立 enum 专用的阶段语义。

Ink v0 的枚举成员块不接受字段、函数、嵌套类型或普通表达式语句。因此 `comptime generate();` 也不能仅凭 `comptime` 前缀成为枚举成员；枚举结构生成必须使用上述结构化区域控制。以后增加枚举方法时，可以扩展 `enum_member_item`，不改变现有分支语法。

枚举声明和分支使用专用 CST 结构：

```text
EnumDeclarationSyntax
├─ TypeDeclarationPrefix
├─ enum
├─ Identifier
├─ GenericParameterClause?
└─ EnumMemberBlockSyntax
   ├─ {
   ├─ SeparatedSyntaxList<EnumMemberItemSyntax>
   └─ }

EnumBranchSyntax
├─ AttributeList*
├─ Identifier
├─ EnumPayloadClauseSyntax?
│  ├─ (
│  ├─ SeparatedSyntaxList<TypeSyntax>
│  └─ )
└─ EnumDiscriminantClauseSyntax?
   ├─ =
   └─ ExpressionSyntax
```

`EnumPayloadClauseSyntax` 不复用函数参数节点。函数参数还可以包含名称、默认值和参数包，而位置枚举载荷只保存非空的 `TypeSyntax` 逗号列表；实现只复用 `parse_type()` 与通用 `SeparatedSyntaxList`。

`enum_member_comptime_item` 也不建立 enum 专用 CST 节点。它继续产生统一的 `ComptimeBlockControl`、`ComptimeIfControl`、`ComptimeMatchControl`、`ComptimeForControl` 或 `ComptimeWhileControl`，解析调用携带的区域种类为 `EnumMember`。

enum 成员递归下降 Parser 依靠首 Token 即可确定入口，不需要回溯：

```cpp
parse_enum_member_item() {
    if (at(TokenKind::Comptime))
        return parse_comptime_region_control(RegionKind::EnumMember);

    return parse_enum_branch();
}

parse_enum_branch() {
    auto attributes = parse_attribute_lists();
    auto name = expect(TokenKind::Identifier);

    Optional<EnumPayloadClauseSyntax> payload;
    if (at(TokenKind::LeftParen))
        payload = parse_enum_payload_clause();

    Optional<EnumDiscriminantClauseSyntax> discriminant;
    if (consume_if(TokenKind::Equal))
        discriminant = parse_expression();

    return EnumBranchSyntax{
        attributes,
        name,
        payload,
        discriminant
    };
}
```

`enum_member_comptime_item` 的 FIRST 集合为 `comptime`；`enum_branch` 的 FIRST 集合为 `[` 或 `Identifier`，二者不相交。`comptime` 是关键字，不能作为分支名称；`[attribute] comptime if (...) ...` 也不合法，因为 attribute 只能附着于具体分支。

载荷列表中的逗号由右括号限定。判别表达式 Parser 在当前定界深度遇到不属于表达式的 `,` 或 `}` 时自然停止；分支返回后，由 `parse_enum_member_block()` 统一要求下一个 Token 为 `,` 或 `}`。

命名枚举声明复用类和接口已经确认的 `type_declaration_prefix` 与 `generic_parameter_clause`，但不提供继承子句：

```ebnf
enum_declaration =
    type_declaration_prefix,
    "enum", identifier,
    [ generic_parameter_clause ],
    enum_body ;
```

因此 attribute 全部位于访问修饰符等类型 modifier 之前，枚举不接受函数 decorator；访问修饰符可以省略，其默认含义不由 Parser 决定。成员块强制存在并由 `}` 自行结束，不支持 `enum E;` 前向声明，也不在 `}` 后写分号。`final enum E {}` 可以形成完整 CST，再由语义分析拒绝无意义的 `final`。

枚举没有继承列表，也不使用 C++ 的 `enum E : u8` 底层类型语法。固定整数表示继续写成：

```ink
[repr(u8)]
enum Color {
    red,
    green,
    blue
}
```

`[repr]` 的参数和适用性由 attribute 语义检查，不改变枚举声明头 Parser。

Ink v0 不再引入第二套 `union` 或 `variant` 关键字。带载荷 `enum` 是安全的判别联合，不提供忽略当前活动分支而读取重叠存储的语义。

`enum` 是值类型，不隐含堆分配、垃圾回收、对象身份、继承或 vptr。泛型枚举在具体类型参数实例化后具有确定布局。

## 2. 分支与活动载荷

每个枚举值在任意时刻恰好具有一个活动分支。无载荷分支只保存其判别状态；带载荷分支同时保存该分支的值：

```ink
const color = Color.red;
const value = Optional::<int>.some(10);
const empty = Optional::<int>.none;
```

只有活动分支的载荷处于对象生命周期内。程序不能直接读取、写入或析构非活动分支的存储，也不能把默认枚举布局当作无判别的内存覆盖区使用。

分支的完全限定名称由枚举类型和分支名称组成。是否允许同一枚举中的不同分支使用同名载荷字段，留给命名载荷语法议题确定。

普通值表达式必须使用这种完全限定名称；Ink 不提供根据期望类型推导 `.red` 或 `.some(value)` 的值简写。前导点形式只用于 `match`、`if (match ...)` 和 `while (match ...)` 中由被匹配值提供枚举类型的分支模式。

## 3. 递归枚举

直接包含自身会产生无限大小，因此编译错误：

```ink
enum InvalidList<T: type> {
    end,
    node(T, InvalidList::<T>) // 编译错误：无限递归布局
}
```

通过裸指针或未来的拥有型间接容器建立递归合法：

```ink
enum Node {
    leaf(int),
    next(Node*)
}
```

编译器必须对类型布局依赖图执行递归检查；不能仅对源码中的直接自引用做字符串检查。

## 4. `match` 和穷尽检查

枚举使用 `match` 按活动分支访问：

```ink
match (optional) {
    .none => {
        print("empty");
    }

    .some(value) => {
        print(value);
    }
}
```

这是 Parser 议题 24 的 `match_statement`：每个分支体是一条普通语句或语句块，分支之间不写逗号，整个结构也不追加分号。有值的 `match_expression` 则要求每个分支以逗号结束。

分支模式中的 `.name` 由被匹配枚举类型提供上下文，不按照全局名称查找。

对闭合枚举的 `match` 必须覆盖全部分支，否则编译错误。可以使用 `_` 覆盖剩余分支：

```ink
match (color) {
    .red => handle_red();
    _ => handle_other();
}
```

匹配过程读取判别状态并只进入对应分支，不复制或构造非活动载荷。议题 33 与 Parser 议题 23 规定模式绑定借用载荷，其可写性由被匹配对象的 `Enum&`、`const Enum&` 或 place 访问能力传播；匹配不会隐式消费或移动不可复制载荷。

## 5. 复制能力

枚举的复制能力按照议题 02 结构化传播：

```text
copyable(Enum) =
    Enum 没有 [noncopyable]
    && Enum 没有用户析构函数
    && 每个分支的每个载荷类型都可复制
```

即使某个运行时值当前位于无载荷或可复制分支，只要另一个分支含有不可复制载荷，整个枚举类型仍然不可复制。

```ink
enum MaybeFile {
    none,
    some(File)
}
```

如果 `File` 为 `[noncopyable]`，`MaybeFile` 自动不可复制。外层仍可显式添加 `[noncopyable]`，防止以后修改分支后意外恢复复制能力。

## 6. 构造与销毁

构造一个分支时只构造该分支的载荷。某个载荷构造失败时，只清理此前已经成功构造的载荷，不把整个枚举视为已经完成初始化。

销毁枚举时，编译器根据有效判别状态只销毁活动分支的载荷。非活动分支没有对象生命周期，不执行清理。

分支切换、枚举赋值以及用户是否可以为枚举声明方法或析构函数的具体语法留给后续议题；任何规则都必须保持“始终只有一个活动分支”的不变量。

## 7. 默认布局

没有显式表示属性的带载荷枚举在语义上包含：

```text
enum storage = {
    discriminant,
    storage large/aligned enough for the largest payload,
}
```

这不是固定字段顺序的源码承诺。编译器可以根据目标 ABI 选择判别值大小、放置顺序、填充以及 niche 编码，但必须正确保存所有分支状态和载荷有效表示。

默认布局的 `size` 和 `alignment` 可以由编译期反射观察，但程序不能假定它跨目标、编译器 ABI 版本或布局变更保持相同。

## 8. 通用 niche 优化

如果某个载荷类型存在语言和目标 ABI 明确认定为无效的位模式，编译器可以使用这些位模式编码其他枚举分支，从而省略独立判别字段。

例如非空引用的空地址模式不能表示有效 `T&`，因此可以编码 `Optional::<T&>.none`：

```text
Optional::<T&>.none        = null address
Optional::<T&>.some(value) = non-null reference address
```

接口引用的有效值要求规范对象地址和接口表有效，因此 `Optional::<Interface&>` 可以使用空接口引用作为 `none`，而不增加第三个机器字。

并非所有类型都有 niche：

- `u8` 的全部位模式都是有效数值，`Optional::<u8>` 通常需要额外状态；
- `T*` 自身允许 `null`，如果 `Optional::<T*>` 需要区分 `none` 和 `some(null)`，不能直接用空指针同时表示二者；
- 嵌套枚举需要足够多的不同状态，单个 niche 未必够用。

niche 优化是所有枚举都可使用的通用布局优化，不根据 `Optional` 的名称触发。它通常减少存储、复制量和缓存压力，但不会消除读取活动分支时必要的状态检查。

除非某个目标 ABI 或显式表示属性另有保证，源码不能把某项 niche 选择当作可移植 FFI 布局承诺。

## 9. 标准库 `Optional::<T>`

Ink 不提供 `T?` 类型语法，也不把 `Optional::<T>` 定义成编译器内建类型。核心标准库使用普通 Ink 泛型枚举定义：

```ink
enum Optional<T: type> {
    none,
    some(T)
}
```

编译器内建的 `try_cast::<T&>` 返回 `core.Optional::<T&>`，但该返回类型仍是标准库声明。最小核心库环境必须提供规范的 `core.Optional`；用户定义的同名类型不能替换内建操作所引用的核心声明。

`Optional::<T&>` 可以像其他包含引用的值一样返回或长期保存，但不会因被包在枚举载荷中而延长目标生命周期。引用失效后解包并访问目标属于 UB。

`Optional` 的 `if (match ...)` 解包和 `match (...)` 规则由议题 33 规定。议题 34 已确定不提供后缀 `?` 传播；便捷方法和额外构造 API 不属于枚举布局本身，留给标准库议题。

## 10. 无载荷枚举和 `[repr]`

无载荷枚举默认只保证能够区分其声明分支，不保证可移植的整数宽度或数值。

需要固定整数表示时使用内建 `[repr(IntegerType)]` 属性，并为需要稳定协议值的分支显式指定整数：

```ink
[repr(u8)]
enum Color {
    red = 1,
    green = 2,
    blue = 3
}
```

`IntegerType` 必须是允许作为枚举判别表示的整数类型，所有显式值必须能够由该类型表示。重复判别值、未显式赋值分支的编号规则以及整数与枚举之间的受检查转换语法留给后续议题。

`[repr(u8)]` 固定存储宽度，但是否与某个外部函数调用 ABI 的 C 枚举类别相同仍由 FFI 映射规则决定。

## 11. FFI

默认布局的带载荷枚举不能直接作为稳定 C ABI 类型传递。其判别字段位置、载荷偏移和 niche 选择都可能随 Ink 目标 ABI 变化。

需要与外部判别联合交互时，v0 使用显式的 `[repr]` 无载荷枚举、整数标签和独立载荷存储进行适配。带载荷枚举的 `[repr(C)]` 或其他稳定布局形式留给后续议题。

Ink v0 不提供无判别 `union` 供程序绕过活动分支检查。确实需要设备寄存器、协议覆盖存储或 C union 时，应在专门的底层存储与 FFI 议题中设计，不把安全 `enum` 的规则悄悄放宽。

## 12. 反射

编译期反射可以访问：

- 枚举名称、泛型参数、属性、`size` 和 `alignment`；
- 所有声明分支及声明顺序；
- 每个分支的载荷类型；
- 当前实例化后的判别和 niche 布局信息；
- `[repr]` 及显式整数判别值。

普通源码反射看到的是分支和载荷语义；需要 ABI 工具时通过专门布局视图观察实际判别编码和载荷偏移。

动态反射仍由 `[reflect]` 显式选择。进入动态反射的枚举描述符可以枚举分支、载荷签名和用户元数据，但不能假设默认布局永远使用独立整数判别字段。

## 13. 热更新与 ABI

以下变化属于枚举布局或协议 ABI 变化：

- 增加、删除或重新排序分支；
- 改变分支名称；
- 改变载荷数量、类型或顺序；
- 改变 `[repr]` 或显式判别值；
- 改变导致 niche 编码或整体布局不兼容的载荷表示。

旧枚举值不能在没有兼容性证明或迁移的情况下按新布局解释。动态反射名称仍遵守议题 22：重命名等于删除旧分支并创建新分支。

## 14. 后续问题

以下内容留给后续议题：

- 分支构造表达式的上下文简写；
- 命名载荷和分支内字段语法；
- 枚举方法、分支切换和用户析构函数；
- 重复及自动整数判别值规则；
- 整数到枚举的受检查转换 API；
- 带载荷枚举的稳定 `[repr(C)]` 布局；
- 必要时单独设计的底层无判别覆盖存储。
