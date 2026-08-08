# Parser 议题 02：Token 游标、Trivia 与连续符号

> 状态：已确认，议题 16 补充 `::<...>` 泛型后缀与泛型结束符中的 `>` 处理
> 确认日期：2026-08-03

## 1. 单一原始游标

Parser 在 Tokenizer 产生的单一 full-fidelity Token 列表上工作。游标至少维护：

```text
TokenCursor {
    tokens: Token[]
    raw_index: TokenId
}
```

`raw_index` 始终指向尚未消费的下一个真实 Token，包括 Trivia 和 EOF。Parser 只接收成功的词法结果，不创建一个删除 Trivia 的第二 Token 列表，也不修改原始 Token 顺序。

## 2. 两类观察操作

游标提供两类基本观察：

```text
peek_raw(offset = 0)
peek_significant(offset = 0)
```

- `peek_raw` 按原始顺序查看 Token，包括 Trivia；
- `peek_significant` 跳过 Trivia，查看第 `offset` 个非 Trivia Token；
- 两种操作都不消费 Token；
- EOF 不是 Trivia，`peek_significant` 必须能够观察到它。

`peek_significant` 中的“跳过”只影响语法判断，不表示从 CST 或 Token 流中删除 Trivia。

## 3. 消费与 CST 写入

语法 Parser 消费一个显著 Token 时，在 CST 中按原始顺序写入从 `raw_index` 到该 Token 的所有 Token。前置 Trivia 因此得到保留。

在开始一个新的语法子节点之前，位于该子节点首个显著 Token 之前的 Trivia 保留为当前父节点的直接子元素。子节点在首个显著 Token 处开始；子节点结束后尚未消费的 Trivia 留给父节点处理。

例如：

```ink
func a() {}

/* comment */

func b() {}
```

其顶层结构为：

```text
SourceFile
├─ FunctionDeclaration(a)
├─ Newline
├─ Newline
├─ BlockComment
├─ Newline
├─ Newline
├─ FunctionDeclaration(b)
└─ EndOfFile
```

位于语法结构内部的 Trivia 仍保留在相应父结构中：

```ink
return /* reason */ value;
```

```text
ReturnStatement
├─ Keyword(return)
├─ Whitespace
├─ BlockComment
├─ Whitespace
├─ NameExpression
│  └─ Identifier(value)
└─ Symbol(';')
```

该规则只确定 Trivia 在 CST 中的结构位置，不声明注释在语义上属于前一个还是后一个声明。格式化、重构和保留注释的代码生成可以在 CST 之上定义各自的注释关联策略。

## 4. 根节点覆盖全部 Token

`SourceFile` 根节点从第一个原始 Token 开始并覆盖 EOF。文件开头、文件结尾以及两个顶层语法节点之间的所有 Trivia 都作为根节点或其后代的真实叶节点保留。

完成解析后，游标必须准确到达 EOF；EOF 也必须作为 CST 的真实 Token 叶节点出现一次。任何未被正常语法消费的真实 Token 都必须进入 `Error` 节点，不能遗留在 CST 之外。

## 5. 连续符号匹配

Tokenizer 将每个合法符号字符输出为单独的 `Symbol(character)` Token。Parser 可以提供：

```text
match_symbol('<')
match_symbols("<=")
match_symbols("->")
match_symbols("...")
peek_longest_symbol_sequence()
```

`match_symbols(sequence)` 仅在以下条件全部满足时成功：

1. 下一个显著 Token 是序列的第一个字符；
2. 后续字符对应的 Token 在完整原始 Token 列表中逐个直接相邻；
3. 每个 Token 都是对应字符的 `Symbol`；
4. 序列中间不存在 Trivia、EOF 或其他 Token；
5. Token 的源码跨度也按字节直接相邻。

所以：

```ink
a <= b        // 可以匹配 "<="
a < = b       // 不匹配 "<="
a </*x*/= b   // 不匹配 "<="
```

匹配成功只产生语法分组，不产生新的词法 Token：

```text
Operator
├─ Symbol('<')
└─ Symbol('=')
```

## 6. 全语言复合符号集合与最长匹配

Ink v0 注册以下多字符 Symbol 序列：

```text
...  ::<  <<=  >>=  ++  --
..   ->   =>   <=   >=   ==  !=  <<  >>  &&  ||
+=   -=   *=   /=   %=   &=   |=  ^=
```

其中 `++` 和 `--` 是保留的非法序列，不是运算符。其余序列的具体语法角色由引用它们的产生式定义。增加新的多字符 Symbol 终结字符串时，必须同步加入这一全语言集合；Parser 各优先级层不得维护彼此不同的局部集合。

在普通语法上下文中，Parser 必须从当前位置按上述全语言集合识别最长的直接相邻序列；不存在匹配项时，当前单个 Symbol 自身构成结果。当前产生式只能消费这个完整结果，不能因为较长结果在本层无效而退回其较短前缀：

```ink
a&&b       // "&&"，不能拆成 "&" 和一元 "&"
a & &b     // Trivia 分开两个 "&"，解析为 a & (&b)
value*=2   // "*="，乘法层不能先消费 "*"
++value    // 保留的非法 "++"，不能拆成两个一元 "+"
+ +value   // Trivia 分开的两个 "+"，可以形成嵌套一元运算
```

最长匹配只建立 Parser 的符号序列观察结果，不在 Token 列表中创建合并 Token，也不改变 full-fidelity CST 的叶节点。每个字符仍以独立 `Symbol` Token 保存。

## 7. 两种受限定界覆盖

普通上下文不得拆分最长复合符号。Ink v0 只允许以下两项具有明确提交条件的定界规则暂时逐字符观察较长序列；除此之外没有局部优先级覆盖。

### 7.1 泛型结束符

已经进入泛型形参或实参列表时，列表顶层的单个 `>` 是显式右定界符。Parser 必须逐个消费 `>`，即使它与后续 `>` 或 `=` 直接相邻：

```ink
Array::<Array::<i32>>
```

这里最后两个 `>` 始终是两个真实 Token，并分别关闭内外两层泛型实参列表。普通表达式上下文仍按全语言集合把同样的相邻字符识别为 `>>`；括号内部需要 `>`、`>=` 或 `>>` 运算时，也恢复普通最长匹配。

这一覆盖只在当前泛型列表顶层期待右定界符时生效，不能被其他语法层用于拆开 `>=`、`>>`、`>>=` 等序列。Tokenizer 不需要重新分词，Parser 也不创建临时拆分 Token。

### 7.2 类型构造后缀

已经提交到显式 `type` 产生式时，`type_postfix_suffix` 按单个 Symbol 消费 `*` 和 `&`，因此可以把直接相邻的 `&&` 解释为两个引用类型后缀。这里的语法上下文已经确定为类型，不需要 EndSet 试探：

```ink
var value: Data&&; // 形成两个 ReferenceTypeSuffix；组合是否合法由语义分析检查
```

普通表达式中的同一覆盖必须更加保守。议题 30 的 `terminal_type_constructor_tail` 只可以在 checkpoint 内逐字符试探 `*`、`&` 及后续类型构造后缀，包括把 `&&` 暂时观察成两个 `&`。只有完整最大尾链之后的下一个显著 Token 属于当前调用者的 EndSet 时才提交；否则必须同时回滚游标、临时 CST 和试探诊断，再按全语言最长匹配解析普通运算符：

```ink
T&&;       // 尾链到达 ";"，提交为两层引用类型值
T&&value;  // 尾链不能到达 EndSet，回滚后解析为 T && value
```

显式类型后缀和表达式尾链都绝不能拆开 `*=`、`&=` 等赋值复合终端；这些序列必须先作为完整 assignment operator 留给语句入口。尾链成功后也只是在 CST 中分别保留原始 Symbol Token，不产生合并或重分词 Token。

## 8. 词法成功前置条件

如果 Tokenizer 产生 Error Token，则当前文件的词法结果失败，编译驱动直接停止该文件的后续编译，不进入 Parser。失败 Token 流仍由 Tokenizer 保存，用于无损源码还原和错误展示。

因此 Parser 的 `Error` CST 节点只保存词法有效但不符合当前位置语法的真实 Token，不保存词法 Error Token。

## 9. 确认结论

Ink Parser 只接收成功的词法结果，并使用一个指向完整 Token 列表的原始游标。语法观察可以跳过 Trivia，但所有 Trivia 仍按原始顺序写入 CST。多个单字符 Symbol 绝不跨越 Trivia 或其他 Token 组合；普通上下文必须按全语言集合最长匹配，并且不能回退为较短前缀。`++`、`--` 是保留的非法序列。泛型列表顶层的 `>` 定界以及类型构造后缀，是仅有的两类受限逐字符覆盖；前者只关闭当前泛型列表，后者在显式类型上下文中直接消费，在普通表达式中则必须以到达调用者 EndSet 为提交条件。尾链试探失败必须完整回滚，赋值复合终端永远不可拆分。所有符号字符在 CST 中仍保留为各自的真实 Token。
