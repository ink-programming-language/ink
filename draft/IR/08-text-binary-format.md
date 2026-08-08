# InkIR 文本、二进制与缓存格式

> 状态：已确认设计；格式仅供编译器内部使用，不构成公开输入格式、分发格式或稳定 ABI。

本文只规定逻辑 InkIR 的规范表示，不重新定义 type、opcode 或调用语义。module 实体和 origin 见 [`01-architecture.md`](./01-architecture.md)，type/value/memory 见 [`02-types-values-memory.md`](./02-types-values-memory.md)，Core opcode schema 见 [`03-core-instructions.md`](./03-core-instructions.md)，staging 见 [`05-staging-comptime.md`](./05-staging-comptime.md)，调用与 runtime 抽象见 [`06-calling-convention-runtime.md`](./06-calling-convention-runtime.md)，加载后的验证见 [`07-verification-passes.md`](./07-verification-passes.md)。所有数字 tag、required payload 字段顺序、semantic projection 和 exact canonical text grammar 的唯一 source of truth 是 [`10-schema-registry.md`](./10-schema-registry.md)；本章示例必须按该 registry 生成。

## 1. 范围与设计目标

本章定义三件彼此相关但职责不同的内容：

- canonical text：供 `--emit=ink-ir`、调试、golden test 和差异比较使用的确定性文本 dump；
- sectioned binary：供同一精确编译器版本读取的内部缓存容器；
- cache identity：决定一个缓存产物可否复用的版本、目标、源码和 comptime 依赖身份。

v0 明确采用以下约束：

1. 文本与二进制都不是用户可依赖的长期接口；升级编译器可以同时升级两者。
2. v0 只实现 canonical text printer，不实现以 InkIR 文本作为编译输入的 parser。
3. `--emit=ink-ir` 只输出 `ClosedModule[target]`。若实现 Staged dump，应使用独立的实验选项，例如 `--emit=staged-ink-ir`，并清楚标为内部调试输出。
4. 二进制格式是 Ink 自有的小端、分节、长度前缀容器，不复用 C++ 对象布局、LLVM bitcode 或宿主内存 dump。
5. 所有从磁盘加载的二进制产物先经过边界安全解码，再经过对应的 Staged 或 Closed verifier，未经验证的 operation 不得执行、优化或 lowering。
6. 同一语义输入、相同版本集合、相同 `TargetKey` 和相同受跟踪依赖必须产生逐字节相同的 canonical text 与 canonical binary。

canonical text 的主要价值是可审查性；binary 的主要价值是加载速度。两者表达同一个逻辑 module，但不要求通过相同的中间对象实现编码。

## 2. Canonical text 的文件级约定

canonical text 必须满足：

- 编码为 UTF-8，无 BOM；
- 换行固定为 LF；
- 文件以一个 LF 结束；
- 缩进固定为两个 ASCII 空格；
- 不输出行尾空白；
- 不输出注释、指针地址、线程号、arena index、哈希表迭代顺序或计时信息；
- 所有集合、映射、属性和符号在打印前按本章规则规范排序；
- canonical 模式完整打印 `OriginTable`；operation 使用 `loc(#oN)`，symbol、block、block argument、template 和 plan node 使用 `origin(#oN)`；
- 隐藏 origin、缩写大型常量或只打印 CFG 片段的输出属于 diagnostic view，不是 canonical text，也不得参与 golden 或语义 hash。

文件开头必须声明文本语法版本和产物种类：

```text
ink-ir 0.1 closed {
  ...
}
```

`closed` 和 `staged` 是仅有的 v0 artifact kind。它们分别对应 `ClosedModule[target]` 和完整的 `StagedModule` 容器。canonical printer 不得把含 `ElaborationPlan` 或 deferred template 的 module 标成 `closed`。

### 2.1 文件级结构

完整逻辑顺序固定为：

```text
Header
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
NormalizedTemplateTable   // staged only
ElaborationPlan          // staged only
```

空表仍然打印。这样做使 artifact kind、验证边界和 diff 不依赖 printer 是否省略空项。

Header 必须恰按 10 §15.2 的 FieldId 1—30 顺序完整打印，不能使用“至少”列表、省略默认值或把 digest 移到自由 extension。revision 管理 schema/语义规则版本，对应 SHA-256 字段管理本次构建规范化后的实际配置；revision 相同但授权集合、handler set/config、pass 序列或 registration encoding 不同仍必须产生不同 artifact/cache identity。

Staged 和 Closed artifact 都必须具有完整 `TargetKey`。受限 import-selection profile 发生在 `StagedModule` 建立之前，不序列化为本格式的 Staged artifact；这避免同一种 artifact 在执行到一半时才补入目标身份。任何 target-dependent type、PDB、布局结果或 comptime 值都只能在该 TargetKey 下解释。

`module_content_sha256` 是该逻辑 module 的规范源码/语义输入摘要，不是包含该字段的文本或 binary 文件自哈希；binary 完整性由 `ContainerDigest` 单独负责，避免循环定义。

## 3. 词法与名称

### 3.1 保留符号

canonical text 使用以下 sigil：

| 形式 | 含义 | 编号作用域 |
| --- | --- | --- |
| `@name` | canonical symbol reference | module |
| `%vN` | SSA value；entry receiver/参数/hidden channel 都是 block argument，另含其他 block argument 和 operation result | function/独立 CFG region |
| `^bbN` | block | 所属 region |
| `!tN` | type table entry | module |
| `#aN` | attribute table entry | module |
| `#cN` | constant table entry | module |
| `#sN` | source file entry | module |
| `#oN` | origin DAG entry | module |
| `#depN` | dependency manifest entry | module |
| `#tmplN` | normalized template entry | Staged module |
| `#pN` | elaboration plan node | Staged module |
| `#regN` | committed module registration record | module |

`N` 是无前导零的十进制非负整数。表 ID 在各自 module 内从零连续编号。value ID 按规范定义顺序编号；block ID 按规范 block 顺序编号。这些打印 ID 只是 canonical artifact 内引用，不是跨构建语义身份。

下列内容不得用作任何可序列化 ID：

- C++ 对象地址、`Type*`、`Operation*` 或 vtable 地址；
- arena 分配顺序和进程内 `DeclId`；
- fixed-point round、worker 编号、任务完成顺序；
- 当前 checkout 的绝对物理路径；
- comptime 虚拟地址或宿主文件句柄。

### 3.2 标识符和 symbol path

operation 名称必须是带命名空间的 ASCII token：

```text
opcode = opcode-segment "." opcode-segment { "." opcode-segment }
opcode-segment = [a-z][a-z0-9_]*
```

例如 `const.int`、`arith.add`、`mem.load`、`cf.cond_br`、`obj.init.begin`、`async.task.publish_success` 和 `stage.force_value`。opcode 至少包含一个点；最后一段是 operation 名，其余各段组成命名空间。无命名空间 opcode 不合法。

symbol path 的人类可读部分采用：

```text
@canonical.module::Type
@canonical.module::function#7C318A2E7C318A2E7C318A2E7C318A2E7C318A2E7C318A2E7C318A2E7C318A2E
@canonical.module::function#7C318A2E7C318A2E7C318A2E7C318A2E7C318A2E7C318A2E7C318A2E7C318A2E::local_helper
```

其词法结构为：

```text
symbol-reference = "@" module-path { "::" symbol-component }
module-path = path-segment { "." path-segment }
symbol-component = path-segment [ "#" symbol-key-digest ]
path-segment = bare-segment | quoted-segment
bare-segment = [A-Za-z_][A-Za-z0-9_]*
symbol-key-digest = 64 个大写十六进制数字
```

至少必须有一个 symbol component。不匹配 bare 规则的 segment 必须用双引号包围。所有源语言标识符在进入 symbol identity 前必须已经通过语言规定的 NFC 检查。quoted segment 的转义规则与普通 Unicode 字符串相同。只有 `SymbolKey` 恰好由 module、nominal path 和 declaration kind 唯一决定时才省略后缀；overload、closed generic instance、signature-dependent symbol 和其他扩展身份必须打印完整 SHA-256，不得截短。verifier 从完整 `SymbolKey` 重算并核对该后缀。

文本中可读 path 不是 `SymbolKey` 的唯一字段。SymbolTable entry 还必须显式打印声明种类、canonical module identity、规范签名、已闭合 generic arguments、source-backed declaration key 和必要的 overload key。需要 digest 后缀时，printer 必须从这些字段导出包含 64 个大写十六进制数字的完整 SHA-256 后缀；canonical text 不允许截短。verifier 以完整 `SymbolKey` 重算结果为准。

### 3.3 字符串和字节串

Unicode string 使用双引号并采用唯一转义：

- `\"` 和 `\\` 分别表示引号和反斜杠；
- `\n`、`\r`、`\t`、`\0` 表示四个常用控制字符；
- 其他 ASCII 控制字符和所有非 ASCII Unicode scalar 使用 `\u{HEX}`；
- `HEX` 使用大写、无多余前导零；surrogate code point 和大于 `0x10FFFF` 的值非法；
- canonical printer 不直接输出非 ASCII 字节，从而避免终端编码和 Unicode 显示差异；解码后的字符串必须是有效 Unicode scalar sequence。只有语义上属于 Identifier 或 symbol path segment 的字段要求 NFC，普通字符串常量不得被静默规范化。

任意字节只能用 byte blob 表示：

```text
hex"00017FFF80FE"
```

每个字节恰好两个大写十六进制数字，不允许分隔符、奇数位数或省略前导零。源文件内容、对象文件和宿主内存不得为了方便而伪装成 Unicode string。

### 3.4 Printer grammar 的边界

v0 虽不提供 text parser，printer 输出仍由确定语法约束。共同 token 为：

```text
decimal = "0" | [1-9][0-9]*
hex-digit = [0-9A-F]
value-ref = "%v" decimal
block-ref = "^bb" decimal
type-ref = "!t" decimal
type-use = builtin-short | type-ref | inline-capability
inline-capability = "!place<" access "," type-use ">" | "!exception" | "!runtime-handle<" runtime-handle-kind ">" | "!runtime-object<" runtime-object-kind { "," type-use } ">"
attribute-ref = "#a" decimal
constant-ref = "#c" decimal
source-ref = "#s" decimal
origin-ref = "#o" decimal
dependency-ref = "#dep" decimal
template-ref = "#tmpl" decimal
plan-ref = "#p" decimal
artifact-kind = "staged" | "closed"
```

binary 中任何 type-use 都仍是 TypeId。inline-capability 只允许 place/exception/runtime_handle/runtime_object，并必须结构精确匹配 TypeTable 唯一 record；它不隐式创建或省略 record。其他 composite type 只能打印 `!tN`。角括号内逗号后无空格；完整 token、file/table/function/operation EBNF 以 10 §15 为准。

operation/signature 中的 canonical type use 固定为：

```text
type-use = builtin-short | type-ref | inline-special-type
inline-special-type = "!place<" access "," type-use ">"
                       | "!exception"
                       | "!runtime-handle<" runtime-handle-kind ">"
                       | "!runtime-object<" runtime-object-kind { "," type-use } ">"
access = "init" | "ro" | "rw"
```

`builtin-short` 仅是 registry 固定的 `i32`、`bool`、`ptrsize` 等 builtin spelling。pointer、reference、slice、array、tuple、nominal、function 和 interface-reference 等其他复合类型一律打印 `!tN`。inline capability/runtime-object spelling 也必须结构精确匹配 TypeTable 中已存在的唯一 record；text 不得借 inline spelling 创建未列出的类型，binary 仍统一编码对应 TypeId。其他章节 schema 表中的 `ptr<T>`、`slice<T>` 等是类型约束 metavariable，不覆盖本节 canonical printer 规则。

空白、逗号、等号、冒号、花括号和换行由各 record/opcode schema 固定，而不是任意 pretty-print 选择。record 外壳、table 顺序和 operation envelope 由本章定义；`schema-payload` 由中央 opcode registry 定义。未来若增加 text parser，它必须只接受声明版本的完整 grammar，并在构造 unverified module 后运行与 binary decoder 相同的 verifier；不能把未知 opcode 当作 opaque operation。

## 4. 精确常量表示

### 4.1 整数、布尔和索引

语义整数按其确切 bit pattern 打印，而不是按宿主整数或可能丢失宽度的十进制打印：

```text
#c0 = int 0x0000002A : i32
#c1 = int 0xFFFFFFFE : i32
#c2 = int 0xFF : u8
#c3 = bool true : bool
```

十六进制位数必须恰好等于 `ceil(bit_width / 4)`；最高半字节中超出 bit width 的位必须为零。signedness 来自类型，常量本身不重复声明补码解释。非语义的表计数、ID、对齐值和长度使用无前导零的十进制无符号整数。

### 4.2 浮点

浮点常量必须按格式和准确 bits 打印，不使用十进制浮点 spelling：

```text
#c4 = float bits<0x3F800000> : f32
#c5 = float bits<0x7FC00001> : f32
#c6 = float bits<0x8000000000000000> : f64
```

bit 数量必须与类型格式一致。这样可以区分 `+0`、`-0`、NaN payload 和 sNaN/qNaN。合法性及 TargetContext 的 NaN/PDB 语义由 constant verifier 和执行模型检查，printer 不进行十进制重舍入。

### 4.3 聚合、重定位和 target blob

聚合常量按逻辑元素打印：

```text
#c7 = tuple [#c0, #c3] : !t12
```

不得把宿主结构体的 padding、对象头或 vptr 原样序列化为语言聚合。padding 默认不是语义值。

允许的常量地址必须是可验证重定位，而不是宿主地址：

```text
#c8 = reloc(@canonical.module::global, addend = -0x20) : ptr<ro, !t4, 0>
```

addend 必须在目标地址宽度内精确可表示。target-specific byte blob 必须同时记录产生它的 `TargetKey`、endianness、逻辑类型和布局 hash；换一个 TargetKey 后不得复用。

## 5. Operation、CFG 与 table 的文本形态

### 5.1 通用 operation envelope

每个 opcode schema 定义自己的 operand、result、attribute、region 和 successor 形态，但共享以下 envelope：

```text
[result-list " = "] opcode schema-payload [attribute-dict] type-signature loc(#oN)
```

规范规则如下：

- 单结果写为 `%v3 = ...`，多结果写为 `(%v3, %v4) = ...`，零结果省略等号；
- operand 之间使用逗号和一个空格；
- attribute dict 使用 `{key = value, ...}`，key 按 UTF-8 字节序排列；
- operation 的 type signature 必须足够使 operand 与 result type 可独立验证；
- successor 明确写 block 和 edge arguments；
- nested region 使用花括号并按两个空格递增缩进；
- 不含 nested region 的 operation 固定占一个物理行，不因行宽或参数数量自动折行；nested region 只按 schema 规定的位置换行；
- 每条 operation 以非空 `loc(#oN)` 结束；effect、`MayUnwind`、`MayTrap` 和 speculatability 不作为可覆盖 schema 的自由属性打印。

下列是位于 canonical Function body 内的 CFG fragment；完整 Function record 外壳、entry binding 和 wrapper 必须由 10 §15.4 补齐：

```text
      ^bb0 origin(#o6):
        %v2 = arith.add %v0, %v1 : (i32, i32) -> i32 loc(#o7)
        %v3 = arith.cmp %v2, %v0 {predicate = sgt} : (i32, i32) -> bool loc(#o8)
        cf.cond_br %v3, ^bb1(%v2), ^bb2(%v0) : (bool) -> () loc(#o9)
      ^bb1(%v4: phi(0) i32 origin(#o10)) origin(#o11):
        cf.return %v4 : (i32) -> () loc(#o12)
      ^bb2(%v5: phi(0) i32 origin(#o13)) origin(#o14):
        cf.return %v5 : (i32) -> () loc(#o15)
```

block argument 是 SSA 定义。InkIR 不打印或编码独立 `phi` instruction。对于 place/reference/slice/interface/DynamicRef 等带 verifier-only authority/generation 的值，每个 successor argument 把该 edge 的物理 SSA value 与 side facts 作为不可拆分的 incoming pair；decoder/verifier据 edge 顺序为目标 block argument 重建 edge-indexed generation phi。canonical binary 不单独保存 generation number，但必须保存完整 edge operand和类型，不能把一条 edge 的 address/descriptor 与另一条 edge 的 generation 合并。owner authority 的 meet 只有在每条 owner incoming edge 的 lifecycle/cleanup obligation 已显式 discharge，或已转移给另一支配 owner/cleanup state 时才能降为 borrow；否则 join 非法，不能靠 meet 丢失 owner。所有 incoming 都转移完整、未别名 owner obligation 时结果才仍为 owner；永不从 borrow 升级。`CapabilityRebind` 消费动态选中的 incoming generation。

FunctionTable 的参数必须逐项打印 block-argument role/index 与 `parameter_passing_mode`，结果必须打印 `result_passing_mode`，不能由类型布局或函数体反推。exact grammar 见 10 §15.4；摘要为：

```text
parameter = value-ref ": parameter(" index ") " parameter-passing-mode " " type " " origin
parameter-passing-mode = "value" | "object" | "const_reference" | "mutable_reference" | "raw_pointer"
result-spec = "value " type | "result_destination " type | "void " ("void" | "never")
```

entry receiver、parameter、hidden result destination、async `task_self`/`task_result_storage` 与 module initializer/finalizer `global_lifecycle` 都是带 `BlockArgumentRoleTag + RoleIndex` 的 entry block arguments；没有独立 parameter ValueId。固定顺序、类型、authority 和 text spelling 见 10 §11.2/§15.4。`value` result 只用于单标量 SSA 或 unit，`result_destination` 只用于 address-only 逻辑结果，`void` transport mode 保存的逻辑类型仍必须明确是 `void` 或 `never`。Function record 还按统一 schema 显式打印 callable/function/receiver kind、calling convention、entry identity 和行为 contract；这些字段即使能从 body 猜出也不得省略。constructor 的 FunctionSignature 固定为 logical `void` + result mode `void`，其 initializing destination 由 call operation 的独立 destination role 表示。

stable Function record 同时打印 lineage-baseline `StableEffectEnvelope`、重算的 `StableEffectEnvelopeHash`、当前 `BehaviorContracts` 与 `BehaviorContractDigest`。envelope/hash 在 code-only compatibility lineage 内固定；behavior contract 的 nothrow/construction-nounwind/body-no-fail 位只可从弱到强，后续发布不得减弱。exact body summary 必须同时满足 envelope 与当前 contract。上述字段全部进入 semantic projection；依赖接口摘要必须覆盖被调用方实际 contract digest，不能让新调用者的 nounwind/no-fail 证明在未来 publish 中失效。

调用形态遵循调用约定章节定义的 opcode，而不是把所有调用压成一个含不可信 flags 的通用 call：

```text
%v2 = call.direct @canonical.module::add(%v0, %v1) : (i32, i32) -> i32 loc(#o20)
call.direct @canonical.module::make(%v0) to %v3 {destination_role = initializing_receiver} : (i32) -> !t9 loc(#o21)
%v4 = place.as_alive %v3 : (!place<init,!t9>) -> !place<rw,!t9> loc(#o22)
call.invoke callee_kind = direct @canonical.module::work(%v0) normal ^bb1(result(0)) unwind ^bb2(exception) : (i32) -> i32 loc(#o23)
async.call callee_kind = direct @canonical.module::fetch(%v0) to %v6 {destination_role = result} : (i32) -> !runtime-object<task,i32> loc(#o24)
```

上例的 inline `!runtime-object<task,i32>` 必须结构精确匹配 TypeTable 中唯一 kind=`runtime_object` 的 address-only `Task<i32>` record；inline spelling 不省略该 record。record 显式编码 runtime kind、ordered type arguments 和 `RuntimeStorageAbiHash`，size/alignment 由 TargetContext 重算，不把 runtime 私有字段编码成普通 aggregate。它不能使用 kind=`runtime_opaque` 或 `runtime_handle` 冒充可占 storage 的 Task。具体 operation schema 必须拒绝不匹配的 payload，例如可 unwind 调用缺少 unwind successor、address-only 结果没有 `to` destination、或者把 direct callee 填入 indirect schema。

带 destination 的 call/async/reflection operation 仍不产生 address-only logical SSA result。normal postcondition 把 destination 的抽象对象状态变为 Alive；随后单独打印的 `place.as_alive` 只物化这一已验证事实。对于 invoke 形态，该 operation 写在 normal successor 内，不能出现在 unwind successor。binary 不编码 `committed_destination` 一类隐式 edge sentinel；`place.as_alive` 具有自己的 OpcodeTag 和普通 operand/result payload。destroy 后的 `place.as_uninitialized` 只接受拥有重建权限的 rw place并物化 AllocatedUninitialized proof；rollback 后继续使用原 init capability fresh begin，不打印该 opcode。

两个 opcode 的 registry trait 都包含 `CapabilityRebind`：decoder/verifier 按 CFG path 重建 `PlaceCapabilityGeneration`，要求 source generation 被消费且旧 generation/派生 capability 没有后续 use。generation 是 verifier state，不分配可序列化 ValueId、ConstantId 或独立 binary record；operation、operand/result 与 trait tag 已足以确定性重建。printer 不能把 rebind 折叠为注释或同 ValueId 的隐式类型变化。

`ObjectLifetimeGeneration`、reference/interface/DynamicRef borrow generation 和 safe-slice `BorrowGenerationSet` 同样由 owner commit/destroy、`place.borrow`/`place.from_ref`/`place.deref`、`slice.*` 与 runtime-view schema 的 dataflow 重建，不编码可伪造 generation number。canonical binary 必须保留足以重跑该 verifier 的 opcode、edge、owner/borrow authority 和类型信息；raw pointer/symbol relocation 记录不携带 checked generation。

### 5.2 表和规范顺序

module 级表的 canonical 顺序是：

- source file：严格按 `(CanonicalModuleIdentity, SourceRole, LogicalPath UTF-8 bytes, ContentDigest)` 排序并保持 tuple 唯一；同内容但不同 logical path 仍有确定先后，重复 tuple 非法；
- origin：所有 parent 先于 child，然后按 origin kind 和结构字段排序；
- type：先按依赖 SCC 拓扑序，再按规范结构 key 排序；nominal type 用完整 `SymbolKey` 打破平局；
- attribute 和 constant：按类型化规范编码排序；
- symbol：按完整 `SymbolKey` 排序；
- global、function 和 runtime declaration：按被引用 symbol 的顺序；
- dependency：按 dependency kind、normalized access identity、handler revision 排序；
- template：按稳定 `TemplateIdentity` 排序；
- plan node：按 canonical work key 排序，而不是构建或执行完成顺序。

函数内 region 的 block 顺序固定为：entry 首先，然后对 successor 采用 opcode schema 定义的 edge 顺序进行稳定先序遍历，最后追加仍未访问的不可达 block 并按其结构 key 排序。不可达 block 的 key 包含规范 opcode/type/edge 结构、source-backed lowering role 和稳定生成 ordinal；不能以容器插入顺序打破平局。operation 保持 block 内语义顺序。ValueId 按 canonical block arguments（entry 已含参数）再按 operation results 的规范顺序连续分配；优化前对象地址不参与排序。

类型、attribute 和 constant uniquing 在内存中可以使用任意高效结构，但 printer 必须先得到规范结构 key，再统一分配 dense ID。并发插入先后不得改变输出。

## 6. Source origin 的文本表示

`OriginTable` 是必须存在的 provenance DAG，不是可选 debug line table。SourceRange 是文件内 UTF-8 字节的半开区间 `[Start, End)`。

一个完整示例：

```text
source #s0 = {
  module = "app.core",
  role = primary,
  byte_length = 1842,
  content_sha256 = hex"8A9F0B4C6C18E53D12A39D759C228F2A91A01A43E9C4C8C4374178A6037F2C10",
  logical_path = "src/core.ink",
  display_path = "src/core.ink"
}

origin #o0 = source(#s0, [42, 51))
origin #o1 = source(#s0, [220, 381))
origin #o2 = instantiation(generic_decl = @app.core::Box, request_site = #o0, closed_instance = @app.core::Box#A12F009BA12F009BA12F009BA12F009BA12F009BA12F009BA12F009BA12F009B, parent = #o0)
origin #o3 = region_expansion(control = #o0, selected_body = #o1, iteration = {ordinal = 2, identity_sha256 = hex"2233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF0011"}, parent = #o2)
origin #o4 = synthetic(reason = "implicit cleanup", parent = #o3)
origin #o5 = merged(primary = #o4, related = [#o0, #o2])
```

要求：

- 每个 symbol、block、block argument、template、plan node 和 operation 必须引用非空 origin；
- `Source` 节点必须引用存在的 source file，且 `0 <= Start <= End <= byte_length`；
- generated declaration 引用真实 source-backed template，不创建虚构源文件；
- `Instantiation`、`RegionExpansion`、`Synthetic` 和 `Merged` 的 parent/related 边必须无环；
- `RegionExpansion.selected_body` 引用 source origin，iteration 保存 ordinal 和 canonical identity digest；Closed origin 不反向依赖已被移除的 TemplateTable 或 meta-only ConstantTable entry；
- `Merged.primary` 保留诊断主位置，`related` 去重后按 origin 结构 key 排序；不得借 map 插入顺序选择 primary；
- source entry 必须携带内容 digest；canonical `display_path` 只能省略或从 canonical logical path 确定性导出，物理绝对路径不得进入 canonical text/binary；
- canonical semantic hash 默认排除 origin DAG 的展示结构和 display path，但缓存身份仍包含精确 source content digest，以免跨源码错误复用。

若用户请求不带 origin 的易读 dump，printer 可以提供 non-canonical view；该 view 必须在首行标明 `diagnostic-view`，且不得被 binary encoder 或 cache hasher 接受。

## 7. Staged artifact 的文本表示

Staged text 在 Typed Core tables 之后打印 `NormalizedTemplateTable` 和 `ElaborationPlan`。deferred template 保持 source-backed normalized form；不得伪装成已完成名称绑定和类型检查的 CFG。

示意：

```text
template #tmpl0 statement {
  identity = {module = "app.core", source_decl = "decl/7/body/2", content_sha256 = hex"00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF"},
  source = #s0,
  source_range = [220, 381),
  lexical_environment = hex"102132435465768798A9BACBDCEDFE0F102132435465768798A9BACBDCEDFE0F",
  captures = [type @app.core::T, comptime_value #c2],
  normalized_body = hex"4E4849520001"
} origin(#o31)

plan #p0 = stage.select_if {
  selector = core_value(@app.core::f, region = "body/entry", value = %v7),
  then_template = #tmpl0,
  else_template = #tmpl1,
  sink = statement(@app.core::f, region = "body/entry", anchor = #o30, position = before),
  work_key = hex"A0A1A2A3A4A5A6A7A8A9AAABACADAEAFB0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
} origin(#o32)
```

`normalized_body` 在文本中可以采用其专用的规范结构语法；上例用 byte blob 表示尚未展开的 schema 细节，不表示任意源码字符串。无论采用何种内部 representation，都必须具有版本字段、显式 capture、真实 source range 和规范 hash。若模板 schema 未实现可审查文本 printer，canonical Staged dump 可以拒绝输出，不能退化为指针或不确定 debug 字符串。

`stage.*` plan node 的语义由 staging 章节定义。Closed text 中出现 `template`、`plan`、meta-only type 或任意 `stage.*` plan node 都是 verifier 错误。

## 8. Binary container 总体布局

### 8.1 固定 header

v0 binary 使用小端字节序。固定 header 为 88 字节：

| Offset | 大小 | 字段 | v0 约束 |
| ---: | ---: | --- | --- |
| 0 | 8 | `Magic` | `49 4E 4B 49 52 00 0D 0A`，即 `INKIR\0\r\n` |
| 8 | 4 | `EndianMarker` | 小端编码的 `0x01020304`，文件字节为 `04 03 02 01` |
| 12 | 4 | `HeaderSize` | `88` |
| 16 | 2 | `ContainerMajor` | v0 为 `0` |
| 18 | 2 | `ContainerMinor` | v0 为 `1` |
| 20 | 1 | `ArtifactKind` | `1 = Staged`，`2 = Closed` |
| 21 | 1 | `HeaderFlags` | v0 必须为 `0` |
| 22 | 1 | `HashAlgorithm` | `1 = SHA-256` |
| 23 | 1 | `Reserved1` | 必须为 `0` |
| 24 | 4 | `SectionCount` | 受 decoder budget 限制 |
| 28 | 4 | `Reserved2` | 必须为 `0` |
| 32 | 8 | `DirectoryOffset` | canonical v0 必须为 `88` |
| 40 | 8 | `DirectorySize` | 必须等于 `SectionCount * 64`，检查溢出 |
| 48 | 8 | `FileSize` | 必须等于实际文件长度 |
| 56 | 32 | `ContainerDigest` | 整个文件的 SHA-256，计算时本字段视为全零 |

decoder 必须逐字段读取，不得把文件映射后 `reinterpret_cast` 为宿主 struct。`EndianMarker` 用来发现错误读取，不表示 v0 支持大端文件。

`ContainerDigest` 覆盖 header、directory、padding 和所有 section。缓存文件名中的外部 content digest 仍需独立核对，不能只相信文件内部声称的 digest。

### 8.2 Section directory

directory 紧跟 header，每项固定 64 字节：

| Offset | 大小 | 字段 |
| ---: | ---: | --- |
| 0 | 2 | `SectionKind` |
| 2 | 2 | `SectionVersion` |
| 4 | 4 | `SectionFlags` |
| 8 | 8 | `Offset` |
| 16 | 8 | `StoredSize` |
| 24 | 8 | `LogicalSize` |
| 32 | 32 | `SectionDigest` |

v0 `SectionFlags`：

- bit 0 `Required`；
- bit 1 `Semantic`；
- bit 2 `Compressed`，v0 必须为零；
- bit 3 `DebugOnly`；
- 其他位必须为零。

所有已知 v0 section 的 `SectionVersion` 固定为 `1`，cache decoder 要求精确匹配。v0 不压缩 section，因此 `StoredSize == LogicalSize`。directory entry 按 `SectionKind` 严格递增，不允许重复。第一个 section 位于 directory 末尾向上对齐到 8 字节的位置；后续 section 同样 8 字节对齐，间隙必须填零。offset/size 不得溢出、越界、重叠，也不得指向 header 或 directory 内部；最后一个 section 必须恰好结束于 `FileSize`，不允许未归属的尾随字节。

`SectionDigest` 是该 section 已存储字节的 SHA-256。即使 `ContainerDigest` 已验证，也单独验证 section digest，以便准确识别损坏并支持未来的受控按节加载。

### 8.3 Section kind

v0 registry 固定为：

| Kind | 名称 | Staged | Closed | 语义性 |
| ---: | --- | --- | --- | --- |
| 1 | `Manifest` | 必需 | 必需 | 是 |
| 2 | `Strings` | 必需 | 必需 | 是 |
| 3 | `SourceFiles` | 必需 | 必需 | 部分字段是 |
| 4 | `Origins` | 必需 | 必需 | provenance |
| 5 | `Types` | 必需 | 必需 | 是 |
| 6 | `Attributes` | 必需 | 必需 | 是 |
| 7 | `Constants` | 必需 | 必需 | 是 |
| 8 | `Symbols` | 必需 | 必需 | 是 |
| 9 | `Globals` | 必需 | 必需 | 是 |
| 10 | `Functions` | 必需 | 必需 | 是 |
| 11 | `RuntimeDeclarations` | 必需 | 必需 | 是 |
| 12 | `Dependencies` | 必需 | 必需 | 是 |
| 13 | `NormalizedTemplates` | 必需 | 禁止 | 是 |
| 14 | `ElaborationPlan` | 必需 | 禁止 | 是 |
| 15 | `OptionalDebugMetadata` | 可选 | 可选 | 否 |
| 16 | `ModuleRegistrations` | 必需 | 必需 | 是 |

必需表为空时也编码 count 为零的 section，并且 directory entry 必须设置 `Required`。表中语义性为“是”或“部分字段是”的 section 同时设置 `Semantic`；`Origins` 设置 `Required` 但不设置 `Semantic`；`OptionalDebugMetadata` 只设置 `DebugOnly`。ModuleRegistrations 在 Staged/Closed 都 required，并至少含唯一 RegistrationSummary。Closed artifact 出现 13 或 14 必须拒绝；Staged artifact 缺少其中任一项也必须拒绝。unknown section 只有同时满足“不带 `Required` 且标为 `DebugOnly`”时才可按长度安全跳过；unknown semantic section 一律使缓存失效。

module cache writer 必须省略 `OptionalDebugMetadata`，避免相同 `FinalModuleKey` 因宿主展示信息而产生不同字节。显式 diagnostic binary 可以携带该 section，但它属于 non-canonical、non-cacheable 产物，必须设置 Manifest 的 `ContainerFlavor = Diagnostic`；cache loader 只接受 `ContainerFlavor = Canonical`。绝对物理路径也只能出现在这种诊断产物或进程内 sidecar 中。

`Manifest` 必须记录第 2.1 节列出的全部版本和身份，另外记录 binary container version、active module DAG、semantic option set、TargetContext 摘要、capability policy、comptime handler revision、pass pipeline revision、registration encoding revision、module registration set/interface digest 以及第 12.6 节定义的 `SemanticModuleDigest`；exact 30-field schema 只以 10 §3 为准。

## 9. Section 内部编码

### 9.1 基本整数和 record

除固定 header 和 directory 外，整数采用规范 ULEB128 或 SLEB128：

- unsigned count、ID、长度和 enum tag 使用 ULEB128；
- 有符号 relocation addend 等字段使用 SLEB128；
- 必须使用最短编码；超长、溢出、负零等非规范形式拒绝；
- decoder 在分配前检查 count、length 和乘加溢出。

`Manifest` 的 exact envelope 来自 10 §3：`FieldCount : ULEB128` 后接 30 个严格递增、恰为 1—30 的 `(FieldId : ULEB128, FieldLength : ULEB128, FieldPayload[FieldLength])`；没有 FieldFlags、optional field 或 extension field。`Strings` 使用第 9.2 节的专用紧凑编码；其余 table section 先编码 `RecordCount : ULEB128`，随后每个 record 使用 10 §11.1 的共同外壳：

```text
RecordCount : ULEB128
repeat RecordCount times:
  RecordKind : ULEB128
  RecordFlags : ULEB128
  RecordLength : ULEB128
  RequiredPayload : byte[RecordLength]
```

已知 `RecordKind` 的 required fields 按 10 固定顺序编码。required record 的 `RecordFlags` 必须为零且不允许 extension TLV；bit 0 `Optional` 只允许 Section 15，其他 bit 为零。只有 OptionalDebugMetadata 的 optional record 可以在 required prefix 后编码按 FieldId 递增的 `(ExtensionFieldId, ExtensionLength, Bytes)` 并跳过 unknown optional extension。required/semantic section 的 unknown RecordKind、unknown required field、重复/乱序、非零保留位或 record 未恰好消费 RecordLength 全部拒绝。

### 9.2 字符串、enum 和引用

`Strings` section 编码 `StringCount`，再编码每项的 `ByteLength + UTF8Bytes`。字符串先去重，再按 UTF-8 字节序排序；不允许 NUL 终止假设、非法 UTF-8 或非 NFC 标识符。其他 section 用 zero-based `StringId` 引用。

enum、RecordKind、type kind 和 opcode 使用 IR schema registry 中明确分配的稳定 tag，不使用 C++ enum 的物理值或字符串 hash。tag 0 保留为 invalid；同一 `IrSemanticsRevision` 内 tag 唯一且不复用，增删或改变 schema 必须更新 revision 和生成的 encoder/decoder golden。table reference 使用 ULEB128 ID 并逐一进行范围检查。schema 允许时可以前向引用，但最终必须全部解析；origin 例外，v0 要求 parent 先编码，以便在线验证 DAG。

### 9.3 精确数值

APInt 编码为：

```text
BitWidth : ULEB128
Bytes : byte[ceil(BitWidth / 8)]  // little-endian
```

`BitWidth` 必须大于零并受实现预算限制，最高字节未使用位必须为零。signedness 仍来自语义类型。

浮点编码为 `FloatFormatTag + ByteLength + ExactBits`，bits 按小端字节顺序存储，长度必须与 format registry 一致。NaN 不做 canonical payload 改写。byte blob 使用 `ByteLength + Bytes`。聚合逐逻辑元素编码，不包含宿主 padding。

### 9.4 Type、attribute、constant 和 symbol 图

type、attribute 和 constant 的 record 以 schema tag 开头，只编码逻辑字段。递归 type 图按 SCC 拓扑序分配 ID；同一 SCC 内按 nominal `SymbolKey` 或规范结构 anchor 排序。无法获得稳定 anchor 的匿名递归构造在 v0 不可序列化，应在更早阶段规范成带身份的类型或被 verifier 拒绝。

map attribute 的 key 按 canonical UTF-8 字节序编码，set 按元素规范编码排序；重复 key/element 非法。binary encoder 不得序列化 uniquing table bucket、hash seed 或 insertion order。

`SymbolKey` 必须按字段编码，而不是只保存 printer 展示字符串。至少包含声明 kind、canonical module identity、source-backed declaration key、规范函数/类型签名、closed generic arguments 和必要的 overload key。

### 9.5 CFG 和 operation

Function record 内嵌有序 region、block 和 operation。ValueId 只按 canonical region/block 顺序的 block arguments（entry 已包含 receiver/parameter/hidden channels）、再按 operation results 隐式分配；没有独立 parameter ValueId。普通 operand 和 `ExistingValue` edge argument引用该 ID。Function required payload 的 exact 顺序来自 10 §9.4：

```text
FunctionRecordKindTag
SymbolId
CallableKindTag
FunctionKindTag
ReceiverKindTag
ReceiverTypePresent + ReceiverTypeId
EntryIdentityTag
CallingConventionTag
TargetAbiTag
ParameterCount + (ParameterTypeId, ParameterPassingModeTag, ParameterOriginId)
LogicalResultTypeId
ResultPassingModeTag
BehaviorContracts
BehaviorContractDigest
StableEffectEnvelopePresent + StableEffectEnvelope
StableEffectEnvelopeHashPresent + StableEffectEnvelopeHash
AttributesPresent + AttributeSetId
AbiDigest
FunctionOriginId
BodyPresence + Regions
```

`ParameterOriginId` 与 `FunctionOriginId` 属于 non-semantic provenance projection，但在 canonical binary 中仍必需。每个 BlockArgument 依 10 §11.2 编码 `BlockArgumentRoleTag, RoleIndex, TypeId, OriginId`；entry 的 receiver/parameters/result_destination/task_self/task_result_storage/global_lifecycle 以及非 entry phi 由此可 round-trip，不从位置猜测后丢弃 role。`ResultPassingModeTag` 的 v0 值为 `Value`、`ResultDestination`、`Void`；decoder 必须逐值验证与 logical result type 的相容性，不能扫描 body、return 或 call site 反推。

每条 operation 至少编码：

```text
OpcodeTag
ResultCount + ResultTypeIds
OperandCount + OperandValueIds
OperationAttributeCount              // revision 1 必须为 0
OperationAttributeRefs               // 必须为空
RegionCount + Regions
SuccessorCount + (BlockId, EdgeArguments)
OpcodeSpecificPayload
OriginId
```

带 `to %destination` 的 call-like opcode 必须在 `OpcodeSpecificPayload` 显式编码：

```text
DestinationPresent : u8
DestinationValueId : ULEB128       // present 时
ConstructedTypeId : ULEB128        // present 时
DestinationRoleTag : ULEB128       // result | initializing_receiver
```

`result` role 的普通同步调用要求 callee logical result 与 `ConstructedTypeId` 相同且 `result_passing_mode = result_destination`；async 构造的 ConstructedTypeId 是 `Task<T>`，callee logical result仍是 T。`initializing_receiver` 只允许 constructor；constructor 自身仍是 logical `void`/mode `void`，文本 call 的 `-> T` 打印 constructed channel type T，不把 T 改写成函数逻辑结果。canonical text 一律显式打印 `{destination_role = result|initializing_receiver}`。SSA ResultCount/OperandCount 都不包含 destination；例如 async.await_copy 是 O1+D1，arrow T 为 constructed channel。

每个 `EdgeArgument` 以显式 tag 编码：

```text
ExistingValue(ValueId)
NormalResult(LogicalResultIndex)
UnwindException
```

`NormalResult` 和 `UnwindException` 是 `call.invoke`、`async.invoke` 等 terminator 的 schema-produced edge value，不分配前驱 SSA `ValueId`，也不能在其对应 successor 之外使用。canonical text 分别打印为 `result(N)` 和 `exception`。只有目标 block argument 获得普通 `%vN`；verifier 检查 produced edge value 的 kind、index、类型、唯一合法 successor 和目标 block argument 一一匹配。普通 `cf.br`/`cf.cond_br` edge argument 必须全部是 `ExistingValue`。

`OpcodeTag` 来自受 `IrSemanticsRevision` 管理的显式 registry。unknown opcode 必须拒绝，不能当成 opaque pure operation。effect、may-unwind、may-trap、stage legality 和 speculatability 必须由 opcode schema 与 attributes 重新推导；若为加载加速额外保存 effect summary，verifier 必须重算并要求完全相等，summary 不具有覆盖权。

block 必须编码 block origin、argument role/index/type/origin 和有序 operation。ValueId、block ID 和 region 顺序都不得来自内存地址。binary decode 完成后仍需执行 CFG、dominance、SSA、place/lifetime、call/unwind 以及 artifact-kind verifier。

### 9.6 Staged template 和 plan

`NormalizedTemplates` 中每个 record 按 10 §10.2 的 exact 顺序编码：

```text
TemplateKind
RegionKind
SourceDeclarationKey
SourceFileId + SourceRange
NormalizedHirSchemaVersion
NormalizedHirPayload
LexicalEnvironmentKey
CaptureCount + Captures
OriginId
SemanticDigest
```

capture record 包含 `CaptureKind`、canonical key、准确 semantic type、access mode 和 origin。`NormalizedHirPayload` 是 Ink 自有、版本化、bounds-checked 的结构编码，不是 C++ AST dump，也不是待重新 parse 的源码字符串；其内部所有 list 同样使用 count/length budget。decoder 必须重新计算 `SemanticDigest`，并验证 template 不含宿主 pointer、临时 arena handle 或未声明 capture。

`ElaborationPlan` 中每个 node 按 10 §10.3 的 exact 顺序编码：

```text
StageOpcodeTag
InputCount + tagged PlanInputs
TemplateRefCount + TemplateIds
TaggedSink
ParentElaborationContext
RequiredCapabilities : Vec<CapabilityTag>
DependencyPlanNodeIds
CanonicalWorkKey
OriginId
```

PlanInput tag 对应 staging 章节的 `Constant`、`PlanResult`、`CoreValue`、`TemplateCapture` 和 `InstanceArgument`。`CoreValue` 必须编码 owner SymbolId、region structural path 与 local ValueId；`TaggedSink` 必须编码 owner、source-backed insertion anchor 和 position。显式 dependency 列表必须与 `PlanResult` 引用推导出的边一致，plan graph 必须满足 Staged verifier 的环、类型、stage 和 sink 规则。

Closed container 禁止这两个 section。关闭 module 时仍需保留的诊断 provenance 已自包含在 `Origins` 中，不得通过 origin 反向保活 TemplateId、PlanNodeId、meta value 或开放 declaration handle。

### 9.7 Module registration

`ModuleRegistrations` 是 Staged/Closed 都 required 的 semantic section。其首条且唯一一条 RecordKind 1 `RegistrationSummary` 按 `RegistrationEncodingRevision : U`、`RegistrationCount : U`、`ModuleRegistrationSetDigest : D32`、`ModuleRegistrationInterfaceDigest : D32` 编码；revision 与两个 digest 分别匹配 Manifest fields 28—30，count 恰等于后续 RecordKind 2 数量。空注册集仍编码 summary，不能省略 section。summary 的 canonical text prefix 唯一为 `registration_summary`。

每条 RecordKind 2 `ModuleRegistration` 按 10 §10.4 的 exact 顺序编码：

```text
RegistrationIdentity : D32
RecordType : TypeId
Value : ConstantId
ProducerSymbol : SymbolId
DecoratorApplicationKey : D32
DecoratorApplicationOrderPath : Vec<U>
SourceBackedCallsiteKey : D32
DynamicControlPath : Vec<TaggedDynamicPathStep>
EmissionOrdinal : U
ProtocolSchemaDigest : D32
OriginId
```

`DecoratorApplicationOrderPath` 是 non-empty、odd-length 的 module-global `Vec<U>`。root `[N]` 按 canonical source-file order，再按文件内 lexical decorator-application traversal 分配 N；generated child 在 parent path 后追加 `(ParentSemanticOutputOrdinal, ChildLocalApplicationOrdinal)`。前者来自 parent transaction 全部 ordered semantic-output events 的统一序列，不是 registration-only ordinal；后者来自该 generated output 内 lexical application traversal。vector 按无符号数值逐元素 lexicographic order 比较，短真前缀先于 child。decoder 拒绝 empty/even-length path，Staged verifier 核对 source/output derivation；cache replay 必须重现这些 ordinal。

dynamic path tag/payload 唯一为：1 `Expansion(ExpansionContextKey)`；2 `Call(CallSiteKey, InvocationOrdinal)`；3 `Branch(ControlNodeKey, ArmOrdinal)`；4 `Loop(ControlNodeKey, IterationOrdinal, IterationIdentityDigest)`。binary 先写 tag 再写列出的 payload；text 唯一写成 `[expansion(DIGEST), call(DIGEST, U), branch(DIGEST, U), loop(DIGEST, U, DIGEST)]` 的实际 path 子序列。path 保持结构嵌套/执行顺序；call 的动态进入和 loop iteration 分别由 `InvocationOrdinal` 与 `IterationOrdinal` 确定，均不得由 worker、round、容器插入顺序或 digest 字节序代替。

`EmissionOrdinal` 是同一 active application 每次实际到达 `ct.register_module_item` 的 application-wide、从 0 连续递增 registration-arrival ordinal，不在 callsite/path 内重置，也不同于 parent transaction 的 `ParentSemanticOutputOrdinal`。只有 pending-batch commit/writer 前可以合并 replay duplicate：相同 identity 的候选必须是完整 logical record 逐字段相同，包括 canonical Origin structure，不能只比较 S fields 或 raw OriginId；否则是 determinism error。writer 输出与 canonical decoder 都要求 `RegistrationIdentity` 唯一、`(DecoratorApplicationOrderPath, EmissionOrdinal)` 唯一、同一 path 映射到同一 decorator key/producer，且每个 path 的 ordinal 恰为 `0..N-1`；decoder 对重复项直接拒绝，绝不去重或规范化。section items 严格按 `(DecoratorApplicationOrderPath, EmissionOrdinal, RegistrationIdentity)` 排序，identity 仅作最终 tie-break。identity 必须由 decoder/verifier 按 `H("ink.module-registration.v0", CanonicalModuleIdentity, ModuleContentDigest, ProducerSymbol.SymbolKey, DecoratorApplicationKey, DecoratorApplicationOrderPath, SourceBackedCallsiteKey, DynamicControlPath, EmissionOrdinal)` 重算；它是 module-content-local identity，不承诺跨 module version 保持不变。

`RecordType`/`Value` 必须满足中央谓词 `StaticRegistrationEncodable`。v0 frozen graph 仅允许 10 §10.4 列出的 closed、layout-complete typed constants 和受控 relocation；String 使用 module-owned immutable bytes/length，不运行普通 destructor。`ct.register_module_item` 只在 typed-Core comptime decorator context 执行，按 10 §14.7 的 S6/P34 schema 发射 pending record，并在 Closed IR 中消失；提交后的 Section 16 保留。

`ProtocolSchemaDigest` 的 tuple 必须包含 `RegistrationTypeSemanticIdentity`、`LogicalFrozenShape`、`TargetLayoutDigest`、`RegistrationEncodingRevision` 和 `RuntimeAbiRevision`。`ModuleRegistrationSetDigest = H("ink.module-registration.set.v0", OrderedModuleRegistrationSemanticProjections)` 覆盖 canonical ordered items 的完整 S projection；该 structured field 编码 `Count : U`，再逐项编码 `ProjectionLength : U + S projection bytes`。`ModuleRegistrationInterfaceDigest = H("ink.module-registration.interface.v0", SortedUniqueRegistrationSchemaTuples)`；该 structured field 编码 `Count : U` 后接 sorted unique `(RegistrationTypeSemanticIdentity : D32, ProtocolSchemaDigest : D32, TargetLayoutDigest : D32, RegistrationEncodingRevision : U, RuntimeAbiRevision : U)`，不含 identity/order path/value。tuple 按字段比较，D32 用 unsigned byte lexicographic order、U 用无符号数值 order，只有全字段相等才去重。code-only hot reload 只在 interface digest 相等时兼容，具体 registration set 与代码作为同一 module version 原子发布/替换。

## 10. 安全解码协议

decoder 把缓存文件当作不可信字节流。加载顺序固定为：

1. 在不分配大对象的前提下读取 88 字节 header，验证 magic、版本、保留位、实际文件长度和 digest；
2. 对 `SectionCount * 64` 做 checked arithmetic，验证 directory 边界；
3. 验证 section kind、flags、严格排序、对齐、零 padding、无重叠、长度和 digest；
4. 根据全局 budget 预检所有 table count、record length、字符串总字节数、aggregate arity、type/region 深度、CFG block/op/value 数和 APInt bit width；
5. 解码 Strings、Manifest 和各依赖 table，检查 canonical ULEB/SLEB、UTF-8、NFC、enum、ID 范围和 record 恰好消费；
6. 构造只能由 decoder 持有的 `UnverifiedStagedModule` 或 `UnverifiedClosedModule`；
7. 从 opcode registry 重建 effect 和 trait，运行完整 `verifyStaged` 或 `verifyClosed`；
8. 只有成功后才产生 `VerifiedStagedModule` 或 `VerifiedClosedModule<TargetKey>` 能力对象。

必须设置可配置但有硬上限的预算，至少覆盖：

- 文件和单 section 字节数；
- section、record、string、type、constant、symbol 数量；
- 单字符串和全部字符串字节数；
- APInt 最大 bit width；
- aggregate 最大元素数；
- type recursion、nested region 和 origin DAG 最大深度；
- function、block、operation、SSA value 和 edge argument 总数；
- template body、plan node 和 fixed-point request 数量；
- Section 16 的 `RegistrationCount`、单条与全部 `DecoratorApplicationOrderPath` 的 U 元素数、单条与全部 `DynamicControlPath` step 数及嵌套深度、frozen constant graph node/depth 和 relocation count。

每个 count/length 在任何 allocation、递归或乘加之前都必须同时通过两类上界：由当前 record/section `remaining bytes` 推导出的局部最大值，以及进程配置的 global hard cap；全部计算使用 checked arithmetic，满足其中一个上界不能替代另一个。

任何失败都不得导致越界、整数回绕、无限递归、超预算分配或执行未验证 operation。不得使用 `mmap + native struct cast`、序列化 vtable、`size_t`、裸指针、对象 padding 或宿主 enum 布局。

普通编译遇到缓存损坏、版本不匹配或 verifier 失败时，应原子地使该 entry 失效并从源码重建；它不是用户源码诊断。显式 cache validation 工具可以报告文件偏移、section 和失败规则。若编译器刚生成的未落盘 IR 无法通过 verifier，则按 compiler bug 报告，并带最小 IR slice 和 origin。

SHA-256 digest 提供内容完整性和寻址，不提供写入者认证。v0 module cache 只接受与当前编译进程处于同一信任域的本地 cache directory；不得把任意远程或低权限主体可写目录当作可信语义 cache。verifier 能证明 IR 结构和语义前置条件合法，不能证明合法 IR 一定由当前源码诚实编译而来。跨主体 remote cache 需要另行设计签名/认证 manifest 和密钥策略，不属于 v0 binary container。

## 11. 版本管理

以下版本必须独立记录，不得合并成一个“编译器版本”：

| 字段 | 管理内容 |
| --- | --- |
| `LanguageRevision` | Ink 源语言语义 |
| `IrSemanticsRevision` | type、opcode、attribute 和 verifier 语义 |
| `TextSyntaxVersion` | canonical text 词法、顺序和打印形式 |
| `BinaryContainerVersion` | header、directory 和 section 编码 |
| `CompilerBuildId` | 具体实现构建，包含会影响 elaboration/codegen 的代码 |
| `TargetAbiRevision` | TargetABI、布局、异常和 LLVM lowering 契约 |
| `RuntimeAbiRevision` | 私有 runtime entry、Task、reflection、dispatch 等契约 |
| `CapabilityPolicyRevision` | comptime 能力批准和归类规则 |
| `CapabilityPolicyDigest` | 本次构建准确、规范化的 capability 授权配置 |
| `ComptimeHandlerRevision` | 各 comptime effect handler 的语义 |
| `HandlerRevisionDigest` | 实际 handler identity/revision/config 集合 |
| `PassPipelineRevision` | 影响序列化阶段和结果语义的 pipeline 配置 |
| `PassPipelineDigest` | 本次构建实际 pass 序列及其语义选项 |
| `RegistrationEncodingRevision` | frozen registration value、relocation、path 和 protocol schema 编码语义 |

v0 cache 采用精确匹配策略：上述任一版本、`TargetKey` 或 semantic option digest 不同即 cache miss。无需尝试跨版本升级 binary。container major/minor 仅用于快速拒绝和未来迁移工具识别，不代表公开兼容承诺。

text printer 升级必须递增 `TextSyntaxVersion` 并更新 golden。因为 v0 没有 text input parser，旧 text 只作为历史调试产物保留；不能把能否读取旧 dump 当作 compiler compatibility。

## 12. 语义摘要和缓存键

### 12.1 Hash 基本规则

v0 使用 SHA-256，采用 domain separation 和长度前缀字段。逻辑形式为：

```text
H(domain, fields...) = SHA256(
  ULEB128(byte_length(UTF8(domain))) || UTF8(domain) ||
  for each field: ULEB128(byte_length(field)) || field
)
```

不得通过简单拼接可变长字符串构造 key。map/set 先按 canonical encoding 排序。hash 输入中的整数、字符串、type、constant、symbol 和 operation 使用与 binary 一致的逻辑规范编码，但不必复用其 section offset 或 table ID。

### 12.2 粗粒度 module content key

v0 选择 module-content-hash 粒度，不做 per-declaration binary cache。静态候选 key 至少为：

```text
StaticModuleKey = H("ink.module-cache.static.v0",
  CanonicalModuleIdentity,
  ModuleSourceIdentitiesAndContentDigests,
  ActiveModuleDagIdentitiesAndContentDigests,
  LanguageRevision,
  IrSemanticsRevision,
  TextSyntaxVersion,
  BinaryContainerVersion,
  CompilerBuildId,
  ArtifactKind,
  TargetKey,
  TargetAbiRevision,
  RuntimeAbiRevision,
  SemanticOptionsDigest,
  CapabilityPolicyRevision,
  CapabilityPolicyDigest,
  ComptimeHandlerRevision,
  HandlerRevisionDigest,
  PassPipelineRevision,
  PassPipelineDigest,
  SchemaRegistryDigest,
  RegistrationEncodingRevision)
```

`ModuleSourceIdentitiesAndContentDigests` 按第 5 节同一 source-file order 编码每个 `(SourceRole, LogicalPath UTF-8 bytes, ContentDigest)`，既覆盖所有参与语义的源码精确字节，也保留会影响 root application traversal 的 logical identity；`ActiveModuleDagIdentitiesAndContentDigests` 按冻结后的 canonical DAG 顺序编码。绝对物理路径、mtime、inode、checkout 目录、origin 的展示选择、diagnostic 色彩和并发数不得直接进入 key。

只把 mtime 当作读取内容 digest 的本地快速索引是实现细节；只要存在不一致，必须回退到内容校验，不能把 mtime 当语义身份。

### 12.3 动态 comptime dependency manifest

纯 comptime 计算没有动态依赖。受跟踪只读 effect 必须返回：

```text
DependencyRecord {
  DependencyKind
  NormalizedAccessIdentity
  ObservedValueOrContentDigest
  HandlerRevision
  ValidationMode
}
```

例如：

- 文件读取记录经 capability policy 规范化的访问身份和文件内容 digest，而不是只记录路径或时间戳；
- 环境变量读取记录变量身份、存在性和值 digest；
- 目录遍历记录规范排序后的名称、类型和所需内容摘要；
- 配置和工具资源记录 handler 能够重新验证的稳定 identity 与 snapshot digest。

若 handler 无法给出确定、可重新验证的 snapshot，effect 必须归类为 noncacheable，不能伪装成 tracked read。v0 的 network、process、clock 和 random 即使由构建配置显式授权也始终 noncacheable，与 staging 章节的 effect policy 一致。

加载采用两阶段协议：

1. 用 `StaticModuleKey` 找到零个或多个候选 manifest；
2. 按 manifest 的 canonical dependency 顺序重新验证当前观察值；
3. 计算 `DependencyManifestDigest = H("ink.dependency-manifest.v0", records...)`；
4. 只有全部 record 匹配时，才用下式查找并接受最终 artifact：

```text
FinalModuleKey = H("ink.module-cache.final.v0", StaticModuleKey, DependencyManifestDigest)
```

零动态依赖使用该 domain 下空 record 序列的真实摘要，不使用全零、缺省字段或省略 final-key 层。候选 manifest 按 digest 排序验证；若多个候选对当前观察都匹配，它们必须导向相同 `SemanticModuleDigest`，否则报告 cache determinism/corruption 并从源码重建。

manifest 本身也编码在 `Dependencies` section，并受 container 和 section digest 保护。active module DAG 冻结后，comptime effect 不得新增 import 或 module dependency；普通 tracked file/config read 只进入 dependency manifest，不改变 module DAG。

### 12.4 可观察写效果与缓存发布

产生外部可观察写效果的 comptime work item，包括文件写入、网络写入、进程控制以及 `build.log`，必须：

- 按 staging 章节规定的 canonical work-item 顺序执行和提交；
- 将包含该效果的本次 module elaboration 标为 noncacheable；
- 不发布 `FinalModuleKey` 对应的可复用 module artifact；
- 不把写入内容或日志保存为“下次命中时重放”的 effect log；v0 明确没有 effect replay。

纯计算和受跟踪只读 work item 可以并行并使用内部 memo，但其 commit 仍按 canonical key 排序。若同一 module 的另一个 work item 发生可观察写，整个最终 module artifact 不发布；实现可以保留与外部效果无关、身份完整的更低层纯 memo，但这不改变 v0 的 module cache 粒度。

`build.log` 输出到 stderr 或独立 build-event side channel，绝不能混入 `--emit=ink-ir` 的 stdout。compiler declaration transaction 的回滚只撤销待提交声明；已经发生的外部写效果不能假装被回滚，因此应尽量在事务验证成功后按规范顺序执行允许延迟的写。无法延迟的 handler 必须明确报告其提交边界。

被 residualize 到 RuntimeWorld 的 runtime effect 尚未在编译期发生，不会因此使 comptime cache 失效；其 opcode、handler/runtime ABI revision 和 residual 结构仍属于 IR 语义 hash。

### 12.5 语义 hash 的包含和排除

必须包含所有会改变 elaboration、解释执行或 AOT 结果的内容，例如：

- type、constant、symbol、global、function 和 operation 语义；
- schema-relevant attributes、strict/fast FP mode、PDB 和 target layout 结果；
- closed generic arguments、normalized template 内容和 plan identity；
- ordered module registration records、准确 frozen constant graph 与获准 symbol/static relocation；
- TargetKey、版本集合、semantic options、capability policy revision/digest、handler revision/digest 与 pass pipeline revision/digest；
- active DAG、source content digest 和 tracked dependency manifest；
- runtime semantic operations 和 private ABI revision。

默认排除：

- printer dense ID 的具体数值，只要规范结构相同；
- display path、绝对物理路径、颜色、诊断措辞和 build timing；
- origin DAG 的展示组织和 debug-only metadata；
- worker 数量、调度、内存地址和 hash seed。

origin 虽不进入 core semantic hash，SourceFileTable 的内容 digest 仍进入 cache key。这样既允许不影响执行语义的 provenance 规范化，也保证 source-backed template、诊断位置和生成身份不会跨错误源码复用。

### 12.6 字节完整性 digest 与语义投影

`ContainerDigest` 和 `SectionDigest` 只验证实际存储字节，必然包含 `OriginId`、Origins section、规范 display metadata 等 non-semantic 字段。它们不得直接充当语义相等性或 cache determinism 的摘要。

`SemanticModuleDigest` 使用独立、由同一 schema registry 定义的字段级 canonical projection：

- operation、block、function、symbol 和其他 record 的 `OriginId`/`origin`/`loc` 字段全部剔除，不以零值占用普通语义字段位置；
- `Origins` 与 `OptionalDebugMetadata` section 整体排除；
- `SourceFiles` 只保留 canonical module/source identity、source role、`LogicalPath` UTF-8 bytes、精确 content digest，以及 template/source-backed identity 明确要求的稳定 range 身份；display path、绝对路径、mtime 和展示名称排除；
- type、attribute、constant、symbol、global、function、runtime declaration、dependency、normalized template、plan 和 module registration 只编码各自 registry 标为 `Semantic` 的字段；
- `ModuleRegistrations` summary 只把 semantic `RegistrationEncodingRevision` 投影进去；每条 `ModuleRegistration` 的全部 S 字段（包括 typed frozen value graph 与 relocation）按 canonical structured order 投影。`RegistrationCount`、`ModuleRegistrationSetDigest` 和 `ModuleRegistrationInterfaceDigest` 都是 N 字段，只由 decoder/verifier 重算核对，不作为重复语义输入；
- table/dense ID 使用 10 §2.2 的 canonical graph projection：先求 semantic reference graph 的 SCC/condensation DAG，SCC 内以稳定 scalar key + edge-label refinement 排 local ordinal，内部引用编码 LocalBackRef，已完成 SCC引用编码 SccRef；完全对称时选择 lexicographically smallest SCC encoding；因此 recursive/互递归 type/function/symbol graph 有限终止，且仅重编号不会改变摘要；
- projection 仍包含 source content digest、`SymbolKey`、template structural identity、TargetKey、版本/配置 revision 与 digest，不能借排除 origin 跨错误源码复用。

逻辑定义为 `SemanticModuleDigest = H("ink.semantic-module.v0", ProjectSemanticFields(module))`。Manifest 保存并由 decoder/verifier重算该值；schema 中每个字段必须显式标为 `Semantic` 或 `NonSemantic`，禁止依赖“整个 section 都算语义”的模糊默认。完整文件逐字节确定性仍是 canonical writer 的额外要求，不能用投影摘要相同掩盖 printer/encoder 不确定性。

## 13. 缓存写入、并发和恢复

cache writer 必须：

1. 在目标 cache directory 内创建唯一临时文件；
2. 完整编码 canonical binary，计算 section 和 container digest；
3. 重新打开或从独立 reader 路径验证 header、digest 和最小结构；
4. 使用同一文件系统内的原子 replace/rename 发布到 `FinalModuleKey`；
5. 失败时删除临时文件，不留下可见的部分 entry。

两个进程并发生成同一 key 时，结果应逐字节一致。若已有合法 entry，后写者可以丢弃自己的临时文件；若相同 key 产生不同 `SemanticModuleDigest`，这是 determinism/compiler bug；即使 semantic digest 相同而 canonical binary 字节不同，也必须报告 canonical encoder nondeterminism，不能静默任选其一。

读取损坏 entry 时，只删除或隔离解析出的精确目标文件。不得对未验证的计算路径、空路径、cache root 或 workspace root 执行递归删除。cache miss、版本不匹配和依赖变化正常触发源码重建。

## 14. 测试要求

最小测试集必须包含：

- canonical text golden：同一 module 重复打印逐字节相同；
- 不同线程数、worklist 分片和 hash seed 下 text/binary 相同；
- Unicode identifier、转义、NFC、空字符串、控制字符和 byte blob；
- 任意 bit width APInt、负数 bit pattern、`+0/-0`、NaN payload 和 sNaN；
- origin DAG、半开 SourceRange、generated declaration 和 merged origin；
- Staged text 中 template/plan 完整性以及 Closed 中强制消除；
- 空/非空 `ModuleRegistrations` 的唯一 summary、四类 dynamic path text/binary round-trip、root/child application order path、application-wide ordinal 连续性、pending replay 去重，以及 canonical decoder 对重复 identity/path+ordinal 的拒绝；
- frozen relocation、非法 resource/reference graph，以及带固定 domain 的 set/interface digest 重算与 corruption negative；
- encode/decode 后 semantic round-trip，再打印得到相同 canonical text；
- header、directory、offset、长度、重叠、对齐、digest、unknown flag 和保留位负例；
- 非规范 ULEB/SLEB、越界 table ID、非法 UTF-8、过深 region/type/origin、超预算 count 的 decoder fuzz；
- unknown required/optional section 和 TLV 的版本行为；
- TargetKey、ABI、compiler build、semantic option、capability policy、handler set/config、pass pipeline、schema registry 和 registration encoding 的 revision 或 digest 任一变化导致 cache miss；
- tracked file/env/directory 依赖不变时命中、内容变化时 miss；
- observable comptime write 和 `build.log` 不发布 module cache、也不 replay；
- cache 临时写失败、截断、并发发布和损坏恢复；
- decoder 之后仍能被 Staged/Closed verifier 拒绝的结构合法但语义非法 corpus。

binary decoder 应持续进行 coverage-guided fuzz。任何新的 section、record kind、opcode 或 extension field 都必须同时增加长度预算、unknown-field 行为、round-trip 和损坏测试。

## 15. v0 实现顺序

建议按以下顺序实现：

1. 建立所有逻辑 table 的 canonical traversal 和稳定结构 key；
2. 实现完整 canonical text printer 与 golden tests，不实现 text parser；
3. 固定版本集合、`TargetKey`、source digest 和 semantic hash API；
4. 实现 binary writer、独立 bounds-checked reader 和 Level 0 decoder tests；
5. 接入 `UnverifiedModule -> verify -> VerifiedModule` 能力边界；
6. 实现粗粒度 module cache、原子发布和损坏回退；
7. 接入 tracked dependency manifest；
8. 最后开启 pure/tracked-read 并行 comptime memo，保持 canonical ordered commit；
9. 只有在真实工作流需要时再设计 text input parser 或跨版本迁移工具。

这一顺序先用可读文本锁定语义和确定性，再引入二进制与缓存复杂度。首个纵切片只需覆盖已实现的 C0 types/opcodes，但 header、origin、版本、安全解码和 cache identity 不能使用以后必然推翻的临时宿主表示。
