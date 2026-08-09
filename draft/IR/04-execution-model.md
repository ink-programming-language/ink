# InkIR 执行模型

## 1. 适用范围

本文规定 typed Core InkIR 的抽象执行语义。相同执行核心服务三种 world：

- `ComptimeWorld`：要求得到编译期结果，使用虚拟目标内存和受限 host capability；
- `ResidualizeWorld`：对 Known 部分求值，对未知运行时输入残留 Closed IR；
- `RuntimeWorld`：对 Verified Closed InkIR 执行具体目标语义。

world 改变 value domain、effect handler 和 capability，不改变 `arith.add`、`cf.cond_br`、`mem.load` 等普通 operation 的语言语义。Ink 不存在永久标记为“comptime function”的第二套函数类别。

## 2. 抽象机器状态

执行状态定义为：

```text
MachineState = {
    Module,
    TargetContext,
    World,
    AddressLayoutTrace,
    AddressLayoutTraceCursor,
    CallStack,
    Memory,
    RuntimeState,
    EffectState,
    Limits,
}
```

每个 activation frame 包含：

```text
Activation = {
    FunctionSymbol,
    CurrentBlock,
    OperationCursor,
    SSAEnvironment,
    PlaceEnvironment,
    ResultDestination?,
    GlobalLifecycle?,
    TaskSelf?,
    TaskResultStorage?,
    UnwindContinuation?,
    OriginContext,
    VersionPin?,
}
```

SSAEnvironment 只保存该 activation 中已定义且当前可达的 SSA value。Memory 保存 storage allocation、初始化状态、活动对象和原始字节。RuntimeState 保存 opaque Task、异常、反射、module version 等运行时对象的抽象句柄。

### 2.1 用户执行序列与 runtime 同步

v0 的一个 `MachineState` 恰有一个 `CallStack`，任一时刻至多执行一条用户 Core operation。Task 是协作式的：只有到达已注册的 await/drive/resume operation 才能推进另一个 Task，不存在后台并行执行用户 async body 或未记录的 user scheduler interleaving。不同宿主线程若同时进入 Ink，则各自具有独立 `MachineState`；v0 没有用户可用的 atomic/volatile operation，因此它们对同一普通 user storage 的无同步冲突访问违反 raw memory 并发前置条件，不能借 runtime 内部线程安全推导出用户数据 race 的定义行为。

Task 状态、ExceptionBox 引用计数、cancel、hot-reload publish/pin 和其他明确标为 runtime-managed concurrent 的 operation 具有 schema 指定的单一 linearization point。发布新状态、payload、registration set 或 version target 使用抽象 release；首次观察该发布并取得对应 handle/pin/payload 使用抽象 acquire；成功的 read-modify-write 同时具有 acquire-release。该顺序只覆盖同一 runtime object/schema 明确发布的数据，不把任意相邻 user memory 自动变成 atomic，也不授权 optimizer 跨 acquire/release 移动可能别名的 user memory effect。外部线程事件的线性化顺序进入 RuntimeWorld effect trace，解释器和 AOT 必须观察同一顺序。

`AddressLayoutTrace` 是RuntimeWorld、AOT replay与差分测试可显式提供的有序目标地址分配记录；ComptimeWorld不接受调用者任意trace，而按TargetKey中的`ComptimeVirtualAddressRevision=1`唯一生成。每条记录先编码`AddressEventTag`：1 `operation_allocation`的payload required顺序为`FunctionSymbolKey, RegionIndexPath : Vec<U>, BlockCanonicalOrdinal : U, OperationCanonicalOrdinal : U, RootInvocation : TaggedRootInvocation, DynamicActivationPath : Vec<CallActivationStep>, AllocationOrdinalWithinOperation : U`；2 `static_relocation`的payload required顺序为`ModuleOrdinal : U, SymbolKey : D32, RelocationFieldPath : Vec<U>, RelocationOrdinal : U`。RootInvocation tag 1 `comptime_work`/`CanonicalWorkKey : D32`，tag 2 `runtime_entry`/`EntryFunctionSymbolKey : D32, ExternalInvocationOrdinal : U`；CallActivationStep恰为`SourceBackedCallsiteKey : D32, InvocationOrdinal : U`，只保存root之后的call frames。region/block/operation ordinal来自verified canonical Function/region traversal。Origin、source offset、dense ID、worker、arena、宿主地址与wall clock只供诊断，绝不进入event identity。

revision 1 canonical virtual allocator只支持AddressSpace=0。全部static_relocation event先按ActiveModuleGraph dependency-first ModuleOrdinal、SymbolKey bytes、RelocationFieldPath数值lexicographic、RelocationOrdinal数值序严格递增生成；随后operation_allocation按动态实际执行顺序追加，同一operation多次allocation由从0连续的AllocationOrdinalWithinOperation区分，两类event不能交错。cursor初值为目标可表示的最小非零地址，先按请求alignment向上对齐，再分配`[base, base+extent)`并把cursor移到end；extent必须大于0，alignment必须是TargetContext允许的2次幂，算术使用无溢出目标pointer width，释放不回收或复用地址。相同frozen inputs与Comptime execution path因此产生逐bit相同trace；空间耗尽是受控comptime resource failure。RuntimeWorld仍可使用满足同一tagged event identity、不重叠和目标约束的外部trace，永远不能转成语言可观察raw address的runtime私有allocation可以不进入trace。

## 3. Block 与 operation 的执行

进入 block 时，解释器先将 terminator 传入的 successor arguments 同时绑定到 block arguments，然后从第一条 operation 开始顺序执行。若 value 带 `LifetimeAuthority`、`PlaceCapabilityGeneration`、`ObjectLifetimeGeneration` 或 `BorrowGenerationSet`，选中 edge 的 side facts 与地址/descriptor 作为一个不可拆分的 abstract value 一同绑定；block argument 的 generation phi 不能交叉配对不同 edge。owner 只有在所有 incoming edge 都转移完整 owner obligation 时保留，否则合流 value 至多为 borrow；但 owner incoming 的生命周期/cleanup obligation 必须已经在对应 edge 显式 discharge，或转移给另一条支配后续使用的 owner capability/cleanup state。若仅靠 join 降级就会丢失唯一 obligation，该 join 非法。`CapabilityRebind` 消费实际选中 edge 的 generation。

普通 operation 的执行步骤为：

1. 从 SSAEnvironment 读取全部 operand；
2. 检查动态前置条件和 world capability；
3. 按 opcode 语义计算结果或执行效果；
4. 同时绑定全部 SSA result；
5. 前进到下一条 operation。

terminator 改变 block、返回调用者、进入 unwind continuation 或结束进程级执行。terminator 不产生隐式 fallthrough。

operation operand 已经是值，读取 operand 本身没有二次求值。源码从左到右求值通过 lowering 时生成的 operation 顺序体现；optimizer 不得以 SSA 数据依赖缺失为理由重排可能影响源码顺序的 effect。

## 4. 控制流

### 4.1 无条件与条件分支

`cf.br` 将实参传给唯一 successor。`cf.cond_br` 只接受 `bool`，根据其值选择一个 successor，并只求取所选边对应的 successor arguments；这些 arguments 在进入 terminator 前已经由普通 operation 计算完成。

如果源码要求仅在某分支计算表达式，lowering 必须把计算放入对应 block，不能把它提前到 `cf.cond_br` 之前。

### 4.2 多路分支

`cf.switch` 对整数或规范 enum discriminant 进行确定性分派。case key 不得重复且按规范 key 顺序排列；default successor 无条件必须存在，即使 verifier 已证明 case 穷尽全部位模式，也不能省略该结构边。

enum niche 布局不允许通过任意物理 tag load 实现语义分支。前端使用 `enum.discriminant` 或 `enum.is_variant`，由 TargetContext 选择 tag 或 niche lowering。

### 4.3 循环

普通运行时循环只由 CFG 回边表示。结构化 `comptime for/while` 属于 ElaborationPlan，不使用运行时 CFG 假装已经闭合。

解释器在每条 backedge、call、stage expansion 和 effect operation 上检查 fuel。超过限制是受控编译失败或运行时执行器错误，不得被解释成语言级 `false`、异常或残留代码。

## 5. 值域

### 5.1 Concrete value

RuntimeWorld 和完全求值的 ComptimeWorld 使用 concrete value：

- 定宽整数保存准确 bit width 和 bit pattern；
- `bool` 只有 `false` 与 `true`；
- 浮点保存类型、准确位模式和 TargetContext float mode；
- raw pointer 保存 address space 与目标宽度地址位；
- reference 保存非空 target address、准确 referent type 和 verifier/解释器层的 `ObjectLifetimeGeneration`；copy 保留 generation；
- builtin `unit` 是唯一的零载荷 singleton SSA value；源码类型拼写 `()` 已在进入 Core 前规范化，不形成零元 tuple record；
- slice 在执行器内部逻辑上保存 data pointer、`ptrsize` length 与覆盖元素范围的 `BorrowGenerationSet`，并携带 verifier 层面的 no-escape 类型性质；在 Core InkIR 中它和其他多字 aggregate 一样通过 address-only place 传递；
- aggregate 的解释器内部值按语义字段排列，但 Core InkIR 的产生、传递与返回使用 place 和最终存储，不形成普通多结果 SSA 快照；
- `!runtime-handle<K>` capability value 保存带准确 semantic kind 的线性 runtime handle；Core 不存在 `runtime.opaque` value，`runtime.opaque` 只是不形成 place/SSA value 的类型边界。

interface reference、reflection adapter 的 checked 临时 view 和其他受检查的非拥有 descriptor 同样关联其 complete object 的 `ObjectLifetimeGeneration`。该临时 view 不是 Core type/value；每次 adapter use 都比较当前 Alive object/range generation。destroy 终结旧 generation，同址重建建立 fresh generation，所以旧 borrow 不复活。raw pointer 与 RawSlice 只保存扁平数值地址，不携带该抽象身份。

`runtime.object<kind,args...>` 在执行器中是带 kind、闭合类型参数、`RuntimeStorageAbiHash` 和 opaque private state 的 address-only object。执行器只通过注册 handler 操作其状态，但仍按 TargetContext 提供的有限 size/alignment 把它放入 allocation；`runtime.opaque` 与 runtime handle 不形成这种 typed storage。

### 5.2 Partial value

ResidualizeWorld 可以使用 `Known(value)`、`Residual(value-id)` 和由二者组成的 typed aggregate。partial value 不是 Closed InkIR 类型，不能被存入 Closed object 或序列化为普通常量。

当 operation 的全部 operand Known 且 effect handler 允许执行时，executor 可以计算 Known 结果；否则按照 opcode 的 residualization rule 生成 typed operation。强制 comptime boundary 对 Residual 结果报错。

### 5.3 不存在 undef/poison

InkIR 不提供 `undef`、`poison` 或“尚未决定但可作为任意值使用”的 SSA value。未初始化只存在于 storage 状态，不能被 `mem.load` 成功读取。

优化器如果证明源码违反 UB 前置条件，可以按 UB 规则优化不可达路径，但不得通过在普通合法路径中制造 poison 来传播额外未定义行为。

## 6. 内存与地址

### 6.1 Allocation

抽象 Memory 中每个 allocation 记录：

```text
Allocation = {
    StorageKind,
    AddressSpace,
    BaseAddress,
    ByteSize,
    Alignment,
    ByteInitializationMap,
    ActiveObjectTree,
    InitializationTransactionTree,
    Mutability,
    LifetimeState,
    VersionAssociation?,
}
```

ComptimeWorld 使用上段canonical allocator产生的虚拟目标地址。虚拟地址必须遵守目标的指针宽度、对齐、字段 offset 和地址运算，不得直接暴露宿主 C++ 指针；任何会影响Known result、control selection、generated declaration或cache output的pointer equality/ordering/integer往返都由该唯一trace决定。

Core InkIR 不存在可凭任意 place/raw pointer 调用的通用 deallocation。stack allocation 在 activation teardown 时释放其 storage，但此前必须完成所有 Alive 对象的显式 cleanup；revision 1 的 Task frame与exception record/box只由专用runtime storage schema创建/释放，release必须消费allocator kind、version和owner identity均匹配的不可伪造token，并只在实际storage动作上声明Allocate/Deallocate。version pin是专用线性runtime owner/lease，`pin`/`unpin`/`transfer_pin`只具有中央registry声明的memory与runtime effects，不等同于分配storage。`StorageKind=heap` 仅可见于受信 extern/runtime effect summary；通用 heap、GC、arena owner 与 collector 操作未注册，executor 必须拒绝而非补造实现。

### 6.2 Place 访问

place operation 在 allocation 内建立准确 typed subobject 位置。`mem.load` 要求：

- place 可读；
- 对应完整对象或标量已经开始生命周期；
- 被读取的全部 value bytes 已初始化；
- 对齐满足 operation 声明，或使用显式 unaligned operation；
- union/enum active payload 与读取类型相符；
- 动态对象状态没有被销毁。

place 还携带 owner/borrow `LifetimeAuthority`、`PlaceCapabilityGeneration`，并在指向 Alive object 时关联 `ObjectLifetimeGeneration`。borrow authority 即使可写也不能开始、提交、销毁或重建生命周期；owner authority 只能来自 allocation/global/按值参数/caller destination 及其受控投影。任何 generation 不匹配的 reference/place/slice/interface view use 都属于失效借用，不能仅按相同地址位判断有效。

任何隐式读取 `sizeof(T)`、`alignof(T)` 或 typed object representation 的访问都要求 `SizedObjectType(T)`。`void`、`never`、function、incomplete nominal、`runtime.opaque` 和只表示接口身份的类型可以作为 raw pointer pointee，但不能用于 `place.deref`、element-scaled `ptr.offset`、typed raw load/store 或 object lifecycle operation；这类 pointer 只允许进行显式 byte offset、地址比较、访问权转换和 byte-count raw memory operation。

`mem.store` 只用于已初始化且可赋值的标量 place。建立对象使用 `obj.init.*`；替换已有对象使用 `obj.assign.*` 或先显式销毁再初始化，不能用 store 绕过生命周期。

### 6.3 裸指针

默认地址空间的 raw pointer 地址位宽等于 `ptrsize`。`ptr.offset`、指针索引和 pointer/integer 往返按目标地址位宽取模。形成任意地址本身是合法操作；它不证明地址对应 allocation、对象或对齐。

revision 1 只执行默认 integral address space `AddressSpace=0`，没有注册非默认 address-space extension；Closed verifier 必须在执行前拒绝所有相关非零 type、constant 和 operation，executor 不得回退到默认地址规则。未来语义 revision 只有在完整定义其地址位宽、null、比较、整数往返、cast、layout、memory、lowering 与 feature identity 后才能增加 extension。

解引用、raw load/store 和调用通过 raw pointer 访问对象时，程序负责满足有效范围、活动生命周期、准确表示、对齐和并发前置条件。v0 的并发前置条件要求不存在来自另一 `MachineState` 的未同步冲突访问；由于没有用户级 atomic operation，普通共享可变 user storage 不能通过 runtime acquire/release 自动满足该条件。违反属于 UB。

raw pointer 的数值 equality、ordering、整数往返和对外输出都按当前 `AddressLayoutTrace` 分配的目标地址 bits 执行。RuntimeWorld/AOT在不同合法外部trace下可以观察到不同绝对地址或ordering；这种差异是显式执行输入差异。ComptimeWorld则固定使用TargetKey选择的canonical trace，不能把任意trace作为未记录cache input。optimizer只能使用对所有Runtime合法trace都成立的地址事实，不能假定某次宿主进程的image base、stack base或allocator顺序。

因为采用扁平数值地址语义，普通 raw pointer lowering 不自动附加 LLVM `inbounds`、`nuw`、`nsw`、`nonnull`、`dereferenceable`、`noalias` 或 `readonly`。只有独立证明可以增加这些事实。

### 6.4 Padding 与字节操作

padding 不属于语义值并默认保持未初始化。读取任何未初始化字节，包括 padding，属于 raw memory 前置条件违约。

`raw.memcpy` 和 `raw.memmove`：

- 只复制字节，不执行 typed copy；
- 要求源范围每个字节都已初始化；
- 不开始或结束 typed object 生命周期；
- 不修复 vptr、enum active payload、module version 或其他隐藏 ABI 状态；
- 目标之后按类型访问时，程序仍需证明活动生命周期和有效对象表示。

语言级 copy 按字段、基类和活动 enum payload 执行，不复制 padding。

## 7. 对象生命周期

storage 和 object lifetime 分离：

```text
Absent storage
    -> AllocatedUninitialized
    -> PartiallyInitialized
    -> Alive
    -> Destroying
    -> AllocatedUninitialized
    -> Deallocated
```

只有 `Alive` 对象可以正常读取、赋值和销毁。构造期间只能读取已经初始化的基类或字段，并受构造期 `this` 不逃逸规则约束。

初始化状态按动态 transaction tree 跟踪，而不是只给完整 allocation 一个布尔位。allocation root 的首次初始化建立 root transaction；非平凡 base、field、element 或 enum payload 在 active parent 内建立 fresh child transaction。child 正常完成时只把该 child 变为 `Alive` 并按构造顺序登记进 parent 的 `CommittedChildMask`，parent 仍为 `PartiallyInitialized`；只有 root commit 才发布完整结果或绑定。每个 active parent 同时至多有一个未完成的直接 active child，同一 subobject path 在 active 或 Alive 时不能重复 begin；transaction 的包含关系必须与对象子对象树一致，identity 不能作为 SSA value 伪造、比较或存储。

child 构造失败时，最终 callee 先逆序清理该 child 已提交的 descendant，再由 outward-unwind schema postcondition 唯一 rollback 该 child；parent identity、已初始化 trivial leaf 和先前提交的 sibling 保持不变，因此本地 catch 可以在相同 child path 上以 fresh identity 重试。active aggregate parent 中的 direct trivial leaf 在 `obj.init` 成功时已经拥有自己的 fresh ObjectLifetimeGeneration，所以构造期可以受限借用；parent commit 保留该 generation。若 parent 随后继续向外 unwind，它再逆序销毁自己的 committed child、终结已初始化 trivial leaf generation 并 rollback；已终结的 child/leaf identity 不得复活。`call.invoke` 的 unwind edge只观察这一后置状态，调用 operation 和转发 activation 不重复 rollback。完整 `obj.destroy*` 同样先按析构顺序终结全部 descendant generation，再终结 complete-object generation。

状态转换不会修改既有 SSA place 的静态 access 类型。commit 或 destination-call normal postcondition 建立 Alive fact 后，执行器只允许 `place.as_alive` 把同地址/同 path 的 init capability 物化为 ro/rw capability；unwind path 没有该事实。destroy 建立 AllocatedUninitialized fact 后，只有准确 rw owner 且没有存活 borrow/child capability 的路径才能用 `place.as_uninitialized` 重新取得 init capability。两者都是零运行时行为的已验证 `CapabilityRebind`：消费当前 `PlaceCapabilityGeneration` 并产生 next generation，旧 generation 及其派生 view 随后失效；ancestor root rebind 还终止构造期 descendant views。它们不改变生命周期，也不复活旧 transaction identity。rollback 仅终结 transaction 并保留原 init generation，所以失败重试直接 fresh begin，不经过 `place.as_uninitialized`。

`var value: T;` 不产生用户可观察的未初始化绑定。lowering 先建立内部 final storage，再执行默认初始化；完整成功后才让绑定可见并登记清理。构造 unwind 只销毁 transaction tree 中已经成功建立的子对象。

析构、defer 和隐式清理按语言规则为 nothrow。若它们内部仍试图传播异常，runtime 执行 fatal path，不进入外层 catch。

## 8. 调用与返回

### 8.1 Activation 建立

调用按源码顺序先计算 callee，再从左到右计算实参，最后执行 call operation。direct、indirect、virtual、interface、reflection 和 async construction 都遵守相同求值顺序。

每个逻辑 `object T` 实参在 caller 管理的准确参数最终 storage 中完成独立生命周期后进入 `PreparedParameterObject` 状态；在此状态 caller 暂时持有唯一 owner 与 cleanup obligation，但不得把该对象作为普通 SSA aggregate 暴露。若任一后续实参求值、dispatch/version selection 或 call boundary 建立在 activation 创建前失败，caller cleanup CFG 必须按逆源码顺序销毁已经 prepared 的参数对象。建立 callee activation 与参数 obligation handoff 是同一个原子语义步骤：它消费 caller 的全部 prepared owner，并把对应 Alive owner place 绑定到 callee entry；不存在 caller 和 callee 同时拥有或双方都不拥有的中间状态。

进入 callee 时：

- scalar/value parameter 绑定为准确 scalar 或 builtin `unit` SSA parameter；
- `object T` 绑定为 Alive owner `!place<rw,T>`，并由 callee 在 normal 或 outward-unwind activation exit 时按逆参数顺序恰好清理一次；`const_reference`、`mutable_reference` 和 `raw_pointer` 分别绑定为准确 `ref<ro,T>`、`ref<rw,T>` 和 raw-pointer SSA，不建立参数对象；slice、interface reference 和其他 address-only by-value parameter 使用前述独立 `object` place；
- sync aggregate/noncopyable result 使用 caller-provided destination，entry `result_destination` 已携带 caller 建立的 active transaction 与 owner authority；
- async body/generated resume entry 由 runtime 绑定 borrowed rw `task_self`；logical result `T` 为 address-only 时还绑定 owner init `task_result_storage`，其 storage 初始为 `AllocatedUninitialized` 且尚无 transaction；
- may-unwind call 保存 unwind continuation；
- hot-reload stable entry 在同一受保护选择协议中原子取得 version-local target 与 pin，然后才允许进入该 target；不得先暴露可卸载裸地址再补 pin。

entry arguments 的顺序和角色不是宿主调用栈约定的偶然结果，而是 Function/Global record 唯一推导的语义 envelope：receiver、ordered parameters、可选 `global_lifecycle`、可选 sync `result_destination`、async `task_self`、可选 async `task_result_storage`。constructor 只有 initializing `receiver`，没有普通 result destination。GlobalRecord 指定的唯一 initializer/finalizer activation 才由 module runtime取得匹配 global 的 owner place；普通 `mem.global_place` 永远是 borrow。`task_self` 的 `LifetimeAuthority = borrow`，不能 destroy/reinitialize Task；`task_result_storage` 只授予结果对象的 lifecycle authority，不授予 frame storage 释放权。Task 隐藏通道都不能作为用户值逃逸，且只有 runtime/schema 允许的 frame spill 能使其跨暂停。

call operation 的 normal/unwind successor 只在 handoff 已完成后可达，所以 caller 在这些 successor 上不再拥有参数对象 cleanup obligation；参数清理由 callee activation teardown 完成。相反，发生于 call operation 之前的实参或边界准备失败仍位于 caller cleanup CFG，不能误走 callee unwind edge。verifier 在每条动态路径上要求每个 `PreparedParameterObject` 恰好被 caller failure cleanup 或 activation handoff 消费一次。TargetABI 即使把 copyable 参数拆进寄存器，也必须保留这个语义状态机。

### 8.2 返回

scalar result 由 `cf.return` 返回 SSA value。对于 fresh address-only result storage，storage owner 在首次构造调用前显式执行且只执行一次 root `obj.init.begin`；若 destination 是 active parent 内的非平凡 subobject，则其 owner 对准确 child place 建立 fresh child transaction。将已有 caller destination 原样转发给下一层时，该 destination 已携带同一 active transaction identity，转发层不得重复 begin 或提前 commit。最终完成该 destination 构造的 callee 在每条成功路径完整初始化对象并恰好执行一次 `obj.init.commit` 后再执行无值 `cf.return`；child commit 只登记到 parent，root commit 才发布结果。`call.* ... to %destination` 本身不隐式插入 begin/commit。

destination call 的 normal path 只附带“准确对象已经 Alive”的 verifier state proof，不返回 address-only program value。caller 需要读取、赋值、借用或销毁结果时，在 normal path 对原 destination 执行 `place.as_alive` 得到准确 ro/rw place；该转换在 unwind path 非法。

async body 的源码 `return` 由 lowering 改写为 Task publication。void/unit/scalar 结果直接进入对应 publish variant；address-only 结果在 entry `task_result_storage` 上延迟到实际 return-result 构造前才 `obj.init.begin`，成功后 commit、以 `place.as_alive` 取得 readonly capability并发布。若 body 在 begin 后失败，generated exception boundary 先完成 partial cleanup/rollback；只有 result storage 已回到 `AllocatedUninitialized` 才能 publish failure。`task_self` 保证 publish operand 有规范来源，但始终只是 runtime 提供的 borrow。

如果函数 outward unwind，callee 先清理当前 transaction 内已经提交的 descendant，再由 schema 后置条件 rollback 当前 root 或 child。root destination 回到未初始化状态；child destination 回到未初始化状态而 parent 继续 active，除非 parent 自己也向外 unwind。调用者不得清理一个从未成功建立的完整结果，也不得对已经 rollback 的 transaction 再执行 commit 或 rollback。

`never` 函数没有正常 return。`void` return 不产生 SSA result。返回 builtin `unit` 则产生专用 singleton SSA value；TargetABI 可以消除其物理传递，但不得把它与 `void` 混同。

## 9. 异常、清理与 trap

### 9.1 Exception propagation

可能抛出的 call 使用显式 normal/unwind successor。throw operation 建立或复用异常记录，然后转移到当前 unwind continuation。

catch dispatch 按源码顺序检查 handler。`throw;` 复用当前记录，并跳过当前同组 handler，继续向外传播。catch binding 是只读 view，不复制异常对象。

### 9.2 Cleanup CFG

Closed IR 用显式 CFG 表示正常和异常清理：

```text
call.invoke callee_kind = direct @work(...)
    normal ^success
    unwind ^cleanup(exception)

^cleanup(%incoming: !exception):
    %active = eh.entry %incoming
    obj.destroy %second
    obj.destroy %first
    eh.resume %active
```

`return`、`break` 和 `continue` 必须经过其离开 scope 所需的正常清理块。条件构造通过 CFG 和初始化状态决定清理集合，不使用隐式全局 cleanup stack。

### 9.3 Trap 与 fatal

trap 表示边界检查失败、显式 trap 或 TargetContext 规定的 PDB trap。fatal 表示未捕获异常边界、nothrow 违约、pending Task 非法销毁等不可恢复运行时失败。

trap 和 fatal：

- 立即终止当前语言执行；
- 不执行 RAII destructor、defer 或 catch；
- 不得被 lowering 成普通可捕获异常；
- 可以调用 runtime 观测钩子，但钩子不能恢复程序执行。

## 10. Target-dependent、PDB boundary 与 UB

Ink 的全部 target-dependent behavior 由 TargetContext 显式确定。仅依赖 layout、地址宽度、relocation 或 strict float `nan_mode` 的 operation 标记为 `TargetDependent`；相同 TargetKey 下它们仍可按自身其他 effect 进行 constant-fold、CSE 和 speculative execution，不能仅因依赖目标就一律禁止推测。

真正的 `PdbBoundary` 是目标规则在普通定义域之外选择具体 value 或 trap 的 operation，例如整数除零、signed `MIN/-1`、超位宽 shift 和越界/非数 float-to-int。它同时标记 `TargetDependent`，可能 trap 时还标记 `MayTrap`；其规则进入 `PdbTableRevision` 和 TargetKey。PDB boundary 可能产生具体 value 或 trap，但不产生 poison。

`PdbBoundary` operation：

- 不可在不知道 TargetContext 时 constant-fold；
- 不可假设总能正常返回；
- 不可推测、复制、CSE、删除或跨可能改变是否执行的控制边界移动；
- comptime 与 AOT 必须使用同一目标规则。

因此 strict float arithmetic、layout projection、symbol relocation 等仅 `TargetDependent` operation 可以在相同 TargetKey 下正常推测；上述不可跨控制边界规则不能扩大到它们。是否 target-dependent、是否 PDB boundary、是否可能 trap 是三个分别由 registry 推导的事实，输入文件不能自行声明或互相替代。

UB 是程序违反前置条件，不是一条自动插入的 trap 指令。实现可以提供 sanitizer mode 把部分 UB 检查降低成 trap，但 sanitizer 不改变普通语言语义。

## 11. 整数与浮点

### 11.1 整数

普通定宽整数 add/sub/mul/neg 按结果位宽取模。比较显式区分 signed 与 unsigned。整数 widening、narrowing 和 signedness cast 均按准确位模式和目标类型规则执行。

除法、余数、shift 和 float-to-int 使用专用 operation/schema，不允许用 LLVM 的 poison 语义替代 Ink PDB。

### 11.2 strict 浮点

strict operation 使用 round-to-nearest, ties-to-even，保留 subnormal，浮点异常标志不可观察且不 trap。

`TargetContext.nan_mode` 精确定义：

- 多个 NaN operand 中选择哪一个；
- payload 与符号传播；
- sNaN quiet 规则；
- 无 NaN 输入但产生 invalid 结果时的 NaN 位模式。

comptime software evaluator、Closed interpreter 和 LLVM lowering 必须在同一 TargetKey 下产生该模式允许的准确位级结果。

### 11.3 fast math

每个浮点 operation 独立携带规范 flags。源码 `[fast_math]` 在 v0 展开为：

```text
reassociate
contract
no_signed_zero
flush_to_zero
denormals_are_zero
```

它不授予 `approx_reciprocal`、`approx_function`、`nnan` 或 `ninf`。`[assume_finite]` 独立建立有限值 UB 契约并可产生 `nnan/ninf` 事实。

函数内联后保留每条 operation 原有 flags，不能按新宿主函数的 attribute 整体重写。

## 12. 动态对象、反射与异步

虚调用、接口调用、反射 adapter、Task drive 和 hot-reload version selection 使用相应 semantic operation。核心 executor 不读取其私有字节布局，而是调用 RuntimeWorld handler。

handler 必须满足 operation schema 的事务边界、正常/unwind 结果、pin 生命周期和 nothrow 要求。相同 handler 接口也允许 ComptimeWorld 提供语义等价实现；没有 comptime handler 时，对应 effect 只能 residualize，若处于强制 comptime boundary 则报错。

async function call 只同步构造惰性 Task，不执行 async body。Task 以后由 await/drive operation 推进。`[nothrow] async` 只禁止 Task 进入 failed，不把同步构造入口标记为 nounwind。

`ct.register_module_item` 只由 ComptimeWorld执行。到达时它读取一个完整 Known scalar/unit或address-only逻辑快照，按 central frozen-encoding schema形成 typed Constant，并向当前 fixed-point pending batch追加一条 ordered semantic record；未到达、未选模板和失败 batch都不产生可见记录。它不在 RuntimeWorld执行，也不调用用户安装/撤销代码。

## 13. Module 初始化与版本

active module DAG 在普通 staged fixed point 前冻结。跨 module initialization 先按 dependency-first DAG 顺序执行，ready module 以规范 module identity 打破平局。每个 Closed module 的 semantic Global records 必须能够唯一重建一个进入语义投影与 `SemanticModuleDigest` 的 `ModuleInitializationPlan`；canonical artifact 只在每个 GlobalRecord 编码 policy、ordinal 和 dependencies，不注册第二个 plan record。实现可以在验证后物化等价的内部执行表，但不能序列化另一套权威顺序：

```text
ModuleInitializationPlan = {
    ProcessSteps: Vec<GlobalLifecycleStep>,
    ThreadSteps: Vec<GlobalLifecycleStep>,
}

GlobalLifecycleStep = {
    GlobalSymbolKey,
    StorageKind: process | thread_local,
    Ordinal,
    Dependencies: Vec<GlobalSymbolKey>,
    InitializerFunction?,
    FinalizerFunction?,
}
```

每个需要dynamic initialization或finalization的static definition在ProcessSteps中恰好出现一次；每个thread_local definition无论使用constant image还是dynamic initializer、无论是否有finalizer，都在ThreadSteps中恰好出现一次。step引用的initializer/finalizer必须就是该GlobalRecord声明的独占lifecycle function。Dependencies只引用同一module、同一process/thread policy分区中ordinal更小的step，不能自环或成环；跨module顺序只由已经冻结的active module DAG表达，不重复写入Global dependency vector。`ProcessSteps`与`ThreadSteps`的ordinal分别从零连续且等于list index；两者都是dependency-first的stable Kahn顺序，每次从ready set选择最小`GlobalSymbolKey`。从序列化Global records重建的plan，以及实现验证后物化的内部执行表，都必须与重算结果逐项相同；实现不能使用declaration insertion order、linker order或hash-map iteration order代替。静态constant image且无finalizer的global在publish前安装，不需要process step；image-only TLS仍由每线程step物化storage、安装image、建立Alive并加入AlivePrefix。若static只有image initializer和finalizer，则image installation先建立Alive object，runtime在对应ordinal验证并登记cleanup obligation，不调用不存在的initializer function。依赖module已由外层DAG完整初始化，当前module的static global依赖还必须在使用它的dynamic step之前可用。

module runtime 以当前 module instance/version（thread-local global 另加当前 thread identity）为每个 step 建立唯一 owner capability并将其绑定为 hidden `global_lifecycle` entry place：initializer 从 `!place<init,T>` 完成 begin、构造与 commit，finalizer 从 Alive owner place完成 destroy。一次 lifecycle invocation 终结或转移该 obligation 后不得复制重入；普通调用不得进入这些 runtime-only functions。其他代码用 `mem.global_place` 只取得 Alive borrow，因此重复查询不会获得 global destroy/reinitialize 权限。

process-global 状态机为 `Uninitialized -> Initializing(AlivePrefix) -> Alive -> Finalizing -> Destroyed`，runtime/embedding安装失败另进入终态 `Failed`。runtime 按 `ProcessSteps` 推进；每个 step 只有在 initializer normal completion且目标已经 Alive 后才加入 `AlivePrefix`。revision 1所有process/TLS initializer与finalizer均为sync/void/nothrow，故lifecycle entry没有Ink unwind successor或exception-token ownership问题；违反contract进入`rt.fatal(nothrow_violation)`。runtime/embedding级失败时，当前未完成transaction由runtime按已验证plan rollback，随后逆序finalization/destroy `AlivePrefix`，丢弃pending registration/relocation/metadata，版本进入`Failed`且不得发布；该失败不是可捕获Ink exception。撤销Alive version时只按实际Alive step的逆plan顺序finalization；从未开始、已rollback或已经销毁的global不得调用finalizer。

v0 的 TLS policy 规范枚举名为 `eager_thread_activation`：线程进入或切换 module version 时由 runtime 显式急切执行 `ThreadSteps`，绝不由 `mem.global_place` 隐式 first-use initialization。每个 `(module instance, version, thread identity)` 具有状态 `Uninitialized -> Initializing(AlivePrefix) -> Alive -> Finalizing -> Destroyed`，初始化或 embedding 失败另进入终态 `Failed`。runtime 必须在该线程执行该版本第一条普通用户 operation 前完成 activation；code-only publish 后，stable entry 在允许线程切换并进入新 version-local body 前同样必须完成新版本 activation。`mem.global_place` 查询 TLS global 时要求状态为 `Alive`，或当前正处于该 plan 的 initializer activation且目标已在 `AlivePrefix`；它本身永不执行 initializer。

为使普通 call schema 不因隐藏 TLS 初始化突然增加 unwind edge，v0 要求 thread-local initializer 和 finalizer 均为 nothrow。TLS initializer 内再次请求同一 `(module, version, thread)` activation、访问尚未进入 `AlivePrefix` 的同组 TLS global，或在 `Finalizing/Destroyed/Failed` 状态取得 place，都是 `invalid_dynamic_state` fatal；访问已经 Alive 且满足显式 dependency 的 earlier TLS global合法。runtime/embedding failure 发生时先逆序清理 `AlivePrefix` 再进入 `Failed`，随后拒绝该线程进入版本。线程退出或版本从该线程 detach 时按 `ThreadSteps` 逆序 finalization，且只处理实际 Alive prefix；每个 TLS object 恰好销毁一次。

module version 的公开状态是单一事务：先验证 plan 并初始化该版本 process globals，准备 readonly typed registration records、受控 relocations、reflection metadata 与 stable-entry targets，再以同一个 release/publish同步点原子公开。取得新 stable target/version pin 与线程 version activation 的成功观察使用 acquire，因此看到新 target 的执行也看到已完成的 process globals 与同次 registration set；TLS object 的 publication 只对匹配 thread identity生效。撤销版本时先阻止新入口并等待该版本 pin/Task/exception/snapshot等使用者静默，再按不可伪造 version owner原子移除该版本 records，完成各线程 TLS detach 和 process-global 逆序 finalization，随后释放代码/data；不能只按业务逻辑键删除，以免误删新版本同键记录。

code-only update比较 `ModuleRegistrationInterfaceDigest`，它只覆盖从当前 records 提取并去重排序的 `(RegistrationTypeSemanticIdentity, ProtocolSchemaDigest, TargetLayoutDigest, RegistrationEncodingRevision, RuntimeAbiRevision)` schema tuple，不覆盖 record identity或value集合；v0要求这一 unique schema set 的 digest相等。`ModuleRegistrationSetDigest` 可以变化，并与 stable entries、reflection metadata一起切换，因此在 unique schema set 不变时可以增加、删除或修改具体记录；增加某 schema 的第一条 record 或删除其最后一条会改变 interface digest，不属于 v0 code-only update。看到新 stable target 的线程也必须看到同次发布的新 registration set与完成初始化的 globals。

启用 hot reload 时，新调用通过 stable entry 选择当前 version-local body；已经进入旧 body 的 activation、已创建 Task、ExceptionBox 和 pinned reflection snapshot 继续使用旧版本。v0 只允许 layout/ABI hash 完全相同的代码替换，不迁移普通对象。

raw pointer、reference 和两字 interface reference 不自行 retain module version。动态卸载前由 loader/owner 保证这些非拥有 borrow 已静默；违反该前置条件属于 runtime embedding contract 违约。

## 14. 解释执行与 AOT 等价

对同一个 Verified Closed Module、TargetKey、入口参数、外部 effect/linearization trace 和 `AddressLayoutTrace`，且两侧 Limits 都未耗尽：

- Closed interpreter 与 AOT executable 必须产生相同语言可观察结果；
- strict 数值、target-dependent/PDB boundary、normal/unwind、trap/fatal、runtime acquire-release 观察顺序和源码顺序必须一致；
- raw pointer equality、ordering、整数往返和输出必须使用同一 AddressLayoutTrace；AOT concrete execution 可以先记录其合法目标地址分配，interpreter 再以该 trace replay，或测试 harness 为两者提供同一受约束 trace；
- opaque runtime 实现可以具有不同内部分配、表布局和调度结构，但只有从未通过语言级 raw address、pointer integer、ordering 或 effect 暴露的私有差异可以忽略；一旦可观察就必须进入 AddressLayoutTrace 或外部 effect trace；
- fast-math operation 只需落在其明确许可集合内；
- UB 程序不要求等价结果。

Limits/fuel 耗尽是受控工具或 embedding 资源失败，不是语言 completion；只在一侧耗尽不能据此宣称语言语义不等价，也不能把耗尽伪装成 `false`、Ink exception 或 trap。差分测试应以 interpreter 作为语义 oracle，但 oracle 本身必须使用 TargetContext、AddressLayoutTrace 与同一 runtime linearization trace，而不是宿主 C++ 算术、即时进程地址、指针或浮点环境。
