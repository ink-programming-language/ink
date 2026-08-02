# Tokenizer 议题 06：Unicode 标量字面量

> 状态：已确认  
> 确认日期：2026-08-02

## 1. ScalarLiteral

单引号定界的字面量产生 `ScalarLiteral` Token：

```ink
'A'
'中'
'😀'
'\n'
'\u{1F600}'
```

Ink 核心语言没有内建 `char` 类型，因此 TokenKind 不命名为 `CharLiteral`。Tokenizer 只验证并记录一个 Unicode Scalar Value；默认类型、上下文目标类型和标准库 `UnicodeScalar` 构造由后续语义阶段处理。

## 2. 必须准确包含一个 Unicode scalar

去除定界符并完成转义后，ScalarLiteral 必须准确表示一个 Unicode Scalar Value。

合法示例：

```ink
'A'
'é'
'😀'
```

非法示例：

```ink
''   // 零个 scalar
'ab' // 两个 scalar
```

用户感知字符或 grapheme cluster 不等于 Unicode scalar。下列可视整体由多个 scalar 组成，因此非法：

```ink
'é'  // U+0065 + U+0301
'🇨🇳' // 两个 regional indicator scalar
```

Tokenizer 按实际解码结果计数，不对字面量正文做 NFC 或 grapheme cluster 合并。

## 3. 固定转义集合

ScalarLiteral 支持以下简单转义：

```text
\\  U+005C REVERSE SOLIDUS
\'  U+0027 APOSTROPHE
\"  U+0022 QUOTATION MARK
\0  U+0000 NULL
\n  U+000A LINE FEED
\r  U+000D CARRIAGE RETURN
\t  U+0009 CHARACTER TABULATION
```

例如：

```ink
'\\'
'\''
'\"'
'\0'
'\n'
'\r'
'\t'
```

未列出的简单转义是错误，Tokenizer 不能把反斜杠静默删除：

```ink
'\q' // 错误：未知转义
```

## 4. 两位十六进制转义

`\xNN` 必须具有准确两个十六进制数字，并产生 `U+0000—U+00FF` 范围的 scalar：

```ink
'\x00'
'\x1B'
'\x7F'
'\xFF'
```

十六进制数字允许 `0-9`、`a-f` 和 `A-F`。以下形式非法：

```ink
'\x1'   // 少于两个数字
'\x123' // 完成两个数字后仍有额外 scalar
'\x_G'  // 非法数字
```

`\xFF` 表示 Unicode scalar `U+00FF`，不是把未经 UTF-8 解码的原始字节 `FF` 插入源码或字符串存储。

## 5. Unicode 转义

通用 Unicode 转义使用：

```text
\u{H...}
```

花括号内必须有 `1—6` 个十六进制数字：

```ink
'\u{0}'
'\u{41}'
'\u{4E2D}'
'\u{1F600}'
```

不支持无花括号的 `\uFFFF`，也不允许花括号内出现下划线、空白、正负号或前缀 `0x`：

```ink
'\u0041'     // 错误
'\u{1_F600}' // 错误
'\u{ 41 }'   // 错误
'\u{0x41}'   // 错误
```

## 6. Scalar Value 范围

所有直接字符和转义结果都必须满足：

```text
0 <= codepoint <= 0x10FFFF
codepoint not in 0xD800—0xDFFF
```

因此：

```ink
'\u{D800}'   // 错误：surrogate
'\u{DFFF}'   // 错误：surrogate
'\u{110000}' // 错误：超出 Unicode 范围
```

UTF-8 源码本身已经由议题 01 排除编码后的 surrogate；本节对数值转义执行相同 Scalar Value 验证。

## 7. 不允许物理换行

ScalarLiteral 不能包含 `LF` 或 `CRLF`，也不提供反斜杠续行：

```ink
'a
'
```

遇到逻辑换行前仍未找到合法结束单引号时，Tokenizer 产生未闭合 ScalarLiteral 错误 Token。换行本身留给后续 `LineBreak` Trivia Token，以便错误恢复继续扫描下一行。

需要换行 scalar 时使用 `\n` 或 `\r`。

## 8. 不可见格式字符必须转义

bidirectional control、zero-width character、variation selector 和其他 `Default_Ignorable_Code_Point` 不能直接出现在 ScalarLiteral 正文中。需要对应 scalar 值时必须显式使用 Unicode 转义：

```ink
'\u{200B}'
'\u{202E}'
```

该限制防止肉眼不可见的方向或零宽字符隐藏在源码定界符之间，同时不剥夺程序表达相应 Unicode 数值的能力。

议题 01 禁止的原始 C0 control 和 `DEL` 同样不能直接出现；它们可以通过本节合法转义表示。

## 9. 定界符与相邻 Token

未转义单引号结束 ScalarLiteral。单引号自身必须写为 `\'`，反斜杠自身必须写为 `\\`。

单引号不形成 lifetime、转义标识符或其他运算符 Token。相邻两个 ScalarLiteral 是两个独立 Token：

```ink
'a''b'
```

它们之间是否缺少运算符由 parser 诊断，Tokenizer 不自动拼接。

ScalarLiteral 不接受类型后缀。紧邻其后的 Identifier 形成另一个 Token，并通常由 parser 报告缺少分隔或运算符。

## 10. Token 表示

合法 Token 的概念信息为：

```text
ScalarLiteral {
    span: [start_byte, end_byte)
    raw: exact source byte slice
    codepoint: u32
}
```

例如：

```ink
'😀'
'\u{1F600}'
```

具有相同 `codepoint`，但 `raw` 和字节跨度不同。Full-fidelity Token 流能够逐字节恢复各自拼写。

实现可以延迟转义求值，但在把 Token 交给正常 parser 前必须完成合法性检查并能提供准确 codepoint。

## 11. 错误恢复与诊断

ScalarLiteral 诊断至少应区分：

- 空字面量；
- 转义后包含多个 scalar；
- 未闭合单引号；
- 未知简单转义；
- `\xNN` 数字数量错误；
- `\u{...}` 缺少花括号、数字数量错误或含非法字符；
- surrogate 或超出 `U+10FFFF`；
- 直接出现必须转义的不可见格式字符；
- 物理换行或 EOF 提前结束。

错误 Token 的 `raw` 必须覆盖已经消费的原始字节并参与议题 02 的无损源码分区。错误恢复不能吞掉下一行中明显独立的 Token。
