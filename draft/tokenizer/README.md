# Ink 词法规则设计

本目录用于逐项讨论并记录 Ink 从 UTF 编码源码到 token stream 的词法规则。只有已经确认的规则才创建独立议题文件；讨论中的方案不提前写入规范。

## 议题状态

| 编号 | 议题 | 状态 | 文件 |
| --- | --- | --- | --- |
| 01 | 源码编码与字符流 | 已确认，严格 UTF-8、确定换行与原始字节跨度 | [`01-source-encoding-and-character-stream.md`](./01-source-encoding-and-character-stream.md) |
| 02 | 空白、注释与 Full-Fidelity Token 流 | 已确认，Trivia 也是 Token，单一列表可逐字节恢复源码 | [`02-comments-whitespace-and-full-fidelity-tokens.md`](./02-comments-whitespace-and-full-fidelity-tokens.md) |
| 03 | Unicode 标识符与规范化 | 已确认，使用 XID、强制 NFC、排除不可见格式字符 | [`03-identifiers-unicode-and-normalization.md`](./03-identifiers-unicode-and-normalization.md) |
| 04 | 硬关键字、上下文 `from` 与内建类型 Token | 已确认，`from` 始终是 Identifier Token，仅在两处 parser 上下文按拼写识别 | [`04-keywords-and-builtin-type-tokens.md`](./04-keywords-and-builtin-type-tokens.md) |
| 05 | 数字字面量 | 已确认，四种整数进制、十进制浮点、严格分组下划线与类型后缀 | [`05-numeric-literals.md`](./05-numeric-literals.md) |
| 06 | Unicode 标量字面量 | 已确认，单引号准确表示一个 Unicode scalar，支持固定转义 | [`06-unicode-scalar-literals.md`](./06-unicode-scalar-literals.md) |
| 07 | 普通字符串字面量 | 已确认，双引号单行字符串、统一 Unicode 转义、无插值 | [`07-string-literals.md`](./07-string-literals.md) |
| 08 | Raw 与多行字符串字面量 | 已确认，一对/三对双引号决定行模式，`r` 前缀关闭转义 | [`08-raw-and-multiline-string-literals.md`](./08-raw-and-multiline-string-literals.md) |
| 09 | 单字符 Symbol Token | 已确认，所有合法符号逐字符产生统一 Token，复合含义由 parser 识别 | [`09-single-character-symbol-tokens.md`](./09-single-character-symbol-tokens.md) |
| 10 | Token 流与错误恢复契约 | 已确认，完整字节分区、固定 EOF、错误 Token、词法失败门控和确定性输出 | [`10-token-stream-and-error-contract.md`](./10-token-stream-and-error-contract.md) |

词法议题在本目录内独立编号，从 `01` 开始。完成词法层后再进入语法规则目录；词法分析不提前承担名称绑定、类型检查或语义消歧职责。
