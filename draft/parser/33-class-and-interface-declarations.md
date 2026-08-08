# Parser 议题 33：类与接口声明

> 状态：已确认，成员区域 `comptime` 展开同步固定控制头括号；Parser 议题 40 抽取 class 公共尾部并在表达式上下文允许省略类名
> 确认日期：2026-08-05

## 1. 目标

本议题定义命名 `class`、命名 `interface`、字段、成员块和嵌套类型声明的 Parser 结构。Parser 只根据 Token 建立无损 CST，不查询名称所指声明的种类，不决定默认可见性，也不判断某个结构完整的成员是否符合类或接口语义。

```ink
[reflect]
public final class Vector<T: type> : Sequence::<T>, Serializable {
    [align(16)]
    private var data: T*;

    public const capacity: ptrsize;

    func size() const -> ptrsize {
        return this->capacity;
    }

    class Iterator {
        var current: T*;
    }

    comptime if (build.mode == BuildMode.debug) {
        var debug_id: u64;
    }
}

public interface Sequence<T: type> : Iterable::<T> {
    func at(index: ptrsize) const -> const T&;
}
```

函数成员完全复用 Parser 议题 31；所有 `comptime` 结构完全复用 Parser 议题 32 的统一区域控制。

## 2. 完整 EBNF

```ebnf
class_declaration =
    type_declaration_prefix,
    "class", identifier,
    class_definition_tail ;

class_definition_tail =
    [ generic_parameter_clause ],
    [ inheritance_clause ],
    class_member_block ;

interface_declaration =
    type_declaration_prefix,
    "interface", identifier,
    [ generic_parameter_clause ],
    [ inheritance_clause ],
    interface_member_block ;

type_declaration_prefix =
    type_annotation_sequence,
    type_modifier_sequence ;

type_annotation_sequence =
    { attribute_list } ;

type_modifier_sequence =
    { type_modifier } ;

type_modifier =
      access_modifier
    | "final" ;

inheritance_clause =
    ":", type, { ",", type } ;

class_member_block =
    "{", { class_member_item }, "}" ;

class_member_item =
      field_declaration
    | function_declaration
    | class_declaration
    | interface_declaration
    | enum_declaration
    | class_member_comptime_item ;

interface_member_block =
    "{", { interface_member_item }, "}" ;

interface_member_item =
      field_declaration
    | function_declaration
    | class_declaration
    | interface_declaration
    | enum_declaration
    | interface_member_comptime_item ;

field_declaration =
    field_annotation_sequence,
    field_modifier_sequence,
    field_binding_mode,
    identifier,
    ":", type,
    [ field_initializer ],
    ";" ;

field_annotation_sequence =
    { attribute_list } ;

field_modifier_sequence =
    { access_modifier } ;

field_binding_mode =
      "var"
    | "const" ;

field_initializer =
    "=", expression ;
```

`attribute_list`、`access_modifier`、`generic_parameter_clause` 和 `function_declaration` 复用 Parser 议题 31；`type` 复用议题 29；`expression` 复用表达式议题；`enum_declaration` 由后续枚举 Parser 议题完成内部定义。所有逗号列表遵守议题 07，不接受尾随逗号。

## 3. `comptime` 区域的标准 EBNF 展开

Parser 议题 32 使用一个接收 `RegionKind` 的统一控制 Parser。为了保持 Parser 议题 04 的标准 EBNF，类成员实例机械展开为：

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

接口成员实例只是把输出区域换成 `InterfaceMemberRegion`：

```ebnf
interface_member_comptime_item =
    "comptime",
    (
        interface_member_block
      | interface_member_if_tail
      | interface_member_match_tail
      | interface_member_for_tail
      | interface_member_while_tail
    ) ;

interface_member_if_tail =
    "if", "(", if_condition, ")", interface_member_block,
    [
        "else",
        ( interface_member_block | interface_member_if_tail )
    ] ;

interface_member_match_tail =
    "match", "(", expression, ")", "{",
    interface_member_match_arm,
    { interface_member_match_arm },
    "}" ;

interface_member_match_arm =
    match_arm_pattern, "=>", interface_member_block ;

interface_member_for_tail =
    "for", "(", for_binding_mode, for_pattern,
    "in", for_source, ")", interface_member_block ;

interface_member_while_tail =
    "while", "(", while_condition, ")", interface_member_block ;
```

两组名称只是标准 EBNF 的区域适配，不建立两套 CST 或语义。实际实现调用同一个 `parse_comptime_region_control(region_rules)`，分别传入 `ClassMember` 或 `InterfaceMember`。

## 4. 类型声明前缀

类和接口只接受零个或多个 attribute list，不接受函数专用 decorator。全部 attribute 必须位于全部类型修饰符之前：

```ink
[reflect]
[noncopyable]
private final class Resource {}

[exception]
public interface Error {}
```

一旦进入 modifier 阶段就不能返回 annotation 阶段：

```ink
private [reflect] class Invalid {} // 语法错误
```

`type_modifier_sequence` 按源码顺序保留全部访问修饰符和 `final`：

```ink
final private class Reordered {}
public private class Conflicting {}
final final class Duplicated {}
final interface ClosedInterface {}
```

这些形式都可以形成完整类型声明 CST。修饰符源码顺序本身合法，格式化器统一输出 `access final class`；重复、冲突以及 `final interface` 的合法性由语义分析处理。访问修饰符省略后的默认含义也不属于 Parser。

## 5. 名称、泛型与继承顺序

`class` 或 `interface` 后必须紧跟一个 Identifier；命名声明不接受匿名名称：

```ink
class Buffer {}
interface Reader {}
```

匿名或局部命名的 `class { ... }`、`class Node { ... }` 类型表达式由 Parser 议题 40 定义。它们复用本议题的 `type_declaration_prefix` 和 `class_definition_tail`，只在表达式上下文把 `identifier` 改为可选；本议题的命名声明入口仍然要求类名。

泛型参数位于名称之后，继承列表之前：

```ink
class Vector<T: type> : Sequence::<T> {}
interface Range<T: type> : Iterable::<T> {}
```

声明位置的 `<` 确定开始 `generic_parameter_clause`；表达式泛型应用则由连续 `::<` 引导，两者不会竞争同一终端。空 `<>` 和尾随逗号都不合法；泛型默认值、最终参数包和参数语义继续复用既有统一规则。

## 6. 继承列表

继承列表直接复用完整 `type`：

```ebnf
inheritance_clause =
    ":", type, { ",", type } ;
```

因此 Parser 可以保存：

```ink
class Player : Entity, Renderable, Serializable {}
interface Range<T: type> : Iterable::<T> {}

class BadPointerBase : Base* {}
class BadMultipleBase : FirstBase, SecondBase {}
interface BadConcreteParent : ConcreteClass {}
```

Parser 不查询某个名称是具体类、接口、开放泛型声明或 `final class`。语义分析负责要求继承项是允许继承的闭合名义类型，检查类最多一个具体父类、接口父项、重复项、继承环和 `final` 限制。继承列表不接受尾随逗号。

## 7. 强制成员块与声明结束

命名类和接口必须提供成员块，空块合法：

```ink
class Empty {}
interface Marker {}
```

Ink v0 不提供类型前向声明：

```ink
class Node;       // 语法错误
interface Reader; // 语法错误
```

模块名称绑定不依赖文本顺序，递归指针或引用可以直接引用当前类型：

```ink
class Node {
    var next: Node*;
}
```

成员块的 `}` 自行结束声明，不要求也不允许额外结尾分号。`class Empty {};` 中的 `;` 按议题 08 成为非法空顶层或成员项。

## 8. 字段声明

字段必须显式写 `var` 或 `const`，并且必须显式标注类型：

```ink
class Point {
    var x: i32;
    var y: i32;
    const dimension: ptrsize = 2;
}
```

以下旧形状不属于字段语法：

```ink
class Invalid {
    x: i32;           // 缺少 var 或 const
    var y = 1;        // 缺少类型
    const z = 2;      // 缺少类型
}
```

字段初始化器对 `var` 和 `const` 都可选：

```ink
var first: i32;
var second: i32 = 2;
const third: i32;
const fourth: i32 = 4;
```

Parser 不判断字段是否被充分初始化。默认初始化、构造函数成功返回前的完全初始化、`const` 字段恰好初始化一次以及声明初始化器与构造逻辑的冲突都由语义分析检查。

字段不支持元组解构或逗号多声明；每个字段只声明一个 Identifier，并以自己的 `;` 结束。

## 9. 字段前缀

字段允许零个或多个 attribute list，随后允许零个或多个访问修饰符，最后必须进入 `var` 或 `const` 核心：

```ink
class Buffer {
    [reflect]
    [align(16)]
    private var storage: byte[16];

    public const capacity: ptrsize = 16;
}
```

字段不接受 decorator，也不接受 `static`、`final`、`virtual`、`override`、`async` 或 `implicit` 等函数/类型修饰符：

```ink
private [reflect] var value: i32; // 语法错误
var public value: i32;            // 语法错误
static var shared: i32;           // 语法错误
```

多个访问修饰符可以先形成完整字段 CST，再由语义分析报告重复或冲突。省略访问修饰符后的默认含义不由 Parser 决定。

字段初始化器中的 `comptime` 继续是普通表达式：

```ink
var capacity: int = comptime GetCapacity();
```

它只强制初始化值在编译期求出，不条件性地增加或删除字段。

## 10. 类成员集合

类成员块是以下结构的封闭集合：

```text
field declaration
function declaration
nested class declaration
nested interface declaration
nested enum declaration
ComptimeRegionControl::<ClassMemberRegion>
```

成员位置的 `var` 或 `const` 总是字段，不会建立局部或 module 绑定。构造函数和析构函数仍然是 Parser 议题 31 的普通 `function_declaration`；Parser 议题 38 允许 block 函数定义携带 C++ 风格构造初始化列表。名称是否等于所属类名、是否以 `~` 开头、初始化目标类别以及生命周期约束都在语义分析中确定。

类成员块不接受：

- `import`；
- 普通表达式语句或运行时控制流语句；
- module/local binding declaration；
- decorator 声明；
- 单独的空 `;`；
- C++ 式访问分区标签。

访问权限只能逐声明书写：

```ink
class Account {
    private var balance: i64;
    public func balance_value() const -> i64 {
        return this->balance;
    }
}
```

`public:`、`protected:` 和 `private:` 都是语法错误；Parser 不维护影响后续成员的访问状态。

## 11. 接口成员集合

接口 Parser 广泛接受与类相同的声明形状，以保持结构解析与接口语义分离：

```ink
interface StructurallyComplete {
    var state: i32;
    static func create();
    func StructurallyComplete();

    class Nested {}
}
```

以上字段、静态函数和同名生命周期候选都能形成完整 CST。语义分析随后拒绝接口实例字段、v0 不支持的静态接口函数、构造/析构、非法访问级别、`final` 方法和其他不符合接口契约的成员。

普通接口函数以 `;` 结束时表示必需方法；带语句块时表示默认方法。子接口中的 `override func f();` 可以由既有语义解释为重新抽象。Parser 只保存函数 body 形状和修饰符。

接口的 `comptime` 未选中分支仍必须语法正确，但未激活声明不进入名称绑定、接口有效方法集合或其他语义检查。

## 12. 嵌套类型

类和接口成员块都允许嵌套命名 `class`、`interface` 和 `enum`。嵌套类与接口递归复用本议题的完整产生式：

```ink
class Tree {
    private class Node {
        var value: i32;
        var next: Node*;
    }

    public interface Visitor {
        func visit(node: const Node&);
    }
}
```

Parser 只建立嵌套 CST。嵌套类型没有隐式外层 `this`、不增加外层对象布局、能否引用外层泛型参数以及 `Tree.Node` 的名称绑定和访问权限都属于语义分析。

## 13. 确定性递归下降

类型和成员声明都具有确定的 introducer：

```text
class       → class_declaration
interface   → interface_declaration
func        → function_declaration
var/const   → field_declaration（成员区域）
enum        → enum_declaration
comptime    → 当前 RegionKind 的统一区域控制
```

annotation 和 modifier 出现在 introducer 之前，因此实现可以先扫描一个无损声明前缀，再由最终 introducer 分派：

```cpp
SyntaxNode Parser::parse_type_member(const RegionRules& region) {
    if (at(TokenKind::Comptime)) {
        return parse_comptime_region_control(region);
    }

    auto prefix = parse_declaration_prefix();

    switch (peek_significant().kind) {
    case TokenKind::Var:
    case TokenKind::Const:
        return parse_field_declaration(prefix);
    case TokenKind::Func:
        return parse_function_declaration(prefix);
    case TokenKind::Class:
        return parse_class_declaration(prefix);
    case TokenKind::Interface:
        return parse_interface_declaration(prefix);
    case TokenKind::Enum:
        return parse_enum_declaration(prefix);
    default:
        return recover_type_member(prefix, region);
    }
}
```

`parse_declaration_prefix` 必须保留 annotation 与 modifier 的原始顺序和 Trivia，并记录从 annotation 阶段进入 modifier 阶段的位置。最终声明 Parser 根据 introducer 验证前缀形状：decorator 后得到 `class`、modifier 后再次得到 attribute、字段前出现 `static` 等都产生语法错误和准确 `ErrorNode`，但不需要名称绑定。

`extern` 仍按议题 31 在函数前缀中同时消费紧随其后的字符串字面量。`final` 和访问修饰符既可能属于函数前缀，也可能属于类型前缀；Parser 通过最终 `func`、`class` 或 `interface` introducer 决定 CST 归属，不查询符号表。

## 14. CST

类与接口分别建立：

```text
ClassDeclaration
├─ TypeDeclarationPrefix
├─ Keyword(class)
├─ Identifier
├─ GenericParameterClause?
├─ InheritanceClause?
└─ ClassMemberBlock

InterfaceDeclaration
├─ TypeDeclarationPrefix
├─ Keyword(interface)
├─ Identifier
├─ GenericParameterClause?
├─ InheritanceClause?
└─ InterfaceMemberBlock
```

字段建立：

```text
FieldDeclaration
├─ FieldAnnotationSequence
├─ FieldModifierSequence
├─ Keyword(var | const)
├─ Identifier
├─ Symbol(:)
├─ Type
├─ FieldInitializer?
└─ Symbol(;)
```

继承列表保存每个完整 `type`、真实逗号和 Trivia。成员块保存真实花括号、全部成员和错误节点。`comptime` 成员使用议题 32 的统一控制节点，其 body 保持 `ClassMemberBlock` 或 `InterfaceMemberBlock`。

## 15. 错误恢复

成员区域的同步起点至少包括：

```text
[
@
public protected private
extern static virtual override final async implicit
func var const class interface enum comptime
}
```

不同 introducer 的合法前缀集合不同，但这些 Token 都能帮助 Parser 找到下一候选成员。真实错误 Token 必须保存在 `ErrorNode` 中，不能为了到达下一个成员而静默删除。

典型恢复包括：

- 缺少类型名称时在泛型参数、`:`、`{` 或 `;` 前插入 `MissingToken(identifier)`；
- 泛型参数列表缺少 `>` 时复用议题 31 的恢复；
- 继承列表缺少类型或逗号时同步到下一个类型起点或成员块 `{`；
- 缺少成员块 `{` 时建立缺失 Token，并在可行位置开始恢复成员；
- 字段缺少名称、`:`、类型、初始化表达式或 `;` 时保留已有子树并插入准确缺失节点；
- member block 缺少 `}` 时在外层声明同步点插入 `MissingToken('}')`；
- `class Name;` 中保留真实 `;` 为错误子节点，不能把它解释成前向声明；
- 声明后的额外 `;` 不能变成空成员。

Parser 议题 32 的 `comptime` 恢复继续使用当前 `RegionRules`，因此分支或循环 body 缺少 `}` 时恢复成准确的成员 block，而不是 `StatementBlock`。

## 16. REPL 完整性

以下输入在 EOF 处返回 `Incomplete`：

```ink
class Buffer
class Buffer<T: type
class Buffer : Base,
class Buffer {
class Buffer { var data:
interface Reader { func read(
```

显式给出不能完成声明的 Token 时可以返回 `Complete` 加语法错误：

```ink
class Buffer;
class Buffer {}
class Buffer {};
```

第二项合法；第一项是被明确终止的非法前向声明；第三项包含合法类声明和额外非法分号。重复修饰符、`final interface`、接口字段、非法继承关系或未完成字段初始化都是结构完整后的语义问题，不改变 REPL Parser 状态。

## 17. Parser 与语义分析边界

Parser 不负责：

- 决定省略访问修饰符后的默认可见性；
- 检查重复、冲突或不适用于声明种类的修饰符；
- 判断继承项是具体类、接口、开放泛型、闭合类型或 `final class`；
- 检查单一具体继承、接口继承图、重复父项或继承环；
- 判断字段默认初始化、构造初始化或 `const` 初始化是否充分；
- 判断函数是普通方法、构造函数还是析构函数；
- 检查接口字段、静态接口函数、默认方法或重新抽象是否合法；
- 检查嵌套类型捕获、名称访问、重复成员或对象布局；
- 求值 `comptime` 条件、选择分支、展开循环或计算最终成员集合；
- 计算泛型实例、类布局、vtable、接口表、反射描述符或 ABI。

这些检查都在完整 CST 建立以后进行。Parser 只保证每个节点的 Token 结构、区域 block 种类、恢复节点和源码覆盖准确。

## 18. 确认结论

命名类和接口共享 attribute、访问修饰符、泛型参数、完整 `type` 继承列表和强制成员块骨架。Parser 议题 40 的 class 类型表达式进一步复用相同 class 尾部，只在表达式上下文允许省略类名。字段必须显式写 `var` 或 `const` 以及类型，初始化器对两者都可选；字段 attribute 位于访问修饰符之前。类和接口都允许嵌套类型，并使用逐声明访问修饰符而非 C++ 访问分区标签。接口 Parser 广泛接受完整成员声明形状，再由语义分析限制有效接口成员。所有条件成员都通过 Parser 议题 32 的统一 `ComptimeRegionControl` 实现，不存在类或接口专用的 `comptime` 语言。
