# InkIR v0 Schema Registry

> 状态：v0 规范性 source of truth
>
> 适用版本：`IrSemanticsRevision = 1`，`TextSyntaxVersion = 0.1`，`BinaryContainerVersion = 0.1`

## 1. 权威性与生成边界

本文是 InkIR v0 所有 schema 名称、数字 tag、binary required payload、canonical text 生成规则、effect、trait、stage legality 和 semantic projection 的唯一权威清单。逻辑语义仍由其他章节解释；一旦其他章节中的示意拼写、实现 enum 或手写 printer 与本文冲突，v0 codec/printer/verifier 以本文为准，并应修正文档冲突。

实现必须从同一份机器可读 registry 数据生成或构造以下消费者：

```text
SchemaRegistryV0
|- CanonicalTextPrinter
|- BinaryEncoder
|- BoundsCheckedBinaryDecoder
|- StructuralVerifier
|- OpcodeVerifier
|- EffectAndTraitDeriver
|- SemanticProjector
`- RegistryGoldenGenerator
```

不得分别手写互不校验的 text opcode switch、binary enum、verifier arity table 和 effect table。允许手写复杂语义 verifier callback，但 callback identity、输入 schema、tag 和调用入口必须由 registry 记录引用。

所有 tag 使用无符号整数，binary 中以 canonical ULEB128 编码。共同规则：

- `0` 永远保留为 `Invalid`，不得表示默认合法值；
- 同一 `IrSemanticsRevision` 内已经分配的 tag 永不复用，即使对应实体被删除；
- 改变 required payload 字段、字段顺序、类型约束、effect、trait、stage legality 或 canonical text form 必须提升 `IrSemanticsRevision`；
- 只改变空白和 printer 拼写还必须提升 `TextSyntaxVersion`；
- tag 是 binary identity，canonical name 是 text identity，decoder 必须核对二者属于同一 registry entry；
- 未知 required section、record、type、attribute kind、constant kind、enum tag、opcode 或 stage opcode 一律拒绝；只有 `OptionalDebugMetadata` 中明确标为 optional 的长度前缀记录可以跳过；
- cache loader 要求 registry digest、IR revision 与 container revision 精确匹配，不做启发式升级。

`SchemaRegistryDigest` 为本章所有带数字的表和 payload schema 的 canonical machine-readable projection 的 SHA-256。它进入 Manifest 和 cache compatibility key，但不替代 `IrSemanticsRevision`。

## 2. 编码记法与投影类别

### 2.1 Binary 字段记法

本章 payload 表使用：

| 记法 | Binary 编码 |
| --- | --- |
| `U` | canonical ULEB128 |
| `I` | canonical SLEB128 |
| `u8` | 一个字节 |
| `Bool` | `u8`，只允许 `0` 或 `1` |
| `D32` | 32 字节 SHA-256，不带长度 |
| `Str` | `StringId : U`，指向 Strings section |
| `Bytes` | `ByteLength : U` 后跟准确字节 |
| `Ref<T>` | 对应 table 的 zero-based ID，以 `U` 编码 |
| `Enum<T>` | 本章对应 enum tag，以 `U` 编码 |
| `Bits<T>` | 当前 enum 不超过 64 项时以 canonical ULEB128 bit mask 编码 |
| `Opt<T>` | `Present : Bool`，为 `1` 时紧跟 `T` |
| `Vec<T>` | `Count : U` 后按顺序编码 `T` |
| `APInt` | `BitWidth : U` 后跟 `ceil(BitWidth / 8)` 个小端字节 |
| `FloatBits` | `FloatFormatTag : U + ByteLength : U + little-endian bits` |
| `RegionPath` | `Vec<U>`，每项是父 region 中的 canonical child ordinal |

required payload 内没有宿主对齐 padding。outer record 的 `RecordLength` 必须恰好覆盖全部 required payload 和 extension TLV；decoder 不得用 C++ struct layout 读取。

本章所有 `H(domain, fields...)` 均使用 08 §12.1 的同一个 SHA-256 framing：UTF-8 domain 和每个 field 都先编码 canonical byte length；structured field 先按本 registry 的 binary schema（包括 `Vec` count、variant tag 和 required field order）形成唯一 bytes，再作为一个 length-prefixed field 输入。任何公式不得退化为裸拼接或实现私有对象布局。

### 2.2 Semantic projection

字段表的“投影”列只有三种值：

- `S`：进入 `SemanticModuleDigest`；引用字段不编码当前 dense ID，而引用 canonical graph projection；
- `K`：不改变语言语义，但进入 cache compatibility/static key，例如 text/binary version 和 compiler build；
- `N`：不进入语义或 cache identity，例如 origin 展示、display path 和打印 ID。

`K` 在 semantic/nonsemantic 二分中属于 nonsemantic。`S` projection 不包含自己的 digest 字段，避免循环。每个字段必须在本章显式标记，禁止按“整个 section 大概是 semantic”猜测。

canonical graph projection 先对 semantic reference graph 求 SCC，按 condensation DAG 拓扑顺序编码；每个 SCC 内按 record kind、canonical non-reference scalar key 和有序 edge-label refinement 得到稳定 local ordinal。SCC 内引用编码 `LocalBackRef(ordinal)`，已完成 SCC 引用编码 `SccRef(canonical_scc_ordinal, local_ordinal)`；若 refinement 后仍完全对称，则枚举允许排列并选择 lexicographically smallest SCC byte encoding。这样递归/互递归 graph 有限终止且不依赖 dense ID、地址或插入顺序。printer 的 dense ID 必须从同一 canonical SCC/order 分配。

## 3. Manifest FieldId

Manifest 使用按 `FieldId` 严格递增的顶层 TLV。下表 1—30 在 canonical v0 artifact 中全部 required；Closed artifact 的 `NormalizedHirSchemaVersion` 写 `0`，Staged 写 `1`。

Manifest section 的完整 payload 为 `FieldCount : U` 后接恰好 FieldCount 个 `(FieldId : U, FieldLength : U, FieldPayload : Bytes[FieldLength])`；v0 `FieldCount` 必须为 30，FieldId 必须严格递增并恰为 1—30。没有 FieldFlags、optional field 或 extension field；unknown、重复、缺失、乱序、长度未恰好消费一律拒绝。字段 payload 只含下表“编码”列本身，不再嵌套 Bytes length。

| FieldId | Canonical name | 编码 | 投影 | 约束 |
| ---: | --- | --- | --- | --- |
| 1 | `language_revision` | `U` | S | v0 language revision |
| 2 | `ir_semantics_revision` | `U` | S | 必须为 `1` |
| 3 | `text_syntax_version` | `U major + U minor` | K | 必须为 `0, 1` |
| 4 | `binary_container_version` | `U major + U minor` | K | 必须为 `0, 1` |
| 5 | `compiler_build_id` | `D32` | K | 精确构建摘要 |
| 6 | `artifact_kind` | `Enum<ArtifactKind>` | S | Staged/Closed |
| 7 | `container_flavor` | `Enum<ContainerFlavor>` | N | cache 只接受 Canonical |
| 8 | `target_key` | `D32` | S | 完整 TargetKey 摘要 |
| 9 | `target_abi_revision` | `U` | S | 精确匹配 |
| 10 | `runtime_abi_revision` | `U` | S | 精确匹配 |
| 11 | `module_identity` | `Str` | S | canonical module identity |
| 12 | `module_content_sha256` | `D32` | S | 规范源码/语义输入摘要 |
| 13 | `active_module_dag_sha256` | `D32` | S | 冻结 DAG 摘要 |
| 14 | `semantic_options_sha256` | `D32` | S | 所有语义选项 |
| 15 | `capability_policy_revision` | `U` | S | comptime policy |
| 16 | `comptime_handler_revision` | `U` | S | handler 集合 |
| 17 | `pass_pipeline_revision` | `U` | K | 精确 cache compatibility |
| 18 | `semantic_module_digest` | `D32` | N | decoder 重算并核对 |
| 19 | `dependency_manifest_digest` | `D32` | S | 空 manifest 也使用真实 digest |
| 20 | `normalized_hir_schema_version` | `U` | S | Closed 为 0，Staged 为 1 |
| 21 | `target_context_digest` | `D32` | S | layout/PDB/nan mode |
| 22 | `required_feature_set` | `Vec<U>` | S | 升序、无重复；v0 通常为空 |
| 23 | `schema_registry_digest` | `D32` | K | 本 registry 的 digest |
| 24 | `active_dependency_interface_digest` | `D32` | S | 冻结依赖接口摘要 |
| 25 | `capability_policy_digest` | `D32` | S | 本次规范化授权集合和参数 |
| 26 | `handler_revision_digest` | `D32` | S | 本次 handler set/config |
| 27 | `pass_pipeline_digest` | `D32` | K | 本次规范化 pass 序列 |
| 28 | `registration_encoding_revision` | `U` | S | v0 必须为 1 |
| 29 | `module_registration_set_digest` | `D32` | N | `ink.module-registration.set.v0`，重算核对 |
| 30 | `module_registration_interface_digest` | `D32` | N | `ink.module-registration.interface.v0`，重算核对 |

### 3.1 Manifest enum

| Enum | Tag | Name |
| --- | ---: | --- |
| `ArtifactKind` | 1 | `Staged` |
|  | 2 | `Closed` |
| `ContainerFlavor` | 1 | `Canonical` |
|  | 2 | `Diagnostic` |

## 4. SectionKind 与 RecordKind

SectionKind 与 08 容器目录一致：

| SectionKind | Name | RecordKind registry |
| ---: | --- | --- |
| 1 | `Manifest` | 顶层 FieldId TLV，无 RecordKind |
| 2 | `Strings` | `StringCount + length-prefixed UTF-8`，无 RecordKind |
| 3 | `SourceFiles` | 1 `SourceFile` |
| 4 | `Origins` | 1 `Source`；2 `Instantiation`；3 `RegionExpansion`；4 `Synthetic`；5 `Merged` |
| 5 | `Types` | 1 `Type` |
| 6 | `Attributes` | 1 `Attribute`；2 `AttributeSet` |
| 7 | `Constants` | 1 `Constant` |
| 8 | `Symbols` | 1 `Symbol` |
| 9 | `Globals` | 1 `Global` |
| 10 | `Functions` | 1 `Function` |
| 11 | `RuntimeDeclarations` | 1 `RuntimeDeclaration` |
| 12 | `Dependencies` | 1 `FileRead`；2 `EnvironmentRead`；3 `DirectoryRead`；4 `ConfigRead`；5 `ToolResourceRead` |
| 13 | `NormalizedTemplates` | 1 `Template` |
| 14 | `ElaborationPlan` | 1 `PlanNode` |
| 15 | `OptionalDebugMetadata` | 1 `DisplayPath`；2 `Annotation` |
| 16 | `ModuleRegistrations` | 1 `RegistrationSummary`；2 `ModuleRegistration` |

RecordKind tag 只在所属 section 内解释。required/semantic section 中任何其他值非法。Section 15 的未知 record 只有其 outer record length 合法且 record flags 声明 optional 时才可跳过。

Strings section 的完整 payload 为 `StringCount : U` 后接恰好 StringCount 个 `(ByteLength : U, Utf8Bytes[ByteLength])`。字符串按 UTF-8 bytes 字典序去重排序后分配 zero-based StringId；无效 UTF-8、语义 identifier 所需但不满足 NFC、重复或乱序均拒绝。StringId 的 semantic projection 展开实际 bytes，不包含 dense ID。

## 5. 公共 enum 与 bitset

### 5.1 类型、访问与布局 enum

| Enum | Tag | Canonical name |
| --- | ---: | --- |
| `BuiltinTypeTag` | 1 | `i8` |
|  | 2 | `i16` |
|  | 3 | `i32` |
|  | 4 | `i64` |
|  | 5 | `i128` |
|  | 6 | `u8` |
|  | 7 | `u16` |
|  | 8 | `u32` |
|  | 9 | `u64` |
|  | 10 | `u128` |
|  | 11 | `ptrsize` |
|  | 12 | `f16` |
|  | 13 | `f32` |
|  | 14 | `f64` |
|  | 15 | `bool` |
|  | 16 | `void` |
|  | 17 | `never` |
|  | 18 | `unit` |
| `AccessTag` | 1 | `ro` |
|  | 2 | `rw` |
|  | 3 | `init` |
| `FloatFormatTag` | 1 | `f16` |
|  | 2 | `f32` |
|  | 3 | `f64` |

### 5.2 FunctionSignature 与调用 enum

| Enum | Tag | Canonical name |
| --- | ---: | --- |
| `CallableKindTag` | 1 | `function` |
|  | 2 | `constructor` |
|  | 3 | `destructor_body` |
|  | 4 | `runtime_thunk` |
| `FunctionKindTag` | 1 | `sync` |
|  | 2 | `async` |
| `ReceiverKindTag` | 1 | `none` |
|  | 2 | `mutable_instance` |
|  | 3 | `const_instance` |
|  | 4 | `static_member` |
|  | 5 | `interface_receiver` |
|  | 6 | `initializing_instance` |
| `ParameterPassingModeTag` | 1 | `value` |
|  | 2 | `object` |
|  | 3 | `const_reference` |
|  | 4 | `mutable_reference` |
|  | 5 | `raw_pointer` |
| `ResultPassingModeTag` | 1 | `value` |
|  | 2 | `result_destination` |
|  | 3 | `void` |
| `CallingConventionTag` | 1 | `ink` |
|  | 2 | `c` |
|  | 3 | `runtime_intrinsic` |
|  | 4 | `platform_eh` |
| `EntryIdentityTag` | 1 | `stable` |
|  | 2 | `version_local` |
|  | 3 | `continuation_local` |
|  | 4 | `extern` |
| `CalleeKindTag` | 1 | `direct` |
|  | 2 | `indirect` |
|  | 3 | `virtual` |
|  | 4 | `interface` |
|  | 5 | `reflection` |
|  | 6 | `abi` |
|  | 7 | `decorator_continuation` |
|  | 8 | `async_continuation` |
| `DestinationRoleTag` | 1 | `result` |
|  | 2 | `initializing_receiver` |
| `EdgeArgumentTag` | 1 | `existing_value` |
|  | 2 | `normal_result` |
|  | 3 | `unwind_exception` |
| `SuccessorRoleTag` | 1 | `branch` |
|  | 2 | `true` |
|  | 3 | `false` |
|  | 4 | `case` |
|  | 5 | `default` |
|  | 6 | `normal` |
|  | 7 | `unwind` |
|  | 8 | `found` |
|  | 9 | `missing` |
|  | 10 | `handler` |
|  | 11 | `unmatched` |

`DestinationRoleTag` 的验证规则固定为：

- `result`：普通同步 result destination 的 constructed type 等于 logical result type；`async.call`/`async.invoke` 的 constructed type 是 `Task<T>`，callee logical result 仍是 `T`；
- `initializing_receiver`：只允许 `callable_kind = constructor`，函数 logical result 为 `void`/mode `void`，channel type 是实际 constructed type；

因此 constructor 和 ordinary/async result destination 不会被 binary round-trip 错写成彼此；role 是调用语义角色，不是 storage 类型的别名。

### 5.3 Symbol、Global、Function 与 Runtime enum

| Enum | Tag | Canonical name |
| --- | ---: | --- |
| `SymbolKindTag` | 1 | `type` |
|  | 2 | `function` |
|  | 3 | `global` |
|  | 4 | `constant` |
|  | 5 | `runtime` |
|  | 6 | `stable_entry` |
|  | 7 | `decorator` |
|  | 8 | `external` |
| `GlobalRecordKindTag` | 1 | `declaration` |
|  | 2 | `definition` |
|  | 3 | `imported` |
|  | 4 | `runtime_owned` |
| `GlobalStorageTag` | 1 | `static` |
|  | 2 | `thread_local` |
|  | 3 | `runtime_managed` |
|  | 4 | `extern` |
| `MutabilityTag` | 1 | `constant` |
|  | 2 | `mutable` |
| `LinkageTag` | 1 | `internal` |
|  | 2 | `module` |
|  | 3 | `export` |
|  | 4 | `import` |
|  | 5 | `external` |
| `FunctionRecordKindTag` | 1 | `declaration` |
|  | 2 | `definition` |
|  | 3 | `generated_adapter` |
|  | 4 | `extern_bridge` |
| `RuntimeRecordKindTag` | 1 | `opaque_type` |
|  | 2 | `effect_handler` |
|  | 3 | `dispatch_slot` |
|  | 4 | `reflection_descriptor` |
|  | 5 | `stable_entry` |
|  | 6 | `abi_bridge` |
|  | 7 | `runtime_global` |
|  | 8 | `abi_export` |

`BehaviorContractBit` 使用 bit index：0 `nothrow`；1 `construction_nothrow`；2 `body_no_fail`；3 `hot_reload_stable`；4 `extern_no_unwind`。未分配 bit 必须为零。

### 5.4 Predicate、fast flag、trap 与 capability

| Enum | Tag/bit | Canonical name |
| --- | ---: | --- |
| `IntegerPredicateTag` | 1 | `eq` |
|  | 2 | `ne` |
|  | 3 | `slt` |
|  | 4 | `sle` |
|  | 5 | `sgt` |
|  | 6 | `sge` |
|  | 7 | `ult` |
|  | 8 | `ule` |
|  | 9 | `ugt` |
|  | 10 | `uge` |
| `FloatPredicateTag` | 1 | `oeq` |
|  | 2 | `une` |
|  | 3 | `olt` |
|  | 4 | `ole` |
|  | 5 | `ogt` |
|  | 6 | `oge` |
|  | 7 | `ord` |
|  | 8 | `uno` |
| `FastMathFlagBit` | 0 | `reassociate` |
|  | 1 | `contract` |
|  | 2 | `no_signed_zero` |
|  | 3 | `flush_to_zero` |
|  | 4 | `denormals_are_zero` |
| `TrapKindTag` | 1 | `bounds` |
|  | 2 | `invalid_dynamic_state` |
|  | 3 | `pending_task_destroy` |
|  | 4 | `nothrow_violation` |
|  | 5 | `cleanup_unwind` |
|  | 6 | `pdb_violation` |
| `CapabilityTag` | 1 | `fs_read` |
|  | 2 | `fs_write` |
|  | 3 | `config_read` |
|  | 4 | `environment_read` |
|  | 5 | `build_log` |
|  | 6 | `network` |
|  | 7 | `process` |
|  | 8 | `clock` |
|  | 9 | `random` |
|  | 10 | `target_query` |
|  | 11 | `reflection` |
|  | 12 | `declaration_sink` |

## 6. Effect、trait 与 stage registry

### 6.1 EffectTag

| Tag | Name | 参数编码 |
| ---: | --- | --- |
| 1 | `Pure` | 无 |
| 2 | `ReadMemory` | `Enum<AliasDomainTag>` |
| 3 | `WriteMemory` | `Enum<AliasDomainTag>` |
| 4 | `Allocate` | `Enum<StorageKindTag>` |
| 5 | `Deallocate` | `Enum<StorageKindTag>` |
| 6 | `BeginLifetime` | 无 |
| 7 | `EndLifetime` | 无 |
| 8 | `MayUnwind` | 无 |
| 9 | `MayTrap` | 无 |
| 10 | `TargetPDB` | 无 |
| 11 | `ComptimeEffect` | `Enum<CapabilityTag>` |
| 12 | `RuntimeEffect` | `Enum<RuntimeEffectHandlerTag>` |
| 13 | `Control` | 无 |
| 14 | `DeclarationSink` | `Enum<SinkKindTag>` |
| 15 | `MayDiverge` | 无 |

| Enum | Tag | Name |
| --- | ---: | --- |
| `AliasDomainTag` | 1 | `typed` |
|  | 2 | `raw` |
|  | 3 | `exception` |
|  | 4 | `runtime_metadata` |
|  | 5 | `runtime_registry` |
|  | 6 | `reflection_snapshot` |
|  | 7 | `task_state` |
|  | 8 | `task_result` |
|  | 9 | `external` |
| `StorageKindTag` | 1 | `stack` |
|  | 2 | `heap` |
|  | 3 | `exception` |
|  | 4 | `task_frame` |
|  | 5 | `exception_box` |
|  | 6 | `runtime` |
| `RuntimeEffectHandlerTag` | 1 | `trap` |
|  | 2 | `fatal` |
|  | 3 | `dynamic_destroy` |
|  | 4 | `virtual_dispatch` |
|  | 5 | `interface_dispatch` |
|  | 6 | `exception` |
|  | 7 | `exception_match` |
|  | 8 | `interface_lookup` |
|  | 9 | `dynamic_cast` |
|  | 10 | `reflection_lookup` |
|  | 11 | `reflection_dispatch` |
|  | 12 | `version_select` |
|  | 13 | `version_pin` |
|  | 14 | `task_construct` |
|  | 15 | `async_suspend` |
|  | 16 | `task_drive` |
|  | 17 | `task_publish` |
|  | 18 | `task_destroy` |
|  | 19 | `cancel_request` |
|  | 20 | `cancel_query` |
|  | 21 | `decorator` |
|  | 22 | `ffi` |

一个 opcode 的有效 effect 是 registry static effect、已解析 callee effect summary 与 schema callback 推导 effect 的规范并集。serialized effect summary 只可作 hint，verifier 必须重算。

### 6.2 TraitTag 与 StageLegalityTag

| TraitTag | Name |
| ---: | --- |
| 1 | `Terminator` |
| 2 | `ConstantLike` |
| 3 | `Speculatable` |
| 4 | `HasRegions` |
| 5 | `CallLike` |
| 6 | `InvokeLike` |
| 7 | `EdgeProducing` |
| 8 | `DestinationChannel` |
| 9 | `NoNormalReturn` |
| 10 | `Lifetime` |
| 11 | `PlaceProducing` |
| 12 | `RuntimeSemantic` |
| 13 | `SymbolUser` |
| 14 | `MaySuspend` |
| 15 | `OwnedHandle` |
| 16 | `Variadic` |
| 17 | `NoFallthrough` |
| 18 | `CapabilityRebind` |
| 19 | `BorrowedHandle` |
| 20 | `OrderedSemanticEmit` |

| StageLegalityTag | Name | 合法位置 |
| ---: | --- | --- |
| 1 | `SC` | Staged typed Core 与 Closed Core |
| 2 | `S` | 仅 Staged typed Core，关闭前消除 |
| 3 | `C` | 仅 Closed Core |
| 4 | `Plan` | 仅 ElaborationPlan，不是 Core operation |

## 7. Type registry

### 7.1 TypeKindTag

| Tag | Canonical kind | KindPayload，按顺序 | Stage |
| ---: | --- | --- | --- |
| 1 | `builtin` | `Enum<BuiltinTypeTag>` | SC |
| 2 | `pointer` | `Enum<AccessTag>, Ref<Type>, AddressSpace : U` | SC |
| 3 | `reference` | `Enum<AccessTag>, Ref<Type>` | SC |
| 4 | `slice` | `Enum<AccessTag>, Ref<Type>` | SC |
| 5 | `array` | `Ref<Type>, Ref<Constant>` | SC |
| 6 | `tuple` | `Vec<Ref<Type>>` | SC |
| 7 | `nominal_class` | `Ref<Symbol>` | SC |
| 8 | `nominal_enum` | `Ref<Symbol>` | SC |
| 9 | `nominal_interface` | `Ref<Symbol>` | SC |
| 10 | `interface_reference` | `Enum<AccessTag>, Ref<Type>` | SC |
| 11 | `function` | `FunctionSignaturePayload` | SC |
| 12 | `function_pointer` | `Ref<Type(function)>` | SC |
| 13 | `runtime_opaque` | `Ref<Symbol>, Vec<Ref<Type>>` | SC |
| 14 | `place` | `Enum<AccessTag>, Ref<Type>` | SC |
| 15 | `exception` | 空 | SC |
| 16 | `runtime_handle` | `Enum<RuntimeHandleKindTag>` | SC |
| 17 | `runtime_object` | `Enum<RuntimeObjectKindTag>, Vec<Ref<Type>>, RuntimeStorageAbiHash : D32` | SC |
| 32 | `meta_type` | 空 | S |
| 33 | `declaration_handle` | `Enum<SymbolKindTag>` | S |
| 34 | `comptime_sequence` | `Vec<Ref<Type>>` | S |
| 35 | `dependent` | `Ref<Template>, Bytes dependent-key` | S |

`RuntimeHandleKindTag`：1 `type_snapshot`；2 `interface_snapshot`；3 `function_snapshot`；4 `member_view`；5 `exception_box`；6 `version_owner`；7 `version_pin`；8 `decorator_continuation`；9 `abi_handle`。

`RuntimeObjectKindTag` v0 只有 1 `task`，且恰有一个 logical result type 参数 T。T 只允许 void、never，或 closed runtime-representable Copyable(T)，并且不得含 NoEscape 值；因此 noncopyable `Task<File>` 在 v0 type verifier 即拒绝。canonical text 固定为 `!runtime-object<task,T>`。`TargetContext + RuntimeAbiRevision + kind + arguments` 必须重算并核对 `RuntimeStorageAbiHash`，由此提供有限 size/alignment；runtime object 总是 `AddressOnly`、`SizedObjectType`、`SealedRuntimeStorage` 且 noncopyable，可以形成 owner/destination place，但字段、对象 bytes 和物理布局私有，只能由匹配 kind 的 runtime schema 操作。generic `place.addr`/`place.deref`/projection、typed/raw load/store、copy/assign/generic destroy，以及覆盖任何 active/partial sealed range 的 byte operation 全部拒绝；只允许 storage allocation、destination/owner begin、capability rebind 和注册的 Task operation。`runtime_opaque` 与之相反：它不是 `SizedObjectType`，不得形成 place、取 `sizeof` 或承载 Task storage；`runtime_handle` 是线性 verifier capability，也不是用户可存储对象。interface reference 仍使用 TypeKindTag 10；Optional reference/interface descriptor 是普通可复制 address-only nominal/enum type，不得编码为 runtime object。

### 7.2 Type record payload

`Types.RecordKind = 1` 的 required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `TypeKind` | `Enum<TypeKindTag>` | S |
| 2 | `KindPayload` | 上表对应 payload | S |
| 3 | `AttributeSet` | `Opt<Ref<AttributeSet>>` | S |
| 4 | `LayoutDigest` | `Opt<D32>` | S |
| 5 | `Origin` | `Opt<Ref<Origin>>` | N |

所有使用到的 builtin 也具有 Type record；canonical text 在 operation/signature 中把 `builtin` entry 打印成 `i32` 等短名，其他类型打印 `!tN`。property bit 不序列化为权威字段，verifier 从结构和 TargetContext 重算 `Copyable`、`AddressOnly`、`LayoutComplete` 等性质。

`FunctionSignaturePayload` required 顺序：

```text
CallableKindTag
FunctionKindTag
ReceiverKindTag
Opt<ReceiverTypeId>
CallingConventionTag
ParameterCount
repeat ParameterCount: ParameterTypeId, ParameterPassingModeTag
LogicalResultTypeId
ResultPassingModeTag
```

它不包含 nothrow、entry identity、默认实参、可见性或 hot reload 状态。

## 8. Attribute 与 Constant registry

### 8.1 AttributeKeyTag 与 AttributeValueKindTag

AttributeKeyTag 按 canonical key 的 UTF-8 顺序分配，因此 tag 顺序也是 printer 顺序：

| Tag | Key | 允许 value kind |
| ---: | --- | --- |
| 1 | `abi_tag` | Bytes/String |
| 2 | `addend` | SInt |
| 3 | `alignment` | UInt |
| 4 | `as` | Type/Symbol |
| 5 | `base` | Symbol |
| 6 | `bits` | APInt/FloatBits |
| 7 | `body_may_fail` | Bool |
| 8 | `callee_kind` | Enum |
| 9 | `construction_may_unwind` | Bool |
| 10 | `dst_alignment` | UInt |
| 11 | `fast` | EnumSet |
| 12 | `field` | Symbol |
| 13 | `index` | UInt |
| 14 | `kind` | Enum |
| 15 | `method` | Symbol |
| 16 | `predicate` | Enum |
| 17 | `slot` | Symbol |
| 18 | `src_alignment` | UInt |
| 19 | `value` | Bool/UInt/SInt/String |
| 20 | `variant` | Symbol |

| AttributeValueKindTag | Name | payload |
| ---: | --- | --- |
| 1 | `Bool` | `Bool` |
| 2 | `UInt` | `U` |
| 3 | `SInt` | `I` |
| 4 | `APInt` | `APInt` |
| 5 | `FloatBits` | `FloatBits` |
| 6 | `String` | `Str` |
| 7 | `Bytes` | `Bytes` |
| 8 | `Type` | `Ref<Type>` |
| 9 | `Constant` | `Ref<Constant>` |
| 10 | `Symbol` | `Ref<Symbol>` |
| 11 | `Enum` | `EnumDomainTag : U, EnumValueTag : U` |
| 12 | `EnumSet` | `EnumDomainTag : U, BitMask : U` |
| 13 | `Array` | `Vec<Ref<Attribute>>` |
| 14 | `Dictionary` | `Vec<Ref<Attribute>>`，按 key tag 排序 |
| 15 | `TargetKey` | `D32` |
| 16 | `Layout` | `D32` |
| 17 | `SourceRange` | `Ref<SourceFile>, Start : U, End : U` |
| 18 | `Null` | 空 |

`EnumDomainTag`：1 `IntegerPredicate`；2 `FloatPredicate`；3 `FastMathFlag`；4 `TrapKind`；5 `CalleeKind`；6 `Capability`；7 `EntryIdentity`。

`Attribute` required payload 为 `AttributeKeyTag, AttributeValueKindTag, ValuePayload`，全部 `S`。`AttributeSet` required payload 为 `Vec<Ref<Attribute>>`，必须按 key tag 严格递增且无重复，全部 `S`。非语义 debug annotation 不得放入 Attributes section。

### 8.2 ConstantKindTag

| Tag | Canonical kind | KindPayload | Stage |
| ---: | --- | --- | --- |
| 1 | `int` | `APInt` | SC |
| 2 | `bool` | `Bool` | SC |
| 3 | `float` | `FloatBits` | SC |
| 4 | `null` | 空 | SC |
| 5 | `unit` | 空 | SC |
| 6 | `symbol_relocation` | `Ref<Symbol>, Addend : I` | SC |
| 7 | `tuple` | `Vec<Ref<Constant>>` | SC |
| 8 | `array` | `Vec<Ref<Constant>>` | SC |
| 9 | `nominal_aggregate` | `Ref<Symbol>, Vec<Ref<Constant>>` | SC |
| 10 | `target_blob` | `D32 TargetKey, D32 LayoutDigest, Bytes` | SC |
| 11 | `string` | `Bytes canonical UTF-8` | SC |
| 32 | `meta_type` | `Ref<Type>` | S |
| 33 | `declaration_handle` | `Ref<Symbol>` | S |
| 34 | `comptime_sequence` | `Vec<Ref<Constant>>` | S |

`Constant` required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `ConstantKind` | `Enum<ConstantKindTag>` | S |
| 2 | `Type` | `Ref<Type>` | S |
| 3 | `KindPayload` | 上表 | S |
| 4 | `Origin` | `Opt<Ref<Origin>>` | N |

aggregate constant 只描述逻辑组成部分，不是 aggregate SSA，也不编码 padding。`string` 的 type 必须是准确 core.String，bytes 是 canonical Unicode scalar sequence 的 UTF-8；普通 constant 保留逻辑 String 值，StaticRegistrationEncodable/registration image lowering 才把它 materialize 为 module-owned immutable bytes + length，且不建立普通可析构 String object。nominal aggregate 可以递归引用 string constant。

## 9. Source、Origin 与 module entity record

### 9.1 SourceFiles 和 Origins

`SourceFile` required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `ModuleIdentity` | `Str` | S |
| 2 | `SourceRole` | `Enum<SourceRoleTag>` | S |
| 3 | `ByteLength` | `U` | N |
| 4 | `ContentDigest` | `D32` | S |
| 5 | `LogicalPath` | `Str` | S |
| 6 | `DisplayPath` | `Opt<Str>` | N |

`SourceRoleTag`：1 `primary`；2 `included`；3 `generated_input`。`generated_input` 仍必须对应真实输入字节，不能伪造虚拟源码。

SourceFiles canonical order 严格按 `(CanonicalModuleIdentity, SourceRoleTag, LogicalPath UTF-8 bytes, ContentDigest)` 的逐字段 lexicographic order；同内容但不同 logical path 由 path bytes 确定先后。该 tuple 在 table 内必须唯一，重复 record 非法。`DecoratorApplicationOrderPath` 的 root ordinal 只基于此唯一 source-file order，再基于各文件内 lexical decorator-application traversal 分配。

Origin section 全部字段为 `N`：

| RecordKind | Required payload，按顺序 |
| ---: | --- |
| 1 Source | `SourceFileId, Start : U, End : U` |
| 2 Instantiation | `GenericSymbolId, RequestOriginId, ClosedInstanceSymbolId, ParentOriginId` |
| 3 RegionExpansion | `ControlOriginId, SelectedBodyOriginId, IterationOrdinal : U, IterationIdentityDigest : D32, ParentOriginId` |
| 4 Synthetic | `ReasonStringId, ParentOriginId` |
| 5 Merged | `PrimaryOriginId, Vec<RelatedOriginId>` |

parent 必须先编码；DAG 无环；range 为文件内半开字节区间。

### 9.2 Symbol record

`Symbol` required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `SymbolKind` | `Enum<SymbolKindTag>` | S |
| 2 | `CanonicalModuleIdentity` | `Str` | S |
| 3 | `StructuralPath` | `Str` | S |
| 4 | `SourceDeclarationKey` | `Bytes` | S |
| 5 | `SignatureDigest` | `D32` | S |
| 6 | `ClosedGenericArguments` | `Vec<Ref<Constant>>` | S |
| 7 | `SymbolKeyDigest` | `D32` | N，重算核对 |
| 8 | `DisplayName` | `Str` | N |
| 9 | `Origin` | `Ref<Origin>` | N |

### 9.3 Global record

`Global` required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `RecordKind` | `Enum<GlobalRecordKindTag>` | S |
| 2 | `Symbol` | `Ref<Symbol>` | S |
| 3 | `Type` | `Ref<Type>` | S |
| 4 | `Storage` | `Enum<GlobalStorageTag>` | S |
| 5 | `Mutability` | `Enum<MutabilityTag>` | S |
| 6 | `Linkage` | `Enum<LinkageTag>` | S |
| 7 | `Initializer` | `Opt<Ref<Constant>>` | S |
| 8 | `InitializerFunction` | `Opt<Ref<Symbol>>` | S |
| 9 | `FinalizerFunction` | `Opt<Ref<Symbol>>` | S |
| 10 | `Attributes` | `Opt<Ref<AttributeSet>>` | S |
| 11 | `LayoutDigest` | `D32` | S |
| 12 | `Origin` | `Ref<Origin>` | N |

`Initializer` 与 `InitializerFunction` 最多一个 present；import/declaration 不得携带定义 initializer。

### 9.4 Function record

`Function` required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `RecordKind` | `Enum<FunctionRecordKindTag>` | S |
| 2 | `Symbol` | `Ref<Symbol>` | S |
| 3 | `CallableKind` | `Enum<CallableKindTag>` | S |
| 4 | `FunctionKind` | `Enum<FunctionKindTag>` | S |
| 5 | `ReceiverKind` | `Enum<ReceiverKindTag>` | S |
| 6 | `ReceiverType` | `Opt<Ref<Type>>` | S |
| 7 | `EntryIdentity` | `Enum<EntryIdentityTag>` | S |
| 8 | `CallingConvention` | `Enum<CallingConventionTag>` | S |
| 9 | `TargetAbiTag` | `D32` | S |
| 10 | `Parameters` | `Vec<(Ref<Type>, Enum<ParameterPassingModeTag>, Ref<Origin>)>` | type/mode S，origin N |
| 11 | `LogicalResultType` | `Ref<Type>` | S |
| 12 | `ResultPassingMode` | `Enum<ResultPassingModeTag>` | S |
| 13 | `BehaviorContracts` | `Bits<BehaviorContractBit>` | S |
| 14 | `BehaviorContractDigest` | `D32` | S，重算核对 |
| 15 | `StableEffectEnvelope` | `Opt<StableEffectEnvelopePayload>` | S |
| 16 | `StableEffectEnvelopeHash` | `Opt<D32>` | S，重算核对 |
| 17 | `Attributes` | `Opt<Ref<AttributeSet>>` | S |
| 18 | `AbiDigest` | `D32` | S |
| 19 | `FunctionOrigin` | `Ref<Origin>` | N |
| 20 | `BodyPresence` | `Bool` | S |
| 21 | `BodyRegions` | present 时 `Vec<Region>` | S，内部 origin N |

Function type 中的签名字段必须与此处 3—6、8、10—12 一致；entry identity、contract、effect envelope 和 ABI digest 不属于普通 function type。constructor 必须是 `CallableKind=constructor`、`FunctionKind=sync`、`ReceiverKind=initializing_instance`、logical `void`、result mode `void`。async function logical result T 必须为 void、never，或 closed runtime-representable Copyable(T)，且不含 NoEscape。`EntryIdentity=stable` 必须 present envelope 与 envelope hash；其他 entry 两者必须 absent。BehaviorContractDigest 对所有 function required，按 `(CanonicalLineageIdentity, CurrentNothrow, CurrentConstructionMayUnwind, CurrentBodyMayFail)` 固定顺序计算；不适用的 bool 固定为 false。三个 current bool 从 BehaviorContracts 重算，不接受摘要覆盖。

`MemoryAccessUpperBoundTag`：1 `none`；2 `read_any`；3 `read_write_any`。EffectUpperBoundPayload required 顺序为：

```text
MemoryAccessUpperBoundTag
RuntimeEffectsAny : Bool
RuntimeEffects : Vec<RuntimeEffectHandlerTag>   // Any=false；否则 count 必须 0
AllocateKindsAny : Bool
AllocateKinds : Vec<StorageKindTag>             // Any=false；否则 count 必须 0
DeallocateKindsAny : Bool
DeallocateKinds : Vec<StorageKindTag>           // Any=false；否则 count 必须 0
MayTrap : Bool
MayDiverge : Bool
TargetPDB : Bool
```

三个 Vec 均按 tag 严格递增且无重复。StableEffectEnvelopePayload required 顺序为 `SyncEffects : EffectUpperBoundPayload, AsyncConstructionEffects : EffectUpperBoundPayload, AsyncBodyEffects : EffectUpperBoundPayload, SyncMayUnwind : Bool, ConstructionMayUnwind : Bool, BodyMayFail : Bool`。这是 lineage 首次兼容基线后固定并参与 hash 的 replacement 上界，不是当前 body summary；三个 exception bit 同样是固定 upper-bound bits。当前 BehaviorContracts 只能相对已发布 predecessor 单调加强；调用点使用 baseline envelope 与 current contract 的交集，exact body summary 必须同时满足两者。loader 必须重算 envelope hash、BehaviorContractDigest 和 AbiDigest，验证 exact body 是 envelope 子集、满足 current contract，并通过 active published-lineage metadata 验证 contract 未减弱。BehaviorContractDigest 及其值必须进入 import dependency、`active_dependency_interface_digest`、incremental/cache key 和 AOT assumption identity。

### 9.5 RuntimeDeclaration record

共同 required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `RuntimeRecordKind` | `Enum<RuntimeRecordKindTag>` | S |
| 2 | `Symbol` | `Ref<Symbol>` | S |
| 3 | `RuntimeAbiRevision` | `U` | S |
| 4 | `KindPayload` | 下表 | S |
| 5 | `Attributes` | `Opt<Ref<AttributeSet>>` | S |
| 6 | `AbiDigest` | `D32` | S |
| 7 | `Origin` | `Ref<Origin>` | N |

| RuntimeRecordKind | KindPayload，按顺序 |
| --- | --- |
| OpaqueType | `Ref<Type>, Bits<TypePropertyTag>` |
| EffectHandler | `Enum<RuntimeEffectHandlerTag>, Enum<StageLegalityTag>, Opt<Enum<CapabilityTag>>` |
| DispatchSlot | `OwnerSymbolId, SlotOrdinal : U, FunctionTypeId, EntryIdentityTag` |
| ReflectionDescriptor | `OwnerSymbolId, DescriptorKindTag : U, Opt<FunctionTypeId>, VersionPolicyTag : U` |
| StableEntry | `LogicalSymbolId, VersionLocalSymbolId, FunctionTypeId, TargetAbiTag : D32` |
| AbiBridge | `FunctionTypeId, CallingConventionTag, BridgeDirectionTag : U, TargetAbiTag : D32` |
| RuntimeGlobal | `TypeId, GlobalStorageTag, MutabilityTag` |
| AbiExport | `CSymbol : Str, InkTargetSymbolId, CFunctionTypeId, BridgeSymbolId, TargetAbiTag : D32, StableAddress : Bool, ExceptionPolicyTag` |

`TypePropertyTag`：1 `Copyable`；2 `NeedsDestroy`；3 `AddressOnly`；4 `StableAddress`；5 `NoEscape`；6 `SealedRuntimeStorage`。这里只允许 runtime schema 声明的性质，且 verifier 仍按 type kind 重算；runtime_object 固定具有 AddressOnly 与 SealedRuntimeStorage，固定不具有 Copyable。`BridgeDirectionTag`：1 `InkToExternal`；2 `ExternalToInk`。`ExceptionPolicyTag`：1 `Nothrow`；2 `CatchAndStatus`；3 `CatchAndCallback`。

## 10. Dependency、Template 与 ElaborationPlan record

### 10.1 Dependency

五种 Dependency RecordKind 共享 payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `NormalizedAccessIdentity` | `Bytes` | S |
| 2 | `ObservedDigest` | `D32` | S |
| 3 | `HandlerRevision` | `U` | S |
| 4 | `ValidationMode` | `Enum<DependencyValidationTag>` | S |
| 5 | `DisplayIdentity` | `Opt<Str>` | N |

`DependencyValidationTag`：1 `Content`；2 `ExistenceAndContent`；3 `OrderedListing`；4 `ExactValue`。network/process/clock/random 在 v0 不具有 Dependency RecordKind，实际执行即 noncacheable。

### 10.2 Template enum 与 record

| Enum | Tag | Name |
| --- | ---: | --- |
| `TemplateKindTag` | 1 | `value_expression` |
|  | 2 | `statement_block` |
|  | 3 | `top_level_items` |
|  | 4 | `class_member_items` |
|  | 5 | `interface_member_items` |
|  | 6 | `enum_member_items` |
|  | 7 | `function_body` |
|  | 8 | `generic_declaration_body` |
| `RegionKindTag` | 1 | `value` |
|  | 2 | `statement` |
|  | 3 | `top_level` |
|  | 4 | `class_member` |
|  | 5 | `interface_member` |
|  | 6 | `enum_member` |
|  | 7 | `module_registration` |
| `CaptureKindTag` | 1 | `comptime_value` |
|  | 2 | `runtime_value` |
|  | 3 | `place` |
|  | 4 | `type` |
|  | 5 | `declaration_handle` |
|  | 6 | `lexical_declaration` |
|  | 7 | `target_config` |
| `CaptureAccessTag` | 1 | `read` |
|  | 2 | `read_write` |
|  | 3 | `initialize` |

`Template` required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `TemplateKind` | `Enum<TemplateKindTag>` | S |
| 2 | `RegionKind` | `Enum<RegionKindTag>` | S |
| 3 | `SourceDeclarationKey` | `Bytes` | S |
| 4 | `SourceFile` | `Ref<SourceFile>` | S，通过 content digest |
| 5 | `SourceRange` | `Start : U, End : U` | N |
| 6 | `NormalizedHirSchemaVersion` | `U` | S |
| 7 | `NormalizedHirPayload` | `Bytes` | S |
| 8 | `LexicalEnvironmentKey` | `D32` | S |
| 9 | `Captures` | `Vec<Capture>` | S，capture origin N |
| 10 | `Origin` | `Ref<Origin>` | N |
| 11 | `SemanticDigest` | `D32` | N，重算核对 |

Capture required 顺序：`CaptureKind, CanonicalKey : Bytes, ExpectedTypeId, CaptureAccess, OriginId`。

### 10.3 Plan enum 与 record

| Enum | Tag | Name |
| --- | ---: | --- |
| `StageOpcodeTag` | 1 | `stage.force_value` |
|  | 2 | `stage.force_block` |
|  | 3 | `stage.select_if` |
|  | 4 | `stage.select_match` |
|  | 5 | `stage.expand_for` |
|  | 6 | `stage.expand_while` |
|  | 7 | `stage.instantiate` |
| `PlanInputTag` | 1 | `constant` |
|  | 2 | `plan_result` |
|  | 3 | `core_value` |
|  | 4 | `template_capture` |
|  | 5 | `instance_argument` |
| `SinkKindTag` | 1 | `value` |
|  | 2 | `statement` |
|  | 3 | `top_level_declaration` |
|  | 4 | `class_member` |
|  | 5 | `interface_member` |
|  | 6 | `enum_member` |
|  | 7 | `module_registration` |
| `InsertionPositionTag` | 1 | `before` |
|  | 2 | `after` |
|  | 3 | `replace` |
|  | 4 | `append` |

PlanInput payload：

| PlanInputTag | payload，按顺序 |
| --- | --- |
| Constant | `ConstantId, ExpectedTypeId` |
| PlanResult | `PlanNodeId, ResultIndex : U, ExpectedTypeId` |
| CoreValue | `OwnerSymbolId, RegionPath, ValueId, ExpectedTypeId` |
| TemplateCapture | `TemplateId, CaptureIndex : U, ExpectedTypeId` |
| InstanceArgument | `InstanceIdentity : D32, ArgumentIndex : U, ExpectedTypeId` |

Sink payload：

```text
SinkKindTag
OwnerSymbolId
RegionStructuralPath : RegionPath
SourceBackedAnchorKey : D32
InsertionPositionTag
```

Value sink 的 owner/path/anchor 使用消费者位置；append 以区域 source-backed end anchor 表示，不能留空。

`PlanNode` required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `StageOpcode` | `Enum<StageOpcodeTag>` | S |
| 2 | `Inputs` | `Vec<TaggedPlanInput>` | S |
| 3 | `TemplateRefs` | `Vec<Ref<Template>>` | S |
| 4 | `Sink` | `TaggedSink` | S |
| 5 | `ParentElaborationContext` | `D32` | S |
| 6 | `RequiredCapabilities` | `Vec<Enum<CapabilityTag>>` | S |
| 7 | `DependencyPlanNodes` | `Vec<Ref<PlanNode>>` | S |
| 8 | `CanonicalWorkKey` | `D32` | S |
| 9 | `Origin` | `Ref<Origin>` | N |

dependency list 必须与 PlanResult 引用推导出的边完全相等，按 PlanNode canonical key 排序且无环。

### 10.4 ModuleRegistration record

SectionKind 16 是 Staged/Closed 都 required 的 semantic section；其逻辑 collection 名为 `TypedRegistrations`，canonical section/table 名统一为 `ModuleRegistrations`/`module_registrations`，不得再建立同义第二节。section 第一个且唯一一个 RecordKind 1 `RegistrationSummary` payload 为 `RegistrationEncodingRevision : U` (S)、`RegistrationCount : U` (N)、`ModuleRegistrationSetDigest : D32` (N，重算)、`ModuleRegistrationInterfaceDigest : D32` (N，重算)。`RegistrationEncodingRevision` 与两个 digest 必须分别匹配 Manifest fields 28—30；`RegistrationCount` 必须恰等于后续 RecordKind 2 的数量，不与不存在的 Manifest count 比较。其后是按下述 structural order 排序的 RecordKind 2 `ModuleRegistration`：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `RegistrationIdentity` | `D32` | S，重算核对 |
| 2 | `RecordType` | `Ref<Type>` | S |
| 3 | `Value` | `Ref<Constant>` | S |
| 4 | `ProducerSymbol` | `Ref<Symbol>` | S |
| 5 | `DecoratorApplicationKey` | `D32` | S |
| 6 | `DecoratorApplicationOrderPath` | `Vec<U>` | S |
| 7 | `SourceBackedCallsiteKey` | `D32` | S |
| 8 | `DynamicControlPath` | `Vec<TaggedDynamicPathStep>` | S |
| 9 | `EmissionOrdinal` | `U` | S |
| 10 | `ProtocolSchemaDigest` | `D32` | S，重算核对 |
| 11 | `Origin` | `Ref<Origin>` | N |

`DecoratorApplicationOrderPath` 是 non-empty、odd-length 的 module-global hierarchical `Vec<U>`。root application path 恰为 `[CanonicalSourceTraversalApplicationOrdinal]`；该 root ordinal 从 canonical source-file order，再从文件内 lexical decorator-application traversal 依次从零分配。由 generated output 发现的 child application 在 parent path 后恰追加 `(ParentSemanticOutputOrdinal, ChildLocalApplicationOrdinal)`：前者是 parent transaction 中所有 ordered semantic-output events 共用的从零连续序号，不是 registration-only `EmissionOrdinal`；后者是该 generated output 内 lexical decorator-application traversal 的从零序号。所有 ordinal 只从 normalized source/output order 派生；hash bytes、fixed-point round、线程、worker、work queue、arena/container insertion order 都不得参与，cache replay 必须重现相同 semantic-output 与 registration-arrival 两套 ordinal。decoder 必须拒绝空 path 或偶数长度；Staged verifier 还必须核对 root/child derivation。相同 order path 的 records 必须解析到同一 `DecoratorApplicationKey` 和 `ProducerSymbol`。

`DynamicPathStepTag`：1 `expansion`，payload `ExpansionContextKey : D32`；2 `call`，payload `CallSiteKey : D32, InvocationOrdinal : U`；3 `branch`，payload `ControlNodeKey : D32, ArmOrdinal : U`；4 `loop`，payload `ControlNodeKey : D32, IterationOrdinal : U, IterationIdentityDigest : D32`。`DynamicControlPath` 按结构嵌套/执行路径顺序编码这些 tagged steps，不是 opaque hash，也不得含执行 round、线程、worker、容器插入或 hash iteration 顺序。同一 call site 的递归/重复调用以 `InvocationOrdinal` 区分；loop 中 canonical identity 相等的不同元素仍以 `IterationOrdinal` 区分。identity 固定为：

```text
H("ink.module-registration.v0",
  CanonicalModuleIdentity,
  ModuleContentDigest,
  ProducerSymbol.SymbolKey,
  DecoratorApplicationKey,
  DecoratorApplicationOrderPath,
  SourceBackedCallsiteKey,
  DynamicControlPath,
  EmissionOrdinal)
```

identity 不含 value、全局 commit 顺序或调度顺序。`EmissionOrdinal` 是该 active decorator application 中每次实际到达 `ct.register_module_item` 时按实际源码执行顺序分配的 application-wide ordinal，从 0 开始连续递增；它不在每个 callsite/path 内重新从零计数。identity 因包含 `ModuleContentDigest` 而是 module-content-local，不承诺跨 module version 保持不变；跨版本由完整 registration set 与代码原子发布/替换。只有 pending-batch commit/writer 前可以合并 replay duplicate：相同 `RegistrationIdentity` 的候选必须是完整 logical record 逐字段相同，包括 canonical Origin structure（不是只比 S projection 或 raw OriginId）；否则是 determinism error。writer 输出前必须保证 `RegistrationIdentity` 唯一、`(DecoratorApplicationOrderPath, EmissionOrdinal)` 唯一，并对每个 path 检查 ordinal 恰为 `0..N-1`。canonical section 的 decoder 遇到上述任一种重复都直接拒绝，绝不去重或规范化；它同样重做连续性检查。path 比较按 `U` 元素的无符号数值逐元素 lexicographic order，较短的真前缀先于其 child；records 严格按 `(DecoratorApplicationOrderPath, EmissionOrdinal, RegistrationIdentity)` 排序。identity 只作最终 tie-break，禁止用任何 D32 字节序替代 program order。

`StaticRegistrationEncodable(T, Constant)` 是中央派生 verifier predicate：T/constant graph 已 closed、LayoutComplete 且类型精确匹配；不得含 runtime/meta/host handle、reference、NoEscape、active lifetime、普通 cleanup/resource ownership或任意需要用户 ctor/dtor/callback 的字段。v0 允许递归 scalar、unit、enum、tuple、array、已闭合 nominal aggregate、受限 symbol relocation，以及 ConstantKind `string`；String frozen image是 readonly module-owned immutable bytes + length，无释放责任且不运行普通 String destructor。用户自定义 destructor/resource type 默认拒绝，只有未来中央 registry 分配准确、无用户代码/无 release responsibility 的 frozen encoding 才能放行，用户 attribute 不能自行声明。

relocation 只允许指向当前 version 的 immutable static data、stable function entry 或 registry 允许的 static metadata；function pointer 不得指向 version_local body。record 装载、替换或卸载不运行用户 install/remove callback。`ProtocolSchemaDigest = H("ink.module-registration.protocol.v0", RegistrationTypeSemanticIdentity, LogicalFrozenShape, TargetLayoutDigest, RegistrationEncodingRevision, RuntimeAbiRevision)`，decoder 重算。

`ModuleRegistrationSetDigest = H("ink.module-registration.set.v0", OrderedModuleRegistrationSemanticProjections)`，其中该 structured field 编码为 `Count : U`，再为每条 canonical ordered record 编码 `ProjectionLength : U + 完整 S projection bytes`；projection 包含 identity/order path/value。`ModuleRegistrationInterfaceDigest = H("ink.module-registration.interface.v0", SortedUniqueRegistrationSchemaTuples)`，其中该 structured field 编码为 `Count : U` 后接 sorted unique fixed-schema tuples，每个 tuple 恰按 `(RegistrationTypeSemanticIdentity : D32, ProtocolSchemaDigest : D32, TargetLayoutDigest : D32, RegistrationEncodingRevision : U, RuntimeAbiRevision : U)` 编码，不含 RegistrationIdentity、order path 或 value。tuple 按字段逐项排序：D32 使用 unsigned byte lexicographic order，U 使用无符号数值 order；只有全部字段相等才视为重复。两个 Manifest digest 都重算；code-only hot reload要求 interface digest 相等，而 identity/value set 可以随新 module version变化并由 runtime与代码版本原子发布/替换。record owner 是当前 artifact module version 的容器语义，不编码可伪造 runtime token。

## 11. Region、Block 与 Operation binary schema

### 11.1 Outer record envelope

除 Manifest 和 Strings 外，每个 section record 都使用同一外壳，字段顺序不可改变：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `RecordKind` | `U` | S |
| 2 | `RecordFlags` | `U` | N；required record 必须为 0 |
| 3 | `RecordLength` | `U` | N |
| 4 | `RequiredPayload` | `RecordLength` 覆盖的准确 bytes | 由本章逐字段决定 |

`RecordKind` 虽能由 payload 推断，也必须编码并验证。`RecordFlags` bit 0 是 `Optional`，只允许 Section 15；bit 1—63 保留且必须为零。required record 不允许 extension TLV。optional debug record 的 required prefix 后可有 `(ExtensionFieldId : U, ExtensionLength : U, Bytes)`，按 FieldId 递增；未知 optional extension 可以跳过，任何其他未知字段必须拒绝。

### 11.2 Region 和 block

`CoreRegionRoleTag`：1 `function_body`；2 `nested_body`；3 `decorator_body`；4 `cleanup_body`。Function record 恰有一个 `function_body` region；operation schema 决定每个 nested region 的 role 和顺序。

Region required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `RegionRole` | `Enum<CoreRegionRoleTag>` | S |
| 2 | `BlockCount` | `U` | S |
| 3 | `Blocks` | `BlockCount` 个 Block payload | S，内部 origin 为 N |
| 4 | `RegionOrigin` | `Ref<Origin>` | N |

`BlockArgumentRoleTag`：1 `phi`；2 `receiver`；3 `parameter`；4 `result_destination`；5 `task_self`；6 `task_result_storage`；7 `region_argument`；8 `global_lifecycle`。

Block required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `BlockArgumentCount` | `U` | S |
| 2 | `BlockArguments` | 重复 `(BlockArgumentRoleTag : U, RoleIndex : U, TypeId : U, OriginId : U)` | role/index/type S，origin N |
| 3 | `OperationCount` | `U` | S |
| 4 | `Operations` | 重复 Operation payload | S，内部 origin 为 N |
| 5 | `BlockOrigin` | `Ref<Origin>` | N |

BlockId 是 region 内 canonical block ordinal，ValueId 是 function/独立 CFG region 内按 canonical region/block 顺序的 block arguments、再按 block 内 operation results 分配的 canonical ordinal；entry receiver/parameters/hidden channels 已是 block arguments，没有独立 parameter ValueId。它们不另存 identity record。semantic projection 对其引用展开目标结构和规范 CFG position，不把偶然 dense number 作为身份。

function entry block argument固定顺序为：receiver（若有，RoleIndex=0）；ordered logical parameters（RoleIndex=parameter index）；GlobalRecord 指定的 InitializerFunction/FinalizerFunction 可各有一个 runtime-provided `global_lifecycle`（RoleIndex=0）；sync 且 result mode=result_destination 时一个 hidden active-owner `!place<init,T>` result_destination（RoleIndex=0）；async body固定一个 runtime-provided `task_self`（RoleIndex=0），其类型为 borrowed-authority `!place<rw,!runtime-object<task,T>>`；async body 且 logical T address-only 时再有一个 runtime-generated owner `!place<init,T>` task_result_storage（RoleIndex=0）。constructor 只有 initializing receiver，不另加 result destination。global_lifecycle 的 type 是匹配 global 的唯一 owner place：initializer 为 `!place<init,T>`，finalizer 为 `!place<ro/rw,T>`；只有 runtime 持有匹配 module-version lifecycle capability 时建立，普通函数不得获得、destroy 或 reinitialize global。task_self sealed、不可 escape/copy/address/project，只能作为 frame-internal logical capability spill；task_result_storage 初始为 AllocatedUninitialized、无 TransactionId，真正构造结果时 fresh `obj.init.begin`，可跨 await spill，成功后 commit/as_alive/publish，失败若已 begin则 cleanup+rollback并在 publish_failure 前回到 AllocatedUninitialized。它由 sealed Task frame runtime schema合法派生但不是普通 place projection。非 entry CFG block argument全部 role=phi、RoleIndex=0；nested region entry 显式参数用 region_argument/source index。role/type/order不匹配一律拒绝。

### 11.3 Edge argument 与 successor

EdgeArgument required payload：

| `EdgeArgumentTag` | 后续字段 | 语义 |
| --- | --- | --- |
| `existing_value` | `ValueId : U` | 已在前驱中定义的普通 SSA value |
| `normal_result` | `LogicalResultIndex : U` | schema 在 normal edge 产生；v0 只允许 index 0 |
| `unwind_exception` | 空 | schema 在 unwind edge 产生的 incoming exception |

Successor required payload 固定为 `SuccessorRoleTag, SuccessorOrdinal : U, BlockId : U, EdgeArgumentCount : U, EdgeArguments...`，全部 S。`SuccessorOrdinal` 是同 role 内从 0 开始的序号；非重复 role 必须为 0。`call.invoke`、`async.invoke`、`async.await*` 和 `abi.invoke` 的 role 顺序固定 normal 后 unwind；lookup 固定 found 后 missing；`cf.cond_br` 固定 true 后 false；`cf.switch` 固定 case 的规范常量顺序后 default。

`normal_result` 与 `unwind_exception` 不引用前驱 ValueId。目标 block argument 才取得普通 ValueId；verifier 必须检查 sentinel kind、结果 index、目标类型、唯一合法 successor 和一一对应关系。address-only destination 的 normal edge只产生 lifecycle proof，不携带 `normal_result` 或隐式 place sentinel。

### 11.4 Operation common record

每条 Operation required payload 按以下顺序编码：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `OpcodeTag` | `U` | S |
| 2 | `ResultCount` | `U` | S |
| 3 | `ResultTypeIds` | `ResultCount × Ref<Type>` | S |
| 4 | `OperandCount` | `U` | S |
| 5 | `OperandValueIds` | `OperandCount × U` | S |
| 6 | `OperationAttributeCount` | `U` | S；v0 必须为 0 |
| 7 | `OperationAttributeRefs` | 空 | S |
| 8 | `RegionCount` | `U` | S |
| 9 | `Regions` | `RegionCount × Region` | S，origin N |
| 10 | `SuccessorCount` | `U` | S |
| 11 | `Successors` | `SuccessorCount × Successor` | S |
| 12 | `OpcodeSpecificPayload` | 由 OpcodeTag 唯一选择 | S |
| 13 | `OriginId` | `Ref<Origin>` | N |

v0 operation 的花括号字段全部来自 typed `OpcodeSpecificPayload`，不是 AttributeTable 中可自由增删的属性；因此 `OperationAttributeCount` 必须为零。保留该字段只为让将来的 revision 能统一扩展共同 envelope，不能在 revision 1 中偷渡未知 effect、type 或 successor 控制位。

### 11.5 DestinationPayload

所有带 `to %destination` 的 schema 内嵌同一 required payload，字段顺序固定：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `DestinationValueId` | `U` | S |
| 2 | `ConstructedTypeId` | `Ref<Type>` | S |
| 3 | `DestinationRole` | `Enum<DestinationRoleTag>` | S |

可选 destination 编码为 `DestinationPresent : Bool`，present 时立即跟随上述三项。canonical text 固定打印 `to %vN {destination_role = result}` 或 `to %vN {destination_role = initializing_receiver}`；`ConstructedTypeId` 固定打印为 operation type signature 的 `-> T` channel type。role 字段是 required schema payload，不能被省略或用同名自由 attribute 覆盖。

`result` role 的普通同步 destination 要求 constructed type 等于 callee logical result 且 result mode 为 `result_destination`；异步构造要求 constructed type 是准确 `Task<T>`，而 callee logical result仍是 `T`；`initializing_receiver` 只允许 constructor，callee logical result/mode 固定 `void`/`void`。destination 不计入 SSA `ResultCount`，但作为语义 use 参与 dominance、borrow 和 lifecycle 验证。

normal completion只建立准确 storage 已 Alive 的 proof。normal-dominated block 必须显式用 `place.as_alive` 把原 `!place<init,T>` capability generation 重绑定为 `!place<ro/rw,T>`；不存在 `committed_destination` edge argument。unwind rollback终结 transaction但保留原 init generation，不需要 `place.as_uninitialized`。

## 12. Opcode namespace、shape 与 payload registry

### 12.1 OpcodeTag namespace

OpcodeTag 为 16-bit 语义编号，以 ULEB128 编码。高字节是 namespace，低字节是 namespace-local tag：

| 范围 | namespace | v0 policy |
| --- | --- | --- |
| `0x0000`—`0x00FF` | structural/reserved | 不分配 executable opcode |
| `0x0100`—`0x01FF` | `const` | Core |
| `0x0200`—`0x02FF` | `cf` | Core |
| `0x0300`—`0x03FF` | `arith` | Core |
| `0x0400`—`0x04FF` | `pdb` | Core |
| `0x0500`—`0x05FF` | `fp` | Core |
| `0x0600`—`0x06FF` | `cast` | Core/runtime cast |
| `0x0700`—`0x07FF` | `enum` | Core |
| `0x0800`—`0x08FF` | `slice` | Core |
| `0x0900`—`0x09FF` | `mem` | Core |
| `0x0A00`—`0x0AFF` | `place` | Core |
| `0x0B00`—`0x0BFF` | `raw` | Core |
| `0x0C00`—`0x0CFF` | `ptr` | Core |
| `0x0D00`—`0x0DFF` | `obj` | Core |
| `0x0E00`—`0x0EFF` | `call` | Core |
| `0x0F00`—`0x0FFF` | `eh` | Core |
| `0x1000`—`0x10FF` | `rt` | trap/fatal/versioning |
| `0x1100`—`0x11FF` | `async` | Task/cancellation/continuation |
| `0x1200`—`0x12FF` | `reflect` | reflection |
| `0x1300`—`0x13FF` | `decorator` | staged decorator |
| `0x1400`—`0x14FF` | `abi` | external ABI calls |
| `0x1500`—`0x15FF` | `ct` | Staged ComptimeWorld typed Core |
| `0x1600`—`0xEFFF` | reserved | revision 1 不得出现 |
| `0xF000`—`0xFFFF` | experimental/private | canonical artifact 不得出现 |

namespace 内未列出的 low byte 全部保留；同一 `IrSemanticsRevision` 内不得给删除项或别名复用。StageOpcodeTag 属于 ElaborationPlan 的独立 enum，不占 OpcodeTag namespace。decoder 对任何未知 tag 必须报错，不能按 opaque pure operation 跳过。

### 12.2 ShapeSchemaTag

shape 记法为 `R/O/G/S/D`：operation SSA result、operand、region、successor、destination；`*` 是由 payload 中 count 决定的有界序列。

| Tag | Name | Exact structural shape |
| ---: | --- | --- |
| 1 | `NullaryResult` | `R1 O0 G0 S0 D0` |
| 2 | `UnaryResult` | `R1 O1 G0 S0 D0` |
| 3 | `BinaryResult` | `R1 O2 G0 S0 D0` |
| 4 | `TernaryResult` | `R1 O3 G0 S0 D0` |
| 5 | `NullaryEffect` | `R0 O0 G0 S0 D0` |
| 6 | `UnaryEffect` | `R0 O1 G0 S0 D0` |
| 7 | `BinaryEffect` | `R0 O2 G0 S0 D0` |
| 8 | `TernaryEffect` | `R0 O3 G0 S0 D0` |
| 9 | `VariadicEffect` | `R0 O* G0 S0 D0` |
| 10 | `OneSuccessor` | `R0 O* G0 S1 D0` |
| 11 | `TwoSuccessors` | `R0 O* G0 S2 D0` |
| 12 | `ManySuccessors` | `R0 O* G0 S* D0` |
| 13 | `CallNoUnwind` | `R0\|1 O* G0 S0 D0\|1` |
| 14 | `CallInvoke` | `R0 O* G0 S2 D0\|1` |
| 15 | `DestinationEffect` | `R0 O* G0 S0 D1` |
| 16 | `DestinationInvoke` | `R0 O* G0 S2 D1` |
| 17 | `LookupBranch` | `R0 O* G0 S2 D0`，found edge 产生 result(0) |
| 18 | `AwaitBranch` | `R0 O1 G0 S2 D0`，normal 可产生 result(0) |
| 19 | `AwaitDestination` | `R0 O1 G0 S2 D1` |
| 20 | `SingleRegion` | `R0 O* G1 S0 D0` |
| 21 | `CompletionTerminator` | `R0 O* G0 S0 D0`，NoFallthrough |
| 22 | `VariadicResult` | `R1 O* G0 S0 D0` |

### 12.3 PayloadSchemaTag

下表字段全部为 S projection。括号中的 text form 是 operation line 中在 type signature 前的唯一拼写；不打印 binary-only count、ordinal 和 ABI digest 的重复可读形式。

| Tag | Name | Binary required payload，按顺序 |
| ---: | --- | --- |
| 1 | `Empty` | 空 |
| 2 | `Constant` | `ConstantId` |
| 3 | `Symbol` | `SymbolId` |
| 4 | `SymbolAddend` | `SymbolId, Addend : I` |
| 5 | `IntegerPredicate` | `Enum<IntegerPredicateTag>`，text `{predicate = name}` |
| 6 | `FloatPredicate` | `Enum<FloatPredicateTag>`，text `{predicate = name}` |
| 7 | `FastMath` | `Bits<FastMathFlagBit>`，text 为空集时省略，否则 `{fast = [name, ...]}`，按 bit 递增 |
| 8 | `Switch` | `CaseCount : U, repeat (ConstantId, CaseSuccessorOrdinal : U), DefaultSuccessorOrdinal : U` |
| 9 | `SelectorSymbol` | `SymbolId`，field key 由 opcode 固定为 `base`/`field`/`variant`/`slot`/`method` |
| 10 | `StaticIndex` | `Index : U`，text `{index = N}` |
| 11 | `Alignment` | `Alignment : U`，text `{alignment = N}` |
| 12 | `RawTransferAlignment` | `DestinationAlignment : U, SourceAlignment : U` |
| 13 | `TrapKind` | `Enum<TrapKindTag>`，text `{kind = name}` |
| 14 | `Call` | `CallPayload` |
| 15 | `EhMatch` | `HandlerCount : U, repeat (ExceptionTypeId, HandlerSuccessorOrdinal), UnmatchedSuccessorOrdinal` |
| 16 | `EhPayload` | `AsTypeId`，text `{as = T}` |
| 17 | `ThrowTarget` | `ExceptionTypeSymbolId, ConstructorSymbolId, ArgumentCount : U` |
| 18 | `InterfaceMake` | `InterfaceSymbolId, DestinationPayload` |
| 19 | `InterfaceUp` | `AncestorInterfaceSymbolId, DestinationPayload` |
| 20 | `TryClass` | `TargetClassSymbolId, Enum<TryClassResultFormTag>, Opt<DestinationPayload>` |
| 21 | `TryInterface` | `TargetInterfaceSymbolId, DestinationPayload` |
| 22 | `ReflectLookup` | `ModuleSymbolId` |
| 23 | `ReflectLookupFunction` | `ModuleSymbolId, ExpectedFunctionTypeId` |
| 24 | `ReflectLookupMember` | `Enum<ReflectionMemberKindTag>` |
| 25 | `ReflectCall` | `ExpectedFunctionTypeId, ReceiverPresent : Bool, ArgumentCount : U, Opt<DestinationPayload>` |
| 26 | `AsyncCall` | `CallPayload`，destination required 且 role=result |
| 27 | `Await` | `AwaitedLogicalTypeId` |
| 28 | `PublishResult` | `Enum<ResultPassingModeTag>, LogicalResultTypeId` |
| 29 | `DecoratorRegion` | `Enum<DecoratorKindTag>, Layer : U, FunctionTypeId` |
| 30 | `Continuation` | `NextLayer : U, FunctionTypeId, Opt<DestinationPayload>` |
| 31 | `TransferPin` | `Enum<PinOwnerKindTag>` |
| 32 | `AbiCall` | `AbiCallPayload` |
| 33 | `ResultMode` | `Enum<ResultPassingModeTag>, LogicalResultTypeId` |
| 34 | `ModuleRegistration` | `Enum<RegistrationValueFormTag>, SourceBackedCallsiteKey : D32` |

`TryClassResultFormTag`：1 `raw_pointer`；2 `optional_reference`。`ReflectionMemberKindTag`：1 `field`；2 `function`；3 `constructor`；4 `base`；5 `interface`。`DecoratorKindTag`：1 `sync`；2 `async`。`PinOwnerKindTag`：1 `task`；2 `exception`；3 `snapshot`；4 `frame`。`RegistrationValueFormTag`：1 `value`；2 `object`。所有 enum 的 text spelling 为表中小写 name。

`CallPayload` required 顺序：

```text
CalleeKindTag
CalleeVariantPayload
FunctionTypeId
EntryIdentityTag
CallingConventionTag
TargetAbiTag : D32
ExplicitArgumentCount : U
Opt<DestinationPayload>
```

CalleeVariantPayload：direct 为 `SymbolId`；indirect 为空且 operand 0 是 function value；virtual 为 `SlotSymbolId` 且 operand 0 是 receiver；interface 为 `MethodSymbolId` 且 operand 0 是 interface place；reflection 为 `ExpectedFunctionTypeId, ReceiverPresent : Bool` 且 operand 0 是 function snapshot；abi/decorator/async continuation 不允许出现在普通 CallPayload。其余 operands 是按 FunctionSignature 顺序的 receiver（若未由 variant 占用）和显式参数；`ExplicitArgumentCount` 必须准确消费 operand 尾部。

`AbiCallPayload` required 顺序：

```text
AbiCalleeKindTag
DirectSymbolId                 // direct only
CFunctionTypeId
BridgeSymbolId
ExpectedInkFunctionTypeId
TargetAbiTag : D32
ExplicitArgumentCount : U
Opt<DestinationPayload>
```

`AbiCalleeKindTag`：1 `direct`；2 `indirect`。indirect 的 operand 0 是 C function pointer，direct 没有该 operand。bridge 和 ABI type 必须来自 RuntimeDeclaration/Function record，不能用自由 bytes 替代。

### 12.4 Effect 与 trait 表记法

后续 opcode 表使用完整 registry name 的紧凑组合：`R(x)` = `ReadMemory(x)`，`W(x)` = `WriteMemory(x)`，`A(x)` = `Allocate(x)`，`D(x)` = `Deallocate(x)`，`RT(x)` = `RuntimeEffect(x)`。`CallSummary`、`AsyncConstructionSummary`、`AsyncBodySummary` 和 `ExternalEffectSummary` 是从已经验证的 callee/slot/envelope record 规范推导的 effect set，不是可序列化自由 flag；表中列出的 static effect 与其取并集。`Pure` 不能与 memory/control/runtime effect 同时作为“无效果”承诺；出现 `Pure 或 R(typed)` 的 schema 必须按 operand representation 唯一推导其中一个。

## 13. Core OpcodeTag registry

下表的 `Sx/Py` 分别引用 ShapeSchemaTag/PayloadSchemaTag。`Traits` 为空写 `—`；未列出的 trait 不成立。type/CFG rule 是 schema 的 required 部分，不是说明性文字。

### 13.1 `const`、`cf`、`arith` 与 `pdb`

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 257 (`0x0101`) | `const.int` | S1/P2 | Constant kind int，result type/bit width 精确匹配 | Pure | SC | ConstantLike, Speculatable |
| 258 (`0x0102`) | `const.bool` | S1/P2 | Constant kind bool，result bool | Pure | SC | ConstantLike, Speculatable |
| 259 (`0x0103`) | `const.float` | S1/P2 | Constant kind float，format/bits 精确匹配 | Pure | SC | ConstantLike, Speculatable |
| 260 (`0x0104`) | `const.null` | S1/P2 | Constant kind null，result raw pointer | Pure | SC | ConstantLike, Speculatable |
| 261 (`0x0105`) | `const.unit` | S1/P2 | Constant kind unit，result unit | Pure | SC | ConstantLike, Speculatable |
| 262 (`0x0106`) | `const.symbol_addr` | S1/P4 | symbol relocation，result compatible raw pointer | Pure, TargetPDB | SC | ConstantLike, Speculatable, SymbolUser |
| 263 (`0x0107`) | `const.function` | S1/P3 | symbol function signature 精确匹配，取得 stable entry | Pure, TargetPDB | SC | ConstantLike, Speculatable, SymbolUser |
| 513 (`0x0201`) | `cf.br` | S10/P1 | successor role branch；全部 edge args existing_value | Control | SC | Terminator, EdgeProducing, NoFallthrough |
| 514 (`0x0202`) | `cf.cond_br` | S11/P1 | operand 0 bool；successor true/false；其余 operands 只经 edge args 引用 | Control | SC | Terminator, EdgeProducing, NoFallthrough |
| 515 (`0x0203`) | `cf.switch` | S12/P8 | key 为 integer/enum discriminant；case unique 且按 constant canonical order；default 最后 | Control | SC | Terminator, EdgeProducing, NoFallthrough, Variadic |
| 516 (`0x0204`) | `cf.select` | S4/P1 | `(bool,T,T)->T`，T 为同一 scalar 或 unit | Pure | SC | Speculatable |
| 517 (`0x0205`) | `cf.return` | S9/P1 | void/destination result O0；value/unit result O1 且精确匹配；never 禁止 | Control | SC | Terminator, NoFallthrough |
| 518 (`0x0206`) | `cf.unreachable` | S5/P1 | verifier 证明无合法运行时到达路径 | Control | SC | Terminator, NoNormalReturn, NoFallthrough |
| 769 (`0x0301`) | `arith.add` | S3/P1 | `(I,I)->I` 同一整数，mod 2^N | Pure | SC | Speculatable |
| 770 (`0x0302`) | `arith.sub` | S3/P1 | 同上 | Pure | SC | Speculatable |
| 771 (`0x0303`) | `arith.mul` | S3/P1 | 同上 | Pure | SC | Speculatable |
| 772 (`0x0304`) | `arith.neg` | S2/P1 | `(I)->I` | Pure | SC | Speculatable |
| 773 (`0x0305`) | `arith.and` | S3/P1 | `(I,I)->I` | Pure | SC | Speculatable |
| 774 (`0x0306`) | `arith.or` | S3/P1 | `(I,I)->I` | Pure | SC | Speculatable |
| 775 (`0x0307`) | `arith.xor` | S3/P1 | `(I,I)->I` | Pure | SC | Speculatable |
| 776 (`0x0308`) | `arith.not` | S2/P1 | `(I)->I` | Pure | SC | Speculatable |
| 777 (`0x0309`) | `arith.cmp` | S3/P5 | `(I,I)->bool`，predicate/signedness 匹配 | Pure | SC | Speculatable |
| 1025 (`0x0401`) | `pdb.sdiv` | S3/P1 | `(iN,iN)->iN` | TargetPDB, MayTrap | SC | — |
| 1026 (`0x0402`) | `pdb.udiv` | S3/P1 | `(uN,uN)->uN` | TargetPDB, MayTrap | SC | — |
| 1027 (`0x0403`) | `pdb.srem` | S3/P1 | `(iN,iN)->iN` | TargetPDB, MayTrap | SC | — |
| 1028 (`0x0404`) | `pdb.urem` | S3/P1 | `(uN,uN)->uN` | TargetPDB, MayTrap | SC | — |
| 1029 (`0x0405`) | `pdb.shl` | S3/P1 | `(I,ptrsize)->I` | TargetPDB, MayTrap | SC | — |
| 1030 (`0x0406`) | `pdb.lshr` | S3/P1 | `(uN/ptrsize,ptrsize)->same` | TargetPDB, MayTrap | SC | — |
| 1031 (`0x0407`) | `pdb.ashr` | S3/P1 | `(iN,ptrsize)->iN` | TargetPDB, MayTrap | SC | — |
| 1032 (`0x0408`) | `pdb.fptosi` | S2/P1 | `(F)->iN` | TargetPDB, MayTrap | SC | — |
| 1033 (`0x0409`) | `pdb.fptoui` | S2/P1 | `(F)->uN/ptrsize` | TargetPDB, MayTrap | SC | — |

### 13.2 `fp` 与 `cast`

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 1281 (`0x0501`) | `fp.add` | S3/P7 | `(F,F)->F`，fast flags 受 TargetContext 验证 | Pure, TargetPDB | SC | Speculatable |
| 1282 (`0x0502`) | `fp.sub` | S3/P7 | 同上 | Pure, TargetPDB | SC | Speculatable |
| 1283 (`0x0503`) | `fp.mul` | S3/P7 | 同上 | Pure, TargetPDB | SC | Speculatable |
| 1284 (`0x0504`) | `fp.div` | S3/P7 | 同上；浮点除零不是 Ink trap | Pure, TargetPDB | SC | Speculatable |
| 1285 (`0x0505`) | `fp.neg` | S2/P1 | `(F)->F`，只翻转 sign bit | Pure | SC | Speculatable |
| 1286 (`0x0506`) | `fp.fma` | S4/P7 | `(F,F,F)->F` | Pure, TargetPDB | SC | Speculatable |
| 1287 (`0x0507`) | `fp.cmp` | S3/P6 | `(F,F)->bool` | Pure | SC | Speculatable |
| 1288 (`0x0508`) | `fp.assume_finite` | S2/P1 | `(F)->F`；输入必须已证明 finite | Pure | SC | — |
| 1537 (`0x0601`) | `cast.int` | S2/P1 | integer 到 integer；result type 给出宽度/signedness | Pure | SC | Speculatable |
| 1538 (`0x0602`) | `cast.int_to_float` | S2/P1 | integer 到 float，RNE | Pure | SC | Speculatable |
| 1539 (`0x0603`) | `cast.float` | S2/P1 | float 到 float | Pure, TargetPDB | SC | Speculatable |
| 1540 (`0x0604`) | `cast.bit` | S2/P1 | 等 bit width scalar bitcast | Pure | SC | Speculatable |
| 1541 (`0x0605`) | `cast.ptr` | S2/P1 | raw pointer/ptrsize 间或 compatible raw pointer | Pure | SC | Speculatable |
| 1542 (`0x0606`) | `cast.ptr_access` | S2/P1 | `ptr<rw,T,A> -> ptr<ro,T,A>` | Pure | SC | Speculatable |
| 1543 (`0x0607`) | `cast.ref_access` | S2/P1 | `ref<rw,T> -> ref<ro,T>` | Pure | SC | Speculatable |

### 13.3 `enum` 与 generation-aware `slice`

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 1793 (`0x0701`) | `enum.value` | S1/P9 | selector 是无 payload variant；result 是 scalar enum | Pure, TargetPDB | SC | Speculatable, SymbolUser |
| 1794 (`0x0702`) | `enum.init_variant` | S6/P9 | operand `!place<init,E>` active；variant 属于 E | W(typed), BeginLifetime, TargetPDB | SC | Lifetime, SymbolUser |
| 1795 (`0x0703`) | `enum.discriminant` | S2/P1 | scalar enum 或 readable enum place -> ptrsize | Pure 或 R(typed), TargetPDB | SC | — |
| 1796 (`0x0704`) | `enum.is_variant` | S2/P9 | scalar enum 或 readable enum place -> bool | Pure 或 R(typed), TargetPDB | SC | SymbolUser |
| 2049 (`0x0801`) | `slice.init` | S8/P1 | `(%dst:!place<init,slice<A,T>>, %first:!place<ro/rw,T>, %length:ptrsize)->()`；length > 0；first 捕获当前 ObjectLifetimeGeneration | R(typed), W(typed), BeginLifetime | SC | Lifetime |
| 2050 (`0x0802`) | `slice.data` | S2/P1 | readable slice place -> compatible raw pointer；显式丢失 generation authority | R(typed) | SC | — |
| 2051 (`0x0803`) | `slice.length` | S2/P1 | readable slice place -> ptrsize | R(typed) | SC | — |
| 2052 (`0x0804`) | `slice.index` | S3/P1 | readable slice place + ptrsize -> element place；结果携带 borrow authority + selected generation | R(typed), MayTrap | SC | PlaceProducing |
| 2053 (`0x0805`) | `slice.subslice` | S9/P1 | dst init slice、source readable slice、begin/end ptrsize；结果保留所选子范围 generation set | R(typed), W(typed), BeginLifetime, MayTrap | SC | Lifetime |
| 2054 (`0x0806`) | `slice.init_empty` | S6/P1 | `!place<init,slice<A,T>> -> ()`；建立 canonical `(null,0)` 与空 BorrowGenerationSet | W(typed), BeginLifetime | SC | Lifetime |

`BorrowGenerationSet`、ObjectLifetimeGeneration 和 element borrow authority 是 verifier dataflow state，不进入 type/operation binary payload，也不打印为自由 attribute；decoder 后 verifier 必须从 `slice.init`/`init_empty`/`subslice`/`index`/`data` 重新推导。`slice.init` 不接受 raw pointer，空 slice 只能用 `slice.init_empty`。`slice.init`、`slice.init_empty` 和 `slice.subslice` 只填充当前 active owner transaction，不自动 commit；final completer 必须随后显式执行一次 `obj.init.commit`。

### 13.4 `mem`、`place`、`raw` 与 `ptr`

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 2305 (`0x0901`) | `mem.alloca` | S1/P1 | result `!place<init,T>`，T SizedObjectType | A(stack) | SC | PlaceProducing |
| 2306 (`0x0902`) | `mem.global_place` | S1/P3 | symbol global；普通 use 只产生 matching generation 的 borrow-authority place，绝不产生 owner | Pure, TargetPDB | SC | PlaceProducing, SymbolUser |
| 2307 (`0x0903`) | `mem.load` | S2/P1 | readable Alive place of scalar/unit -> T | R(typed) | SC | — |
| 2308 (`0x0904`) | `mem.store` | S7/P1 | `(T,!place<rw,T>)->()`，Alive scalar/unit | W(typed) | SC | — |
| 2309 (`0x0905`) | `mem.load_unaligned` | S2/P11 | readable Alive place -> scalar | R(typed) | SC | — |
| 2310 (`0x0906`) | `mem.store_unaligned` | S7/P11 | value + writable Alive place | W(typed) | SC | — |
| 2561 (`0x0A01`) | `place.deref` | S2/P1 | compatible raw pointer -> borrow-authority place；T 非 SealedRuntimeStorage；raw 前置成立时捕获当前 ObjectLifetimeGeneration | Pure | SC | PlaceProducing |
| 2562 (`0x0A02`) | `place.from_ref` | S2/P1 | reference -> same access/type place | Pure | SC | PlaceProducing |
| 2563 (`0x0A03`) | `place.as_alive` | S2/P1 | `!place<init,T>` -> `!place<ro/rw,T>`；当前 path 已有 commit/destination normal Alive proof | Pure | SC | PlaceProducing, CapabilityRebind |
| 2564 (`0x0A04`) | `place.as_uninitialized` | S2/P1 | owner-authority `!place<rw,T>` -> `!place<init,T>`；destroy 后准确 state 为 AllocatedUninitialized，无 live borrow/child capability；readonly 非法 | Pure | SC | PlaceProducing, CapabilityRebind |
| 2565 (`0x0A05`) | `place.field` | S2/P9 | field 属于准确 class；保留 path/access 上限 | Pure, TargetPDB | SC | PlaceProducing, SymbolUser |
| 2566 (`0x0A06`) | `place.base` | S2/P9 | concrete declared base path | Pure, TargetPDB | SC | PlaceProducing, SymbolUser |
| 2567 (`0x0A07`) | `place.tuple_element` | S2/P10 | static tuple index 合法 | Pure, TargetPDB | SC | PlaceProducing |
| 2568 (`0x0A08`) | `place.array_element` | S3/P1 | array place + ptrsize -> element place | Pure, TargetPDB, MayTrap | SC | PlaceProducing |
| 2569 (`0x0A09`) | `place.enum_payload` | S2/P9 | active/partially initialized variant proof | Pure, TargetPDB | SC | PlaceProducing, SymbolUser |
| 2570 (`0x0A0A`) | `place.addr` | S2/P1 | nonsealed place -> compatible raw pointer；丢失 lifecycle authority | Pure | SC | — |
| 2571 (`0x0A0B`) | `place.borrow` | S2/P1 | Alive place -> non-owning reference | Pure | SC | — |
| 2817 (`0x0B01`) | `raw.load` | S2/P11 | compatible raw pointer -> scalar T；访问范围与 active/partial SealedRuntimeStorage 不相交 | R(raw) | SC | — |
| 2818 (`0x0B02`) | `raw.store` | S7/P11 | scalar T + compatible raw pointer；访问范围与 active/partial SealedRuntimeStorage 不相交 | W(raw) | SC | — |
| 2819 (`0x0B03`) | `raw.memcpy` | S8/P12 | dst, src, byte count；未授权重叠非法；两范围均与 active/partial sealed range 不相交 | R(raw), W(raw) | SC | — |
| 2820 (`0x0B04`) | `raw.memmove` | S8/P12 | dst, src, byte count；允许彼此重叠；两范围均与 active/partial sealed range 不相交 | R(raw), W(raw) | SC | — |
| 2821 (`0x0B05`) | `raw.memset` | S8/P11 | dst, u8 byte, byte count；范围与 active/partial sealed range 不相交 | W(raw) | SC | — |
| 3073 (`0x0C01`) | `ptr.offset` | S3/P1 | raw pointer + ptrsize elements -> same pointer type；T sized | Pure, TargetPDB | SC | Speculatable |
| 3074 (`0x0C02`) | `ptr.byte_offset` | S3/P1 | raw pointer + ptrsize bytes -> same pointer type | Pure | SC | Speculatable |
| 3075 (`0x0C03`) | `ptr.cmp` | S3/P5 | compatible address spaces；predicate 仅 eq/ne/ult/ule/ugt/uge | Pure | SC | Speculatable |

`CapabilityRebind` 的共同语义是消费当前 verifier-only `PlaceCapabilityGeneration` 并产生 next generation；地址和 projection path 不变，旧 generation 的任何后续 use 非法。两条 rebind 虽是 `Pure`，也没有 `Speculatable` trait，optimizer 不得复制、CSE、hoist 或 sink。rollback 只终结 active transaction并保留原 init generation；`place.as_uninitialized` 只服务 destroy 后 owner `rw` 到 `init` 的重绑定。

### 13.5 `obj` 与 `call`

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 3329 (`0x0D01`) | `obj.init.begin` | S6/P1 | owner-authority init place；建立 fresh active transaction | BeginLifetime | SC | Lifetime |
| 3330 (`0x0D02`) | `obj.init` | S7/P1 | owner transaction 内 nonsealed init place + scalar/unit value，顺序固定 dst,value | W(typed), BeginLifetime | SC | Lifetime |
| 3331 (`0x0D03`) | `obj.init.copy` | S7/P1 | owner transaction 内 nonsealed init dst + readable Alive address-only Copyable src；generation 匹配 | R(typed), W(typed), BeginLifetime | SC | Lifetime |
| 3332 (`0x0D04`) | `obj.init.commit` | S6/P1 | owner-authority nonsealed transaction root 完整且无 active child；唯一 commit并建立 fresh ObjectLifetimeGeneration | W(typed), BeginLifetime | SC | Lifetime |
| 3333 (`0x0D05`) | `obj.assign.copy` | S7/P1 | nonsealed writable Alive dst + readable Alive Copyable src；authority/generation 均匹配 | R(typed), W(typed) | SC | — |
| 3334 (`0x0D06`) | `obj.destroy` | S6/P1 | owner-authority `!place<ro/rw,T>` Alive；T nonsealed、静态完整、析构 nothrow；终结 generation | W(typed), EndLifetime, CallSummary | SC | Lifetime |
| 3335 (`0x0D07`) | `obj.destroy_dynamic` | S7/P1 | owner-authority nonsealed complete object + matching versioned vtable；终结 generation | W(typed), EndLifetime, RT(dynamic_destroy) | SC | Lifetime, RuntimeSemantic |
| 3585 (`0x0E01`) | `call.direct` | S13/P14 | CalleeKind=direct；ink；nothrow；result form/role 精确匹配 | CallSummary；stable 再加 RT(version_select) | SC | CallLike, DestinationChannel, SymbolUser |
| 3586 (`0x0E02`) | `call.indirect` | S13/P14 | CalleeKind=indirect；function value/signature 精确；独立 nothrow proof | CallSummary | SC | CallLike, DestinationChannel |
| 3587 (`0x0E03`) | `call.virtual` | S13/P14 | CalleeKind=virtual；receiver/slot/signature/version table 匹配；nothrow | CallSummary, RT(virtual_dispatch) | SC | CallLike, DestinationChannel, SymbolUser |
| 3588 (`0x0E04`) | `call.interface` | S13/P14 | CalleeKind=interface；interface place/method/signature 匹配；nothrow | CallSummary, RT(interface_dispatch) | SC | CallLike, DestinationChannel, SymbolUser |
| 3589 (`0x0E05`) | `call.invoke` | S14/P14 | callee kind direct/indirect/virtual/interface/reflection；恰有 normal/unwind；scalar normal 用 result(0)，address normal 不带 result | CallSummary, MayUnwind, Control；kind-specific RT effect | SC | Terminator, CallLike, InvokeLike, EdgeProducing, DestinationChannel, NoFallthrough |

`obj.init` 的 operand 顺序以本表为 binary source of truth，即 destination 在前、value 在后。任何旧示意若写成相反顺序必须在进入 canonical IR 前规范化。

Call schema 的 SSA/result channel 固定：void 无 result/destination；unit/scalar nothrow call 恰有一个 SSA result；address-only result 使用 destination role=result；constructor 使用 destination role=initializing_receiver；invoke 的 unit/scalar只在 normal edge产生 `result(0)`，address/constructor normal edge不产生 value。所有 destination normal postcondition只建立 Alive proof，successor 用 `place.as_alive` 重绑定 capability。

### 13.6 `eh`、trap 与 fatal

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 3841 (`0x0F01`) | `eh.entry` | S2/P1 | 只在 unwind successor 首部；incoming -> active exception | RT(exception), Control | SC | OwnedHandle |
| 3842 (`0x0F02`) | `eh.match` | S12/P15 | active token；handler source order，catch-all 最后，unmatched required | RT(exception_match), Control | SC | Terminator, EdgeProducing, NoFallthrough |
| 3843 (`0x0F03`) | `eh.payload` | S2/P16 | active token -> readonly nonescaping payload view | R(exception) | SC | — |
| 3844 (`0x0F04`) | `eh.end_catch` | S10/P1 | consume active token；唯一 normal successor | RT(exception), EndLifetime, Control | SC | Terminator, EdgeProducing, NoFallthrough |
| 3845 (`0x0F05`) | `eh.throw` | S21/P17 | constructor args；新 exception payload/record | A(exception), BeginLifetime, RT(exception), MayUnwind, Control | SC | Terminator, NoNormalReturn, NoFallthrough, SymbolUser |
| 3846 (`0x0F06`) | `eh.throw_copy` | S21/P1 | readable Copyable exception source | A(exception), R(typed), BeginLifetime, RT(exception), MayUnwind, Control | SC | Terminator, NoNormalReturn, NoFallthrough |
| 3847 (`0x0F07`) | `eh.throw_from` | S21/P17 | constructor args + active cause；成功后 consume cause | A(exception), BeginLifetime, RT(exception), MayUnwind, Control | SC | Terminator, NoNormalReturn, NoFallthrough, SymbolUser |
| 3848 (`0x0F08`) | `eh.rethrow` | S21/P1 | consume active token，跳过当前 handler list | RT(exception), MayUnwind, Control | SC | Terminator, NoNormalReturn, NoFallthrough |
| 3849 (`0x0F09`) | `eh.resume` | S21/P1 | consume active token，交外层 continuation | RT(exception), MayUnwind, Control | SC | Terminator, NoNormalReturn, NoFallthrough |
| 4097 (`0x1001`) | `rt.trap` | S21/P13 | 不可 catch，不运行语言 cleanup | RT(trap), MayTrap, Control | SC | Terminator, RuntimeSemantic, NoNormalReturn, NoFallthrough |
| 4098 (`0x1002`) | `rt.fatal` | S21/P13 | O0 或 O1 active exception；fail-fast | RT(fatal), Control | SC | Terminator, RuntimeSemantic, NoNormalReturn, NoFallthrough |

## 14. Runtime、decorator 与 ABI OpcodeTag registry

本节与调用/runtime 章节的 v0 canonical list 一一对应。`abi.export` 是 RuntimeDeclaration RecordKind 8，不是 CFG opcode，因而不占 OpcodeTag。

### 14.1 Dynamic cast

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 1601 (`0x0641`) | `cast.class_up` | S2/P9 | class ref -> declared concrete base ref；保持 access/lifetime | Pure, TargetPDB | SC | Speculatable, SymbolUser |
| 1602 (`0x0642`) | `cast.interface_make` | S15/P18 | object ref + destination；constructed type 是普通 address-only interface_reference；唯一 runtime commit | R(runtime_metadata), W(typed), BeginLifetime, TargetPDB, RT(interface_lookup) | SC | DestinationChannel, RuntimeSemantic, SymbolUser |
| 1603 (`0x0643`) | `cast.interface_up` | S15/P19 | readable interface-reference place + destination；ancestor registered；唯一 runtime commit | R(typed), R(runtime_metadata), W(typed), BeginLifetime, TargetPDB | SC | DestinationChannel, RuntimeSemantic, SymbolUser |
| 1604 (`0x0644`) | `cast.try_class` | scalar S2/P20 或 object S15/P20 | source 有效 class/interface view；raw-pointer form 失败为 null；optional-reference form destination 构造 Some/None | R(runtime_metadata), RT(dynamic_cast), TargetPDB；object 再加 W(typed), BeginLifetime | SC | DestinationChannel, RuntimeSemantic, SymbolUser |
| 1605 (`0x0645`) | `cast.try_interface` | S15/P21 | source 有效 class/interface view；ordinary Optional<InterfaceRef> destination 构造 Some/None | R(runtime_metadata), W(typed), BeginLifetime, RT(dynamic_cast), TargetPDB | SC | DestinationChannel, RuntimeSemantic, SymbolUser |

interface reference 与 Optional descriptor 都不是 `runtime_object`；它们按各自普通 type schema 验证 Copyable/address-only 性质。所有 destination variant normal completion 由该 opcode schema 原子执行唯一 commit，caller 随后用 `place.as_alive` 重绑定。

### 14.2 Reflection

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 4609 (`0x1201`) | `reflect.lookup_type` | S17/P22 | String place；found/missing；found 产生 owned type_snapshot | R(runtime_registry), RT(reflection_lookup), RT(version_select), Control | SC | Terminator, EdgeProducing, OwnedHandle, RuntimeSemantic, NoFallthrough |
| 4610 (`0x1202`) | `reflect.lookup_interface` | S17/P22 | String place；found 产生 owned interface_snapshot | R(runtime_registry), RT(reflection_lookup), RT(version_select), Control | SC | Terminator, EdgeProducing, OwnedHandle, RuntimeSemantic, NoFallthrough |
| 4611 (`0x1203`) | `reflect.lookup_function` | S17/P23 | String place + exact expected signature；found 产生 owned function_snapshot | R(runtime_registry), RT(reflection_lookup), RT(version_select), Control | SC | Terminator, EdgeProducing, OwnedHandle, RuntimeSemantic, NoFallthrough |
| 4612 (`0x1204`) | `reflect.lookup_member` | S17/P24 | owner snapshot + String place；found 产生 owner-bounded borrowed member_view | R(reflection_snapshot), RT(reflection_lookup), Control | SC | Terminator, EdgeProducing, RuntimeSemantic, NoFallthrough |
| 4613 (`0x1205`) | `reflect.snapshot.clone` | S2/P1 | owned K_snapshot -> 第二个相同 kind linear owner | R(runtime_registry), W(runtime_registry), RT(version_pin) | SC | OwnedHandle, RuntimeSemantic |
| 4614 (`0x1206`) | `reflect.snapshot.release` | S6/P1 | consume snapshot owner；全部 borrowed view 已结束 | R(runtime_registry), W(runtime_registry), RT(version_pin) | SC | OwnedHandle, RuntimeSemantic |
| 4615 (`0x1207`) | `reflect.call` | S13/P25 | sync pinned snapshot；仅已证明 nothrow；void/scalar/destination form与 expected signature 精确 | CallSummary, RT(reflection_dispatch)；destination 再加 W(typed), BeginLifetime | SC | CallLike, DestinationChannel, RuntimeSemantic |

lookup 的 found transition 原子执行 version select + pin；v0 不注册 `reflect.snapshot.acquire`。may-unwind reflection 调用唯一使用 `call.invoke` 且 `CalleeKind=reflection`，不注册 `reflect.invoke`。

### 14.3 Async、Task 与 cancellation

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 4353 (`0x1101`) | `async.call` | S15/P26 | async callee；T=void/never或closed runtime-representable Copyable且无NoEscape；construction_may_unwind=false；required Task<T> destination role=result；唯一 commit | AsyncConstructionSummary, A(task_frame), W(typed), BeginLifetime, RT(task_construct)；stable 再加 RT(version_select) | SC | CallLike, DestinationChannel, RuntimeSemantic |
| 4354 (`0x1102`) | `async.invoke` | S16/P26 | async callee；T约束同 async.call；Task<T> destination；normal/unwind只表示同步 construction completion/failure | AsyncConstructionSummary, A(task_frame), W(typed), BeginLifetime, RT(task_construct), MayUnwind, Control；stable 再加 RT(version_select) | SC | Terminator, CallLike, InvokeLike, EdgeProducing, DestinationChannel, RuntimeSemantic, NoFallthrough |
| 4355 (`0x1103`) | `async.await` | S18/P27 | rw Task<T> place；normal 对 unit/scalar产生 result(0)，void/never无 result；never normal successor只能以 verifier-proven unreachable 结束；unwind exception | R(task_state), W(task_state), A(exception), RT(async_suspend), MayUnwind, MayDiverge, Control | SC | Terminator, EdgeProducing, MaySuspend, RuntimeSemantic, NoFallthrough |
| 4356 (`0x1104`) | `async.await_copy` | S19/P27 | rw Task<T> + required T destination；T address-only Copyable；normal不产生 edge value | R(task_state), R(task_result), W(task_state), W(typed), BeginLifetime, A(exception), RT(async_suspend), MayUnwind, MayDiverge, Control | SC | Terminator, EdgeProducing, DestinationChannel, MaySuspend, RuntimeSemantic, NoFallthrough |
| 4357 (`0x1105`) | `async.task.drive_once` | S6/P27 | compiler/runtime generated；created -> pending，执行到 suspend/final | AsyncBodySummary, R(task_state), W(task_state), RT(task_drive), MayTrap, MayDiverge | SC | RuntimeSemantic, MaySuspend |
| 4358 (`0x1106`) | `async.task.publish_success` | S21/P28 | void O1；value/object O2；result mode/type 精确；pending -> succeeded；object operand 是 Task 内部已 Alive ResultStorage | R(task_state), W(task_state), RT(task_publish), Control；nonunit value 再加 W(task_result) | SC | Terminator, RuntimeSemantic, NoFallthrough |
| 4359 (`0x1107`) | `async.task.publish_failure` | S21/P1 | Task place + active exception；consume token，pending -> failed | R(task_state), W(task_state), A(exception_box), RT(task_publish), Control | SC | Terminator, RuntimeSemantic, NoFallthrough |
| 4360 (`0x1108`) | `async.task.destroy` | S6/P27 | owner-authority Task<T> place；非 pending 清理并使 storage uninitialized；pending 走 schema-defined fatal且不完成 | R(task_state), W(task_state), W(typed), EndLifetime, D(task_frame), RT(task_destroy), MayDiverge | SC | Lifetime, RuntimeSemantic |
| 4361 (`0x1109`) | `async.cancel.request` | S6/P27 | rw Task<T>；nothrow、幂等、线程安全 request publish | W(task_state), RT(cancel_request) | SC | RuntimeSemantic |
| 4362 (`0x110A`) | `async.cancel.is_requested` | S2/P27 | ro/rw Task<T> -> bool；不 drive Task | R(task_state), RT(cancel_query) | SC | RuntimeSemantic |
| 4363 (`0x110B`) | `async.continuation_invoke` | S14/P30 | staged async decorator 下一层；同一 Task/frame；normal/unwind；不可逃逸或并发 | AsyncBodySummary, RT(async_suspend), MayDiverge, Control；effective next-layer BehaviorContract 允许时才加 MayUnwind | S | Terminator, InvokeLike, EdgeProducing, MaySuspend, NoFallthrough |

`Task<T>` 的 constructed/operand type必须是 `!runtime-object<task,T>`。Task 的 sealed storage 不能进入 generic copy、destroy、place/raw memory schema；只有本表 Task opcode 和通用 owner `obj.init.begin`/capability rebind 能操作其 place。v0 不注册 `async.await_cancel_on_request`、`async.all` 或 `async.all_cancel_on_error`；它们必须在 canonical IR 前展开为 generated state machine、基本 await/cancel 和已注册 runtime callable。

### 14.4 Decorator continuation

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 4865 (`0x1301`) | `decorator.region` | S20/P29 | single-entry nested decorator_body；entry/exit signature 精确；continuation capability非 SSA | nested region effect upper bound, Control | S | HasRegions |
| 4866 (`0x1302`) | `decorator.continuation_invoke` | S14/P30 | next sync layer fresh activation；normal/unwind；optional destination 原样转发 | CallSummary, Control；effective next-layer BehaviorContract 允许时才加 MayUnwind | S | Terminator, InvokeLike, EdgeProducing, DestinationChannel, NoFallthrough |
| 4867 (`0x1303`) | `decorator.continuation_yield` | S21/P33 | value/result_destination/void 与包围 signature 匹配；address form只传播 committed proof | Control | S | Terminator, NoFallthrough |

decorator opcode 全部必须在 ClosedModule 前消除。`decorator.continuation_invoke` 的 DestinationPayload 可以是 result 或 initializing_receiver，但必须与原调用 channel 完全相同；它不新建、commit 或 rollback 转发 transaction。

### 14.5 Hot reload pin

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 4161 (`0x1041`) | `rt.version.pin` | S2/P1 | version_owner -> linear version_pin；不选择 current version | R(runtime_registry), W(runtime_registry), RT(version_pin) | SC | OwnedHandle, RuntimeSemantic |
| 4162 (`0x1042`) | `rt.version.unpin` | S6/P1 | consume version_pin；覆盖 uses/transfer 已结束 | R(runtime_registry), W(runtime_registry), RT(version_pin) | SC | OwnedHandle, RuntimeSemantic |
| 4163 (`0x1043`) | `rt.version.transfer_pin` | S7/P31 | consume pin并把唯一 release responsibility 交给匹配 Alive owner | R(runtime_registry), W(runtime_registry), RT(version_pin) | SC | OwnedHandle, RuntimeSemantic |
| 4164 (`0x1044`) | `rt.version.current_owner` | S1/P1 | 仅 version_local 或 inherited-pin compiler helper entry；物化当前 activation 的 borrowed version_owner，不选择/新建 pin | Pure | SC | BorrowedHandle, RuntimeSemantic |

stable-entry selection 不是单独 opcode；stable target call 的有效 effect 固有 `RT(version_select)`，并使用 Function record 的 StableEffectEnvelope。只有 verifier 已证明持有匹配 pin 的 version_local target 才能使用精确 body summary并省略 version_select。`rt.version.current_owner` 是 version_owner 的唯一 Core producer，只物化已有 activation owner；结果不可存储、return、capture 或 escape，且只能供匹配 `rt.version.pin` 或立即验证用途使用。`rt.version.pin` 只接受这一 borrowed owner generation并产生独立 owned pin。

### 14.6 External ABI

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 5121 (`0x1401`) | `abi.call` | S13/P32 | direct/indirect C target + registered bridge；仅已证明不产生 Ink unwind；result form 精确 | ExternalEffectSummary, RT(ffi) | SC | CallLike, DestinationChannel, RuntimeSemantic |
| 5122 (`0x1402`) | `abi.invoke` | S14/P32 | registered failure bridge；恰有 normal/unwind；raw foreign exception不得直接成为 token | ExternalEffectSummary, RT(ffi), MayUnwind, Control | SC | Terminator, CallLike, InvokeLike, EdgeProducing, DestinationChannel, RuntimeSemantic, NoFallthrough |

`call.*` 只接受 calling convention `ink`，`abi.*` 只接受 `c` 和已注册 bridge。缺少受信外部 effect 声明时 ExternalEffectSummary 是 read/write-any、任意 allocate/deallocate/runtime effect、MayTrap、MayDiverge 的保守上界。`abi.export` required record payload 已在 9.5 节定义；它的 canonical text 位于 runtime declaration table，不能出现在 block 中。

### 14.7 Comptime module registration

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 5377 (`0x1501`) | `ct.register_module_item` | S6/P34 | value form operand 为 scalar/unit T；object form为 `!place<ro,T>`；两者必须生成满足 StaticRegistrationEncodable(T, Constant) 的 frozen constant | DeclarationSink(module_registration)；object form 再加 R(typed) | S | OrderedSemanticEmit |

该 opcode 只能在 ComptimeWorld 的 active decorator-application context 执行；动态 CFG 自然决定是否以及执行几次。`OrderedSemanticEmit` 表示它不可 CSE、复制、推测、删除或越过其他 semantic emit 重排，但 deterministic record emission 本身不使 comptime/module cache noncacheable。`SourceBackedCallsiteKey` 是 payload，ProducerSymbol、DecoratorApplicationKey、DecoratorApplicationOrderPath、DynamicControlPath 和 application-wide EmissionOrdinal 来自已验证执行 context。每次执行向当前 pending declaration/registration batch 发射 ModuleRegistration record；batch 验证、ordinal continuity 和 identity collision 检查成功后才按 canonical key提交。它不是 Plan node，不新增 StageOpcodeTag，不可 residualize；ClosedModule 中 opcode 必须消失，但已提交 ModuleRegistrations section 保留。

canonical text 固定为 `ct.register_module_item %v {value_form = value|object, source_backed_callsite_key = sha256"..."} : (T) -> () loc(#oN)`；object form 的 T 在 operand type signature 中是 `!place<ro,T>`。payload field 顺序仍以 P34 binary 顺序为准，text dictionary 使用上述两个 key。

### 14.8 明确未注册的名称

以下拼写在 revision 1 中没有 OpcodeTag，decoder/printer/verifier 必须拒绝：`reflect.snapshot.acquire`、`reflect.invoke`、`async.await_cancel_on_request`、`async.all`、`async.all_cancel_on_error`、通用 `rt.call`、`runtime.invoke`、`obj.init.abort`、`committed_destination` edge sentinel，以及把 runtime operation 伪装成自由 attribute 的任何别名。

## 15. Canonical text 完整生成语法

本节定义 printer 输出，不定义宽松 parser。双引号内是 ASCII literal，`LF` 是单个 U+000A。文件为 UTF-8、无 BOM、无 CR、无尾随空白，最后恰有一个 LF。缩进恰为每层两个 ASCII space；不输出空行或注释。

### 15.1 Lexical value 与 type-use

```ebnf
digit = "0" | nonzero-digit ;
nonzero-digit = "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
upper-hex-digit = digit | "A" | "B" | "C" | "D" | "E" | "F" ;
decimal = "0" | nonzero-digit, { digit } ;
signed-decimal = [ "-" ], decimal ;
digest = "sha256\"", 64 * upper-hex-digit, "\"" ;
bytes = "hex\"", { upper-hex-digit, upper-hex-digit }, "\"" ;
string = "\"", { printable-ascii-except-quote-backslash | string-escape }, "\"" ;
string-escape = "\\\"" | "\\\\" | "\\n" | "\\r" | "\\t" | "\\0" | "\\u{", upper-hex-digit, { upper-hex-digit }, "}" ;
bare-segment = ( ASCII-letter | "_" ), { ASCII-letter | digit | "_" } ;
quoted-segment = string ;
path-segment = bare-segment | quoted-segment ;
module-path = path-segment, { ".", path-segment } ;
symbol-key-digest = 64 * upper-hex-digit ;
symbol-component = path-segment, [ "#", symbol-key-digest ] ;
symbol-reference = "@", module-path, "::", symbol-component, { "::", symbol-component } ;
optional = "none" | "some(", value, ")" ;
vector = "[", [ value, { ", ", value } ], "]" ;
unsigned-vector = "[", [ decimal, { ", ", decimal } ], "]" ;
value-ref = "%v", decimal ;
block-ref = "^bb", decimal ;
type-ref = "!t", decimal ;
attribute-ref = "#a", decimal ;
constant-ref = "#c", decimal ;
source-ref = "#s", decimal ;
origin-ref = "#o", decimal ;
dependency-ref = "#dep", decimal ;
template-ref = "#tmpl", decimal ;
plan-ref = "#p", decimal ;
registration-ref = "#reg", decimal ;
dynamic-control-path = "[", [ dynamic-path-step, { ", ", dynamic-path-step } ], "]" ;
dynamic-path-step = "expansion(", digest, ")"
                  | "call(", digest, ", ", decimal, ")"
                  | "branch(", digest, ", ", decimal, ")"
                  | "loop(", digest, ", ", decimal, ", ", digest, ")" ;

type-use = builtin-short | type-ref | inline-capability ;
inline-capability = "!place<", access, ",", type-use, ">"
                  | "!exception"
                  | "!runtime-handle<", runtime-handle-kind, ">"
                  | "!runtime-object<", runtime-object-kind, { ",", type-use }, ">" ;
```

`builtin-short`、`access`、runtime handle/object kind 只取第 5、7 节 registry 的 canonical lowercase name。binary 中所有这些位置仍编码 `Ref<Type>`；inline-capability 只是 text spelling，并且必须结构精确匹配 TypeTable 中唯一 record，不能隐式创建或省略 record。inline-capability 只允许 `place`、`exception`、`runtime_handle`、`runtime_object` 四个 TypeKind；pointer/reference/slice/array/tuple/nominal/function/function-pointer/interface-reference/runtime-opaque/dependent 等一律打印 `!tN`。角括号内逗号后没有空格，例如 `!place<init,!t9>` 与 `!runtime-object<task,i32>`。03/06 中 `ptr<T>` 等写法只作 schema metavariable，不是 canonical type-use。

string 的 decoded sequence 必须是 Unicode scalar sequence；可打印 ASCII 直接输出，quote/backslash 和 `\n`、`\r`、`\t`、`\0` 使用上面唯一短 escape，其余 control/non-ASCII scalar 用 uppercase、无前导零 `\u{HEX}`，禁止 surrogate 或大于 0x10FFFF。symbol-reference 必须至少有一个 `::symbol-component`；只有 module/nominal path/declaration kind 已唯一决定 identity 时才省略 digest，overload/closed generic/signature-dependent identity 必须打印完整 64 位 uppercase SHA-256，quoted segment 复用 string escape且 identifier 解码后必须 NFC。bool 为 `true|false`；enum 为 registry lowercase name；bitset 的 name 按 numeric tag 递增；vector 按字段 schema 指定的 semantic sequence 或 canonical set order打印，不得擅自重排。optional 永不通过省略字段表达。APInt 和 FloatBits 使用准确位宽的 uppercase hex，禁止十进制浮点。`DynamicControlPath` 字段必须使用 `dynamic-control-path` 的专用 spelling并保持 path step 顺序；`DecoratorApplicationOrderPath` 使用 `unsigned-vector` 并保持层次路径元素顺序；两者都不得退化为泛化 tagged-variant/set renderer。

### 15.2 File、header 与 table wrapper

```ebnf
file = "ink-ir 0.1 ", artifact-kind, " {", LF,
       header,
       source-files, origins, types, attributes, constants, symbols,
       globals, functions, runtime-declarations, module-registrations, dependencies,
       [ normalized-templates, elaboration-plan ],
       "}", LF ;

table = "  ", table-name, " {", LF, { record }, "  }", LF ;
module-registrations = "  module_registrations {", LF,
                       registration-summary-record,
                       { module-registration-record },
                       "  }", LF ;
registration-summary-record = "    registration_summary {registration_encoding_revision = ", decimal,
                              ", registration_count = ", decimal,
                              ", module_registration_set_digest = ", digest,
                              ", module_registration_interface_digest = ", digest, "}", LF ;
module-registration-record = "    ", registration-ref,
                             " = module_registration {registration_identity = ", digest,
                             ", record_type = ", type-ref,
                             ", value = ", constant-ref,
                             ", producer_symbol = ", symbol-reference,
                             ", decorator_application_key = ", digest,
                             ", decorator_application_order_path = ", unsigned-vector,
                             ", source_backed_callsite_key = ", digest,
                             ", dynamic_control_path = ", dynamic-control-path,
                             ", emission_ordinal = ", decimal,
                             ", protocol_schema_digest = ", digest,
                             ", origin = ", origin-ref, "}", LF ;
```

最后两个 table 只在 staged artifact 中出现，并且 staged 时即使为空也必须打印。其余十一个 table 总是打印。table-name 固定依次为 `source_files`、`origins`、`types`、`attributes`、`constants`、`symbols`、`globals`、`functions`、`runtime_declarations`、`module_registrations`、`dependencies`、`normalized_templates`、`elaboration_plan`。空表唯一拼写为：

```text
  globals {
  }
```

Header 唯一形式是 `  header {`、30 个四空格缩进 field line、`  }`；field 按 FieldId 顺序，每行 `name = value` 且没有逗号：

```text
  header {
    language_revision = <decimal>
    ir_semantics_revision = 1
    text_syntax_version = 0.1
    binary_container_version = 0.1
    compiler_build_id = <digest>
    artifact_kind = <staged|closed>
    container_flavor = canonical
    target_key = <digest>
    target_abi_revision = <decimal>
    runtime_abi_revision = <decimal>
    module_identity = <string>
    module_content_sha256 = <digest>
    active_module_dag_sha256 = <digest>
    semantic_options_sha256 = <digest>
    capability_policy_revision = <decimal>
    comptime_handler_revision = <decimal>
    pass_pipeline_revision = <decimal>
    semantic_module_digest = <digest>
    dependency_manifest_digest = <digest>
    normalized_hir_schema_version = <decimal>
    target_context_digest = <digest>
    required_feature_set = [<ascending decimals>]
    schema_registry_digest = <digest>
    active_dependency_interface_digest = <digest>
    capability_policy_digest = <digest>
    handler_revision_digest = <digest>
    pass_pipeline_digest = <digest>
    registration_encoding_revision = 1
    module_registration_set_digest = <digest>
    module_registration_interface_digest = <digest>
  }
```

### 15.3 Non-function record envelope

除 Function 外，record 固定占一行：四空格、record prefix、一个空格、field dictionary、LF。dictionary 唯一形式为 `{field = value, field = value}`；空 dictionary 为 `{}`。字段名是本章 required payload field 的 exact `lower_snake_case`，顺序与 binary required payload 完全相同，所有 optional/count/digest/origin 字段都打印；binary count 不单独打印，vector 的 `[]` 已唯一决定 count。variant `KindPayload` 打印 `payload = variant_name(positional_value, ...)`，positional value 顺序与对应 payload 表一致。

| Table | Record prefix |
| --- | --- |
| source_files | `#sN = source_file` |
| origins | `#oN = source`、`instantiation`、`region_expansion`、`synthetic` 或 `merged` |
| types | `!tN = type` |
| attributes | `#aN = attribute` 或 `#aN = attribute_set` |
| constants | `#cN = constant` |
| symbols | `@symbol = symbol` |
| globals | `global @symbol` |
| runtime_declarations | `runtime @symbol` |
| module_registrations | `registration_summary`（恰一次且首先）或 `#regN = module_registration` |
| dependencies | `#depN = file_read`、`environment_read`、`directory_read`、`config_read` 或 `tool_resource_read` |
| normalized_templates | `#tmplN = template` |
| elaboration_plan | `#pN = plan_node` |

`module_registrations` 不使用允许任意 `{ record }` 的通用 table body：首条必须恰为一个 `registration-summary-record`，其后只能是零个或多个 `module-registration-record`，不得出现第二个 summary、在 summary 前出现 item 或使用任何其他 prefix。summary 和 item 的 field dictionary 仍按第 10.4 节 binary required payload 顺序完整打印。

Symbol record prefix 的 `@symbol` 必须等于其 SymbolKey canonical reference；Global/Runtime prefix 必须等于 payload 的 Symbol field，dictionary 中仍完整打印 `symbol = @symbol`，避免 prefix 成为隐式字段。Type record 的 `kind_payload` 使用第 7.1 节 kind payload 顺序；Constant/Origin/Runtime/Dependency variant 同理。record field value 引用使用对应 sigil，string/bytes/digest/enum/vector/optional 使用 15.1 节 spelling。这个机械规则和第 3—10 节字段表共同构成每种 record 的完整 grammar，不允许 printer 选择别名、重排或省略默认值。

### 15.4 FunctionSignature、entry envelope 与 block

Function record 不使用通用一行 envelope，唯一模板为：

```text
    func @symbol {record_kind = K, callable_kind = K, function_kind = K, receiver_kind = K, receiver_type = OPT, entry_identity = K, calling_convention = K, target_abi_tag = DIGEST} (ENTRY_BINDINGS) -> RESULT_SPEC {behavior_contracts = [...], behavior_contract_digest = DIGEST, stable_effect_envelope = OPT, stable_effect_envelope_hash = OPT, attributes = OPT, abi_digest = DIGEST, origin = #oN} body none
```

definition 把末尾 `body none` 换成：

```text
body {
      ^bb0 origin(#oN):
        ...
    }
```

entry binding 以 `, ` 分隔并按 11.2 节顺序；形式固定为：

```ebnf
entry-binding = value-ref, ": receiver(0) ", type-use, " origin(", origin-ref, ")"
              | value-ref, ": parameter(", decimal, ") ", parameter-mode, " ", type-use, " origin(", origin-ref, ")"
              | value-ref, ": result_destination(0) ", type-use, " origin(", origin-ref, ")"
              | value-ref, ": task_self(0) ", type-use, " origin(", origin-ref, ")"
              | value-ref, ": task_result_storage(0) ", type-use, " origin(", origin-ref, ")"
              | value-ref, ": global_lifecycle(0) ", type-use, " origin(", origin-ref, ")" ;
parameter-mode = "value" | "object" | "const_reference" | "mutable_reference" | "raw_pointer" ;
result-spec = "value ", type-use
            | "result_destination ", type-use
            | "void void"
            | "void never" ;
```

entry bindings 同时是 binary entry BlockArguments 的 canonical text，不在 `^bb0` 后重复。非 entry block 固定打印 `^bbN(`、逗号分隔的 `%vM: phi(0) T origin(#oK)`、`) origin(#oK):`；无参数仍打印 `^bbN() origin(#oK):`。nested region entry 用 `region_argument(index)` 替代 phi。entry block 固定省略空括号，写 `^bb0 origin(...)`。StableEffectEnvelope 以一行 nested dictionary 打印，字段顺序严格为 9.4 节 payload 顺序；set-bound 的 `Any=true` 打印 `any`，否则打印 tag-sorted list。

CFG merge 不能靠 role side fact meet 丢失 owner obligation。owner incoming 降为 borrow 只有在每一条对应 edge 上 lifecycle/cleanup obligation 已显式 discharge，或已转移给另一支配 owner/cleanup state时合法；否则 join 非法。

### 15.5 Operation line、successor 与 region

无 region operation 固定一行：

```ebnf
operation = indent, [ result-binding, " = " ], opcode, operation-body,
            " : (", [ type-use, { ", ", type-use } ], ") -> ", channel-types,
            " loc(", origin-ref, ")", LF ;
result-binding = value-ref | "(", value-ref, { ", ", value-ref }, ")" ;
channel-types = "()" | type-use | "(", type-use, { ", ", type-use }, ")" ;
edge = block-ref, [ "(", edge-argument, { ", ", edge-argument }, ")" ] ;
edge-argument = value-ref | "result(", decimal, ")" | "exception" ;
destination = " to ", value-ref, " {destination_role = ", ("result" | "initializing_receiver"), "}" ;
```

type signature 左侧严格列出 `OperandValueIds` 的类型，不含 DestinationValueId；右侧依次是 SSA results，若无 SSA result但有 destination则是 ConstructedTypeId，若为 edge-produced value则是 logical produced type，否则 `()`。因此 `async.await_copy` 为 O1+D1，文本 `: (!place<rw,!runtime-object<task,T>>) -> T`，destination 不在 operand list或 ResultCount 重复编码。

普通非 CFG/non-call opcode 的 `operation-body` 为：零 operands 时空，否则一个空格加 `%vN, ...`；随后若 PayloadSchema 非 Empty，再加一个空格和 `{field = value, ...}`，字段按 PayloadSchema binary 顺序。P3/P4 的 SymbolId 仍在 dictionary 中打印，不伪装为 SSA operand。DestinationEffect 再在 dictionary 后打印 destination clause，其中 dictionary 不重复 DestinationPayload。P2 固定打印 `{constant = #cN}`；P9 的 key 由 opcode 固定为 `base|field|variant|slot|method`；P5/P6/P7/P10—P13 按 12.3 节给出的 key spelling。空 payload 不打印 `{}`。

CFG 特例唯一模板：

```text
cf.br ^bbN(args)
cf.cond_br %cond, ^bbT(args), ^bbF(args)
cf.switch %key [case #cN -> ^bbN(args), ...] default ^bbD(args)
eh.match %active [T -> ^bbN(%active), ...] unmatched ^bbU(%active)
eh.end_catch %active -> ^bbN(args)
```

case 按 Constant canonical order；每个完整模板之后仍追加共同 type signature 与 `loc`。

CallPayload callee renderer 固定为：direct `@symbol`；indirect `%callee`；virtual `%receiver {slot = @symbol}`；interface `%receiver {method = @symbol}`；reflection `%snapshot [receiver(%vN)]`。nothrow call 写 `opcode callee(args)`；`call.invoke` 写 `call.invoke callee_kind = kind callee(args)`；async 写 `async.call|async.invoke callee_kind = kind callee(args)`；需要 destination 时紧跟 destination clause。invoke 再按顺序写 ` normal EDGE unwind EDGE`。`reflect.call` 写 `reflect.call %snapshot receiver(none|%vN) args(%vN, ...)`；ABI 写 `abi.call|abi.invoke c_callee(args) {c_function_type = !tN, bridge = @symbol, expected_ink_function_type = !tN, target_abi_tag = DIGEST}`。所有这些随后追加共同 type signature/loc。

lookup 固定写 `opcode operands {typed payload fields} found EDGE missing EDGE`；await 固定写 `async.await %task normal EDGE unwind EDGE`；await_copy 在 `%task` 后打印 destination再打印 normal/unwind；continuation invoke 同 invoke；completion terminator没有 successor。Successor keyword、role、顺序和 edge sentinel 必须与 11.3/OpcodeSchema 完全一致。

有 region operation 把本应结束该行的 LF 改为 ` region(role, origin(#oN)) {` + LF，内部 block/operation增加两空格，结束行是原 operation indent 加 `}` + LF。region role 必须是 schema 要求的 CoreRegionRoleTag。`decorator.region` 是 v0 唯一 HasRegions opcode；空 region 非法。任何 operation—including nested operation—仍在其 header 的 `{` 前打印共同 type signature 和 `loc`。

### 15.6 Canonical generation order

printer 必须先建立 2.2 节 canonical SCC projection，再依次分配 table ID、function/block/value ID，最后只按本节模板串接 bytes。table/record/field、block successor、attribute/set、effect set 和 capability set 的排序规则均来自本 registry；禁止 pretty-printer 根据行宽换行。两个实现若输入 semantic/nonsemantic record 都相同，必须产生逐 byte 相同文本；golden generator 必须覆盖每个 enum、空/非空 table、每个 PayloadSchema、destination 两种 role、三种 edge argument、全部 BlockArgumentRole 和 nested region。

## 16. Registry conformance 与生成产物

机器可读 registry 至少生成：所有 C++ enum、name/tag 双向表、binary field decoder/encoder、canonical text emitter、opcode arity/type/effect/stage/trait table、unknown-tag rejection、semantic projector 和覆盖 golden。构建时必须断言：同 enum/tag 无重复；所有 OpcodeTag 在 namespace 内唯一；每个 opcode 引用已定义 Shape/Payload/Effect/Stage/Trait；03 Core、05 typed-Core `ct.*` 与 06 runtime canonical opcode name 集合和本表完全相等；StageOpcodeTag 恰为 1—7；所有 required record 字段具有 S/K/N；每个 text field 有唯一 renderer。

Section 16 conformance 还必须生成：summary-first/unique 检查、order path non-empty/odd-length 检查、identity 与 path+ordinal uniqueness、per-path ordinal continuity、四类 dynamic path decoder、identity/protocol/set/interface 四个 digest 重算器，以及 set/interface domain/preimage golden；canonical decoder 不得复用 pending-batch replay deduplicator。

加载顺序固定为 bounds-check container/section/record envelope，验证 canonical integer/string/order，按 registry 解码 required payload，解析全部引用，重算 SCC projection/digest/effect/trait/type/lifecycle facts，最后运行 Staged 或 Closed verifier。任一 unknown required tag、payload 未恰好消费、noncanonical order、hash mismatch 或 text/binary schema disagreement均使 artifact 整体无效，不允许部分执行或降级成 opaque operation。
