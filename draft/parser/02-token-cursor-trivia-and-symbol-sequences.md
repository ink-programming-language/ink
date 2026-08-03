# Parser 议题 02：Token 游标、Trivia 与连续符号

> 状态：已确认  
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

## 6. 语法上下文决定组合方式

不存在一个在所有 Parser 上下文中强制执行的全局复合符号合并阶段。

表达式语法可以在当前允许的运算符集合中优先匹配最长的连续符号序列：

```ink
a >>= 2
```

类型语法则可以逐个消费泛型结束符：

```ink
Array<Array<i32>>
```

这里最后两个 `>` 始终是两个真实 Token。表达式 Parser 可以把它们组织为右移运算符，类型 Parser 可以把它们分别用作两层泛型的结束符。Tokenizer 不需要重新分词，Parser 也不需要拆分 Token。

## 7. 词法成功前置条件

如果 Tokenizer 产生 Error Token，则当前文件的词法结果失败，编译驱动直接停止该文件的后续编译，不进入 Parser。失败 Token 流仍由 Tokenizer 保存，用于无损源码还原和错误展示。

因此 Parser 的 `Error` CST 节点只保存词法有效但不符合当前位置语法的真实 Token，不保存词法 Error Token。

## 8. 确认结论

Ink Parser 只接收成功的词法结果，并使用一个指向完整 Token 列表的原始游标。语法观察可以跳过 Trivia，但所有 Trivia 仍按原始顺序写入 CST。多个单字符 Symbol 仅由当前语法上下文识别为连续符号结构，而且绝不跨越 Trivia 或其他 Token 组合。
