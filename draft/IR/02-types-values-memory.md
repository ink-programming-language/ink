# InkIR 类型、值与内存模型

## 1. 适用范围

本文规定 typed Core InkIR 的类型身份、常量、SSA value、place、目标布局、裸指针、原始字节、对象生命周期和复制语义。本文同时适用于 StagedModule 中已经类型化的 Core InkIR 与 ClosedModule；阶段专用 meta type 和不可逃逸规则由 [`05-staging-comptime.md`](./05-staging-comptime.md) 补充。

本文不固定 LLVM 类型、寄存器分类、vtable 字节布局或公开 Ink-to-Ink ABI。物理布局由 `TargetContext` 和私有 `TargetABI` 决定，逻辑类型及其可观察语义由本文决定。

## 2. 类型身份与规范表示

### 2.1 内建标量类型

Core InkIR 直接使用以下规范类型名：

```text
i8 i16 i32 i64 i128
u8 u16 u32 u64 u128
ptrsize
f16 f32 f64
bool
void
never
()
```

源码别名在进入 Core InkIR 前规范化：

```text
int  -> i64
uint -> u64
byte -> u8
```

`void` 表示函数没有结果值；`never` 表示控制流不能正常返回；`()` 是具有一个普通零元素值的空元组类型。三者不得合并。

整数类型携带准确位宽与 signedness。`ptrsize` 是独立语义类型，宽度由目标默认地址空间决定，数值语义为同宽无符号整数。`bool` 只有 `false` 和 `true` 两个有效值，不能与整数隐式互换。

### 2.2 复合类型

复合类型在 `TypeTable` 中驻留。canonical operation 中普通复合类型使用 `!tN`；为了让 verifier capability 与 sealed runtime channel 一眼可辨，printer 对 `place`、`exception`、`runtime-handle` 和 `runtime-object` 使用由文本格式规范固定的 inline spelling，但该 spelling 仍必须结构匹配 TypeTable 中唯一 record，不能隐式创建或省略类型。下列描述符至少覆盖：

```text
ptr<rw, T, address-space>
ptr<ro, T, address-space>
ref<rw, T>
ref<ro, T>
slice<rw, T>
slice<ro, T>
array<T, N>
tuple<T0, T1, ...>
nominal.class<@type-symbol>
nominal.enum<@type-symbol>
nominal.interface<@interface-symbol>
function<calling-convention, parameters, result>
function-pointer<function-type>
runtime.object<@runtime-kind, type-arguments...>
runtime.opaque<@runtime-type-symbol>
```

`rw` 与 `ro` 只描述当前访问路径。它们不递归改变内部指针或引用目标，也不隐含唯一别名、线程安全或底层存储事实上可写。

named class、enum 与 interface 使用名义身份。tuple 使用规范化后的有序元素类型列表作为结构身份。数组长度是类型身份的一部分。类型别名不得产生第二个 Core 类型身份。

`ptr<A,T,S>` 允许 T 是 `void`、`never`、函数、incomplete nominal 或 `runtime.opaque`，但这只建立带类型标签的数值地址。`ref<A,T>`、`slice<A,T>`、`array<T,N>`、普通对象字段和 `!place<A,T>` 要求相应对象/元素满足下文的 `SizedObjectType`；function pointer 使用独立 `function-pointer<function-type>`，不把函数签名伪装成可解引用对象。

`runtime.object<kind,args...>` 与 `runtime.opaque`、`!runtime-handle` 是三个不同类别。runtime object 是能占据 caller/storage 的 address-only、noncopyable、stable-address 语义对象；TargetContext 与 `RuntimeAbiRevision` 为闭合 kind/arguments 提供有限 size、alignment 与 `RuntimeStorageAbiHash`，所以它可以满足 `LayoutComplete`/`SizedObjectType`，并恒有 `SealedRuntimeStorage`。其字段、有效字节表示和私有状态仍不可见，只能由该 kind 注册的 runtime opcode 创建、查询和销毁；禁止普通 field projection、aggregate constant、`place.addr`/`place.deref`、typed/raw load/store、`obj.init.copy`、`obj.assign.copy` 或任意 byte-count memory operation 访问或重叠其 active/partial storage。需要把地址交给私有 runtime 时，由对应 schema 直接接受 typed place，不先暴露通用 raw pointer。`Task<T>` 属于 runtime object；v0 只允许 `T` 为 `void`、`never` 或闭合 runtime-representable `Copyable(T)`，并且 `ContainsNoEscapeValue(T)` 必须为 false，所以 `Task<File>` 与 `Task<safe-slice>` 都在 type/function verification 时拒绝，而不是生成一个永远无法合法 await 的 Task。`runtime.opaque` 没有可供 Core 使用的 size/alignment，仍只能作为 opaque pointer pointee；`!runtime-handle` 是不可存储的 verifier/runtime capability，不是对象类型。

`StaticRegistrationEncodable(T, Constant)` 是独立的中央派生谓词，不等于 `Copyable(T)`、`NeedsDestroy(T) = false` 或普通对象可以跳过生命周期。它只证明某个准确 typed constant能形成 readonly frozen module-image representation，且安装/撤销不需要执行用户代码或承担独立释放责任。v0可内建支持 String 的 module-owned immutable bytes/length frozen encoding以及递归aggregate；用户类型不能仅靠attribute自行声称该性质。frozen record在table中按只读T view使用，不获得普通可变对象、copy、assign或destroy语义。

### 2.3 函数类型

函数类型记录：

- 调用约定类别；
- 同步或由运行时文档规定的特殊调用类别；
- 有序参数语义类型和参数传递模式；
- 逻辑结果类型与结果传递模式；
- 成员接收者及其 `rw`/`ro` 限定；
- 对 virtual/interface override 有意义的准确签名成分。

`[nothrow]`、`[fast_math]`、可见性、热更新状态和默认实参不进入普通函数类型身份。它们属于声明契约、函数体 operation flags 或调用点效果。通过无法静态确定声明契约的函数值进行间接调用时，必须按可能 unwind 处理。

参数和结果使用以下逻辑分类术语：

- `value`：单标量值或专用 unit 值，可以直接作为一个 SSA value 传递；
- `object`：语义上按值，但必须物化在准确最终存储中；
- `result-place`：由调用者提供的最终结果位置。

`FunctionSignature` 的规范 parameter-passing 枚举为 `value|object|const_reference|mutable_reference|raw_pointer`，result-passing 枚举为 `value|result_destination|void`。`value` 与 `object` 分别承载上述同名逻辑分类；`const_reference`、`mutable_reference` 和 `raw_pointer` 明确表示不建立按值参数对象的引用或裸指针通道。`result_destination` 是本文概念术语 `result-place` 在函数签名、规范文本与二进制记录中的拼写，不是第四种对象分类。`void` 不产生值或结果位置。`()` 具有一个逻辑 unit 值，但没有物理载荷；它可以使用专用 unit 常量表达，不得借此承载任意聚合。多字 descriptor 和任何聚合都不得归入 `value` 模式。

具体寄存器、栈、隐藏参数和 C ABI 分类不进入函数类型身份，由 [`06-calling-convention-runtime.md`](./06-calling-convention-runtime.md) 与 TargetABI lowering 决定。

### 2.4 IR 内部能力类型

以下类型只用于 IR 验证和执行，不是用户可声明的普通值类型：

```text
!place<access, T>
!exception
!runtime-handle<kind>
```

place 是对准确 typed storage 的能力，不能存入普通对象、形成普通常量或跨任意 FFI。`!exception` 和 runtime handle 只通过各自 opcode schema 操作，不能用整数、裸指针或 `bitcast` 伪造。

## 3. 类型性质

TypeTable 为每个闭合类型保存或可重新推导以下语义性质：

```text
Copyable
NeedsDestroy
RegisterRepresentable
AddressOnly
HasStableAddressRequirement
ContainsNoEscapeValue
HasNicheSet
LayoutComplete
SizedObjectType
SealedRuntimeStorage
```

这些性质来自类型结构、声明属性和 TargetContext，反序列化时必须重新验证，不能信任缓存中的单个布尔摘要。

除 `void`、`never` 与专用 unit 例外外，普通闭合值类型的 `RegisterRepresentable` 与 `AddressOnly` 互斥。`Copyable` 与 `AddressOnly` 不互斥：slice、interface reference 和可复制聚合仍然可以是 address-only。

### 3.1 RegisterRepresentable

只有规范上恰好由一个标量表示的值可以直接作为 SSA value：

- 整数、浮点、`bool`、`ptrsize`；
- raw pointer、reference 和 function pointer；
- TargetABI 明确声明为单标量表示的无载荷 enum。

`()` 是没有物理载荷的专用 unit 值，不计入单标量 ABI 分类；`void` 完全不产生值。slice、interface reference、非空 tuple、array、class、带载荷 enum 和其他多字或聚合值即使很小、可复制或能被某个目标拆进寄存器，也不是普通 SSA value。

### 3.2 AddressOnly

以下类型必须通过 place 和最终存储操作：

- slice、interface reference 和其他多字 descriptor；
- class、array、非空 tuple 与带载荷 enum；
- `[noncopyable]` 类型；
- 具有析构责任或必须保持稳定地址的对象；
- 任何包含 address-only 子对象的类型；
- runtime schema 明确标记为 address-only 的对象。

TargetABI 可以把不可观察地址的 copyable aggregate 或多字 descriptor 物理拆进多个寄存器，但这只是私有 lowering：规范 IR 中仍使用 place、caller destination 和显式 typed copy。物理拆分不得产生额外语言级复制、移动、析构或地址变化，也不得把拆出的机器分量重新暴露成 Core InkIR SSA 聚合。

### 3.3 `LayoutComplete(T)` 与 `SizedObjectType(T)`

在给定完整 TargetKey 下，`LayoutComplete(T)` 表示 TypeTable 与 TargetContext 能唯一重建 T 的规范 storage layout：有限且可表示的 size/alignment、value representation，以及适用时全部 base/field/element/payload offset、stride、tag/niche 和隐藏 ABI 状态。它不能由“当前 backend 恰好能处理”或缓存布尔值代替；开放 generic、未解析 import、forward/incomplete nominal declaration 都不满足。

`SizedObjectType(T)` 是更强的 verifier predicate：`LayoutComplete(T)` 成立，并且 T 表示可以占据 storage、开始 typed lifetime 且具有有限 `sizeof(T)`/`alignof(T)` 的对象类型。以下类型永远不是 SizedObjectType：

- `void`、`never` 与函数签名类型 `function<...>`；
- incomplete nominal declaration；
- `runtime.opaque<...>`，即使某个 runtime 私下知道其表示；
- 只表示 interface 声明身份而不是具体 interface-reference descriptor 的 nominal interface 类型。

整数、浮点、`bool`、`ptrsize`、unit/空 tuple、raw pointer、reference、function pointer、闭合 `runtime.object`，以及 layout complete 的 array、非空 tuple、class、enum、slice/interface-reference descriptor 可以是 SizedObjectType。runtime object 的 LayoutComplete 只公开不透明 storage shell 的 size/alignment/ABI hash，不授予字段或 raw representation 访问。这里判断的是 T 自身：例如 `ptr<rw, void, 0>` 这个 pointer value自身是 SizedObjectType，但它的 pointee `void` 不是，所以可以存取该 pointer value，却不能对它执行元素缩放或形成 `!place<...,void>`。

raw pointer 类型允许 unsized pointee，以便表示 C opaque handle、函数/不完整对象地址和纯字节地址。凡 operation 需要 `sizeof(T)`、`alignof(T)`、typed object lifetime、有效表示或 typed dereference，其 T 必须满足 `SizedObjectType(T)`；只观察/变换地址 bits 的 `const.null`、`cast.ptr`、`cast.ptr_access`、`ptr.byte_offset` 和 `ptr.cmp` 不要求 pointee sized。byte-count `raw.memcpy`/`raw.memmove`/`raw.memset` 同样不依赖 pointee layout，但不会因此建立 typed 访问权。

## 4. 常量

### 4.1 常量类别

Core InkIR 常量包括：

- 准确位宽整数位模式；
- `bool`；
- 准确 IEEE 位模式的浮点常量；
- `null` raw pointer；
- `()`；
- 符号地址与目标重定位 addend；
- 全部组成部分均可静态编码的常量对象描述；
- TargetContext 已经解析的布局、判别或 PDB 结果。

十进制字面量只是一种可读输入。规范二进制和语义 hash 使用准确位模式。规范文本中的浮点常量必须能够无损恢复全部位，包括负零、Infinity、NaN payload 和 sNaN 位模式；实现应打印十六进制位模式，十进制说明只能作为非语义注释。

常量对象描述只用于常量表、global initializer 和直接原位初始化，不产生普通聚合 SSA value。读取其中的标量子值或把整个对象初始化到 storage，必须分别使用相应的标量常量或对象初始化 operation。

### 4.2 地址常量

Closed IR 中的地址常量只能是：

```text
null
symbol-address(@symbol, addend)
```

它们在解释器中映射到虚拟目标地址，在 AOT 中映射到重定位。宿主 C++ 指针、编译器 arena 地址和当前进程 descriptor 地址不得进入常量表。

任意数值地址必须先以 `ptrsize` 常量表示，再通过 `cast.ptr` 形成 raw pointer。该转换不证明地址有效。

### 4.3 不存在 `undef` 与 `poison`

InkIR 不提供 `undef`、`poison`、未初始化 SSA value 或“每次使用可以任选位模式”的常量。未初始化只存在于 storage 的字节初始化图和对象生命周期状态中。读取未初始化 storage 违反内存访问前置条件，不能成功产生一个 SSA value。

## 5. SSA value 与 place

### 5.1 SSA value

SSA value 由函数参数、block argument 或 operation result 唯一定义，并遵守 region 内支配规则。SSA value：

- 具有准确规范类型；
- 除专用 unit 值和单标量无载荷 enum 外，只能属于单标量 ABI 分类；
- 不携带隐式析构责任；
- 不表示尚未开始生命周期的对象；
- 不允许通过复制一个 value ID 暗中复制 address-only 对象；
- 可以在 block 边上传给类型完全一致的 block argument。

ResidualizeWorld 的 `Known` 与 `Residual` 是执行器 value domain，不是可以出现在 Closed TypeTable 中的类型。

### 5.2 Place

place 至少携带：

```text
ElementType
Access = init | ro | rw
AddressSpace
KnownAlignment
AllocationAssociation?
SubobjectPath
LifetimeFact
InitializationTransactionId?
PlaceCapabilityGeneration
LifetimeAuthority = owner | borrow
ObjectLifetimeGeneration?
```

place 与 raw pointer 不同：

- place 只能由验证过的 allocation、global、参数结果位置、投影、引用或显式裸指针解引用 operation 产生；
- place 不能执行普通整数运算；
- place 的 element type 不能由 `bitcast` 改写；
- place 自身不是用户可保存的值；
- 从 raw pointer 形成 place 不证明后续访问合法，raw memory 前置条件仍在 load/store/init/destroy 时检查。

`LifetimeAuthority` 与读写 access 正交。allocation、runtime 唯一提供的 global lifecycle entry、按值参数对象、constructor/result destination 及其受控 subobject projection 可以携带 `owner`，允许 schema 在准确路径开始/提交/销毁生命周期；`place.from_ref`、`place.deref`、普通 `mem.global_place` 和普通 borrow projection 只能产生 `borrow`，即使 access 为 `rw` 也不能据此执行 `obj.init.begin`、`obj.init.commit`、`obj.destroy*` 或 `place.as_uninitialized`。owner authority 不能经普通 copy、raw pointer round-trip 或 FFI 伪造。`place.deref` 的 raw pointer 本身不携带 lifetime generation，但在其 unsafe/raw 前置条件成立的那次执行上，形成的 borrowed place捕获当前 referent `ObjectLifetimeGeneration`；无有效 Alive referent 时该 operation 或其首次 typed use违反 raw memory 前置条件。

global 的完整 lifecycle authority 由 module runtime 按 module-instance/version（thread-local 另含 thread identity）建立，且只作为对应 GlobalRecord 指定 initializer/finalizer function 的 hidden `global_lifecycle` entry place 出现。v0 每个这种 function 恰好服务一个 global，不能同时被另一个 GlobalRecord 引用，也不能兼任 initializer 与 finalizer：initializer 取得 `!place<init,T>` owner，finalizer 取得 Alive `!place<ro/rw,T>` owner。普通函数反复执行 `mem.global_place` 只得到可别名 borrow，不能因此 destroy、reinitialize 或取得第二份 cleanup obligation。

`ro` place 只能读取或建立进一步只读借用。`rw` place 可以读取、赋值和建立可写借用。`init` place 只允许构造操作，不能读取旧值。`InitializationTransactionId` 不是用户可读的运行时字段；它是 verifier/执行器沿 CFG 跟踪的线性事实。fresh init place 在 `obj.init.begin` 前没有 transaction identity，已开始的 root 或 child destination 携带准确 identity，转发调用必须保留它。

生命周期状态变化与可访问 place capability 的物化分离。`obj.init.commit` 或 caller-destination call 的 normal postcondition 先把准确对象状态变为 `Alive`；随后 `place.as_alive` 才能把同地址、同 allocation/subobject path 的 `!place<init,T>` 重绑定为 `!place<ro,T>` 或 `!place<rw,T>`。结果访问权不能超过 allocation/binding 的 mutability，且不再携带 active transaction identity。反向地，`obj.destroy*` 先把对象状态变为 `AllocatedUninitialized`；只有持有拥有销毁与重新初始化权限的 `!place<rw,T>`、没有存活 borrow/child capability 且 verifier 已证明准确状态时，`place.as_uninitialized` 才能把同一 place 重绑定为 fresh `!place<init,T>`。只读 place 永远不能通过该 operation 升级为初始化权限。这两个 operation 都不开始、提交、销毁或 rollback 生命周期，只物化 verifier 已有的路径事实，并在 target lowering 中消除。

每个 place capability 还属于一个 verifier-only `PlaceCapabilityGeneration`。`place.as_alive` 与 `place.as_uninitialized` 具有 `CapabilityRebind` trait：在每条动态路径上消费准确 source generation 并产生该 subobject 的 next generation；source generation、从它投影出的 descendant capability 以及旧 generation 的任何 SSA alias 随后都不可使用。ancestor root 的 rebind 还使构造期借出的 descendant view 失效，root Alive 后必须从新 root capability 重新投影。该线性代际规则不产生运行时内存效果，但 operation 不可复制、不可推测、不可越过相关 commit/destroy/CFG join 移动。

因此在 commit 后直接把原 `!place<init,T>` 交给 `mem.load`、`obj.destroy` 或普通 borrow 仍然非法；必须先取得 `place.as_alive` 结果。失败/unwind 路径没有 Alive fact，不能执行该转换。rollback 只终结 active TransactionId，不改变原 init capability generation；同一 init place 可以按规则直接 fresh `obj.init.begin` 重试，不需要也不能先执行 `place.as_uninitialized`。类似地，`place.as_uninitialized` 不能把仍 Alive、正在 Destroying、只读借用或仅仅 rollback 的对象伪装成新的待初始化 storage。

### 5.3 Place 投影

字段、具体基类、tuple element、array element 和 enum payload 的投影保留父 place 的地址空间、对象版本和访问能力，并按照 TargetContext 使用准确 offset 和 alignment。

普通 enum payload 投影必须具有活动分支证明；构造中的 enum payload 投影必须属于已经选择但尚未提交的分支。不能通过整数 offset 绕过这两个条件。

## 6. Storage 与目标内存

### 6.1 Allocation

抽象 allocation 至少记录：

```text
StorageKind
AddressSpace
BaseAddress
ByteSize
Alignment
ByteInitializationMap
ActiveObjectTree
InitializationTransactionTree
StorageMutability
LifetimeState
VersionAssociation?
```

`mem.alloca` 只分配 storage，不自动产生用户可观察的对象值。global storage 的分配、初始化与析构由 module initializer/finalizer 使用相同对象操作完成。

Core InkIR 不提供无来源的通用 `mem.alloc`/`mem.dealloc` token 对。stack allocation 的 storage 在 activation teardown 时由执行模型释放，但其 Alive 对象必须先按 cleanup plan 显式销毁。heap、GC、arena、Task frame、exception record、module pin 等 storage 只能由具体已注册 runtime owner schema 成对取得和释放；该 schema 持有不可伪造且 allocator kind/version 匹配的 owner token，并声明 `Allocate`/`Deallocate` effect。普通 place、raw pointer 或 `mem.alloca` 结果不能伪造这种 owner token。

ComptimeWorld 使用虚拟目标内存。虚拟地址、offset、对齐、字节序和 pointer wrap 必须使用 TargetContext，不能把宿主地址或宿主 `sizeof` 当作目标结果。

### 6.2 对象状态

对象生命周期状态为：

```text
AllocatedUninitialized
PartiallyInitialized
Alive
Destroying
Deallocated
```

正常状态转换为 `AllocatedUninitialized -> PartiallyInitialized -> Alive -> Destroying -> AllocatedUninitialized`；只有尚无活动对象的 allocation root 才能进一步进入 `Deallocated`。除 allocation-only 的 `Deallocated` 外，这些状态属于 `ActiveObjectTree` 的每个对象节点，而不只是整块 allocation：因此一个 `PartiallyInitialized` parent 可以包含若干已经 `Alive` 的 child。

`InitializationTransactionTree` 是映射到声明对象包含树的动态 identity tree：每条 live parent/child edge 必须对应准确的直接 subobject 关系，rollback 后为同一路径创建的新 identity 则作为新的历史节点。每个 transaction 节点至少记录 `TransactionId`、准确 `SubobjectPath`、可选 `ParentTransactionId`、状态 `Active|Committed|RolledBack`、`InitializedLeafMask`、`CommittedChildMask` 和构造顺序。allocation/root destination 的 transaction 没有 parent；非平凡 base、field、array/tuple element 或 enum payload 的 storage owner 在 parent transaction active 时，对准确 child init place 执行 `obj.init.begin`，从而建立一个 fresh child identity。由 `obj.init` 等不展开的内建操作直接建立的标量/unit leaf 进入 `InitializedLeafMask`，不为每个标量额外创建 transaction identity。

active transaction 必须满足严格嵌套和栈纪律：

- parent 必须保持 active，child path 必须是 parent 的准确、尚未初始化且按语言顺序当前可构造的直接子对象；
- 每个 active parent 同时至多有一个未完成的直接 active child；同一 subobject path 在 active 或 Alive 时不能重复 begin；
- 除合法的 ancestor/descendant 包含关系外，两个 active transaction 的 byte range 不得部分重叠，union、niche 或 base offset 复用不能绕过 `SubobjectPath` 检查；
- 已经 Alive/Committed 的 child、其 descendant 或与 active child 重叠的路径不能再次 begin；
- child commit/rollback 后其 identity 进入终态；若 child rollback，owner 可以在同一未初始化 child path 上以新的 identity 重试，旧 identity 不得复活。

child `obj.init.commit` 只把该 child object 转为 `Alive`，并按构造顺序把它登记进仍 active parent 的 `CommittedChildMask`；它不提交 parent。parent 只能在没有 active child、全部必需 child 已进入 `CommittedChildMask` 或 `InitializedLeafMask`、且自身 constructor body 正常结束时 commit。最外层 root commit 才把完整结果提交为用户可观察的 `Alive` binding。

每次 root 或 child 从非 Alive 状态成功 commit 都为该准确对象节点建立 fresh `ObjectLifetimeGeneration`。所有从该对象形成的 reference、safe slice、interface/DynamicRef view 和 borrowed place 记录这代抽象身份；destroy 终结它，rollback 因对象从未 Alive 而不产生 generation。generation 与 `TransactionId`、`PlaceCapabilityGeneration` 相互独立：transaction 标识一次构造尝试，object generation 标识一次已提交生命周期，place generation 标识当前可用的线性 capability 代际。

初始化失败时，最终失败的 child callee 先逆序清理该 child transaction 内已经提交的 descendant，再由 callee outward-unwind schema postcondition 把 child 标为 `RolledBack` 并使 child path 回到 `AllocatedUninitialized`；parent 仍保持 active，先前提交的 sibling 仍 Alive，因此本地 catch 可以对该 child 创建 fresh transaction 后重试。若 parent 自身继续 outward unwind，它必须逆序销毁自己 `CommittedChildMask` 中的所有 Alive child，再由自己的 outward-unwind postcondition rollback；trivial leaf 只丢弃 lifetime/initialization fact，不调用析构。该回退不是普通可写的生命周期边，也不由名为 `obj.init.abort` 的 opcode 表示。

聚合的 `ActiveObjectTree` 记录已经成功初始化的基类、字段、tuple/array 元素以及 enum 活动载荷；它与 transaction tree 的 `InitializedLeafMask` 和 `CommittedChildMask` 必须一致。只有 `Alive` 对象可以执行普通 load、赋值和完整销毁。构造期间只能访问已经初始化的 child，并受构造期 `this` 不逃逸规则限制。

对象生命周期和 storage 存在期分离。销毁把 storage 恢复为未初始化状态，但不释放 storage；stack storage 由 activation teardown 释放，其他 storage 由持有匹配 owner token 的注册 runtime release schema 释放。

### 6.3 默认初始化

源码：

```ink
var buffer: Buffer;
```

不建立用户可观察的未初始化绑定。概念 lowering 为：

```text
%buffer.storage = mem.alloca : !place<init, Buffer>
obj.init.begin %buffer.storage
perform the selected ordinary default initialization directly into %buffer.storage
the final completing initializer executes exactly one obj.init.commit
%buffer = place.as_alive %buffer.storage : !place<init, Buffer> -> !place<rw, Buffer>
expose %buffer as the binding place
```

默认初始化在 semantic elaboration 中解析为内建默认初始化或选中的零参数构造函数。若不存在合法普通默认初始化，源码声明非法。

规范化 Core InkIR 不保留一个含义模糊的 `default_value<T>` SSA 常量。标量默认初始化降低成准确常量加 `obj.init`，随后由当前 initializer commit；聚合和 class 默认初始化使用字段构造或 `call.* ... to %result-place`。非平凡 child 的 owner 先开始 child transaction，child constructor 作为该 child 的最终完成者 commit；外层 parent 不重复 child commit，只在全部 child 完成后提交自身。只有 root 初始化成功后，绑定才可见并登记清理；失败时只清理 transaction tree 中已经 commit 的 child。

构造初始化列表中未显式列出的基类或字段同样先解析声明初始化器，再解析普通默认初始化，不能先默认构造后执行赋值。

## 7. TargetContext 与布局

### 7.1 TargetKey

任何影响可观察语义或物理布局的目标输入都进入 `TargetKey`，至少包括：

- target triple、ABI、CPU 与 feature set；
- endian 与 address-space data layout；
- pointer width、alignment 和整数表示；
- class、enum、tuple、array、slice 与 interface 布局版本；
- PDB table revision；
- strict float、subnormal 与 `nan_mode`；
- RuntimeABI 和 TargetABI revision。

StagedModule 与 ClosedModule 都携带完整 TargetKey。它们只能在完全相同的 TargetKey 下执行、解释或 lowering；把同一 artifact 换到另一个目标使用是 verifier 错误。import selection 之前若存在无 TargetKey 的受限 profile 状态，它不是 StagedModule。

### 7.2 基础与指针布局

固定宽度整数和浮点值的数值位宽由类型名确定；其 storage alignment、ABI promotion 和合法原生操作由目标确定。`bool` 只有两种语义值，其 storage 大小和 ABI 表示由 TargetABI 记录。

默认地址空间中：

- raw pointer、reference 和 function pointer 使用一个目标机器字；
- `ptrsize` 与对象 raw pointer 具有相同地址位宽；
- slice 使用 `{data pointer, ptrsize length}` 两个机器字；
- interface reference 使用 `{canonical object pointer, interface table pointer}` 两个机器字。

interface table、vtable 和 function pointer 的目标代码地址表示不必与对象数据指针具有同一 LLVM 类型，但必须由 TargetABI 明确记录。

### 7.3 Array 与 tuple

array 直接内联 `N` 个元素，元素 stride 与目标 `sizeof(T)` 一致。布局服务必须检查长度、stride 和总大小乘法能够由目标对象大小模型表示；不能让宿主整数溢出决定布局。

tuple 的逻辑和物理元素顺序均为源码顺序。TargetContext 可以在元素之间和结尾插入 padding，但不得静默重排元素。空元组的可寻址大小和 alignment 由目标布局版本决定。

### 7.4 Class

class 使用名义布局。具体基类子对象先于本类字段参与生命周期顺序；字段物理顺序保持声明顺序，TargetContext 决定 base offset、vptr offset、字段 offset、padding、size 和 alignment。

普通非虚 class 没有隐藏 vptr。虚类具有一个主 vptr，但 vptr 不是普通字段：

- 不出现在普通字段投影中；
- 不由 typed copy 从源对象逐字复制；
- 构造、复制和版本兼容操作必须建立目标完整类型的正确 vptr；
- 计入 size、alignment 和 ABI 布局 hash。

### 7.5 Enum

enum 在语义上始终具有一个活动分支，但物理布局不承诺独立 tag。TargetContext 可以选择：

- 独立 discriminant 加最大 payload storage；
- niche 编码；
- 显式 `[repr]` 已授权的固定编码。

`enum.discriminant` 返回规范的语义分支序号，而不是直接暴露物理 tag bits。普通 payload place 只能在活动分支证明下形成。默认 enum 布局不得直接用于稳定 C ABI。

## 8. 裸指针、引用与切片

### 8.1 扁平数值裸指针

v0 默认对象地址空间采用扁平数值地址模型：

- pointer value 由 address space 与目标位宽地址 bits 组成；
- pointer offset、索引缩放和 pointer/integer 往返按地址位宽取模；
- 形成任意地址本身合法，不开始对象生命周期，也不证明存在 allocation；
- pointer equality 和 ordering 按目标地址空间中的无符号数值地址定义；
- `null` 是地址 bits 全零；
- 只有访问无效地址、错误对象、错误对齐或失效生命周期时才违反 raw memory 前置条件。

element offset 只有在 pointee `SizedObjectType(T)` 时才定义，因为它使用目标 `sizeof(T)`；`ptr<void>`、`ptr<never>`、`ptr<function<...>>`、`ptr<incomplete>` 和 `ptr<runtime.opaque>` 只能执行 byte offset、地址 cast/access 降级和数值比较等不读取 T layout 的操作，不能用于 typed dereference、typed load/store、`sizeof(T)` 或对象生命周期 operation。

因此普通 raw pointer operation 不自动产生 LLVM `inbounds`、`nuw`、`nsw`、`nonnull`、`dereferenceable`、`noalias` 或 `readonly`。只有独立分析证明相应事实时，TargetABI lowering 才可增加这些属性。

v0 的规则不承诺支持 capability pointer 或具有不可由 `ptrsize` 表示之 provenance 的目标。非默认地址空间必须由后续目标扩展给出完整整数往返和比较规则。

### 8.2 Reference

reference 是非空、非拥有、不可重新指向的别名：

- `ref<rw, T>` 允许通过当前路径修改目标；
- `ref<ro, T>` 只允许通过当前路径读取目标；
- reference 可以长期保存，但不延长目标或 module version 生命周期；
- 复制 reference 只复制别名；
- 目标失效后访问属于 UB；
- reference 不能执行 pointer arithmetic，也不能由 `null` 构造。

每个 checked reference 在 verifier/解释器值域中捕获 referent 当前的 `ObjectLifetimeGeneration`。reference copy 保留同一 generation；`place.from_ref`、load/store/borrow projection 和动态类型操作都要求它仍等于当前 Alive object generation。destroy 终结该 generation，同址同型的下一次 commit 建立 fresh generation，因此旧 reference 不会因地址复用而重新有效。generation 不增加 AOT reference 的物理机器字，也不进入用户可观察比较；它由 Closed verifier 证明并由解释器显式跟踪。

`ref<A,T>` 要求 `SizedObjectType(T)`；v0 没有对 void、函数、不完整类型或 runtime opaque object 的 typed reference。需要保存这类地址时使用 raw pointer 或注册的 runtime handle。

reference 不携带借用检查器证明。`ro` reference 不能映射成“存储不会通过其他别名修改”的优化假设。

### 8.3 Slice

safe slice 是非拥有 descriptor：

```text
slice<T> = { data: ptr<T>, length: ptrsize }
```

以上只是物理载荷；IR verifier/解释器还为非空 slice 关联覆盖完整元素范围的 `BorrowGenerationSet`，它由一个或多个当前 `ObjectLifetimeGeneration` 组成，不增加 AOT descriptor 的机器字。

`slice<A,T>` 要求 `SizedObjectType(T)`，因为 index、范围有效性和 length 都以完整 T 元素计量；opaque byte range 应使用 raw pointer 加显式 byte count，而不是 `slice<void>`。

非空 slice 必须从具有 borrow/owner authority 且捕获当前 generation 的首元素 place 与完整连续范围建立，不能直接从只含数值地址的 raw pointer 构造；其 `BorrowGenerationSet` 覆盖范围内全部 Alive element/subobject。规范 null empty slice 使用独立的无来源 empty 初始化形态，length 为零且 data 为 null；从现有 slice 得到的零长度 subslice 可以保留非 null data 与原 borrow generation。slice index 和 subslice 必须进行 bounds check并验证 generation，失败进入 trap；目标生命周期已经结束时属于失效借用 UB。复制 slice 同时保留物理 descriptor 与抽象 generation set。

safe slice 类型携带独立的 `ContainsNoEscapeValue` 性质。它不能进入 global、普通对象字段、Task 完成存储或其他被对应 no-escape verifier 认定为长期存储的位置。具体跨函数和受限 view 边界由 verifier 文档集中规定，不能通过把 slice 降为匿名两字段 tuple 绕过。

safe slice 的 subslice 把 generation set 限制到对应子范围。销毁并在同址重建任一 backing object 后，旧 slice 即使 data/length 数值相同也已失效。interface reference、checked reflection/DynamicRef view 和其他语义上受检查的非拥有 descriptor 同样关联准确 complete object generation；descriptor copy 保留它。raw pointer 与用户 `RawSlice<T>` 不携带 generation，继续遵守扁平数值地址和每次访问时的 raw memory 前置条件。

长期保存地址与长度使用普通用户类型 `RawSlice<T>`，其字段只是 raw pointer 与 `ptrsize`，不获得 safe slice 的检查保证。

## 9. 原始字节、padding 与对象表示

### 9.1 字节初始化图

每个 allocation 独立记录 byte initialization。写入一个标量只初始化该标量的 value bytes；除非 opcode schema 明确说明，写入不会自动初始化相邻 padding。

padding：

- 不属于对象的语义值；
- 默认保持未初始化；
- 不参与 typed equality、typed copy 或 semantic hash；
- 不能因为位于一个 `Alive` 对象内部就被视为可安全读取。

读取任意未初始化字节，包括 padding，违反 raw memory 前置条件。InkIR 不产生一个包含不确定 padding 的普通 SSA value。

### 9.2 Raw load/store

raw load/store 通过裸地址访问目标 storage。调用方必须保证：

- 地址范围属于可访问 allocation；
- alignment 满足要求，或使用显式 unaligned operation；
- 读取字节已经初始化；
- 按 typed value 读取时位模式是该类型的有效表示；
- 对象生命周期、活动 enum 分支和 storage mutability 均允许访问；
- 并发与设备内存前置条件成立。

typed `raw.load/store<T>` 额外要求 `SizedObjectType(T)`，并按准确 `sizeof(T)`/`alignof(T)` 访问。纯 byte-count copy/set 不要求 raw pointer 的 pointee sized，但也不形成 place、reference、Alive object 或任何 T 的有效表示证明。

违反这些条件属于 UB，不自动产生可捕获异常。sanitizer 可以插入附加 trap，但不改变普通语义。

### 9.3 `raw.memcpy` 与 `raw.memmove`

字节复制 operation：

- 只复制准确字节，不执行语言级 typed copy；
- 要求源范围的每个字节已经初始化；
- `raw.memcpy` 额外要求源与目标不发生未授权重叠；
- 不开始或结束 typed object 生命周期；
- 不建立或修复 vptr、enum 活动分支、module version 或其他隐藏 ABI 状态；
- 不证明目标之后具有任何 typed 有效表示。

因此 `Copyable(T)` 不等于“可以对任意 `T` 使用 raw memcpy”。需要按语言值复制时必须使用 `obj.init.copy` 或 `obj.assign.copy`。

## 10. 构造、复制、赋值与销毁

### 10.1 不存在通用 move

InkIR 不提供普通 `move`、移动构造、移动赋值或“已移动变量”状态。优化器可以消除没有独立语义身份的中间存储，但不得把这种初始化消除打印或序列化成可由其他 pass 观察的隐藏 move。

### 10.2 原位构造

address-only 结果和不可复制结果使用调用者提供的最终 place。构造 expression 可以直接初始化局部、字段、函数参数、返回位置和异常 payload。若实现不能保证原位构造，不可复制程序必须被拒绝，不能回退为临时对象加 move。

caller destination 可以是 allocation root，也可以是 active parent 内的准确 child subobject。对应 storage owner 在该对象的首次构造前执行一次 `obj.init.begin`，建立 fresh root/child transaction；接收 destination 的函数可以把同一 place 和 transaction identity 继续转发给另一个 address-only call，不得重复 begin 或提前 commit。最终完成该 destination 对象构造的 callee 执行一次 `obj.init.commit`：root commit 发布完整结果，child commit 只发布 child 并登记进 parent `CommittedChildMask`。

最终失败的 callee 先清理当前 transaction 内已经成功初始化的 descendant，再由 callee outward-unwind schema postcondition 恰好一次 rollback 当前 transaction。若当前对象是 child，只有 child path 回到 `AllocatedUninitialized`，parent 继续 active 且可以 catch 后以 fresh child identity 重试；若当前对象是 root，整个 destination 回到 `AllocatedUninitialized`。`call.invoke` 的 unwind edge 只携带这个已经成立的状态证明，不另外合成 rollback。中间转发层不得重复 begin、commit 或 rollback，storage owner 也不得对未 commit 的失败对象运行完整对象析构。

聚合构造遵守：

- class：具体基类、字段声明顺序、构造函数体；
- tuple/array：元素从左到右；
- enum：只构造所选分支载荷；
- 非平凡 base/field/element/payload：owner 在 parent active 时创建 child transaction；child 正常完成只登记该 child，随后继续 parent；
- child unwind：先 rollback child；parent catch 可以重试该 child，或者继续 outward unwind；
- parent outward unwind：只逆序销毁 parent `CommittedChildMask` 中已经 commit 的 child，再 rollback parent；
- 完整对象未提交：不调用完整对象自身的用户析构函数。

### 10.3 Typed copy

typed copy 只对 `Copyable(T)` 合法，并执行类型结构定义的语义复制：

- scalar：复制准确语义位模式；
- pointer/reference/slice/interface reference：复制非拥有 descriptor；
- array/tuple/class：按组成部分顺序复制；
- enum：读取源活动分支，只复制该分支及载荷；
- virtual class：为目标完整类型建立正确 vptr，随后复制用户状态；
- padding：不复制且保持目标对应字节未初始化。

typed copy 不调用任意用户代码、不分配、不增加引用计数，也不执行可能失败的系统操作。

单标量 typed copy 先产生准确 SSA value，再用 `obj.init` 初始化或 `mem.store` 赋值。`obj.init.copy` 只在当前 active root/child transaction 的准确目标中填充 address-only 新对象，随后仍须由该 transaction 的最终完成者执行 `obj.init.commit`；child copy commit 只登记 child，不能提交 parent。`obj.assign.copy` 替换已经初始化且支持普通赋值的 address-only 目标。初始化与赋值两类 operation 不能互换。

### 10.4 销毁与释放

`obj.destroy` 执行静态已知完整类型的析构链。`obj.destroy_dynamic` 通过动态销毁入口结束最派生对象生命周期。两者都只结束对象生命周期，不释放 storage。

销毁完成后，相应 place 回到未初始化 storage 状态。stack storage 随 activation teardown 释放；其他 storage 只能由持有匹配 owner token 的具体 runtime release schema 释放，并遵守原始分配地址、大小、alignment、allocator 和 module version 契约。Core registry 不提供通用 `mem.dealloc`。

需要在同一 storage 上重新构造对象时，销毁后的路径先用 `place.as_uninitialized` 取得 fresh init capability，再执行新的 `obj.init.begin`。该转换要求没有仍可使用的 live borrow、subobject place 或旧 dynamic dispatch capability；它不会复用已经终结的 TransactionId。

## 11. 浮点位模式与模式

### 11.1 Strict mode

strict 浮点 operation 使用：

- round-to-nearest, ties-to-even；
- gradual underflow 与 subnormal 保留；
- 不可观察的浮点异常 flags；
- 不启用浮点异常 trap；
- 不重结合，不自动 FMA，不忽略有符号零。

`TargetContext.nan_mode` 必须精确定义：

- 多个 NaN operand 中选择哪一个；
- payload 和 sign 的传播；
- sNaN quiet 规则；
- 没有 NaN operand 但 operation 产生 invalid 时的结果 NaN 位模式。

这些规则属于同一 TargetKey 下稳定的 PDB，不能随优化级别改变。ComptimeWorld、Closed interpreter 和 AOT lowering 必须产生同一目标规则允许的准确位级结果。`cast.bit` 始终原样保留已有浮点位模式。

### 11.2 Narrow fast-math

每条浮点 operation 独立携带规范化 flags。源码 `[fast_math]` 在 v0 只授予：

```text
reassociate
contract
no_signed_zero
flush_to_zero
denormals_are_zero
```

它不授予：

```text
approx_reciprocal
approx_function
nnan
ninf
```

`[assume_finite]` 是独立 UB 契约，可以通过 `fp.assume_finite` 建立后续 `nnan/ninf` 事实，但不授予重结合、收缩、忽略有符号零或 FTZ/DAZ。

inline、specialization 和 residualization 必须保留每条 operation 原有 flags，不能按最终所在函数的 attribute 整体重写。

## 12. 阶段边界与 verifier 摘要

Staged Core InkIR 可以引用已经完整类型化的 meta value，但普通 runtime object、raw bytes 和 Closed 常量不得包含 `type`、开放声明句柄、host pointer 或 capability handle。完整规则由 stage 文档规定。

Closed verifier 至少检查：

- 所有类型身份闭合；每个被物化为 storage object 或被查询 layout 的类型都 `LayoutComplete`；
- 每个需要 sizeof/alignment、typed dereference、typed access 或 object lifetime 的 T 都满足 `SizedObjectType(T)`；unsized-pointee pointer 只用于获准的 address/byte operation；
- TargetKey 与全部 layout/PDB/float 结果一致；
- SSA use 满足支配与准确类型一致性；
- address-only 对象没有隐藏 SSA copy；
- place access、alignment 和初始化状态与 opcode 匹配；
- initialization transaction identity 形成与对象包含关系一致的严格树；begin/commit/rollback 保持栈纪律，没有重复 identity、非法重叠或越序 child；
- child commit 只更新 parent `CommittedChildMask`，parent/root commit 前没有 active child 且所有必需 child 已完成；child rollback 后旧 identity 不可复活，retry 使用 fresh identity；
- load 不读取未初始化 storage；
- typed copy 只用于 `Copyable(T)`；
- enum payload 投影具有活动分支证明；
- reference 非空，slice no-escape 性质没有丢失；
- 每个 transaction identity 在每条动态路径上恰好 begin 一次并终结为 commit 或 rollback；每个成功发布的 root/child object lifetime 恰好结束一次，parent outward unwind 逆序销毁全部且仅销毁 committed child；
- padding 没有被 typed operation 当作语义值；
- 没有 host address、`undef`、`poison` 或 stage-only value。

operation 的逐项 schema、效果和阶段合法性见 [`03-core-instructions.md`](./03-core-instructions.md)，动态执行规则见 [`04-execution-model.md`](./04-execution-model.md)。
