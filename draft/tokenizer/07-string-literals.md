# Tokenizer 议题 07：普通字符串字面量

> 状态：已确认，Tokenizer 议题 08 补充 Raw 与多行形式  
> 确认日期：2026-08-02

## 1. StringLiteral

双引号定界的源码产生一个 `StringLiteral` Token：

```ink
"hello"
"你好"
""
```

普通字符串可以包含零个或多个 Unicode Scalar Value。字符串的运行时类型、存储布局、所有权和标准库接口不由 Tokenizer 决定。

## 2. 转义集合

StringLiteral 使用与议题 06 ScalarLiteral 相同的转义集合：

```text
\\
\'
\"
\0
\n
\r
\t
\xNN
\u{H...}
```

例如：

```ink
"line 1\nline 2"
"quote: \""
"backslash: \\"
"emoji: \u{1F600}"
```

未列出的简单转义是词法错误；Tokenizer 不能静默删除反斜杠。

## 3. 十六进制与 Unicode 转义

`\xNN` 必须具有准确两个十六进制数字，并产生 `U+0000—U+00FF` 的 Unicode scalar。它不向字符串插入未经 UTF-8 解码的原始字节：

```ink
"\xFF"
```

其语义内容是 `U+00FF`；如果字符串以 UTF-8 表示，对应编码为 `C3 BF`，不是单字节 `FF`。

`\u{H...}` 使用 `1—6` 个十六进制数字，禁止下划线、空白、符号和 `0x` 前缀，并必须产生合法 Unicode Scalar Value：

```ink
"\u{0}"
"\u{4E2D}"
"\u{1F600}"
```

surrogate `U+D800—U+DFFF` 和超出 `U+10FFFF` 的结果非法。

## 4. 普通字符串不能跨物理行

普通 StringLiteral 必须在同一逻辑源码行内闭合：

```ink
"first
second"
```

这是未闭合字符串错误。反斜杠加 `LF` 或 `CRLF` 不构成续行语法；Tokenizer 在换行前结束错误 Token，并把换行留给独立 `LineBreak` Trivia Token，以便继续扫描下一行。

需要换行内容时使用 `\n`，或使用 Tokenizer 议题 08 的三引号多行字符串。

## 5. 嵌入 NUL

字符串可以通过转义包含 `U+0000`：

```ink
"before\0after"
```

议题 01 仍禁止源文件直接包含原始 NUL。`\0` 只在字面量求值后产生字符串内容。

运行时字符串 API 必须按明确长度处理这类内容；Tokenizer 不添加或假设 C 风格结尾 NUL。

## 6. 不进行 Unicode 规范化

Tokenizer 不对直接字符或转义后的字符串内容执行 NFC、NFD、NFKC 或其他规范化：

```ink
"é"
"e\u{301}"
```

两者具有不同 Unicode scalar 序列。字符串比较、用户请求的规范化和 grapheme cluster 处理属于标准库语义。

## 7. 不可见格式字符必须转义

bidirectional control、zero-width character、variation selector 和其他 `Default_Ignorable_Code_Point` 不能直接出现在普通字符串正文中。需要相应值时使用 Unicode 转义：

```ink
"\u{200B}"
"\u{202E}"
```

该限制与 ScalarLiteral 一致。普通可见 Unicode 字符和 ASCII Space 可以直接出现；议题 01 禁止的原始控制字符仍不能藏入字符串。

## 8. 不进行字符串插值

普通 StringLiteral 内没有插值词法状态：

```ink
"value = ${value}"
```

`${value}` 只是普通字符串正文。Tokenizer 不把一个 StringLiteral 拆成字符串片段、表达式和恢复定界符。

未来如果增加插值，必须作为独立字面量形式和独立议题设计，不能改变本节普通双引号字符串的既有含义。

## 9. 相邻字符串不自动拼接

相邻双引号字面量产生两个 Token：

```ink
"first" "second"
```

Tokenizer 不把它们拼成一个字符串。是否缺少运算符、逗号或其他语法由 parser 诊断。

StringLiteral 不接受类型后缀。紧邻结束引号的 Identifier 是另一个 Token。

## 10. Token 表示

合法 Token 的概念信息为：

```text
StringLiteral {
    span: [start_byte, end_byte)
    raw: exact source byte slice
    decoded: Unicode scalar sequence
}
```

实现可以延迟分配解码字符串，只保存原始跨度和经过验证的转义索引；但把 Token 交给正常语义阶段前必须确认所有转义及 Scalar Value 合法。

例如：

```ink
"😀"
"\u{1F600}"
```

解码内容相同，`raw` 不同。议题 02 的 full-fidelity Token 流按原样恢复各自源码。

## 11. 错误恢复与诊断

普通字符串诊断至少应区分：

- 未闭合双引号；
- 物理换行或 EOF 提前结束；
- 未知简单转义；
- `\xNN` 数字数量错误；
- `\u{...}` 结构或数字错误；
- surrogate 或超出 Unicode 范围；
- 直接出现必须转义的不可见格式字符；
- 议题 01 禁止的原始控制字符。

错误 Token 的 `raw` 必须覆盖已经消费的原始字节，并保持源码分区。换行错误恢复不能吞掉下一行明显独立的 Token。
