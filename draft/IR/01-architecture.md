# InkIR 架构与阶段边界

## 1. 总体流水线

Ink 编译器采用以下流水线：

```text
source files
    -> tokenizer / full-fidelity CST
    -> declaration collection, import selection and active-module DAG freeze
    -> semantic analysis and normalized source-backed templates
    -> StagedModule
         |- Typed Core InkIR
         |- ElaborationPlan
         `- NormalizedTemplateTable
    -> verifyStaged
    -> elaboration / partial-evaluation fixed point
    -> ClosedModule[target]
    -> verifyClosed
         |- Closed InkIR interpreter
         `- TargetABI lowering -> LLVM IR -> asm / obj / exe
```

`StagedModule` 是编译阶段容器，不表示其中每一项都是 CFG。已经完成 elaboration 的代码使用强类型 Core InkIR；尚未选择或尚未实例化的源码体保存在 `NormalizedTemplateTable`，由 `ElaborationPlan` 引用。

这一区分是语义要求，而不是实现优化。未选中的 `comptime if` 分支、dependent 泛型体和异构 `comptime for` 的每轮 body 按语言规则不能提前完成名称绑定和类型检查。

## 2. 两种可执行产物

### 2.1 StagedModule

`StagedModule` 可以包含：

- 已闭合或部分闭合的 typed function/global/type 定义；
- 允许 meta type 和 comptime-only value 的 typed operation；
- deferred function、statement 和 declaration template；
- generic instance request；
- 结构化 stage 选择与展开计划；
- comptime dependency manifest 和 capability 要求；
- 尚待固定点提交的生成声明与 typed module-registration 批次；
- 已经原子提交、按 module version 拥有的 frozen registration records。

只有 `VerifiedStagedModule` 可以交给 fixed-point elaborator。普通 RuntimeWorld 和 LLVM backend 不接受它。

### 2.2 ClosedModule[target]

`ClosedModule[target]` 必须满足：

- active module DAG 已冻结；
- 所有名称、重载、generic argument、pack 和布局均已解析；
- 所有函数签名和数据类型都可在目标世界表示；
- 不含 `stage.*` plan node、deferred template、meta value、开放声明句柄或 host capability；
- 每个 call target 已闭合，间接调用也有准确 function type；
- 所有 PDB、浮点模式、布局和目标地址规则都有完整 TargetContext；
- 异常、清理、对象生命周期和控制流已经显式化到 Closed verifier 所要求的程度；
- 不含 `ct.register_module_item`，但保留通过 `StaticRegistrationEncodable` 验证的 readonly registration records 及其受控 relocation。

只有 `VerifiedClosedModule<TargetKey>` 可以进入 Closed interpreter 或 TargetABI/LLVM lowering。

## 3. 自研 IR 的抽象模型

InkIR 采用四层递归结构：

```text
Module
  Symbol
    Region
      Block(block arguments...)
        Operation(operands, results, attributes, nested regions, successors)
```

### 3.1 Operation

operation 是最小语义单元，具有：

- 稳定的命名空间 opcode，例如 `arith.add`、`mem.load`、`cf.cond_br`；
- 零个或多个 typed operand；
- 零个或多个 typed SSA result；
- schema 定义的 attribute；
- 零个或多个 nested region；
- 对 terminator 而言，零个或多个 block successor 及对应实参；
- 非空 `OriginId`；
- 由 opcode schema 推导的 effect、unwind、trap、stage 和 speculatability 信息。

序列化文件不得自行声明一个可以覆盖 opcode schema 的“pure”“nothrow”或“speculatable”布尔值。

### 3.2 Region 与 Block

region 包含有序 block。每个 block：

- 有唯一 block ID；
- 通过 block argument 接收来自前驱的 SSA value；
- operation 按语义顺序排列；
- 必须以 terminator 结束；
- terminator 后不得再有 operation。

InkIR 不使用独立 `phi` instruction。控制流合流值由 block argument 表示。

### 3.3 SSA value

SSA value 由 block argument 或 operation result 定义；function receiver、logical parameters和隐藏result/runtime channels都已经是entry block arguments，没有独立 parameter ValueId。定义在所属 region 的支配规则下使用。SSA value 不等同于对象存储：

- 只有单标量表示的整数、浮点、`bool`、raw pointer、reference、function pointer、TargetABI 明确为单标量的无载荷 enum，以及无物理载荷的专用 unit `()` 直接作为 SSA value；
- slice、interface reference、非空 tuple、array、class、带载荷 enum、其他多字 aggregate、noncopyable、address-only 或需要稳定地址的值通过 place 和最终存储操作；
- SSA value 不拥有隐式析构责任；对象生命周期附着于 place 中的已初始化对象。

TargetABI 可以在证明不会产生额外语言级 copy、析构或地址变化时，把可复制 aggregate 的物理参数拆进多个寄存器；这只是 lowering，不把该类型重新分类为 Core InkIR SSA value。

### 3.4 Place

place 是指向准确类型存储的 IR 能力，不是用户可构造的普通裸地址。它记录：

- element type；
- 可读、可写或只初始化访问能力；
- 对齐保证；
- address space；
- 必要时的动态对象/子对象身份；
- verifier 可追踪的初始化状态。

place 可以降低成目标指针，但在 InkIR 中不得用普通整数或 raw pointer 冒充。place 本身不能存入用户对象、跨任意 FFI 或进入 Closed 常量。

## 4. Module 内容

逻辑 module 至少包含以下表：

```text
ModuleHeader
SourceFileTable
OriginTable
TypeTable
AttributeTable
ConstantTable
SymbolTable
GlobalTable
FunctionTable
RuntimeDeclarationTable
ModuleRegistrationTable
DependencyManifest
ElaborationPlan?          // Staged only
NormalizedTemplateTable? // Staged only
```

Staged 和 Closed module 都必须携带完整 `TargetKey` 与相应 `TargetContext`。不需要目标布局的受限 import-selection profile 发生在 `StagedModule` 建立之前，其状态称为 pre-Staged profile state，不得序列化或伪装成本规范的 Staged artifact。

`ModuleRegistrationTable` 是逻辑 module entity；binary 中使用独立 required semantic section。Staged artifact 只含已经 fixed-point commit 的记录，Closed artifact 含最终全集。每条记录的运行时 version owner由 loader从当前 artifact/publish transaction附加，不把可伪造的进程内 handle序列化进 IR。

## 5. 符号与身份

### 5.1 规范符号身份

IR 内符号使用层级路径：

```text
@canonical.module::Type
@canonical.module::function#overload-key
@canonical.module::function#overload-key::local-helper
```

文本中的可读名称不是唯一身份的全部内容。实现必须将声明种类、规范签名、闭合 generic arguments 和必要的 source-backed declaration key 纳入 `SymbolKey`。

### 5.2 不允许序列化的身份

下列内容不得作为跨进程身份：

- C++ 对象地址、arena index 的偶然值或 vtable pointer；
- 当前进程中的 `DeclId`、`Type*`、`Operation*`；
- 相对 import 拼写、module alias 或 checkout 的绝对物理路径；
- fixed-point round、线程编号或任务完成序号。

开放声明句柄在缓存中使用可重绑定的 canonical module identity、module content digest 和 declaration structural path。

## 6. Source origin

每个 symbol、block argument 和 operation 都必须能够追溯到 `OriginId`。origin 是 provenance DAG，而不是单个文件偏移：

```text
Source(file, range)
Instantiation(generic_decl, request_site, closed_instance, parent)
RegionExpansion(control, selected_body, iteration_identity, parent)
Synthetic(reason, nearest_source_parent)
Merged(primary, related_origins)
```

`Source` 使用 `SourceFileId + SourceRange`，其中 range 是文件内 UTF-8 字节的半开区间 `[Start, End)`。生成声明始终引用真实 source template，不创建不存在的虚拟源码。

origin 默认不进入语义内容 hash，但必须绑定准确 source content digest。诊断调用栈由执行 frame 与 origin context 重建，不要求每个 operation 复制整条栈。

## 7. 类型与布局边界

语义类型身份和目标物理布局分离：

- named class/enum/interface 使用 nominal identity；
- tuple 使用规范化 structural identity；
- alias 在进入 Core InkIR 前规范化；
- `int`、`uint` 和 `byte` 等 canonical alias 在 IR 中分别写成 `i64`、`u64` 和 `u8`；
- 字段、基类、tuple element 和 enum payload 的 offset 由 TargetContext 查询；
- runtime opaque type 只暴露允许的语义 operation，不在 Closed InkIR 中展开私有字节布局。

布局结果可以进入 Closed IR attribute 和常量，但必须受 `TargetKey` 约束。把一个 Closed module 换到不同 TargetKey 上执行或 lowering 是 verifier 错误。

## 8. Effect 模型

每个 opcode schema 声明一个或多个 effect：

```text
Pure
ReadMemory(alias-domain)
WriteMemory(alias-domain)
Allocate(storage-kind)
Deallocate(storage-kind)
BeginLifetime
EndLifetime
MayUnwind
MayTrap
MayDiverge
TargetPDB
ComptimeEffect(capability)
RuntimeEffect(handler)
Control
DeclarationSink
```

这些 effect 用于解释器调度、optimizer 合法性、comptime capability 检查和 verifier。`MayDiverge` 表示 operation 可能没有 normal、unwind 或 trap completion；不能根据某个当前实现体看起来会终止而消去该上界。没有 effect 仍不代表可以任意删除：产生诊断的 stage plan node、具有 source-order 契约的 operation 和另有不可消去 runtime handler 的 operation 仍受各自 schema 约束。

v0 采用隐式顺序效果模型：同一 block 中的 operation 按顺序执行，不额外传递显式 memory token。pass 只有在 effect/alias 分析证明不改变可观察行为时才可重排。

启用 code-only hot reload 的 stable entry 不采用当前 version-local body 的精确摘要，而采用 compatibility lineage 首次基线中固定的 `StableEffectEnvelope` 上界，并额外固有不可消去、不可 CSE、不可推测、不可复制的 `RuntimeEffect(version_select)`。当前发布的 `BehaviorContracts` 可以把异常通道单调加强为 nothrow/no-fail，但后续兼容版本不得减弱；exact body 必须同时满足固定 envelope 与当前 contract。只有调用已固定到持有匹配 version pin 的 version-local entry，或全程序证明已永久禁用该更新边界时，optimizer 才能改用具体 body summary；envelope/contract 的字段、digest 和兼容规则见 [`06-calling-convention-runtime.md`](./06-calling-convention-runtime.md)。

同一次 module publish transaction 还原子切换 stable entries、reflection metadata 和 typed registration set。`ModuleRegistrationInterfaceDigest` 对当前 records 中出现的 unique `(RegistrationTypeSemanticIdentity, ProtocolSchemaDigest, TargetLayoutDigest, RegistrationEncodingRevision, RuntimeAbiRevision)` schema tuple 集合定界兼容性；在该集合不变时 record identity/value set 可以随兼容版本变化。增加某 schema 的第一条 record 或删除其最后一条会改变该集合，v0 不把这种 0↔N 变化视为 code-only update。旧 set只按不可伪造的旧 version owner撤销，不能按业务逻辑键误删新版本记录。安装/撤销 descriptor不是用户生命周期 hook，也不执行 registration value 的普通 constructor/destructor。

## 9. TargetContext 与 TargetKey

TargetContext 至少提供：

- target triple、ABI、CPU 与 feature set；
- data layout、endianness、各 address space 指针宽度和对齐；
- primitive/aggregate/class/enum/interface 的布局服务；
- PDB 操作表及其 lowering revision；
- strict float、subnormal 和 `nan_mode`；
- C ABI 分类和平台异常 ABI 选择；
- RuntimeABI revision；
- hot-reload ABI/layout hashing 规则。

`TargetKey` 是这些语义输入的规范摘要，不能只保存 target triple。任何 target-dependent constant folding、comptime 结果或二进制缓存都必须包含该摘要。

## 10. 执行入口

核心 API 应当以验证后的能力类型表达阶段边界：

```text
verifyStaged(UnverifiedStagedModule) -> VerifiedStagedModule
closeAndVerify(VerifiedStagedModule, TargetContext, ComptimePolicy) -> VerifiedClosedModule<TargetKey>
interpret(VerifiedClosedModule<TargetKey>, EntrySymbol, RuntimeWorld)
lowerToLLVM(VerifiedClosedModule<TargetKey>, TargetABI)
```

不得只在可修改的普通 `Module` 上设置 `StageKind = Closed`，然后让 backend 信任该字段。阶段能力必须由完整 verifier 成功构造。

## 11. 确定性

以下输出必须由规范遍历产生：

- function、block、SSA value 和 origin 的打印编号；
- type、constant、symbol 和 section table 的顺序；
- declaration expansion 与 generic instance identity；
- cache key 和 semantic digest；
- effectful comptime work item 的提交顺序。

规范顺序不得依赖哈希表迭代、指针地址或并行完成顺序。pure/read-only 工作可以并行，但提交的 declarations、diagnostics、effects 和最终 IR 必须与规范串行模型等价。
