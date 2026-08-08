# Parser 议题 31：函数声明

> 状态：已确认，Parser 议题 38 增加 C++ 风格构造初始化列表
> 确认日期：2026-08-05

## 1. 目标

本议题统一自由函数、类成员函数、接口方法、构造函数候选、析构函数候选、泛型函数、异步函数和外部函数的声明语法。Parser 只根据 Token 结构建立无损 CST，不查询名称、所属类型、继承槽、函数类型或声明上下文的语义身份。

下列声明共享同一个 `function_declaration` 节点：

```ink
func run() {}

extern "C" func native_entry(value: i32) -> i32;

class Buffer {
    func Buffer(capacity: ptrsize) {}
    func ~Buffer() {}
    func size() const -> ptrsize {}
    static func create() -> Buffer {}
}

class Derived : Base {
    override final func update() {}
}
```

Parser 不在这里决定哪个普通 `identifier` 与所属类同名，也不把 `~ Identifier` 直接降低为析构语义。生命周期身份、修饰符组合、覆盖关系和函数体要求都由语义分析处理。

## 2. 完整 EBNF

```ebnf
function_declaration =
    function_annotation_sequence,
    function_modifier_sequence,
    "func",
    function_name,
    [ generic_parameter_clause ],
    function_parameter_clause,
    [ member_receiver_qualifier ],
    [ return_clause ],
    function_body ;

function_annotation_sequence =
    { attribute_list | decorator_application } ;

attribute_list =
    "[", attribute_application,
    { ",", attribute_application }, "]" ;

attribute_application =
    identifier, [ application_argument_clause ] ;

decorator_application =
    "@", identifier, [ application_argument_clause ] ;

application_argument_clause =
    "(", [ argument_list ], ")" ;

function_modifier_sequence =
    { function_modifier } ;

function_modifier =
      access_modifier
    | extern_modifier
    | "static"
    | "virtual"
    | "override"
    | "final"
    | "async"
    | "implicit" ;

access_modifier =
      "public"
    | "protected"
    | "private" ;

extern_modifier =
    "extern", string_literal ;

function_name =
      identifier
    | "~", identifier ;

generic_parameter_clause =
    "<", generic_parameter_list, ">" ;

generic_parameter_list =
    generic_parameter, { ",", generic_parameter } ;

generic_parameter =
    identifier, ":", type, [ parameter_suffix ] ;

function_parameter_clause =
    "(", [ function_parameter_list ], ")" ;

function_parameter_list =
    function_parameter, { ",", function_parameter } ;

function_parameter =
    parameter_name, ":", type, [ parameter_suffix ] ;

parameter_name =
    identifier ;

parameter_suffix =
      parameter_pack_suffix
    | default_argument ;

parameter_pack_suffix =
    "..." ;

default_argument =
    "=", expression ;

member_receiver_qualifier =
    "const" ;

return_clause =
    "->", type ;

function_body =
      function_definition
    | ";" ;

function_definition =
    [ constructor_initializer_clause ],
    statement_block ;

constructor_initializer_clause =
    ":", constructor_initializer,
    { ",", constructor_initializer } ;

constructor_initializer =
    constructor_initializer_target,
    call_suffix ;

constructor_initializer_target =
    identifier,
    { constructor_initializer_target_suffix } ;

constructor_initializer_target_suffix =
      ".", identifier
    | generic_argument_suffix ;
```

`argument_list` 和 `call_suffix` 复用议题 15 的普通调用规则；`generic_argument_suffix` 复用议题 16；`type` 复用议题 29；`expression` 复用议题 12—24、29、30；`statement_block` 复用议题 09。所有逗号列表继续遵守议题 07，不接受尾随逗号。

## 3. Attribute、decorator 与修饰符顺序

函数声明的所有 attribute list 和 decorator application 必须整体位于全部函数修饰符之前：

```ink
[reflect(info = meta("network", "async"))]
@trace(level = 2)
[nothrow]
public async func load() -> Data {
    // ...
}
```

`function_annotation_sequence` 可以交错包含多个方括号 attribute list 和多个 decorator application。进入 `function_modifier_sequence` 后不能再返回 annotation 阶段：

```ink
public [reflect] func f() {} // 语法错误
async @trace func g() {}    // 语法错误
```

属性和装饰器的参数统一使用 `application_argument_clause`。省略实参括号与显式空括号是两个不同源码形状，CST 必须保留差异：

```ink
@trace
func first() {}

@trace()
func second() {}
```

同理，`[reflect]` 与 `[reflect()]` 都能形成结构完整但形状不同的 attribute application。具体 attribute 或 decorator 是否允许省略括号、接受哪些实参以及能否重复，属于语义检查。

函数参数、返回位置和函数体 block 不重新开放 annotation 序列：

```ink
func invalid_parameter([attribute] value: i32); // 语法错误
func invalid_return() -> [attribute] Result;     // 语法错误
[attribute] { work(); }                          // 不是函数体语法
```

Attribute 只能附着整个函数声明；decorator 继续只应用于整个函数。未来若增加参数、返回或 block attribute，必须扩展对应专用产生式，不能把函数开头的 `function_annotation_sequence` 隐式传播到这些内部位置。

## 4. 函数修饰符

Tokenizer 议题 04 把 `public`、`protected`、`private`、`extern`、`static`、`virtual`、`override`、`final`、`async` 和 `implicit` 都产生为硬关键字。Parser 在 `func` 之前按源码顺序重复收集 `function_modifier`：

```ink
public async func read() -> Data;
extern "C" func native_read(handle: ptrsize) -> i32;
override final func update() {}
```

`extern_modifier` 必须同时消费紧随其后的 `string_literal`。Parser 不解释字符串内容：

```ink
extern "C" func c_entry();
extern r"platform" func platform_entry();
```

第二种 ABI 名称是否存在由语义分析决定。

产生式有意允许重复和任意源码顺序：

```ink
async public func reordered() {}
public public func duplicated() {}
static virtual func conflicting() {}
```

三者都能形成完整 CST；第一种重排不会仅因顺序不同而产生语义错误。语义分析负责报告重复修饰符、多个访问级别和冲突组合。格式化器的规范顺序为：

```text
access
extern
static
virtual / override
final
async
implicit
func
```

因此格式化结果使用 `public override final async func` 等顺序。`final class` 由类声明语法处理，不属于本议题的 `function_modifier_sequence`。

## 5. 函数名称与生命周期候选

`constructor` 和 `destructor` 都不是关键字。普通名称是一个 Identifier Token；析构形状是两个独立 Token：

```ink
func calculate() {}
func Resource(value: i32) {}
func ~Resource() {}
func ~ /* lifecycle */ Resource() {}
```

`~` 与后续 Identifier 之间可以存在 Trivia。格式化器统一输出紧凑的 `~Resource`。

Parser 只建立以下名称节点：

```text
IdentifierFunctionName
DestructorFormFunctionName
```

它不查询当前类名。因此：

```ink
class File {
    func Other() {}  // Parser：普通名称；语义：普通成员或其他名称规则
    func ~Other() {} // Parser：析构形状；语义：名称与所属类不匹配
}
```

构造函数身份由类内普通函数名称与所属类名相同确定；析构函数身份由类内 `~ Identifier` 形状及名称匹配确定。参数、泛型、返回、修饰符和函数体限制全部在语义阶段检查。

Ink v0 不提供运算符重载名称。`operator` 是普通 Identifier，因此 `func operator() {}` 是名为 `operator` 的普通函数；`operator+`、`operator[]` 或 `operator()` 不能匹配 `function_name`。

## 6. 泛型形参子句

泛型形参位于函数名称之后、运行时参数之前：

```ink
func convert<T: type, N: ptrsize = 4>(value: const T&) -> Data;
```

声明中的尖括号列表必须非空：

```ink
func valid<T: type>() {}
func invalid<>() {} // 语法错误：声明列表不能为空
```

泛型形参列表不允许尾随逗号：

```ink
func make_pair<
    First: type,
    Second: type
>() {}
```

`generic_parameter` 与运行时形参共享 `name: type`、默认值和尾随包标记的 Token 结构：

```ink
func parse<Base: u32 = 10>(text: StringView) -> i64;
func visit<Types: type...>() {}
```

泛型列表本身已经建立编译期参数域，因此 `T: comptime type` 不属于语法。参数包和默认值由 `parameter_suffix` 的互斥分支表示，不能同时出现。

一个列表最多一个包、包必须最后、包不能有默认值、无默认参数必须位于默认参数之前，这些都需要查看同一列表中的其他形参，统一由语义分析报告。Parser 保留每个节点及其源码位置，不为了满足顺序约束移动或删除形参。

## 7. 普通函数形参

运行时参数子句始终存在，空列表写作 `()`：

```ink
func tick() {}

func connect(
    host: StringView,
    port: u16 = 443
) {}
```

每个运行时形参必须拥有名称 Token、冒号和类型。Ink v0 不接受无名的仅类型形参或形参解构：

```ink
func unnamed(i32);        // 语法错误
func destructure((x, y): Pair); // 语法错误
```

Tokenizer 把 `_` 产生为普通 Identifier Token，所以它自然匹配 `parameter_name`。语义分析在运行时形参位置把该准确拼写解释为忽略绑定：

```ink
func ignore(_: i32) {}
```

普通名称和 `_` 在 CST 中都保存原始 Identifier；Parser 不为 `_` 创建新的 TokenKind。

运行时参数包同样使用尾随 `...`：

```ink
func print_all<Types: type...>(values: const Types&...) {}
```

运行时参数包必须最后、至多一个、不能拥有默认值，并且必须与可确定其展开形状的编译期类型包对应。这些要求属于议题 68 的语义分析，不改变本节 CST。

## 8. 默认表达式的结束位置

`default_argument` 使用完整 `expression`，但外层参数列表提供明确的顶层停止集合：

```text
泛型形参默认值： , 或 >
运行时形参默认值：, 或 )
```

嵌套括号、方括号、调用和其他内部结构拥有自己的结束符，不会被外层逗号或右定界符提前结束：

```ink
func f<Limit: bool = (left > right)>(
    value: i32 = choose(first, second)
) {}
```

为了让泛型声明的闭合 `>` 保持确定，默认表达式顶层出现比较 `>`、`>=` 或移位 `>>` 时必须加括号：

```ink
func good<N: bool = (left > right)>() {}
func bad<N: bool = left > right>() {} // 第一个 > 闭合泛型列表
```

Parser 只按嵌套深度和停止集合结束表达式，不根据期望类型或常量值猜测右尖括号含义。

## 9. `const` 接收者与返回子句

参数右括号之后可以出现一个尾随 `const`，随后才是可选返回子句：

```ink
func size() const -> ptrsize;
func clear();
```

以下顺序不是本议题语法：

```ink
func size() -> ptrsize const; // 语法错误
```

Parser 不查询函数是否位于类或接口中。因此自由函数、静态函数、构造函数或析构函数带尾随 `const` 都能形成 CST，随后由语义分析报告非法上下文。

尾随 `const` 限定隐式接收者并属于成员签名；它不是 attribute，也不属于 `func` 前面的 `function_modifier_sequence`。普通函数纯度或副作用保证应由未来独立属性表达。

返回子句使用直接相邻的 `->` 两个 Symbol Token，类型使用议题 29 的统一 `type`：

```ink
func value() -> i32;
func callback() -> func(i32) -> bool;
```

省略 `return_clause` 固定表示语义结果 `void`，不根据函数体推导返回类型。CST 继续保留“没有返回子句”，由 lowering 补入语义 `void`。

## 10. 函数体形状

Parser 统一接受语句块或真实分号：

```ink
func defined() {
    work();
}

func declared();
```

函数定义的语句块前可以出现 Parser 议题 38 的构造初始化列表：

```ink
func Player(id: i64, name: String)
    : Entity(id), name(name)
{}
```

Parser 不读取所属类名称来决定是否允许该子节点；只有语义分析确认当前函数为构造函数后，初始化目标才可以解释为直接具体基类或本类字段。初始化列表不能出现在以 `;` 结束的声明上。

`function_body` 不使用自动分号插入，也不把缺少 body 的 EOF 当作隐式 `;`。分号 Token 或完整 `statement_block` 必须真实存在或通过议题 03 的恢复节点表示缺失。

函数所处语义环境决定哪种 body 合法，例如：

- 普通类成员通常必须有函数体；
- `virtual func f();` 可以表示抽象虚函数；
- 接口要求可以使用分号，默认方法使用函数体；
- 外部函数通常使用分号；
- 构造函数和析构函数必须有函数体。

Parser 不为这些环境建立不同的函数声明产生式，也不把带分号节点改判为前向定义。

## 11. 确定性解析流程

推荐的递归下降流程为：

```text
parse declaration annotations in source order
parse zero or more function modifiers
expect func
parse identifier or ~ identifier
if next token is <: parse non-empty generic parameter clause
parse mandatory runtime parameter clause
if next token is const: consume receiver qualifier
if next adjacent symbols are ->: parse return type
if next token is ':':
    parse constructor initializer clause
    parse mandatory statement block
else:
    parse statement block or semicolon
```

声明区外层 Parser 可以先收集通用 annotation prefix，再根据后续关键字分派声明种类。`final class` 进入类声明；修饰符序列后出现 `func` 才进入本议题。`async func(` 在明确类型或表达式上下文中仍由议题 29、30 解析为函数类型，而在声明区域由外层调用者选择函数声明入口。

函数名称之后的 `<` 不需要议题 16 的表达式泛型后缀邻接消歧：声明上下文中它只能开始 `generic_parameter_clause`。名称与 `<` 之间允许普通 Trivia。

解析不需要查询：

- 名称是否为类型或当前类名；
- 修饰符是否适用于当前声明；
- 参数类型是否可实例化；
- 默认表达式是否为常量；
- 父类是否存在可覆盖虚槽；
- 当前函数是否允许省略 body。

## 12. CST 结构

建议使用以下完整节点形状：

```text
FunctionDeclaration
  annotations: [AttributeList | DecoratorApplication]
  modifiers: [FunctionModifier | ExternModifier]
  func_token
  name: IdentifierFunctionName | DestructorFormFunctionName
  generic_parameters: GenericParameterClause?
  parameters: FunctionParameterClause
  receiver_qualifier: ConstReceiverQualifier?
  return_clause: ReturnClause?
  body: FunctionDefinition | SemicolonFunctionBody

FunctionDefinition
  constructor_initializers: ConstructorInitializerClause?
  body: StatementBlock

GenericParameter
  name_token
  colon_token
  type
  suffix: ParameterPackSuffix | DefaultArgument?

FunctionParameter
  name_token
  colon_token
  type
  suffix: ParameterPackSuffix | DefaultArgument?
```

`AttributeApplication` 和 `DecoratorApplication` 复用议题 15 的 `PositionalArgument`、`ListExpansion` 与 `NamedArgument` 子节点。`@trace` 没有 argument-clause 子节点；`@trace()` 则保存真实左右括号和空实参列表状态。

每个关键字、符号、字符串、标识符、分隔逗号、分号和全部 Trivia 都必须保留。实现可以在 lowering 后把缺失返回子句解释为 `void`，但不能在 full-fidelity CST 中伪造不存在的 `-> void` Token。

## 13. Parser 与语义边界

Parser 不负责：

- 判断 `identifier` 是否构造函数名称；
- 判断 `~ identifier` 是否为合法析构函数；
- 判断构造初始化目标是直接基类、本类字段还是非法名称；
- 检查访问修饰符、`extern`、`static`、`virtual`、`override`、`final`、`async` 与 `implicit` 的重复、顺序或组合；
- 检查 `static` 是否仅用于类成员；
- 检查尾随 `const` 是否只用于非静态实例成员；
- 检查默认参数顺序、默认表达式类型和求值阶段；
- 检查泛型或运行时参数包数量、位置和对应关系；
- 检查 `_` 的忽略绑定语义和命名调用限制；
- 判断返回类型、函数体或分号在当前声明上下文是否合法；
- 执行重载、覆盖、接口实现、异常或 `[nothrow]` 检查；
- 判断 attribute 与 decorator 是否存在、参数是否合法或是否允许附着当前函数。

上述错误不能反向改变已经确定的 Token 归属、参数边界、修饰符顺序或 body 形状。

## 14. 错误恢复

函数声明使用局部定界符恢复，不扫描穿过完整的下一项声明。建议同步集合包括：

```text
attribute list:       ]  @  function modifier  func
generic parameters:  ,  >  (
runtime parameters:  ,  )  const  ->  :  {  ;
return clause:       :  {  ;
initializer list:    ,  {  ;
function body:       }  next declaration starter  EOF
```

典型恢复行为：

```ink
func () {}                 // 缺失 function_name
func ~() {}                // ~ 后缺失 Identifier
func f<T type>() {}        // 缺失 :
func f<T: type,>() {}      // 尾随逗号 ErrorNode
func f(value i32) {}       // 缺失 :
func f(value: i32,) {}     // 尾随逗号 ErrorNode
func f() -> {}             // 缺失 return type
func f() const const {}    // 多余第二个 const
func f()                   // 缺失 function_body
```

缺失名称、冒号、类型、右定界符或 body 使用零宽度 `MissingToken`；源码中真实存在的错误 Token 必须进入 `ErrorNode`。Parser 不能为了继续解析而删除尾随逗号、第二个 `const`、错误位置的 annotation 或冲突修饰符。

`extern` 后缺失字符串时，在 `func` 或下一声明起始 Token 前插入缺失字符串节点，不把 `func` 吞成错误 ABI 名称。默认表达式恢复必须尊重括号嵌套；只有到达当前参数层级的逗号或闭合定界符才结束该默认值。

## 15. REPL `Incomplete`

一旦已经提交到函数声明，REPL EOF 只有在继续输入能够自然闭合当前结构时才返回 `Incomplete`：

```ink
func
func name<
func name<T: type
func name(
func name(value: i32
func name() const
func name() ->
func name() :
func name() : Base(
func name() {
```

这些输入分别可以通过补充名称、泛型右尖括号、参数右括号、返回类型或语句块结束符完成。`func name();` 和完整块函数返回 `Complete`。

只有明确冲突 Token 的输入进入普通语法错误恢复，而不是 `Incomplete`：

```ink
func name(]
func name() -> ;
func ~123()
```

如果 EOF 发生在未闭合 attribute、decorator 实参或 `extern` 字符串结构中，外层声明前缀解析器与对应 Tokenizer/Parser 规则共同报告 `Incomplete`。语义错误不会改变 REPL 结构状态；例如自由函数尾随 `const` 或普通类成员使用分号，只要 Token 结构完整，Parser 仍返回 `Complete`，随后由语义分析诊断。

## 16. 确认结论

Ink 函数声明按“annotation 序列、修饰符序列、`func`、名称、可选非空泛型形参、必需运行时参数、可选尾随 `const`、可选返回子句、可选构造初始化列表、block 或分号 body”的固定顺序解析。初始化列表只伴随 block 定义；Attribute 与 decorator 必须全部位于修饰符之前，修饰符按源码顺序无损保存，合法组合交给语义分析。

泛型和运行时形参统一使用 `name: type`、`= expression` 默认值与互斥的尾随 `...` 包标记。所有普通逗号列表禁止尾随逗号；泛型声明 `<>` 非法而运行时 `()` 合法。省略返回子句固定表示 `void`，不进行函数体返回类型推导。

构造、析构、静态成员、只读成员、虚函数、最终函数、接口要求和外部函数共享同一 CST。Parser 只确定 Token 结构、恢复边界和 REPL 完整性，不承担名称绑定、继承分析、类型检查或函数语义分类。
