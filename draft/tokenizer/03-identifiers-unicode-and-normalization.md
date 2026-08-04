# Tokenizer 议题 03：Unicode 标识符与规范化

> 状态：已确认  
> 确认日期：2026-08-02

## 1. 标识符字符集合

Ink 标识符使用 Unicode 标准派生属性：

```text
IdentifierStart :=
    "_"
    | Unicode_XID_Start

IdentifierContinue :=
    "_"
    | Unicode_XID_Continue
```

首字符必须满足 `IdentifierStart`，后续零个或多个字符必须满足 `IdentifierContinue`。

合法示例：

```ink
value
_private
用户
用户ID
Δvalue
данные
```

非法或被拆分的示例：

```ink
2value      // 数字不能开始标识符
hello-world // '-' 不是 IdentifierContinue
😀value     // emoji 不是 XID_Start
user$name   // '$' 不是 IdentifierContinue
```

非 ASCII 数字、组合字符和其他 Unicode 字符只有在首字符之后且属于 `XID_Continue` 时才可能出现。

## 2. 必须已经是 NFC

每个完整标识符的 Unicode scalar 序列必须处于 NFC。Tokenizer 先扫描候选标识符，再验证其完整拼写；它不能静默规范化后产生另一个名称。

例如由单个 `U+00E9` 组成的 NFC 拼写可以合法出现在：

```ink
const café = 1;
```

如果源码用 `U+0065 U+0301` 表示相同视觉字符但完整标识符不是 NFC，Tokenizer 产生标识符规范化诊断，并建议规范后的源码拼写。

不自动改写可以保证：

- Token 的 `raw` 与用户文件一致；
- 搜索、重命名和源码映射不会引用编译器私下创建的拼写；
- 符号表不会同时接受规范等价但字节不同的两个名称；
- full-fidelity Token 流仍能逐字节恢复源码。

NFC 要求只作用于标识符，不改变字符串、注释或其他 Token 的原始文本。

## 3. 大小写敏感

标识符按照 NFC 验证后的实际 Unicode scalar 序列精确比较，不进行 ASCII 或 Unicode 大小写折叠：

```ink
value
Value
VALUE
```

以上是三个不同标识符。Tokenizer、名称绑定、反射名称和符号身份必须保持同一大小写规则。

## 4. 禁止不可见格式字符

标识符不能包含用于排版控制而非名称书写的不可见格式字符，包括：

- bidirectional formatting/control character；
- zero-width joiner 和 zero-width non-joiner；
- `Default_Ignorable_Code_Point`；
- variation selector；
- Unicode whitespace。

即使某个 Unicode 数据表或标识符配置允许其中一项作为延续字符，Ink 的标识符 profile 仍将其排除。

Tokenizer 不能静默删除这些字符。诊断应给出准确原始字节跨度、Unicode code point，并说明不可见字符位于相邻哪些可见字符之间。

议题 01 对普通代码区域 Unicode 空白和原始控制字符的限制仍优先适用。

## 5. 允许混合书写系统

Ink 不把不同 Unicode script 出现在同一标识符中定义为词法错误：

```ink
const user数量 = 10;
const HTTP状态 = 200;
```

这类中英文组合在实际代码中有合理用途。Tokenizer 只应用 XID、NFC 和不可见字符规则，不根据 script 猜测作者意图。

编译器或 IDE 可以对高度可疑的 confusable、同作用域视觉近似名称或异常 script 组合发出安全警告，但该警告：

- 不改变 TokenKind 或 `raw`；
- 不自动重命名；
- 不改变标识符相等性；
- 不得作为不同实现产生不同 token stream 的依据。

## 6. 下划线

下划线 `_` 可以开始或继续标识符：

```ink
_
_value
__internal
```

Tokenizer 对单独的 `_` 产生普通 `Identifier` Token，不设置特殊 `Underscore` TokenKind。Parser 可以在模式、参数或其他特定语法位置把拼写精确为 `_` 的 Identifier 解释为忽略占位符；在其他位置如何处理由语法和名称绑定规则决定。

Lexer 不保留双下划线、尾随下划线或其他前缀给实现使用。标准库和 ABI 若需要命名约定，应在对应规则中定义，而不是改变 Identifier TokenKind。

## 7. 关键字分类顺序

Tokenizer 按以下顺序处理可能的名称：

```text
scan maximal IdentifierStart IdentifierContinue*
→ validate excluded invisible characters
→ validate NFC
→ compare complete spelling with hard-keyword table
→ emit Keyword or Identifier TokenKind
```

关键字只使用 ASCII 小写拼写，并按大小写精确比较：

```ink
func // hard keyword
Func // Identifier
函数 // Identifier
```

最长标识符扫描先于关键字分类，因此：

```ink
function
functional
```

不会被拆成关键字 `func` 加后续字符。

硬关键字的准确集合由 Tokenizer 议题 04 定义。

## 8. 不提供转义标识符

Ink 不提供通过额外定界符把关键字改写为普通标识符的机制，例如：

```text
`keyword`
r#keyword
@keyword
```

硬关键字不能直接用作用户声明名称。外部 ABI、序列化字段或反射数据需要表示任意字符串名称时，使用对应系统的显式字符串或 `Identifier` 转换接口，不改变 Ink 源码标识符词法。

反引号、`#` 和 `@` 是否具有其他 TokenKind 由后续标点和属性议题决定；本议题只排除它们作为转义名称前缀。

## 9. Unicode 数据版本

`XID_Start`、`XID_Continue`、NFC 和相关排除属性必须绑定到明确的 Ink 语言版本，而不能查询宿主操作系统当前 Unicode 数据库。

同一 Ink 语言版本的不同编译器实现必须使用相同 Unicode 数据版本和同一 Ink identifier profile。编译器应在版本信息或构建元数据中暴露所使用的 Unicode 数据版本。

升级 Unicode 数据属于明确的语言版本变更。升级过程必须检查新增或重新分类字符是否会让旧源码的 Token 边界、关键字分类或非法字符诊断发生变化；不能在无语言版本变化的编译器补丁中静默升级。

## 10. Token 表示

Identifier Token 至少具有：

```text
kind = Identifier
span = original UTF-8 byte range
raw  = exact original source slice
```

实现可以缓存解码后的 Unicode scalar 序列、哈希或 interned symbol，但规范身份仍来自通过 NFC 验证的准确标识符拼写。缓存不能覆盖 `raw` 或破坏议题 02 的源码无损还原。

Keyword Token 同样保存原始字节切片，只是具有独立的关键字 TokenKind 或可确定的关键字枚举值。

## 11. 诊断

标识符诊断至少应区分：

- 非法首字符；
- 非法延续字符导致的 Token 边界；
- 非 NFC 标识符；
- 禁止的不可见格式字符；
- 关键字被用在要求普通标识符的语法位置；
- 当前语言版本不允许的 Unicode 标识符字符。

非 NFC 诊断可以显示建议的 NFC 拼写，但正常构建不能自动应用修复后继续输出目标文件。IDE 可以把建议作为显式源码编辑提供给用户。
