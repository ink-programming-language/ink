# Diagnostics 议题 01：公共诊断模型、格式化边界与稳定编号

> 状态：已确认
>
> 确认日期：2026-08-08

## 1. 目标

Ink 使用同一个公共诊断模型承载 Tokenizer、Parser 和后续语义分析产生的诊断。每一种诊断必须同时具有：

- 供实现使用的内部符号身份；
- 供命令行、IDE、测试、日志和机器接口使用的稳定编号；
- producer 提交的主源码范围、类型化参数和结构化相关位置；
- 由中央 Formatter 生成、再由具体 Consumer 渲染的正文、note 和严重级别。

诊断编号用于稳定识别诊断种类，不表示消息文本、严重级别、是否阻止编译或当前由哪个执行阶段产生诊断。

诊断处理流水线固定分为：

```text
Tokenizer / Parser / Semantic producer
    -> Diagnostic { Kind, PrimarySpan, Arguments, Related }
    -> DiagnosticFormatter + Core registry
    -> presentation-neutral formatted diagnostic
    -> terminal / JSON / LSP / other consumer
```

## 2. `DiagnosticKind` 与稳定编号

`DiagnosticKind` 是 Core 所有的内部符号身份，例如 `InvalidUtf8`、`MissingBaseDigits` 和未来的 Parser、语义分析诊断种类。其他模块不得定义另一套同类公共枚举。

每个可发出的 `DiagnosticKind` 必须在本议题的中央登记表中具有一个显式 32 位无符号编号。实现不能把枚举声明顺序、编译器自动分配的枚举值、容器索引或默认消息文本当作稳定编号。概念关系为：

```text
DiagnosticKind
    -> DiagnosticNumber: uint32
    -> DiagnosticCode: stable display string
```

`Diagnostic` 实例保存 `DiagnosticKind`、`PrimarySpan`、类型化 `Arguments` 和结构化 `Related`。稳定编号和显示码由 Kind 唯一派生；最终正文和 note 由 `DiagnosticFormatter` 按中央登记项及参数生成，`Diagnostic` 不保存最终 `Message` 字符串。Core 分别提供 Kind 的符号名称、所属域、32 位编号和显示码查询；符号名称与用户可见正文是不同属性。`Diagnostic` 可以提供只读的编号和显示码便捷查询，但实现不得在同一实例中维护可以与 Kind 不一致的可修改编号副本。

## 3. 32 位编号布局

诊断编号的高 8 位是分配域，低 24 位是域内序号：

```text
31                  24 23                                      0
+---------------------+-----------------------------------------+
|  Domain: 8 bits     |  Local number: 24 bits                  |
+---------------------+-----------------------------------------+

number = (domain << 24) | local_number
```

域内序号 `0` 保留，不分配给可发出的诊断。每个域从 `1` 开始单调分配；达到某个值后，即使对应诊断废弃，也不能重新使用该值。

当前分配域为：

| 高 8 位 | 归属 | 显示前缀 | 用途 |
| --- | --- | --- | --- |
| `0x00` | 保留 | 无域字母 | 无效值和内部哨兵 |
| `0x01` | Tokenizer | `T` | 词法与源码字符流诊断 |
| `0x02` | Parser | `P` | 语法与 Parser 恢复诊断 |
| `0x03` | Semantic analysis | `S` | 名称绑定、类型检查和其他语义诊断 |
| `0x04`—`0xFF` | 未分配 | 未定义 | 留给模块系统、驱动和其他未来诊断域 |

域是稳定的编号分配命名空间，不是对编译器运行时阶段的动态记录。已经登记的诊断即使后来由另一个阶段检测或转发，也保留原编号；只有诊断的语义身份确实不同，才登记新编号。Parser 产生的语法诊断使用 `P`，因此 `S` 始终表示 Semantic analysis，不表示 Syntax。

## 4. 显示码

用户可见显示码由固定前缀 `INK-`、域前缀和十进制域内序号组成：

```text
INK-T0001
INK-P0001
INK-S0001
```

域内序号至少补齐为四位；超过四位后按完整十进制值显示，不截断也不回绕。显示码只是 32 位编号的确定性文本形式，同一编号只能有一个规范显示码。

显示码不使用 `E`、`W` 或其他严重级别前缀。同一种诊断可以按工具链策略显示为 warning、升级为 error 或被显式抑制，而编号和显示码保持不变。

## 5. `Unknown` 哨兵

`DiagnosticKind::Unknown`、编号 `0x00000000` 及域内序号 `0` 只用于默认初始化、无效输入和防御性检查。其保留显示码是无域字母的 `INK-0000`，但它不属于可发出的诊断，也不得进入正常诊断结果或序列化输出。

公共转换函数收到 `Unknown` 时返回对应的保留编号、显示码和内部占位名称或消息；收到其他未登记的 Kind 时必须返回明确的内部占位结果或报告契约错误，不能把它伪装成一个有效的 `INK-T`、`INK-P` 或 `INK-S` 编号。

## 6. Producer 的结构化提交契约

Tokenizer、Parser 和语义分析等 producer 只负责检测问题并提交以下语义数据：

```text
Diagnostic {
    Kind
    PrimarySpan
    Arguments: typed values
    Related: structured related locations and arguments
}
```

`PrimarySpan` 是当前诊断的主要源码半开字节范围。每个参数化 Kind 由 Core Formatter 定义自己的参数约定；`Arguments` 必须按该约定提交整数、Unicode scalar、源码拼写、枚举值或其他 Core 定义的类型化值，不能提交已经拼好的句子、带颜色的片段或 `printf` 风格的无类型实参。

`Related` 保存附属源码位置、关系种类和对应的类型化参数。它用于表达“此前声明于此处”“对应的开始定界符在此处”一类关系，但 producer 不直接生成最终 note 文本。没有参数或相关位置的诊断使用空集合，不通过空消息字符串表达缺省状态。

Producer 不负责：

- 保存或拼接最终 `Message`；
- 选择英文或其他本地化正文；
- 生成 `note:` 前缀和附属说明文本；
- 把字节跨度转换为行列号；
- 决定终端颜色、源码摘录、JSON 字段或 LSP 布局；
- 因显示策略改变 Kind 或稳定编号。

结构化提交使同一个诊断可以被不同 Consumer 一致处理，也避免 producer 测试被最终措辞或终端布局绑定。

## 7. `DiagnosticFormatter` 与严重级别

Core 的 `DiagnosticFormatter` 是诊断正文和 note 的唯一生成入口。它读取中央登记中的默认正文和默认严重级别，并按 Core 所有的格式化规则解释 `Arguments` 与 `Related`，产生与具体输出媒介无关的格式化结果：

- 默认严重级别；
- 由类型化参数生成的主要正文；
- 由结构化 `Related` 生成的 note 正文及其源码跨度。

稳定编号和显示码继续由原始 `Diagnostic` 的 Kind 派生，不复制到格式化结果。无参数诊断使用登记的默认正文；参数化诊断由 Formatter 统一解释参数。参数缺失或类型不匹配属于编译器内部契约错误；Formatter 防御性地保留默认正文并忽略无法使用的细节，不能退化为 producer 自行提供的自由文本。

默认 severity 是中央登记项的一部分，由 Formatter 查询并应用，而不是由 producer 重复填写。未来的项目策略可以在 Formatter 之后升级、降级或抑制诊断，但默认值、策略调整后的有效值和稳定编号彼此正交；改变 warning、error 或其他严重级别不能分配新编号，也不能修改显示码。

默认正文的措辞改进、消息本地化、参数化形式和 note 模板同样不改变稳定编号。反之，如果两个情况需要不同的抑制策略、不同的修复语义或需要由工具可靠地区分，就应登记为两个不同的 `DiagnosticKind` 和编号，而不是只改变消息参数。

## 8. Renderer 与 Consumer

Renderer 或 Consumer 接收 presentation-neutral 的格式化诊断，并决定目标媒介的布局。职责包括：

- 终端输出的路径、行列号、颜色、源码摘录、下划线和 note 排列；
- JSON 的字段布局、字符串转义和结构化相关位置；
- LSP 的 range、severity、code、message 和 related information 映射；
- 其他 IDE、日志或机器接口需要的等价表示。

行列号由 Consumer 使用当前 SourceText 或 source manager 从字节跨度计算，不回写到 `Diagnostic`。Renderer 可以按媒介能力选择布局和样式，但不得改变 Kind、编号、参数语义或 Formatter 已确定的正文含义，也不得重新实现某个 producer 的消息模板。

## 9. 分配与兼容性规则

中央登记表是所有稳定诊断编号的唯一规范来源，并遵守以下规则：

1. 新诊断只能在归属域当前最大序号之后追加；
2. 已登记编号不得重排、压缩、复用或转交给另一种诊断；
3. 删除的诊断保留登记项并标记为 retired，后续实现不能重新发出或占用其编号；
4. 重命名内部 C++ 枚举项不能改变已经发布的编号；
5. 每个可发出的 Kind 必须准确映射一个编号，每个编号也只能映射一个 Kind；
6. 增加 Kind、编号映射和中央登记项必须在同一个变更中完成，并通过双射和显示格式测试。

实现中的 Core 中央登记必须在同一个条目中保存 Kind 符号名称、显式 32 位编号、所属域、显示码、默认 severity 和默认正文，并作为枚举声明与元数据查询的唯一来源。参数化正文、参数约定和 note 模板同样只能实现在 Core 的 `DiagnosticFormatter` 中；具体模块不得另写平行编号、显示码、默认严重级别或消息映射。

各 Tokenizer、Parser 和语义议题可以按名称引用诊断种类，但不得复制编号表。公共编号转换由 Core 提供，具体模块只产生已登记的 `DiagnosticKind`。

## 10. 测试分层

诊断测试按职责分层，避免一个 producer 测试同时锁定消息措辞、终端布局和行列换算：

1. Core registry 测试验证 Kind 与编号的双射、域和显示码、编号唯一性、`Unknown` 禁止发出、默认 severity 和默认正文；
2. Producer 测试验证给定源码或语法输入产生准确的 Kind、`PrimarySpan`、类型化 `Arguments` 和 `Related`，不直接断言最终 `Message` 或终端文本；
3. `DiagnosticFormatter` 测试验证无参数默认正文、各参数化正文、Related note、默认 severity 及异常参数的防御性回退；
4. Renderer/Consumer 测试分别验证 terminal、JSON、LSP 的行列换算、颜色或无颜色布局、转义、字段和相关位置表示；
5. 少量端到端测试验证 producer、Formatter 与 Consumer 能正确连接，但不在每个词法、语法或语义用例中重复全部展示断言。

消息措辞变化只需要更新 Formatter 层的对应测试；终端或协议布局变化只更新相应 Consumer 测试。稳定编号及 producer 结构化语义没有变化时，其测试不应随展示改动而变化。

## 11. Tokenizer 编号登记

当前 28 种 Tokenizer 诊断首次按既有 `DiagnosticKind` 列表顺序分配编号如下；登记完成后，编号不再依赖 C++ 枚举的物理声明顺序。表格顺序和空缺编号都属于稳定兼容性契约。

| 域内序号 | 32 位编号 | 显示码 | `DiagnosticKind` |
| --- | --- | --- | --- |
| 1 | `0x01000001` | `INK-T0001` | `InvalidUtf8` |
| 2 | `0x01000002` | `INK-T0002` | `UnexpectedBom` |
| 3 | `0x01000003` | `INK-T0003` | `LoneCarriageReturn` |
| 4 | `0x01000004` | `INK-T0004` | `NonAsciiWhitespace` |
| 5 | `0x01000005` | `INK-T0005` | `ForbiddenControlCharacter` |
| 6 | `0x01000006` | `INK-T0006` | `InvalidCharacter` |
| 7 | `0x01000007` | `INK-T0007` | `IdentifierNotNfc` |
| 8 | `0x01000008` | `INK-T0008` | `InvisibleCharacter` |
| 9 | `0x01000009` | `INK-T0009` | `MissingBaseDigits` |
| 10 | `0x0100000A` | `INK-T0010` | `DigitOutOfRange` |
| 11 | `0x0100000B` | `INK-T0011` | `MisplacedNumericSeparator` |
| 12 | `0x0100000C` | `INK-T0012` | `MissingExponentDigits` |
| 13 | `0x0100000D` | `INK-T0013` | `UnknownNumericSuffix` |
| 14 | `0x0100000E` | `INK-T0014` | `InvalidNumericSuffix` |
| 15 | `0x0100000F` | `INK-T0015` | `UnsupportedNonDecimalFloat` |
| 16 | `0x01000010` | `INK-T0016` | `EmptyScalarLiteral` |
| 17 | `0x01000011` | `INK-T0017` | `MultipleScalarValues` |
| 18 | `0x01000012` | `INK-T0018` | `UnterminatedScalarLiteral` |
| 19 | `0x01000013` | `INK-T0019` | `UnknownEscape` |
| 20 | `0x01000014` | `INK-T0020` | `InvalidHexEscape` |
| 21 | `0x01000015` | `INK-T0021` | `InvalidUnicodeEscape` |
| 22 | `0x01000016` | `INK-T0022` | `InvalidUnicodeScalar` |
| 23 | `0x01000017` | `INK-T0023` | `UnterminatedStringLiteral` |
| 24 | `0x01000018` | `INK-T0024` | `MultilineOpeningLineBreakRequired` |
| 25 | `0x01000019` | `INK-T0025` | `UnterminatedMultilineStringLiteral` |
| 26 | `0x0100001A` | `INK-T0026` | `InvalidMultilineIndentation` |
| 27 | `0x0100001B` | `INK-T0027` | `UnterminatedBlockComment` |
| 28 | `0x0100001C` | `INK-T0028` | `BlockCommentNestingLimit` |

Parser 和语义分析尚未登记具体诊断。它们未来分别从 `0x02000001`（`INK-P0001`）和 `0x03000001`（`INK-S0001`）开始追加。

## 12. 确认结论

Ink 的每个可发出 `DiagnosticKind` 都具有一个由 Core 管理的显式 32 位稳定编号。高 8 位选择分配域，低 24 位是不可复用的域内序号；规范显示码使用 `INK-T0001`、`INK-P0001` 和 `INK-S0001` 一类形式。Producer 只提交 Kind、`PrimarySpan`、类型化 `Arguments` 和结构化 `Related`；`DiagnosticFormatter` 根据中央登记集中生成正文、note 和默认严重级别，Renderer/Consumer 再决定 terminal、JSON、LSP 等布局。编号命名空间不随实际检测阶段、消息或 severity 改变；中央登记表是唯一编号来源，`Unknown` 只作不可发出的内部哨兵。
