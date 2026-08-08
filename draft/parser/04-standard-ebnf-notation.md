# Parser 议题 04：标准 EBNF 记法

> 状态：已确认，非终结符统一使用 `snake_case`；议题 02 规定复合 Symbol 终结字符串的全语言最长匹配；议题 27 同步 `from` 硬关键字的终结字符串记法；2026-08-08 补充 statement context 的保留结构起点谓词以及终止型类型构造尾链的互补守卫
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

binding_declaration = ( "var" | "const" ), identifier ;
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
- `? ... ?` 是标准 EBNF special sequence，用于引用 Tokenizer 已经定义的开放 Token 类别，或在准确位置引用对应议题已经规范定义的零宽度语法谓词。

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
"::<"
```

Tokenizer 已经完成 Keyword、BuiltinType、Identifier 和 Symbol 的分类，因此：

- `"func"` 匹配拼写为 `func` 的 Keyword Token；
- `"i32"` 匹配拼写为 `i32` 的 BuiltinType Token；
- `"("` 匹配一个 `Symbol('(')` Token；
- `"->"` 匹配直接相邻的 `Symbol('-')` 与 `Symbol('>')` 两个 Token；
- `"::<"` 匹配依次直接相邻的两个 `Symbol(':')` 与一个 `Symbol('<')` Token。

terminal string 不会覆盖 Tokenizer 已经确定的 TokenKind。需要按某个 Identifier 的准确拼写建立上下文语法角色时，必须使用下一节的 special sequence 明确写出，不能把该 Identifier 伪装成硬关键字。

一个 terminal string 内的字符必须在源码中直接相邻，不能跨 Trivia。不同 EBNF syntactic term 之间则通过议题 02 的显著 Token 视图匹配，可以存在 Trivia。

Symbol terminal string 还必须遵守议题 02 的全语言最长匹配。普通上下文不能用较短 terminal string 消费一个已注册较长序列的前缀，例如 `"&"` 不能消费 `&&` 的第一个字符，`"*"` 不能消费 `*=` 的第一个字符。`++`、`--` 虽然不是合法表达式运算符，也注册为保留的非法序列，因此不能分别匹配成两个一元 `"+"` 或 `"-"`。

全语言最长匹配只有两类受限的规范覆盖：已经进入泛型列表并在顶层期待右定界符时，单个 `">"` 可以逐字符关闭列表；类型构造后缀可以逐字符消费 `*` 和 `&`。后一类在已经提交的显式 `type` 上下文中直接生效；在普通表达式中，议题 30 的 `terminal_type_constructor_tail` 必须在 checkpoint 内试探，并且只有完整最大尾链到达调用者 EndSet 时才提交。表达式试探失败时必须完整回滚，随后恢复普通最长匹配；`*=`、`&=` 等赋值复合终端在任何情况下都不能被类型后缀拆开。两类覆盖都不改变任何真实 Token。

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

## 5. Token 类别与零宽度谓词

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

少数依赖调用者上下文的确定性语法规则也使用 special sequence 放在实际检查位置。例如：

```ebnf
statement_expression =
    ? next significant Token is neither Keyword(Var), Keyword(Const), nor Keyword(Match), and the next two significant Tokens are not Keyword(Comptime) followed by Keyword(Match) ?,
    expression ;
```

这种 special sequence 是不消费 Token 的谓词，其准确含义必须由对应 Parser 议题完整定义。上例只做忽略 Trivia 的有限 Token 前瞻：它既排除 `var`、`const` 声明起点，也排除语句入口保留给结构控制的裸 `match` 与 `comptime match`；不查询名称、符号表或类型。汇总文法当前只把 special sequence 用于这类声明或结构起点排除、类型构造 Symbol 后缀的受限逐字符消费、函数返回类型最大消费以及终止型类型构造尾链的互补 EndSet 判定；实现不得把它推广为依赖名称绑定或语义类型的任意判定。

当一个可空位置必须与一个受谓词约束的非空分支互斥时，不能写成无条件的 `[ nonempty_branch ]`。本 draft 使用一对逻辑互补的 special sequence 守卫：正分支只在对应谓词成立时接受并消费 Token，零宽度空分支只在不存在满足同一谓词的正分支时成立。例如议题 30 的 `terminal_type_constructor_tail_decision` 在完整最大尾链能够到达调用者提供的 `EndSet` 时必须选择并消费该尾链；只有不存在这样的尾链时，否定守卫才允许消费零个 Token。两条 EBNF 分支因此在规范上互斥，不依赖备选顺序、Parser checkpoint 的尝试顺序或错误恢复策略。实现可以使用事务性 checkpoint 计算该谓词，但 checkpoint 只是实现规范谓词的手段，不能改变接受的 Token 序列或 CST 归属。

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

当某项合法性确实依赖无法由上下文无关产生式表达的上下文条件时，对应议题必须以明确文字列出规范性约束；汇总文法可以使用标准 special sequence 在准确检查位置引用该约束，不能发明未定义的 EBNF 运算符。议题 02 的全语言 Symbol 最长匹配、泛型顶层 `>` 定界覆盖和终止型类型构造尾链的互补 EndSet 守卫属于所有相关产生式共同遵守的 Token 匹配约束。受守卫的非空分支与其零宽度否定分支必须准确互补；不得保留一个绕过正谓词的无条件空分支。

表达式优先级和结合性也必须最终对应到无歧义的标准 EBNF 分层产生式；实现可以使用 Pratt Parser 或其他等价算法，但实现算法不改变规范文法。

## 8. 确认结论

Ink Parser 的正式语法统一使用 ISO/IEC 14977 风格的标准 EBNF。所有非终结符使用不含空格的 `snake_case` 名称。可选、重复、分组、连接和分支使用标准记号；终结字符串表示具有规范 TokenKind 的准确源码拼写，多字符终结字符串要求底层 Symbol Token 直接相邻，并遵守议题 02 的全语言最长匹配；special sequence 引用 Tokenizer 的开放 Token 类别或对应议题明确定义的零宽度语法谓词。无法写入上下文无关产生式的 Token 匹配约束必须在对应议题中明确列为规范规则。
