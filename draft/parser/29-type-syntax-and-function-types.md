# Parser 议题 29：统一类型语法与函数类型

> 状态：已确认，议题 30 补充普通表达式中的复合类型值和运算符消歧；Parser 议题 15 的命名实参规则同步到列表展开
> 确认日期：2026-08-04

## 1. 统一 `type` 非终结符

所有明确要求类型的语法位置复用同一个 `type` 非终结符：

```ebnf
type =
    [ "const" ],
    type_primary,
    { type_postfix_suffix } ;

type_primary =
      type_name
    | parenthesized_type_expression
    | tuple_type
    | function_type ;

type_name =
      identifier
    | builtin_type ;

builtin_type = ? BuiltinType Token ? ;
```

它用于绑定类型标注、参数和字段类型、函数结果、泛型形参约束、`catch` 类型以及后续声明议题明确引用类型的位置。Parser 不为这些位置分别维护互不一致的指针、数组或函数类型子文法。

`identifier` 在 Parser 阶段不区分普通闭合类型名称、类型别名、值为 `type` 的编译期绑定或不是类型的普通名称。`builtin_type` 直接引用 Tokenizer 已经分类的 `BuiltinType` Token。名称最终是否在当前位置产生闭合 `type` 由名称绑定和语义分析判断。

## 2. 前置 `const`

类型内部只接受一个前置 `const`：

```ink
const Data
const Data&
const Data*
const Data[]
```

不提供 C++ 的后置拼写，也不提供指针对象自身的类型内 `const`：

```ink
Data const&  // 语法错误
Data const*  // 语法错误
Data* const  // 语法错误
```

前置 `const` 先作用于 `type_primary`，随后才依次应用后缀：

```text
const Data*    = (const Data)*
const Data[4]  = (const Data)[4]
const Data*[]  = ((const Data)*)[]
```

需要让指针绑定自身不可重新赋值时，使用 Parser 议题 10 的声明关键字：

```ink
const pointer: Data* = get_data();
```

这里第一个 `const` 属于绑定声明，不属于 `Data*`。声明开头和类型开头的同一 `Keyword(Const)` 由语法上下文区分。

Parser 只限制前置 `const` 最多一个，不按直接可见的 `type_primary` 种类判断限定是否合法。该判断必须在别名展开、泛型替换和计算类型求值后统一完成：

```ink
const Data           // 语法和语义均合法
const func() -> void // 语法合法，语义错误
const void           // 语法合法，语义错误
const const Data     // 语法错误
```

这样直接函数类型、函数类型别名和编译期计算得到的函数类型遵守同一语义规则。

## 3. 类型后缀链

类型值可以继续使用已经确认的调用、索引、切片、成员和泛型后缀，并增加指针、引用和空方括号类型构造：

```ebnf
type_postfix_suffix =
      postfix_suffix
    | "*"
    | "&"
    | empty_bracket_suffix ;

empty_bracket_suffix =
    "[", "]" ;
```

`postfix_suffix` 复用 Parser 议题 15—17 的定义：

```ebnf
postfix_suffix =
      ordinary_postfix_suffix
    | generic_argument_suffix
    | slice_suffix ;

ordinary_postfix_suffix =
      call_suffix
    | index_suffix
    | member_suffix
    | pointer_member_suffix ;
```

所有后缀按照源码从左到右组合：

```ink
Data*[4]                       // (Data*)[4]
Data[4]*                       // (Data[4])*
Data[4]&                       // (Data[4])&
Data*[]                        // (Data*)[]
make_pair<i32, String>()&      // 对调用产生的 type 构造引用
Container.instantiate<i32>()*  // 对实例化结果构造指针
```

Parser 固定拼写和结合方向，不判断每种组合是否是合法类型。`Data&&`、`Data&*`、`Data&[4]`、`(Data&)*` 等 Token 序列都可以形成完整类型 CST；引用是否允许再次引用、成为数组元素或被指针包装，由语义分析在别名展开和泛型替换后统一验证。

议题 30 将这些构造加入普通表达式时，不会把 `type_postfix_suffix` 整体作为另一个表达式分支。`*`、`&` 使用必须结束于当前子表达式边界的局部试解析；空 `[]` 使用独立类型构造后缀。无括号类型构造后再调用、访问成员、应用泛型或参与运算时必须分组，而 `[expression]` 可以继续完成 `T*[N]` 一类数组类型尾链：

```ink
T*[N]        // (T*)[N]
T*(value)    // 普通表达式中是 T * (value)
(T*)(value)  // 对完整 T* 类型值应用调用后缀
(T*).metadata
```

## 4. 非空方括号保持中性

Parser 议题 15 的非空索引后缀继续使用：

```ebnf
index_suffix =
    "[", expression, "]" ;
```

在类型入口中，它故意不提前固定为“数组类型”或“索引表达式”：

```ink
Data[4]
TypeTable[index]
```

两种写法具有相同 Token 结构。CST 保存中性的 `BracketPostfixSuffix`；语义分析根据左侧结果决定：

- 左侧编译期结果是 `type` 时，`[expression]` 构造固定数组类型；
- 左侧是可索引的编译期值时，执行普通索引，并要求整个类型链最终得到 `type`；
- 两种解释都不成立时产生语义错误。

固定数组长度位置接受完整 `expression`。语义阶段要求它能够在编译期求值为整数，并负责零长度、负值、溢出、目标布局上限和资源预算等约束：

```ink
byte[HeaderSize + PayloadSize] // 语法合法
byte[runtime_length]           // 语法合法，不能编译期求值时语义错误
```

空的 `T[]` 没有普通索引含义，始终构造安全切片类型。议题 30 允许同一空后缀出现在普通表达式入口；若左侧最终不是 `type`，则产生语义错误。带冒号的 `[start:end]` 继续是 Parser 议题 17 的普通切片表达式后缀；若它出现在类型链中，最终结果仍必须在编译期成为 `type`。

## 5. 加括号类型与计算类型

类型位置允许一个加括号的普通表达式计算类型：

```ebnf
parenthesized_type_expression =
    "(", type_or_expression, ")" ;

type_or_expression =
      type
    | expression ;
```

例如：

```ink
(Data)
(if (Use64) i64 else i32)
(if (Use64) i64 else i32)*
```

名称、成员访问、泛型实例化、调用和方括号后缀组成的类型值链可以直接出现；`if_expression`、中缀运算和其他不属于该后缀链的完整表达式必须使用括号。Parser 只确认结构，语义分析要求括号内容能够在编译期求值并且结果准确为 `type`。

圆括号入口使用一个中性 CST 节点，不通过符号表判断 `(Name)` 是“括号类型”还是“返回 type 的括号表达式”。两种来源都必须满足相同的最终 `type` 要求。遇到顶层逗号时则进入下一节的专用元组类型结构。

议题 30 已把复合类型值加入完全普通的表达式入口。名称、调用、成员、泛型、非空方括号和圆括号继续复用中性表达式 CST；`const`、函数类型、空 `[]` 以及受子表达式结束位置约束的 `*`、`&` 才增加专门结构。复杂中缀或 `if_expression` 计算类型时仍使用本节的圆括号形式。

## 6. 元组类型

元组类型保持 Parser 议题 07 和语义议题 69 已确认的区分，并加入编译期列表展开：

```ebnf
tuple_type =
      "(", ")"
    | "(", type, ",", ")"
    | "(", tuple_type_item, ",", tuple_type_item,
      { ",", tuple_type_item }, ")"
    | "(", list_expansion, ")" ;

tuple_type_item =
      type
    | list_expansion ;

list_expansion =
    "...", expression ;
```

基本形态为：

```ink
()            // 空元组类型
(i32)         // 加括号的 i32，不是元组
(i32,)        // 单元素元组类型
(i32, String) // 双元素元组类型
```

两个以上源码元素不允许尾随逗号。只有 `(type,)` 使用尾随逗号消除单元素歧义。

`list_expansion` 是列表元素，不是普通一元运算符或类型后缀：

```ink
(...Types)
(Header, ...Types, Footer)
```

展开表达式必须在编译期产生类型序列。一个单独展开项不需要尾随逗号，因为前置 `...` 已经明确选择元组类型结构。空、单元素和多元素序列分别产生空、单元素和多元素元组类型；混合列表的最终元素数量同样在展开后确定。

`"..."` 要求三个 `Symbol('.')` Token 在源码中直接相邻。`...` 与后续表达式之间可以存在普通 Trivia。

议题 30 让本节结构与普通元组表达式共享中性 `ParenthesizedCommaList` CST。明确类型入口直接要求它产生元组类型；普通表达式入口则由期望类型 elaboration 区分元组类型值和包含多个 `type` 值的普通元组：

```ink
const Pair: type = (i32, String); // 一个元组类型值
const Types = (i32, String);      // 一个 (type, type) 元组值
```

没有期望类型时默认形成普通元组值，Parser 不因元素看起来像类型名称而改变 CST。

## 7. 同步与异步函数类型

函数类型使用以下产生式：

```ebnf
function_type =
    [ "async" ],
    "func", "(",
    [ function_type_parameter,
      { ",", function_type_parameter } ],
    ")",
    [ "->", type ] ;

function_type_parameter =
      type
    | list_expansion ;
```

函数类型参数只写类型，不写参数名称或默认值：

```ink
func()
func(i32, u32)
func(const Data&) -> bool
func() -> (i32, String)

func(value: i32) // 语法错误
func(i32 = 10)   // 语法错误
```

省略 `-> type` 等价于返回 `void`。CST 不伪造一个不存在的 `void` Token；AST lowering 或类型语义把缺失结果规范化为 `void`。

函数声明采用同一缺省规则：`func name(parameters) { ... }` 的公开结果类型固定为 `void`，不是待函数体分析后推导的占位。返回非 `void` 结果的声明必须显式书写 `-> type`。Parser 仍保留“返回类型子句缺失”的真实 CST 形状，lowering 才补入语义上的 `void`。

`->` 后递归消费一个完整 `type`：

```ink
func() -> Data*             // 返回 Data*
func() -> Data[4]           // 返回 Data[4]
func() -> (Data, Error)     // 返回元组
func() -> func(i32) -> bool // 返回另一个函数值
```

如果需要把后缀应用于整个函数类型，必须使用圆括号：

```ink
(func() -> Data)*
(func() -> Data)&
```

这些外层组合最终是否合法继续由语义分析判断。

`async` 是函数种类和函数类型的一部分：

```ink
const loader: async func(Path&) -> Data = &load_async;
var task: Task<Data> = loader(path);
```

它不等同于同步任务工厂：

```ink
func(Path&) -> Task<Data>
```

两种类型不能互相赋值。`async func()` 省略结果时逻辑结果为 `void`，调用产生 `Task<void>`。

函数参数类型列表允许在编译期展开：

```ink
func(...Types) -> Result
func(Context&, ...Types) -> Result
async func(...Types) -> Data
```

例如 `Types = [i32, String]` 时，`func(...Types) -> bool` 形成闭合的 `func(i32, String) -> bool`。它不产生 C varargs、运行时开放函数类型或隐藏 `Any[]`。

议题 30 允许 `func(` 和 `async func(` 在普通表达式入口直接开始函数类型值：

```ink
inspect_type(func(i32) -> bool);
inspect_type(async func(Path&) -> Data);
```

函数声明的 `func identifier` 与函数类型的 `func (` 由 Token 形状区分，不需要名称查询。

## 8. `[nothrow]` 不属于函数类型

`[nothrow]` 是可调用声明上的内建属性，不进入 `function_type`、普通重载签名或函数值类型：

```ink
[nothrow]
func calculate(value: i32) -> i32;

const callback: func(i32) -> i32 = &calculate;
```

下列类型拼写不存在：

```ink
[nothrow] func(i32) -> i32
func(i32) -> i32 [nothrow]
```

该决定不削弱语义议题 34 的强静态保证。`[nothrow]` 函数必须证明异常不会逃出；普通 `func(...)` 值不保存属性，因此调用未知函数值时按可能抛出处理，并且必须由 catch-all 覆盖才能位于 `[nothrow]` 函数中。

其他属性列表同样不成为本节 `type` 的前缀或后缀。属性 Parser 在声明或其他明确属性目标外层保存属性节点，再由对应属性规范验证附着位置。

## 9. 列表展开对已有后缀的补充

Parser 议题 15 的普通调用实参列表和议题 16 的泛型实参列表都需要识别前置展开：

```ebnf
positional_argument =
      expression
    | list_expansion ;

argument_list =
      positional_argument_list, [ ",", named_argument_list ]
    | named_argument_list ;

positional_argument_list =
    positional_argument, { ",", positional_argument } ;

named_argument_list =
    named_argument, { ",", named_argument } ;

named_argument =
    identifier, "=", expression ;

generic_argument =
      type_or_expression
    | list_expansion ;

generic_argument_list =
    generic_argument, { ",", generic_argument } ;
```

例如：

```ink
target(...values)
target(prefix, ...values, mode = fast)
Other<Header, ...Types, Footer>
Vector<Data*>
```

普通调用、attribute application 与 decorator application 的实参列表还支持 Parser 议题 15 的 `identifier = expression` 命名实参。位置实参和 `list_expansion` 必须位于全部命名实参之前；泛型实参列表 `<...>` 仍是独立语法，本节不为它增加命名泛型实参。

`list_expansion` 的表达式可以是标识符、调用或其他完整表达式：

```ink
Other<...select_types()>
(...select_types())
```

具体列表要求编译期类型序列、普通编译期实参序列还是运行时参数包，由该列表的语义消费者检查。展开不是一般表达式，因此以下源码是语法错误：

```ink
const invalid = ...Types;
```

普通列表仍不允许尾随逗号。展开为空不会把前导、连续或尾随逗号变成合法源码；Parser 先验证源码列表结构，语义阶段再展开元素数量。

## 10. CST 与语义边界

建议的主要 CST 节点包括：

```text
TypeSyntax
ConstTypeQualifier
TypeName
ParenthesizedTypeExpression
ParenthesizedCommaList
FunctionType
TypePostfixSuffix
BracketPostfixSuffix
ListExpansion
```

CST 必须保留 `const`、`async`、`func`、`->` 的两个 Symbol Token、每个后缀符号、括号、方括号、逗号、展开点和全部 Trivia。后缀可以保存为基础节点加有序列表，也可以在 lowering 时逐层建立嵌套 AST；两种实现都必须保持相同的从左到右结合。

Parser 不负责：

- 查询标识符是否表示类型；
- 判断计算表达式是否能在编译期求值；
- 判断非空方括号是数组构造还是编译期索引；
- 判断固定数组长度是否合法；
- 判断指针、引用、数组、切片和函数类型组合是否合法；
- 展开类型别名、泛型替换或参数包；
- 判断 `[nothrow]`、`async` 或调用 ABI 的语义兼容性。

这些检查统一发生在名称绑定、编译期求值和类型分析阶段，不能反向改变已经建立的规范 CST。

## 11. 错误恢复

缺少类型主项、右括号、右方括号、泛型结束符、函数参数类型或函数结果类型时，Parser 按议题 03 插入零宽度 `MissingToken` 或建立 `ErrorNode`，并保留已经消费的真实 Token：

```ink
var a: const ;          // 缺少 type_primary
var b: Data[;           // 缺少长度表达式和 ]
var c: func(i32,);      // 非法尾随逗号
var d: func() -> ;      // 缺少结果 type
var e: (i32, String,);  // 多元素元组非法尾随逗号
```

类型入口的常用同步 Token 包括 `,`、`)`、`]`、`>`、`=`、`;`、`{` 和 EOF。恢复实现不能吞掉后续声明体的 `{`，也不能把真实右定界符归入错误类型节点。泛型结束符继续遵守议题 16 的嵌套 `>` 和 `>>` 规则。

REPL 在类型结构可能通过继续输入闭合时报告 `Incomplete`，例如缺少 `)`、`]`、`>` 或 `->` 后尚未输入结果类型；已经遇到明确冲突 Token 的结构则报告 `Error`。

## 12. 确认结论

Ink 使用一个统一的 `type` 非终结符。类型内部 `const` 只使用一个前置拼写，限定是否适用于最终类型由语义分析判断；名称或计算得到的类型值可以按源码从左到右继续应用普通后缀以及 `*`、`&`、`[]` 类型构造。议题 30 进一步允许这些复合类型进入普通表达式，并使用终止型尾链消除 `*`、`&` 与普通运算符的歧义。非空方括号由 CST 中性保存，数组构造、编译期索引或运行时容器索引交给语义分析；固定数组长度接受完整表达式并在语义阶段要求编译期整数。

`()`、`(T,)` 和多元素元组保持明确区分并共享中性圆括号逗号列表 CST，类型序列使用列表级 `...expression` 展开。函数类型写作 `[async] func(types) [-> type]`，可以作为普通表达式中的类型值；省略结果等价于 `void`，`async` 属于函数类型而 `[nothrow]` 只属于声明属性。函数、元组、泛型和普通实参列表可以在各自确认的位置使用前置展开，但 `...` 不成为普通表达式或类型后缀。
