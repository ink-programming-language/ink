# Parser 议题 04：标准 EBNF 记法

> 状态：已确认，非终结符统一使用 `snake_case`  
> 确认日期：2026-08-03

## 1. 采用标准

Ink 的语法规则使用 ISO/IEC 14977 风格的标准 Extended BNF（EBNF）描述。Parser draft 不定义 `KW(...)`、`SYM(...)`、`SEQ(...)`、后缀 `?`、后缀 `*` 或后缀 `+` 等自定义元语法。

EBNF 描述语言接受哪些 Token 序列；Parser 的递归下降、表驱动、生成器或其他实现方式不属于语法规范。

## 2. 基本记号

本 draft 使用以下标准 EBNF 结构：

```ebnf
declaration = function_declaration | class_declaration ;

qualified_name = identifier, { ".", identifier } ;

optional_alias = [ "as", identifier ] ;

binding_declaration = ( "let" | "var" ), identifier ;
```

其含义为：

- `=` 定义一个非终结符；
- `;` 结束一条产生式；
- `,` 表示顺序连接；
- `|` 分隔可选分支；
- `[ ... ]` 表示整体出现零次或一次；
- `{ ... }` 表示整体出现零次或多次；
- `( ... )` 只用于分组；
- `n * primary` 表示准确重复 `n` 次；
- `(* ... *)` 是 EBNF 规范自身的注释；
- `? ... ?` 是标准 EBNF special sequence，仅用于引用 Tokenizer 已经定义、但不适合在 Parser 文法中按字符重写的 Token 类别。

至少出现一次写为一个元素后接它的重复，而不使用后缀 `+`：

```ebnf
one_or_more_identifiers = identifier, { identifier } ;
```

## 3. 非终结符命名

非终结符使用小写英文单词和下划线组成的 `snake_case` meta-identifier。非终结符名称内部不能出现空格：

```ebnf
function_declaration
parameter_list
return_type
```

同一个 meta-identifier 在所有 Parser 议题中保持相同含义。尚未定义的非终结符可以在前置示例中被引用，但必须在后续对应语法议题中完成定义。

## 4. 终结字符串

双引号中的 terminal string 表示 Ink 源码中的准确拼写：

```ebnf
"func"
"("
"->"
```

Tokenizer 已经完成 Keyword、BuiltinType、Identifier 和 Symbol 的分类，因此：

- `"func"` 匹配拼写为 `func` 的 Keyword Token；
- `"i32"` 匹配拼写为 `i32` 的 BuiltinType Token；
- `"("` 匹配一个 `Symbol('(')` Token；
- `"->"` 匹配直接相邻的 `Symbol('-')` 与 `Symbol('>')` 两个 Token。

一个 terminal string 内的字符必须在源码中直接相邻，不能跨 Trivia。不同 EBNF syntactic term 之间则通过议题 02 的显著 Token 视图匹配，可以存在 Trivia。

因此：

```ebnf
return_type = "->", type ;
```

接受：

```ink
-> i32
```

但不把以下源码中的两个 Symbol 识别为 `"->"`：

```ink
- > i32
```

如果语法确实允许两个符号之间存在 Trivia，产生式必须写成两个 terminal string：

```ebnf
separated_symbols = "-", ">" ;
```

## 5. Token 类别

Identifier、字面量等开放集合通过标准 special sequence 引用 Tokenizer TokenKind：

```ebnf
identifier = ? Identifier Token ? ;
integer_literal = ? IntegerLiteral Token ? ;
string_literal = ? StringLiteral Token ? ;
```

special sequence 中的文字是对 Tokenizer 议题的引用，不是 Ink 源码，也不创建新的 TokenKind。

EOF 可以定义为：

```ebnf
end_of_file = ? EndOfFile Token ? ;
```

Trivia 不出现在每条产生式中；它由议题 02 的统一游标规则隐式跳过并完整写入 CST。

## 6. 示例

函数声明的一部分可以使用标准 EBNF 写为：

```ebnf
function_declaration =
    "func", identifier, "(", [ parameter_list ], ")",
    [ return_type ], function_body ;

parameter_list = parameter, { ",", parameter } ;

return_type = "->", type ;

identifier = ? Identifier Token ? ;
```

该示例只展示 EBNF 记法，不提前确认完整函数声明、参数、类型或函数体语法；这些规则在后续议题中逐项确定。

## 7. EBNF 与附加约束

能够由上下文无关语法准确表达的结构必须写入 EBNF。名称可见性、类型相容性、重复声明、`comptime` 求值成功等语义条件不写入 EBNF，而在对应语义规则中单独规定。

当某项合法性确实依赖无法由标准 EBNF 表达的上下文条件时，产生式之后必须以明确文字列出约束，不能发明未定义的 EBNF 运算符。

表达式优先级和结合性也必须最终对应到无歧义的标准 EBNF 分层产生式；实现可以使用 Pratt Parser 或其他等价算法，但实现算法不改变规范文法。

## 8. 确认结论

Ink Parser 的正式语法统一使用 ISO/IEC 14977 风格的标准 EBNF。所有非终结符使用不含空格的 `snake_case` 名称。可选、重复、分组、连接和分支使用标准记号；终结字符串表示准确源码拼写，多字符终结字符串要求底层 Symbol Token 直接相邻；开放 Token 类别使用标准 special sequence 引用 Tokenizer 定义。
