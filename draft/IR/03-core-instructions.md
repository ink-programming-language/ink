# InkIR 核心指令与规范 opcode 注册表

## 1. 范围与设计原则

本文规定 typed Core InkIR 的结构实体、规范 opcode、操作数与结果形状、效果、阶段合法性及局部 verifier 规则。执行顺序和抽象机状态见 [`04-execution-model.md`](./04-execution-model.md)，类型、place、对象生命周期和目标内存前置条件见 [`02-types-values-memory.md`](./02-types-values-memory.md)，调用约定和运行时对象见 [`06-calling-convention-runtime.md`](./06-calling-convention-runtime.md)。

Core InkIR 只保留一种运行时/编译期共用的强类型指令语义。ComptimeWorld、ResidualizeWorld 和 RuntimeWorld 可以提供不同 value domain 与 effect handler，但不得为同一 opcode 发明第二套数值或内存语义。

本注册表遵守以下原则：

- opcode 必须带稳定命名空间；无命名空间的 `add`、`load`、`call`、`invoke`、`destroy` 不是合法规范拼写；
- SSA 只承载单标量值、专用 unit 值和 TargetABI 明确为单标量的无载荷 enum；
- slice、interface reference、非空 tuple、array、class、带载荷 enum 和其他聚合是 address-only，通过 place 与 caller destination 操作；
- Core InkIR 没有 `phi`、`undef`、`poison`、通用 move 或隐式 fallthrough；
- operation 的 effect、是否可 unwind、是否可 trap、阶段和 speculatability 由 opcode schema 与已解析 callee 契约共同推导，序列化输入不得覆盖；
- 每条 operation 必须携带非空 `OriginId`，规范文本用 `loc(#oN)` 表示。

## 2. 共同记录与文本约定

### 2.1 Operation 共同形状

本文使用下列概念文本形状：

```text
%result = namespace.opcode %operand0, %operand1 {payload_field = value} : (operand-types) -> result-type loc(#o12)
namespace.terminator ... successor ^bb1(...) loc(#o13)
```

零结果 operation 省略等号左侧；多结果 operation 使用括号结果列表。v0 operation花括号只打印中央PayloadSchema的typed字段，`OperationAttributeCount`固定为0；payload字段顺序、类型表引用和二进制字段编号由文本/二进制格式文档固定。

本文表格和代码块统一是 noncanonical schema/CFG pseudocode：`(T, T) -> T`、`ptr<T>`、`slice<T>`、命名式 `!tName`、说明性 payload、省略字段及为展示 successor 而换行都只表达语义，不可直接交给 parser。逐 opcode 的准确单行格式见 [`11-instruction-reference.md`](./11-instruction-reference.md)，最终 canonical renderer 与 envelope 见 10 §15.5 和 [`08-text-binary-format.md`](./08-text-binary-format.md)；二者不允许上述省略。普通复合 type use 的真实文本为 `!tN`；只有文本格式明确列出的 place/exception/runtime-handle/runtime-object 使用 inline special spelling，且仍匹配 TypeTable record。

schema 中 raw pointer、reference、slice 和 interface-reference 的 `A` 属于 `ValueAccess = ro|rw`；place 的 `A` 属于 `PlaceAccess = init|ro|rw`。`PlaceAccess.init` 只能由具有构造后置条件的 place-producing schema 产生，不是可复用于普通 value type 的第三种 access。parser、registry 与 verifier 必须按字段所在 type kind 选择准确 enum domain，不能用一个包含 `init` 的宽枚举同时接受两类字段。

除 opcode 专门允许外，verifier 对每条 operation 都检查：

- operand 数量、顺序和准确类型匹配 schema；
- SSA 定义支配使用，block edge 实参与 block argument 一一匹配；
- result 只能属于 schema 允许的单标量、unit、内部 token 或 place capability；
- place 的 element type、访问能力、alignment、address space 与生命周期事实满足操作要求；
- symbol、type、constant 和 attribute 引用存在且种类准确；
- `loc(#oN)` 指向有效 provenance DAG 节点；
- 当前模块阶段与 TargetKey 满足 schema。

### 2.2 阶段标记

本文表格使用：

| 标记 | 含义 |
| --- | --- |
| `SC` | 可出现在 StagedModule 的 typed Core 与 ClosedModule |
| `S` | 只能出现在 StagedModule，elaboration 后必须消失 |
| `C` | 只能出现在 ClosedModule |

一个 `SC` opcode 不等于它必定能在 ComptimeWorld 中具体执行。若其 effect 没有 comptime capability/handler，ResidualizeWorld 可以残留该 operation；强制 comptime boundary 则报告编译错误。

### 2.3 Effect 记法

表格中的效果名对应架构文档定义：

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
TargetDependent
PdbBoundary
ComptimeEffect(capability)
RuntimeEffect(handler)
Control
DeclarationSink
```

`TargetDependent` 表示 operation 的值、地址或布局依赖当前 TargetContext；在 TargetKey 已固定且没有其他阻碍时，它仍可 fold、CSE 或推测执行。`PdbBoundary` 只标记目标规则在正常输入域之外选择具体 value 或 trap 的真正 platform-dependent behavior 边界；它必须同时具有 `TargetDependent`，并在可能 trap 时具有 `MayTrap`，不可推测、复制、CSE、删除或跨改变其动态执行性的控制边界移动。`MayDiverge` 表示 operation 或已解析 callee 可能永不产生 normal、unwind 或 trap completion，例如无限执行、挂起等待或 fatal；普通 trap 由 `MayTrap` 单独表示，不自动蕴含 `MayDiverge`。call 与 dynamic destroy 的有效效果是 opcode 固有效果和 verifier 从已解析 callable/runtime schema 推导的效果摘要之并集。

## 3. Module、symbol 与 constant

### 3.1 结构实体不是可执行 opcode

Module、Symbol、TypeTable 和 ConstantTable 是 IR 容器实体，不按 CFG 顺序执行。下表只把概念映射到 10 章定义的真实 record/table；左列不是可序列化 opcode、record kind 名或 canonical text：

| 记录种类 | 主要内容 | 阶段 |
| --- | --- | --- |
| Header + Manifest | schema version、module identity、dependency digest、完整 TargetKey | `SC` |
| Type + Symbol | 名义类型身份、属性、layout contract、成员引用 | `SC` |
| Function | 逻辑签名、body region 或外部声明、调用契约、effect summary | `SC` |
| Global | 类型、storage/linkage、initializer/finalizer、初始化 policy/ordinal/dependencies、版本身份 | `SC` |
| constant Global | 命名常量统一使用`SymbolKind=global`的Global definition，携带准确类型与ConstantTable initializer；`Mutability=constant`且无dynamic lifecycle | `SC` |
| RuntimeDeclaration | runtime handler/schema identity 与 TargetABI revision | `SC` |
| ModuleRegistration | frozen typed constant、producer/application/control identity 与 protocol schema digest | `SC` |

每个 symbol 具有非空 origin。实体的 exact required payload、record kind 与 canonical printer 由 10 §9.2—9.5、§10.4 和 §15.3—15.4 唯一规定；不得从概念表补造 `sym.*` 文本形式。

symbol 的可读名称不是完整身份。反序列化时必须按 canonical module identity、声明种类、闭合签名和 generic arguments 重建 `SymbolKey`，不能信任宿主地址或 arena index。

### 3.2 Function body entry envelope

函数逻辑签名只描述 receiver、源码可见参数和逻辑结果；Core CFG 的 entry block 还显式承载由调用边界或 runtime 提供的隐藏通道。每个 entry block argument 都具有 registry 定义的 `BlockArgumentRoleTag` 与 role-local index，并严格按下列顺序出现：

1. 存在 receiver 时，一个 `receiver`；
2. 按 `ordered_parameters` 顺序出现全部 `parameter`，其中 address-only by-value 参数仍是准确参数对象 place；
3. GlobalRecord 指定的 initializer/finalizer function 有一个 `global_lifecycle`；
4. sync 普通函数的 `result_passing_mode = result_destination` 时，一个 `result_destination`；
5. async body/generated resume entry 固定有一个 `task_self`；
6. async body 的 logical result `T` 为 address-only 时，再有一个 `task_result_storage`。

这些隐藏 block argument 不进入源码参数列表或 `FunctionSignature.ordered_parameters`，但由 Function record、文本和 binary 的 entry-envelope schema 唯一推导，不能靠扫描 uses 猜测。constructor 的 initializing receiver 使用唯一 `receiver` argument，类型为携带 caller active transaction 的 `!place<init,C>`；constructor 的逻辑结果为 `void`，没有额外 `result_destination`。destructor receiver、普通实例 receiver 和所有参数的准确 access/type/authority 由其 passing mode 决定。

普通 parameter 的 Core entry 类型必须由逻辑 parameter-passing mode 唯一推导：`value T` 为准确 scalar/unit SSA `T`，`object T` 为 Alive owner `!place<rw,T>`，`const_reference T` 为 `ref<ro,T>`，`mutable_reference T` 为 `ref<rw,T>`，`raw_pointer ptr<A,T,S>` 为同一准确 raw-pointer 类型。`object` entry place 表示已经完成 activation handoff 的独立参数对象；callee 持有其唯一 cleanup obligation，不能把它降成 borrow 后遗失 obligation，也不能与 caller 的源对象或 prepared storage 共享对象身份。registry、binary 与 verifier 必须使用这一映射，不能只比较逻辑 T 而接受任意物理 block-argument 类型。

`global_lifecycle` 只由 module runtime 向准确 GlobalRecord 的唯一 initializer 或 finalizer activation提供：initializer 类型为 owner `!place<init,T>`，finalizer 为 owner `!place<ro/rw,T>`，并与当前 module instance/version（thread-local 还与 thread identity）绑定。v0 一个 lifecycle function 只服务一个 global，不能被多个 GlobalRecord 复用或兼任 init/finalize；普通调用边、branch 与 `mem.global_place` 都不能产生该角色。所有process/TLS initializer与finalizer固定sync/void/nothrow；违反该contract进入fatal，不能产生或泄漏exception token越过lifecycle boundary。runtime/embedding级安装失败仍可在逆序清理AlivePrefix后把版本置为Failed，但不是Ink unwind。GlobalRecord 的 initialization policy、ordinal 和 dependencies 必须能唯一重建执行模型规定的 `ModuleInitializationPlan`；thread-local global 的 policy 固定为 `eager_thread_activation`，并在线程进入或切换 module version 时由 runtime 急切执行，`mem.global_place` 只查询已经 Alive 的 TLS object。sync `result_destination` 是 caller 传入的 owner-authority `!place<init,T>`，entry 时已经携带唯一 active `TransactionId`；callee 不得重复 `obj.init.begin`，最终完成者在无值 `cf.return` 前 commit，中间 forwarding callee 只传播 inner call 的 normal/unwind 后置条件。async `task_self` 是 runtime 从当前 Alive Task 物化的 `!place<rw,!runtime-object<task,T>>`，其 `LifetimeAuthority = borrow`；角色名不授予销毁、重初始化或 deallocation 权限。它不可逃逸、复制、转为 raw address 或经普通 sealed-storage operation 访问，只能供允许该角色的 async runtime schema 使用；跨暂停时它作为 frame-internal logical capability spill。

async address-only `task_result_storage` 是 runtime 从 sealed Task frame 合法派生、但无法由普通 `place.field` 伪造的 owner-authority `!place<init,T>`。entry 时 storage 为 `AllocatedUninitialized` 且没有 active transaction；进入最终 return-result 构造时 body 才执行 fresh `obj.init.begin`。构造可以连同 transaction/cleanup state 跨暂停；成功路径完成构造、`obj.init.commit`、`place.as_alive` 为 readonly capability，再以 `async.task.publish_success` 发布。失败路径若已经 begin，必须先逆序清理并 rollback；`async.task.publish_failure` 前 result storage 必须回到 `AllocatedUninitialized`。该 capability 不可逃逸、返回、存储到用户对象、转为 raw address、释放底层 storage 或用于结果之外的对象。

非 entry 普通 CFG block argument 的 role 固定为 `phi`；nested region 自身的 entry arguments 使用 `region_argument`，除非拥有该 region 的 opcode schema 显式规定更窄角色。branch edge 不得产生或伪造 `receiver`、`global_lifecycle`、`result_destination`、`task_self` 或 `task_result_storage`。

### 3.3 标量常量 operation

| Opcode | 形状 | Effect | 阶段 | 关键 verifier 规则 |
| --- | --- | --- | --- | --- |
| `const.int` | `%r = const.int {constant = #cN} : () -> iN/uN/ptrsize` | `Pure` | `SC` | Constant的APInt bit数与目标类型位宽一致；有符号十进制只作注释 |
| `const.bool` | `%r = const.bool {constant = #cN} : () -> bool` | `Pure` | `SC` | Constant准确为两个bool语义值之一 |
| `const.float` | `%r = const.float {constant = #cN} : () -> f16/f32/f64` | `Pure` | `SC` | Constant必须保留负零、Infinity、NaN payload与sNaN bits |
| `const.null` | `%r = const.null {constant = #cN} : () -> ptr<..., T, A>` | `Pure` | `SC` | Constant类型准确；只产生raw pointer，不产生reference/place |
| `const.unit` | `%r = const.unit {constant = #cN} : () -> unit` | `Pure` | `SC` | Constant为builtin `unit`的唯一零载荷值，不允许换成`void`或零元tuple record |
| `const.symbol_addr` | `%r = const.symbol_addr {symbol = @s, addend = N} : () -> ptr<..., T, A>` | `Pure`, `TargetDependent` | `SC` | symbol可重定位且address space/目标宽度匹配 |
| `const.function` | `%r = const.function {symbol = @f} : () -> function-pointer<signature>` | `Pure`, `TargetDependent` | `SC` | signature精确匹配；hot reload下取得stable entry，不泄露version-local body |

示例：

```text
%one = const.int {constant = #c10} : () -> i64 loc(#o10)
%neg_zero = const.float {constant = #c11} : () -> f64 loc(#o11)
%null = const.null {constant = #c12} : () -> !t7 loc(#o12)
```

ConstantTable 可以保存用于 global initializer 的 address-only 常量对象描述，但不存在 `const.aggregate` SSA instruction。常量对象必须直接初始化到最终 storage；其 padding 不是语义常量且默认不初始化。

## 4. 控制流 `cf.*`

### 4.1 分支与选择

| Opcode | 形状 | Effect | 阶段 | 关键 verifier 规则 |
| --- | --- | --- | --- | --- |
| `cf.br` | `cf.br ^dest(%args...)` | `Control` | `SC` | edge 实参与目标 block arguments 类型完全一致 |
| `cf.cond_br` | `cf.cond_br %cond, ^yes(%a...), ^no(%b...)` | `Control` | `SC` | `%cond` 必须是 `bool`；两个 successor 都显式存在 |
| `cf.switch` | `cf.switch %key [case K -> ^bb(...), ...] default ^def(...)` | `Control` | `SC` | key 为整数或规范 enum discriminant；case 不重复且顺序规范化；default successor 无条件必需，即使 case 穷尽全部位模式 |
| `cf.select` | `%r = cf.select %cond, %yes, %no : T` | `Pure` | `SC` | `T` 只能是同一单标量类型或 unit；两个 value 已在 select 前求值 |

`cf.cond_br` 不提供短路求值。源码短路、条件表达式或只在一个分支执行的 effect 必须用 CFG block 表示。

```text
cf.cond_br %ready, ^bb1(%value), ^bb2(%fallback) : (bool) -> () loc(#o20)

^bb3(%merged: i64 origin(#o21)) origin(#o22):
  cf.return %merged : (i64) -> () loc(#o23)
```

### 4.2 返回与不可到达

| Opcode | 形状 | Effect | 阶段 | 关键 verifier 规则 |
| --- | --- | --- | --- | --- |
| `cf.return` | `cf.return` 或 `cf.return %value : T` | `Control` | `SC` | `void`/destination-result 无 operand；单标量或 unit 结果恰有一个准确类型 operand；`never` 函数不可到达 return |
| `cf.unreachable` | `cf.unreachable` | `Control` | `SC` | 只能由 verifier 可证明无运行时到达路径或前置 terminator 结构产生 |

`cf.unreachable` 不是 trap。若它在 Verified IR 的执行中被到达，表示 IR、backend 或 verifier 缺陷，不属于 Ink 程序的语言结果。

sync destination-result 的 `cf.return` 还要求 entry `result_destination` 已由当前最终完成者 commit，或已由被转发调用的 normal postcondition证明 commit；它不携带 `T`。async body 不用 `cf.return` 发布逻辑结果：void/unit/scalar 分别使用对应 `async.task.publish_success` variant，address-only `T` 使用 entry `task_result_storage` 完成构造后发布；失败边界使用 `async.task.publish_failure`。

## 5. 整数、位运算与 PDB `arith.*` / `pdb.*`

### 5.1 回绕算术与位运算

| Opcode | Operand/result | Effect | 阶段 | 语义与 verifier |
| --- | --- | --- | --- | --- |
| `arith.add` | `(T, T) -> T` | `Pure` | `SC` | `T` 为同一整数类型；模 `2^N` 回绕 |
| `arith.sub` | `(T, T) -> T` | `Pure` | `SC` | 同一整数类型；左操作数减右操作数，结果按位宽执行 mod 2^N |
| `arith.mul` | `(T, T) -> T` | `Pure` | `SC` | 同一整数类型；保留完整乘积的低 N 位，即 mod 2^N |
| `arith.neg` | `(T) -> T` | `Pure` | `SC` | 模 `2^N` 加法逆元，无符号类型同样合法 |
| `arith.and` | `(T, T) -> T` | `Pure` | `SC` | 准确位模式按位与 |
| `arith.or` | `(T, T) -> T` | `Pure` | `SC` | 准确位模式按位或 |
| `arith.xor` | `(T, T) -> T` | `Pure` | `SC` | 准确位模式按位异或 |
| `arith.not` | `(T) -> T` | `Pure` | `SC` | 准确位模式按位取反 |
| `arith.cmp` | `(T, T) -> bool` | `Pure` | `SC` | 相同整数类型支持 `eq/ne/slt/sle/sgt/sge/ult/ule/ugt/uge`；`bool` 只支持 `eq/ne`，并可与 `const.bool false` 组合表达逻辑非；signed/unsigned predicate 与 operand 类型相容 |

普通整数指令没有可序列化的 `nsw` 或 `nuw`。后端只能在独立证明后对降低结果增加相应优化事实。

`bool` 不是整数类型，不能使用 `arith.add/and/not` 等整数 operation；它只复用 `arith.cmp` 的 `eq/ne` schema，逻辑非规范 lowering 为与 `const.bool false` 比较。源码 `&&` 和 `||` 的短路语义用 `cf.cond_br` 与 block argument 表示，不生成会提前求值右操作数的 eager boolean opcode。

```text
%sum = arith.add %lhs, %rhs : (i32, i32) -> i32 loc(#o30)
%less = arith.cmp %sum, %limit {predicate = slt} : (i32, i32) -> bool loc(#o31)
```

### 5.2 目标相关除法、余数与移位

| Opcode | Operand/result | Effect | 阶段 | 固定输入域语义 |
| --- | --- | --- | --- | --- |
| `pdb.sdiv` | `(iN, iN) -> iN` | `TargetDependent`, `PdbBoundary`, `MayTrap` | `SC` | 非零且非 `MIN/-1` 时向零截断 |
| `pdb.udiv` | `(uN, uN) -> uN` | `TargetDependent`, `PdbBoundary`, `MayTrap` | `SC` | 非零除数时无符号除法 |
| `pdb.srem` | `(iN, iN) -> iN` | `TargetDependent`, `PdbBoundary`, `MayTrap` | `SC` | 正常域余数与被除数同号或为零 |
| `pdb.urem` | `(uN, uN) -> uN` | `TargetDependent`, `PdbBoundary`, `MayTrap` | `SC` | 正常域无符号余数 |
| `pdb.shl` | `(T, ptrsize) -> T` | `TargetDependent`, `PdbBoundary`, `MayTrap` | `SC` | count 小于位宽时左移并丢弃高位 |
| `pdb.lshr` | `(uN/ptrsize, ptrsize) -> same` | `TargetDependent`, `PdbBoundary`, `MayTrap` | `SC` | count 小于位宽时高位补零 |
| `pdb.ashr` | `(iN, ptrsize) -> iN` | `TargetDependent`, `PdbBoundary`, `MayTrap` | `SC` | count 小于位宽时复制符号位 |

除零、`MIN/-1` 与超位宽 shift 的具体 value 或 trap 由当前 TargetContext 的 PDB table 决定。即使结果未使用，只要目标规则可能 trap，operation 也不能删除、推测执行或跨控制依赖移动。

```text
%q = pdb.sdiv %numerator, %denominator : (i64, i64) -> i64 loc(#o32)
%shifted = pdb.shl %bits, %count : (u32, ptrsize) -> u32 loc(#o33)
```

## 6. 浮点 `fp.*`

### 6.1 数值指令

| Opcode | Operand/result | Effect | 阶段 | 关键规则 |
| --- | --- | --- | --- | --- |
| `fp.add` | `(F, F) -> F` | `Pure`, `TargetDependent` | `SC` | strict 或逐条 fast flags |
| `fp.sub` | `(F, F) -> F` | `Pure`, `TargetDependent` | `SC` | 同一浮点格式；按 TargetContext strict 模式和已验证 fast flags 执行减法与舍入 |
| `fp.mul` | `(F, F) -> F` | `Pure`, `TargetDependent` | `SC` | 同一浮点格式；按 TargetContext strict 模式和已验证 fast flags 执行乘法与舍入 |
| `fp.div` | `(F, F) -> F` | `Pure`, `TargetDependent` | `SC` | 浮点除零不产生 Ink trap；结果遵守 IEEE 与 nan_mode |
| `fp.neg` | `(F) -> F` | `Pure` | `SC` | 翻转符号位，保留 payload |
| `fp.fma` | `(F, F, F) -> F` | `Pure`, `TargetDependent` | `SC` | 一次舍入；来自 explicit FMA 或合法 contract |
| `fp.cmp` | `(F, F) -> bool` | `Pure` | `SC` | predicate 显式为 `oeq/une/olt/ole/ogt/oge/ord/uno` |
| `fp.assume_finite` | `(F) -> F` | `Pure` | `SC` | 值不变；NaN/Infinity 输入违反显式 UB 契约；不得无证明插入或推测执行 |

`F` 为相同的 `f16`、`f32` 或 `f64`。strict operation 使用 round-to-nearest, ties-to-even、保留 subnormal、不 trap，并严格应用 `TargetContext.nan_mode`。这些 operation 只有 `TargetDependent`，不是 `PdbBoundary`，所以在相同 TargetKey 下仍可按普通 pure operation 推测执行。

每条数值 operation 的 fast flag 集只能是：

```text
reassociate
contract
no_signed_zero
flush_to_zero
denormals_are_zero
```

`approx_reciprocal`、`approx_function`、`nnan` 和 `ninf` 不是 v0 fast flag。`fp.assume_finite` 建立的有限性事实也不能反向授予其他 fast flag。

```text
%product = fp.mul %x, %y {fast = [contract]} : (f64, f64) -> f64 loc(#o40)
%sum = fp.add %product, %z {fast = [contract]} : (f64, f64) -> f64 loc(#o41)
```

若 optimizer 将上述两条合法收缩为 `fp.fma`，新 origin 必须通过 provenance DAG 指向两条源 operation，不能随意继承其中一条的位置。

## 7. 转换 `cast.*` 与 PDB 转换

| Opcode | Operand/result | Effect | 阶段 | 语义与 verifier |
| --- | --- | --- | --- | --- |
| `cast.int` | `(I) -> J` | `Pure` | `SC` | 扩宽按源 signedness 做 sign/zero extension；缩窄保留低位；同宽 signedness 转换保留 bits |
| `cast.int_to_float` | `(I) -> F` | `Pure` | `SC` | 按源数学值转换，round-to-nearest, ties-to-even |
| `cast.float` | `(F) -> G` | `Pure`, `TargetDependent` | `SC` | 扩宽/缩窄遵守 strict 舍入、subnormal 与 nan_mode |
| `pdb.fptosi` | `(F) -> iN` | `TargetDependent`, `PdbBoundary`, `MayTrap` | `SC` | 正常域向零截断；NaN、Infinity、越界按 PDB table |
| `pdb.fptoui` | `(F) -> uN/ptrsize` | `TargetDependent`, `PdbBoundary`, `MayTrap` | `SC` | 正常域向零截断到无符号范围；NaN、Infinity、负值与越界按 PDB table |
| `cast.bit` | `(S) -> D` | `Pure` | `SC` | 仅等位宽整数/`ptrsize`/浮点标量；原样保留 bits |
| `cast.ptr` | `raw-pointer <-> raw-pointer` 或 `ptrsize <-> raw-pointer` | `Pure` | `SC` | 默认 integral address space 中保留准确地址 bits；可显式改变 pointee type 和 ro/rw 路径能力；不产生 place/reference |
| `cast.ptr_access` | `ptr<rw,T,A> -> ptr<ro,T,A>` | `Pure` | `SC` | 只允许降低访问能力，不改变地址、T 或 address space |
| `cast.ref_access` | `ref<rw,T> -> ref<ro,T>` | `Pure` | `SC` | 只允许降低访问能力；reference 仍非空且非拥有 |

`bool` 不参加 `cast.int` 或 `cast.bit`；整数与 `bool` 之间必须由比较或明确的语言内建 lowering 表达。`cast.bit` 不能用于 raw/function pointer、place、聚合、reference、runtime handle 或 exception token。

`cast.ptr` 对 raw pointer 的显式 ro/rw 改变只改变当前访问路径，不证明底层 storage 可写。由 `ptr<ro,T,A>` 显式转换到 `ptr<rw,U,A>` 本身合法；之后写入仍必须满足 storage mutability、对象表示、alignment 与 lifetime 前置条件。v0 只支持同一默认 integral address space 内的 raw pointer 转换；pointee 可以是 `void`、`never`、函数、incomplete nominal 或 `runtime.opaque`，因为 cast 不读取 pointee layout。reference、safe slice、function pointer 和 interface reference 不参与。

```text
%wide = cast.int %small : (i16) -> i64 loc(#o50)
%as_float = cast.int_to_float %wide : (i64) -> f64 loc(#o51)
%native = pdb.fptosi %as_float : (f64) -> i32 loc(#o52)
%address = cast.ptr %raw : (ptr<rw, u8, 0>) -> ptrsize loc(#o53)
```

class/interface upcast、checked dynamic cast 和 reflection conversion 使用 [`06-calling-convention-runtime.md`](./06-calling-convention-runtime.md) 中的 `cast.class_up`、`cast.interface_make`、`cast.interface_up`、`cast.try_class` 与 `cast.try_interface`；它们不是本节数值 cast 的别名。

## 8. 聚合、enum 与 slice

### 8.1 不提供聚合 SSA 指令

Core InkIR 不提供 `agg.make`、`agg.extract`、`agg.insert` 或多结果寄存器 tuple。非空 tuple、array、class、带载荷 enum、slice 与 interface reference 均通过：

- `place.*` 取得最终对象或子对象位置；
- `obj.init.*`、`obj.assign.*` 与 `obj.destroy*` 改变生命周期；
- `enum.*` 与 `slice.*` 执行具有额外语义约束的操作；
- call 的 `to %destination` 形式直接建立 address-only 结果。

TargetABI 可以在最后 lowering 中把可复制聚合物理拆进寄存器，但不得把该实现细节重新序列化为 Core InkIR 聚合 SSA。

### 8.2 Enum operation

| Opcode | 形状 | Effect | 阶段 | 关键 verifier 规则 |
| --- | --- | --- | --- | --- |
| `enum.value` | `%e = enum.value {variant = @E::V} : !tE` | `Pure`, `TargetDependent` | `SC` | 只用于 TargetABI 已分类为单标量的无载荷 enum |
| `enum.init_variant` | `enum.init_variant %dst {variant = @E::V} : !place<init,!tE>` | `WriteMemory(typed)`, `BeginLifetime`, `TargetDependent` | `SC` | `SizedObjectType(E)`；`%dst` 已执行 `obj.init.begin`；variant 属于 E；进入该分支的部分初始化状态 |
| `enum.discriminant` | `%d = enum.discriminant %e : !tE -> ptrsize` | `Pure` 或 `ReadMemory(typed)`, `TargetDependent` | `SC` | SSA 只允许单标量 enum；address-only enum 传只读 place；结果是语义分支序号 |
| `enum.is_variant` | `%b = enum.is_variant %e {variant = @E::V} : !tE -> bool` | `Pure` 或 `ReadMemory(typed)`, `TargetDependent` | `SC` | 单标量 enum 使用 SSA operand，address-only enum 使用只读 place；只比较语义 variant，不暴露物理 tag/niche bits |

`enum.init_variant` 只选择活动分支，不凭空初始化 payload。带 payload 分支随后用 `place.enum_payload` 取得 init place，并按正常对象顺序初始化；非平凡 payload 先以该 place 开始 child transaction，payload commit 只登记进 enum parent，最后再由 `obj.init.commit` 提交完整 enum。

### 8.3 Slice operation

slice 是 address-only 两字 descriptor，以下 operation 都直接读写 slice place：

| Opcode | 形状 | Effect | 阶段 | 关键 verifier 规则 |
| --- | --- | --- | --- | --- |
| `slice.init` | `slice.init %dst, %first, %length` | `ReadMemory(typed)`, `WriteMemory(typed)`, `BeginLifetime` | `SC` | slice/element 为 SizedObjectType；dst 是 active owner transaction；first 是捕获当前 ObjectLifetimeGeneration 的 `!place<ro/rw,T>`；length > 0 且完整连续范围 Alive；只填充物理 data/length 与 verifier-only BorrowGenerationSet，仍由 final completer 随后显式 `obj.init.commit` |
| `slice.init_empty` | `slice.init_empty %dst` | `WriteMemory(typed)`, `BeginLifetime` | `SC` | dst 是 active owner transaction 的准确 slice；只填充 canonical `{null, 0}` 与空 BorrowGenerationSet，仍由 final completer 随后显式 `obj.init.commit` |
| `slice.data` | `%p = slice.data %slice` | `ReadMemory(typed)` | `SC` | slice 已 Alive且 generation set 有效；返回兼容 raw pointer并显式丢弃 checked borrow generation |
| `slice.length` | `%n = slice.length %slice` | `ReadMemory(typed)` | `SC` | slice 已 Alive；返回 `ptrsize` |
| `slice.index` | `%element = slice.index %slice, %index` | `ReadMemory(typed)`, `MayTrap`, `TargetDependent` | `SC` | `SizedObjectType(element)`；index 为 `ptrsize`；按TargetContext的`sizeof(T)`计算element address；越界进入 bounds trap；对应 generation 必须仍 Alive；结果为继承 ro/rw access、`borrow` authority 和准确 ObjectLifetimeGeneration 的 element place |
| `slice.subslice` | `slice.subslice %dst, %slice, %begin, %end` | `ReadMemory(typed)`, `WriteMemory(typed)`, `BeginLifetime`, `MayTrap`, `TargetDependent` | `SC` | slice/element 为 SizedObjectType；dst 是 active owner transaction；按TargetContext的`sizeof(T)`计算data offset；`begin <= end <= length`，否则 bounds trap；只填充对应子范围 descriptor/BorrowGenerationSet并允许零长度非 null data，仍由 final completer 随后显式 `obj.init.commit` |

这些 operation 保留 safe slice 的 `ContainsNoEscapeValue`。把 descriptor 字段复制进普通匿名 tuple 或 raw storage 不能绕过 no-escape verifier。`BorrowGenerationSet` 是从 operand dataflow 与对象树重建的验证事实，不作为隐藏用户字段或可伪造 binary payload；typed copy 保留它，raw pointer/`slice.data` 不保留。

## 9. Place、memory 与 raw bytes

### 9.1 Storage 与根 place

| Opcode | 形状 | Effect | 阶段 | 关键 verifier 规则 |
| --- | --- | --- | --- | --- |
| `mem.alloca` | `%p = mem.alloca : !place<init,T>` | `Allocate(stack)` | `SC` | `SizedObjectType(T)`；大小/alignment 可表示；只分配 storage，不开始 T 生命周期 |
| `mem.global_place` | `%p = mem.global_place @g : !place<A,T>` | `Pure`, `TargetDependent` | `SC` | symbol 是类型匹配且 `SizedObjectType(T)` 的 Alive global；A 来自 mutability；结果始终是匹配 generation 的 borrow authority，绝不产生 lifecycle owner；TLS global 必须已由 module runtime 按 `eager_thread_activation` plan 初始化，查询本身永不触发初始化 |

address-only parameter、constructor destination 和 function result destination 直接作为 entry block 的 place argument，不用伪造 `mem.param` opcode。stack storage 随 activation 销毁；其对象 cleanup 必须在 frame 释放前显式完成。

Core registry 没有通用 `mem.alloc` 或 `mem.dealloc`。非 stack storage 由已注册 runtime owner schema 产生不可伪造的 owner token，并由同一 allocator kind/version 的 matching release schema 消耗；普通 place/raw pointer 不是释放权限。`Deallocate(storage-kind)` 保留在 effect taxonomy 中供这些 runtime operation 使用。

### 9.2 Place formation 与投影

| Opcode | 形状 | Effect | 阶段 | 关键 verifier 规则 |
| --- | --- | --- | --- | --- |
| `place.deref` | `%p = place.deref %raw : ptr<A,T,S> -> !place<A,T>` | `Pure` | `SC` | 要求 `SizedObjectType(T)` 且非 `SealedRuntimeStorage`；只形成 `borrow` authority capability；raw 前置条件成立时捕获当前 referent ObjectLifetimeGeneration，后续 typed use 要求其仍匹配；不能执行 lifecycle operation |
| `place.from_ref` | `%p = place.from_ref %ref : ref<A,T> -> !place<A,T>` | `Pure` | `SC` | `SizedObjectType(T)`；保留 access 和 captured ObjectLifetimeGeneration，产生 `borrow` authority；要求 generation 仍匹配当前 Alive referent，不延长生命周期 |
| `place.as_alive` | `%live = place.as_alive %storage : !place<init,T> -> !place<A,T>` | `Pure` | `SC` | `CapabilityRebind`；`A = ro\|rw`；当前路径已证明准确 T 因 commit 或 destination-call normal postcondition 而 Alive；地址/path 不变；A 不超过 owner mutability；消费 source PlaceCapabilityGeneration，产生无 active TransactionId 的 next generation |
| `place.as_uninitialized` | `%storage = place.as_uninitialized %old : !place<rw,T> -> !place<init,T>` | `Pure` | `SC` | `CapabilityRebind`；当前路径已证明准确 T 经 destroy 回到 AllocatedUninitialized；调用方具有销毁与重新初始化权限且没有存活 borrow/child capability；消费 source generation，产生无 TransactionId 的 next init generation；只读 place 非法，不能用于 rollback |
| `place.field` | `%f = place.field %base {field = @T::f}` | `Pure`, `TargetDependent` | `SC` | parent/field 均 `LayoutComplete` 且为 SizedObjectType；字段属于准确 class；不提升 parent access，并结合字段 lifetime fact 推导结果能力 |
| `place.base` | `%b = place.base %derived {base = @Derived::<base-selector>}` | `Pure`, `TargetDependent` | `SC` | selector是Derived定义中唯一`SymbolKind=base` member，不是Base type symbol；derived/base 均 `LayoutComplete` 且为 SizedObjectType；只允许已声明single concrete base路径，不执行virtual dispatch |
| `place.tuple_element` | `%e = place.tuple_element %tuple {index = N}` | `Pure`, `TargetDependent` | `SC` | tuple/element 均 `LayoutComplete` 且为 SizedObjectType；N 为静态合法索引 |
| `place.array_element` | `%e = place.array_element %array, %index` | `Pure`, `TargetDependent`, `MayTrap` | `SC` | array/element 均 `LayoutComplete` 且为 SizedObjectType；index 为 `ptrsize`；越界进入 bounds trap |
| `place.enum_payload` | `%p = place.enum_payload %enum {variant = @E::V}` | `Pure`, `TargetDependent` | `SC` | enum/payload 均 `LayoutComplete` 且为 SizedObjectType；当前路径证明 V 活动，或处于已选择 V 的部分初始化状态 |
| `place.addr` | `%raw = place.addr %p : !place<A,T> -> ptr<A,T,S>` | `Pure` | `SC` | T 不是 `SealedRuntimeStorage`；只暴露地址；不把 place 生命周期事实编码进 raw pointer |
| `place.borrow` | `%ref = place.borrow %p : !place<A,T> -> ref<A,T>` | `Pure` | `SC` | T 已 Alive；reference 非空并捕获当前 ObjectLifetimeGeneration；借用不得超过该 generation；copy 保留 generation |

投影不开始、提交或结束对象生命周期。对 Alive parent，结果至多继承 parent 的 ro/rw 能力；对 PartiallyInitialized parent，已经初始化的子对象可以得到受 parent 限定的可读 place，尚未初始化的子对象只能得到 init place，未来子对象不可访问。尤其 `place.field` 取得的 init place 仍必须由 `obj.init*` 初始化；非平凡 child 的 storage owner 必须对该 init place 执行 `obj.init.begin` 后才能把它作为 `to %destination`，投影本身不隐式建立 child transaction。对尚未初始化字段执行 `mem.load` 非法。

`place.as_alive` 与 `place.as_uninitialized` 也不是生命周期 operation：它们只把 path-sensitive verifier 已经证明的状态物化成正确访问类型，并可在 lowering 中擦除。二者具有 `CapabilityRebind` trait，虽无运行时 memory effect，却在每条动态路径上消费 source `PlaceCapabilityGeneration` 并产生 next generation；旧 generation、本次构造期从其投影的 descendant capability 及其 SSA alias 此后全部非法，operation 不可复制、推测或越过相关 lifecycle boundary。commit 后仍把旧 `!place<init,T>` 直接传给 load/borrow/destroy 非法；destination call 的 normal path 需要可访问 capability 时显式执行 `place.as_alive`。invoke unwind path 没有 Alive fact，不能执行该转换，但 rollback 后原 init generation 保持有效，可以直接 fresh begin。destroy 后若要重建对象，则先执行 `place.as_uninitialized`，再以 fresh identity 执行 `obj.init.begin`；它不能用于 rollback 或复活旧 TransactionId。

`place.addr` 可以为 initializing_instance 的内部实现暴露地址，但从 PartiallyInitialized object 得到的 raw pointer 受构造期 `this` 不逃逸规则约束；它不能存入外部对象、Task、global 或传给不能证明不保存该地址的调用。普通 `place.borrow` 仍要求目标子对象已经 Alive。

`LifetimeAuthority` 与 `init|ro|rw` access 分开验证。allocation、runtime-provided `global_lifecycle` entry、按值参数对象和 caller destination 可以携带 owner authority；普通 `mem.global_place`、`place.from_ref`/`place.deref` 及从 borrowed parent 得到的 projection 只能是 borrow authority。global initializer/finalizer 之外的函数不得 destroy 或 reinitialize global；反复取得 global place 也不会复制 cleanup obligation。所有 `obj.init.*`、`obj.destroy*` 和 `place.as_uninitialized` 都要求准确 owner authority；rw borrow 只允许普通写访问，不能结束或重新开始 referent 生命周期。

`SealedRuntimeStorage` place 只允许 allocation/destination formation、`obj.init.begin`、`place.as_alive`/`place.as_uninitialized` 以及其 runtime kind 注册的 typed semantic opcode。它不能进入普通 projection、`place.addr`/`place.deref`、typed/raw load/store、copy/assign、generic destroy 或 byte-count raw memory operation；即使 raw address 从别处取得，只要访问范围与 active/partial sealed storage 重叠也必须拒绝。私有 runtime schema 直接接受 typed place 并承担其 owner/effect/postcondition，不通过通用 raw pointer 打开后门。

### 9.3 Typed scalar load/store

| Opcode | 形状 | Effect | 阶段 | 关键 verifier 规则 |
| --- | --- | --- | --- | --- |
| `mem.load` | `%v = mem.load %p : !place<ro/rw,T> -> T` | `ReadMemory(typed)` | `SC` | T 为 `SizedObjectType` 的单标量或 unit；对象 Alive；value bytes 已初始化；自然 alignment 满足 |
| `mem.store` | `mem.store %v, %p : T, !place<rw,T>` | `WriteMemory(typed)` | `SC` | T 为 `SizedObjectType`、已初始化且可赋值的单标量或 unit；不开始新生命周期 |
| `mem.load_unaligned` | `%v = mem.load_unaligned %p {alignment = N}` | `ReadMemory(typed)` | `SC` | T 为 `SizedObjectType` 的单标量；显式 alignment 合法且目标支持/可 lowering |
| `mem.store_unaligned` | `mem.store_unaligned %v, %p {alignment = N}` | `WriteMemory(typed)` | `SC` | T 为 `SizedObjectType`；与 store 相同，但不声称自然 alignment |

`mem.store` 不能替代 `obj.init`，也不能逐字段伪造 address-only result 的提交。volatile、atomic 和设备内存将在独立 memory extension 中定义；v0 普通 load/store 不隐含这些属性。

### 9.4 Raw memory

| Opcode | 形状 | Effect | 阶段 | 关键 verifier/执行前置条件 |
| --- | --- | --- | --- | --- |
| `raw.load` | `%v = raw.load %ptr {alignment = N} : ptr<ro/rw,T,A> -> T` | `ReadMemory(raw)` | `SC` | T 为 `SizedObjectType` 的单标量；按准确 sizeof/alignment 检查范围、已初始化 bytes、表示与 lifetime |
| `raw.store` | `raw.store %v, %ptr {alignment = N}` | `WriteMemory(raw)` | `SC` | T 为 `SizedObjectType` 的单标量；按准确 sizeof/alignment 检查范围、mutability 与 lifetime；不开始 typed lifetime |
| `raw.memcpy` | `raw.memcpy %dst, %src, %bytes {dst_alignment = D, src_alignment = S}` | `ReadMemory(raw)`, `WriteMemory(raw)` | `SC` | 范围不发生未授权重叠；源每个 byte 已初始化 |
| `raw.memmove` | `raw.memmove %dst, %src, %bytes {dst_alignment = D, src_alignment = S}` | `ReadMemory(raw)`, `WriteMemory(raw)` | `SC` | 允许重叠；源每个 byte 已初始化 |
| `raw.memset` | `raw.memset %dst, %byte, %bytes {alignment = N}` | `WriteMemory(raw)` | `SC` | byte 为 `u8`；只初始化字节，不建立任何 typed object |

`raw.memcpy`、`raw.memmove` 和 `raw.memset` 以显式 byte count 工作，因此允许 operand raw pointer 带 unsized pointee；它们只验证地址空间、范围、byte initialization、重叠和显式 alignment，不读取 `sizeof(pointee)`，也不能据此形成 typed place 或证明某个 T Alive。

上述 raw range 还必须与全部 active/partial `SealedRuntimeStorage` allocation/subobject 不相交；这一检查基于 abstract allocation/range，不因 pointer pointee 被 cast 成 `u8`、`void` 或整数地址而消失。

违反 raw memory 前置条件属于 UB，不自动产生可捕获异常或 `MayTrap`。sanitizer 可以在另一条编译配置中显式插入 `rt.trap`，但不能改变普通 opcode schema。

### 9.5 裸指针操作

| Opcode | 形状 | Effect | 阶段 | 关键规则 |
| --- | --- | --- | --- | --- |
| `ptr.offset` | `%q = ptr.offset %p, %elements : ptr<A,T,S>, ptrsize -> ptr<A,T,S>` | `Pure`, `TargetDependent` | `SC` | 要求 `SizedObjectType(T)`；使用目标 `sizeof(T)` 缩放，乘加按地址宽度取模；不自动 inbounds |
| `ptr.byte_offset` | `%q = ptr.byte_offset %p, %bytes : ptr<A,T,S>, ptrsize -> ptr<A,T,S>` | `Pure` | `SC` | 直接按 byte 地址位宽取模；允许 unsized pointee |
| `ptr.cmp` | `%b = ptr.cmp %p, %q {predicate = eq/ne/ult/ule/ugt/uge}` | `Pure` | `SC` | address space 兼容；按无符号数值地址比较；不要求 pointee sized |

因此 `ptr<void>`、`ptr<never>`、`ptr<function<...>>`、`ptr<incomplete-nominal>` 和 `ptr<runtime.opaque>` 不得作为 `ptr.offset` 或 `place.deref` 的 operand，也不能进入 `raw.load/store`、typed object lifecycle 或任何隐式 `sizeof/alignof` schema；它们仍可用于 `cast.ptr`、`cast.ptr_access`、`ptr.byte_offset`、`ptr.cmp` 和 byte-count raw memory operation。

形成任意数值地址本身不 trap、不开始对象生命周期，也不证明存在 allocation。后续解引用和访问仍承担完整前置条件。

## 10. 对象初始化、复制、赋值与销毁 `obj.*`

### 10.1 生命周期 operation

| Opcode | 形状 | Effect | 阶段 | 关键 verifier 规则 |
| --- | --- | --- | --- | --- |
| `obj.init.begin` | `obj.init.begin %dst : (!place<init,T>) -> ()` | `BeginLifetime` | `SC` | `SizedObjectType(T)` 且 dst 具有 owner authority；对 root：storage 为 `AllocatedUninitialized`；对 child：准确 parent transaction active、child 尚未初始化且构造顺序允许；建立 fresh transaction identity；禁止重复/非法重叠 begin |
| `obj.init` | `obj.init %dst, %value : (!place<init,T>, T) -> ()` | `WriteMemory(typed)`, `BeginLifetime` | `SC` | T 为非 sealed `SizedObjectType` 的单标量或 unit；dst 属于当前 active root/child transaction 且尚未建立该值；若 dst 是 active aggregate parent 的 direct leaf，则原子使 leaf Alive、建立 fresh leaf ObjectLifetimeGeneration，并把 `(path, generation, construction order)` 记录进 `InitializedLeafMask`/ActiveObjectTree；root scalar/unit 的 generation 仍由 root commit 建立；不是 constructor call |
| `obj.init.copy` | `obj.init.copy %dst, %src : (!place<init,T>, !place<ro/rw,T>) -> ()` | `ReadMemory(typed)`, `WriteMemory(typed)`, `BeginLifetime` | `SC` | T 为非 sealed `SizedObjectType`、address-only，`Copyable(T)` 且 `CopyConstructionEnabled(T)`；dst 是当前 active root/child transaction 的准确对象；src Alive；不可失败地按语义组成部分复制且不复制 padding，为每个 descendant 建立 fresh generation/ActiveObjectTree，trivial leaf 与非平凡 child 分别记入当前 transaction 的 `InitializedLeafMask`/`CommittedChildMask`，外层对象随后仍由当前 transaction commit |
| `obj.init.commit` | `obj.init.commit %dst : (!place<init,T>) -> ()` | `WriteMemory(typed)`, `BeginLifetime` | `SC` | T 为非 sealed `SizedObjectType`且 dst 具有 owner authority；dst 是当前 transaction root；没有 active child，所有必需 base/field/element/payload 已初始化且 constructor body 正常完成；当前对象转为 Alive 并建立 fresh ObjectLifetimeGeneration；若是 child，只登记进 active parent `CommittedChildMask` |
| `obj.assign.copy` | `obj.assign.copy %dst, %src : (!place<rw,T>, !place<ro/rw,T>) -> ()` | `ReadMemory(typed)`, `WriteMemory(typed)`；enum 分支可不同时再加 `EndLifetime`, `BeginLifetime` | `SC` | T 为非 sealed `SizedObjectType`、address-only，`Copyable(T)` 且 `CopyAssignmentEnabled(T)`；dst/src 均 Alive；unsame-variant enum 只在所有 payload 可递归 assign/construct 且 `NeedsDestroy=false` 时原子终结旧 payload generation tree并建立 fresh 新 tree，否则必须显式展开 CFG；不能替代初始化 |
| `obj.destroy` | `obj.destroy %object : (!place<A,T>) -> ()` | `WriteMemory(typed)`, `EndLifetime`, callee effect summary | `SC` | `A = ro\|rw`；T 是非 sealed `SizedObjectType` 且静态完整类型已知；object 具有 owner authority且 Alive；nominal class 从准确 `NominalSemanticProperties.Destructor` special member（若 present）开始并按声明逆序执行唯一 field/base 析构链，全链 nothrow；完成后终结 ObjectLifetimeGeneration 并回到未初始化 storage |
| `obj.destroy_dynamic` | `obj.destroy_dynamic %complete_object, %vtable` | `WriteMemory(typed)`, `EndLifetime`, dynamic-destroy `CallSummary`, `RuntimeEffect(dynamic_destroy)`, `RuntimeEffect(version_select)`, `MayDiverge` | `SC` | complete object 对 nonsealed layout-complete SizedObjectType 具有 owner authority；virtual 完整对象与 table/layout version 匹配；选择当前兼容的完整 nothrow 析构闭包，终结 generation但不释放 storage；析构链的 memory/runtime/allocate/deallocate/trap/fatal 上界不得因 nothrow 被擦除；关闭热更新后的合法pass可在证明版本覆盖整个dynamic extent时消除version_select |

`obj.init.begin` 表示初始化事务开始，不表示用户可观察绑定已经存在。每次 begin 都建立 fresh `TransactionId`；root identity 没有 parent，child identity 指向准确 active parent identity，因此动态 transaction 构成与对象包含关系一致的树。identity 是 verifier/执行器事实，不是可存储、可比较或可伪造的 SSA value；把 destination 转发给 callee 必须原样保留 identity。

transaction tree 只允许声明对象树上的严格包含关系。每个 active parent 同时至多有一个未完成的直接 active child；相同 subobject path 不能在 active/Alive 时重复 begin，两个非 ancestor/descendant transaction 不能发生 byte-range 部分重叠。child rollback 后可以在同一已恢复为未初始化的 path 上重试，但新 begin 必须产生新 identity，旧 identity 永久终止。niche、共享尾 padding 或 base offset 相同不能把两个逻辑 subobject 合并成同一 transaction。

`obj.init.commit` 是本核心生命周期注册表中唯一可显式把当前 transaction root 提交为 Alive 的 operation；active parent 内 direct scalar/unit leaf 则由 `obj.init` 的固定原子后置条件建立，不是可替代 root commit 的第二 opcode。commit 还必须保证该对象最终 vptr、active enum variant 和其他隐藏 TargetABI 状态正确。child commit 只终结 child transaction，并把 child 的 Alive lifetime fact 按顺序登记到仍 active parent 的 `CommittedChildMask`；parent 保持 PartiallyInitialized。parent commit 不替换已登记 leaf/child generation，只为 parent 本身建立 fresh generation。只有 root commit 发布完整 caller result/binding。已注册的运行时扩展 schema 可以为其 final-completer normal edge 规定同一唯一 commit 后置条件，例如 `async.await_copy`，但这不产生第二个通用生命周期 opcode。

commit 只改变对象状态并终结 transaction，不把 `!place<init,T>` 隐式改写成另一种 SSA 类型。需要读取、借用、赋值或销毁刚提交对象的路径必须显式执行 `place.as_alive`；该 proof operation 对 child 只取得 child capability，不提交 parent。caller-destination call 的 normal postcondition 同理，call 自身不产生隐藏的 address-only program value。

v0 不注册 `obj.init.abort`。最终失败的 callee 必须先按当前 transaction 的 cleanup plan 逆序销毁其 committed descendant，再由其 outward-unwind schema postcondition 恰好一次 rollback 当前 transaction。child rollback 只恢复 child path，parent identity、parent 的既有 `InitializedLeafMask`/`CommittedChildMask` 和先前 Alive sibling 保持不变；parent 可以 catch 后以 fresh child identity 重试。parent 若继续 outward unwind，则再逆序销毁自己 `CommittedChildMask` 中的全部 committed child，按逆构造顺序终结 `InitializedLeafMask` 中的每个 trivial leaf generation，并 rollback parent；root rollback 最终恢复整项 storage。`call.invoke` 的 unwind edge 只携带对应后置条件已经成立的状态证明；invoke operation 和中间转发 activation 都不得再次合成 rollback。

`obj.destroy` 与 `obj.destroy_dynamic` 终结的不只是 complete-object generation；它们必须按唯一析构顺序终结全部存活 descendant leaf/child generation，然后终结 root generation。任一捕获 field/base/element generation 的构造期或成活期 borrow 在完整对象销毁后都失效，同地重建不会恢复它。

typed copy 与 raw byte copy 严格分离：

- typed copy 按 base、field、element 和活动 enum payload 的语言顺序复制；
- virtual class 的目标 vptr 来自目标完整类型，不逐字复制源 vptr；
- padding 保持目标未初始化；
- noncopyable 类型没有 `obj.init.copy` 或 `obj.assign.copy` 后门；
- Core InkIR 没有通用 `obj.move`。

### 10.2 直接初始化与默认初始化纵切片

下例表示 semantic elaboration 已经为某个 `i64` 绑定选定精确常量 `0` 的直接初始化；它不自行规定每种源语言类型的普通默认初始化值：

```text
%storage = mem.alloca : () -> !place<init,i64> loc(#o70)
obj.init.begin %storage : (!place<init,i64>) -> () loc(#o71)
%zero = const.int {bits = 0x0000000000000000} : () -> i64 loc(#o72)
obj.init %storage, %zero : (!place<init,i64>, i64) -> () loc(#o73)
obj.init.commit %storage : (!place<init,i64>) -> () loc(#o74)
%value = place.as_alive %storage : (!place<init,i64>) -> !place<rw,i64> loc(#o75)
```

class 的默认初始化必须先在 semantic elaboration 中解析到一个合法内建初始化器，或者准确解析到该 class `NominalSemanticProperties.DefaultInitializer` 中 present 的唯一零参数 constructor；另一个同签名 function 不能替代该 role。caller 在最终 storage 上执行 `obj.init.begin`，再使用普通 `call.direct` 或 `call.invoke ... to %storage {destination_role = initializing_receiver}`；constructor callee 初始化 base/field，并在正常返回前执行 `obj.init.commit`。抽象 class 或没有合法默认初始化都是 semantic error，不生成“未初始化绑定”。

constructor unwind 时，callee 显式逆序销毁已经成功建立的子对象，不运行完整对象 destructor；随后 callee outward-unwind schema postcondition 终止 active transaction，并把 destination 的数据流状态恢复为 `AllocatedUninitialized`。`call.invoke` 的 unwind edge 观察并传播该状态证明，不自行执行回退；绑定从未可见，caller 不销毁完整结果。

### 10.3 层次化 child transaction 与重试示例

下例是 `Wrapper` constructor body 的语义 CFG pseudocode。entry `%self` 已携带 caller 建立的 active root identity，因此 constructor 不对 `%self` 重复 begin。非平凡 `buffer` field 的 owner 显式开始 child；第一次构造 unwind 后，child 已由 callee postcondition rollback，而 parent `%self` 仍 active，所以本地 handler 可以用 fresh child identity 重试。两次 normal edge 都只证明 `buffer` child 已 commit，最后才提交 root：

```text
^entry(%self: !place<init,!tWrapper> origin(#o200)) origin(#o201):
  %buffer = place.field %self {field = @Wrapper::buffer} : (!place<init,!tWrapper>) -> !place<init,!tBuffer> loc(#o202)
  obj.init.begin %buffer : (!place<init,!tBuffer>) -> () loc(#o203)
  call.invoke callee_kind = direct @buffer::make_default() to %buffer {destination_role = result} normal ^buffer_ready unwind ^first_failed(exception) : () -> !tBuffer loc(#o204)

^first_failed(%incoming: !exception origin(#o205)) origin(#o206):
  %active = eh.entry %incoming : (!exception) -> !exception loc(#o207)
  eh.match %active [class @Retryable -> ^retry(%active)] unmatched ^propagate(%active) : (!exception) -> () loc(#o208)

^retry(%caught: !exception origin(#o209)) origin(#o210):
  eh.end_catch %caught -> ^retry_begin : (!exception) -> () loc(#o211)

^retry_begin origin(#o212):
  obj.init.begin %buffer : (!place<init,!tBuffer>) -> () loc(#o213)
  call.invoke callee_kind = direct @buffer::make_default() to %buffer {destination_role = result} normal ^buffer_ready unwind ^retry_failed(exception) : () -> !tBuffer loc(#o214)

^retry_failed(%incoming2: !exception origin(#o215)) origin(#o216):
  %active2 = eh.entry %incoming2 : (!exception) -> !exception loc(#o217)
  eh.resume %active2 : (!exception) -> () loc(#o218)

^propagate(%outer: !exception origin(#o219)) origin(#o220):
  eh.resume %outer : (!exception) -> () loc(#o221)

^buffer_ready origin(#o222):
  %count = place.field %self {field = @Wrapper::count} : (!place<init,!tWrapper>) -> !place<init,i64> loc(#o223)
  %zero = const.int {bits = 0x0000000000000000} : () -> i64 loc(#o224)
  obj.init %count, %zero : (!place<init,i64>, i64) -> () loc(#o225)
  obj.init.commit %self : (!place<init,!tWrapper>) -> () loc(#o226)
  cf.return : () -> () loc(#o227)
```

`^buffer_ready` 的两个 incoming path 可以合并，因为两者具有相同 active parent identity、相同 `InitializedLeafMask`/`CommittedChildMask` 和相同 child Alive fact；已经终结的 child identity 不是 live capability，不妨碍该 join。若 `Wrapper` 在 `buffer` 之前还有已 commit 的 base/field，`^retry_failed` 和 `^propagate` 在 `eh.resume` 前必须逆序 `obj.destroy` 它们；随后 constructor 的 outward-unwind postcondition rollback `%self` root。重试不能复活第一次 child identity，也不能在第一次 child 仍 active 时执行第二次 begin。

## 11. 同步调用 `call.*`

### 11.1 共同结果约定

所有同步调用使用同一逻辑结果规则：

- `void` 不产生 SSA result，也没有 destination；
- `never` 不产生 value 或 destination；effective call summary 必须证明所有实际 completion 只可能是 unwind、trap、fatal 或无 completion，并分别保留 `MayUnwind`、`MayTrap`、`RuntimeEffect(fatal)`、`MayDiverge` 的准确上界。nothrow never call 后必须以 `cf.unreachable` 结束 block，may-unwind never invoke 的 normal successor 不接收结果且只包含 verifier 证明不可达的终止路径；
- 单标量与 unit 结果使用一个 SSA result；
- address-only 结果不产生 SSA result，使用 `to %destination`；
- constructor 没有普通逻辑结果，但同样使用 `to %destination` 传入 initializing_instance 的完整对象最终 storage；
- `to %destination` 可以指向 allocation root，也可以指向 active parent 的准确 child；两者都要求 destination 已有且只有一个 active init transaction；storage owner 在 callee 与全部实参求值完成后、首次构造调用前执行一次 `obj.init.begin`，转发已有 destination 时不得重复 begin；
- 最终完成当前对象构造的 callee 在正常返回前执行一次 `obj.init.commit`；child commit 只登记 active parent，root commit 才发布完整结果；中间 callee 可以把同一 destination/identity 继续转发而不重复 commit；
- 最终失败的 callee cleanup 后由 outward-unwind schema postcondition rollback 当前 transaction；child unwind edge 证明 child 已回到未初始化而 parent 仍 active，root unwind edge 证明整个 destination 已回到未初始化；
- caller 按源码顺序先求值 callee，再从左到右求值实参，最后执行 call operation。

每个 `object T` 实参都使用 verifier 跟踪但不作为用户 SSA 值暴露的 obligation 状态机 `Unprepared -> Preparing -> PreparedParameterObject -> TransferredToActivation -> Destroyed`。caller 在准确参数最终 storage 中完成 begin、构造与 commit 后才进入 `PreparedParameterObject`，并暂时持有唯一 owner/cleanup obligation；若后续实参求值、dispatch/version selection 或 call boundary 建立在 activation handoff 前失败，caller 必须按逆源码顺序销毁全部已 prepared 参数对象。call operation 建立 callee activation 的同一原子语义步骤消费这些 caller obligation，并把每个 owner `!place<rw,T>` 绑定到对应 entry parameter；该点之后 caller 不得再读取、销毁或转移这些对象，callee 在 normal 或 outward-unwind 退出时按逆参数顺序恰好清理一次。fatal/`MayDiverge` 路径按其不运行语言 cleanup 的既有规则处理。CFG verifier 必须证明每个 prepared obligation 在所有前驱上恰好由 caller failure cleanup 或 activation handoff 消费，不能依赖后端 ABI 的寄存器拆分隐式完成转移。

每个 `call.*` 或 reflection adapter 的实际 Ink target 都必须满足 10 中央注册表定义的 `ExecutableInkTarget`：普通 function 需要已验证 implementation body，stable entry 需要在当前 active/pinned version owner 中解析到已验证 body，`extern` 则需要 linker/loader 已按完整 Ink ABI 唯一解析。`EntryIdentity`、签名匹配或 bodyless declaration 本身都不证明可执行；abstract declaration、Closed decorator provenance 和 `destructor_body` 永远不得作为普通调用目标。constructor 只能由 direct callee 以 `initializing_receiver` destination 调用，抽象 class 不得形成该 destination；destructor 只能由 `obj.destroy*` 的已验证析构链进入。间接函数值、reflection snapshot 与 dispatch table 的 producer 必须把所有可达 target 限制到同一谓词。

### 11.2 Nothrow call operation

| Opcode | Callee 形状 | Effect | 阶段 | 关键 verifier 规则 |
| --- | --- | --- | --- | --- |
| `call.direct` | `@function(%args...)` | effective call effect | `SC` | resolved direct target 满足 `ExecutableInkTarget`，且声明契约/独立证明保证不会 unwind |
| `call.indirect` | `%callee(%args...)` | conservative effective call effect | `SC` | function type 准确且 producer provenance 只能到达 `ExecutableInkTarget`；普通函数值不携带隐式 nothrow，只有独立证明才合法 |
| `call.virtual` | `%receiver {slot = @Base::method}(%args...)` | effective slot effect, `RuntimeEffect(virtual_dispatch)` | `SC` | receiver、slot、override signature 与版本 table 匹配；slot concrete、非 abstract 且最终 target 满足 `ExecutableInkTarget`、nothrow |
| `call.interface` | `%interface_place {method = @I::method}(%args...)` | effective slot effect, `RuntimeEffect(interface_dispatch)` | `SC` | receiver 是 address-only interface reference 的 readable place，不是两字 SSA descriptor；object/table pair 与 layout 匹配；binding concrete 且最终 target 满足 `ExecutableInkTarget`、nothrow |

标量和 address-only 形状示例：

```text
%sum = call.direct @math::add(%lhs, %rhs) {function_type = !t42, entry_identity = stable, calling_convention = ink, target_abi_tag = sha256"0000000000000000000000000000000000000000000000000000000000000000", explicit_argument_count = 2} : (i64, i64) -> i64 loc(#o80)
call.direct @buffer::Buffer(%capacity) {function_type = !t43, entry_identity = stable, calling_convention = ink, target_abi_tag = sha256"0000000000000000000000000000000000000000000000000000000000000000", explicit_argument_count = 1} to %buffer {destination_role = initializing_receiver} : (ptrsize) -> !t44 loc(#o81)
```

direct target 还必须具有 `stable`、`version_local`、`continuation_local` 或 `extern` 入口身份，但该枚举仅选择入口机制，不替代 `ExecutableInkTarget` 和 callable-role 检查。普通源码调用在 hot reload 模式下进入 stable entry；没有 version pin 的路径不得直接调用 version-local body。`extern` 表示由编译/链接阶段按唯一完整 Ink ABI 解析的外部定义；`calling_convention = c` 的声明不得使用 `call.*`，必须经 `abi.call`/`abi.invoke` bridge。

effective call effect 由 target identity 推导，不能一律使用当前可见 body summary：

- `stable` direct target 使用 compatibility lineage 固定的 `StableEffectEnvelope.SyncEffects`，并固有不可消去、不可 CSE、不可推测、不可复制的 `RuntimeEffect(version_select)`；异常通道由固定 baseline bit 与当前只可单调加强的 `BehaviorContracts` 共同收窄，nothrow 形态要求二者合成后的 effective may-unwind 为 false；
- stable virtual/interface slot 同样使用根 slot 的 `StableEffectEnvelope.SyncEffects` 与 `RuntimeEffect(version_select)`，再并入相应 dispatch effect；code-only publish 后不能用旧 body 的较弱摘要优化跨调用行为；
- `version_local` target 只有在 verifier 证明匹配 version pin 覆盖完整调用 dynamic extent 时才使用所选 body 的精确 effect summary；`continuation_local` 使用闭合 continuation region 的精确 summary；
- `extern` 和普通 indirect target 使用其已验证 declaration/closed function type 能证明的 summary，无法证明的字段取保守上界；v0 普通函数类型不携带用户可承诺的 pure/read-only 或 nothrow contract。

只有全程序证明关闭热更新，或证明 stable target 在整个选择与调用范围内被冻结且 pin 完整覆盖，pass 才能把 stable effective effect 收窄为 version-local 精确 summary。当前 exact body 必须同时满足固定 envelope 上界和已经发布、以后不可减弱的 BehaviorContracts；单次实现较弱不能建立新契约。具体字段和兼容规则见 [`06-calling-convention-runtime.md`](./06-calling-convention-runtime.md)。

### 11.3 May-unwind `call.invoke`

所有可能产生 Ink unwind 的同步 direct、indirect、virtual、interface 或获准 runtime adapter 调用统一使用一个 terminator：

```text
call.invoke callee_kind = direct @might_fail(%arg) {function_type = !t45, entry_identity = stable, calling_convention = ink, target_abi_tag = sha256"0000000000000000000000000000000000000000000000000000000000000000", explicit_argument_count = 1}
    normal ^ok(result(0))
    unwind ^cleanup(exception)
    : (i64) -> i64 loc(#o82)
```

v0 中 `call.invoke` 允许的中央 `CalleeKindTag` 子集为 `direct|indirect|virtual|interface|reflection`；`abi|decorator_continuation|async_continuation` 分别只能进入其专用 opcode schema。callee 与 argument 的具体形状、`ExecutableInkTarget`/concrete-dispatch 证明复用对应 nothrow call schema，reflection 还要求 snapshot 选中的准确 target 满足同一谓词。`call.invoke` 的效果为按上述 target identity 推导的 effective call effect、`MayUnwind` 与 `Control` 的并集；stable target 仍保留 envelope 与 `RuntimeEffect(version_select)`，不能因当前 version-local body 较弱而收窄。

`result(0)` 和 `exception` 是 terminator 产生并传入 successor 的 schema-produced edge value，不分配 invoke 所在 block 的普通 SSA ValueId，也不能在对应 successor 之外使用。normal successor 的首个结果 block argument 类型为函数单标量/unit 结果；unwind successor 的首个 block argument 类型为 `!exception`。void 或 address-only 结果没有 normal result argument。

对于带 `to %destination` 的 address-only 调用，normal edge 额外证明最终完成者已经唯一 commit 当前 root/child transaction；child normal edge 只更新 active parent `CommittedChildMask`，root normal edge 才发布完整结果。unwind edge 额外证明最终失败者的 outward-unwind schema postcondition 已经唯一 rollback 当前 transaction；child failure 保留 active parent，因此 handler 可以用 fresh child identity 重试，root failure 才恢复整项 destination。`call.invoke` 自身不隐式执行 begin、commit 或 rollback；中间 forwarding activation 只传播这些证明。

normal edge 证明 destination Alive，但不会把原 `!place<init,T>` 静默改成 `!place<rw,T>`，也不产生 address-only logical SSA result。normal successor 若要访问或销毁该对象，必须对原 destination 执行 `place.as_alive`；该 operation 由 normal-edge state proof 验证。unwind successor 仍持有已 rollback 的 init storage fact，不能执行 `place.as_alive`，但可以按重试规则以 fresh TransactionId 再次 begin。

address-only 形状：

```text
call.invoke callee_kind = direct @buffer::Buffer(%capacity) {function_type = !t43, entry_identity = stable, calling_convention = ink, target_abi_tag = sha256"0000000000000000000000000000000000000000000000000000000000000000", explicit_argument_count = 1} to %buffer {destination_role = initializing_receiver}
    normal ^constructed
    unwind ^cleanup(exception)
    : (ptrsize) -> !tBuffer loc(#o83)
```

`call.invoke` 必须是 block terminator。即使没有本地 catch 或 cleanup，may-unwind 调用仍使用 invoke，并让 unwind 路径最终通过 `eh.resume` 外传；不能换成隐式展开的 `call.direct`。

### 11.4 调用与生命周期 verifier

Verifier 至少检查：

- ordered argument 的数量、准确类型与 passing mode 匹配闭合 logical signature；
- address-only 按值参数已经在独立参数 storage 中直接构造或显式 copy-initialize；
- noncopyable 命名对象没有隐藏 move；
- 每个 direct target、间接函数值 provenance、dispatch table final target 和 reflection snapshot target 都满足 `ExecutableInkTarget`；ordinary call 不得选择 abstract/bodyless declaration、Closed decorator provenance 或 `destructor_body`；
- `to %destination` 只用于 `SizedObjectType` 的 address-only 结果或 callable kind 为 constructor 的初始化 destination；destination 是兼容 init place、大小/alignment/address space 正确，并已经处于唯一 active root/child transaction；child destination 的 parent identity 必须 active 且对象路径/构造顺序准确；
- constructor target 的 callable kind 是 constructor，receiver kind 是 initializing_instance，且没有独立 SSA result；destination nominal class 必须 concrete，不含 abstract virtual slot 或未实现 interface binding；默认初始化调用的 target 还必须恰等于 `NominalSemanticProperties.DefaultInitializer`；
- virtual/interface slot 必须 concrete，最终 ImplementationFunction/AdapterSymbol 必须是已验证 bodyful stable binding；abstract slot 或仅有 abstract declaration 的 interface member 不得进入 reachable dispatch；
- scalar result 不能写入 caller destination，address-only result 不能作为 SSA result；
- nothrow call opcode 的 outward unwind 路径不可达；
- invoke normal/unwind successor 的 edge arguments、cleanup 与 exception token 所有权完整；
- destination call normal path 只有在 schema 已证明当前 root/child commit 后才可执行 `place.as_alive`；unwind path 不得伪造 Alive capability；destroy 后只有满足唯一 owner、无存活 borrow/child capability和 AllocatedUninitialized 证明的路径才可执行 `place.as_uninitialized`；
- 每个 caller-destination transaction 的动态路径最终或者由最终完成者唯一 commit，或者在已完成 descendant cleanup 后由最终失败者的 outward-unwind schema postcondition 唯一 rollback；child commit/rollback 不终止 parent，中间转发层不得重复这些状态转换；
- transaction identities 形成严格对象包含树，active child 遵守栈纪律且无重复/非法重叠 begin；parent commit 前没有 active child 且 required mask 完整，parent outward unwind 逆序销毁全部且仅销毁 committed child；rollback 后重试必须使用 fresh identity。

async、reflection、decorator、dynamic dispatch table、hot reload stable entry 和 C ABI bridge 的附加 opcode 见 [`06-calling-convention-runtime.md`](./06-calling-convention-runtime.md)，本注册表不把它们折叠成语义不透明的 `rt.call`。

## 12. 异常与展开 `eh.*`

### 12.1 Exception token 所有权

`!exception` 是线性所有权 token，但“线性”指每条动态控制路径上恰好一个 owner，不要求 SSA 文本只出现一次。只读查询可以借用 token；`eh.match` 可以把同一 token 写在多个互斥 successor edge 上，但运行时只选择一条边。

一个典型 handler 入口为：

```text
^unwind(%incoming: !exception origin(#o90)) origin(#o91):
  %active = eh.entry %incoming : (!exception) -> !exception loc(#o92)
  eh.match %active [class @IoError -> ^io(%active), catch_all -> ^any(%active)] unmatched ^outer(%active) : (!exception) -> () loc(#o93)

^io(%exception: !exception origin(#o94)) origin(#o95):
  %payload = eh.payload %exception {as = class @IoError} : (!exception) -> !place<ro,!tIoError> loc(#o96)
  eh.end_catch %exception -> ^after : (!exception) -> () loc(#o97)
```

`eh.entry` 消费 incoming token 并产生当前 handler list 的 active token。`eh.match` 把所有权转移给唯一选中的 successor。`eh.payload` 只借用、不消费；`eh.end_catch`、`eh.rethrow`、`eh.resume`、`eh.throw_from` 的 cause transfer 或 fatal path 最终恰好选择一个消费动作。

### 12.2 Handler operation

| Opcode | 形状 | Effect | 阶段 | 所有权与 verifier |
| --- | --- | --- | --- | --- |
| `eh.entry` | `%active = eh.entry %incoming : !exception` | `RuntimeEffect(exception)`, `Control` | `SC` | 只在 unwind successor 首部；consume incoming，produce active |
| `eh.match` | `eh.match %active [handlers...] unmatched ^outer(%active)` | `RuntimeEffect(exception_match)`, `Control` | `SC` | terminator；按源码顺序；catch-all 最多一个且最后；转移 token |
| `eh.payload` | `%p = eh.payload %active {as = class/interface ...}` | `ReadMemory(exception)` | `SC` | handler 已由匹配证明；返回 readonly、不可逃逸 view；只借用 token |
| `eh.end_catch` | `eh.end_catch %active -> ^normal` | `ExceptionDestroySummary`, `RuntimeEffect(exception)`, `EndLifetime`, `Control` | `SC` | consume当前ActiveUnwindRecord token并释放本record/backing引用；普通独占record的last owner销毁payload/pins，共享ExceptionBox只释放本record强引用，只有box last owner才销毁payload/cause/pins |

v0 没有 exception filter 或 multi-catch 伪操作。重叠 class/interface handlers 不得重排。catch binding 永久 readonly，不能通过 copy 一个 view 延长 payload 生命周期。

### 12.3 Throw、rethrow 与 resume terminator

| Opcode | 形状 | Effect | 阶段 | 关键语义 |
| --- | --- | --- | --- | --- |
| `eh.throw` | `eh.throw @ExceptionType(%constructor_args...)` | `ExceptionCreateSummary`, `Allocate(exception)`, `WriteMemory(exception)`, `BeginLifetime`, `RuntimeEffect(exception)`, `MayUnwind`, `Control` | `SC` | ExceptionType满足中央ExceptionPayloadClass；指定constructor的receiver/type/参数与payload storage逐项匹配，直接构造并传播；summary含constructor与失败cleanup/deallocation |
| `eh.throw_copy` | `eh.throw_copy %source` | `ExceptionCreateSummary`, `Allocate(exception)`, `ReadMemory(typed)`, `WriteMemory(exception)`, `BeginLifetime`, `RuntimeEffect(exception)`, `MayUnwind`, `Control` | `SC` | 只允许Alive source T满足ExceptionPayloadClass、`Copyable(T)`且`CopyConstructionEnabled(T)`；不 memcpy payload，按 `obj.init.copy` 的完整 descendant-generation 规则建立；summary含copy/destructor失败cleanup |
| `eh.throw_from` | `eh.throw_from @NewType(%args...), %active` | `ExceptionCreateSummary`, `ExceptionDestroySummary`, `Allocate(exception)`, `WriteMemory(exception)`, `BeginLifetime`, `RuntimeEffect(exception)`, `MayUnwind`, `Control` | `SC` | 最后operand必须是当前active owner token；新payload成功后把旧owner转移为cause，constructor失败则清理new record、release旧token并传播constructor exception，不保留第二条活跃ownership |
| `eh.rethrow` | `eh.rethrow %active` | `RuntimeEffect(exception)`, `MayUnwind`, `Control` | `SC` | consume token；复用同一记录并跳过当前 handler list |
| `eh.resume` | `eh.resume %active` | `RuntimeEffect(exception)`, `MayUnwind`, `Control` | `SC` | consume token；把 unmatched/cleanup 后异常交给外层 continuation |

这些 opcode 都是 terminator，没有 normal successor。析构、defer 或 cleanup 中再次试图让异常逃逸时，不得执行普通 `eh.resume` 覆盖当前异常；runtime 必须转入 fatal path。

## 13. Trap、fatal 与 verifier-only unreachable

| Opcode | 形状 | Effect | 阶段 | 语义 |
| --- | --- | --- | --- | --- |
| `rt.trap` | `rt.trap {kind = K}` | `RuntimeEffect(trap)`, `MayTrap`, `Control` | `SC` | 立即产生 trap completion；不可 catch；不运行 RAII/defer |
| `rt.fatal` | `rt.fatal {kind = K}` 或 `rt.fatal %exception {kind = K}` | `RuntimeEffect(fatal)`, `MayDiverge`, `Control` | `SC` | fail-fast 终止；可消费 active exception；不可恢复；不是 trap |
| `cf.unreachable` | `cf.unreachable` | `Control` | `SC` | verifier proof marker；没有合法运行时语义 |

`kind` 是 schema versioned enum，例如 `bounds`、`invalid_dynamic_state`、`pending_task_destroy`、`nothrow_violation` 与 `cleanup_unwind`；不是任意宿主字符串。PDB operation 根据 TargetContext 发生的目标 trap 可以直接进入与 `rt.trap` 等价的终止路径，不必先伪造一个可捕获异常。

trap/fatal 都不执行语言级 stack cleanup。runtime 观测钩子可以记录 origin、traceback 与 kind，但不能恢复执行或改变结果。

## 14. 阶段、扩展与未知 opcode

### 14.1 Stage plan node 的边界

`stage.*` plan node、dependent template 和 structured comptime selection 属于 StagedModule 的 ElaborationPlan/NormalizedTemplateTable，并由 [`05-staging-comptime.md`](./05-staging-comptime.md) 定义。它们不是可偷偷混入 Closed CFG 的普通 runtime opcode。`DeclarationSink` 是 effect taxonomy；它既可描述 plan commit，也可描述下节显式 Staged-only typed Core semantic emission。

typed Core 中的 `SC` opcode 在 StagedModule 内仍使用本文语义：

- 全部 operand Known 且 handler/capability 允许时，ComptimeWorld 可以执行；
- 否则 ResidualizeWorld 按 schema 生成等价 residual operation；
- 进入 ClosedModule 前，meta value、开放声明句柄和 stage plan node 必须全部消失；
- `TargetDependent` operation 只有在相同 TargetContext 下才可 fold，所得值受 TargetKey 约束；其中只有 `PdbBoundary` 禁止跨改变动态执行性的控制边界推测。

### 14.2 Staged-only typed Core `ct.*`

v0 注册一个 `ct.register_module_item` opcode，其精确 record/identity/frozen-value规则见 [`05-staging-comptime.md`](./05-staging-comptime.md) 与中央 schema registry。它有两个一元无结果 form：单标量/unit operand `T`，或 address-only operand `!place<ro,T>`；后者在执行时读取一个 Known 逻辑快照。payload 含 compiler-derived source-backed callsite key。

该 operation 的阶段为 `S`，effect 为 `DeclarationSink(module_registration)`，address-only form 另有 `ReadMemory(typed)`，trait 为 `OrderedSemanticEmit`。只有 active decorator application 的 ComptimeWorld 能执行并向当前 pending batch产生一条记录；它不可 residualize、删除、CSE、复制、推测或重排。Closed verifier 无条件拒绝其 opcode，即使结果记录已经存在或 operation 位于不可达 block。结构化 output record是 module entity，不是留下该 opcode 到 RuntimeWorld执行。

### 14.3 Runtime 扩展入口

以下命名空间具有独立规范，不在本文重复展开：

| 命名空间 | 规范范围 |
| --- | --- |
| `async.*` | Task 构造、drive、await、publish、cancel 与组合 |
| `reflect.*` | pinned snapshot、lookup、typed operand/private adapter descriptor、destination 与动态调用；不存在 Core `DynamicRef` type/value |
| `decorator.*` | continuation region 与 fresh activation |
| `cast.class_*` / `cast.interface_*` / `cast.try_*` | class/interface 地址调整与 checked dynamic cast |
| `rt.version.*` | hot reload pin、transfer 与 stable entry 生命周期 |
| `abi.*` | C ABI 与其他显式外部 bridge |

实现不得把这些具有所有权、版本或异步语义的 operation 统一替换成无法恢复语义的任意 `call.runtime`。

### 14.4 Registry 演进

opcode identity 由 namespace、name 和 schema version 决定。反序列化器遇到未知 opcode 时：

- 若模块 header 未声明并协商对应 required feature，拒绝模块；
- 不得因为结果未使用而跳过未知 opcode；
- 不得相信未知 attribute 声称它 pure、nothrow 或无副作用；
- optional debug/annotation record 必须使用与 executable opcode 分离的可忽略记录通道。

## 15. Closed verifier 最小清单

除了各表逐项规则，Verified Closed InkIR 至少满足：

1. 所有 opcode 已注册且阶段为 `SC`/`C`，不存在 `stage.*`、`ct.register_module_item`、meta value、开放 symbol 或 host capability；
2. 每个 block 有且仅有一个 terminator，不存在隐式 fallthrough，所有 successor arguments 类型准确；
3. SSA 只承载获准的单标量/builtin `unit`/内部 token/place，address-only 值没有聚合 SSA 快照；TypeTable 不存在零元素 tuple record，普通 value type 只接受 `ValueAccess`，place 才能接受 `PlaceAccess.init`；
4. 所有 `TargetDependent` type、layout、PDB 与 strict float operation 使用与模块 TargetKey 完全一致的 TargetContext；只有 `PdbBoundary` 应用不可跨控制边界推测规则；任何需要 sizeof/alignment、typed dereference、typed access 或 object lifetime 的 operand 都满足 `SizedObjectType`，unsized-pointee pointer 只进入获准的 address/byte operation；revision 1 没有非默认 address-space extension，所有非零 address space 都拒绝；
5. `mem.load/store`、raw memory、place 投影、enum variant 与 slice bounds 满足准确 schema；
6. `obj.init.begin`、子对象初始化、`obj.init.commit`、outward-unwind failure transition、assign、destroy 和 runtime-owner release 的数据流状态闭合；transaction identity 形成无重复/非法重叠的严格树，child commit/rollback 不误提交 parent，且不存在伪造的 `obj.init.abort` 或 `mem.dealloc`；
7. noncopyable 值没有 hidden copy/move，typed copy 没有被 raw memcpy 替代；
8. 所有 may-unwind 同步调用使用 `call.invoke`，nothrow call 没有 outward unwind；
9. 每条 invoke unwind path 接收一个 exception token，并在每条动态路径上恰好转移或消费一次；
10. trap、fatal、unreachable、PDB trap 与普通 exception propagation 没有混同；
11. 每个 symbol、block argument 和 operation 都具有有效 origin provenance；
12. effect/alias、speculatability 与 callee summary 由 schema 重新推导，不能由输入文件伪造；
13. 每个 `object` parameter 的 entry type 是 Alive owner `!place<rw,T>`，每条 caller 路径上的 prepared parameter obligation 恰好由失败 cleanup 或 activation handoff 消费，handoff 后由 callee 恰好清理一次；
14. GlobalRecord 的 policy/ordinal/dependencies 能唯一重建无环、连续、stable-order 的 `ModuleInitializationPlan`；需要 finalization 的 global 即使没有动态 initializer 也进入 plan，thread-local policy 只能是 `eager_thread_activation` 且 lifecycle functions nothrow。
