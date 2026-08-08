# Ink 编译器实现 Roadmap

## 1. 总体方向

Tokenizer 和 Parser 完成后，下一阶段应优先建设语义层和最小端到端纵切片，而不是直接实现完整 AOT，也不应建立独立的 AST 解释器。

Ink 编译流水线采用以下结构：

```text
CST
 ↓
紧凑 AST + 语义 side tables
 ↓
声明收集 / 名称绑定 / 类型检查
 ↓
Staged InkIR
 ↓ comptime + 泛型实例化 + residualization
Closed InkIR[target]
 ├─ RuntimeWorld 参考解释器
 └─ LLVM lowering → LLVM IR → object / executable
```

其中：

- CST 保留完整源码结构、Trivia、错误节点和缺失 Token；
- AST 只承载后续语义分析所需的抽象结构；
- Staged InkIR 可以包含开放泛型、编译期元值和阶段控制；
- 固定点展开和部分求值产生目标相关的 Closed InkIR；
- RuntimeWorld 解释器和 LLVM AOT 后端都只执行或接收 Closed InkIR；
- LLVM 不负责理解 Ink 泛型、反射、`comptime` 或声明展开。

实现顺序采用“解释器优先、AOT 早期薄验证、comptime 主体随后、AOT 最终产品化”的策略：

1. 先用参考解释器验证 InkIR 和语言运行时语义；
2. InkIR 核心子集稳定后立即接入一个最小 LLVM lowering，防止 IR 过度偏向解释器；
3. 在共享执行核心上实现 `Known / Runtime` 部分求值和编译期执行；
4. Closed InkIR 边界稳定后，再完善目标 ABI、链接和完整 AOT 工作流。

## 2. 首个纵切片范围

首个可执行纵切片刻意限制在以下功能：

- `i32`、`bool` 和 `void`；
- 整数与布尔字面量；
- `var` 和 `const` 局部绑定；
- 普通函数、参数、返回值和函数调用；
- 基本算术、比较和逻辑运算；
- `if`、`while` 和 `return`；
- 一个最小 trap 或输出 intrinsic；
- 随后加入值泛型、`comptime expression` 和 `comptime if`。

以下功能不进入首个纵切片：

- class、interface 和 enum；
- 指针、引用、数组、切片和复杂聚合；
- RAII、析构和 `defer`；
- 异常；
- 反射和 decorator；
- async、Task 和调度；
- 完整 FFI 和稳定外部 ABI。

语法已经支持这些结构，不表示语义层必须一次性横向实现全部语法。尚未启用的语义功能应产生明确的能力诊断，不得触发断言或内部崩溃。

## 3. 里程碑

### M0：稳定 Parser 基线与工程门禁

工作内容：

- 将当前 Parser、Core、CMake 和集成测试变更整理为独立、可回归的完成基线；
- 明确首个纵切片支持的语法与语义能力；
- 建立 compile-pass、compile-fail、IR golden、run 和跨执行引擎 differential 五类测试；
- 为 AST、Staged InkIR 和 Closed InkIR 约定确定性的文本 dump；
- 为每个新编译阶段定义输入不变量、输出不变量和 verifier。

完成门槛：

- Tokenizer 和 Parser 全部测试稳定通过；
- 相同输入的 dump 可以重复产生相同结果；
- 测试可以区分语法错误、名称错误、类型错误、阶段错误、IR 错误和后端错误。

### M1：CompilationSession、源码身份与 AST

工作内容：

- 增加 CompilationSession，集中管理一次编译的文件、模块、声明、类型、目标和诊断；
- 增加 SourceManager 和 SourceFileId，使 SourceRange 保持文件内半开字节范围，同时由所属上下文携带文件身份；
- 增加稳定的 ModuleId、DeclId、ExprId、TypeId 和 FunctionId；
- 对 Identifier 和其他高频字符串进行 interning；
- 建立 TargetContext，保存 target triple、指针宽度、字节序、数据布局和目标能力；
- 实现 CST 到紧凑 AST 的 lowering；
- AST 节点保存 SourceRange 和必要的 CST origin，但不复制 Trivia；
- 将错误 CST 结构 lowering 为可继续分析的 AST 错误占位节点。

AST 和 HIR 不应成为两棵内容高度重复的树。初期优先采用“紧凑 AST + 语义 side tables”；如果后续将该层命名为 HIR，则应让它取代重复的 Typed AST，而不是再增加一层机械副本。

完成门槛：

- 所有当前 CST 结构都能安全 lowering；
- 错误输入不会导致 lowering 崩溃；
- AST dump 确定且不依赖内存地址；
- 多文件诊断能够准确关联 SourceFileId 和 SourceRange。

### M2：最小语义分析

工作内容：

- 顶层和局部声明收集；
- 词法作用域、名称绑定和遮蔽规则；
- builtin type 注册；
- canonical Type interner 和稳定 TypeId；
- 表达式期望类型与实际类型；
- place、value、可写性和 constness；
- 函数签名、参数、调用和返回检查；
- 最小转换规则和重载集合基础；
- ErrorType/ErrorValue，使一次错误后可以继续分析；
- 在 Core 统一诊断表中追加 INK-S 语义诊断。

第一阶段只要求单 module 核心程序完成语义检查，多 module 能力在后续里程碑接入。

完成门槛：

- 每个非错误表达式都具有 TypeId 和 value category；
- 可以正确检查首个纵切片中的完整函数；
- 重定义、未解析名称、不可写 place、实参不匹配和返回类型错误具有稳定诊断；
- 语义层不依赖 LLVM Type。

### M3：Typed InkIR 与参考解释器

工作内容：

- 定义 module、declaration、function、basic block、operation 和 value 的稳定身份；
- Staged InkIR 从一开始就是强类型 IR；
- 首批操作覆盖常量、算术、比较、转换、place、load/store、call、branch、return 和 trap；
- 每个操作记录类型、效果类别和源码 origin；
- 建立 Staged InkIR verifier 和 Closed InkIR verifier；
- 实现共享的 frame、调用栈、控制流、值和语义操作框架；
- 实现 RuntimeWorld 参考解释器；
- 目标整数、浮点、指针和内存语义读取 TargetContext，不使用宿主 C++ 类型偶然代替。

初期可以使用 typed CFG、虚拟寄存器和显式内存操作，不要求一开始完成优化型 SSA。可变局部变量可以先通过 place/load/store 表示，后续再做提升。

完成门槛：

- 核心程序可以从源码经过完整前端生成 InkIR；
- Closed InkIR 可以由 RuntimeWorld 执行；
- 函数、递归、分支和循环具有调用栈与资源限制；
- 每个 IR 输入在执行前通过 verifier；
- 整数溢出、除法、移位和 trap 行为符合 Ink 语义，而不是直接继承宿主 C++ 行为。

### M4：LLVM AOT 薄竖切

工作内容：

- 为 M3 的最小 Closed InkIR 操作集实现 LLVM lowering；
- 建立 LLVMContext、Module、TargetMachine 和 DataLayout；
- 输出 LLVM IR 和 object；
- 使用最小入口与链接流程生成本机 executable；
- 对每个 LLVM Module 运行 LLVM verifier；
- 建立 interpreter 与 LLVM O0 的 differential 测试。

该阶段只验证 Closed InkIR 的后端边界，不追求完整运行时、跨目标 ABI、异常、调试信息或优化。

完成门槛：

- 同一核心程序在参考解释器和 LLVM O0 下具有相同 stdout、stderr、退出状态和 trap 分类；
- 前端和语义模块不包含 LLVM Type；
- LLVM lowering 只接收 Closed InkIR。

### M5：Comptime C0/C1 与部分求值

#### C0：纯编译期求值

工作内容：

- 为执行核心增加 ComptimeWorld；
- 支持纯标量运算、普通函数调用和目标配置查询；
- 实现 `comptime expression` 和 `comptime if`；
- 强制编译期上下文要求最终值完全已知；
- 为执行步数、递归深度、循环次数、内存和调用栈设置预算；
- 编译期外部效果默认拒绝。

#### C1：Residualization 与显式泛型函数

工作内容：

- 引入 `Known(value)`、`Runtime(ir_value)` 和内部错误/不可达状态；
- 混合静态与动态输入时执行已知计算并生成残留 InkIR；
- 实现显式泛型参数绑定；
- 以“泛型声明身份 + 规范化编译期实参 + TargetContext + 受跟踪依赖”为实例缓存键；
- 检测进行中的递归实例、无限实例序列和代码膨胀；
- 保证普通运行时副作用不会仅因实参已知而被移动到编译期；
- 建立 ResidualizeWorld，并与 RuntimeWorld、ComptimeWorld 共享操作语义。

验收示例：

- `comptime fibonacci(10)` 完全求值得到已知结果；
- `add::<2>(runtime_value)` 残留为对运行时值加常量 2；
- 同一个纯函数在 RuntimeWorld 与 ComptimeWorld 中结果一致；
- 部分求值前后的程序在 RuntimeWorld 中结果一致；
- Closed InkIR verifier 保证没有开放泛型、编译期专用元值、未决名称、未决类型或阶段操作。

### M6：Module、条件 Import 与 AOT MVP

工作内容：

- manifest/source-root 和规范 package/module 身份；
- 静态 import、相对 import、成员 import 与别名；
- active module DAG、环检测和拓扑处理；
- 条件 import 先收集有限候选边，再用 C0 ComptimeWorld 求值 guard；
- 未选中的 import 分支不进入名称绑定和类型检查；
- 编译期声明区域不得在 active DAG 冻结后创建未知 import；
- 完善目标布局、调用约定、入口 ABI、最小 runtime 和链接流程；
- 支持 `--emit=ink-ir`、`--emit=llvm-ir`、`--emit=asm`、`--emit=obj` 和 `--emit=exe`；
- 建立 interpreter、LLVM O0 和 LLVM O2 三方 differential 测试。

条件 import 的处理顺序固定为：

```text
parse candidate modules
→ collect finite candidate imports
→ evaluate import guards
→ normalize module paths
→ build active module DAG
→ reject cycles
→ topologically perform complete semantic analysis
```

完成门槛：

- 静态和条件 import 均遵守 active DAG；
- inactive import 不要求目标 module 存在或通过语义检查；
- 可以生成本机可执行文件；
- 解释器、AOT O0 和 AOT O2 对核心程序持续保持一致。

### M7：聚合、对象与生命周期

建议按以下顺序逐步加入：

1. tuple、array、slice、pointer、reference 和 target layout；
2. aggregate initialization、place projection 和边界检查；
3. class 和 enum 的基础表示、构造和 match；
4. 按值复制、copyable/noncopyable 和保证原地构造；
5. 确定初始化、部分初始化和临时对象生命周期；
6. 析构、RAII 和 `defer`；
7. return、break、continue 和普通作用域退出的统一 cleanup plan。

IR 应显式表示初始化状态、drop、cleanup edge 和必要的 drop flag。不得让 AST 解释器、RuntimeWorld 和 LLVM 后端分别推导三套清理规则。

完成门槛：

- 正常退出路径上的析构和 `defer` 顺序准确；
- 构造失败只清理已经成功初始化的部分；
- 命名不可复制对象不会通过隐藏的 C++ move 绕过 Ink 语义；
- 解释器和 AOT 的生命周期事件序列一致。

### M8：完整 Comptime C2、反射与动态分派

工作内容：

- `type`、FunctionDecl、GenericDecl 等编译期元值；
- 类型泛型、class 泛型和参数包；
- 异构编译期 tuple 和静态迭代；
- Statement、TopLevel、ClassMember、InterfaceMember 和 EnumMember region sink；
- 声明选择、重复展开和固定点提交；
- 每轮采用稳定声明快照，生成结果验证后事务化提交到下一轮；
- 展开结果重新执行访问检查、名称绑定、类型检查、布局依赖检查和 IR verifier；
- interface、继承、vtable 和动态销毁；
- 编译期结构反射、运行时登记元数据和 decorator；
- 禁止字符串转源码、动态 Identifier splice 和任意裸 IR builder。

完成门槛：

- 相同实例只生成一次；
- inactive comptime 分支不执行、不绑定也不检查依赖语义；
- 重复声明、布局环、反射当前未完成类型、固定点不收敛和代码量超限均有明确诊断；
- 动态反射只能观察已经生成并登记的闭合实例；
- 元值不能逃逸到 Closed InkIR 或运行时 ABI。

### M9：同步异常

工作内容：

- throw、rethrow、try/catch 和类型匹配；
- exception record、cause 和 traceback；
- 正常控制流与异常 unwind 复用 M7 的 cleanup plan；
- 异常存储和 runtime ABI；
- LLVM 平台异常 lowering；
- 未处理异常的 fail-fast 边界。

RAII 必须在异常之前稳定。异常实现不得新建第二套析构或 `defer` 规则。

完成门槛：

- 正常退出和异常退出具有一致的清理语义；
- 部分初始化对象在 unwind 中正确清理；
- interpreter 与 AOT 对 catch 选择、rethrow、cause 和清理顺序一致。

### M10：Async 与 Task

建议按以下顺序加入：

1. 单任务 lazy async/await；
2. Task 生命周期和 pending 析构检查；
3. 失败结果与异常传播；
4. 解释器 continuation 和确定性测试 scheduler；
5. AOT 状态机 lowering；
6. `await all`；
7. 协作式取消；
8. virtual/interface async 分派；
9. 动态反射 async 调用；
10. async decorator。

Async 放在最后，是因为它同时依赖生命周期、异常、动态分派、解释器挂起恢复、运行时调度和 AOT frame ABI。

### M11：工程化与性能

工作内容：

- module query 和增量编译缓存；
- 编译期外部依赖的内容哈希与失效策略；
- 并行 module 编译；
- 基础 InkIR 优化；
- debug info 和 PDB；
- 标准库、package、linker 和构建体验；
- 交叉编译；
- fuzz/property 测试；
- 性能分析与编译时间预算。

只有参考解释器已经成为明确瓶颈时，才考虑从 Closed InkIR 或后续低级 IR lowering 到 bytecode VM。Bytecode 不应反向成为 Ink 语言语义或 comptime 的唯一规范表示。JIT 和 ORC 同样不进入当前核心路线。

## 4. 必须保持的架构边界

### 4.1 不解释 CST 或 AST

CST 和 AST 用于语法与语义分析，不作为运行时或 comptime 的规范执行形式。否则普通解释、编译期执行和 LLVM lowering 会形成三套语义。

### 4.2 一个执行核心，多个 World

以下环境共享指令、调用栈、控制流、内存、异常和生命周期等语言语义：

- ComptimeWorld：要求编译期完成的完整求值；
- ResidualizeWorld：执行已知部分并生成残留 InkIR；
- RuntimeWorld：解释 Closed InkIR。

不同 World 只在允许的值、效果 capability 和操作 handler 上存在差异。

### 4.3 Closed InkIR 是硬边界

Closed InkIR 必须满足：

- 已固定 TargetContext；
- active module DAG 已固定；
- 不含开放泛型参数或参数包；
- 不含编译期专用元值；
- 不含未决名称、类型或布局；
- 强制 comptime 位置不依赖 Runtime 值；
- 所有运行时调用目标闭合；
- 所有操作通过 verifier。

解释器和 LLVM 后端均不得绕过该边界。

### 4.4 Target 不等于 Host

以下语义必须来自 TargetContext：

- `ptrsize`；
- 字节序；
- 类型大小、对齐和字段偏移；
- 整数与浮点运算规则；
- 地址和指针表示；
- ABI 和调用约定。

编译期临时内存、宿主地址、文件句柄和其他宿主资源不得残留到运行时程序。

### 4.5 Comptime 效果默认拒绝

文件、环境变量、网络、时钟、随机数、进程和目标 extern call 等操作必须经过显式 capability 和 handler。允许的外部输入必须进入依赖记录或不可复现性记录，并参与缓存失效。

### 4.6 LLVM 只属于后端

前端类型、语义分析和 InkIR 不使用 LLVM Type 作为语言类型身份。LLVM lowering 负责：

- 将已经闭合的 Ink 类型和布局映射为 LLVM 类型；
- ABI lowering；
- 目标合法化；
- LLVM IR verification；
- object/assembly 生成。

当前构建只启用 LLVM Native target，因此 AOT MVP 先支持宿主目标；交叉编译在工程化阶段再扩展。

### 4.7 不提前固定多余 IR 层

首个版本可以从 Closed InkIR 直接 lowering 到 LLVM IR。当 RAII、异常、async 或多个低级后端确实需要显式 cleanup、unwind、ABI 或状态机表示时，再增加可验证的 Lowered/Codegen InkIR。不得仅为了形式完整提前复制一套 Runtime MIR。

## 5. 测试门禁

每个语言功能必须同时具备适用的以下测试：

- compile-pass；
- compile-fail，断言 DiagnosticKind、SourceFileId、SourceRange、Arguments 和 Related；
- AST/IR 确定性 golden；
- RuntimeWorld 执行；
- 强制 comptime 执行；
- residualization 前后等价；
- interpreter、LLVM O0 和 LLVM O2 differential；
- 多 TargetContext 语义测试；
- 资源预算和取消测试；
- verifier 对非法 IR 的拒绝测试。

最高价值的长期测试关系是：

```text
普通运行纯函数
    ==
comptime 运行同一函数
    ==
原程序 RuntimeWorld
    ==
residual program RuntimeWorld
    ==
LLVM AOT O0
    ==
LLVM AOT O2
```

## 6. 近期实施顺序

最值得立即实施的四组变更为：

1. 固定 Parser 完成基线，并建立新阶段测试门禁；
2. 增加 CompilationSession、SourceManager、稳定 ID、字符串 interning 和 TargetContext；
3. 增加 CST 到 AST lowering、AST verifier 和确定性 dump；
4. 增加声明收集、名称绑定、TypeId interner 和首个单文件语义检查纵切片。

完成上述四组变更后，进入 Typed Staged InkIR、Closed InkIR verifier 和 RuntimeWorld 参考解释器。公共完整编译驱动继续遵守既有的 `ink --emit=...` 方案，不把编译阶段拆成互不一致的独立语义管线。
