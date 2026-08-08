# InkIR 到 LLVM IR 的 Lowering 与首个纵切片

## 1. Lowering 边界

LLVM backend 只接收 `VerifiedClosedModule<TargetKey>`。在此边界之前必须已经完成：

- import selection、active module DAG freeze；
- generic instance、parameter pack 和 structured comptime 展开；
- 所有名称、重载和布局解析；
- 强制 comptime 求值；
- cleanup CFG、normal/unwind 和对象生命周期显式化；
- runtime-representability 和 Closed verifier。

LLVM 不需要理解 deferred template、开放泛型、meta type、declaration sink、Known/Residual 或 fixed-point transaction。

v0 不要求序列化独立的 `CodegenInkIR`。TargetABI lowering 可以在 backend 内部建立临时 lowering plan，再直接构造 LLVM module。若以后需要多个低级 backend、ABI 级优化或低级 IR 解释器，可以增加非稳定的 `LoweredInkIR[target]`，但它不得追溯改变 Closed InkIR 语义。

## 2. Lowering 上下文

backend 上下文至少包含：

```text
LoweringContext = {
    VerifiedClosedModule,
    TargetContext,
    TargetABI,
    LLVMContext,
    TypeMap,
    SymbolMap,
    RuntimeIntrinsicMap,
    ExceptionABI,
    RelocationModel,
    CodeModel,
    OptimizationPolicy,
}
```

TargetABI 负责参数分类、寄存器/栈传递、C ABI、平台异常 ABI 和 LLVM attribute。OptimizationPolicy 不得改变 Ink source semantics；`-O0` 与 `-O3` 只能改变实现质量。

## 3. 类型映射

### 3.1 Primitive

| InkIR type | LLVM 表示 |
| --- | --- |
| `iN` / `uN` | `iN`；signedness 保留在 operation，不编码进 LLVM integer type |
| `bool` | 计算通常使用 `i1`；存储和 ABI 使用 TargetContext 的 bool layout |
| `ptrsize` | 默认地址空间准确指针宽度的 integer；Ink 不存在独立 `iptrsize` |
| `f16` / `f32` / `f64` | 对应 LLVM floating type，前提是 target lowering 能满足 Ink strict/fast mode |
| `void` | 无 SSA result；不是空元组 |
| `never` | 无正常返回路径，函数可标记 `noreturn` |
| `()` | 专用 singleton unit SSA value；ABI 可以消除，但不能与 `void` 混淆 |

`int`、`uint` 和 `byte` 在进入 Closed IR 前已经规范化为 `i64`、`u64` 和 `u8`。

### 3.2 Pointer、reference 与 slice

raw pointer 和 reference 降低为对应 LLVM address space 的 pointer。默认 raw pointer 是 integral pointer；如果 target 声明为 non-integral/capability pointer，v0 TargetContext 必须拒绝该 target，而不是静默改变扁平数值地址语义。

reference 的语言前置条件允许在准确边界添加 `nonnull`，但 reference、`const` target 和 slice 都不自动产生 `noalias`。`const T&` 也不能仅凭类型获得跨调用的 LLVM `readonly`，除非完整 effect analysis 证明 callee 不通过其他 alias 修改对象。

`ObjectLifetimeGeneration`、slice `BorrowGenerationSet` 和 interface/DynamicRef 的 checked borrow generation 是 Closed verifier 与解释器语义，不增加 release AOT pointer/descriptor 的物理机器字。lowering 只有在已经证明每个 use 仍属于同一 Alive generation 后才可擦除这些 facts；相同 LLVM address、type 或重用的 alloca 不得让 destroy 前的 reference/view 在重建后复活。sanitizer 可以选择物化 generation cookie，但普通语义不依赖该检查。

slice 是 Core InkIR 中的 address-only 值，其逻辑存储表示为：

```text
{ T addrspace(A)* data, ptrsize length }
```

TargetABI 可以在不引入额外语言级 copy、析构或地址变化时把两个字段拆到寄存器，但这不让 slice 变成 Core InkIR SSA 值；安全 slice 的 no-escape 性质必须在 ABI 边界和 optimizer 中保持。

### 3.3 Aggregate、class 与 enum

array、tuple 和 class 按 TargetContext 已确定的字段顺序、offset、alignment 和 tail padding 构造 LLVM storage type。tuple element 不得为了优化重排。

virtual class 的 vptr、concrete base adjustment 和 interface table 属于 private TargetABI。enum 根据确定的 tag 或 niche layout 构造 storage type；semantic `enum.*` operation 不能在前端被替换成未经 layout service 的固定 tag 访问。

`runtime.object<kind,args...>`（包括 `Task<T>`）按 TargetContext/RuntimeAbiRevision 提供的有限 opaque storage size、alignment 与 `RuntimeStorageAbiHash` 形成 address-only LLVM storage shell；其内部可以是固定大小 byte array、private target type 或受控 indirect handle，但只能由匹配 runtime intrinsic 访问。backend 不生成字段 GEP、普通 aggregate constant、typed raw load/store、memcpy copy 或自行解释私有状态。`runtime.opaque`、exception/runtime handle、reflection snapshot owner 和 runtime lease 则只使用不完整 pointer/handle，不能形成 Core typed object storage。Closed InkIR binary 不嵌入 runtime 私有 C++ struct layout。

## 4. 常量与全局

APInt 直接按 bit width 和 bit pattern 构造 LLVM ConstantInt。浮点常量按准确 bits 构造 ConstantFP，不通过 host decimal conversion。

aggregate constant 按逻辑 element 和 TargetContext layout 构造。padding 不属于 semantic constant；backend 可以为只读目标文件表示选择具体 padding bits，但不得让合法程序读取这些默认未初始化 padding，也不得把 padding 纳入 typed equality/copy。

relocatable pointer constant 通过规范 symbol/offset 构造。host pointer、compiler object address 和 comptime virtual address在 Closed verifier 已被拒绝。

module globals 按 active DAG 和 module initialization plan 生成初始化/销毁入口。每个 v0 lifecycle function 的 hidden `global_lifecycle` Core argument 由 runtime-private module entry ABI 物化为准确 global address加已验证的唯一 owner side state；initializer/finalizer分别只服务一个 GlobalRecord。普通 `mem.global_place` 即使降低为同一 LLVM global pointer也仍是 borrow，LLVM pointer 等同不能授予 destroy/reinitialize authority。热更新开启时，version-local globals、publish transaction 和排空销毁通过 runtime intrinsics 管理；module/thread generation在擦除 lifecycle capability前必须完成验证。

Verified Closed `ModuleRegistrationTable` lowering 为 private readonly typed data、中央 frozen-encoding backing、受控 symbol relocations 与 loader descriptor。built-in frozen String 使用 module-owned immutable bytes/length表示；nominal record按TargetContext layout初始化逻辑字段而不把padding变成可观察值。函数 relocation只到stable entry，其他获准static metadata/data地址由当前module-version owner覆盖。backend不生成registration constructor、destructor、rollback、ELF/C++全局init hook或任意用户install/remove callback，也不对frozen record调用普通T的析构。

静态AOT可把相同registration schema的records合并成readonly section和typed index，但必须保留canonical identity/order和按module version的可见性语义。动态loader在globals成功初始化后准备descriptor，并把stable entries、reflection metadata和registration set原子publish；retire先按version owner+RegistrationIdentity移除records，再析构globals。`ct.register_module_item`在Closed verifier前已消失，LLVM backend看到它一律视为未验证输入。

## 5. 整数 operation

### 5.1 Wrapping arithmetic

Ink 定宽整数普通 add/sub/mul/neg 取模，因此 lowering 使用没有 `nsw`/`nuw` 的 LLVM integer operation。只有独立范围证明 pass 可以给具体 LLVM instruction 添加 nowrap flag。

signed/unsigned comparison 分别映射对应 `icmp` predicate。整数 cast 按 InkIR 的 truncate、zero-extend、sign-extend、bitcast 和 modulo 规则显式选择 LLVM operation。

### 5.2 PDB arithmetic

LLVM 的 `sdiv`、`udiv`、`srem`、`urem`、shift 和 FP-to-int 在部分输入上可能产生 poison 或具有比 Ink 更强的前置条件。backend 必须查 `PdbTable` 和目标宽度的 lowering recipe：

- 只有已经证明输入位于普通 LLVM operation 的定义域时，才直接发出对应裸 instruction；
- 其他情况直接发出实现该 TargetKey 原生 PDB 的目标 instruction sequence，v0 基线使用带 `sideeffect` 和准确寄存器/clobber 约束的 inline asm；
- 非原生宽度或没有单条目标指令的 operation 可以调用目标专用 helper，但 helper 必须准确实现同一 `PdbTable` recipe，并作为不可推测、不可删除的 PDB boundary；
- 不得插入跨平台范围检查、条件分支、guard/select 或结果修正来统一目标差异，也不得让 LLVM poison 代替 Ink PDB value/trap；
- 目标若没有可验证的原生 recipe 或准确 helper，TargetContext capability 检查必须拒绝该 operation/target 组合。

PDB lowering boundary 保持原 operation 的 `MayTrap`、顺序与不可推测性质，不能被移到该 operation 本来不会执行的路径。

v0 的 LLVM API 实现细节与目标指令约束见 [`../implementation/llvm-pdb-lowering.md`](../implementation/llvm-pdb-lowering.md)；该实现说明不能反向放宽本节的语义边界。

## 6. 裸指针 lowering

raw pointer 算术采用目标地址宽度取模。最直接的语义 lowering 为：

```text
ptrtoint
wrapping integer add/mul
inttoptr
```

backend 可以使用等价的 non-inbounds GEP，但必须证明其地址位结果和 integral-pointer 语义一致。普通 raw pointer arithmetic 不添加 `inbounds`。

field/base/tuple place lowering 可以使用结构化 GEP；只有 allocation/lifetime/range 证明完整时才添加 `inbounds`。pointer comparison 按无符号数值地址，因此在需要时先转换为准确宽度整数，而不是使用具有不同 provenance 假设的高级比较。

raw load/store 根据 operation 显式 alignment 生成。unaligned operation 使用目标支持的低 alignment 或 runtime helper。无效地址、生命周期或对象表示属于语言 UB；sanitizer build 可以插入额外检查。

任何普通 raw/byte-count operation 的 abstract range 都必须在 lowering 前证明不与 active/partial `SealedRuntimeStorage` 重叠。把 pointer cast 成 `i8*`、`void*`、整数地址或 unknown pointee 不会清除 sealed range；只有匹配 runtime-kind intrinsic 能取得其私有地址。无法证明不重叠时拒绝 lowering，不能先发出 LLVM memcpy/memset 再依赖 opaque type 阻止访问。

element-scaled GEP、typed load/store、`place.deref` 和任何需要 object layout 的 lowering 只接受 `SizedObjectType` pointee。unsized-pointee pointer 保持为整数宽度地址或 opaque backend pointer，只能进行显式 byte offset、地址比较、访问权转换和 byte-count raw memory lowering；backend 不得为它猜测 element size、alignment 或 dereferenceable 范围。

## 7. 浮点 lowering

### 7.1 Strict operation

目标必须保证 round-to-nearest, ties-to-even、strict subnormal 和关闭浮点 trap。异常 flags 不可观察。

当普通 LLVM FP instruction 和 constant folder 精确满足 `TargetContext.nan_mode` 时，可以直接使用。不能保证 payload、符号、sNaN quiet 或 invalid-result NaN bits 时，backend 必须选择 constrained intrinsic、target-specific sequence 或 runtime helper，并阻止会改变规定 bit pattern 的 fold。

cross compilation 不使用宿主 C++ 浮点计算 strict constant；constant folding 调用 target-aware software evaluator。

### 7.2 Fast-math flags

InkIR flags 到 LLVM 的映射必须逐项完成：

| InkIR permission | LLVM/target lowering |
| --- | --- |
| `reassociate` | `reassoc` |
| `contract` | `contract` 或明确 FMA lowering |
| `no_signed_zero` | `nsz` |
| `flush_to_zero` | target denormal mode、helper 或受控 outlining |
| `denormals_are_zero` | target denormal input mode、helper 或受控 outlining |
| `[assume_finite]` proof | `nnan`、`ninf` |

单独 `[fast_math]` 不得映射为 LLVM `fast`，也不得添加 `arcp`、`afn`、`nnan` 或 `ninf`。

FTZ/DAZ 在部分 LLVM target 上是 function-level mode，而 InkIR 语义保存在单条 operation。若内联后一个 function 同时包含 strict 和 fast operation，backend 必须选择能保持两者的方案，例如阻止不兼容内联、把一类 operation outline 到 helper、使用 target-specific per-instruction control或软件 lowering；不能用一个 function attribute 把 strict operation 一并改成 FTZ/DAZ。

## 8. Place、生命周期与 copy

`place<T>` 降低为准确 address space pointer 加 backend side information。`mem.alloca` 通常映射为 entry/allocation-scope 中的 LLVM `alloca`，但只在不改变动态 lifetime 或异常清理时提升。

`LifetimeAuthority` 也是必须先验证再擦除的 side information。来自 reference/raw dereference 的 rw borrow 可以降低为可写 pointer，却不能因此调用 destructor、开始新 lifetime 或获得 deallocation/reinitialization 权限；只有 owner path 可降低这些 operation。LLVM pointer type 不区分二者，不能被反向用作 authority 证明。

`TransactionId` 通常只作为 Closed verifier 和 lowering plan 的证明状态，不必物化为运行时值。path-sensitive lowering 把每个 active transaction 的 `InitializedLeafMask`、`CommittedChildMask` 和 cleanup plan 表示为已知 CFG 路径、必要时的 frame/local flag，或二者组合；不能通过删除 identity 证明而丢失失败重试、join 或 unwind 顺序。

`place.as_alive` 与 `place.as_uninitialized` 保持完全相同的 backend address、address space 和 alignment，只改变已经由 Closed verifier 证明的 capability view，通常不生成 LLVM instruction。lowering plan 必须先消费 source `PlaceCapabilityGeneration`、建立 next generation 并使旧 generation 及其派生 view 失效，才能擦除 `CapabilityRebind` marker；不能因为两个 generation 降为相同 LLVM pointer 就让旧 SSA use 越代存活。若 debug/sanitizer build 选择保留状态检查，它们也只能验证既有 postcondition，不能改变普通 build 的 lifetime、异常或 trap 语义。任何 LLVM load/store/destroy 必须来自正确的 live capability；backend 不能仅因底层 pointer 相同而绕过转换。

`obj.init.begin`、`obj.init.commit` 和 `obj.destroy` 在完成各自真实初始化、提交和销毁语义后，可以附带生成 LLVM lifetime intrinsic；Ink 对象生命周期不能只依赖这些可被忽略的 hint。child commit 的 lifetime start 仅覆盖准确 child object并更新 parent cleanup state，不能开始或提交 parent lifetime；只有 root commit 才发布完整 result/binding。constructor、destructor、active enum payload 和 dynamic object state 仍由实际 control flow 与 calls 表示。

child outward unwind 必须先清理 child transaction 内已经提交的 descendant，再 rollback child；如果 parent 继续 outward unwind，则按逆构造顺序逐层清理 parent 的 `CommittedChildMask` 并 rollback。若本地 catch 重试同一 child path，lowering 必须建立 fresh logical identity，并保证旧 child state不能被后续 path 复用。EH lowering 可以把静态已知 mask 化为 CFG，也可以使用显式 cleanup flag，但 normal/unwind successor 的状态必须与 Closed InkIR 后置条件相同。

scalar load/store 映射普通 LLVM memory operation。typed copy：

- trivial scalar/field 可以分别 load/store；
- class copy 必须重新建立目标 vptr；
- enum copy 只复制 active payload并构造目标 discriminant/niche；
- noncopyable type 没有 copy lowering；
- optimizer 只有在证明类型的全部语义、lifetime、padding 和 hidden state 均允许时才可把 typed copy 合并成 memcpy。

`raw.memcpy/memmove` 映射 LLVM intrinsic，但前端前置条件仍要求源范围全部初始化，且该 operation 不建立 typed lifetime。

stack storage 由 activation teardown 释放；其他 storage 只由具体 runtime owner release intrinsic 消费匹配的 allocator/version/owner token 后释放。backend 不生成接受任意 pointer 或 place 的通用 `mem.dealloc` helper。

## 9. 函数与调用约定

### 9.1 Ink internal logical ABI

scalar result 可以作为 LLVM direct result。address-only、noncopyable 或需要原位构造的结果添加 caller-provided destination。该 destination 在 Ink 逻辑 ABI 中是 result channel，不是普通用户参数。

TargetABI 可以把它映射为 LLVM `sret`、普通 private pointer 或 target-specific indirect result。只有满足 LLVM attribute 契约时才添加 `sret`、`noalias`、`dereferenceable` 或 alignment attribute。

lowering 前，Core function entry 的 semantic block arguments 已按固定 envelope 排列：receiver、logical parameters、可选 `global_lifecycle`、可选 sync `result_destination`、async `task_self`、可选 async `task_result_storage`。不存在需要另行编号的 parameter value。sync result destination 已带 active transaction side state；LLVM 隐藏 pointer 只是其物理地址，不能被后端误当成尚未 begin 的普通输出缓冲区。constructor 的 initializing receiver可以复用物理 result-storage 位置，但 Core 中仍是 receiver role且没有第二个 result argument。

async body 使用 runtime-private resume ABI。`task_self` 降低为 frame/task context 中当前 Alive Task 的 borrowed capability，`task_result_storage` 降低为 RuntimeABI/TargetContext 给出的 private result slot地址；它们可以来自固定寄存器、frame offset或经过验证的 rematerialization。backend 不得为此暴露 Task struct layout、生成普通 `place.field`/public GEP，或赋予 `task_self` owner/destroy authority。address-only result slot entry 时仍是 AllocatedUninitialized；实际 return-result 构造点的 begin/commit/rollback side state必须在擦除前验证，并在跨 suspend 时编码进 frame cleanup/state machine。成功 publication使用已 commit 的只读 slot，failure publication要求 slot无active transaction且为未初始化状态。

destination call 的正常路径保证 destination 已完整初始化；unwind 路径保证它未成为完整对象。

### 9.2 Direct/indirect/virtual/interface

direct call 映射已解析 LLVM function。indirect call 使用准确闭合 function type，不能把不匹配签名通过 opaque cast 静默调用。

virtual/interface call 先按 TargetABI 加载 slot 和完成 receiver adjustment，再以根槽的准确逻辑签名调用。v0 sync/async override result 都必须与槽声明完全相同，因此不生成 covariant result thunk。

dynamic destroy slot 与 deallocation 分离，且是 nounwind。interface receiver 逻辑上携带 canonical complete object pointer 和 interface table。

### 9.3 Function attributes

只有 Ink declaration 的 `[nothrow]` 稳定 contract 才允许普通同步 function/call 标记 LLVM `nounwind`。间接调用默认 may-unwind。

`[nothrow] async` 只保证 Task body 不发布 failed；Task construction 仍可能因 frame allocation、parameter capture 或 version pin unwind，因此 async construction entry 不自动标记 `nounwind`。

`never`/fatal/trap entry 可使用 `noreturn`。纯度、readonly、noalias 等 attribute 由完整 effect/escape analysis证明，而不是从 surface `const` 猜测。

## 10. 异常 ABI

Closed InkIR 的 `call.invoke`、cleanup block、catch dispatch、throw/rethrow 和 `eh.resume` 按目标选择：

- Itanium family：landingpad/personality/resume 或等价 funclet lowering；
- Windows family：catchswitch/catchpad/cleanuppad/cleanupret 等；
- 无 native EH target：runtime setjmp-style 或显式 error continuation，前提是可观察语义等价。

异常 record、cause、throw origin 和 traceback 的字节布局属于 opaque RuntimeABI。catch 顺序、`throw;` 跳过当前 handlers、readonly catch binding 和 cleanup 顺序是 normative，不能交给平台 ABI 自由决定。

trap/fatal 使用独立 noreturn intrinsic，不走 personality，不触发 cleanup。

## 11. Runtime semantic operation

以下 operation 通常降低为 versioned runtime intrinsic：

- reflection lookup、snapshot pin、DynamicRef adapter；
- Task construct、drive、await、publish、cancel、destroy；
- decorator continuation invoke/yield；
- exception box、throw record 和 unhandled boundary；
- stable-entry target selection、version pin、publish 和 quiescence；
- dynamic destroy、interface conversion 和 checked dynamic cast；
- comptime-only effect 在 Closed module 中不得出现；对应 runtime effect 需要 RuntimeWorld lowering。

intrinsic 名称、LLVM signature 和 opaque struct 是同一次兼容构建的 private RuntimeABI，由 `RuntimeAbiRevision` 和 ABI hash 约束，不是稳定 Ink library ABI。

## 12. Hot reload lowering

启用热更新时，公开可替换函数具有 stable entry。entry 以 acquire 语义原子取得同一次兼容发布中的 current version-local target 与 version pin，然后转入该 target；不能先加载裸 target 再补 pin。已经取得 pin 的 activation/Task/reflection snapshot 不重新读取 current target。

`rt.version.current_owner` 只把 version-local/helper activation 已有的 hidden version context 物化为 borrowed IR capability，通常降低为对固定 activation field/register 的读取或完全擦除；它不读取 current-version cell、不新增 pin，也不能出现在没有已 pin context 的函数。`rt.version.pin` 才由该 owner 建立额外 owned pin，`unpin`/`transfer_pin` 分别释放或转移责任。

InkIR 的 stable call 以 compatibility record 中 phase-specific `StableEffectEnvelope` 加 `RuntimeEffect(version_select)` 为优化边界。lowering 必须把 version selection 保留为对 optimizer 可见的 side-effecting runtime intrinsic、受约束原子序列或语义等价 barrier；它不可删除、不可 CSE、不可推测、不可复制，也不能跨 publish、pin lifetime、EH 或 Task ownership transfer 移动。当前 version-local body 的较弱 memory/runtime/allocate/deallocate/trap/diverge summary 不能给 stable call 添加 `readnone`、`readonly`、`willreturn` 等更强 LLVM 属性。

异常相关 LLVM attribute 使用发布记录中只可单调加强的 `BehaviorContracts` 与固定 envelope baseline 的交集。只有 effective sync may-unwind 为 false 才能给 stable sync entry/call 添加 `nounwind`；async construction/body 分别使用 construction nounwind 与 body no-fail contract，不能互相替代。当前 body 偶然不抛出或不失败不是 contract；一旦新调用者依赖了加强后的 contract，后续 code-only publish 不得减弱它。

v0 publish 只接受相同：

- logical function signature 和 calling convention；
- sync/async kind、result convention 和 nothrow dimensions；
- class/enum/tuple/global layout hash；
- vtable/interface slot set 与 adapter ABI；
- RuntimeABI revision。

还必须保持相同 registration encoding revision与`ModuleRegistrationInterfaceDigest`；该digest覆盖从当前 records 提取并去重排序的 `(RegistrationTypeSemanticIdentity, ProtocolSchemaDigest, TargetLayoutDigest, RegistrationEncodingRevision, RuntimeAbiRevision)` schema tuple，而不覆盖具体record集合。`ModuleRegistrationSetDigest`可以变化，但当前 records 所代表的 unique schema set必须不变；某 schema 的第一条增加或最后一条删除不是 v0 code-only update。新set只能与本次stable target、reflection metadata和globals同一事务公开，旧set只能按旧version owner撤销。

关闭热更新的静态 AOT build 可以把 stable entry 和 pin operation 消除为直接调用，但必须在 InkIR pass 中证明没有 reflection/version 语义需要它们；或者证明同一版本在完整选择/调用 dynamic extent 内被冻结且已有 pin 覆盖全部 version-local 地址。仅仅观察到当前只有一个版本、profile 单态或某个 body 是 pure 都不足以消除该边界。

## 13. C ABI 边界

Ink 不定义稳定 Ink-to-Ink binary ABI。`extern "C"` 是唯一 v0 稳定二进制边界，且只允许目标 C ABI 可表示的类型和约定。

Ink class、interface、exception、open generic、tuple、Task 和私有 runtime handle 不能直接出现在稳定 C signature。需要通过 `repr(C)` 类型、不透明 C handle、错误码、回调和具体闭合 wrapper 显式适配。

TargetABI 负责 C parameter classification、varargs 限制、name linkage 和 unwind 边界。C function 进入 Ink 固定浮点环境前后必须满足相应 ABI 责任。

## 14. LLVM 验证和优化顺序

建议 backend pipeline：

```text
VerifiedClosedModule
-> BuildTargetLoweringPlan
-> LowerTypesAndRuntimeDeclarations
-> LowerGlobalsAndModuleInit
-> LowerFunctionsAndCFG
-> LowerEHAndRuntimeOps
-> AttachProvenAttributesOnly
-> llvm::verifyModule
-> LLVM optimization pipeline
-> llvm::verifyModule
-> target code generation
```

LLVM optimization 后必须保留 InkIR 的 PDB operation/lowering boundary、strict/fast FP 边界、trap/exception 区分、cleanup、version pin 和 result destination 语义。对高风险 operation 应增加 IR-to-LLVM pattern tests 和 interpreter/AOT 差分测试。

## 15. 首个纵切片 C0

首个纵切片不需要实现完整 object/runtime ABI，但必须走真实阶段边界：

```text
source
-> CST
-> minimal semantic binding/type checking
-> StagedModule
-> force one comptime expression
-> ClosedModule
-> Closed interpreter
-> LLVM IR
-> native executable
```

### 15.1 C0 类型

```text
i32
i64
u32
u64
bool
void
never
```

首片可以只在 TargetContext 中启用上述类型；不能用 host `long` 或 C++ promotion 替代准确位宽语义。

### 15.2 C0 Core operation 与 plan opcode

```text
const.int
const.bool
arith.add
arith.sub
arith.mul
arith.neg
arith.cmp
cast.int
cf.br
cf.cond_br
cf.return
cf.unreachable
call.direct
```

ElaborationPlan 另外只需实现一个 plan opcode：

```text
stage.force_value
```

`stage.force_value` 不属于 typed Core CFG，也不得进入 Closed IR；它引用已经 elaboration 的 typed value region 或 source-backed template，并驱动 ComptimeWorld 得到 Known value。

除法、余数、shift、memory、exceptions、class 和 async 暂不进入 C0，避免在首片中伪造尚未设计完整的 PDB/lifetime/runtime 行为。

### 15.3 C0 comptime

最小目标示例：

```ink
[nothrow]
func add(left: i32, right: i32) -> i32
{
    return left + right;
}

const Answer: i32 = comptime add(20, 22);
```

C0 executor 只允许 pure integer/bool operation、direct call 和 CFG。`stage.force_value` plan node 要求结果 Known；fuel、call depth 和 integer bit width 均受限。成功后 Closed module 中只保留 `const.int 42`，不得残留 stage plan node。

### 15.4 C0 验收

- canonical text golden 稳定；
- binary encode/decode semantic round-trip；
- malformed CFG/type/stage artifact 被 verifier 拒绝；
- comptime 与 Closed interpreter 都得到 42；
- LLVM AOT executable 返回或打印相同结果；
- 修改 TargetKey、schema 或 compiler build digest 导致 cache miss；
- 并行运行和不同哈希种子产生逐字节相同 canonical IR。

## 16. 递增实现顺序

### C1：Place 与普通函数

- stack/global storage、load/store、default initialization；
- scalar caller/callee、function pointer 和 result destination；
- arrays、tuples、checked indexing 和 raw pointer numeric operation；
- typed copy 与 basic destructor cleanup。

### C2：对象与布局

- class/base/field、constructor/destructor、enum/tag/niche；
- vptr、vtable、interface two-word reference；
- dynamic cast/destroy 与 reflection descriptor skeleton。

### C3：异常

- invoke/normal/unwind、cleanup CFG；
- throw/catch/rethrow/cause、ExceptionBox；
- Itanium 与 Windows 至少各一个端到端 target。

### C4：完整 staging

- deferred generic body、heterogeneous `comptime for`；
- declaration sink、transactional fixed point；
- tracked read、effect ordering、binary staged cache。

### C5：装饰器、异步与热更新

- continuation region elimination；
- Task construction/drive/await/cancel；
- async virtual/interface/reflection adapter；
- stable entry、version pin、pinned reflection snapshot 和 code-only publish。

每一阶段都必须同时增加 verifier、canonical text、binary round-trip、interpreter 和 LLVM differential tests，不能只增加 parser 能接受的表面语法。

## 17. 建议源码模块

实现时可以按以下模块组织，不在本设计稿中冻结具体 C++ 类名：

```text
src/include/ink/ir
src/lib/ir
src/include/ink/semantic
src/lib/semantic
src/include/ink/comptime
src/lib/comptime
src/include/ink/backend
src/lib/backend
src/testcase/ir
src/testcase/comptime
src/testcase/backend
```

IR core 应独立于 parser CST 具体节点类。NormalizedTemplateTable 可以引用稳定的 normalized HIR 数据和 Core 的 `SourceFileId`/`SourceRange`，不能长期保存 CST arena 裸 pointer。
