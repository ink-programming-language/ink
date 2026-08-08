# Parser 议题 30：普通表达式中的复合类型值

> 状态：已确认；Parser 议题 35 把聚合初始化放入普通 postfix 层，议题 40 补充复用普通 class 结构的类型表达式；2026-08-08 把终止型类型构造尾链改为互补 EndSet 守卫并补全规范调用矩阵
> 确认日期：2026-08-04

## 1. 目标

议题 14 已允许名称和内建类型作为一等编译期 `type` 值，议题 29 则定义了明确类型入口中的完整 `type` 文法。本议题把复合类型构造补入普通表达式，使函数可以保存、返回和传递完整类型值：

```ink
const PointerType: type = Data*;

func make_pointer_type<T: type>() -> type {
    return T*;
}

inspect_type(Data*);
inspect_type(const Data&);
inspect_type(func(i32) -> bool);
```

Parser 仍不查询名称是否表示类型，也不执行编译期求值。它只根据 Token 结构建立唯一 CST；名称绑定和语义分析负责确认类型构造操作数最终准确产生 `type`。

## 2. 不把完整 `type` 作为基础表达式分支

普通表达式不能简单增加一个完整 `type` 备选：

```text
postfixable_primary_expression =
      existing_postfixable_primary_expression
    | type ;
```

该写法会使 `Identifier`、内建类型、圆括号、元组、调用、索引、成员和泛型后缀同时匹配 `expression` 与 `type`，迫使 Parser 保存两棵候选树或依赖符号表。

Ink 改为复用现有表达式结构，只增加具有独特 Token 形状的类型值入口和类型构造后缀。议题 14 保存最终完整的 `postfixable_primary_expression`，直接函数类型则进入封闭分支；本议题直接定义新增分支：

```ebnf
const_type_value_expression =
    "const", postfixable_type_primary ;

function_type_expression =
    function_type ;

direct_function_type_expression =
    [ "const" ], function_type_expression ;
```

实现可以让 `const_type_value_expression` 与议题 29 的类型入口共享可接后缀的 `postfixable_type_primary` 解析器，但不能再次接受第二个前置 `const`。直接函数类型及其可选 `const` 包装使用封闭的 `direct_function_type_expression` 分支，不进入普通 postfix 循环；它在 CST 中仍分别保存 `FunctionTypeExpression` 和存在时的 `ConstTypeValueExpression`。议题 35 把聚合初始化定义为当前 postfix 操作数之后的表达式专用后缀，议题 40 进一步把 `class_type_expression` 加入议题 14 的 `structured_expression`；两者都不把整个 `type` 非终结符直接并入普通表达式。

明确类型位置中的圆括号以及泛型实参等混合位置也只解析一次中性表达式，不再建立 `type_or_expression` 一类重叠分支：

```ebnf
parenthesized_type_expression =
    "(", expression, ")" ;

generic_argument =
      generic_argument_expression
    | generic_list_expansion ;

generic_list_expansion =
    "...", generic_argument_expression ;
```

`generic_argument_expression` 仅在当前泛型实参的顶层排除 `>`、`>=` 和 `>>` 运算，嵌套定界结构内部仍复用普通 `expression`；它仍然建立中性表达式 CST，而不是另设类型分支。这些位置最终是否准确产生 `type` 或满足对应泛型形参，由期望类型和语义分析检查。

普通名称及其后缀继续形成中性表达式节点：

```ink
Data                    // IdentifierExpression
Vector::<i32>           // GenericPostfixExpression
make_type()             // CallExpression
Data[4]                 // BracketPostfixExpression
(i32, String)           // ParenthesizedCommaList
```

新增的确定形状包括：

```ink
const Data              // ConstTypeValueExpression
func(i32) -> bool       // FunctionTypeExpression
async func() -> Data    // FunctionTypeExpression
Data[]                  // EmptyBracketTypeSuffix
Data*                   // PointerTypeSuffix
Data&                   // ReferenceTypeSuffix
class { ... }           // ClassTypeExpression
class Node { ... }      // 带内部递归名称的 ClassTypeExpression
```

## 3. 类型构造尾链

`*` 和 `&` 同时是表达式运算符与类型构造符，不能在普通表达式中无条件加入最高优先级后缀；空 `[]` 虽没有普通索引含义，也需要和后续数组、指针或引用构造形成同一条可重复尾链。普通表达式使用一个带语法谓词的终止型类型构造尾链：

```ebnf
postfix_expression =
      direct_function_type_expression
    | postfixable_primary_expression,
      { expression_postfix_suffix },
      terminal_type_constructor_tail_decision ;

terminal_type_constructor_tail_decision =
      terminal_type_constructor_tail
    | ? no terminal_type_constructor_tail satisfying its final predicate starts at the current position ? ;

expression_postfix_suffix =
      postfix_suffix
    | aggregate_initialization_suffix ;

terminal_type_constructor_tail =
    type_constructor_start,
    { type_symbol_suffix | empty_bracket_suffix | index_suffix },
    ? this is the maximal complete type-constructor tail, it did not split a compound assignment operator, and the next significant Token at the current nesting level belongs to the caller-provided expression terminator set without being consumed ? ;

type_constructor_start =
      type_symbol_suffix
    | empty_bracket_suffix ;

type_symbol_suffix =
    ( "*" | "&" ),
    ? this Symbol is consumed as a type-constructor suffix; an explicit type context may consume adjacent suffix Symbols character by character, while an expression tail additionally obeys terminal_type_constructor_tail, and no compound assignment operator is split ? ;
```

这里的 `postfixable_primary_expression` 表示议题 14 的普通基础表达式以及非函数 `const_type_value_expression`，但排除 `direct_function_type_expression`。函数类型自身的返回 `type` 可以包含完整后缀，封闭限制只阻止后缀越过整个直接函数类型的右边界。议题 35 的聚合初始化后缀只属于普通表达式的 `expression_postfix_suffix`；它可以与普通 `postfix_suffix` 从左到右重复组合，但不进入被明确 `type` 复用的共享后缀集合。

`terminal_type_constructor_tail_decision` 的两个分支是逻辑互补的规范性语法守卫，不是按书写顺序尝试的有序选择：

> 若从当前位置能够消费一条完整最大 `terminal_type_constructor_tail`，且该尾链不拆分复合赋值运算符并在其后立即到达当前子表达式的显式结束符，则正分支成立并必须消费整条尾链，零宽度否定分支不成立；否则正分支不成立，只有零宽度否定分支成立。

因此不能把该决定重新写成 `[ terminal_type_constructor_tail ]`，也不能在实现中允许成功的正分支与无条件空分支同时成立。以 `T*[];` 为例，完整候选尾链 `*[]` 到达 `;`，所以 `T` 后的否定守卫失败；同一 Token 串不能再把 `T` 提前结束为乘法左操作数，并把 `[]` 解释为空数组右操作数。该互斥关系属于接受语言和 CST 归属本身，不依赖递归下降实现先试哪条分支。

具体实现可以把空 `[]` 与其他普通后缀放在同一个循环中；产生式只用于表达“空 `[]` 是类型构造而不是普通索引”。一旦无括号的 `*`、`&` 或空 `[]` 开始构造类型，后面只继续接受 `*`、`&`、`[]` 和用于数组构造的非空 `[expression]`。调用、成员、泛型、切片或其他表达式运算要作用于已构造的完整类型值时，必须先加括号：

```ink
(T*)(value)
(T*).metadata
(T*)::<Argument>
(T*) == OtherType
```

`T*[N]` 是专门保留的类型构造尾链，因此不要求把 `T*` 单独括起来。

## 4. `parseExpression(EndSet)` 与局部试解析

表达式 Parser 的规范入口为 `parseExpression(EndSet)`，调用者必须传入当前入口在同一嵌套层次允许紧随完整表达式的显式结束符。所有语法调用点遵守下表；“继承”表示沿用当前外层调用者的 `EndSet`：

| 表达式调用点 | 规范 `EndSet` |
|---|---|
| 文件、工具 API 或 REPL 的独立完整表达式 | `EOF` |
| 表达式内部各优先级层的一元或中缀操作数 | 继承；当前运算符不加入 `EndSet` |
| `var`、`const` 和字段初始化器，赋值右侧，表达式语句，`defer` 表达式，`return` 表达式 | `;` |
| `throw` 的被抛出表达式 | `from`、`;` |
| 赋值左侧的 `statement_expression` | `=`、`+=`、`-=`、`*=`、`/=`、`%=`、`&=`、`|=`、`^=`、`<<=`、`>>=`，每项均作为不可拆分的完整终端 |
| `if`、`while`、`match` 的圆括号条件或 scrutinee，match condition 的右侧，`parenthesized_expression`，`parenthesized_type_expression` | `)` |
| `if_expression` 或 `generic_argument_if_expression` 的条件 | `)` |
| `if_expression` 或 `generic_argument_if_expression` 的真分支 | `else` |
| `if_expression` 或 `generic_argument_if_expression` 的假分支 | 继承 |
| `for_source` 的第一项 | `..`、`)` |
| `for_source` 中 `..` 后的第二项 | `)` |
| 调用、attribute application、decorator application、函数形参默认值、圆括号逗号列表、tuple type 和 function type 参数列表中的普通表达式或 `list_expansion` 操作数 | `,`、`)` |
| 普通索引内的表达式 | `]` |
| slice 下界 | `:` |
| slice 上界 | `]` |
| 数组字面量的每个元素 | `,`、`]` |
| 泛型实参、`generic_list_expansion` 操作数和泛型形参默认实参的顶层 `generic_argument_expression` | `,`、`>` |
| enum discriminant | `,`、`}` |
| match-expression arm 的表达式 body | `,` |
| 聚合初始化的显式字段值 | `,`、`}` |

逗号列表最后一个元素仍把右定界符纳入集合；这只表示表达式可以在该处结束，不表示列表接受尾随逗号。形参默认值、实参、圆括号逗号列表、数组元素、泛型实参、enum 成员和聚合字段是否允许省略、是否要求逗号以及是否允许尾随逗号，继续由各自外层产生式决定。

表中只列出成功解析所需的语法结束符。批处理错误恢复可以在调用者已经报告缺失 `;`、`)`、`]`、`,` 等错误后，把 `}`、外层 StopSet 或 `EOF` 加入同步集合；这些恢复锚点不能反向成为合法源码的额外结束符，也不能改变在真实语法结束符前的尾链决定。REPL 的 `EOF` 只有在当前入口本来接受完整表达式，或者未闭合结构按第 11 节返回 `Incomplete` 时参与判定。

这些集合由真实外层产生式确定，不能把任意“当前不是中缀运算符”的 Token 当成结束符。标识符、字面量、`(`、`[` 以及一元运算符都可能开始 `*` 或 `&` 的右操作数；`{` 是聚合初始化 postfix 的起点，也不是类型尾链结束符。嵌套括号、方括号、泛型列表或聚合体内使用该嵌套入口自身的集合，不得把更外层的同形 Token 提前当作结束符。

遇到候选 `*`、`&` 或空 `[]` 时，Parser 必须计算 `terminal_type_constructor_tail_decision` 的互补守卫。可以使用如下局部 checkpoint 实现：

```text
1. 保存 Token 游标、临时 CST、诊断缓冲区位置、错误恢复和 REPL 状态；
2. 试解析从当前位置能够形成的最大完整 type constructor tail；
3. 若尾链后的下一显著 Token 属于 EndSet，则正守卫成立，原子提交整个尾链，并禁止零宽度否定分支；
4. 否则正守卫失败，完整回滚所有临时状态，否定守卫成立，并让普通表达式优先级层从原位置解析。
```

“最大完整”表示只要下一个后缀仍能属于同一类型构造尾链，就不能提前提交较短前缀。失败时也不得保留部分后缀、临时 CST 节点或试探诊断。

以下 Token 串必须作为消歧回归测试。前四行各自只允许一棵类型尾链 CST；空白不能把它们改成乘法或按位与。第五行用括号明确要求空数组作为乘法右操作数：

```ink
return T*[];        // SliceType(PointerType(T))
return T*[N];       // ArrayType(PointerType(T), N)
return T&[];        // SliceType(ReferenceType(T))
return T&[N];       // ArrayType(ReferenceType(T), N)
return T * ([]);    // T 乘以空数组值
```

下列用例覆盖不能由通用 `;`、`)`、`,` 推导出的调用点，防止调用者漏传 `from`、`..`、`:`、`else` 或外层右定界符：

```ink
throw T* from cause;
for (const item in T* .. U*) {}
values[T*:U*]
if (condition) T* else U*
Wrapper::<T*[N]>
enum Kind { Pointer = T*, Other }
match (value) { .some => T*, .none => U*, }
Record { field: T* }
```

这些用例中的每条无括号尾链都必须在对应调用点的结束符前提交。若移除该结束符、换成能够开始右操作数的 Token，或者候选尾链后仍有普通 postfix 或中缀操作，则正守卫失败并走普通表达式路径；如果回滚后的普通表达式也不能消费完整 Token 串，才报告语法错误。

例如：

```ink
return T*;        // * 后到达 ;，PointerTypeValue(T)
inspect(T&)       // & 后到达 )，ReferenceTypeValue(T)
Wrapper::<T**>    // ** 后到达 >，Pointer(Pointer(T))

var x = T * n;    // n 不是结束符，乘法
var y = T & mask; // mask 不是结束符，按位与
var z = T**p;     // 试探 ** 后仍有 p，回滚为 T * (*p)
T*&x              // 回滚为 T * (&x)
T[]+x             // 空 [] 无普通索引含义，且尾链未到结束符，语法错误
(T[])+x           // 分组后的切片类型值参与加法；运算是否合法由语义检查
```

赋值入口把完整赋值运算符作为 `EndSet` 中不可拆分的复合终端。类型尾链不得抢走 `*=`、`&=` 等运算符的首字符：

```ink
T*=value  // T *= value
T&=mask   // T &= mask
T* = value // T* 类型值后接普通 =；左侧是否可赋值由语义检查
T& = value // T& 类型值后接普通 =；左侧是否可赋值由语义检查
```

前两行的复合终端要求字符直接相邻；后两行存在 Trivia，因而分别形成类型构造后缀与普通 `=`。同一不可拆分规则适用于全部已登记复合赋值运算符。

空白和注释不参与判定：

```ink
T*;
T * ; // 同样是 T* 类型值
```

`T*(value)` 的 `(` 不是表达式结束符，因而解析为乘法；要调用构造出的类型值必须分组：

```ink
T*(value)   // T * (value)
(T*)(value) // Call(Parenthesized(T*), value)
```

聚合初始化同样是继续作用于当前表达式结果的 postfix 操作，左花括号不属于 `terminal_type_constructor_tail` 的 `EndSet`。因此类型尾链后继续聚合初始化必须分组，而中缀运算右侧的聚合后缀只绑定右操作数：

```ink
T* {}           // 语法错误
(T*) {}         // 对分组后的 T* 应用聚合后缀；语义通常拒绝指针类型
T*(value) {}    // T * AggregateInitialization((value), {})
(T*(value)) {}  // 对完整乘法结果应用聚合后缀
```

复合类型位于后续运算左侧时同样分组：

```ink
(T*) == (U*) // 合法结构，类型是否可比较另行检查
T* == U*     // 第一处 * 不在当前子表达式末尾
```

该规则只需要对短后缀链进行有界试解析，不回溯已经完成的整个表达式，也不查询符号表。泛型应用由连续复合终端 `::<` 明确引导，不再需要用 `<` 的邻接或试解析在泛型应用与比较运算之间消歧；`::<` 内部三个字符不得包含 Trivia，左侧表达式与该复合终端之间可以存在 Trivia，格式化器规范输出时不留空格。

## 5. `const` 类型值

普通表达式入口中的 `const` 只表示议题 29 的前置类型限定，不是可以冻结任意运行时值的通用一元运算符：

```ink
const ReadOnlyPointer: type = const Data*;
inspect_type(const Data&);
```

声明开头和初始化表达式中的两个 `const` 由语法上下文区分：

```ink
const ReadOnlyPointer: type = const Data*;
// ^ binding declaration          ^ type qualifier
```

前置限定最多出现一次：

```ink
const const Data // 语法错误
```

Parser 统一允许 `const` 后出现任意结构合法的 `type_body`。非函数目标复用可接后缀的 `postfixable_type_primary` 分支；直接函数类型进入封闭分支。限定目标是否有意义由语义分析在别名展开、泛型替换和计算类型求值后判断：

```ink
const Data*          // 语法和语义均合法
const func() -> void // 语法合法，语义错误
const void           // 语法合法，语义错误
```

因此直接写出的函数类型与名称最终解析成函数类型时使用同一语义检查，不为字面形状建立不完整的 Parser 特例。

## 6. 函数类型值

`func(` 和 `async func(` 在普通表达式入口中直接开始议题 29 的函数类型：

```ink
const Predicate: type = func(i32) -> bool;
const Loader: type = async func(Path&) -> Data;

inspect_type(func(i32, String) -> bool);
inspect_type(async func() -> Data);
```

函数声明入口的 `func identifier` 与函数类型的 `func (` 由 Token 形状区分。Ink v0 不使用相同开头定义匿名函数表达式。

函数参数列表后的 `->` 一旦出现就提交为返回类型子句，不能回退为对整个函数类型的 `pointer_member_suffix`。箭头右侧继续递归并最大化地消费完整 `type`：

```ink
func() -> Data*        // 返回 Data* 的函数类型
func() -> func(i32)    // 返回另一个函数值的函数类型
(func() -> Data)*      // 后缀作用于整个函数类型
```

直接函数类型是封闭表达式分支，因此 `func() -> Data*` 中的 `*` 唯一属于返回类型，`func()*` 是语法错误。需要对整个函数类型构造指针、引用、数组或应用其他后缀时，必须先写成 `(func())*`、`(func())&` 等加括号形式。

`[nothrow]` 仍是声明属性，不进入函数类型值：

```ink
[nothrow] func(i32) -> bool // 不是函数类型表达式
```

## 7. 空方括号、数组构造与普通索引

普通表达式现在接受空的后缀 `[]`，但它只表示安全切片类型构造：

```ink
Data[]   // 语法合法，切片类型
values[] // 语法合法；values 不产生 type 时语义错误
```

非空 `[expression]` 继续使用议题 15、29 的中性 `BracketPostfixSuffix`：

```ink
Data[4]        // Data 是 type：固定数组类型
values[index]  // values 是运行时容器：普通索引
table[key]     // Map 索引
TypeTable[i]   // 编译期集合索引
```

Parser 不因空索引看起来不适用于某个名称而拒绝节点；语义分析分别检查：

- 空 `[]` 的左侧必须在编译期产生 `type`；
- 非空 `[expression]` 的左侧若产生 `type`，则构造固定数组并要求长度为合法编译期整数；
- 否则左侧必须提供内建或用户定义的索引能力；
- 编译期容器索引可以产生 `type` 并继续参与类型构造。

容器索引不受空 `[]` 规则影响：

```ink
var values: Vector::<i32>;
var value = values[index];

var users: Map::<String, User>;
var user = users[name];
```

`T*[]` 优先形成类型构造尾链。由于空白不消歧，乘以空数组值需要括号：

```ink
T*[]     // (T*)[]
T * []   // 同样是 (T*)[]
T * ([]) // 乘以空数组值
```

## 8. 元组类型与类型值元组

普通表达式中的圆括号逗号结构使用中性 `ParenthesizedCommaList` CST。Parser 不根据元素名称判断它是元组类型还是包含 `type` 值的普通元组：

```ink
const Pair: type = (i32, String);
const Types = (i32, String);
```

语义 elaboration 使用期望类型决定：

- 期望结果为 `type` 时，构造元组类型；
- 期望结果为 `(type, type)` 等普通元组类型时，构造包含类型值的元组；
- 没有期望类型时，默认按照普通元组值推导，`(i32, String)` 的类型为 `(type, type)`。

例如：

```ink
const Pair: type = (i32, String); // 一个元组类型值
const Types = (i32, String);      // 一个 (type, type) 元组值

func make_pair_type() -> type {
    return (i32, String);         // return 期望 type
}

func inspect_type(value: type);
inspect_type((i32, String));      // 元组类型

func inspect_types(values: (type, type));
inspect_types((i32, String));     // 普通元组值
```

明确类型入口天然提供 `type` 期望：

```ink
var pair: (i32, String);
```

`()`、`(T,)`、`(...Types)` 和多元素逗号列表继续保留议题 07、14、29 的源码形状规则；本节只统一其 CST 和上下文解释。

## 9. 调用与构造共用 CST

类型调用不增加 Parser 专用构造节点：

```ink
Point(10, 20)
factory(10, 20)
SelectedType(10, 20)
```

三者统一形成：

```text
CallExpression {
    callee,
    arguments
}
```

语义分析在名称绑定和编译期求值后区分：

- callee 是函数值：普通调用；
- callee 是 `type`：按照议题 06 进行显式构造函数调用；
- 两者都不是：语义错误。

泛型类型同样复用普通后缀：

```ink
Vector::<i32>(capacity)
Map::<String, User>()
```

Parser 不建立 `ConstructorExpression`，也不通过构造函数表改变调用 CST。解析后的语义节点可以记录已选择的函数或构造函数候选。

## 10. CST 与语义边界

建议复用或增加以下 CST 节点：

```text
IdentifierExpression
BuiltinTypeExpression
ParenthesizedExpression
ParenthesizedCommaList
CallExpression
BracketPostfixSuffix
ConstTypeValueExpression
FunctionTypeExpression
EmptyBracketTypeSuffix
PointerTypeSuffix
ReferenceTypeSuffix
```

Parser 不负责：

- 判断名称、调用、成员或索引结果是否为 `type`；
- 判断 `const` 是否能限定最终类型；
- 区分普通调用和构造调用；
- 区分非空方括号的数组构造、运行时索引或编译期索引；
- 根据元素内容猜测圆括号逗号列表是元组类型还是元组值；
- 验证指针、引用、数组、切片和函数类型组合是否合法。

这些决定发生在名称绑定、期望类型 elaboration、编译期求值和类型分析阶段，不能反向改写已经确定的 Token 归属和表达式结合结构。

## 11. 恢复和 REPL

类型尾链试解析必须隔离临时诊断，不能在回滚为乘法或按位与以后残留“缺少类型后缀”诊断。推荐返回三态：

```text
Matched     成功到达当前表达式结束符，提交类型尾链
NotMatched 回滚并进入普通运算符解析
Incomplete 已开始能够继续闭合的类型结构，但在 REPL EOF 处结束
```

例如 `T*[`、`const (`、`func(i32` 和 `async func() ->` 在 REPL 中若只缺少可继续输入的结束部分，应报告 `Incomplete`。已经遇到明确冲突 Token 的结构按议题 03 建立 `ErrorNode` 或 `MissingToken`。

实现可以把候选后缀先保存在临时小数组中，只有试探成功才物化 CST；也可以使用支持游标和诊断事务的 Parser checkpoint。无论内部表示如何，失败试探不能重复消费或丢失真实 Token。

## 12. 确认结论

Ink 允许完整复合类型作为普通一等编译期值。表达式文法不把整个 `type` 增加为重叠分支，而是复用标识符、圆括号、调用、索引、成员和以连续 `::<` 引导的泛型等中性表达式结构，并只增加 `const`、同步或异步函数类型、Parser 议题 40 的 class 类型表达式、空 `[]` 以及受终止位置约束的 `*`、`&` 类型构造。

无括号 `*`、`&` 类型尾链必须结束于当前子表达式边界；继续调用、访问、聚合初始化或参与运算时使用括号。空 `[]` 只构造切片类型，非空方括号继续由语义区分数组构造和容器索引。元组逗号结构由 CST 中性保存，并根据期望类型区分元组类型与包含类型值的普通元组。类型调用与普通函数调用共享 `CallExpression`，聚合初始化同样只在语义阶段要求左侧结果为支持该初始化形式的 `type`，最终含义统一由语义分析确定。
