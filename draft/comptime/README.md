# MetaIR Comptime 设计记录

本目录记录一套独立实验性 MetaIR 的编译期语法与执行模型。它来自交互式设计讨论，与仓库现有 Ink 语法、`draft/parser` 中的 `comptime` 区域以及 `draft/topics` 中的既有语义方案无关，不修改或替代那些设计。

当前设计以完全无二义性和可按 LL(1) 方式解析为目标。已经确认的规则写入独立议题文件；尚未讨论清楚的细节只列为待定项，不在记录中补作默认决定。

## 议题状态

| 编号 | 议题 | 状态 | 文件 |
| --- | --- | --- | --- |
| 01 | MetaIR 语法与编译期执行模型 | 当前方案已确认，后续可继续修订 | [01-metair-syntax-and-execution.md](01-metair-syntax-and-execution.md) |

本轮把“comptime 库”解释为独立设计文档库，因此当前只建立 `draft/comptime` 并保存设计记录，没有创建 C++ 编译目标。是否进一步建立 `src/lib/comptime` 实现库，留到 MetaIR 数据结构、解释器接口和源码边界确定后决定。
