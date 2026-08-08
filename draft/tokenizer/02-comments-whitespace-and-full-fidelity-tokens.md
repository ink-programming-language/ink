# Tokenizer 议题 02：空白、注释与 Full-Fidelity Token 流

> 状态：已确认，根据后续讨论修订为单一 Token 列表  
> 确认日期：2026-08-02

## 1. Trivia 也是 Token

Tokenizer 只输出一个按照原始源码顺序排列的 Token 列表：

```text
TokenizedBuffer {
    tokens: Token[]
}
```

空白、换行、注释和文件开头的 UTF-8 BOM 都是 Token。它们的 `TokenKind` 被分类为 Trivia，因此 parser 可以跳过，但不会从词法结果中丢失。

不存在独立的 `Trivia[]`，也不把 Trivia 规范性地挂靠为前后语法 token 的附属字段。

## 2. Token 保存原始字节

每个 Token 至少具有：

```text
Token {
    kind: TokenKind
    span: [start_byte, end_byte)
    raw: exact source byte slice
}
```

`raw` 可以是对不可变 source buffer 的借用切片，不要求为每个 Token 单独复制内存；但它在语义上表示该 Token 对应的完整原始 UTF-8 字节，而不是规范化后的字符串。

对任意成功扫描的源文件，按顺序拼接所有非零长度 Token 的 `raw`，必须逐字节得到原文件：

```text
concat(token.raw for token in tokens)
== original_source_bytes
```

该保证包括 BOM、空格与 Tab 的准确组合、`LF` 与 `CRLF` 的区别、注释定界符和注释正文。

## 3. Token 跨度构成源码分区

除可选的零长度 EOF Token 外，Token 跨度按照 `start_byte` 严格递增，并对原始文件形成无重叠、无空洞的连续分区：

```text
tokens[0].start_byte == 0
tokens[i].end_byte == tokens[i + 1].start_byte
tokens[last].end_byte == source_byte_length
```

空文件可以没有非零长度 Token。若实现产生 EOF Token，其跨度为：

```text
[source_byte_length, source_byte_length)
```

EOF Token 不参与源码字节拼接，也不破坏分区规则。

## 4. Trivia TokenKind

当前至少定义以下 Trivia TokenKind：

```text
Utf8Bom
SpacesAndTabs
LineBreak
LineComment
BlockComment
```

`TokenKind.is_trivia` 对这些种类返回 `true`。这是一种 token 分类，不是另一条存储通道。

Parser 使用过滤视图消费有效语法 token：

```text
for const token in tokens {
    if token.kind.is_trivia {
        continue;
    }

    parse(token);
}
```

格式化器、文档工具、IDE 和源码重写器使用完整列表。

## 5. BOM Token

议题 01 允许的文件开头 UTF-8 BOM 形成一个 `Utf8Bom` Trivia Token：

```text
kind = Utf8Bom
span = [0, 3)
raw  = EF BB BF
```

Parser 忽略该 Token，因此 BOM 不影响语法；full-fidelity 工具仍能准确还原它。非首位置的 `U+FEFF` 不形成 `Utf8Bom`。

## 6. 空白与换行 Token

相邻的 ASCII Space 和 Tab 可以合并成一个 `SpacesAndTabs`：

```text
" \t  \t" → one SpacesAndTabs Token
```

Token 的 `raw` 保留每个字符的准确次序，合并不影响无损还原。

每个逻辑换行产生一个 `LineBreak`：

```text
LF   → raw length 1
CRLF → raw length 2
```

`LF` 与 `CRLF` 具有相同 Trivia 分类，但原始字节切片不同。换行不参与自动分号插入，也没有普通语法意义。

## 7. 行注释

行注释从 `//` 开始，结束于下一个 `LF`、`CRLF` 或 EOF：

```ink
// comment
const value = 10; // trailing comment
```

`LineComment` Token 包含 `//` 和之后的正文，但不包含终止换行。换行随后形成独立的 `LineBreak` Token：

```text
LineComment("// comment")
LineBreak("\n")
```

行注释在 EOF 结束是合法的。`U+0085`、`U+2028` 和 `U+2029` 按议题 01 不属于逻辑换行，因此不会终止行注释。

## 8. 嵌套块注释

块注释由 `/*` 开始、对应的 `*/` 结束，并支持有限深度的词法嵌套：

```ink
/*
    outer
    /* inner */
*/
```

扫描器维护深度：

```text
read /*  → depth += 1
read */  → depth -= 1
depth 0  → block comment ends
```

完整嵌套块形成一个 `BlockComment` Token。它的 `raw` 包含外层和内层定界符、正文以及其中全部换行，不为内部换行另外产生 `LineBreak` Token；实现仍须扫描这些原始字节以维护诊断行号。

空块注释 `/**/` 合法。

## 9. 注释扫描状态

块注释内部只有 `/*` 和 `*/` 影响嵌套深度。引号、反斜杠、`//`、关键字和其他 token 拼写只是注释文本：

```ink
/* "not a string" // not a line comment */
```

行注释内部的 `/*` 和 `*/` 同样只是正文。

反过来，字符串或字符字面量扫描状态中的 `//`、`/*` 和 `*/` 不形成注释：

```ink
const url = "https://example.com";
const text = "/* not a comment */";
```

Tokenizer 必须根据当前词法状态决定注释定界符是否有效。

## 10. Trivia 形成强制 Token 边界

Trivia 两侧的字符不能在 parser 跳过 Trivia 后重新组合成一个 Token：

```ink
first/* comment */second
```

完整列表为：

```text
Identifier("first")
BlockComment("/* comment */")
Identifier("second")
```

Parser 的过滤视图中仍是两个 `Identifier`，不会得到 `Identifier("firstsecond")`。

同理：

```ink
+/* comment */+
```

产生两个独立 `+` Token。最长匹配不能跨越任何 Trivia Token。

## 11. 换行不参与语法

Ink 使用显式分号和成对定界符，不执行自动分号插入：

```ink
const value
    =
    calculate
    (
        10
    )
;
```

过滤 Trivia 后的语法 Token 与单行写法相同。反斜杠加换行也不构成特殊续行形式。

## 12. 文档注释

当前不赋予 `///`、`//!`、`/**` 或 `/*!` 特殊文档语义。它们分别按照普通 `LineComment` 或可嵌套 `BlockComment` 扫描，并完整保留原始拼写。

未来增加文档系统时可以进一步细分相应 TokenKind，但不能改变已经确定的注释结束位置、嵌套或 Token 边界。

## 13. 错误 Token 与无损恢复

正常批量编译遇到非法编码、非法字符或未闭合块注释时必须失败，不能用替换字符产生另一个可成功编译的程序。

为了让 IDE 模式在错误源码上仍能无损保存，Tokenizer 可以产生带诊断的错误 Token，例如：

```text
InvalidBytes
InvalidCharacter
UnterminatedBlockComment
```

错误 Token 的 `raw` 仍是准确原始字节切片，并参与连续源码分区。它们不属于 Trivia，也不能交给 Parser 当作合法语法 Token。只要 Tokenizer 产生错误 Token，本文件的词法结果就是失败；编译驱动不得对它调用 Parser。整个失败的 `TokenizedBuffer` 仍满足字节拼接还原规则，供错误展示和源码工具使用。

未闭合块注释的错误 Token 从 `/*` 覆盖到 EOF；诊断至少报告最外层起点、剩余嵌套深度，以及能够合理展示时最近未闭合层的位置。

## 14. 实现边界

Tokenizer 可以只保存 `kind` 和对共享不可变 source buffer 的跨度，由 `raw` 属性按需构造借用切片。所谓“Token 列表无损恢复源码”不要求把每段文本复制到独立堆对象，但 Token 列表必须覆盖每个原始字节并保持 source buffer 的可访问生命周期。

编译器的高性能 parser 可以使用跳过 Trivia 的迭代器或预计算语法 Token 索引，但优化视图不能取代规范性的 full-fidelity Token 列表。
