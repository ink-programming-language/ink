# InkIR 验证、Pass 契约与确定性

## 1. 原则

InkIR verifier 是执行器和 backend 的安全边界。任何来源的 module，包括编译器刚生成的内存对象、本地缓存和未来可能实现的文本 parser 输出，都必须经过相应 verifier。

验证不能只检查“看起来能打印”。它必须证明：

- 结构和引用完整；
- operation schema、类型和控制流一致；
- SSA、place、初始化和生命周期合法；
- effect、normal/unwind 和 trap 边界完整；
- staged/closed 阶段不变量成立；
- target、runtime 和 ABI 前置条件满足；
- 执行或 lowering 不会依赖 host-only 数据。

## 2. 能力类型 API

建议的宿主 API 为：

```text
decodeBinary(bytes, DecodeLimits) -> DecodedArtifact | CacheMiss | CorruptArtifact
verifyStaged(DecodedOrBuiltStagedModule, StagedVerificationContext) -> VerifiedStagedModule
closeAndVerify(VerifiedStagedModule, StagedVerificationContext) -> VerifiedClosedModule<TargetKey>
verifyClosed(DecodedOrBuiltClosedModule, ClosedVerificationContext) -> VerifiedClosedModule<TargetKey>
interpret(VerifiedClosedModule<TargetKey>, ...)
lowerToLLVM(VerifiedClosedModule<TargetKey>, ...)
```

`StagedVerificationContext` required地持有trusted immutable `TargetContext`、与artifact Section 17逐节点匹配的`FrozenActiveModuleGraph`、`ResolvedDependencyArtifacts`、`BuildInputSnapshot`、`SourceSyntaxSnapshot`、`CapabilityPolicy`、`ComptimeHandlerRegistry`与K-only pass/schema/build identities。`ClosedVerificationContext` required地持有同一`TargetContext`、`ResolvedDependencyArtifacts`、`CapabilityPolicy`和`ComptimeHandlerRegistry`，并携带需要校验cache/build compatibility时的K-only identities；它不接收source snapshots。两类context都必须重算Manifest的TargetKey/TargetContextDigest、active DAG/interface digest、capability/handler digest与RequiredFeatureSet；每个non-root graph node必须唯一绑定实际已验证provider artifact及其exports/stable mapping，不允许只凭D32伪造HasImplementationBody或import closure。`BuildInputSnapshot` 与 `SourceSyntaxSnapshot`只允许按已登记 `ContentDigest` 提供 immutable source bytes、source declaration tree 与 normalized HIR/structural-path replay，不得在验证中回退到 live host filesystem、可变 CST arena 或只凭一个 digest 接受路径。`verifyStaged` 必须使用这两个 snapshot 完整重放 stable declaration lookup、每个 structural-path ordinal 与 HIR node 终点。

`VerifiedStagedModule` 和 `VerifiedClosedModule` 应当是不可伪造、不可通过修改普通枚举获得的能力对象。`VerifiedStagedModule` 还必须内部持有不可伪造、不可序列化的 `SourceReplayBinding`，把该能力绑定到 `verifyStaged` 实际使用的两份冻结 snapshot 及其精确 source revision/digest 集合；`closeAndVerify` 必须经此 binding 取得同一 snapshot 并再次执行完整 source replay，binding 缺失、已失效或与 module/source revision 不匹配即拒绝。实现也可以把两份 snapshot 作为 `closeAndVerify` 的显式参数，但仍必须与 `VerifiedStagedModule` 中承诺的 binding 逐项相等，不能用调用方新提供的同名 host path 替换。成功 close 才把这些 replay 事实承诺进 `SemanticModuleDigest`。任何会改变 IR 或这些 snapshot 绑定的 pass 都使旧验证能力失效，必须返回新的未验证 module 或在 pass manager 中重新验证所声明的不变量。

`verifyClosed` 的签名刻意不接收 source bytes、`BuildInputSnapshot` 或 `SourceSyntaxSnapshot`，但仍必须接收上述ClosedVerificationContext，以验证target、provider、policy与handler的不可逆digest承诺。它只验证 artifact 内部可重算的 SourceFile identity/content digest、path/tag canonical encoding、transition 类型、冗余 stable-key/content-digest 绑定与摘要；它不声称从 digest 逆推出 source tree，也不得隐式读取 host。需要重新审计源码真实性时，调用方必须以冻结 snapshots 重新执行 `closeAndVerify`。

## 3. 第 0 层：二进制解码验证

binary decoder 在构造大量对象前检查：

- magic、artifact kind、container version 和完整 header；
- fixed header `ArtifactKind` 与 Manifest `artifact_kind` 逐值相等；text verifier 对应要求 outer kind 与 Header 相等，任一不一致在选择 section/stage 规则前即拒绝；
- 文件长度、payload length、section offset、section length 与无重叠约束；
- header、directory、payload 和需要认证 section 的 digest；
- section count、table count、string length、aggregate arity、region nesting、CFG size 和递归深度预算；Normalized HIR 还分别限制payload总bytes、node count、单node bytes、vector arity/item总数、child edge、nesting depth、identifier/structural-path bytes、各类table reference与materialized memory；Section 16 还分别限制 registration count、OrderPath U总数、DynamicPath step总数/深度、frozen constant graph node/depth与relocation count；每个count在分配前同时受remaining bytes可容纳上界和global cap约束；
- ULEB/SLEB canonical encoding，不接受超长或溢出编码；
- UTF-8、字符串转义后长度和 symbol path 语法；
- table index 与 local reference 范围；
- APInt width/byte count、float format/bit width 一致；
- required section 唯一性与依赖顺序；
- unknown required section/opcode/type 直接使缓存失效；unknown optional metadata 按长度安全跳过。

decoder 不得序列化或信任宿主 enum 物理值、`size_t`、pointer、vtable、对象 padding 或 arena address。

缓存损坏默认报告为 cache corruption 并重新构建；只有显式校验工具才把它当作用户输入诊断。任何损坏都不得导致越界访问、无限分配或执行未验证 operation。

## 4. 第 1 层：公共结构验证

### 4.1 表和身份

必须检查：

- TypeId、ConstantId、SymbolId、FunctionId、BlockId、ValueId、RegistrationId、OriginId 唯一且引用存在；
- canonical symbol identity 不冲突；
- entity owner/cardinality满足10 §9.2矩阵：nominal Type、Global、Function、RuntimeDeclaration与SymbolKind匹配，required definition/import entity恰好存在，同表owner不重复，同一Runtime symbol恰有一个RuntimeDeclaration，只有stable-entry组合可跨Function/Runtime两表共享；CallableKind=runtime_thunk与`SymbolKind=runtime` Function均拒绝；`SymbolKind=reserved_constant`拒绝，命名常量必须是`global` symbol加constant Global definition；
- 当前module的logical stable symbol必须同时具有bodyless `EntryIdentity=stable` Function declaration和唯一StableEntry RuntimeDeclaration；其VersionLocalSymbol是generated `version_local_body` definition，parent=`versioned_owner(LogicalStableSymbol, ModuleVersionContextKey)`且key等于当前Manifest/semantic input重算值。不同semantic context的物理body不得复用SymbolKey；K-only variant必须复用该语义SymbolKey，但以不同ArtifactVariantKey/LoadedVersionKey隔离cache、物理mangling与version owner。active dependency只保留bodyless logical import projection；
- 每个GeneratedDeclarationIdentity必须满足10 §9.2逐GeneratorKind parent/role/`SemanticOutputOrdinal`推导矩阵：generic instance与version body固定0，decorator declaration绑定完整expansion_context且每event一个declaration，bridge由direct owner或完整indirect runtime signature唯一化，adapter/thunk从不读取child identity的完整owner-slot序列取index，special member固定role ordinal，lowering helper绑定versioned entity owner；泛化连续编号、发现顺序、worker/round/hash次序或缺少反向owner引用均拒绝。source_backed Symbol的ClosedGenericArguments必须empty，闭合实例只能使用non-empty generated generic_instance；stable泛型function instance必须建立自己的logical mapping；
- GenericDeclarationProvenance marker必须exact-decode tag-2 `GenericDeclarationSignatureSurfaceVector`：内外ParameterKinds逐项相等，surface kind/count与ProducedSymbolKind唯一匹配，generic parameter/pack ordinal和use-kind有效，dependent expression tree只含获准inline semantic node且不残留TypeId/SymbolId/TemplateId/自由digest，单一parameter与直接pack splice不得改写成退化tree，self declaration只用无payload sentinel。verifyStaged/closeAndVerify从source binder与open entity重建同一vector，verifyClosed从retained vector重算SignatureDigest、terminal StableDeclarationStep digest与SymbolKey；只比较两个D32或保留当前SymbolKey self回边均拒绝。marker完整key preimage可达的GenericMarkerSymbolKeyDependencyGraph必须先按claimed reference建图并证明无环，再dependency-first重算；间接marker-key cycle不能交给canonical-record SCC或hash迭代；
- AbiImport/AbiBridge/AbiExport只作为declarative RuntimeDeclaration存在，backend据此合成C wrapper；不得另建Function `extern_bridge`，`FunctionRecordKind=reserved_extern_bridge`与任何Function `calling_convention=c`均拒绝；
- local ID 不跨越其合法 function/region scope；
- origin DAG 无非法循环，所有 source origin 绑定存在的 source file 和合法半开范围；
- Origin writer 已按完整 required payload 做结构 interning，parent先于child；canonical decoder/verifier拒绝两个结构相同而OriginId不同的重复record；
- 从 canonical binary 解码的semantic table已经严格符合canonical order且无重复；decoder/verifier不得静默重排或去重。编译器内存builder可以在调用verifier前显式canonicalize，但这会形成一个新的待验证artifact。
- 每个GlobalRecord的RecordKind×module ownership×Linkage×Storage与initializer cardinality满足10 §9.3闭合矩阵：actual-storage definition恰有typed Constant或InitializerFunction，runtime-owned/import/declaration无module lifecycle；static仅在dynamic initializer/finalizer存在时进入`eager_module`，所有thread_local definition无条件进入`eager_thread_activation`，其他项为`none`。非none分区ordinal从零连续，dependency只指向同分区更小ordinal；stable Kahn按ready set最小Global identity重算后逐项等于编码ordinal，image-only TLS与static image+finalizer step都不能遗漏。
- initializer/finalizer必须是private version-local lowering helper并绑定当前`versioned_entity_owner(Global.Symbol, ModuleVersionContextKey)`，SemanticOutputOrdinal分别固定0/1且由该Global字段反向唯一引用；activation/finalization持有匹配module-version owner，helper、global storage generation、AlivePrefix与cleanup plan不得跨Global/version拼接或在publish后重新选择current body。

### 4.2 Operation schema

每条 operation 必须满足 opcode registry：

- operand/result/region/successor 数量；
- 每个 operand/result 的类型约束；
- required/optional attribute 集合与 attribute 类型；
- terminator、symbol、constant-like 等 trait；
- 合法 artifact kind 和 world；
- effect、may-unwind、may-trap、PDB 与 speculatability；
- nested region capture/yield 规则；
- successor argument 与目标 block argument 一一对应且类型相同。

缓存中的 effect summary 只可作为加速索引，verifier 必须从 opcode 与 attribute 重新推导并比较。

### 4.3 CFG

每个非 external function：

- 至少有一个 entry block；
- entry block arguments 的 `BlockArgumentRoleTag`、role-local index、type 和固定顺序与 Function/Global record 推导的 envelope 精确一致：可选 receiver、ordered parameters、可选 global_lifecycle、可选 sync result_destination、async task_self、可选 async task_result_storage；passing mode为object的逻辑参数T固定映射为Alive owner `!place<rw,T>` parameter argument，其他parameter按其mode映射准确SSA/typed handle；不存在第二套独立parameter ValueId；
- constructor 只有 initializing receiver 而没有额外 result_destination；sync result_destination entry 已携带 active owner transaction；async task_self 是borrowed rw Task place，address-only T的task_result_storage是runtime-created owner init place且entry时AllocatedUninitialized、没有TransactionId；
- global_lifecycle只允许出现在恰好一个GlobalRecord指定的initializer或finalizer entry，v0 lifecycle function不得被另一global复用或兼任init/finalize；initializer取得匹配`!place<init,T>` owner，finalizer取得匹配Alive `!place<ro/rw,T>` owner；普通调用不能进入该runtime-only entry；
- receiver/parameter/global_lifecycle/result_destination/task_self/task_result_storage 只允许出现在对应 function entry；非 entry CFG block argument role 为 phi，nested region entry为region_argument或其owner opcode规定的更窄role；
- 每个 block 恰好以 terminator 结束；
- terminator 后无 operation；
- successor 属于同一 CFG region；
- normal edge、unwind edge 和 stage control edge 不得混用；
- `cf.return` 与 function result convention 一致；
- `never` function 没有正常 return；
- 进程内未规范化 CFG 可以暂存 unreachable block 供诊断，但进入 canonical Staged/Closed artifact 前必须删除；canonical encoder/decoder/verifier 发现任何从 region entry 不可达的 block 都拒绝，`cf.unreachable` 作为可达 block 的 terminator 仍合法。

### 4.4 SSA 与支配

SSA verifier 检查：

- 每个 value 恰有一个定义；
- operation result 只在其定义之后且受其支配的位置使用；
- block argument 只在所属 block 及其支配后继中使用；
- nested region 只能使用显式 region argument、显式 capture 或在 schema 允许时使用外层支配 value；
- value 不从 nested region 非法逃逸；
- 多结果 operation 的所有结果同时定义；
- 没有独立 phi opcode 或从 predecessor 偷读局部 value；block argument 本身是规范 phi；
- place/reference/slice/interface block argument 把每条 incoming edge 的 value 与其 `LifetimeAuthority`、`PlaceCapabilityGeneration`、`ObjectLifetimeGeneration` 或 `BorrowGenerationSet` 作为不可拆分的 abstract pair 合流，不能把一条 edge 的地址/descriptor 与另一条 edge 的 generation 配对；reflection private DynamicRef不是Core value，不能成为block argument；
- generation side fact 形成与该 block argument 绑定的 edge-indexed generation phi，后续 `CapabilityRebind` 在线性数据流上消费实际选中的 incoming generation；owner authority 只有在每条 incoming edge 都把完整且未别名的 owner/cleanup obligation 转移给该 block argument 时保留，否则 value 至多 meet 为 borrow，绝不从 borrow 升级为 owner；每条 owner incoming 的 obligation 还必须已在该 edge 显式 discharge，或转移给另一支配 owner/cleanup state，否则 join 因丢失唯一 lifecycle obligation而非法。

线性 runtime handle 使用同一 edge-sensitive ownership dataflow：owned snapshot、version pin及其他 `OwnedHandle` 在每条动态路径上必须恰好被 matching release/unpin/transfer/final owner 消费一次；branch 可以把同一 owner 写到互斥 successor edge，但 join 不得复制、丢失或把不同 owned identity 混为一个。`BorrowedHandle`/member view不承担 release，但其所有 uses 必须受 owner lifetime支配，不能存储、返回、跨 owner release、跨不获准暂停或通过 phi 逃逸。clone schema 明确产生 fresh owned identity，绝不只是复制原 token。ExceptionBox是内部`RuntimeHandleKind=exception_box`，revision 1没有用户SSA producer；其强引用只存在于failed Task、ActiveUnwindRecord和cause backing中，由exception/task schema验证最后owner释放。

## 5. 第 2 层：类型、常量和布局验证

### 5.1 类型规范化

必须检查：

- type 构造参数完整且规范化；
- alias 不残留在 Core InkIR；
- `byte/int/uint` 已分别规范化为 `u8/i64/u64`；
- `void`、`never`、builtin `unit` 不发生隐式互换；源码 `()` 已在进入 Core 前规范化为 `unit`，TypeTable 不得出现零元素 tuple；
- array Type.KindPayload恰为`ElementType : Ref<Type>, ElementCount : U`；ElementCount是canonical无符号字段而不是ConstantId或待执行comptime value，`size(ElementType) * ElementCount`及最终大小/对齐不得溢出目标布局限制；
- slice、reference、raw pointer 的 const/address-space/no-escape 信息一致；
- revision 1 的所有 pointer type、constant 与 operation 只能使用默认 `AddressSpace=0`；任何非零值直接拒绝，不存在可协商的v0扩展；
- function type 不含 default argument、参数名称、`comptime function` 位或 `[nothrow]` 位；
- 每个被AbiImport/AbiBridge/AbiExport或AbiCallPayload引用为`CFunctionTypeId`的Type(function)都必须满足唯一矩阵：ordinary callable（`CallableKind=function`）、`FunctionKind=sync, ReceiverKind=none, ReceiverType=absent, CallingConvention=c`；全部parameter mode恰为`value`并按引用record的TargetAbiTag和zero-based parameter position满足四参`CAbiSafe`，通过该谓词的repr(C) aggregate合法。logical result为void时mode恰为void，其他获准C类型时mode恰为value并在result position满足`CAbiSafe`；物理sret不改变该逻辑mode。never、result_destination、object/const_reference/mutable_reference/raw_pointer parameter mode与varargs均拒绝；raw pointer类型只能作为mode=value的位置，不能与raw_pointer passing-mode混同；status、callback与result-out channel必须是该矩阵内的普通value位置并由唯一CAbiPositionPayload指出，不得形成隐藏channel或第二种signature；
- staged-only meta/dependent type 只出现在 Staged verifier 允许的位置；
- `runtime.object<kind,args...>` 的 kind/arguments 闭合且具有当前 RuntimeAbiRevision 注册的 size/alignment/RuntimeStorageAbiHash；它是 address-only/noncopyable SizedObjectType，但只能由该 kind 的 semantic operation 使用；v0 task kind恰有一个T，且T必须为void/never或闭合runtime-representable、Copyable且CopyConstructionEnabled，并满足`!ContainsNoEscapeValue(T)`；
- `runtime.opaque` 不能形成 place、typed reference、`sizeof/alignof` 或 object lifetime，opaque runtime type 与 `!runtime-handle` 都不能冒充 runtime object。

v0类型与operation registry不含通用heap、arena或tracing GC模型。任何未注册heap/arena/gc storage kind、通用allocation/deallocation、collector root、write barrier、safepoint或tracing descriptor都必须拒绝；专用task/exception allocation effect不能被解释为通用用户对象分配能力。

### 5.2 常量

整数常量必须记录 bit width、signed semantic type 和准确 bit pattern。浮点常量必须记录 format 与准确 bits。aggregate constant 按逻辑 element 编码，不复制宿主结构体内存；array constant的元素数必须恰等于Type.`ElementCount`且每项类型等于`ElementType`，不能另存或从constant vector反推第二个`ElementCount`来源。

常量必须满足其类型的有效表示。v0 不存在 reference constant；reference 只能由合法 place 的 borrow operation 建立。raw pointer constant 只允许规范 null 或可验证 relocation，host pointer 和 comptime temporary address 不合法。

### 5.3 Target layout

Closed verifier 使用 TargetContext 重算并核对：

- size、alignment、field/base/tuple offset；
- enum tag/niche strategy 与 active payload 表示；
- vptr/interface representation 的 TargetABI revision；
- `ptrsize` 和各 address space pointer width；
- 每个 runtime object kind 的 opaque storage size/alignment 与 `RuntimeStorageAbiHash`，且没有通过 field/raw typed access 暴露私有表示；
- layout hash 和 ABI hash；
- target-specific byte blob 的类型、长度、endianness 和 TargetKey。

TargetContext还必须先从不含TargetKey/LayoutDigest派生值的`TargetBlobConstantAbiRulesProjection`重算Manifest.TargetKey，再派生完整TargetBlobConstantAbiTable并重算TargetContextDigest；raw rule与derived entry数量、registry builtin type、byte length和zero-bit mask逐项双射。任何`target_blob`的entry/payload TargetKey都等于当前Manifest.target_key，LayoutDigest按该TargetKey与Constant.Type重算，不能形成TargetKey摘要自环或接受另一target的自洽table。

序列化的布局结果不能覆盖 TargetContext。任何不一致使缓存失效或 module 验证失败。

### 5.4 `ContainsNoEscapeValue`

`ContainsNoEscapeValue(T)` 是结构闭包性质：safe slice 具有该性质，array、tuple、class、enum 或其他聚合只要任一可达组成部分具有该性质，整体就具有该性质；type alias、匿名 tuple lowering、byte cast 或私有 ABI 拆分不得清除该标记。

具有该性质的值只允许存在于当前同步 activation 的参数对象、局部对象、临时对象、block argument 和直接受控调用实参中。verifier 必须证明它不会被写入或捕获到任何可能超过本次同步调用动态范围的 sink，包括：

- global、module state、static registration record、普通对象字段和长期容器；
- function result 或 caller result destination；
- closure/coroutine/async Task frame、Task completion storage、ExceptionRecord 或 ExceptionBox；
- reflection/FFI/type-erased storage、host effect payload 或其他 opaque runtime object；
- 未知间接调用、C ABI 调用或没有声明并验证 no-escape 参数契约的 runtime handler。

同步 Ink call 可以把该值传给准确 function type 中同样标为 no-escape 的参数；callee verifier 必须禁止该参数进入上述 sink，并禁止从结果、异常或 runtime effect 间接逃逸。async declaration、async construction adapter 和可跨暂停的 continuation 不得按值捕获此类参数。CFG 合流、inline、specialization、aggregate flattening 和 TargetABI 寄存器拆分都必须保持这一证明；不能证明时拒绝 IR，而不是把 safe slice 降为 `RawSlice`。

## 6. 第 3 层：Place、初始化与生命周期验证

verifier 对每个 CFG path 跟踪 place 状态：

```text
Unallocated
AllocatedUninitialized
PartiallyInitialized(mask/tree)
Alive
Deallocated
```

InkIR 没有 moved state。`obj.destroy`/`obj.destroy_dynamic` 令 Alive storage 回到 `AllocatedUninitialized`；只有独立 deallocation 再把它转为 `Deallocated`。

`PartiallyInitialized(mask/tree)` 的 verifier state 至少包含 active root/child `TransactionId` tree、每个 active transaction 的 `InitializedLeafMask`、每个 initialized leaf 的 `(SubobjectPath, ObjectLifetimeGeneration, ConstructionOrdinal)`、`CommittedChildMask`，以及 committed child 的 Alive fact。identity 是线性验证事实而非 SSA value；active transaction CFG join 只在 active parent identity、两个 mask、leaf generation/order 与 Alive child facts 全部相同时成立。已经 commit 或 rollback 的 terminal child identity 不属于 live state，因此两次不同 child identity 的成功路径可以在相同 parent/mask 状态处合流。普通 Alive owner/borrow 的 generation 可以按上一节形成 edge-indexed phi，但不能借此合并不一致的 active transaction。

必须证明：

- allocation 存在后才能取得 place；deallocation 后不能再使用；stack storage 只由 activation teardown 释放，其他 storage release 必须是消费准确 allocator kind/version/owner token 的具体 runtime schema，Core 不接受通用 `mem.dealloc`；
- 任何使用 element size、alignment 或 typed representation 的 `ptr.offset`、`place.deref`、typed raw load/store 和 object lifecycle operation 都要求 `SizedObjectType(T)`；unsized-pointee pointer 只能进入 schema 明确允许的 cast/access、byte offset、address compare 和显式 byte-count raw memory operation；
- `mem.load` 只读取 Alive object 或已初始化 scalar/subobject；
- `mem.store` 不用于建立 nontrivial object；
- lifecycle operation 的 place 具有准确 `owner` LifetimeAuthority；`place.from_ref`、`place.deref`、普通 `mem.global_place` 和 borrowed projection 即使 access 为 rw 也只能读写当前 Alive 对象，不能 begin/commit/destroy/reinitialize；global owner只来自GlobalRecord唯一initializer/finalizer的runtime-provided `global_lifecycle` entry并绑定准确module/thread generation；owner authority 不得由 raw pointer、普通 copy、FFI 或 access cast 伪造；
- `obj.init.*` 的 destination 尚未初始化且类型准确；
- `place.as_alive` 保持地址、allocation、subobject path、类型与 alignment，只在当前路径已证明准确 object Alive 时把 init capability 物化为不超过 owner mutability 的 ro/rw capability；结果不携带 active TransactionId；
- `place.as_uninitialized` 只接受拥有销毁/重建权限的 rw place 并保持同一 storage identity，只在 destroy 后准确状态为 AllocatedUninitialized 且没有存活 borrow/child capability 时产生无 TransactionId 的 init capability；它不能执行 destroy、处理 rollback 或复活旧 identity；rollback 后原 init generation 保持，可以直接 fresh begin；
- 两个 rebind opcode 都具有 `CapabilityRebind` trait：每条动态路径上恰好消费 source `PlaceCapabilityGeneration` 并产生 next generation；旧 generation、SSA alias 和从它派生的 descendant capability 此后不得使用，ancestor root rebind 还使构造期 descendant view 失效；它们不可复制、推测或越过对应 commit/destroy/CFG join 移动；
- 每次成功 root/child commit 建立 fresh `ObjectLifetimeGeneration`，active aggregate parent 的 direct scalar/unit leaf 在 `obj.init` 成功时原子建立 fresh leaf generation，root scalar/unit 仍到 commit 才建立 root generation；parent commit不替换已有leaf/child generation，rollback与complete destroy按逆构造/析构顺序终结全部descendant generation后再终结root；reference、borrowed place、safe slice `BorrowGenerationSet`和interface reference的每次use都要求所捕获generation仍与当前Alive object/range匹配，同址同型重建不能让旧borrow复活；reflection private DynamicRef只能临时继承这些已验证fact，不能自行刷新generation；
- `slice.init` 的非空 data source 必须是携带有效 generation 的 checked place 并覆盖完整连续范围，不能直接使用 raw pointer；`slice.init_empty` 是唯一无来源 canonical null-empty 构造；subslice/typed copy 保留对应 generation set，`slice.data`/RawSlice 只产生扁平 raw address而不保留它；
- `obj.assign.*` 的 destination 已 Alive；
- `obj.destroy.*` 恰好作用于 Alive object；
- noncopyable/address-only value 不被隐式 materialize、复制或作为非法 SSA aggregate 返回；
- language typed copy 只在 `copyable(T)` 成立时出现，并按字段/active payload 语义；
- raw memory operation 不被用来伪造 typed lifetime、vptr 或 enum active state；
- `SealedRuntimeStorage` 只进入 allocation/destination formation、owner begin、capability rebind 和匹配 runtime-kind schema；普通 projection、`place.addr`/`place.deref`、typed/raw load/store、copy/assign/generic destroy 与任何重叠 active/partial sealed range 的 byte-count raw operation 全部非法，pointer cast 不清除该 range fact；
- padding 未初始化字节不被 raw load、比较或 `raw.memcpy` 源读取；
- 条件初始化在 CFG 合流处使用一致状态或显式初始化 flag；
- function 每个正常出口都完成 result destination，unwind 出口保持其未初始化；
- 每个 address-only `call.* ... to %destination` 都处于唯一 active root 或 child transaction：fresh storage owner 的 `obj.init.begin` 支配首次 root 构造调用，非平凡 direct subobject 的 owner 在 active parent 内对准确 child path 建立 fresh child；转发层复用同一 transaction identity 且不得重复 begin/commit/rollback；
- transaction identity tree 与对象包含树严格同构；一个 active parent 同时至多有一个 active child，同一 subobject path 在 active/Alive 时不能重复 begin，非 ancestor/descendant transaction 不得部分重叠，base/field/element/payload 构造顺序必须满足类型 schema；child rollback 后可以在恢复为未初始化的同一路径 fresh begin，但旧 identity 永久终止；
- child 正常完成时恰好一次 `obj.init.commit`，只把 child 标为 Alive 并登记 parent `CommittedChildMask`；parent 没有 active child且 leaf/child masks 完整时才可 commit，只有 root commit 发布完整结果；最终失败的 callee先清理当前 transaction 的 committed descendant，再由 outward-unwind schema postcondition唯一 rollback 当前 root/child；child rollback 保留 parent identity、masks 与既有 sibling，parent 若继续 outward unwind 则按逆构造顺序逐层 cleanup/rollback；call opcode 和转发层不合成这些状态转换；
- commit 或 destination-call normal postcondition 后若路径需要访问 address-only 对象，必须显式使用 `place.as_alive`；原 `!place<init,T>` 不能直接用于 load/borrow/assign/destroy。invoke unwind path 没有 Alive proof，不得执行该转换；
- `async.await` 与 `async.await_copy` 恰有 normal/unwind 两个 CFG successor，pending 只暂停 activation 并登记当前 operation continuation，不形成第三个 successor；`async.await_copy` 是 schema 特例，其 active transaction 跨暂停保持，完整且不可失败的 typed Copy 后 normal postcondition 唯一 commit，只有 Task failure 进入 failure postcondition 并 rollback，successor 不得重复转换；
- constructor unwind 只销毁已完成的子对象，成功后完整对象恰好开始一次生命周期；
- destructor/dynamic destroy 后 storage 回到 uninitialized，再由独立 operation 释放。

passing mode为`object`的call operand必须追溯到显式构造完成的prepared owner `!place<rw,T>`。verifier按源码顺序跟踪每个parameter cleanup obligation：activation建立前较后实参、dispatch、adapter验证或资源失败由caller逆序销毁全部prepared对象；activation建立点必须原子把全部owner转移到callee；其后normal/unwind由callee恰好清理一次。async construction还要证明转移前caller、转移后construction cleanup及成功Task capture三段owner互斥且无空档。

Task destination允许owner执行`obj.init.begin`，但`SealedRuntimeStorage`禁止generic `obj.init.commit`；只有async.call/async.invoke或匹配runtime adapter的normal schema postcondition可原子消费transaction、建立fresh Task generation和Alive proof。failure schema只rollback，任何显式Task generic commit都非法。

如果完整路径敏感分析过于昂贵，lowering 可以显式生成初始化 flag 和 cleanup CFG，使局部 verifier 证明同一性质；不得直接放弃验证。

## 7. 第 4 层：调用、异常与效果验证

### 7.1 调用

每个 call site 必须满足：

- direct symbol 或 indirect function type 完整闭合；
- parameter 数量、类型、passing mode 和 source-order lowering 一致；
- scalar/result-destination convention 与 callee 匹配；
- result destination 具有准确类型、对齐和未初始化状态；
- direct/indirect/virtual/interface/reflection call 使用对应 opcode，不能绕过 receiver adjustment 或 adapter；
- stable entry、version-local body 和 decorator continuation target 不混淆；
- stable direct/virtual/interface call 从 compatibility record 的 `StableEffectEnvelope` 重建 phase-specific effect，并额外包含 `RuntimeEffect(version_select)`；只有持有匹配 pin 的 version-local call 才能使用具体 body summary；
- `EntryIdentityTag=extern` target仍只使用Ink logical ABI的`call.*`；C boundary只使用declarative AbiImport/AbiBridge/AbiExport与`abi.call`/`abi.invoke`。repr(C)类型定义先满足`CRepresentationSafe(T, TargetAbiTag, TargetKey)`；每个实际C参数、结果、status、callback和result-out位置再满足`CAbiSafe(T, TargetAbiTag, TargetKey, CAbiPositionPayload)`，position的Root/RootOrdinal/AggregateFieldPath与TargetAbiTag、ABI digest提交的唯一classification逐项一致；
- AbiCallPayload的direct form没有C function-pointer operand且DirectSymbol解析唯一AbiImport，BridgeSymbol只能是相同AbiImport identity或ExternalTarget绑定它的InkToExternal bridge；indirect form的operand 0是C function pointer、没有DirectSymbol，并且只接受ExternalTarget/TrustedExternalEffects均absent的InkToExternal bridge。`ForeignFailurePolicyPayload=nothrow`才允许abi.call，`ForeignFailurePolicyPayload=status_to_exception`只允许abi.invoke；call-site字段不能覆盖record policy；

passing mode为object的logical T在call site和entry都必须使用06定义的prepared/handed-off owner place表示，不得把原始source object、普通T SSA或无owner borrow直接当参数。`destructor_body`必须满足sync/mutable_instance/零参数/void/ink/nothrow闭合矩阵，只能由obj.destroy或obj.destroy_dynamic的compiler-generated plan进入，任何普通call、function value、slot、reflection、decorator或extern引用都非法。reflection `reflect.call`还必须同时验证target_nothrow和adapter_nothrow；adapter_nothrow覆盖signature/type/layout/generation/resource/result适配，任一证明缺失都必须使用callee_kind=reflection的call.invoke。

### 7.2 Normal/unwind

默认 call 可能 unwind。只有 callee declaration 的稳定 `[nothrow]` contract 或 runtime opcode schema 可以证明 nounwind。

may-unwind call 或 Task construction 必须使用 `call.invoke`、`async.invoke` 形态或具有等价显式 unwind successor 的专用 schema。`eh.throw`、`eh.throw_copy`、`eh.throw_from`、`eh.rethrow` 和 `eh.resume` 本身是线性转移异常所有权的传播 terminator，不需要再包一层 invoke。unwind successor 接收准确 exception token/record view，不能当普通 SSA result 返回。

析构、defer、隐式清理、`implicit` constructor 和 dynamic destroy 的 nothrow contract 必须验证；其内部异常传播边在 Closed IR 中只能进入 fatal path。普通非 `implicit` constructor 默认仍可 unwind。`rt.fatal %exception`是合法线性token endpoint并消费一个active token；operandless rt.fatal不消费token。两种形式都无normal/unwind/trap successor且不执行语言cleanup。

### 7.3 Effect

effect verifier 检查：

- memory read/write 与 alias domain 合法；
- volatile/atomic/address-space operation 只有在对应 opcode 已定义时出现，v0 不用普通 load/store attribute 模拟未设计语义；
- comptime effect 具有 capability 和 handler；
- RuntimeWorld effect 具有 runtime lowering；
- declaration sink默认只接收source-backed template expansion；唯一v0例外是`DeclarationSink(module_registration)`的`ct.register_module_item`，它可在active decorator application中接收schema允许的scalar/unit SSA或准确`!place<ro,T>`，但必须满足`StaticRegistrationEncodable`、`OrderedSemanticEmit`和第8节完整事务规则；其他sink不得借此例外接收普通SSA；
- trap/fatal 不连接 catch 或 cleanup edge；
- PDB operation保留PdbBoundary/MayTrap性质，不能标记speculatable；TargetDependent只表示结果依赖TargetContext，若没有其他阻止条件仍可speculate；
- `MayDiverge`、`StableEffectEnvelope` 与 `RuntimeEffect(version_select)` 按 registry/compatibility record 重新推导；stable selection effect 不得标记可删除、可 CSE、可推测或可复制，当前 body summary 只能逐字段证明为 envelope 子集，不能覆盖 stable call effect；
- `obj.destroy_dynamic`从dynamic-destroy stable envelope无条件并入完整CallSummary、`RuntimeEffect(version_select)`和`MayDiverge`；nothrow不允许擦除析构链memory/runtime/allocate/deallocate/trap/fatal上界；关闭热更新后的pass只有证明版本覆盖整个dynamic extent时才可消除version_select；
- async construction从callee/slot/envelope重建verifier-only TaskBodyEffect与TaskDestroySummary并绑定fresh Task generation；await/await_copy/drive并入TaskBodyEffect，destroy并入TaskDestroySummary，phi逐字段取并集，跨函数或provenance丢失时使用最大保守上界；
- direct ExternalEffectSummary从AbiImport及其匹配target-bound AbiBridge的相等`TrustedExternalEffects : Opt<EffectUpperBoundPayload>`重建并计入RuntimeDeclaration AbiDigest与dependency identity；absent时使用ReadWriteAny、全部allocate/deallocate/runtime、TargetDependent、PdbBoundary、MayTrap与MayDiverge。indirect bridge的ExternalTarget与TrustedExternalEffects必须absent并固定使用该最大保守上界；call-site自由attribute不能缩窄；
- `[fast_math]` flag 集不含 v0 禁止的 approximate/finite flags；`nnan/ninf` 只有 `[assume_finite]` 证明时出现。

### 7.4 Async suspension

`async.await` 与 `async.await_copy` 只允许出现在声明为可暂停的 async/coroutine activation 及其规范 continuation region 中；普通 sync function、constructor、destructor、普通 decorator region 和 comptime-only CFG 出现它们一律非法。

verifier 对每个 await 计算 live-across-suspend 集，并要求每个成员具有 schema 声明的 `SpillableAcrossSuspend` 表示：

- scalar SSA 必须能按准确类型写入 frame；host/comptime handle、continuation-local capability 和未声明可 spill 的 runtime token 禁止跨暂停；
- local object/place 若在恢复后仍使用，必须已经 frame-resident，或在 coroutine lowering 前具有可验证的稳定 owner/rematerialization；普通即将退栈的临时 storage 不能留下悬空 place；
- reference/raw pointer 继续遵守语言的非拥有与潜在 UB 规则，但 safe slice 或任何 `ContainsNoEscapeValue` 不能跨暂停、进入 frame 或 continuation capture；
- 可跨暂停的checked reference/interface/typed place必须证明referent generation与owner lifetime覆盖完整暂停区间；其`ObjectLifetimeGeneration`/authority phi作为frame verifier state spill并在resume后重新校验，不能因物理frame只保存地址而遗忘；reflection private DynamicRef不得跨暂停，raw pointer不获得checked保证；
- `async.await_copy` 的 destination storage、transaction identity、部分初始化 tree 和 cleanup state 必须全部 frame-resident并保持到 normal/failure postcondition消费 transaction；
- 所有跨暂停cleanup flag、version pin、Task owner和必要borrow state都必须成为frame的显式逻辑字段，resume/destroy两条路径均恰好释放；failed Task内部ExceptionBox引用只由sealed runtime schema管理，不得物化为frame Core handle；
- pending 不产生第三个 CFG successor；frame state 保存当前 await continuation，Task 完成后才以 `result(N)` 或 `exception` 进入原 normal/unwind successor。

async frame-layout lowering 必须在删除高层 await 前重新验证上述 spill map，并把它纳入 TargetABI/frame layout hash。不能证明 storage 生命周期、capability 可 spill 或 destroy cleanup 闭合时必须拒绝 IR。

## 8. 第 5 层：Staged verifier

Staged verifier 额外检查：

- `StagedModule` 同时具有 typed Core、ElaborationPlan 和自包含 NormalizedTemplateTable；
- 每个PlanNode的StageOpcode、typed Inputs、TemplateRefs、ResultTypes、Sink、结构化ParentElaborationContext、RequiredCapabilities、DependencyPlanNodes、CanonicalWorkKey和Origin完整；Parent context的parent key/instance/decorator/path四项preimage均可从artifact/template context重建，context digest与CanonicalWorkKey逐级重算；PlanResult index存在且ExpectedType等于producer ResultTypes项；dependency vector只等于Inputs内eager PlanResult边并按canonical key排序且无环；
- TemplateRefs/captures保持lazy；每个capture具有闭合TaggedCaptureSource并通过source/category/type/access/scope检查，PlanResult capture可机械定位producer/index。select/expand只在实际选中arm/iteration后解析capture并创建child work item，PlanResult source只进入child direct dependency；未选template不得建立capture dependency、调度producer或触发effect，序列化parent Inputs不得含TemplateCapture；
- deferred template 通过稳定 TemplateId 引用真实 source、normalized HIR、完整VisibleBindings、重算的lexical environment key 和 origin；每个 template 具有non-empty、可从`SourceDeclarationKey`准确解析且同declaration唯一的`TemplateRolePath`，step只使用语法registry的owner-kind/child-role/source-backed ordinal；SourceDeclarationKey、declaration/binding path与RegionPath都按10 §2.1的闭合step schema准确消费，禁止opaque别名bytes；
- `NormalizedHirSchemaVersion=1` 的payload按闭合V1 schema准确消费：node length/tag/tag-selected fields/origin完整，child-before-parent、root最后、每个非root恰被引用一次，无环、无共享、无不可达node；unknown tag、非规范optional、尾随bytes和任何独立decoder budget超限都拒绝；
- HIR中的TemplateId只引用当前`TemplateRolePath`的严格source-backed descendant，全部template-reference edge形成DAG；type/constant/string/symbol/origin引用的range与类别准确，semantic digest按canonical tree projection和外部canonical identity重算而非直接hash artifact-local bytes；
- 未选择 template 不被伪装成已绑定 typed CFG；
- template capture 显式列出实际source，不含裸 C++ pointer、临时 arena handle、未解析canonical key或无法定位的lazy producer；
- `stage.force_value` 的选中表达式必须产生唯一 typed value，最终不允许 Residual；
- `stage.force_block` 整体强制执行且不允许残留 runtime operation；return/break/continue不得把控制转移到模板或模板内新activation之外；
- `stage.select_if/match` 的 selector 必须在对应 boundary Known；
- `stage.expand_for` 的 iterable/length Known，每轮按准确 element type 重新 elaboration；
- `stage.expand_while` 每轮 condition Known，并受 fuel/expansion budget；
- `stage.instantiate` 使用闭合 canonical comptime arguments；
- SinkKind 与 Value、Statement、TopLevel、ClassMember、InterfaceMember、EnumMember、ModuleRegistration 的 producer/context相符；
- `ct.register_module_item` 只出现在 Staged typed Core 的 active decorator application context，operand schema为scalar/unit T或准确的 `!place<ro,T>`；`!place<rw,T>` 必须先显式降权，source-backed callsite key来自compiler；可达执行要求完整Known且不能residualize，具有`OrderedSemanticEmit`并恰好向当前pending batch产生一条record；
- 每条已提交 registration 的非空奇数长度 `DecoratorApplicationOrderPath`、structured DynamicControlPath、source-order invocation/iteration ordinal、application-wide连续`EmissionOrdinal`和RegistrationIdentity均可从module/source/application context重算；root/source traversal、generated-child semantic-output path 与cache replay一致，不含round/thread/commit顺序；records严格按order path、emission ordinal、identity排序且不存在重复identity或重复order slot；只有writer前完整logical record连同canonical Origin相同的pending replay才可去重；
- registration type/value满足中央`StaticRegistrationEncodable`，frozen String/aggregate与symbol relocation准确；function relocation只到stable entry，version-local body、host/resource/noescape/runtime handle/object和需要用户install/remove代码的encoding拒绝；
- active DAG 冻结后不得通过任何 template/effect 产生 import 或 module dependency；
- import selection/fixed point开始前已经冻结只读`BuildInputSnapshot`；全部可缓存tracked-read handler只能查询该snapshot并把准确entry identity/digest/range写入DependencyManifest，live-host bypass或无法稳定验证的读取不得伪装成tracked；
- revision 1任一实际untracked read（包括network/process/clock/random）都使module不具备可验证的版本身份，`closeAndVerify`必须拒绝产生可序列化、可装载或可hot-reload publish的Closed artifact；仅标noncacheable或不发布FinalModuleKey不能放行；
- host write只能进入与snapshot隔离的`BuildOutputNamespace`；当前build计划/正在写/已写的规范identity及其alias都不得作为tracked或untracked read来源，output不注入同一build后续round；
- meta value、declaration handle、host capability 和 comptime pointer 不进入普通 runtime store、return、function pointer或 extern ABI；
- effectful work item 的保守upper bound在调度前决定ordered lane；实际host write/build.log使整个module不可缓存且不replay，unknown/indirect summary按可能host I/O处理；只允许Pure、TargetContext-only或snapshot tracked-read work item并行。

## 9. 第 6 层：固定点事务验证

每轮 elaboration/partial-evaluation fixed point 必须满足：

- 只读取本轮开始时的稳定 declaration snapshot；
- 当前轮外部读取只来自同一个冻结`BuildInputSnapshot`，不会观察当前build的`BuildOutputNamespace`；
- 本轮生成声明、Core IR和registration records写入 pending transaction，不在同轮被其他 work item 观察；
- generated identity 使用 source declaration、closed arguments、canonical control path、loop ordinal/key/value 和 TargetKey，不使用 round 或调度顺序；
- pending batch 在 commit 前完成名称绑定、访问检查、类型检查、布局检查、IR验证和registration frozen constant/relocation验证；
- 整批成功才原子提交，失败不得留下部分声明、registration或cache record；
- 新提交内容只从下一轮可见；
- 重定义、布局环、未完成反射、进行中实例递归和预算耗尽具有不同诊断类别；
- fixed point 终止时没有未解决的 mandatory request或待验证/提交registration。

## 10. 第 7 层：Closed verifier

Closed verifier 必须拒绝：

- `stage.*`、`ct.register_module_item`、ElaborationPlan、NormalizedTemplateTable 和 deferred region；
- open generic entity/template、未闭合pack、dependent Type record、meta value 或 unresolved overload；唯一例外是10 §9.2中被closed generic_instance引用的private source-backed GenericDeclarationProvenance Symbol snapshot，它只内嵌exact-decodable GenericDeclarationSignatureSurfaceVector而没有entity/body/template，且不可lookup、call或export；
- 未确定布局、未完成 declaration、host pointer、compiler handle 或 virtual comptime address；
- 不可 runtime-representable 的 function signature、global、constant 或 captured value；
- 未闭合 direct call 或不准确 indirect function type；
- 强制 comptime boundary 的 residual；
- 没有 RuntimeWorld handler/LLVM lowering 的 effect；
- 缺少 TargetKey、PDB revision、nan mode、layout ABI hash 或 RuntimeABI revision，或required record/identity/Template/Plan/registration/sealed closure内任一InstanceIdentityPayload.TargetKey、ExpansionIdentity.TargetKey、target_blob.TargetKey不等于当前Manifest.target_key；
- 与当前 TargetContext 不一致的 target constant/layout，或dependency镜像identity的TargetKey不同时等于consumer与binding provider的Manifest.target_key；
- 任何非零 address space type、constant 或 operation；
- 未注册heap/arena/gc storage或operation，以及canonical Core中伪造的DynamicRef type/value；
- may-unwind call 缺少 unwind edge；
- address-only/noncopyable result 通过普通 SSA return；
- stable logical declaration、StableEntry mapping与version-local body的FunctionType/TargetAbiTag/ABI/contract不一致，或version-local generated parent缺少/错用当前`ModuleVersionContextKey`；
- async virtual/interface slot 的 Task logical result 或同步 override result 不完全一致。

Closed verifier 还必须从 GlobalRecords 重建唯一 `ModuleInitializationPlan` 并核对 stable-Kahn ordinal/dependency、finalizer-only step、process/TLS policy与lifecycle entry；所有process/TLS initializer与finalizer必须sync/void/nothrow且只服务一个Global，thread-local只能使用`eager_thread_activation`，任何lifecycle Function/entry不得具有Ink unwind successor；runtime/embedding失败才进入Failed并reverse cleanup，`mem.global_place`不得承担隐式初始化effect。每个lifecycle helper及其generated identity、entry hidden owner、global generation和cleanup state还必须解析到同一`ModuleVersionContextKey`与version owner，禁止V1 cleanup调用V2 helper。

Closed verifier 同时必须逐条验证保留的 `TypedRegistrations`（artifact `ModuleRegistrations` section）：section summary/interface/set digest重算一致；item按 `(DecoratorApplicationOrderPath, EmissionOrdinal, RegistrationIdentity)` 严格排序且identity/order slot唯一，每个application的ordinal连续；DynamicControlPath、producer/application/callsite identity和完整record schema均准确；type与constant准确匹配并满足当前TargetKey/RuntimeABI/registration encoding revision的`StaticRegistrationEncodable`；所有 relocation获准；record本身immutable且安装/撤销不调用用户代码。被provenance引用的DecoratorDeclarationSymbol必须保留中央规定的bodyless、不可执行Closed decorator Function declaration以重算signature/SymbolKey，不能保留Staged body或只留裸Symbol。record identity/value set可以变化，但从当前records提取的unique schema/interface digest属于v0 hot-reload compatibility边界。

## 11. Runtime 与 private ABI 验证

semantic verifier 不冻结 opaque runtime bytes，但必须检查逻辑契约：

- class dynamic dispatch 使用 canonical complete object pointer 和准确 adjustment；
- interface value 逻辑上是 `{complete_object, interface_table}` 两字表示；
- dynamic destroy 与 storage deallocation 分离且 nothrow；
- reflection handle 固定 version snapshot，member handle 不超过 snapshot 生命周期；reflect.lookup_type/interface的P22与lookup_function的P23必须以canonical module identity `Str`编码并按UTF-8 bytes核对，禁止旧的module SymbolId/`@M` payload；Closed verifier还必须按10 §9.5重建ReflectionDescriptorIdentityTuple与全部顶层/member name index，拒绝VersionPolicy制造的重复descriptor、lookup多候选以及P24无法区分的同owner同名同member-kind重载；
- dependency import root自身的ClosedGenericArguments无论private_import/reexport都走public semantic closure：builtin/结构/literal/nested selector按自身规则闭合，nonbuiltin top-level entity在同一direct provider具有普通binding；ProviderSealedDeclarationIdentityClosure只从root.DeclarationIdentity/GenericDeclarationProvenance及其内部identity argument reference展开，在builtin或已由同provider binding/owner projection证明的public boundary停止。private generic parent marker及其generic-signature semantic leaf逐record匹配同一provider并独立重算，但identity-only node没有binding、lookup、call、reflection、registration、relocation或re-export权限；
- `rt.version.current_owner` 只在 version-local 或继承已 pin activation 的 compiler-generated helper entry block 物化 borrowed nonescaping owner；其他 region 不得产生/伪造 `version_owner`，`rt.version.pin` 只能接受该来源或 schema 明确转发的同一 capability；
- reflection adapter只从expected signature规定的typed operand临时形成private DynamicRef，layout version/generation与snapshot兼容；private descriptor不能进入Core type/value、CFG、frame、constant或serialization；
- async call 的同步 construction 与 Task body failure 是两个独立契约维度，canonical current bits 分别使用正向 `construction_nothrow` 与 `body_no_fail`；
- async declaration的当前BehaviorContracts只使用正向`construction_nothrow`与`body_no_fail`，nothrow bit必须为零；源码`[nothrow] async`只设置body_no_fail，不得错误设置construction_nothrow；固定StableEffectEnvelope仍分别保留ConstructionMayUnwind/BodyMayFail upper-bound baseline，不能与当前contract bit混同；
- async declaration logical result T、Task type、construction call、body entry、publish 与 await 的T精确一致，并满足v0 Task result admissibility；`Task<noncopyable-address-only>`和含no-escape value的Task在形成Closed type/function前拒绝；
- async body/generated resume entry 的 `task_self` 是当前 Alive Task 的 nonescaping borrowed rw capability，publish/drive schema 的 Task operand 必须追溯到它或 schema-preserving phi；address-only logical T 的 `task_result_storage` 是唯一 runtime-derived owner init capability，entry 无 transaction，success 必须存在 fresh begin/commit/as_alive/publish 链，failure publication 前必须已 cleanup/rollback 或从未 begin；两者都不得由 sealed Task projection伪造、转成 raw address或逃逸；
- Task result destination、Task-specific commit、状态转换和repeated await类型准确；每个Task generation携带可重建的TaskBodyEffect/TaskDestroySummary side facts，consumer并入准确summary或最大保守上界；
- failed Task与每个failed-await ActiveUnwindRecord分别持有ExceptionBox强引用，eh.end_catch/task destroy只释放自身引用，payload/cause/pins仅由最后owner销毁一次；
- destructor_body闭合矩阵、object parameter owner handoff、dynamic destroy summary/version selection均满足06的runtime ABI契约；
- repr(C) definition满足三参`CRepresentationSafe(T, TargetAbiTag, TargetKey)`；extern C signature逐位置满足四参`CAbiSafe(T, TargetAbiTag, TargetKey, CAbiPositionPayload)`，TargetAbiTag注册mapping、Root/RootOrdinal/AggregateFieldPath与classification逐项重算；
- AbiImport保存ForeignFailurePolicyPayload，AbiExport保存ExportExceptionPolicyPayload，AbiBridge按direction保存TaggedBoundaryPolicyPayload并核对ExternalTarget/direct-indirect矩阵；三类policy不得互换。ExternalToInk bridge的ExternalTarget/TrustedExternalEffects必须absent并与AbiExport policy相等，boundary records只能由backend声明式合成wrapper而不得拥有Function extern_bridge；
- catch_and_callback的CallbackFunctionType必须满足唯一CFunctionType矩阵并准确等于`(pointer<ro,ExceptionInfoType>) -> void`；CallbackSymbol只允许解析到CFunctionTypeId逐项相等且ForeignFailurePolicyPayload=nothrow的AbiImport。普通Ink Function、logical stable/StableEntry或EntryIdentity=extern declaration直接充当C callback均拒绝；Ink callable若参与协议必须另经ExternalTarget绑定该AbiImport且双signature/target/policy逐项匹配的InkToExternal bridge，不能把Ink callable identity或地址塞入CallbackSymbol；
- hot reload 只接受 ABI/layout hash 相同的 code replacement；
- hot reload候选重算相等的module-registration interface/schema digest，允许set digest变化但必须把新records与stable entries/reflection/globals原子publish；旧record只由匹配version owner与RegistrationIdentity撤销，不能按业务键误删新版本；
- hot-reload compatibility lineage 保存首次基线的完整 canonical `StableEffectEnvelope` 与固定 hash；当前发布记录另存 `BehaviorContracts` 与 digest，sync nothrow、construction nounwind 和 body no-fail 只能单调加强，后续版本不得减弱；新 exact body 分别不得超过对应 envelope，也必须满足当前 contract；依赖/cache identity 必须覆盖其实际使用的 contract digest；
- raw borrow 不拥有 module pin，loader unload 需要外部 quiescence 证明。

TargetABI lowering 后的 LLVM verifier 不能替代这些检查。

## 12. Pass 分类

### 12.1 Staged passes

建议顺序：

```text
ParseImportSelectionSources
-> CollectStaticCandidateImports
-> ExecuteRestrictedImportGuards
-> NormalizeActiveModulePaths
-> FormAndVerifyActiveModuleDAG
-> FreezeActiveModuleGraph
-> BuildStagedModule
-> VerifyStaged
-> CanonicalizeTypedCore
-> ElaborationPartialEvaluationFixedPoint
-> CloseModule
-> LowerStructuredCleanup
-> VerifyClosed
```

pre-Staged profile state 不是 Staged artifact，不能经过 `VerifyStaged`、写入 Staged cache 或冒充具有完整 TargetKey 的 StagedModule。只有 active module DAG 冻结后才能建立并验证 StagedModule。`CanonicalizeTypedCore` 不得进入 deferred template 并提前绑定未选分支。

### 12.2 Closed semantic passes

可以包括：

- constant folding with TargetContext；
- CFG simplification 和 unreachable elimination；
- block-argument simplification；
- dead pure operation elimination；
- copy propagation 与 scalar replacement；
- escape analysis 与 stack promotion；
- bounds-check elimination；
- devirtualization；
- typed copy/construct/destroy specialization；
- runtime semantic op lowering；
- stable-entry elimination when hot reload disabled。

### 12.3 Target lowering passes

可以包括：

- address-only/result destination 到具体 TargetABI parameter；
- aggregate/class/enum/interface layout materialization；
- exception ABI lowering；
- Task/reflection/hot runtime intrinsic lowering；
- readonly frozen registration data、受控relocation与loader descriptor lowering，不生成用户init/fini callback；
- PDB guard 或 target instruction selection；
- LLVM type/function/global 构造。

## 13. Pass 合法性

每个 pass 必须声明：

```text
AcceptedArtifactKind
RequiredInvariants
PreservedInvariants
MayCreateOrDeleteSymbols
MayChangeCFG
MayChangeEffects
RequiresTargetContext
DeterminismClass
```

以下优化默认非法，除非有额外证明：

- speculative执行MayTrap、PdbBoundary、MayUnwind或observable effect operation；仅有TargetDependent且其他effect允许的operation不因此被禁止；
- 把 strict FP operation 改成带额外 fast flag；
- 把 `[fast_math]` 映射成包含 `nnan/ninf/approx` 的 LLVM `fast`；
- 给普通 pointer/reference/const/slice 自动添加 noalias；
- 用 `memcpy` 替换 nontrivial typed copy；
- 删除 destructor、defer、dynamic destroy 或 version pin；
- 把 trap 改为异常，或让 trap 路径执行 cleanup；
- 跨 stable entry 内联 version-local body而破坏热更新边界；
- 依据当前 stable body 的较弱 summary 删除、CSE、推测、复制或跨越 publish/pin/EH 边界移动 `RuntimeEffect(version_select)`，或把 stable call effect 收窄到 `StableEffectEnvelope` 以下；
- 让 pinned reflection adapter 跳转到当前最新版本；
- 依据 fixed-point 调度顺序选择 symbol ID 或 declaration identity。

## 14. 确定性和 canonicalization

pass manager 必须为同一语义输入产生相同 canonical output：

- worklist 使用规范 symbol/block/operation key；
- 并行 pass 的提交阶段按 canonical key 排序；
- uniquing table 的地址和插入竞争不进入 ID；
- APInt/float/attribute 按唯一 canonical encoding；
- unordered metadata 在打印和 hash 前排序；
- diagnostic primary/related origin 选择规则固定；
- semantic hash 排除 debug/origin 展示差异，但包含所有会改变执行语义的 attribute、TargetKey 和 handler revision。

## 15. 失败分类

验证失败按来源分类：

- 编译器刚生成的 IR 失败：compiler bug，输出最小 IR slice 和 origin；
- 本地缓存结构或语义失败：cache corruption/invalidation，删除该 entry 后重新构建；
- source-backed staged template elaboration 失败：正常源码诊断；
- capability/limit 失败：编译环境或资源诊断；
- runtime embedding ABI/hash 失败：loader/runtime compatibility error；
- LLVM verifier 失败：backend bug。

不得把任意内部 verifier 消息原样伪装成用户源码错误。

## 16. 测试要求

最低测试集合：

- 每个 opcode 的正例、operand/result/attribute/type 负例；
- CFG、dominance、block argument 和 nested region 负例；
- place/lifetime/partial construction/unwind 的 path-sensitive 测试；
- Staged 到 Closed 的强制消除测试；
- binary decoder fuzz、长度/索引/深度/分配预算测试；
- encode/decode semantic round-trip；
- canonical text golden 和并行确定性测试；
- Closed interpreter 与 LLVM AOT 的差分测试；
- 不同 TargetKey 下 PDB、pointer width、endianness、layout 和 nan mode 测试；
- hot reload old/new pin、reflection snapshot 和 Task version 测试；
- optimizer 不能错误 DCE/hoist trap、PDB、unwind、cleanup 和 effects 的回归测试。
