# Tokenizer 议题 08：Raw 与多行字符串字面量

> 状态：已确认  
> 确认日期：2026-08-02

## 1. 四种字符串形式

Ink 使用定界符数量表示单行或多行，使用紧邻前缀 `r` 表示是否关闭转义：

```text
"..."       单行，处理转义
r"..."      单行，不处理转义

"""..."""   多行，处理转义
r"""..."""  多行，不处理转义
```

源码示例：

```ink
const escaped = "line 1\nline 2";
const raw = r"C:\Users\Hello";

const escaped_block = """
    line 1\nline 2
    """;

const raw_block = r"""
    C:\Users\Hello
    \d+\.\d+
    """;
```

单引号仍只用于议题 06 的 Unicode `ScalarLiteral`，不表示字符串。

## 2. `r` 前缀

Raw 前缀准确为小写 ASCII `r`，并且必须与开始双引号直接相邻：

```ink
r"text"
r"""
    text
    """
```

前缀属于完整字符串 Token 的 `raw` 和跨度，不另外产生 Identifier Token。

存在 Trivia 时不形成 Raw 前缀：

```ink
r "text"
```

该源码产生 `Identifier("r")`、Trivia 和普通 `StringLiteral`。这不是词法错误；是否缺少运算符，以及是否应建议移除 Trivia 以形成 Raw 前缀，由 parser 根据语法上下文诊断。

最大 Identifier 扫描仍优先保证：

```ink
raw"text"
```

产生 `Identifier("raw")` 和普通字符串，而不是把末尾 `r` 重新解释为前缀。

## 3. 单行转义字符串

普通一对双引号继续使用议题 07 的规则：

```ink
"text"
"line 1\nline 2"
"quote: \""
"emoji: \u{1F600}"
```

它处理固定简单转义、`\xNN` 和 `\u{...}`，且不能包含物理 `LF` 或 `CRLF`。

## 4. 单行 Raw 字符串

`r"..."` 中反斜杠没有转义意义：

```ink
r"C:\Users\Hello\file.txt"
r"\d+\.\d+"
r"\n"
```

最后一个示例的值是反斜杠和字母 `n`，不是换行。

单行 Raw String 由下一个 `"` 结束，因此正文不能直接包含双引号，也不能用 `\"` 逃逸它：反斜杠仍是普通正文，后面的双引号会结束 Token。需要引号时使用普通转义字符串或多行三引号字符串。

单行 Raw String 不能跨物理行。

## 5. 三引号始终表示多行

`"""` 和 `r"""` 只表示多行字符串。开始定界符后必须立即出现 `LF` 或 `CRLF`：

```ink
const text = """
    first line
    second line
    """;
```

以下形式不作为合法行内三引号字符串：

```ink
"""inline"""
r"""inline"""
```

需要单行内容时使用 `"..."` 或 `r"..."`。

开始定界符后的第一个逻辑换行只建立多行模式，不进入字符串解码值。

## 6. 结束定界符

多行结束 `"""` 必须是其逻辑行上的第一个非 Space/Tab 内容：

```ink
const text = """
    content
    """;
```

结束定界符后的分号或其他语法 Token 可以位于同一行；它们不属于字符串。

出现在正文行中部的三个引号不是结束定界符：

```ink
const text = r"""
    this line contains """ in its middle
    """;
```

在 Raw 多行字符串中，无法直接表示一个首个非空白内容就是 `"""` 的正文行，因为它会结束字面量。可以改用普通多行字符串的转义，或用其他普通字符串组合明确构造该值。

## 7. 多行缩进裁剪

结束定界符之前、从该行开始到 `"""` 之间的 Space/Tab 序列定义缩进前缀：

```ink
func example() {
    const text = """
        first
          second
        third
        """;
}
```

缩进前缀是八个 ASCII Space。每个非空正文行必须以完全相同的原始前缀开始，Tokenizer 从解码值中移除该前缀，得到：

```text
first
  second
third
```

Space 和 Tab 按原始字符精确匹配，不按编辑器显示列宽互换。某个非空行缺少准确前缀时是词法错误，Tokenizer 不能猜测最小公共缩进。

只含 Space/Tab 的空白行如果具有完整前缀，则移除前缀后保留多出的空白；如果其全部空白只是结束前缀的一个准确前缀，则可以全部移除并视为空行。其他不匹配组合产生缩进诊断。

结束定界符位于行首时，缩进前缀为空，不裁剪正文。

## 8. 边界换行

多行字符串解码值不包含：

- 开始定界符后立即出现的第一个逻辑换行；
- 最后一个正文行与结束定界符行之间的逻辑换行。

因此：

```ink
const empty = """
    """;
```

产生空字符串。

示例：

```ink
const text = """
    first
    second
    """;
```

解码值为：

```text
first\nsecond
```

如果需要末尾换行，在结束定界符之前增加一个空正文行。

## 9. 多行换行规范化

多行字符串中的物理 `LF` 和 `CRLF` 在解码值中都成为一个 `U+000A`：

```text
source LF   → decoded LF
source CRLF → decoded LF
```

Token 的 `raw` 仍保留原始一个或两个换行字节，因此编辑器和格式化器可以逐字节恢复源文件。该规范化避免版本控制系统改变源码行尾时意外改变字符串语义。

## 10. 转义多行字符串

无 `r` 前缀的 `"""` 多行字符串处理与普通单行字符串相同的转义：

```ink
const text = """
    first\nsecond
    tab:\tvalue
    quote:\"
    emoji:\u{1F600}
    """;
```

反斜杠后直接跟物理换行不是续行，属于非法转义。物理换行本身由多行结构表达。

需要在一行开头产生三个双引号时，可以逐个转义：

```ink
const text = """
    \"\"\" begins this value line
    """;
```

缩进裁剪依据转义前的原始源码行，转义求值在裁剪后处理正文。

## 11. Raw 多行字符串

带 `r` 前缀的多行字符串不识别任何转义：

```ink
const regex = r"""
    ^\d+\.\d+$
    """;
```

反斜杠、`\n`、`\u{...}`、`${...}` 都是普通正文。它仍应用多行定界、缩进裁剪、边界换行排除和物理换行 LF 规范化；这里的 Raw 表示“不解释反斜杠”，不表示解码值逐字节包含源码缩进和行尾编码。

## 12. 不支持插值

四种字符串形式都不执行字符串插值：

```ink
"${value}"
r"${value}"
"""
    ${value}
    """
r"""
    ${value}
    """
```

`${value}` 均为普通正文。未来若增加插值，必须使用不会改变这些既有字面量含义的新形式。

## 13. 不可见字符和原始控制字符

议题 01 禁止的原始控制字符不能直接出现在任何字符串形式中。

bidirectional control、zero-width character、variation selector 和其他必须显式表示的不可见格式字符：

- 在转义单行或多行字符串中使用 `\u{...}`；
- 在 Raw 字符串中不能直接书写，也没有转义机制，应改用普通转义字符串构造相应值。

Raw 模式不能绕过源码安全检查。

## 14. TokenKind 与 Full-Fidelity

实现可以使用一个带标志的字符串 TokenKind：

```text
StringLiteral {
    multiline: bool
    raw_mode: bool
}
```

也可以使用四个内部枚举项。无论表示如何，Token 都保存：

```text
span
raw exact source slice
decoded Unicode scalar sequence
```

Raw 前缀、三引号、原始缩进和原始 `LF`/`CRLF` 全部属于 `raw`。缩进裁剪、转义求值和换行规范化只影响 decoded value，不改写 Token 源码切片。

## 15. 扫描优先级

在可能形成字符串的当前位置，Tokenizer 按更长定界形式优先识别：

```text
r"""  before r"
"""   before "
```

只有完整相邻前缀才进入对应状态。字符串内部的注释定界符不产生 Trivia Token。

## 16. 诊断与错误恢复

诊断至少应区分：

- 三引号后没有立即换行；
- 单行字符串遇到物理换行；
- 多行结束定界符缺失；
- 正文行不满足结束定界符定义的缩进前缀；
- 转义模式中的非法转义；
- Raw 模式中直接出现禁止的不可见字符；
- EOF 前字面量未结束。

错误 Token 的 `raw` 覆盖已经消费的完整源码字节，并继续满足 full-fidelity 分区。多行恢复应优先在后续可能的行首 `"""` 处重新同步，但正常构建不能把错误字面量当作合法字符串。
