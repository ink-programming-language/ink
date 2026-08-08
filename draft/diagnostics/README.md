# Ink 诊断设计

本目录用于定义跨 Tokenizer、Parser 与后续语义分析阶段共享的诊断模型、稳定编号、集中格式化和消费端渲染边界。具体语言议题仍负责确定哪些源码不合法、如何恢复以及应提交哪些结构化诊断数据；本目录统一规定诊断身份、编号、参数化正文、相关位置、默认严重级别和兼容性要求。

## 议题状态

| 编号 | 议题 | 状态 | 文件 |
| --- | --- | --- | --- |
| 01 | 公共诊断模型、格式化边界与稳定编号 | 已确认，producer 提交结构化数据，Formatter 集中生成正文，Consumer 负责布局；使用 32 位显式编号和 `INK-T0001` 一类稳定显示码 | [`01-diagnostic-model-and-codes.md`](./01-diagnostic-model-and-codes.md) |

诊断议题在本目录内独立编号，从 `01` 开始。公共 `Diagnostic`、`DiagnosticKind`、中央登记、编号查询和 `DiagnosticFormatter` 属于 Core；Tokenizer、Parser 和语义分析模块只提交 Kind、主跨度、类型化参数与相关位置，不能重复定义公共诊断容器、私有编号体系或自行拼接最终消息。终端、JSON、LSP 和其他 Consumer 只负责各自输出布局。
