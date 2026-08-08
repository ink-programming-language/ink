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

`cf.switch` 对整数或规范 enum discriminant 进行确定性分派。case key 不得重复，default successor 必须存在，除非 verifier 已证明 case 穷尽全部位模式。

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
- unit `()` 是唯一的零载荷 singleton SSA value；
- slice 在执行器内部逻辑上保存 data pointer、`ptrsize` length 与覆盖元素范围的 `BorrowGenerationSet`，并携带 verifier 层面的 no-escape 类型性质；在 Core InkIR 中它和其他多字 aggregate 一样通过 address-only place 传递；
- aggregate 的解释器内部值按语义字段排列，但 Core InkIR 的产生、传递与返回使用 place 和最终存储，不形成普通多结果 SSA 快照；
- runtime opaque value 保存带准确 semantic type 的 runtime handle。

interface reference、checked DynamicRef/reflection view 和其他受检查的非拥有 descriptor 同样关联其 complete object 的 `ObjectLifetimeGeneration`。每次 checked use 都比较当前 Alive object/range generation；destroy 终结旧 generation，同址重建建立 fresh generation，所以旧 borrow 不复活。raw pointer 与 RawSlice 只保存扁平数值地址，不携带该抽象身份。

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
    Mutability,
    LifetimeState,
    VersionAssociation?,
}
```

ComptimeWorld 使用虚拟目标地址。虚拟地址必须遵守目标的指针宽度、对齐、字段 offset 和地址运算，不得直接暴露宿主 C++ 指针。

Core InkIR 不存在可凭任意 place/raw pointer 调用的通用 deallocation。stack allocation 在 activation teardown 时释放其 storage，但此前必须完成所有 Alive 对象的显式 cleanup；heap、GC、arena、Task frame、exception record 和 module pin 等 storage 只由其具体 runtime owner schema 创建和释放，release 必须消费 allocator kind、version 和 owner identity 均匹配的不可伪造 token。

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

解引用、raw load/store 和调用通过 raw pointer 访问对象时，程序负责满足有效范围、活动生命周期、准确表示、对齐和并发前置条件。违反属于 UB。

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

child 构造失败时，最终 callee 先逆序清理该 child 已提交的 descendant，再由 outward-unwind schema postcondition 唯一 rollback 该 child；parent identity、已初始化 trivial leaf 和先前提交的 sibling 保持不变，因此本地 catch 可以在相同 child path 上以 fresh identity 重试。若 parent 随后继续向外 unwind，它再逆序销毁自己的 committed child 并 rollback；已经终结的 child identity 不得复活。`call.invoke` 的 unwind edge只观察这一后置状态，调用 operation 和转发 activation 不重复 rollback。

状态转换不会修改既有 SSA place 的静态 access 类型。commit 或 destination-call normal postcondition 建立 Alive fact 后，执行器只允许 `place.as_alive` 把同地址/同 path 的 init capability 物化为 ro/rw capability；unwind path 没有该事实。destroy 建立 AllocatedUninitialized fact 后，只有准确 rw owner 且没有存活 borrow/child capability 的路径才能用 `place.as_uninitialized` 重新取得 init capability。两者都是零运行时行为的已验证 `CapabilityRebind`：消费当前 `PlaceCapabilityGeneration` 并产生 next generation，旧 generation 及其派生 view 随后失效；ancestor root rebind 还终止构造期 descendant views。它们不改变生命周期，也不复活旧 transaction identity。rollback 仅终结 transaction 并保留原 init generation，所以失败重试直接 fresh begin，不经过 `place.as_uninitialized`。

`var value: T;` 不产生用户可观察的未初始化绑定。lowering 先建立内部 final storage，再执行默认初始化；完整成功后才让绑定可见并登记清理。构造 unwind 只销毁 transaction tree 中已经成功建立的子对象。

析构、defer 和隐式清理按语言规则为 nothrow。若它们内部仍试图传播异常，runtime 执行 fatal path，不进入外层 catch。

## 8. 调用与返回

### 8.1 Activation 建立

调用按源码顺序先计算 callee，再从左到右计算实参，最后执行 call operation。direct、indirect、virtual、interface、reflection 和 async construction 都遵守相同求值顺序。

进入 callee 时：

- scalar/value parameter 绑定为 SSA parameter；
- reference 作为非拥有单标量 SSA 传递；slice、interface reference 和其他 address-only parameter 使用准确参数对象 place；
- sync aggregate/noncopyable result 使用 caller-provided destination，entry `result_destination` 已携带 caller 建立的 active transaction 与 owner authority；
- async body/generated resume entry 由 runtime 绑定 borrowed rw `task_self`；logical result `T` 为 address-only 时还绑定 owner init `task_result_storage`，其 storage 初始为 `AllocatedUninitialized` 且尚无 transaction；
- may-unwind call 保存 unwind continuation；
- hot-reload stable entry 在同一受保护选择协议中原子取得 version-local target 与 pin，然后才允许进入该 target；不得先暴露可卸载裸地址再补 pin。

entry arguments 的顺序和角色不是宿主调用栈约定的偶然结果，而是 Function/Global record 唯一推导的语义 envelope：receiver、ordered parameters、可选 `global_lifecycle`、可选 sync `result_destination`、async `task_self`、可选 async `task_result_storage`。constructor 只有 initializing `receiver`，没有普通 result destination。GlobalRecord 指定的唯一 initializer/finalizer activation 才由 module runtime取得匹配 global 的 owner place；普通 `mem.global_place` 永远是 borrow。`task_self` 的 `LifetimeAuthority = borrow`，不能 destroy/reinitialize Task；`task_result_storage` 只授予结果对象的 lifecycle authority，不授予 frame storage 释放权。Task 隐藏通道都不能作为用户值逃逸，且只有 runtime/schema 允许的 frame spill 能使其跨暂停。

### 8.2 返回

scalar result 由 `cf.return` 返回 SSA value。对于 fresh address-only result storage，storage owner 在首次构造调用前显式执行且只执行一次 root `obj.init.begin`；若 destination 是 active parent 内的非平凡 subobject，则其 owner 对准确 child place 建立 fresh child transaction。将已有 caller destination 原样转发给下一层时，该 destination 已携带同一 active transaction identity，转发层不得重复 begin 或提前 commit。最终完成该 destination 构造的 callee 在每条成功路径完整初始化对象并恰好执行一次 `obj.init.commit` 后再执行无值 `cf.return`；child commit 只登记到 parent，root commit 才发布结果。`call.* ... to %destination` 本身不隐式插入 begin/commit。

destination call 的 normal path 只附带“准确对象已经 Alive”的 verifier state proof，不返回 address-only program value。caller 需要读取、赋值、借用或销毁结果时，在 normal path 对原 destination 执行 `place.as_alive` 得到准确 ro/rw place；该转换在 unwind path 非法。

async body 的源码 `return` 由 lowering 改写为 Task publication。void/unit/scalar 结果直接进入对应 publish variant；address-only 结果在 entry `task_result_storage` 上延迟到实际 return-result 构造前才 `obj.init.begin`，成功后 commit、以 `place.as_alive` 取得 readonly capability并发布。若 body 在 begin 后失败，generated exception boundary 先完成 partial cleanup/rollback；只有 result storage 已回到 `AllocatedUninitialized` 才能 publish failure。`task_self` 保证 publish operand 有规范来源，但始终只是 runtime 提供的 borrow。

如果函数 outward unwind，callee 先清理当前 transaction 内已经提交的 descendant，再由 schema 后置条件 rollback 当前 root 或 child。root destination 回到未初始化状态；child destination 回到未初始化状态而 parent 继续 active，除非 parent 自己也向外 unwind。调用者不得清理一个从未成功建立的完整结果，也不得对已经 rollback 的 transaction 再执行 commit 或 rollback。

`never` 函数没有正常 return。`void` return 不产生 SSA result。返回空元组 `()` 则产生专用 singleton unit SSA value；TargetABI 可以消除其物理传递，但不得把它与 `void` 混同。

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

^cleanup(%exception: !exception):
    obj.destroy %second
    obj.destroy %first
    eh.resume %exception
```

`return`、`break` 和 `continue` 必须经过其离开 scope 所需的正常清理块。条件构造通过 CFG 和初始化状态决定清理集合，不使用隐式全局 cleanup stack。

### 9.3 Trap 与 fatal

trap 表示边界检查失败、显式 trap 或 TargetContext 规定的 PDB trap。fatal 表示未捕获异常边界、nothrow 违约、pending Task 非法销毁等不可恢复运行时失败。

trap 和 fatal：

- 立即终止当前语言执行；
- 不执行 RAII destructor、defer 或 catch；
- 不得被 lowering 成普通可捕获异常；
- 可以调用 runtime 观测钩子，但钩子不能恢复程序执行。

## 10. PDB 与 UB

Ink 的 platform-dependent behavior 由 TargetContext 显式确定。PDB operation 可能产生具体 value 或 trap，但不产生 poison。

典型 PDB 包括目标相关的整数除法边界、shift 边界、float-to-int 转换和其他已由语言议题指定的操作。其规则进入 `PdbTableRevision` 和 TargetKey。

PDB operation：

- 不可在不知道 TargetContext 时 constant-fold；
- 不可假设总能正常返回；
- 不可跨可能改变是否执行的控制边界进行 speculative hoist；
- comptime 与 AOT 必须使用同一目标规则。

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

active module DAG 在普通 staged fixed point 前冻结。module initialization 按 DAG 和规范 module identity 排序，使用普通函数、结果位置、unwind 和清理语义。

每个需要动态 initialization/finalization 的 global 在 v0 具有独占 lifecycle function。module runtime 以当前 module instance/version（thread-local global 另加当前 thread identity）建立唯一 owner capability并将其绑定为 hidden `global_lifecycle` entry place：initializer 从 `!place<init,T>` 完成 begin/构造/commit，finalizer 从 Alive owner place完成 destroy。一次 lifecycle invocation终结或转移该 obligation 后不得复制重入；普通调用不得进入这些 runtime-only functions。其他代码用 `mem.global_place` 只取得 Alive borrow，因此重复查询不会获得 global destroy/reinitialize 权限。

module version 的公开状态是单一事务：先验证并初始化该版本 globals，准备 readonly typed registration records、受控 relocations、reflection metadata 与 stable-entry targets，再以同一个 release/publish同步点原子公开。准备失败时，pending records直接丢弃且不运行用户 rollback hook，已构造 globals按普通RAII逆序清理。撤销版本时先阻止新入口并等待该版本 pin/Task/exception/snapshot等使用者静默，再按不可伪造 version owner原子移除该版本 records，随后析构 globals并释放代码/data；不能只按业务逻辑键删除，以免误删新版本同键记录。

code-only update比较 `ModuleRegistrationInterfaceDigest`，它只覆盖从当前 records 提取并去重排序的 `(RegistrationTypeSemanticIdentity, ProtocolSchemaDigest, TargetLayoutDigest, RegistrationEncodingRevision, RuntimeAbiRevision)` schema tuple，不覆盖 record identity或value集合；v0要求这一 unique schema set 的 digest相等。`ModuleRegistrationSetDigest` 可以变化，并与 stable entries、reflection metadata一起切换，因此在 unique schema set 不变时可以增加、删除或修改具体记录；增加某 schema 的第一条 record 或删除其最后一条会改变 interface digest，不属于 v0 code-only update。看到新 stable target 的线程也必须看到同次发布的新 registration set与完成初始化的 globals。

启用 hot reload 时，新调用通过 stable entry 选择当前 version-local body；已经进入旧 body 的 activation、已创建 Task、ExceptionBox 和 pinned reflection snapshot 继续使用旧版本。v0 只允许 layout/ABI hash 完全相同的代码替换，不迁移普通对象。

raw pointer、reference 和两字 interface reference 不自行 retain module version。动态卸载前由 loader/owner 保证这些非拥有 borrow 已静默；违反该前置条件属于 runtime embedding contract 违约。

## 14. 解释执行与 AOT 等价

对同一个 Verified Closed Module、TargetKey、入口参数和外部 effect trace：

- Closed interpreter 与 AOT executable 必须产生相同语言可观察结果；
- strict 数值、PDB、normal/unwind、trap/fatal 和源码顺序必须一致；
- opaque runtime 实现可以具有不同内部分配、表布局和调度结构；
- fast-math operation 只需落在其明确许可集合内；
- UB 程序不要求等价结果。

差分测试应以 interpreter 作为语义 oracle，但 oracle 本身必须使用 TargetContext，而不是宿主 C++ 算术、指针或浮点环境。
