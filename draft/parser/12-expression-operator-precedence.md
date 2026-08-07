# Parser 议题 12：表达式运算符优先级

> 状态：已确认，议题 19 补充一元表达式完整规则，议题 22 在逻辑或之外增加 `if_expression`，议题 30 补充终止型类型构造后缀；2026-08-05 为 `if_expression` 条件增加固定括号；2026-08-06 移除 `then`
> 确认日期：2026-08-03

## 1. 固定优先级

Ink 使用固定的表达式运算符优先级。由高到低为：

```text
1.  后缀：调用 ()、索引 []、成员访问 .、指针成员访问 ->，以及议题 30 的类型构造尾链
2.  一元：+  -  !  ~  *  &  await
3.  乘除余：*  /  %
4.  加减：+  -
5.  移位：<<  >>
6.  按位与：&
7.  按位异或：^
8.  按位或：|
9.  比较：<  <=  >  >=  ==  !=
10. 逻辑与：&&
11. 逻辑或：||
```

该顺序有意让按位运算高于比较运算。例如：

```ink
value & mask == 0
```

解析为：

```ink
(value & mask) == 0
```

而不是 C 风格中容易误读的 `value & (mask == 0)`。

## 2. 分层 EBNF

表达式优先级使用议题 04 确认的标准 EBNF 表示：

```ebnf
expression =
      if_expression
    | logical_or_expression ;

if_expression =
    "if", "(", logical_or_expression, ")",
    expression, "else", expression ;

logical_or_expression =
    logical_and_expression,
    { "||", logical_and_expression } ;

logical_and_expression =
    comparison_expression,
    { "&&", comparison_expression } ;

comparison_expression =
    bitwise_or_expression,
    [ comparison_operator, bitwise_or_expression ] ;

comparison_operator =
    "<" | "<=" | ">" | ">=" | "==" | "!=" ;

bitwise_or_expression =
    bitwise_xor_expression,
    { "|", bitwise_xor_expression } ;

bitwise_xor_expression =
    bitwise_and_expression,
    { "^", bitwise_and_expression } ;

bitwise_and_expression =
    shift_expression,
    { "&", shift_expression } ;

shift_expression =
    additive_expression,
    { ( "<<" | ">>" ), additive_expression } ;

additive_expression =
    multiplicative_expression,
    { ( "+" | "-" ), multiplicative_expression } ;

multiplicative_expression =
    unary_expression,
    { ( "*" | "/" | "%" ), unary_expression } ;

unary_expression =
      postfix_expression
    | unary_operator, unary_expression ;

unary_operator =
    "+" | "-" | "!" | "~" | "*" | "&" | "await" ;
```

`postfix_expression` 的调用、索引和成员访问细节由后续议题定义。本节只规定它整体高于一元运算。议题 30 进一步允许 `*`、`&` 在当前子表达式结束位置形成类型构造尾链；试探不成功时，同一 Token 仍分别进入本节的一元、乘法或按位与层。

```ink
return T*; // PointerTypeValue(T)
T * value  // 乘法
T**value   // T * (*value)
```

该语法谓词不改变普通运算符优先级，也不使用空白或符号表消歧。

## 3. 结合性

乘除余、加减、移位、按位与、按位异或、按位或、逻辑与和逻辑或均为左结合：

```ink
a - b - c       // (a - b) - c
a << b << c     // (a << b) << c
a && b && c     // (a && b) && c
```

一元运算通过递归产生式从右侧组合：

```ink
!*pointer       // !(*pointer)
~-value         // ~(-value)
```

括号可以显式改变默认组合方式：

```ink
a * (b + c)
```

## 4. 比较运算不结合

六种比较运算处于同一优先级，并且整个比较层最多只能出现一个比较运算符：

```ink
min < value
left == right
```

连续比较非法：

```ink
min < value < max
a == b == c
a < b == flag
```

需要用逻辑运算明确表达：

```ink
min < value && value < max
a == b && b == c
```

如果确实需要比较某个比较结果，必须使用括号显式建立内部比较；其类型是否合法仍由类型检查决定。

## 5. 短路逻辑运算

`&&` 和 `||` 是短路逻辑运算：

```ink
ready && perform()
cached || load()
```

- `&&` 先计算左操作数；左侧为 `false` 时不计算右操作数；
- `||` 先计算左操作数；左侧为 `true` 时不计算右操作数。

按位 `&` 和按位 `|` 不具有短路语义。它们与 `&&`、`||` 是不同的运算符。

## 6. 不属于表达式优先级的语法

根据议题 11，赋值只是一条无结果语句，因此 `=`、`+=` 等赋值运算符不进入表达式优先级。

Ink 不提供逗号运算符，也不提供 `?:` 条件运算符。需要条件值时使用议题 21、22 的无块 `if_expression`。它位于逻辑或表达式之外，是完整表达式文法的最低层；作为其他运算的操作数时必须加括号。

## 7. 连续符号 Token

Tokenizer 为每个符号字符产生一个 Token。以下复合运算符只有在构成它们的 Symbol Token 直接相邻时才能由 Parser 识别：

```text
<=  >=  ==  !=  <<  >>  &&  ||  ->
```

例如：

```ink
a <= b // 比较运算
a < = b // 不是 <=
```

## 8. Parser 与 CST

规范文法使用上述分层 EBNF，保证优先级和结合性没有歧义。Parser 实现可以使用 Pratt Parser、precedence climbing 或等价的递归下降结构，但必须产生相同的表达式树。

CST 保留每个单字符 Symbol Token、括号以及其中的 Trivia。实现识别出的复合运算符可以作为 CST 节点类别或解析结果记录，但不能合并或丢弃原始 Token。

本议题只确定表达式的结构与短路边界；除 `&&`、`||` 必需的短路顺序外，其他表达式子项的运行时求值顺序由后续议题统一规定。

## 9. 确认结论

Ink 采用固定且无歧义的表达式优先级。按位运算高于比较运算，全部比较运算处于同一级且不可连续组合；算术、移位、位运算和逻辑运算左结合，一元运算从右侧组合。议题 30 的类型构造尾链只在当前子表达式显式结束位置优先于同形运算符，并不改变其他表达式的优先级。无块 `if_expression` 位于逻辑或之外的最低层；赋值、逗号运算符和 `?:` 不属于 Ink 表达式。
