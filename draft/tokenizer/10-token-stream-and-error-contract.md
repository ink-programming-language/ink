# Tokenizer 议题 10：Token 流与错误恢复契约

> 状态：已确认  
> 确认日期：2026-08-03

## 1. LexedFile

Tokenizer 对每个独立源文件产生：

```text
LexResult = Success(LexedFile) | Failure(LexedFile)

LexedFile {
    source: SourceText
    tokens: Token[]
}
```

`source` 持有不可变原始字节；`tokens` 是议题 02 确定的 full-fidelity 单一 Token 列表。成功与失败结果都保留完整 Token 分区；错误诊断的公共模型与具体数据结构将在独立的 diagnostics draft 中讨论。

Tokenizer 不读取 `import` 指向的文件，不执行宏展开、条件编译、名称绑定或类型检查。每个物理源文件独立产生一个 `LexedFile`。

## 2. Token 基本结构

每个 Token 至少具有：

```text
Token {
    kind: TokenKind
    span: [start_byte, end_byte)
    payload: optional derived data
}
```

原始文本定义为：

```text
token.raw = source.bytes[token.span]
```

`raw` 可以按需构造为对 `SourceText` 的借用切片，不要求每个 Token 复制一份字符串；`LexedFile` 必须保证 source buffer 在所有 Token 被使用期间有效。

## 3. 源码字节连续分区

除零长度 EOF Token 外，Token 跨度按顺序对完整原始源文件形成无重叠、无空洞的连续分区：

```text
tokens[0].start_byte == 0
tokens[i].end_byte == tokens[i + 1].start_byte
last_non_eof.end_byte == source.byte_length
```

该分区包括：

- 可选 UTF-8 BOM；
- 所有语法 Token；
- 空白和注释 Trivia Token；
- 非法编码、非法字符和未闭合结构的错误 Token。

对非 EOF Token 依次拼接 `raw`，必须逐字节得到原始文件：

```text
concat(token.raw for token in tokens if token.kind != EndOfFile)
== source.bytes
```

空文件没有非零长度 Token，但仍具有 EOF Token。

## 4. EndOfFile Token

Token 列表准确地以一个 `EndOfFile` 结束：

```text
kind = EndOfFile
span = [source.byte_length, source.byte_length)
raw  = empty
```

`EndOfFile` 不是 Trivia，也不是错误 Token。Parser 可以把它作为确定的结束终结符，不需要在每次查看 Token 时另行比较数组长度。

EOF Token 不参与源码字节拼接。Tokenizer 不能在列表中间产生 EOF，也不能产生多个 EOF。

## 5. TokenKind 分类

TokenKind 至少支持以下正交查询：

```text
is_trivia
is_error
```

例如：

```text
SpacesAndTabs   is_trivia = true,  is_error = false
LineComment     is_trivia = true,  is_error = false
Identifier      is_trivia = false, is_error = false
Symbol          is_trivia = false, is_error = false
InvalidNumber   is_trivia = false, is_error = true
EndOfFile       is_trivia = false, is_error = false
```

错误 Token 不能伪装成 Trivia。只要列表中存在错误 Token，词法结果就是 `Failure`，编译驱动不得把它交给 Parser。

## 6. 派生 Payload

Token payload 可以缓存已经由词法分析确定的信息，例如：

```text
Identifier
    decoded spelling or intern id

Keyword / BuiltinType
    exact enum value

Symbol
    one ASCII character

IntegerLiteral / FloatLiteral
    base, suffix and component ranges

ScalarLiteral
    Unicode codepoint

StringLiteral
    mode flags and decoded value or lazy escape index
```

Payload 是从 `raw` 和固定语言表得到的派生数据，不能替代原始字节跨度。实现删除缓存并重新计算时必须得到同一结果。

## 7. 错误 Token

正常构建遇到词法错误必须失败，但 Tokenizer 仍为错误字节产生覆盖其原始跨度的 Token。可以使用具体 TokenKind：

```text
InvalidEncoding
InvalidCharacter
InvalidNumber
InvalidScalarLiteral
InvalidStringLiteral
UnterminatedBlockComment
```

实现也可以使用统一 `ErrorToken(error_kind)`，只要能够区分诊断种类并满足相同跨度契约。

错误 Token：

- 保存准确 `raw`；
- 参与连续源码分区；
- 使当前文件的词法结果成为 `Failure`；
- 不被当作合法 Identifier、Literal 或 Symbol；
- 允许 IDE 和其他工具在错误源码上继续进行无损词法处理。

错误恢复不得通过替换、删除或自动补全原始字节使正常构建成功。

## 8. 前进保证

除产生零长度 EOF 外，Tokenizer 每次扫描步骤必须消费至少一个原始字节，或者完成一个此前已开始且最终具有非空跨度的 Token。

遇到无法识别的输入时，扫描器必须产生非空错误 Token 后前进，不能在同一字节无限重复诊断。

对于非法 UTF-8，错误 Token 覆盖实现能够确定属于同一非法序列的原始字节，但不能吞掉随后能够独立开始合法 Token 的字节。

## 9. 错误报告边界

各词法议题负责确定哪些输入不合法，以及错误 Token 必须覆盖哪些原始字节。错误码、消息、相关位置、修复建议以及错误集合的数据结构将在独立的 diagnostics draft 中讨论，不属于本 Tokenizer 议题。

无论实现如何报告错误，Token 的原始 UTF-8 字节跨度始终是源码位置的权威依据。

## 10. Parser 视图

Parser 只接收 `Success(LexedFile)` 的完整 Token 列表，并可以建立跳过 `is_trivia` Token 的游标或索引，但不得删除底层 Token。

由于议题 09 把每个符号保存成一个 `Symbol(character)`，Parser 识别 `<=`、`...`、`->` 等连续序列时必须检查原始 Token 邻接，不能跨 Trivia 组合。

`Failure(LexedFile)` 保留错误 Token 用于源码还原和工具展示，但不进入 Parser。Parser 的错误恢复只处理词法有效 Token 组成的语法错误。

## 11. 确定性

合法 Token 列表和词法诊断只依赖：

```text
source bytes
+ Ink language version
+ pinned Unicode data version
```

Tokenizer 不能依赖：

- 编译目标或 ABI；
- 宿主系统区域和默认编码；
- 当前工作目录；
- 已导入模块的内容；
- 名称是否已经声明；
- 运行时或编译期求值结果。

相同输入和语言版本必须产生相同 TokenKind、跨度、派生 payload 与诊断种类。

## 12. 资源限制

实现可以对源文件大小、单个 Token 长度、注释嵌套深度和字面量解码工作量设置资源上限，但必须：

- 提供明确诊断；
- 保持扫描器前进；
- 不静默截断 Identifier、数字、字符串或注释后继续正常构建；
- 在工具模式中尽可能用错误 Token 覆盖剩余原始字节。

语言实现应公开合理的最低支持范围或可配置上限，避免同一正常规模程序在不同实现中出现不可解释差异。

## 13. Tokenizer 边界

Tokenizer 的最终职责是：

```text
raw source bytes
→ full-fidelity Token list
+ lexical diagnostics
```

以下工作明确属于后续阶段：

- Trivia 与声明文档的关联；
- 复合 Symbol 序列的语法解释；
- 括号配对和语句终止检查；
- 声明、表达式和类型语法；
- 名称绑定、重载和类型检查；
- 编译期执行与 InkIR lowering。
