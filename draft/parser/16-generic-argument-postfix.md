# Parser 议题 16：显式泛型实参后缀

> 状态：已确认，Ink v0 使用 `name::<arguments>` 显式泛型应用；泛型声明仍使用 `name<parameters>`；议题 17 补充切片后缀，议题 29 补充列表展开，议题 30 允许泛型实参表达式使用复合类型值，议题 35 增加表达式专用聚合初始化后缀
> 确认日期：2026-08-03

## 1. 泛型实例化是后缀操作

显式泛型实例化使用最高优先级的泛型实参后缀，并参与普通后缀链：

```ebnf
postfix_expression =
      direct_function_type_expression
    | postfixable_primary_expression,
      { expression_postfix_suffix },
      terminal_type_constructor_tail_decision ;

expression_postfix_suffix =
      postfix_suffix
    | aggregate_initialization_suffix ;

postfix_suffix =
      ordinary_postfix_suffix
    | generic_argument_suffix
    | slice_suffix ;

generic_argument_suffix =
    "::<", [ generic_argument_list ], ">" ;

generic_argument_list =
    generic_argument, { ",", generic_argument } ;

generic_argument =
      generic_argument_expression
    | generic_list_expansion ;

generic_list_expansion =
    "...", generic_argument_expression ;

generic_argument_expression =
      generic_argument_if_expression
    | generic_argument_logical_or_expression ;

generic_argument_if_expression =
    "if", "(", logical_or_expression, ")",
    generic_argument_expression, "else", generic_argument_expression ;

generic_argument_logical_or_expression =
    generic_argument_logical_and_expression,
    { "||", generic_argument_logical_and_expression } ;

generic_argument_logical_and_expression =
    generic_argument_comparison_expression,
    { "&&", generic_argument_comparison_expression } ;

generic_argument_comparison_expression =
    generic_argument_bitwise_or_expression,
    [ generic_argument_comparison_operator, generic_argument_bitwise_or_expression ] ;

generic_argument_comparison_operator =
      "<" | "<=" | "==" | "!=" ;

generic_argument_bitwise_or_expression =
    generic_argument_bitwise_xor_expression,
    { "|", generic_argument_bitwise_xor_expression } ;

generic_argument_bitwise_xor_expression =
    generic_argument_bitwise_and_expression,
    { "^", generic_argument_bitwise_and_expression } ;

generic_argument_bitwise_and_expression =
    generic_argument_shift_expression,
    { "&", generic_argument_shift_expression } ;

generic_argument_shift_expression =
    additive_expression,
    { "<<", additive_expression } ;
```

其中 `postfixable_primary_expression` 与封闭的 `direct_function_type_expression` 由议题 14 定义，`ordinary_postfix_suffix` 由议题 15 定义，`slice_suffix` 由议题 17 定义，`terminal_type_constructor_tail_decision` 由议题 30 定义为正尾链与零宽度否定守卫的互斥选择。泛型后缀只进入可后缀基础表达式分支，并可以与调用、索引、切片和成员访问继续组合；直接函数类型要应用泛型或其他后缀仍须先加括号。

```ink
Vector::<i32>
create::<i32>(10)
object.convert::<T>().field
&create::<i32>
Map::<String, Vector::<i32>>
Vector::<Data*>
Other::<Header, ...Types, Footer>
```

议题 30 已将类型纳入普通表达式值，因此 `generic_argument` 不再并列选择 `type` 和 `expression`，也不再存在 `type_or_expression`。`generic_argument_expression` 复用普通表达式从逻辑或到加法及以下的同一结构，但在当前泛型列表顶层排除 `>`、`>=` 和 `>>`；需要这些运算时按第 7 节加括号。类型实参、整数、布尔值和其他编译期值统一建立 Expression CST；目标泛型形参最终要求哪一类编译期值，由语义分析检查。

类型构造尾链把当前泛型实参的顶层 `>` 作为显式表达式结束符，因此 `Wrapper::<T**>` 可以在不查询 `T` 的情况下确定为双重指针类型实参。若完整最大尾链能够到达 `>`，议题 30 的正守卫必须提交它，不能同时走零宽度分支；因此 `Wrapper::<T*[N]>` 也唯一形成数组类型实参，而不是 `T * [N]`。若 `*` 或 `&` 后仍有当前实参内的普通操作数，则正守卫失败，局部试探回滚并继续按本议题及表达式优先级解析。

Parser 不要求泛型后缀后面立刻出现调用。闭合函数、闭合类型和其他闭合声明都可以作为后续语义允许的值或编译期实体。

## 2. 明确引导符 `::<`

泛型后缀以 terminal string `"::<"` 开始。Tokenizer 仍产生两个 `Symbol(':')` 和一个 `Symbol('<')` Token，但三个 Token 必须在源码中依次直接相邻；`::<` 也属于议题 02 的全语言复合符号集合：

```ink
create::<i32>(10) // 泛型后缀
create:: <i32>    // 非法，不存在连续的 "::<"
create: :<i32>    // 非法，Trivia 打断复合终端
```

`::<` 是一个完整 syntactic term。根据议题 02、04 的显著 Token 视图，左侧表达式与整个 `::<` 终端之间可以存在普通 Trivia，泛型列表内部也可以存在 Trivia：

```ink
create ::<i32>(10)                  // 语法合法
Map::< String, Vector::<i32> >      // 语法合法
```

格式化器的规范输出统一移除左侧与 `::<` 之间的 Trivia，写成 `name::<arguments>`。

## 3. 确定性语法判定

后缀位置看到完整 `::<` 时，Parser 立即提交到 `generic_argument_suffix`。后续缺少实参、逗号或结束 `>` 时，应在泛型后缀内部按议题 03 恢复，不能回退并把其中的 `:` 或 `<` 改作其他表达式结构。

未出现 `::<` 时，单独的 `<` 始终按照议题 12 作为比较运算符处理；Parser 不再检查 `<` 是否紧邻左侧，不扫描后续 Token 猜测平衡泛型列表，也不为泛型与比较建立 checkpoint：

```ink
ordinary_value::<i32> // 语法上是泛型后缀，语义阶段可能拒绝
ordinary_value<i32    // 比较表达式，与是否存在同名泛型声明无关
```

该判定完全由 `::<` 终端字符串决定，不查询名称绑定、类型或重载集合。

## 4. 与比较和移位的边界

采用明确引导符后，紧凑源码不再产生泛型与比较两种候选 CST：

```ink
a<b>>c       // a < (b >> c)
a::<b>>c     // a::<b> > c
a::<b>>>c    // a::<b> >> c
```

前一例没有 `::<`，所以空白不会改变 `<` 的语法角色。后两例已经进入泛型列表，列表顶层先逐字符消费一个 `>` 作为结束符；剩余的一个 `>` 或 `>>` 再按照议题 12 形成比较或移位运算。

因此合法程序的 CST 只由 Token、Trivia 对复合终端内部邻接的影响以及语法定界决定，不依赖左侧名称是否绑定到泛型声明。

## 5. 空实参列表与默认值

空泛型实参列表合法：

```ink
Generic::<>
parse::<>(text)
```

它表示显式请求该开放泛型，并使用所有可省略的默认编译期实参。它不会触发议题 64 已禁止的泛型实参推导。

非空泛型实参列表不允许尾随逗号：

```ink
Pair::<i32, String>  // 合法
Pair::<i32, String,> // 非法
```

每个普通实参统一是 `generic_argument_expression`。议题 30 让类型也可以作为表达式产生的一等编译期值，因此仍可以传入类型、整数、布尔值、声明句柄或其他泛型形参允许的编译期值。`generic_list_expansion` 在绑定前把一个编译期序列展开为零个或多个实参：

```ink
Other::<...Types>
Other::<Header, ...select_types(), Footer>
```

`...generic_argument_expression` 是列表元素而不是普通一元表达式。表达式是否能在编译期产生可展开序列，以及展开元素能否依次满足目标泛型形参，由语义分析检查。

Parser 不在 `type` 与 `expression` 之间选择，也不建立依赖分支顺序的两种候选 CST。`generic_argument_expression` 只是具有明确顶层结束符限制的 Expression 入口；语义阶段按照目标泛型形参要求检查每个结果。

## 6. 嵌套结束符与 `>>`

Tokenizer 始终把每个 `>` 产生为一个单独的 Symbol Token。Parser 在泛型上下文中逐层消费结束符：

```ink
Map::<String, Vector::<i32>>
```

末尾两个相邻的 `>` 分别关闭 `Vector::<i32>` 和外层 `Map::<...>`，不是右移运算符。表达式上下文中的同样两个 Token 仍可按照议题 02、12 组成 `>>`。

该规则也适用于更多嵌套层次，不需要 Tokenizer 重新分词，也不产生合并后的“泛型关闭 Token”。

## 7. 实参中的 `>` 与 `>>`

泛型列表顶层的 `>` 优先作为当前列表结束符。泛型实参自身需要使用 `>`、`>=` 或 `>>` 运算时，必须用括号把该表达式完整包围：

```ink
Predicate::<(N > 0)>
Predicate::<(N >= 0)>
Array::<i32, (N >> 1)>
```

下列形式不作为上述运算表达式解析：

```ink
Predicate::<N > 0>
Array::<i32, N >> 1>
```

规范 EBNF 通过专用 `generic_argument_expression` 分层，在顶层比较运算符中排除 `>`、`>=`，并在顶层移位运算符中排除 `>>`。等价的递归下降实现可以使用以 `,` 和 `>` 为 EndSet 的定界表达式入口。顶层看到 `>` 时先结束当前实参，再逐字符关闭泛型列表；括号、调用实参、索引等嵌套定界结构内部按照普通表达式文法解析，因而仍按全语言最长匹配识别 `>`、`>=` 或 `>>`。右括号之后的 `>` 再关闭泛型实参列表。

泛型实参中的嵌套泛型后缀仍由明确的 `::<` 引导，并按相同定界规则处理。整个过程不需要名称绑定或任意回溯。

## 8. 声明和类型上下文

泛型声明的形参列表继续使用 `name<parameters>`，因为声明产生式已经明确要求形参而不是应用。所有泛型应用则统一使用本议题的 `name::<arguments>`，不因普通表达式或明确类型上下文而改变拼写：

```ink
func create<T: type>() -> T;
const entry = &create::<i32>;
var values: Vector::<i32>;
reflect(Vector::<i32>);
```

`create<T: type>` 中的 `<...>` 是声明形参子句；`create::<i32>` 和 `Vector::<i32>` 中的 `::<...>` 是应用后缀。旧的 `name<arguments>` 不再作为泛型应用接受。语法允许左侧与整个 `::<` 终端之间出现 Trivia，但格式化器规范输出始终为紧贴的 `name::<arguments>`。

## 9. 求值与实例化

泛型实参按照源码从左到右绑定和执行编译期求值。全部显式实参和省略的默认值完成绑定后，才请求规范化的闭合实例。

泛型后缀本身不产生运行时求值顺序上的隐藏操作。闭合声明随后参与调用、取地址、反射或其他后缀时，继续使用议题 13、15 的普通顺序。

## 10. CST 与恢复

泛型实参后缀 CST 保留引导 `::<` 的两个冒号和一个 `<`、全部 Expression 实参、列表展开节点、逗号、结束 `>` 和 Trivia。所有字符仍是各自的真实 Symbol Token；嵌套的相邻 `>` 也不能合并。

识别完整 `::<` 后即提交到泛型后缀。缺失实参、分隔逗号或结束 `>` 时，Parser 在该节点内部插入 `MissingToken` 或建立 `ErrorNode`，不能回退到比较表达式。没有 `::<` 的单独 `<` 始终留给比较层。`name:: <T>` 因 Trivia 打断复合终端，不形成泛型后缀；具体局部诊断措辞留给独立 diagnostics draft。

## 11. 确认结论

Ink v0 使用 `name::<arguments>` 泛型应用形式，泛型声明仍使用 `name<parameters>`。复合终端 `::<` 的三个 Symbol 必须直接相邻，看到它即提交到泛型后缀，不再对比较 `<` 做邻接检查、平衡扫描或投机回溯。普通实参统一按 `generic_argument_expression` 解析，也可以使用列表级 `generic_list_expansion` 展开；空 `::<>` 显式采用默认实参，非空源码列表不允许尾随逗号。专用顶层表达式层排除 `>`、`>=`、`>>`，嵌套 `>>` 在泛型上下文中逐个关闭列表，需要这些运算时必须加括号。整个解析不依赖名称绑定。
