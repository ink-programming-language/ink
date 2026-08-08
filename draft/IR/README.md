# InkIR 设计草案

> 状态：设计基线，尚未形成对外兼容性承诺
>
> 适用范围：Ink v0 的 staged elaboration、编译期执行、Closed IR 解释执行与 LLVM AOT lowering

## 1. 目标

InkIR 是 Ink 编译器内部统一的强类型语义 IR。它需要同时支撑：

- 已完成 elaboration 的普通运行时代码；
- 泛型实例化、结构化 `comptime` 与声明固定点；
- 与目标机器一致的编译期执行；
- Closed InkIR 的直接解释执行；
- 向 LLVM IR 的 AOT lowering；
- RAII、异常、虚分派、接口、反射、装饰器、惰性异步任务和同 ABI 代码热替换；
- 确定性文本 dump、内部二进制缓存、诊断 provenance 与严格 verifier。

InkIR 不承担以下职责：

- 不取代 full-fidelity CST，也不保存注释和全部语法 trivia；
- 不把未选择的 dependent 源码分支伪装成已经完成类型检查的 CFG；
- 不提供用户可调用的 IR Builder、字符串转源码或动态 import；
- 不定义稳定的 Ink-to-Ink 二进制模块 ABI；
- 不冻结 Task、异常记录、反射描述符或 vtable 的跨编译器物理布局；
- 不取代 LLVM 的 C ABI、平台异常 ABI、寄存器分配和指令合法化。

## 2. 已确认的总体选择

本目录中的设计统一采用以下基线：

1. 自研轻量 IR，采用 block-argument SSA、显式 operation/region、命名空间 opcode 和分层 verifier，不引入 MLIR 运行依赖。
2. `StagedModule` 是强类型 Core InkIR、`ElaborationPlan` 和 source-backed `NormalizedTemplateTable` 的组合容器；只有 Core InkIR operation 保证已经完整类型化。
3. `ClosedModule[target]` 不含 deferred template、开放泛型、meta value 或任何 stage plan node。
4. 标量优先使用 SSA；聚合、不可复制类型和 address-only 类型使用调用者提供的最终结果位置。
5. Closed InkIR 是 target-aware 的语义 IR；TargetABI lowering 可以直接生成 LLVM IR，v0 不要求额外的稳定 Codegen IR。
6. 文本格式是规范、确定的 dump/golden 格式；二进制格式只作精确版本匹配的内部缓存；两者都不是公开输入格式或稳定 ABI。
7. `--emit=ink-ir` 表示 Closed InkIR；Staged dump 使用独立的实验选项。
8. 裸指针采用目标地址位宽上的扁平数值地址模型，地址运算取模；形成地址本身不检查，非法解引用是 UB。
9. strict 浮点的 NaN 和 sNaN 位级行为由 `TargetContext.nan_mode` 精确规定；`[fast_math]` v0 只授予重结合、收缩、忽略有符号零、FTZ 和 DAZ，不授予近似倒数、近似函数或有限值假设。
10. padding 默认未初始化且不属于语义值；typed copy 与 `raw.memcpy` 是两套不可混用的语义。
11. 可观察的 comptime 写效果使本次执行不可缓存，并按规范 work-item 顺序提交；纯计算与受跟踪只读效果可以并行和缓存，不做 effect replay。
12. `[nothrow] async` 的同步 Task 构造和异步完成是两个独立效果维度：构造仍可 unwind，Task 本身不得进入 failed。
13. v0 热更新只允许 ABI 和布局完全相同的代码替换；反射 handle 固定取得时的版本快照；同步和异步 virtual/interface override 都要求结果类型完全一致。
14. decorator 驱动的 module registration 由 typed-Core comptime operation 发射为确定性、可缓存的 semantic records；Staged/Closed artifact 都携带 `ModuleRegistrations`，运行时按 module version 原子发布或替换。

## 3. 文档结构

| 文档 | 内容 |
| --- | --- |
| [01-architecture.md](./01-architecture.md) | 层次、模块结构、核心实体、阶段边界与全局不变量 |
| [02-types-values-memory.md](./02-types-values-memory.md) | 类型、常量、SSA、place、存储、对象生命周期、指针和浮点语义 |
| [03-core-instructions.md](./03-core-instructions.md) | Core InkIR opcode、语法、效果、前置条件与 verifier 规则 |
| [04-execution-model.md](./04-execution-model.md) | CFG 解释、顺序、内存、调用、异常、trap、PDB 与世界无关执行核心 |
| [05-staging-comptime.md](./05-staging-comptime.md) | ElaborationPlan、deferred template、partial evaluation、module registration、固定点、能力与缓存 |
| [06-calling-convention-runtime.md](./06-calling-convention-runtime.md) | Ink 逻辑调用约定、运行时抽象、虚分派、反射、异步、装饰器与热更新 |
| [07-verification-passes.md](./07-verification-passes.md) | verifier 分层、阶段能力类型、pass 契约和确定性要求 |
| [08-text-binary-format.md](./08-text-binary-format.md) | canonical text、sectioned binary、registration records、版本字段、cache identity 与安全解码 |
| [09-llvm-lowering.md](./09-llvm-lowering.md) | TargetABI/LLVM 映射、首个纵切片和递增实现顺序 |
| [10-schema-registry.md](./10-schema-registry.md) | v0 数字 tag、record/payload、opcode schema、effect/trait/stage 与 canonical text 的唯一 source of truth |

## 4. 规范用语

本文档使用“必须”“不得”“应当”和“可以”表达约束强度：

- “必须”与“不得”是 IR 合法性或语义要求；
- “应当”是 v0 的实现基线，偏离时需要给出等价性论证；
- “可以”表示不影响可观察语义的实现自由。

带 `normative` 标记的规则属于 InkIR 语义。带 `private TargetABI` 或 `non-normative` 标记的结构只是建议实现，不构成跨编译器、跨构建或跨目标 ABI。

## 5. 版本与兼容政策

以下版本必须分别记录，不能合并成一个模糊的“语言版本”：

- `LanguageRevision`：源码语言语义；
- `IrSemanticsRevision`：operation、type 和 verifier 语义；
- `TextSyntaxVersion`：canonical text 语法；
- `BinaryContainerVersion`：二进制容器编码；
- `CompilerBuildId`：编译器实现构建；
- `TargetAbiRevision`：目标 lowering 和布局规则；
- `RuntimeAbiRevision`：同一次兼容构建中的私有运行时契约；
- `RegistrationEncodingRevision`：module registration frozen value、relocation、application order path、动态路径与 protocol schema 编码语义。

v0 二进制缓存要求关键版本和摘要精确匹配。版本不匹配是 cache miss，不是源码错误。文本 dump 在正式对外发布前也不承诺跨版本可读取。

## 6. 最高级不变量

任何实现和优化 pass 都必须保持以下不变量：

- Core InkIR operation 始终具有完整、规范化的语义类型；
- 未选择的 dependent template 不提前绑定或类型检查；
- Closed InkIR 只包含目标世界可表示、可执行和可 lowering 的内容；
- `void`、`never` 与空元组 `()` 是三种不同类型；
- SSA value、place、未初始化存储和已开始生命周期的对象不能混为一类；
- InkIR 没有 LLVM 风格的 `undef` 或 `poison` 值；PDB 必须由 TargetContext 解析为具体结果或 trap；
- 语言级 copy 不等于字节复制，不可复制类型不能因 lowering 而出现隐藏 copy；
- 可能 unwind 的调用具有显式 unwind 后继；trap 和 fatal 不执行语言清理；
- operation 的效果、可抛出性、可 trap 性和阶段合法性由 opcode 重新推导，不能信任缓存中的摘要；
- module registration identity、层次化 application program-order path、application-wide emission ordinal、结构化动态路径、frozen constant/relocation 与 protocol/interface digest 必须由 registry 规则重算或核对，不能信任缓存中的声明值；
- IR 身份、打印顺序、缓存键和生成声明身份不得依赖地址、哈希表迭代、线程调度或 fixed-point 完成顺序；
- host pointer、compiler arena 地址、裸 `DeclId`、进程句柄和 comptime 临时地址不得进入 Closed InkIR 或跨进程缓存。
