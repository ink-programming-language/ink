# Tokenizer 议题 09：单字符 Symbol Token

> 状态：已确认，根据后续讨论修订为单字符模型；Parser 议题 24 使用 `=>`；Parser 议题 16 使用 `::<` 引导泛型实参后缀
> 确认日期：2026-08-02

## 1. 所有语法符号按单字符扫描

Tokenizer 不识别复合运算符、复合标点或符号的语法用途。每个合法 ASCII 语法符号独立产生一个 `Symbol` Token：

```text
Symbol {
    value: ASCII character
    span: one-byte source range
    raw: exact one-byte source slice
}
```

例如：

```ink
<=
```

始终产生：

```text
Symbol('<')
Symbol('=')
```

不存在 `LessEqual` TokenKind。

## 2. 合法 Symbol 字符表

Ink 当前接受以下单字符 Symbol：

```text
( )
{ }
[ ]

, ;
: . @

+ - * / %
= !
& | ^ ~
< >
```

所有字符使用一个统一 `Symbol` TokenKind，由 `value` 保存准确字符。Tokenizer 不区分：

- 开始定界符与结束定界符；
- 一元、二元和赋值运算符；
- 类型构造符号与表达式运算符；
- 普通点号、范围点号和展开点号；
- 返回箭头与指针成员箭头的组成部分。

这些差异全部由 parser 根据单字符 Token 序列和语法上下文确定。

## 3. 复合标点由 Parser 识别

以下源码只产生单字符 Symbol：

```text
::   → ':' ':'
::<  → ':' ':' '<'
..   → '.' '.'
...  → '.' '.' '.'
->   → '-' '>'
=>   → '=' '>'
```

示例：

```ink
library::ThreadPool
0 .. length
target::<...Types>(...values)
func read() -> Data*;
.ready => run();
```

Parser 在需要的位置识别连续字符序列。Tokenizer 不产生 `ColonColon`、`DotDot`、`Ellipsis`、`Arrow` 或 `FatArrow`。Parser 议题 24 的 `match` 分支箭头因此保存为两个直接相邻的真实 Symbol Token。

## 4. 复合运算符同样由 Parser 识别

运算符源码也保持单字符：

```text
==   → '=' '='
!=   → '!' '='
<=   → '<' '='
>=   → '>' '='
&&   → '&' '&'
||   → '|' '|'
<<   → '<' '<'
>>   → '>' '>'
+=   → '+' '='
>>=  → '>' '>' '='
```

Tokenizer 不判断某个符号是一元、二元、复合赋值或泛型定界符。

运算符的合法拼写、优先级、结合性和语义属于 parser 与后续语义规则，不属于 Tokenizer TokenKind 表。

## 5. 源码连续性是组合条件

Parser 只有在 Symbol Token 的源码跨度首尾连续时，才能把它们识别为复合语法：

```text
tokens[i].end_byte == tokens[i + 1].start_byte
```

因此：

```ink
a <= b
a < = b
a < /* comment */ = b
```

第一行包含连续的 `<`、`=`，可以由表达式 parser 识别为 `<=`。后两行的 Symbol 之间存在 Trivia Token，不能组合成 `<=`。

即使 parser 使用“跳过 Trivia”的语法视图，也必须保留原始 Token 索引或跨度邻接信息，不能跨 Trivia 重组复合符号。

## 6. `::<` 与嵌套泛型不需要 Token 拆分

显式泛型实参后缀由三个直接相邻字符 `::<` 引导。Tokenizer 对它仍产生三个单字符 Symbol；Parser 把完整连续拼写识别成一个复合终结字符串。左侧表达式与 `::<` 之间可以存在 Trivia，格式化器的规范输出不保留该空白：

```ink
Vector::<i32>
Vector ::<i32> // 语法等价，规范格式为 Vector::<i32>
Vector:: <i32> // 非法：::< 内部不能出现 Trivia
```

源码：

```ink
Vector::<Vector::<i32>>
```

末尾始终产生两个独立 Token：

```text
Symbol('>')
Symbol('>')
```

泛型 parser 可以直接把它们解释为两个结束定界符；表达式 parser 可以在相应上下文把相同的连续字符解释为右移运算符。

Tokenizer 不产生 `ShiftRight`，因此不存在先合并再虚拟拆分的特殊规则。

同理：

```ink
Container::<Item::<T>>=value
```

产生三个末尾 Symbol：

```text
'>' '>' '='
```

如何按当前语法状态消费由 parser 决定，原始 Token 列表保持统一。

## 7. 数字与点号边界

议题 05 仍负责判断 `.` 是否属于十进制 FloatLiteral。只有数字扫描器已经确认 `.` 后紧跟十进制数字时，该点才进入数字 Token。

否则每个点独立产生 `Symbol('.')`：

```ink
1.member
1..10
0...value
```

分别形成概念序列：

```text
IntegerLiteral('1') '.' Identifier('member')
IntegerLiteral('1') '.' '.' IntegerLiteral('10')
IntegerLiteral('0') '.' '.' '.' Identifier('value')
```

Parser 决定两个点是范围、三个点是展开，或某个序列在当前位置非法。

## 8. 注释定界优先

`//` 和 `/*` 是决定 Token 边界的词法结构，因此在普通 `/` Symbol 回退之前识别：

```text
"//" → one LineComment Trivia Token
"/* ... */" → one BlockComment Trivia Token
```

它们不是两个 Symbol Token。

存在 Trivia 或其他分隔时，斜杠各自成为 Symbol：

```ink
/ /
/ *
```

块注释之外出现的 `*/` 不结束任何状态，按普通单字符规则产生 `Symbol('*')` 和 `Symbol('/')`；其语法是否合法由 parser 诊断。

## 9. 字符串和 ScalarLiteral 定界优先

单引号、双引号以及 `r"..."` 前缀由议题 06—08 的字面量扫描器识别。合法完整字面量产生一个 Token，而不是把引号逐个产生为 Symbol。

这不属于符号语义判断，而是确定字面量 Token 原始边界所必需的词法状态。

同理，数字中的小数点、指数符号和类型后缀在数字扫描器确认属于同一个数字 Token 时不会另外产生 Symbol 或 Identifier。

## 10. Parser 对符号赋予语义

相同 Symbol 在不同上下文可以具有不同含义：

```ink
a * b       // multiplication
value: T*   // pointer type
*pointer    // dereference

a & b       // bitwise AND
value: T&   // reference type
&value      // address/reference expression

a < b         // comparison
Vector::<i32> // generic delimiters
```

Tokenizer 对这些示例中的 `*`、`&`、`<`、`>` 产生完全相同的 `Symbol` TokenKind 和字符值。

括号配对、属性方括号、列表逗号、分号位置和 `@` 装饰器同样由 parser 解释。

## 11. Full-Fidelity

每个 Symbol Token 的 `raw` 是准确的单个 ASCII 字节。复合符号没有合并后的 Token，因此 full-fidelity 列表天然保留每个字符的独立跨度。

Parser 可以建立覆盖多个相邻 Symbol 的语法节点，但不能修改、替换或删除底层 Token。格式化器仍从完整 Token 列表逐字节恢复源码。

## 12. 非法字符

不在合法 Symbol 表、Identifier、数字、字面量、注释或 Trivia 规则中的源码字符产生错误 Token。Tokenizer 可以报告“当前字符不能开始任何 Token”，但不猜测用户想写哪种复合运算符。

错误 Token 同样保存准确 `raw` 并参与源码连续分区。
