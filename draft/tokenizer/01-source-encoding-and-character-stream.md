# Tokenizer 议题 01：源码编码与字符流

> 状态：已确认  
> 确认日期：2026-08-02

## 1. 源文件编码

Ink 源文件只接受严格合法的 UTF-8，不支持 UTF-16、UTF-32、本地代码页或根据系统区域设置自动猜测编码。

UTF-8 解码必须拒绝：

- 非法起始字节和非法续字节；
- 截断的多字节序列；
- overlong encoding；
- Unicode surrogate code point；
- 超出 `U+10FFFF` 的值。

非法输入是词法错误。Tokenizer 不能用 `U+FFFD REPLACEMENT CHARACTER` 替换后继续把源文件当作有效程序，因为这会让实际编译 token stream 不再对应原始字节内容。

解码后的源码字符模型是 Unicode Scalar Value 序列。

## 2. UTF-8 BOM

文件最开始可以出现一个 UTF-8 BOM：

```text
EF BB BF
```

Tokenizer 在解码入口识别它。BOM 不形成语法字符，但按照议题 02 产生跨度为 `[0, 3)` 的 `Utf8Bom` Trivia Token；因此 full-fidelity Token 列表能够恢复原始文件，首个实际语法 Token 仍从字节偏移 `3` 开始。

`U+FEFF` 出现在文件首字符以外时不具有空白语义。在普通代码区域出现时属于非法字符；在字符串或注释内容中如何保存由对应词法规则决定，但不能借其隐式分隔两个 token。

UTF-16、UTF-32 BOM 不受本规则支持，并会作为非法 UTF-8 输入或非法源码开头被拒绝。

## 3. 逻辑换行

源码只接受以下两种行结束形式：

```text
LF    U+000A
CRLF  U+000D U+000A
```

`CRLF` 在行号计算和词法控制中表示一个逻辑换行。单独的 `CR` 是词法错误，不作为空白或换行恢复。

下列 Unicode 字符不具有 Ink 源码换行语义：

```text
U+0085  NEXT LINE
U+2028  LINE SEPARATOR
U+2029  PARAGRAPH SEPARATOR
```

它们出现在普通代码区域时不能分隔 token。出现在字符串或注释内容中时也不会终止行注释或自动形成源码行号；相应字面量是否接受这些普通 Unicode scalar 由其议题决定。

源文件末尾不要求逻辑换行，最后一行可以直接在 EOF 结束。

## 4. 行结束与原始字节

Tokenizer 不需要把源文件原地改写为统一换行字节。它在词法控制中把 `LF` 和 `CRLF` 映射成同一种逻辑换行事件，同时保留原始 UTF-8 字节和准确跨度。

因此：

- 诊断和源码映射可以准确引用原文件；
- `CRLF` 的原始跨度仍为两个字节；
- token 前后的 trivia 可以保留原始文本；
- 字符串和原始字符串的值是否规范化换行由字面量规则单独决定。

## 5. 普通代码区域的横向空白

普通代码区域只把以下字符识别为横向空白：

```text
U+0020  SPACE
U+0009  CHARACTER TABULATION
```

多个空格和 Tab 可以任意组合用于分隔 token。它们不进入标识符、数字或运算符 token。

其他 Unicode 空白字符不作为 Ink 代码空白，包括但不限于：

```text
U+00A0  NO-BREAK SPACE
U+2003  EM SPACE
U+3000  IDEOGRAPHIC SPACE
U+FEFF  ZERO WIDTH NO-BREAK SPACE
```

这些字符出现在普通代码区域时直接产生非法字符诊断，不能被静默当作 ASCII 空格。这避免视觉相似的源码形成不同 token 边界。

非 ASCII 空白出现在字符串或注释内容中时不是代码分隔符；是否保留为内容由相应词法规则处理。

## 6. 原始控制字符

除合法的 Tab、LF 和组成 CRLF 的 CR 外，源码禁止直接包含 C0 control character 和 `DEL`：

```text
U+0000—U+001F
U+007F
```

该检查适用于整个源文件，包括注释和字面量源码文本。特别是原始 NUL 不能隐藏在注释、普通字符串或原始字符串中。

字符串值需要这些字符时，必须使用字符串议题定义的显式转义形式，例如未来的 `\0` 或 `\x1B`，而不是把控制字符直接嵌入源文件。

## 7. 不进行全文件 Unicode 规范化

Tokenizer 不对源码应用 NFC、NFD、NFKC 或 NFKD，也不做大小写折叠。它按照解码得到的实际 Unicode scalar 保留字符串、注释和 token 拼写。

例如下列文本包含不同的码点序列：

```ink
"é"
"e\u0301"
```

Tokenizer 不能在词法分析前把它们改写成同一序列。字符串转义求值后的准确值、标识符是否要求某种规范化形式以及标识符混淆防护分别由后续议题确定。

## 8. 权威源码跨度

每个 token 和诊断来源位置以原始 UTF-8 文件中的半开字节区间表示：

```text
[start_byte, end_byte)
```

字节偏移从文件第一个原始字节开始计算，包括可选 BOM 和 `CRLF` 的两个原始字节。该区间是编译器、增量解析器、源码映射和语言服务器交换位置的权威表示。

实现同时维护逻辑行起点，以便把字节范围映射为人类可读的行号。终端显示宽度、grapheme cluster 宽度、UTF-16 code unit 列号和编辑器协议位置属于工具适配层，不改变 token 的规范字节跨度。

## 9. 处理顺序

Tokenizer 输入处理概念顺序为：

```text
raw bytes
→ recognize optional initial UTF-8 BOM and emit its Trivia Token
→ strict UTF-8 decode
→ reject forbidden raw controls
→ recognize LF / CRLF logical lines
→ scan syntax and Trivia Tokens from Unicode scalar values
→ attach original UTF-8 byte spans
```

实现可以流式完成这些步骤，不要求实际建立完整中间字符数组，但产生的 token、诊断和位置必须与该模型一致。

## 10. 诊断

编码与字符流诊断至少应区分：

- 非法或截断 UTF-8；
- 非首位置 BOM；
- 单独的 `CR`；
- 普通代码区域中的非 ASCII 空白；
- 禁止的原始控制字符；
- 不能成为任何 token 的 Unicode scalar。

诊断必须包含原始字节范围；如果能够可靠解码周围内容，可以附带 Unicode code point 名称和建议的 ASCII 替代字符。错误恢复不能通过替换字符产生一个可成功输出目标文件的不同程序。
