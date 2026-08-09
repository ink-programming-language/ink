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

`SchemaRegistryDigest` 精确定义为 `H("ink.schema-registry.v0", IrSemanticsRevision, EnumRegistryProjection, RecordRegistryProjection, ShapeRegistryProjection, PayloadRegistryProjection, OpcodeRegistryProjection, TextRendererProjection, VerifierCallbackProjection)`。每个 projection 都按 registry kind tag、再按条目 numeric tag 递增编码；条目编码 canonical name、required field 的 tag/name/type/order/S-K-N、stage、effect、trait、text renderer identity，以及复杂语义 verifier callback 的稳定 name/revision/input schema。未带数字但会改变 codec、projector、effect 或 verifier 结果的规则必须先获得稳定 callback/renderer identity，不能只存在于实现代码或散文中。该摘要进入 Manifest 和 cache compatibility key，但不替代 `IrSemanticsRevision`。

### 1.1 Renderer 与 verifier callback identity

上述两个 projection 不是实现函数地址或任意字符串列表。`RegistrySubjectKindTag` 固定为：1 `enum_domain`；2 `record`；3 `shape`；4 `payload`；5 `opcode`；6 `stage_opcode`；7 `text_production`；8 `global_verifier`。subject identity 编码为 `RegistrySubjectKindTag, CanonicalDomainOrRecordName : UTF8 bytes, NumericTag : U`；没有自身 numeric tag 的 global subject 使用下表分配值。enum domain 先按 canonical domain name UTF-8 排序，domain 内 value 按 numeric tag；其他 subject 按 kind tag、canonical owner name、numeric tag 排序。

enum domain subject的`NumericTag`固定为该domain canonical name在当前`EnumRegistryProjection`全部domain name按UTF-8 unsigned byte lexicographic严格排序后的one-based ordinal；同名domain不得重复，domain内value仍使用其显式numeric tag。其他subject使用本章显式分配的record/shape/payload/opcode/stage/text-production/global tag。这样EnumDomain无需借用只覆盖Attribute value的`EnumDomainTag`，也不存在未定义NumericTag。

`TextRendererKindTag` 闭集为：1 `structural`；2 `generic_record`；3 `function_record`；4 `generic_operation`；5 `branch`；6 `switch`；7 `eh_match`；8 `eh_end_catch`；9 `call`；10 `reflect_call`；11 `abi_call`；12 `lookup`；13 `await`；14 `region`；15 `dynamic_control_path`；16 `module_registration`；17 `normalized_hir`。`TextRendererProjection` 对每个 record、opcode、stage opcode 和下列 text production 编码 `SubjectIdentity, TextRendererKindTag, RendererRevision : U`，revision 1 全为 1。mapping 固定如下：

| TextProductionTag | Canonical subject name | Renderer |
| ---: | --- | --- |
| 1 | `file` | `structural` |
| 2 | `header` | `structural` |
| 3 | `table_wrapper` | `structural` |
| 4 | `dynamic_control_path` | `dynamic_control_path` |
| 5 | `normalized_hir_v1` | `normalized_hir` |

- file/header/table wrapper 使用 `structural`；普通 typed record 与 Plan node 使用 `generic_record`；Function 使用 `function_record`；RegistrationSummary/ModuleRegistration 使用 `module_registration`；DynamicControlPath 使用 `dynamic_control_path`；Template.NormalizedHirPayload字段嵌入`normalized_hir_v1` renderer而非Bytes renderer；
- 默认 opcode 使用 `generic_operation`；`cf.br|cf.cond_br` 使用 `branch`，`cf.switch` 使用 `switch`，`eh.match`/`eh.end_catch` 分别使用对应 renderer；
- `call.*|async.call|async.invoke|async.continuation_invoke|decorator.continuation_invoke` 使用 `call`；`reflect.call` 使用 `reflect_call`；`abi.call|abi.invoke` 使用 `abi_call`；四个 `reflect.lookup_*` 使用 `lookup`；`async.await|async.await_copy` 使用 `await`；`decorator.region` 使用 `region`。

`VerifierCallbackKindTag` 闭集为：1 `enum_value`；2 `record_payload`；3 `shape_arity`；4 `payload_semantics`；5 `opcode_semantics`；6 `plan_semantics`；7 `canonical_graph`；8 `canonical_text`；9 `derived_digest`；10 `normalized_hir_v1`；11 `decoder_budget`；12 `verify_staged`；13 `verify_closed`；14 `origin_interning`；15 `global_initialization_plan`；16 `module_registration`。`VerifierCallbackProjection` 的每项固定编码 `VerifierCallbackKindTag, SubjectIdentity, CallbackName : UTF8 bytes, CallbackRevision : U, InputSchemaDigest : D32`，其中 `CallbackName = "ink.verify." + canonical callback-kind name + "." + canonical subject identity`，revision 1 全为 1，`InputSchemaDigest = H("ink.verifier-input-schema.v0", subject的完整normalized registry entry)`。每个 enum domain、record、shape、payload、opcode 和 StageOpcode 分别自动绑定 kind 1—6；kind 7—16 各绑定下面同 tag 的 global subject。这样新增、删除、改绑或改 revision 都会改变摘要，且实现 symbol、链接地址和编译器函数排列不会进入身份。

| Global subject tag | Canonical subject name |
| ---: | --- |
| 7 | `canonical_graph` |
| 8 | `canonical_text` |
| 9 | `derived_digest` |
| 10 | `normalized_hir_v1` |
| 11 | `decoder_budget` |
| 12 | `verify_staged` |
| 13 | `verify_closed` |
| 14 | `origin_interning` |
| 15 | `global_initialization_plan` |
| 16 | `module_registration` |

`EnumRegistryProjection`、`RecordRegistryProjection`、`ShapeRegistryProjection`、`PayloadRegistryProjection` 与 `OpcodeRegistryProjection` 使用相同 subject identity，并编码本章对应表的全部规范列；type/CFG rule 不直接 hash localized prose，而由完整 normalized input schema、callback identity/revision及其他列共同承诺。任何散文规则若改变 callback 结果，必须同步提升对应 callback revision和`IrSemanticsRevision`；只改 renderer bytes则提升 renderer revision与`TextSyntaxVersion`。

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
| `RegionPath` | `Vec<RegionPathStep>`；step 为 `OwnerBlockOrdinal : U, OwnerOperationOrdinal : U, RegionOrdinal : U` |

required payload 内没有宿主对齐 padding。outer record 的 `RecordLength` 必须恰好覆盖全部 required payload 和 extension TLV；decoder 不得用 C++ struct layout 读取。

`SourceStructuralPath` 是 non-empty `Vec<SourceStructuralStep>`；step 的 required 顺序固定为 `OwnerSyntaxKindTag : U, ChildRoleTag : U, ChildOrdinal : U`，三者必须是 `SourceStructuralSchemaVersion` 选中的语言语法 registry 中已登记且可从父节点机械重放的组合。revision 1只注册`SourceStructuralSchemaVersion=1`；它独立于Manifest的`NormalizedHirSchemaVersion`，所以Closed artifact虽不携带HIR仍可解码source identity。Staged artifact中SourceStructuralSchemaVersion必须等于NormalizedHirSchemaVersion；`TemplateRolePath`、binding definition path与declaration内部HIR path复用该 `SourceStructuralPath`。

logical path的唯一normal form是valid UTF-8 NFC、以`/`分隔、相对module/capability root且至少一个非空segment；禁止前导/尾随`/`、空segment、`.`、`..`、NUL、反斜杠、drive-letter/colon与UNC spelling。identity按normalized UTF-8 bytes大小写敏感比较，不采用宿主case-fold。冻结BuildInputSnapshot必须把host path、symlink/junction/reparse-point和hard-link alias解析到capability root内，拒绝越root、同一声明输入的host alias冲突以及任何BuildOutput alias；host spelling绝不进入artifact identity。

`StableSourceFileIdentityPayload` required顺序固定为`CanonicalModuleIdentityUtf8 : Bytes, SourceRoleTag : U, LogicalPathUtf8 : Bytes`；`SourceFileIdentityPayload`是其后追加`SourceFileContentDigest : D32`的revision identity。`StableDeclarationStep` required顺序为`DeclarationKindTag : U, DeclaredNameUtf8 : Opt<Bytes>, AnonymousRoleTag : U, AnonymousOrdinal : U, DeclarationSignatureDigest : D32`；DeclaredNameUtf8内部是length+NFC UTF-8 bytes而非artifact-local StringId。AnonymousRoleTag复用ChildRoleTag数值域但只允许`top_level_item|member_item|block_item`三种declaration-bearing role；值0是该字段专用的named sentinel，不是有效ChildRoleTag。named step要求nonempty name present且两个anonymous字段为0；anonymous step要求name absent、role为上述非零tag，并在同parent/kind/role分区按source order从0连续。signature digest使用§9.2不读取body/origin的准确declaration signature surface；因此注释、函数body或同文件其他声明变化不改变named declaration step，重命名/改签名则改变identity。`StableDeclarationPath`是从source-file root到声明的non-empty step vector；每个prefix必须在当前revision的source tree按name/kind/signature或anonymous role/ordinal唯一解析。

`SourceDeclarationKey` 字段虽然用 `Bytes` 承载，但其内部唯一编码固定为`StableSourceFileIdentity : StableSourceFileIdentityPayload, SourceStructuralSchemaVersion : U, StableDeclarationPath`，必须恰好消费外层Bytes，禁止content digest、source offset、dense ID、偶然top-level child ordinal、别名spelling、尾随bytes或任意预先hash替代。当前revision通过独立SourceFileContentDigest与该stable key组合验证；同role/path的不同module version故意共享stable declaration lineage，而Template/anchor/callsite/control/expansion等revision identity都额外编码content digest。由此code-only更新能保留SymbolKey、nominal TypeSemanticIdentity与StableEntry lineage，同时任何source-backed cache identity仍对准确revision敏感。

source replay有明确验证层级：`verifyStaged`与`closeAndVerify`持有BuildInputSnapshot/SourceSyntaxSnapshot，必须按ContentDigest读取immutable source bytes并完整验证stable declaration lookup、structural path每个ordinal及HIR node终点；成功close把这些事实承诺进SemanticModuleDigest。独立`verifyClosed`不接收source bytes，只验证SourceFile identity/digest、path/tag canonical encoding、transition类型、所有冗余stable key/content digest绑定和由artifact内字段可重算的摘要，不声称从digest逆推出source tree。Closed中的source path是已承诺provenance/cache identity而非运行时memory-safety authority；若部署要求重新审计源码真实性，调用方必须重新执行closeAndVerify而不是给verifyClosed添加隐式host读取。

`RegionPath` 从 Function body root 开始；空 vector 唯一表示 root region。每个 step 在当前 region 中按 canonical block ordinal 选择 owner block、按该 block 的 operation ordinal 选择具有 nested region 的 owner operation，再按该 operation schema 的 region ordinal 进入 child。任一 ordinal 越界、指向无 region 的 operation、存在第二种路径到同一 region或使用 dense RegionId 代替 step 都拒绝。`NormalizedStructuralNodePath`就是从source declaration root开始的non-empty `SourceStructuralPath`。`NormalizedHirStructuralNodePath`另为non-empty `Vec<NormalizedHirStructuralStep>`，step required顺序是`ParentNormalizedHirNodeTag : U, FieldOrdinal : U, ChildOrdinal : U`；FieldOrdinal必须是该parent tag的中央field序号，scalar child的ChildOrdinal固定0，vector child按source ordinal计数。两类path都必须从对应root机械重放并恰好定位一个node。

本章所有 `H(domain, fields...)` 均使用 08 §12.1 的同一个 SHA-256 framing：UTF-8 domain 和每个 field 都先编码 canonical byte length；structured field 先按本 registry 的 binary schema（包括 `Vec` count、variant tag 和 required field order）形成唯一 bytes，再作为一个 length-prefixed field 输入。任何公式不得退化为裸拼接或实现私有对象布局。

### 2.2 Semantic projection

字段表的“投影”列只有三种值：

- `S`：进入 `SemanticModuleDigest`；引用字段不编码当前 dense ID，而引用 canonical graph projection；
- `K`：不改变语言语义，但进入 cache compatibility/static key，例如 text/binary version 和 compiler build；
- `N`：不进入语义或 cache identity，例如 origin 展示、display path 和打印 ID。

`K` 在 semantic/nonsemantic 二分中属于 nonsemantic。`S` projection 不包含自己的 digest 字段，避免循环。每个字段必须在本章显式标记，禁止按“整个 section 大概是 semantic”猜测。

使用`Bytes`作为binary envelope但声明custom semantic projection的字段必须先由对应verifier callback完整解码，再投影结构内容；绝不把raw bytes、length或其中dense table ID直接放入S projection。revision 1恰有两类注册custom projection：①`Template.NormalizedHirPayload`使用`normalized_hir_v1`，省略NodeByteLength/HirNodeId/OriginId，把Type/Constant/String/Symbol/Template引用替换为canonical semantic identity并保持tag/field/vector/optional结构；②`Dependency.ToolResourceRead.Selector`使用`tool_selector_v1`，按`ToolIdentity, ToolVersion, HandlerRevision, SelectorSchemaTag, SelectorSchemaRevision`选择受信typed-field decoder并投影解码后字段。两者都要求exact decode、canonical re-encode、budget与tail-consumption检查；解码失败时没有fallback bytes identity。任何新Bytes custom projection必须提升registry/IR semantics revision，不能由插件私自添加。

canonical graph projection 把每个 semantic record作为node，把每个S字段内的引用定向为 `record -> referenced dependency`，先求SCC。`RecordSubjectIdentity` 的required bytes固定为 `SectionKindTag : U, RecordSubjectTag : U, RequiredSchemaRevision : U`：SectionKindTag使用08 §6.1的准确section tag；有中央RecordKind/TypeKind/ConstantKind等top-level variant的section以该variant tag作为RecordSubjectTag，单一record schema使用0；RequiredSchemaRevision是10 §1.1中对应`record_payload` callback revision。Region、Block、Operation、member declaration、payload tuple等嵌套结构只进入其top-level owner的`NonReferenceProjection`，不另建graph node。每个引用 occurrence 的 `SemanticReferencePath` 是从required payload root开始的step序列：1 `field(FieldOrdinal)`；2 `variant(VariantTag)`；3 `optional_value(0)`；4 `vector_element(ElementOrdinal)`；5 `tuple_field(FieldOrdinal)`。ordinal均为无符号数值；map/set先按其已注册canonical order形成vector再编号。`EdgeLabel` 是该path按 `(StepTag : U, StepValue : U)` 逐step编码的完整bytes，同一record内必须唯一。`NonReferenceProjection(node)` 按record required schema编码全部S字段，但把每个引用值替换为固定单字节marker `0xFF`，保留其外层optional/variant/vector count与所有非引用scalar bytes。

revision 1 的`AnchorKindTag`闭集为：1 `symbol`；2 `function`；3 `global`；4 `runtime_declaration`；5 `nominal_type`；6 `source_file`；7 `template`；8 `plan_node`；9 `module_registration`。对应anchor bytes为：Symbol/Function/Global/RuntimeDeclaration使用完整`SymbolKey`；nominal Type使用owner SymbolKey；SourceFile使用`SourceRole, LogicalPath, ContentDigest`；Template使用`SourceDeclarationKey, TemplateRolePath`；PlanNode使用重算后的`CanonicalWorkKey`；ModuleRegistration使用重算后的`RegistrationIdentity`。其他record没有anchor。递归SCC必须至少含一个上述anchor，且SCC内所有present `(AnchorKindTag, AnchorBytes)`必须互异；只由anonymous structural type/attribute/constant等无anchor records组成的递归SCC直接拒绝。

SCC内部排序使用下列无hash、长度分帧的partition refinement；`BytesTuple(...)`表示每项先写canonical byte length再写bytes。初始signature为 `Sig0(n)=BytesTuple(RecordSubjectIdentity, NonReferenceProjection(n), AnchorPresent, AnchorKindTag-or-zero, AnchorBytes-or-empty)`；把SCC内不同Sig0按unsigned byte lexicographic order分配从零连续的partition ordinal。第i轮对每个node形成 `Sigi+1(n)=BytesTuple(Sig0(n), EdgeCount, Edges...)`，edge按EdgeLabel排序；每项编码EdgeLabel，然后internal target编码`0 + target当前partition ordinal`，已完成SCC的external target编码`1 + TargetCanonicalProjectionDigest`。按完整Sigi+1 bytes重新排序/编号，只允许分裂已有partition，不允许合并；partition不再变化时停止。每轮至少分裂一个class，所以最多`NodeCount-1`轮；若固定点仍有多node class则SCC具有不可区分成员，artifact拒绝，不枚举排列也不使用dense ID打破平局。

固定点每个class恰一node时，local ordinal就是final signature的unsigned byte lexicographic rank。按local ordinal编码每个node的`RecordSubjectIdentity, NonReferenceProjection, anchor, EdgeCount`及EdgeLabel；internal target写`LocalBackRef(local_ordinal)`，external target只写依赖node已经可用的`TargetCanonicalProjectionDigest`。该长度分帧node序列是ordinal-independent的`CanonicalSccProjectionBytes`，`CanonicalSccKey`就是这些bytes。SCC中local ordinal为L的node唯一派生`CanonicalRecordProjectionDigest = H("ink.canonical-record-projection.v0", CanonicalSccProjectionBytes, L)`；上一段的`TargetCanonicalProjectionDigest`就是external target的这个值，只能在dependency SCC完成后使用。condensation DAG再使用dependency-first稳定Kahn：每一步从当前所有出度为零的SCC中选unsigned byte lexicographic `CanonicalSccKey`最小者，编码后删除指向它的边；key重复意味着semantic projection完全重复，canonical writer必须结构intern为一个record，decoder见重复record则拒绝。最终artifact引用把已经确定的目标写成`SccRef(canonical_scc_ordinal, local_ordinal)`并同时携带/核对CanonicalRecordProjectionDigest；canonical SCC ordinal只参与dense引用，不进入CanonicalSccKey或任何semantic identity，因此插入无关SCC不会改写已有identity。printer dense ID从同一SCC/local顺序分配。该算法的exact step tag、初始signature、transition、终止界和tie-break均属于`canonical_graph` verifier callback revision，任何改变必须提升相应revision与`IrSemanticsRevision`。

### 2.3 Derived digest registry

除各记录章节另有更窄公式外，revision 1 的派生摘要固定如下。表中的 structured field 均先按本章required schema编码再作为单个H field；digest输出字段自身永不进入自己的preimage。

| Digest | Domain 与 ordered preimage |
| --- | --- |
| `TargetKey` | `H("ink.target-key.v0", TargetTriple, Cpu, SortedFeatureSet, DataLayout, Endianness, AddressSpacePointerLayouts, PrimitiveAndAggregateLayoutRules, TargetBlobConstantAbiRulesProjection, PdbTableDigest, StrictFloatMode, SubnormalMode, NanMode, CAbiProfile, ExceptionAbiProfile, ComptimeVirtualAddressRevision, TargetAbiRevision, RuntimeAbiRevision)`；revision 1的ComptimeVirtualAddressRevision固定为1并选择04 §2.1的canonical allocator；只提交下述不含TargetKey派生值的raw rules，不提交derived TargetBlobConstantAbiTable |
| `PdbTableDigest` | `H("ink.pdb-table.v0", CanonicalPdbRecipeTable)` |
| `SourceFileContentDigest` | `H("ink.source-content.v0", ExactSourceBytes)`；不规范化换行、BOM或Unicode，hash输入是SourceFile记录声明的准确bytes |
| `ModuleContentDigest` | `H("ink.module-content.v0", CanonicalModuleIdentity, CanonicallyOrderedModuleSourceInputs)`；source input按SourceFiles canonical order逐项编码`SourceRoleTag, LogicalPath UTF8 bytes, SourceFileContentDigest`，包括全部primary/included/generated_input且除此之外不接受隐式host input |
| `ModuleVersionContextKey` | `H("ink.module-version-context.v1", CanonicalModuleIdentity, ModuleContentDigest, TargetKey, TargetContextDigest, CanonicalRequiredFeatureSet, LanguageRevision, IrSemanticsRevision, TargetAbiRevision, RuntimeAbiRevision, RegistrationEncodingRevision, ActiveModuleDagDigest, ActiveDependencyInterfaceDigest, DirectImportBindingsDigest, SemanticOptionsDigest, CapabilityPolicyDigest, HandlerRevisionDigest, DependencyManifestDigest, ModuleRegistrationSetDigest)`；只含S/语义输入，这些都在生成version-local body前冻结且通过下述RegistrationContextIndependent验证无回指context/body；SchemaRegistryDigest、CompilerBuildId、PassPipelineRevision/Digest等K字段永不进入SymbolKey |
| `ArtifactVariantKey` | `H("ink.artifact-variant.v0", ModuleVersionContextKey, ArtifactKind, TextSyntaxVersion, BinaryContainerVersion, NormalizedHirSchemaVersion, SchemaRegistryDigest, CompilerBuildId, PassPipelineRevision, PassPipelineDigest)`；只用于cache/build compatibility和物理variant区分，不进入SemanticModuleDigest、DeclarationIdentity或SymbolKey |
| `LoadedVersionKey` | `H("ink.loaded-version.v0", SemanticModuleDigest, ArtifactVariantKey)`；loader/version owner、AOT物理mangling与旧/new pin并存使用该key，不得把K-only差异回灌到version_local SymbolKey |
| `TargetAbiTag` | `H("ink.target-abi-tag.v0", TargetTriple, Cpu, SortedFeatureSet, DataLayout, CAbiProfile, ExceptionAbiProfile, TargetAbiRevision)`；只能来自受信TargetABI registry entry并由loader重算 |
| `TargetContextDigest` | `H("ink.target-context.v0", TargetKey, CanonicalLayoutQueryTable, CanonicalPdbRecipeTable, RuntimeStorageAbiTable, TargetBlobConstantAbiTable)` |
| `TargetLayoutDigest` / record `LayoutDigest` | `H("ink.target-layout.v0", TargetKey, TypeSemanticIdentity, CanonicalSize, CanonicalAlignment, FieldAndBaseOffsets, DiscriminantAndNicheLayout, AbiClassification)`；两名称是同一digest，Global的值必须等于其Type的重算结果，registration tuple直接复用该值 |
| `RuntimeStorageAbiHash` | `H("ink.runtime-storage-abi.v0", RuntimeObjectKindTag, TypeArgumentSemanticIdentities, TargetKey, RuntimeAbiRevision, RegisteredSize, RegisteredAlignment, RegisteredOpaqueStorageDescriptor)`；全部字段来自TargetContext的唯一RuntimeStorageAbiTable entry |
| `SymbolKeyDigest` | `H("ink.symbol-key.v0", SymbolKind, CanonicalModuleIdentity, DeclarationIdentity, SignatureDigest, GenericDeclarationProvenanceOrEmpty, ClosedGenericArguments)`；StructuralPath仅为重算display字段，DeclarationIdentity区分source/generated/builtin，generic provenance使用下述含完整GenericDeclarationSignatureSurfaceVector的闭合marker而不保留open entity或自由signature D32，ClosedGenericArguments使用tagged结构而非仅value constants |
| `CapabilityPolicyDigest` | `H("ink.capability-policy.v0", CapabilityPolicyRevision, CanonicalCapabilityGrantSequence)`；sequence使用下述count/length framing与exact grant schema |
| `HandlerRevisionDigest` | `H("ink.comptime-handlers.v0", ComptimeHandlerRevision, CanonicalHandlerSequence)`；sequence使用下述count/length framing与exact handler schema |
| `PassPipelineDigest` | `H("ink.pass-pipeline.v0", PassPipelineRevision, CanonicalOrderedPassSequence)`；只进入K/cache compatibility，不进入SemanticModuleDigest |
| `DependencyManifestDigest` | `H("ink.dependency-manifest.v0", CanonicalDependencySemanticProjectionSequence)`；sequence使用下述count/length framing，排除DisplayIdentity等N字段 |
| `ActiveDependencyInterfaceDigest` | `H("ink.active-dependency-interface.v0", CanonicalActiveDependencyInterfaceProjections)`；projection按冻结active module DAG的dependency-first次序逐module编码下述闭合接口record，loader/compiler从实际依赖artifact重算 |
| `ActiveModuleDagDigest` | `H("ink.active-module-dag.v0", RootOrdinal, CanonicalModuleNodeSequence, CanonicalModuleEdgeSequence)`；三个structured field使用下述ActiveModuleGraph exact schema，node按dependency-first稳定Kahn顺序，edge按`FromOrdinal, ToDependencyOrdinal`数值序严格递增且无重复 |
| `DirectImportBindingsDigest` | `H("ink.direct-import-bindings.v0", CanonicalDirectImportBindingSequence)`；sequence直接取Manifest.direct_import_bindings的required编码并按下述provider/exposure规则重算，不能只摘要ImportedSymbolKey集合 |
| `SemanticOptionsDigest` | `H("ink.semantic-options.v0", CanonicallySortedSemanticOptions)`；每项为`OptionKey UTF8 bytes, SemanticOptionValueKindTag, CanonicalTypedValueBytes`，按OptionKey unsigned UTF8严格递增且无重复 |
| `NormalizedHirSemanticDigest` | `H("ink.normalized-hir.v1", NormalizedHirSchemaVersion, CanonicalHirTreeSemanticProjection)`；projection排除NodeByteLength、dense HirNodeId、OriginId和artifact-local table ID，引用改用canonical semantic identity |
| `LexicalEnvironmentKey` | `H("ink.lexical-environment.v0", SourceDeclarationKey, TemplateRolePath, CanonicallyOrderedVisibleBindingProjections)`；每项按`DeclaringSourceDeclarationKey, BindingDefinitionPath, BindingKindTag, OptionalNameUtf8, TypeSemanticIdentity, CaptureAccessTag`编码 |
| `ControlNodeKey` | `H("ink.control-node.v0", SourceFileContentDigest, SourceDeclarationKey, TemplateRolePath, NormalizedHirStructuralNodePath, NormalizedHirNodeTag)` |
| `TemplateSemanticDigest` | `H("ink.normalized-template.v0", TemplateKind, RegionKind, SourceDeclarationKey, TemplateRolePath, SourceFileContentDigest, NormalizedHirSemanticDigest, LexicalEnvironmentKey, SemanticCaptureProjection)` |
| `InstanceIdentityDigest` | `H("ink.instance-identity.v0", GenericDeclarationSymbolKey, CanonicalClosedGenericArguments, TargetKey)` |
| `SourceBackedAnchorKey` | `H("ink.source-anchor.v0", OwnerSymbolKey, RegionStructuralPath, SourceFileContentDigest, SourceDeclarationKey, NormalizedStructuralNodePath, AnchorRoleTag)` |
| `ParentElaborationContextDigest` | `H("ink.parent-elaboration-context.v0", ParentCanonicalWorkKeyOrEmpty, EnclosingInstanceOrEmpty, DecoratorApplicationOrEmpty, DynamicControlPath)`；后三项使用完整结构化payload |
| `CanonicalWorkKey` | `H("ink.elaboration-work.v0", StageOpcode, Inputs, TemplateSemanticDigests, ResultTypeSemanticIdentities, Sink, ParentElaborationContextDigest, RequiredCapabilities, DependencyWorkKeys)` |
| `DecoratorApplicationKey` | `H("ink.decorator-application.v0", ProducerSymbolKey, DecoratorDeclarationSymbolKey, ApplicationSourceBackedAnchorKey, ParentDecoratorApplicationKeyOrEmpty, EnclosingInstanceIdentityDigestOrEmpty)` |
| `SourceBackedCallsiteKey` | `H("ink.source-callsite.v0", ProducerSymbolKey, SourceFileContentDigest, SourceDeclarationKey, NormalizedStructuralNodePath, TaggedCalleeIdentitySemanticProjection)` |
| `ExpansionContextKey` | `H("ink.expansion-context.v0", DecoratorApplicationKey, DecoratorApplicationOrderPath, ParentSemanticOutputOrdinal, ExpansionSiteIdentityPayload)` |
| `CallSiteKey` | `H("ink.dynamic-callsite.v0", ProducerSymbolKey, SourceBackedCallsiteKey, EnclosingInstanceIdentityDigestOrEmpty)` |
| `IterationIdentityDigest` | `H("ink.iteration-identity.v0", ControlNodeKey, LoopKindTag, IterationOrdinal, IterationSemanticInput)`；for的input为CanonicalIteratedElementSemanticProjection，while的input为固定empty bytes，轮次只由ordinal区分 |
| `ExpansionIdentityDigest` | `H("ink.expansion-identity.v0", SourceDeclarationKey, ExpansionSite, EnclosingInstanceOrEmpty, CanonicalControlPath, TargetKey)`；ExpansionSite含准确template role与发起HIR node path，只用于完整性/cache key，不用于work-item排序 |

`CanonicalRequiredFeatureSet` 的 structured encoding 为 `Count : U` 后接按无符号数值严格递增、无重复的 `FeatureTag : U`；空集合也编码 `Count=0`。Manifest.required_feature_set、ModuleVersionContextKey、StaticModuleKey、instance/work memo 与 trusted TargetContext feature registry必须使用同一vector，不得把“v0通常为空”当作省略授权。

`TargetBlobConstantAbiRulesProjection`是构造TargetKey之前冻结的raw target input，required envelope为`RuleCount : U`后接fixed-schema rules；每条rule required顺序为`RegistryNamespaceUtf8 : Bytes, RegistryEntryTag : U, ByteLength : U, RequiredZeroBitMask : Bytes`，按`(RegistryNamespaceUtf8 unsigned bytes, RegistryEntryTag)`严格递增且无重复。namespace必须nonempty valid UTF-8 NFC，rule必须唯一选择一个允许target-opaque constant的registry_builtin Type entry；ByteLength必须positive，RequiredZeroBitMask恰有ByteLength bytes。确定TargetKey后，TargetContext才从每条rule和所选registry entry派生唯一`TargetBlobConstantAbiTable` entry：`TypeSemanticIdentity, TargetKey, LayoutDigest, ByteLength, RequiredZeroBitMask`；TargetKey等于刚重算的当前值，LayoutDigest按该TargetKey与Type重算，后两项逐字节等于raw rule，table按TypeSemanticIdentity bytes严格递增且不得多项、缺项或重复。完整derived table只进入TargetContextDigest；因此不存在`TargetKey -> TargetBlobConstantAbiTable -> TargetKey/LayoutDigest -> TargetKey`摘要环。

`CanonicalCapabilityGrantSequence` 的 required envelope 为 `Count : U`，再对每项编码 `ProjectionLength : U + CapabilityGrantProjection`；projection required顺序为 `CapabilityTag, TaggedCapabilityScope, ParameterCount : U, CanonicalPolicyParameters`。`CapabilityScopeTag` 闭集为1 `global`/empty、2 `logical_path_prefix`/`NormalizedLogicalPath : Str`、3 `config_namespace`/`NormalizedNamespace : Str`、4 `environment_name`/`EnvironmentNameProfileTag, NormalizedVariableName : Str`、5 `tool_identity`/`ToolIdentity : Str, ToolVersion : Str`、6 `build_output_namespace`/`NormalizedNamespace : Str`、7 `target_profile`/`TargetContextDigest : D32`。scope compatibility固定为：`fs_read|fs_write` 使用logical_path_prefix，`config_read` 使用config_namespace，`environment_read` 使用environment_name，`target_query` 使用target_profile，`build_log` 使用build_output_namespace，`network|process|clock|random|reflection|declaration_sink` 使用global；revision 1中其他组合拒绝。每个policy parameter恰按 `ParameterName : Str, SemanticOptionValueKindTag, CanonicalTypedValueBytes`编码，按ParameterName UTF-8 bytes严格递增且无重复；grant按完整projection bytes unsigned lexicographic严格递增且无重复。所有identity/name/path字符串必须nonempty valid UTF-8 NFC并经对应resource normalizer；writer canonicalize，decoder见别名、乱序或重复即拒绝。

`CanonicalHandlerSequence` 的 envelope 同样为 `Count : U + (ProjectionLength : U, HandlerProjection)...`；HandlerProjection required顺序为 `HandlerIdentity : Str, HandlerRevision : U, SemanticConfigCount : U, CanonicalSemanticConfig`，config item使用上段parameter的exact typed schema并按name严格递增。handler按HandlerIdentity UTF-8 bytes严格递增且无重复。`CanonicalOrderedPassSequence` 的 envelope 为 `Count : U + (ProjectionLength : U, PassProjection)...`，保留pipeline execution order；PassProjection required顺序为 `PassIdentity : Str, PassRevision : U, SemanticOptionCount : U, OrderedSemanticOptions`，option恰按 `OptionKey : Str, SemanticOptionValueKindTag, CanonicalTypedValueBytes`编码，按pass schema声明的从0开始option ordinal顺序且key无重复。HandlerIdentity、PassIdentity、config/option key必须nonempty valid UTF-8 NFC。handler集合与pass序列不得使用plugin object address、加载顺序或未分帧自由字符串参与摘要。

`CanonicalDependencySemanticProjectionSequence` 的 envelope 为 `Count : U`，再对每条record编码 `ProjectionLength : U + DependencySemanticProjection`。projection required顺序恰为 `DependencyRecordKindTag, TaggedKindIdentityPayload, ObservedDigest : D32, HandlerRevision : U, DependencyValidationTag`，KindIdentityPayload使用§10.1选中的typed schema；不编码DisplayIdentity、record flags/length、dense ID或Origin。records按完整projection bytes unsigned lexicographic严格递增且无重复；writer在生成Manifest前intern，decoder与candidate validator对乱序/重复拒绝，并以该同一顺序逐项重观察、重算ObservedDigest和DependencyManifestDigest。

当前artifact唯一`CurrentTargetKey`是由trusted TargetContext从上述raw target inputs重算并逐字节等于Manifest.target_key的值。任何required record、identity、Template、Plan、ModuleRegistration或sealed closure递归可达的`InstanceIdentityPayload.TargetKey`、`ExpansionIdentity.TargetKey`与`target_blob.TargetKey`都必须逐字节等于CurrentTargetKey；从dependency artifact镜像的payload还必须等于该binding所固定provider的Manifest.target_key，而active dependency本身必须具有相同TargetKey。字段及其内部digest即使彼此自洽，只要嵌入另一TargetKey也拒绝。

当前artifact唯一`CurrentTargetAbiTag`必须由Manifest.TargetKey选择的TargetContext按上表公式重算。除明确带自身TargetAbiTag的registry_builtin identity外，Type layout/C representation、Function、StableEntry、AbiImport/AbiBridge/AbiExport、CallPayload/AbiCallPayload及其他所有TargetAbiTag字段都必须逐字节等于CurrentTargetAbiTag；active dependency artifact还必须具有相同TargetKey/TargetAbiRevision。字段之间即使彼此自洽，只要不等于当前target也拒绝，不能在一个artifact内静默混用另一目标ABI entry。

`CanonicalActiveDependencyInterfaceProjection` required顺序为：`CanonicalModuleIdentity, IrSemanticsRevision, TargetKey, TargetAbiRevision, RuntimeAbiRevision, RegistrationEncodingRevision, ModuleRegistrationInterfaceDigest, ExportCount, Exports`。Exports恰为该module所有Visibility=`export`且具有下列public entity owner组合的symbols，按SymbolKey bytes严格递增且无重复；private/module symbol不得出现，export symbol不得遗漏。每项先编码`SymbolKey, SymbolKindTag, PublicDeclarationAttributeProjection`；revision 1因AllowedAttributeOwnerSet全空，PublicDeclarationAttributeProjection唯一编码为`Count : U = 0`，ABI/representation信息只来自后续typed fields，不存在第二套abi_tag attribute。随后按owner编码准确tuple：type为`SignatureTypeProjection, TypeSemanticIdentity, Opt<LayoutDigest>`且presence逐项等于Type record，只有按值ABI使用才额外要求present/LayoutComplete；Function owner只允许`stable_entry` bodyless logical declaration或`external+Function(EntryIdentity=extern)`，tuple为`FunctionSignaturePayloadWithSignatureTypeProjections, DefaultArgumentSemanticProjections, FunctionRecordKindTag, EntryIdentityTag, FunctionAbiDigest, BehaviorContractDigest, Opt<StableEffectEnvelopeHash>`，default vector逐项为none或完整constant semantic projection；普通`SymbolKind=function` definition、decorator和open generic declaration在revision 1不能export。global为`SignatureTypeProjection, TypeSemanticIdentity, GlobalStorageTag, MutabilityTag`，constant global还编码其Initializer的完整constant semantic projection；RuntimeDeclaration owner的runtime以及`external+AbiExport`为`RuntimeRecordKindTag, RuntimeAbiSurfaceProjection, AbiDigest`，surface使用§9.2定义的runtime_signature且不含owner/linked target/origin。同一external symbol不能兼有Function与AbiExport owner，其他SymbolKind/owner组合不能export。function body/effect exact summary、非constant initializer、origin、private/module symbol和registration具体record set不进入该接口projection。direct import root必须存在于对应Exports，且AOT assumption逐项绑定上述digest；任何公开ABI/layout/entry identity/contract/default value/constant value/registration-interface变化都会改变摘要。

`PublicInterfaceReferenceClosure`递归遍历每条Export的公开SignatureTypeProjection、closed generic arguments、default/constant semantic projection、public attribute value、RuntimeAbiSurfaceProjection以及Visibility=export的nested member selector signature。每个从这些公开可name/construct/call表面引用的非builtin Symbol必须满足以下之一：同module且Visibility=`export`并出现在同一Exports；属于active dependency module且出现在该module已验证Exports（显式re-export保留原owner SymbolKey）；DeclarationIdentity=registry_builtin且registry entry允许公开；或它是Visibility=`export`的base/field/enum_variant/interface_member selector，恰由一个已export nominal owner的nested public member projection拥有并按owner/kind/ordinal闭合。selector不进入顶层Exports且不能独立取址。完整nominal TypeSemanticIdentity/LayoutDigest仍封闭private/module base/field/variant及其私有类型以验证布局，但这些sealed implementation nodes不成为下游name lookup或public reference closure root。Export Symbol自身的DeclarationIdentity/InstanceIdentity preimage所引用的private provenance parent不属于可name表面，而由下述ProviderSealedDeclarationIdentityClosure闭合；它不能因参与SymbolKey就自动成为Export。Export root自身的ClosedGenericArguments及其递归type/value/declaration组成则始终属于公开semantic surface，只由本closure、registry builtin规则和普通typed direct-import规则验证，不进入sealed declaration identity closure。Runtime export的公开closure只遍历RuntimeAbiSurfaceProjection；被排除的linked target属于下述ProviderSealedRuntimeClosure而不是公开接口。default constant的symbol relocation仍只能指向export immutable global或stable entry。closure失败即拒绝生成/装载artifact，不能依赖下游猜测未导出的entity。

上述 source structural key 族严格复用 2.1 的 `SourceStructuralPath`/`RegionPath`，不得再次定义 opaque path bytes。`AnchorRoleTag`闭集为1 `before`、2 `after`、3 `replace`、4 `append`、5 `callsite`、6 `declaration`。所有`OrEmpty`先编码Present Bool；所有canonical semantic projection先编码长度。Manifest field `active_module_dag_sha256`必须等于`ActiveModuleDagDigest`，`semantic_options_sha256`必须等于`SemanticOptionsDigest`；decoder/compiler从实际冻结DAG与规范化option set重算，不信任字段自报。

`SemanticOptionValueKindTag`闭集为：1 `bool`，payload Bool；2 `uint`，payload U；3 `sint`，payload I；4 `string`，payload UTF-8 length+bytes；5 `bytes`，payload Bytes；6 `enum`，payload `EnumDomainSubjectIdentity + EnumValueTag : U`；7 `digest`，payload D32；8 `uint_vector`，payload `Vec<U>`。`EnumDomainSubjectIdentity` required顺序为`RegistryNamespace : Str, EnumDomainTag : U, EnumRegistryRevision : U`，三者必须唯一解析到10 §1.1 Enum Registry中的active closed domain，EnumValueTag也必须是该domain已分配值。它进入`EnumRegistryProjection`；unknown kind/domain、非canonical整数/UTF-8、重复option key或未由compiler semantic-option registry登记的key一律拒绝。

`LoopKindTag`闭集为1 `for`、2 `while`并进入`EnumRegistryProjection`。DynamicControlPath的loop step必须由其ControlNodeKey指向的NormalizedHirNodeTag重算准确kind；不得为while伪造element projection，也不得靠改变kind制造第二个identity。

`CanonicalExpansionControlStepTag`闭集为1 `branch`，payload `ControlNodeIdentityPayload, ArmOrdinal : U`；2 `loop`，payload `IterationIdentityPayload`。`CanonicalControlPath` 是按源码嵌套/执行顺序编码的 `Vec<TaggedCanonicalExpansionControlStep>`；空 path 表示声明不受结构化选择/迭代控制。branch的`ArmOrdinal`是对应normalized HIR node中source-ordered arm/then/else的zero-based ordinal，default/else仍占唯一ordinal；loop identity必须按上表重算：`for` 使用准确 canonical iterated-element semantic projection，`while` 使用 absent element，二者都以 zero-based semantic ordinal 区分相等元素或不同轮次。

`ClosedGenericArgumentTag`闭集与payload为：1 `value`/`Ref<Constant>`；2 `type`/`Ref<Type>`；3 `declaration`/`Ref<Symbol>`；4 `pack`/`Vec<TaggedClosedGenericArgument>`。所有引用必须在Closed合法，value不得是meta constant，pack保留嵌套边界与source order且递归深度受容器深度上限约束；禁止把type/declaration参数伪装成Staged-only constant。`InstanceIdentityPayload` required顺序固定为 `GenericDeclarationSymbol : Ref<Symbol>, Arguments : Vec<TaggedClosedGenericArgument>, TargetKey : D32, InstanceIdentityDigest : D32`，其中GenericDeclarationSymbol必须具有下述source-backed GenericDeclarationProvenance marker而不要求保留open entity，最后一项为N并按上表重算；任何引用instance的字段都编码这份payload，不能只保存无法重算的D32。

下列结构化identity payload被Plan parent context、ExpansionIdentity和ModuleRegistration共同复用，字段末尾的`Key`/`Digest`均不进入自身preimage并由verifier重算：

- `SourceBackedAnchorIdentityPayload = {OwnerSymbol : Ref<Symbol>, RegionStructuralPath : RegionPath, SourceFileContentDigest : D32, SourceDeclarationKey : Bytes, NormalizedStructuralNodePath : SourceStructuralPath, AnchorRoleTag : U, SourceBackedAnchorKey : D32}`；digest与stable source key共同选择准确source revision；
- `DecoratorApplicationFrame = {ProducerSymbol : Ref<Symbol>, DecoratorDeclarationSymbol : Ref<Symbol>, ApplicationAnchor : SourceBackedAnchorIdentityPayload, EnclosingInstance : Opt<InstanceIdentityPayload>, DecoratorApplicationKey : D32}`；`DecoratorApplicationIdentityPayload`是non-empty root-to-current `Vec<DecoratorApplicationFrame>`，第0项parent key absent，后续项parent key恰为前一项重算key，current key恰为最后一项；
- `SourceBackedCallsiteIdentityPayload = {ProducerSymbol : Ref<Symbol>, SourceFileContentDigest : D32, SourceDeclarationKey : Bytes, NormalizedStructuralNodePath : SourceStructuralPath, CalleeIdentity : TaggedCalleeIdentity, SourceBackedCallsiteKey : D32}`；`TaggedCalleeIdentityTag`闭集为1 `direct_symbol`/`Ref<Symbol>`、2 `decorator_symbol`/`Ref<Symbol(decorator)>`、3 `runtime_handler`/`RuntimeEffectHandlerTag : U, HandlerRevision : U`、4 `core_opcode`/`OpcodeTag : U`、5 `closed_function_constant`/`Ref<Constant(symbol_relocation)>`，tag 5要求constant Type=function_pointer、Addend=0且target为匹配function symbol；semantic projection展开准确canonical identity，禁止自由D32；`ct.register_module_item` callsite固定使用`core_opcode(ct.register_module_item)`；
- `ControlNodeIdentityPayload = {SourceFileContentDigest : D32, SourceDeclarationKey : Bytes, TemplateRolePath : SourceStructuralPath, NormalizedHirStructuralNodePath, NormalizedHirNodeTag : U, ControlNodeKey : D32}`；
- `ExpansionSiteIdentityPayload = {SourceFileContentDigest : D32, SourceDeclarationKey : Bytes, TemplateRolePath : SourceStructuralPath, NormalizedHirStructuralNodePath, ExpansionRoleTag : U}`；`ExpansionRoleTag`闭集为1 `region_expansion`、2 `template_activation`、3 `generated_declaration`；site只由Closed仍保留并可验证的source-file revision/declaration/path/tag preimage组成，不保存已删除Template的自由SemanticDigest；
- `ExpansionContextIdentityPayload = {DecoratorApplication : DecoratorApplicationIdentityPayload, DecoratorApplicationOrderPath : Vec<U>, ParentSemanticOutputOrdinal : U, ExpansionSite : ExpansionSiteIdentityPayload, ExpansionContextKey : D32}`；
- `CallSiteIdentityPayload = {ProducerSymbol : Ref<Symbol>, SourceBackedCallsite : SourceBackedCallsiteIdentityPayload, EnclosingInstance : Opt<InstanceIdentityPayload>, CallSiteKey : D32}`；
- `IterationIdentityPayload = {ControlNode : ControlNodeIdentityPayload, LoopKindTag : U, IterationOrdinal : U, IteratedElement : Opt<TaggedClosedGenericArgument>, IterationIdentityDigest : D32}`；for要求element present，while要求absent，digest公式中absent映射为固定empty bytes。

`DynamicPathStepTag`闭集为：1 `expansion`/`ExpansionContextIdentityPayload`；2 `call`/`CallSiteIdentityPayload, InvocationOrdinal : U`；3 `branch`/`ControlNodeIdentityPayload, ArmOrdinal : U`；4 `loop`/`IterationIdentityPayload`。因此`DynamicControlPath`保存每个内部摘要的完整preimage；Closed verifier不依赖已删除的Template或Plan表也能逐层重算。

结构化identity中的冗余绑定也是required validation：CallSiteIdentityPayload.ProducerSymbol必须等于其SourceBackedCallsite.ProducerSymbol；ExpansionIdentity.SourceDeclarationKey必须等于ExpansionSite.SourceDeclarationKey；ControlNode、ExpansionSite、anchor和callsite内的SourceDeclarationKey必须绑定同一stable declaration，其SourceFileContentDigest必须在当前module version唯一选择匹配role/path的SourceFile revision；verifyStaged/closeAndVerify再按结构路径定位所声明node，verifyClosed按§2.1层级只验证artifact内可重算部分。同一context重复出现的stable key、content digest、EnclosingInstance与decorator current frame必须逐项相等。每个Key/Digest不仅重算，还必须在当前payload内唯一解析；禁止通过外层、内层使用不同preimage但各自hash正确来拼接伪造context。

`ExpansionIdentity` 的结构化 payload required 顺序固定为 `SourceDeclarationKey : Bytes, ExpansionSite : ExpansionSiteIdentityPayload, EnclosingInstance : Opt<InstanceIdentityPayload>, CanonicalControlPath, TargetKey : D32, ExpansionIdentityDigest : D32`；最后一项不进入自身 preimage并由 verifier 重算。规范 work-item 顺序比较前五项的 canonical bytes：先逐字段 unsigned byte lexicographic，optional absent 在 present 前，path逐 step按数值 tag与payload字段比较且较短真前缀在前；禁止按`ExpansionIdentityDigest`、fixed-point round、worker完成次序、容器顺序或对象地址排序。两个位于同一declaration和instance但HIR callsite不同的expansion因ExpansionSite不同而绝不碰撞。

`SignatureDigest`唯一使用§9.2按SymbolKind与entity surface定义的`H("ink.declaration-signature.v0", SymbolKindTag, OrderedDeclarationSignatureSurfaces)`；不存在第二个`ink.signature.v0`公式。module content、registration和SemanticModuleDigest继续使用08中列出的domain，但其字段编码与排序必须引用本章。任何新增D32派生字段必须先在此表或对应record章节分配domain与完整ordered preimage，不能用“至少包含”代替公式。

`DirectImportBinding` required顺序为`FromModuleOrdinal : U, ImportedSymbolKey : D32, Exposure : Enum<ImportExposureTag>`，Exposure tag闭集为1 `private_import`、2 `reexport`。`CanonicalDirectImportBindingSequence`与Manifest.direct_import_bindings使用完全相同的`Count : U`后接fixed-schema bindings编码，不另加per-item length；vector按`(FromModuleOrdinal无符号数值, ImportedSymbolKey bytes)`严格递增且无重复，ImportedSymbolKey在整个vector也必须唯一。FromModuleOrdinal必须是ActiveModuleGraph中当前RootOrdinal通过一条直接`root -> dependency` edge指向的node；该node绑定的provider artifact Exports必须恰有一个同SymbolKey root，本artifact还必须有逐surface匹配的typed direct import。每个dependency typed import root恰有一个binding；current-module symbol、下述identity-only sealed node、缺typed root或从transitive node跳过direct provider的binding都拒绝。sealed traversal遇到provider Export/public nested selector时必须先结束sealed路径并按普通public boundary规则验证；该public root不是sealed node，适用其自身唯一binding/owner projection。resolver必须在生成任何version-local identity前冻结该sequence并重算DirectImportBindingsDigest；因此仅改变同一active DAG内的直接provider选择或private/reexport exposure也会改变ModuleVersionContextKey。

`Exports`的artifact-local收集规则固定为两组并集：①Symbol.CanonicalModuleIdentity等于当前Manifest.module_identity、Visibility=`export`且满足上表owner组合的本module roots；②Exposure=`reexport`的DirectImportBinding所指provider export roots。普通`private_import`即使provider Symbol原始Visibility=`export`也不自动成为当前module export。re-export保留原owner SymbolKey，但consumer解析只查binding指定的FromModuleOrdinal provider，因此A原始export、B re-export、C从B import时不会同时匹配A/B。并集按SymbolKey bytes严格递增且无重复，本module export不得遗漏。public semantic traversal必须携带seed mode：从当前module Export或reexport root进入的`export_surface`路径遇到dependency top-level symbol时要求唯一binding的Exposure=`reexport`；从private_import root自身typed surface/ClosedGenericArguments进入的`private_import_surface`路径只要求指向同一direct provider的唯一binding，Exposure可以是`private_import`，但该leaf若同时被当前module Export路径到达，则全局唯一binding必须升级为`reexport`。builtin、结构Type、literal Constant与public nested selector继续按§9.2不单独建立binding；因此private import、re-export与provider来源既不冲突又有唯一可编码区分。

## 3. Manifest FieldId

Manifest 使用按 `FieldId` 严格递增的顶层 TLV。下表 1—31 在 canonical v0 artifact 中全部 required；Closed artifact 的 `NormalizedHirSchemaVersion` 写 `0`，Staged 写 `1`。

Manifest section 的完整 payload 为 `FieldCount : U` 后接恰好 FieldCount 个 `(FieldId : U, FieldLength : U, FieldPayload : Bytes[FieldLength])`；v0 `FieldCount` 必须为 31，FieldId 必须严格递增并恰为 1—31。没有 FieldFlags、optional field 或 extension field；unknown、重复、缺失、乱序、长度未恰好消费一律拒绝。字段 payload 只含下表“编码”列本身，不再嵌套 Bytes length。

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
| 13 | `active_module_dag_sha256` | `D32` | N | 从required semantic ActiveModuleGraph重算的冻结 DAG 摘要 |
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
| 31 | `direct_import_bindings` | `Vec<DirectImportBinding>` | S | direct provider ordinal、SymbolKey与private/reexport exposure |

### 3.1 Manifest enum

| Enum | Tag | Name |
| --- | ---: | --- |
| `ArtifactKind` | 1 | `Staged` |
|  | 2 | `Closed` |
| `ContainerFlavor` | 1 | `Canonical` |
|  | 2 | `Diagnostic` |

binary fixed header ArtifactKind、Manifest `artifact_kind` 与 text outer `ink-ir 0.1 staged|closed` 是同一逻辑字段的三次完整性编码，必须逐值相等；任一不一致在第0层解码即使artifact整体无效。required section、stage legality和NormalizedHirSchemaVersion只在通过该相等检查后按该唯一值选择。

## 4. SectionKind 与 RecordKind

SectionKind 与 08 容器目录一致：

| SectionKind | Name | RecordKind registry |
| ---: | --- | --- |
| 1 | `Manifest` | 顶层 FieldId TLV，无 RecordKind |
| 2 | `Strings` | `StringCount + length-prefixed UTF-8`，无 RecordKind |
| 3 | `SourceFiles` | 1 `SourceFile` |
| 4 | `Origins` | 1 `Source`；2 `Instantiation`；3 `RegionExpansion`；4 `Synthetic`；5 `Merged`；6 `Builtin` |
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
| 17 | `ActiveModuleGraph` | 1 `ActiveModuleGraph` |

RecordKind tag 只在所属 section 内解释。required/semantic section 中任何其他值非法。Section 15 的未知 record 只有其 outer record length 合法且 record flags 声明 optional 时才可跳过。

Section 17 在 Staged/Closed 都是required semantic section，恰有一条RecordKind 1。`ActiveModuleGraph` required payload顺序为 `RootOrdinal : U, Nodes : Vec<ActiveModuleNode>, Edges : Vec<ActiveModuleEdge>`；node required顺序为 `CanonicalModuleIdentity : Str, ModuleContentDigest : D32`，edge required顺序为 `FromOrdinal : U, ToDependencyOrdinal : U`，全部为S。Nodes必须nonempty，RootOrdinal必须定位当前Manifest `(module_identity, module_content_sha256)` 且所有node都可从root沿 `module -> dependency` edge到达；node tuple无重复，edge端点有效、无self-edge并按`(FromOrdinal, ToDependencyOrdinal)`严格递增。canonical node order的唯一算法是：从所有零出度node（当前尚无未移除dependency）中选取 `(CanonicalModuleIdentity UTF-8 bytes, ModuleContentDigest bytes)` 最小者，追加后删除指向它的edge，直到全部节点完成；无ready node表示有环并拒绝。编码后每条edge必须满足`ToDependencyOrdinal < FromOrdinal`，全图可达性使root恰为最后一个node。`CanonicalModuleNodeSequence`/`CanonicalModuleEdgeSequence`分别编码`Count : U`后接上述fixed-schema tuples，Manifest.active_module_dag_sha256必须按§2.3重算相等。artifact内graph只使语义自描述；verification/loading context还必须提供每个non-root node对应的已验证dependency artifact，并逐项绑定module/content/interface/target identity，一个D32不能代替provider records。

Strings section 的完整 payload 为 `StringCount : U` 后接恰好 StringCount 个 `(ByteLength : U, Utf8Bytes[ByteLength])`。字符串按 UTF-8 bytes 字典序去重排序后分配 zero-based StringId；无效 UTF-8、语义 identifier 所需但不满足 NFC、重复或乱序均拒绝。每项必须至少被其他section的一个required或optional record引用；canonical writer只编码完整引用闭包，decoder在全artifact引用解析后拒绝零引用字符串。StringId 的 semantic projection 展开实际 bytes，不包含 dense ID。

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
| `ValueAccessTag` | 1 | `ro` |
|  | 2 | `rw` |
| `PlaceAccessTag` | 1 | `ro` |
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
| `ExceptionHandlerKindTag` | 1 | `typed` |
|  | 2 | `catch_all` |

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
|  | 4 | `reserved_constant` |
|  | 5 | `runtime` |
|  | 6 | `stable_entry` |
|  | 7 | `decorator` |
|  | 8 | `external` |
|  | 9 | `base` |
|  | 10 | `field` |
|  | 11 | `enum_variant` |
|  | 12 | `interface_member` |
| `DeclarationVisibilityTag` | 1 | `private` |
|  | 2 | `module` |
|  | 3 | `export` |
| `GlobalRecordKindTag` | 1 | `declaration` |
|  | 2 | `definition` |
|  | 3 | `imported` |
|  | 4 | `runtime_owned` |
| `GlobalStorageTag` | 1 | `static` |
|  | 2 | `thread_local` |
|  | 3 | `runtime_managed` |
|  | 4 | `extern` |
| `GlobalInitializationPolicyTag` | 1 | `none` |
|  | 2 | `eager_module` |
|  | 3 | `eager_thread_activation` |
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
|  | 4 | `reserved_extern_bridge`（任何artifact均拒绝） |
| `RuntimeRecordKindTag` | 1 | `opaque_type` |
|  | 2 | `effect_handler` |
|  | 3 | `dispatch_slot` |
|  | 4 | `reflection_descriptor` |
|  | 5 | `stable_entry` |
|  | 6 | `abi_bridge` |
|  | 7 | `runtime_global` |
|  | 8 | `abi_export` |
|  | 9 | `abi_import` |
|  | 10 | `interface_dispatch_table` |

`BehaviorContractBit` 使用 bit index：0 `nothrow`；1 `construction_nothrow`；2 `body_no_fail`；3 `hot_reload_stable`；4 `extern_no_unwind`。未分配 bit 必须为零。适用矩阵固定为：sync callable 只使用 `nothrow`；async callable 只使用 `construction_nothrow` 与 `body_no_fail`；`hot_reload_stable`当且仅当Function是LogicalStableCallable的`EntryIdentity=stable` declaration或该logical symbol映射的generated version_local_body，两者必须同为true，ordinary version_local/continuation/extern为false；`extern_no_unwind` 只允许 `EntryIdentity=extern` 且设置时必须同时满足对应 sync `nothrow`。不适用 bit 必须为零，不能靠取反构造 may-unwind/may-fail 状态。

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

`target.extern_call` 与 `inline_assembly` 不是 revision 1 的 `CapabilityTag`，不能出现在 capability policy、digest 或 handler dispatch 中；ComptimeWorld 遇到二者必须拒绝，配置、插件或未知字符串不能授予它们。

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
| 10 | `TargetDependent` | 无；结果依赖 TargetContext，但本身不是不可推测边界 |
| 11 | `ComptimeEffect` | `Enum<CapabilityTag>` |
| 12 | `RuntimeEffect` | `Enum<RuntimeEffectHandlerTag>` |
| 13 | `Control` | 无 |
| 14 | `DeclarationSink` | `Enum<SinkKindTag>` |
| 15 | `MayDiverge` | 无 |
| 16 | `PdbBoundary` | 无；执行目标原生 partial-domain behavior，禁止推测、复制或删除 |

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
| 2 | `pointer` | `Enum<ValueAccessTag>, Ref<Type>, AddressSpace : U` | SC |
| 3 | `reference` | `Enum<ValueAccessTag>, Ref<Type>` | SC |
| 4 | `slice` | `Enum<ValueAccessTag>, Ref<Type>` | SC |
| 5 | `array` | `Ref<Type> ElementType, ElementCount : U` | SC |
| 6 | `tuple` | `Vec<Ref<Type>>` | SC |
| 7 | `nominal_class` | `Ref<Symbol>, Enum<NominalCompletenessTag>, Enum<NominalRepresentationTag>, Vec<ClassBaseDeclaration>, Vec<ClassFieldDeclaration>, NominalSemanticPropertiesPayload, ClassVirtualDispatchPayload, Vec<InterfaceImplementationDeclaration>` | SC |
| 8 | `nominal_enum` | `Ref<Symbol>, Enum<NominalCompletenessTag>, Enum<EnumRepresentationTag>, Opt<Ref<Type>> DiscriminantType, Vec<EnumVariantDeclaration>` | SC |
| 9 | `nominal_interface` | `Ref<Symbol>, Enum<NominalCompletenessTag>, Vec<Ref<Type>> DirectAncestors, Vec<InterfaceMemberDeclaration>` | SC |
| 10 | `interface_reference` | `Enum<ValueAccessTag>, Ref<Type>` | SC |
| 11 | `function` | `FunctionSignaturePayload` | SC |
| 12 | `function_pointer` | `Ref<Type(function)>` | SC |
| 13 | `runtime_opaque` | `Ref<Symbol>, Vec<Ref<Type>>` | SC |
| 14 | `place` | `Enum<PlaceAccessTag>, Ref<Type>` | SC |
| 15 | `exception` | 空 | SC |
| 16 | `runtime_handle` | `Enum<RuntimeHandleKindTag>` | SC |
| 17 | `runtime_object` | `Enum<RuntimeObjectKindTag>, Vec<Ref<Type>>, RuntimeStorageAbiHash : D32` | SC |
| 32 | `meta_type` | 空 | S |
| 33 | `declaration_handle` | `Enum<SymbolKindTag>` | S |
| 34 | `comptime_sequence` | `Vec<Ref<Type>>` | S |
| 35 | `dependent` | `Ref<Template>, NormalizedHirStructuralNodePath, Vec<TaggedDependentTypeArgument>` | S |

`RuntimeHandleKindTag`：1 `type_snapshot`；2 `interface_snapshot`；3 `function_snapshot`；4 `member_view`；5 `exception_box`；6 `version_owner`；7 `version_pin`；8 `decorator_continuation`；9 `abi_handle`。snapshot、member_view、exception_box 都是不可存储的线性或 owner-bounded runtime handle，不属于 address-only用户对象；`exception_box` 只由 Task/exception runtime内部拥有，revision 1没有把它直接物化为用户SSA值的producer opcode。

`DependentTypeArgumentTag`闭集及payload为：1 `type`/`Ref<Type>`；2 `constant`/`Ref<Constant>`；3 `declaration`/`Ref<Symbol>`；4 `template_capture`/`CaptureIndex : U`；5 `instance_argument`/`InstanceIdentityPayload, ArgumentIndex : U`。path必须定位所引用Template中`NormalizedHirNodeTag=type_expression`且`TypeExpressionKindTag`恰为`dependent_name`、`generic_application`或`typeof`的node；其他type-expression kind都必须在进入Core前解析为非dependent Type。template_capture index必须落在该Template.Captures，semantic projection同时编码规范index与被选capture的完整SemanticCaptureProjection；instance index必须落在结构化InstanceIdentityPayload.Arguments，semantic projection编码对应TaggedClosedGenericArgument。TypeSemanticIdentity使用TemplateSemanticDigest、path、准确HIR kind和argument canonical identities，禁止opaque dependent key或与HIR重复而可能冲突的第二种form enum。该kind只允许Staged，Closed无条件拒绝。

`NominalCompletenessTag`闭集为1 `declaration`、2 `definition`。`NominalRepresentationTag`闭集为1 `ink`、2 `c`；`EnumRepresentationTag`闭集为1 `ink`、2 `c`、3 `fixed_discriminant`。这些不是自由Attribute，直接进入TypeSemanticIdentity与LayoutDigest preimage。member payload required顺序固定为：

本节`InkCallableSymbolRef`表示`Ref<Symbol>`且SymbolKind只能为`function|stable_entry`，必须有准确Function record；`HasImplementationBody`对function要求自身BodyPresence=true，对stable_entry要求在该symbol所属module的当前active/pinned version owner中解析到已验证StableEntry mapping及BodyPresence=true的version_local_body：同module查当前artifact，dependency查已验证provider artifact，consumer不得伪造本地mapping。任何进入nominal TypeSemanticIdentity且实际可执行、hot-reload-stable或public dispatch的callable role必须保存stable_entry logical symbol，ModuleVersionContextKey-dependent version_local_body只出现在StableEntry mapping，不得直接进入type/dispatch/member payload。abstract declaration-only的DeclaredFunction或InterfaceMemberDeclaration.FunctionSymbol例外：它必须是普通`function` symbol、bodyless declaration、HasImplementationBody=false，只描述signature/contract且永远不作为实际dispatch/call target；其concrete ImplementationFunction、AdapterSymbol、default implementation与special member仍必须是HasImplementationBody的stable_entry。

`ExecutableInkTarget(S, CurrentVersionOwner)` 当且仅当 S 有准确 `CallingConvention=ink` Function record，且满足以下其一：① `HasImplementationBody(S)`；② `EntryIdentity=extern`、RecordKind=declaration、BodyPresence=false，并且 linker/loader 已把它唯一解析到 SymbolKey、FunctionType、FunctionAbiDigest、BehaviorContractDigest 和 TargetAbiTag 逐项相等的 Ink logical ABI 外部定义。未解析 extern 可以作为链接前 declaration 保留，但不得进入可执行 image。abstract bodyless declaration、Closed decorator provenance 和 `CallableKind=destructor_body` 无条件不满足该谓词；constructor 只能经 direct call 使用准确 `initializing_receiver` destination，destructor_body 只能由已验证 `obj.destroy*` 析构链进入。每个可用于 indirect call 的function value/relocation、virtual/interface table target、reflection snapshot target 和 runtime adapter final target都必须证明其全部可达集合满足该谓词；`EntryIdentity`、签名相等或声明存在本身不是实现证明。

| Payload | fields，按顺序 |
| --- | --- |
| `ClassBaseDeclaration` | `SelectorSymbol : Ref<Symbol(base)>, BaseType : Ref<Type(nominal_class)>, SourceOrdinal : U` |
| `ClassFieldDeclaration` | `SelectorSymbol : Ref<Symbol(field)>, FieldType : Ref<Type>, Mutability : Enum<MutabilityTag>, SourceOrdinal : U` |
| `EnumVariantDeclaration` | `SelectorSymbol : Ref<Symbol(enum_variant)>, Discriminant : Ref<Constant>, PayloadType : Opt<Ref<Type>>, SourceOrdinal : U` |
| `InterfaceMemberDeclaration` | `SelectorSymbol : Ref<Symbol(interface_member)>, RootMemberSelector : Ref<Symbol(interface_member)>, FunctionSymbol : InkCallableSymbolRef, FunctionType : Ref<Type(function)>, SourceOrdinal : U` |

`ClassVirtualDispatchPayload` required顺序为`HasPrimaryVptr : Bool, Slots : Vec<VirtualSlotDeclaration>, DynamicDestroyEntry : Opt<Ref<Symbol(runtime)>>`。每个VirtualSlotDeclaration required顺序为`SlotSymbol : Ref<Symbol(runtime)>, RootDeclaringClass : Ref<Symbol(type)>, DeclaredFunction : InkCallableSymbolRef, FunctionType : Ref<Type(function)>, SlotOrdinal : U, ImplementationFunction : Opt<InkCallableSymbolRef>, AdapterSymbol : Opt<InkCallableSymbolRef>, IsAbstract : Bool`。Slots按SlotOrdinal从0连续；IsAbstract=true当且仅当ImplementationFunction与AdapterSymbol都absent。ImplementationFunction是source/inherited concrete override；需要receiver/return adjustment时AdapterSymbol必须是以它为唯一target的compiler-generated logical adapter，拥有独立stable mapping时保存stable_entry，否则AdapterSymbol必须absent，实际dispatch target分别为AdapterSymbol或ImplementationFunction。每个SlotSymbol恰有一条`RuntimeDeclaration(DispatchSlot, role=virtual_method)`，其OwnerSymbol是当前class、SlotOrdinal/FunctionType逐项相等，TargetFunction/EntryIdentity均absent当且仅当IsAbstract，concrete时TargetFunction等于实际logical dispatch target且EntryIdentity逐项匹配；反向每条该class virtual_method slot恰出现一次。derived class Slots以前缀逐项继承base的RootDeclaringClass/DeclaredFunction/FunctionType/ordinal，override只替换ImplementationFunction/AdapterSymbol且满足variance/contract，新增root slot追加；不能重排或改变既有slot type。HasPrimaryVptr当且仅当base已有vptr或Slots非空；false要求Slots/DynamicDestroyEntry都empty。HasPrimaryVptr=true时DynamicDestroyEntry必须present并唯一解析到`RuntimeDeclaration(DispatchSlot, role=dynamic_destroy, ordinal=0)`；其TargetFunction/EntryIdentity都必须present，TargetFunction固定为该class由`stable_adapter`生成的logical stable destroy adapter，signature为sync/void/nothrow、receiver是当前class的mutable place，version-local body按dynamic type执行完整derived-to-base destroy chain；它不属于Slots且`call.virtual`不能选择。declaration form使用`false, [], none`；完整定义、TypeSemanticIdentity和LayoutDigest都覆盖该payload，vptr位置/size由TargetContext从它重算。

`InterfaceImplementationDeclaration` required顺序为`InterfaceType : Ref<Type(nominal_interface)>, SourceOrdinal : U, Bindings : Vec<InterfaceMemberBinding>`；binding required顺序为`InterfaceMemberSelector : Ref<Symbol(interface_member)>, FunctionType : Ref<Type(function)>, Resolution : Enum<InterfaceResolutionTag>, ImplementationFunction : InkCallableSymbolRef, AdapterSymbol : Opt<InkCallableSymbolRef>, MemberOrdinal : U`。`InterfaceResolutionTag`闭集为1 `class_override`、2 `inherited_class`、3 `interface_default`。InterfaceMemberDeclaration为新root时RootMemberSelector必须等于SelectorSymbol；override时它必须等于某个strict ancestor member的RootMemberSelector，FunctionType/contract兼容，且同一interface对一个root最多声明一次。interface inheritance graph必须是DAG。`EffectiveInterfaceMemberOrder(I)`的唯一算法是：初始化空vector，按DirectAncestors source order递归访问各ancestor的effective vector，第一次遇到某RootMemberSelector时追加；随后按I自身member SourceOrdinal处理，未见root则追加，已见root只更新该root的most-derived declaration而不改变ordinal。由此diamond按root去重且DirectAncestors source order是唯一tie-break，不依赖dense ID。implementation vector按SourceOrdinal从0连续且interface无重复；每个Bindings按该effective order从0连续，恰覆盖所有root一次，InterfaceMemberSelector编码root selector，FunctionType与effective declaration准确相等。resolution先选最具体class override，再选继承class实现，最后在所有reachable、同root且HasImplementationBody=true的interface declarations中只允许继承偏序下唯一maximal default；两个不可比较maximal default或缺失实现都拒绝。ImplementationFunction必须是HasImplementationBody的stable_entry；adapter若present必须是compiler-generated、HasImplementationBody、type精确且以ImplementationFunction为唯一target的stable ABI-adjustment logical thunk；无需adjustment时必须absent。每个implementation恰有一条RuntimeDeclaration(InterfaceDispatchTable)逐项镜像，反向亦然；class TypeSemanticIdentity、Runtime AbiDigest与Target layout/table digest都覆盖相同bindings。

InterfaceMemberDeclaration.FunctionSymbol的HasImplementationBody为true当且仅当该member提供default implementation，此时必须是stable_entry；false表示required abstract member并必须是上述ordinary bodyless function。`interface_default` resolution只能引用唯一most-specific、HasImplementationBody=true的member callable；abstract member不能充当default，`class_override|inherited_class`的ImplementationFunction必须HasImplementationBody、可执行且满足contract。class任一VirtualSlotDeclaration.IsAbstract=true或任一interface required member无concrete binding时，该class是abstract：不得形成nominal_aggregate constant、actual-storage Global、constructor destination或exception payload实例；Closed中被证明为complete dynamic type的reachable call.virtual target也不得保留abstract slot。全部slot/binding concrete时才可实例化。

`NominalSemanticPropertiesPayload` required顺序为`DeclaredNoncopyable : Bool, StableAddressRequired : Bool, CopyConstructionEnabled : Bool, CopyAssignmentEnabled : Bool, DefaultInitializer : Opt<InkCallableSymbolRef>, Destructor : Opt<InkCallableSymbolRef>`。这些是closed declaration roles而非自由Attributes；present initializer/destructor必须是当前class准确receiver/type的唯一special member、HasImplementationBody并满足03/06的signature、result mode、lifecycle与nothrow规则。v0 copy没有用户Function body：CopyConstructionEnabled/CopyAssignmentEnabled只授权03 §10.3的结构化、不可失败、逐base/field typed copy，要求DeclaredNoncopyable=false且所有base/field递归Copyable；任一false或递归noncopyable使相应copy operation非法。Destructor present或任一base/field NeedsDestroy使NeedsDestroy=true；StableAddressRequired、vptr、base/field stable-address requirement向外传播。declaration form两个function absent但保留声明bits；definition必须给出完整roles。该payload进入TypeSemanticIdentity，影响ABI的部分进入Layout/AbiDigest，并唯一决定`obj.init.copy|assign.copy|destroy`、Task<T> admissibility和StaticRegistrationEncodable。

declaration form要求全部member/ancestor vector为空且LayoutDigest absent；enum declaration必须保留完整representation surface：`fixed_discriminant`要求DiscriminantType present且为source-declared闭合整数，`c`要求present且等于TargetABI registry唯一C enum underlying type，`ink`要求absent。definition的representation与DiscriminantType必须和同一Symbol declaration surface逐项一致，member vector按SourceOrdinal从0连续严格递增，selector symbol全局唯一并被准确owner Type恰好引用一次。v0 class只有single concrete base chain：ClassBases.size只能为0或1，若present其SourceOrdinal固定为0，base type必须complete且继承图无环；多继承与virtual base无schema、必须拒绝。field type必须满足对象字段规则。class `representation=c`还要求base absent、HasPrimaryVptr=false、整体满足`CRepresentationSafe(T, TargetAbiTag, TargetKey)`并按TargetAbiTag唯一C struct mapping布局；`ink`不承诺C layout。enum definition要求DiscriminantType按上述representation规则确定、每个Discriminant按下段唯一规范化且值唯一，payload type若present必须SizedObjectType；`representation=c`要求所有payload absent，`fixed_discriminant`使用显式DiscriminantType，`ink`使用语言布局选择的内部discriminant且不把它伪装为source-declared字段。interface DirectAncestors vector本身就是source order，ancestor必须是complete nominal_interface、无重复/继承环；member FunctionSymbol/FunctionType必须匹配唯一Function declaration和override contract。class/enum definition的LayoutDigest必须present并从TargetKey、representation、vptr/slot schema与完整base/field/variant graph重算；nominal interface自身不Sized且LayoutDigest absent，interface-reference descriptor另有布局。TypeSemanticIdentity覆盖representation、上述definition/dispatch graph、source order与selector identities，TargetContext不能用一个D32替代缺失的member preimage。

class declaration form的InterfaceImplementationDeclaration vector也必须为空；class definition必须编码完整effective interface set。`representation=c`还要求NominalSemanticProperties中无Destructor/StableAddressRequired、ClassVirtualDispatch为false/empty且interface implementation vector empty。enum的required Discriminant字段按representation唯一规范化：`ink`时必须是builtin `ptrsize`整数Constant，值恰等于该variant的非负SourceOrdinal并可表示；`c|fixed_discriminant`时类型必须准确等于present DiscriminantType并使用source-declared显式值。两类都要求值唯一；`enum.discriminant`返回的始终是上述语义分支SourceOrdinal，不暴露c/fixed物理值或niche布局，FrozenEncodingDescriptor的`source_ordinal`规则与此一致。

`tuple` 的 element vector 必须非空；源码空元组在进入 Core 前唯一规范化为 builtin `unit`，不得再编码第二个零元素 tuple type。pointer 的 `AddressSpace=0` 是 v0 默认 integral address space；revision 1 没有注册其他 address-space extension，因此 Staged/Closed verifier 必须拒绝非零值。未来扩展必须同时注册整数往返、比较、布局、lowering 和 feature identity，不能只放宽该整数。

`RuntimeObjectKindTag` v0 只有 1 `task`，且恰有一个 logical result type 参数 T。T 只允许 void、never，或 closed runtime-representable、Copyable(T) 且 CopyConstructionEnabled(T)，并且不得含 NoEscape 值；因此 noncopyable、copy-construction-disabled 的 result 在 v0 type verifier 即拒绝。canonical text 固定为 `!runtime-object<task,T>`。`TargetContext + RuntimeAbiRevision + kind + arguments` 必须重算并核对 `RuntimeStorageAbiHash`，由此提供有限 size/alignment；runtime object 总是 `AddressOnly`、`SizedObjectType`、`SealedRuntimeStorage` 且 noncopyable，可以形成 owner/destination place，但字段、对象 bytes 和物理布局私有，只能由匹配 kind 的 runtime schema 操作。generic `place.addr`/`place.deref`/projection、typed/raw load/store、copy/assign/generic destroy，以及覆盖任何 active/partial sealed range 的 byte operation 全部拒绝；只允许 storage allocation、destination/owner begin、capability rebind 和注册的 Task operation。`runtime_opaque` 与之相反：它不是 `SizedObjectType`，不得形成 place、取 `sizeof` 或承载 Task storage；`runtime_handle` 是线性 verifier capability，也不是用户可存储对象。interface reference 仍使用 TypeKindTag 10；Optional reference/interface descriptor 是普通可复制 address-only nominal/enum type，不得编码为 runtime object。

### 7.2 Type record payload

`Types.RecordKind = 1` 的 required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `TypeKind` | `Enum<TypeKindTag>` | S |
| 2 | `KindPayload` | 上表对应 payload | S |
| 3 | `AttributeSet` | `Opt<Ref<AttributeSet>>` | S |
| 4 | `LayoutDigest` | `Opt<D32>` | S |
| 5 | `Origin` | `Opt<Ref<Origin>>` | N |

`TypeSemanticIdentity`明确等于Type record的canonical graph projection digest，但投影只含`TypeKind, KindPayload, AttributeSet`，排除`LayoutDigest`与Origin；递归nominal/member/type引用仍按§2.2的SCC算法闭合。SemanticModuleDigest另外投影该TypeSemanticIdentity以及LayoutDigest的presence/value，所以layout仍是语义字段，却不会形成`TypeSemanticIdentity -> LayoutDigest -> TypeSemanticIdentity`自环。TargetLayoutDigest、Global.LayoutDigest和registration schema tuple都引用这个排除layout的TypeSemanticIdentity。`LayoutDigest` present当且仅当`LayoutComplete(T)`：可存储builtin（含unit）、closed pointer/reference/slice/array/nonempty tuple、complete nominal class/enum、function_pointer、interface_reference及runtime_object必须present并由TargetContext重算；function、nominal_interface本体、incomplete nominal、runtime_opaque、place、exception、runtime_handle、meta/declaration/comptime/dependent type必须absent。由此同一logical type不能靠optional presence形成第二种S projection。

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

它不包含 nothrow、entry identity、默认实参、可见性或 hot reload 状态。ReceiverKind与ReceiverType/entry physical binding的矩阵固定为：`none`要求ReceiverType absent且无receiver binding；`static_member`要求ReceiverType present并等于声明owner nominal type，但无receiver binding；`mutable_instance`与`const_instance`要求ReceiverType present且为准确complete nominal class T，并分别建立唯一`receiver(0) : !place<rw,T>`与`receiver(0) : !place<ro,T>`；`interface_receiver`要求ReceiverType present且为准确`interface_reference<access,I>`，建立唯一同access的interface place receiver并携带匹配descriptor/version proof；`initializing_instance`要求ReceiverType present且为准确complete nominal class T，建立唯一`receiver(0) : !place<init,T>` active transaction owner。constructor只允许sync/initializing_instance，destructor_body只允许sync/mutable_instance，async不允许initializing_instance或destructor_body；ordinary function不得使用initializing_instance。任何presence、access、type、role或binding count不匹配均拒绝。

`CFunctionType`是CallingConvention=`c`的唯一FunctionSignaturePayload形态：CallableKind=`function`、FunctionKind=`sync`、ReceiverKind=`none`且ReceiverType absent；每个parameter mode固定`value`，其logical type可为任意在`CAbiPositionPayload(parameter, ordinal, [])`下满足四参`CAbiSafe`的closed C scalar、pointer、enum或repr(C) aggregate，C convention的`value`不套用Ink scalar-only entry规则。logical result若为void则ResultPassingMode=`void`；否则必须closed、非never并在`CAbiPositionPayload(result, 0, [])`满足`CAbiSafe`，ResultPassingMode固定`value`，包括由TargetABI物理分类为sret的aggregate。`object|const_reference|mutable_reference|raw_pointer` parameter mode、`result_destination`、never、receiver、async、constructor/destructor_body/runtime_thunk callable kind与varargs全部拒绝；status/result-out等额外C channel仍必须作为普通value parameter/result存在并由§9.5的CAbiPositionPayload唯一指出，不能产生第二种signature形态。

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
| 7 | `body_no_fail` | Bool |
| 8 | `callee_kind` | Enum |
| 9 | `construction_nothrow` | Bool |
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

revision 1 的 `AllowedAttributeOwnerSet` 对上表全部20个key均为空：这些tag只保留既有binary identity，不得由Type、Global、Function、RuntimeDeclaration或Operation引用；因此上述record的optional `Attributes`必须absent，canonical `Attributes` section必须为空。operation dictionary中的`addend/alignment/as/base/callee_kind/fast/field/index/kind/method/predicate/slot/variant`来自typed `OpcodeSpecificPayload`，不创建Attribute；`nothrow`、`construction_nothrow`和`body_no_fail`只来自Function `BehaviorContracts`，不得用Attribute重复或覆盖。decoder/verifier遇到任何revision 1 Attribute/AttributeSet record、non-absent record Attributes或非零OperationAttributeCount都拒绝。未来revision重新启用某key时必须为其注册非空owner kind、唯一一致性callback并提升semantics revision。

Attribute 的 Array/Dictionary containment edge 必须形成有限 DAG；允许多个 parent 共享同一 immutable child，但禁止直接或间接 containment cycle。该约束在分配递归 materializer 前验证，Attribute reference 不因此获得一般递归值语义。

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
| 12 | `enum_variant` | `Ref<Symbol(enum_variant)>, Opt<Ref<Constant>> Payload` | SC |
| 32 | `meta_type` | `Ref<Type>` | S |
| 33 | `declaration_handle` | `Ref<Symbol>` | S |
| 34 | `comptime_sequence` | `Vec<Ref<Constant>>` | S |

Constant 的 tuple/array/nominal_aggregate/enum_variant/comptime_sequence containment edge 必须形成有限 DAG；symbol relocation、type 和 symbol reference 不是 containment edge。tuple constant 的元素 vector 必须非空，零元素值唯一使用 ConstantKind `unit`。decoder 在 materialization、StaticRegistrationEncodable 和 semantic projection 前拒绝 containment cycle。

`Constant` required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `ConstantKind` | `Enum<ConstantKindTag>` | S |
| 2 | `Type` | `Ref<Type>` | S |
| 3 | `KindPayload` | 上表 | S |
| 4 | `Origin` | `Opt<Ref<Origin>>` | N |

kind与Constant.Type的闭合矩阵为：`int`只接受准确builtin integer/ptrsize且APInt.BitWidth等于类型位宽；`bool`只接受builtin bool；`float`只接受与FloatBits format相同的builtin float；`null`只接受address space 0 pointer，reference/interface-reference不以null表示；`unit`只接受builtin unit；`symbol_relocation`只接受pointer或function_pointer，target symbol的entity/ABI必须与pointee/function type相容，function relocation的Addend固定0且target必须满足LogicalStableCallable或是注册的稳定Ink extern entry，version_local/continuation/runtime body一律拒绝；data addend必须落在同一获准immutable static object且目标宽度可表示；`tuple`要求TypeKind tuple、元素数与逐项类型准确相等；`array`要求TypeKind array、元素数等于Type.ElementCount且逐项类型等于ElementType；`nominal_aggregate`只接受complete nominal_class，payload首个Symbol必须是该class owner type symbol，values按可选single concrete base后全部ClassFieldDeclaration的source order编码且逐项类型准确；`enum_variant`只接受complete nominal_enum，selector必须恰由该enum拥有，Payload presence/type等于EnumVariantDeclaration.PayloadType，discriminant从member declaration唯一派生而不重复编码；`string`只接受准确core.String且bytes是canonical Unicode scalar sequence的UTF-8；`meta_type`只接受TypeKind meta_type；`declaration_handle`只接受匹配SymbolKind的declaration_handle type；`comptime_sequence`只接受TypeKind comptime_sequence且元素数/逐项类型准确匹配。任何kind/type/shape不匹配都在semantic projection前拒绝。

aggregate constant只描述上述逻辑组成部分，不是aggregate SSA，也不编码padding。`target_blob`只接受TargetContext中`TargetBlobConstantAbiTable`按TypeSemanticIdentity唯一登记的closed、trivially copyable、无pointer/reference/relocation、无padding不确定性、无lifetime/resource语义的target-opaque registry_builtin constant type；entry required顺序为`TypeSemanticIdentity, TargetKey, LayoutDigest, ByteLength, RequiredZeroBitMask`，并按§2.3从无TargetKey派生值的raw TargetBlobConstantAbiRulesProjection唯一派生。entry.TargetKey与payload.TargetKey都必须逐字节等于当前Manifest.target_key，entry.LayoutDigest与payload.LayoutDigest都必须等于当前TargetKey下该Constant.Type的重算值；bytes长度必须等于ByteLength，mask覆盖的padding/reserved bits必须为零。未登记type、runtime/meta/user aggregate、含relocation或NeedsDestroy type一律拒绝。普通constant保留逻辑String值，StaticRegistrationEncodable/registration image lowering才把它materialize为module-owned immutable bytes + length，且不建立普通可析构String object。nominal aggregate和enum payload可以递归引用string constant。

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

SourceFiles canonical order 严格按 `(CanonicalModuleIdentity, SourceRoleTag, LogicalPath UTF-8 bytes, ContentDigest)` 的逐字段 lexicographic order；同内容但不同 logical path 由 path bytes 确定先后。artifact只承载当前module的source inputs，因此每条SourceFile.CanonicalModuleIdentity必须恰等于Manifest.module_identity；该 tuple 在 table 内必须唯一，重复 record 非法。`ModuleContentDigest`逐项使用的module identity也因此唯一等于该值，不能用per-file伪造identity制造与StaticModuleKey不同的解释。`DecoratorApplicationOrderPath` 的 root ordinal 只基于此唯一 source-file order，再基于各文件内 lexical decorator-application traversal 分配。

Origin section 全部字段为 `N`：

| RecordKind | Required payload，按顺序 |
| ---: | --- |
| 1 Source | `SourceFileId, Start : U, End : U` |
| 2 Instantiation | `GenericSymbolId, RequestOriginId, ClosedInstanceSymbolId, ParentOriginId` |
| 3 RegionExpansion | `ControlOriginId, SelectedBodyOriginId, IterationOrdinal : U, IterationIdentityDigest : D32, ParentOriginId` |
| 4 Synthetic | `ReasonStringId, ParentOriginId` |
| 5 Merged | `PrimaryOriginId, Vec<RelatedOriginId>` |
| 6 Builtin | `RegistryNamespaceStringId, RegistryEntryTag : U` |

Source与Builtin是无parent root；其余payload中的parent/related origin必须先编码，整体DAG无环；range 为文件内半开字节区间。Builtin只允许匹配registry_builtin DeclarationIdentity或其compiler-generated descendants，namespace/tag逐项相等。writer 必须按完整required payload对Origin DAG做结构化intern，相同record只保留一个OriginId；decoder/verifier拒绝重复结构record。canonical order为root/parent先于child，再按RecordKind和required payload canonical bytes排序，因此不使用插入顺序或反向use-site打破平局。

### 9.2 Symbol record

`Symbol` required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `SymbolKind` | `Enum<SymbolKindTag>` | S |
| 2 | `CanonicalModuleIdentity` | `Str` | S |
| 3 | `Visibility` | `Enum<DeclarationVisibilityTag>` | S，但不进入SymbolKey |
| 4 | `StructuralPath` | `Str` | N，按DeclarationIdentity重算display |
| 5 | `DeclarationIdentity` | `TaggedDeclarationIdentityPayload` | S |
| 6 | `SignatureDigest` | `D32` | S |
| 7 | `ClosedGenericArguments` | `Vec<TaggedClosedGenericArgument>` | S |
| 8 | `SymbolKeyDigest` | `D32` | N，重算核对 |
| 9 | `DisplayName` | `Str` | N |
| 10 | `Origin` | `Ref<Origin>` | N |
| 11 | `GenericDeclarationProvenance` | `Opt<GenericDeclarationProvenancePayload>` | S，present时进入SymbolKey |

`GenericParameterKindTag`闭集为1 `value`、2 `type`、3 `declaration`、4 `pack`并进入EnumRegistryProjection。`GenericDeclarationSignatureTypeProjection`是marker专用的tagged value projection：1 `closed_type`/完整`SignatureTypeProjection`；2 `generic_parameter`/`ParameterOrdinal : U`；3 `dependent_expression`/`NormalizedGenericSignatureExpressionProjectionV1`；4 `pack_expansion`/`ParameterOrdinal : U, ElementUseKind : Enum<GenericSignaturePackUseKindTag>`。tag 1必须递归closed且所有named leaf按普通SymbolKey解析；tag 2要求ordinal有效且ParameterKinds对应项为`type`；tag 3的root必须是非`inferred` type_expression并至少含一个binder-relative/dependent/self leaf，只用于不能由tag 1/2/4表示的复合dependent/self expression，完全闭合type、准确单一type parameter reference和准确直接pack splice的退化tree分别必须改用tag 1、2、4；tag 4要求对应项为`pack`且只能出现在schema允许sequence splice的字段。`GenericSignaturePackUseKindTag`闭集为1 `type_sequence`、2 `function_parameter_sequence`、3 `generic_argument_sequence`，必须与所在field逐值匹配。

`NormalizedGenericSignatureExpressionProjectionV1`不是Template digest或旧HIR bytes，而是table-ID-free的递归structured value：required顺序为`SchemaVersion : U = 1, Root : InlineSemanticHirNode`；每个variant、optional和vector按§2.1 canonical编码，受独立node-count/depth/bytes预算。`InlineSemanticHirNode`只接受§10.2已有numeric tag的`identifier|literal|resolved_reference|dependent_reference|type_expression|unary_expression|binary_expression|call_expression|member_expression|index_expression|aggregate_expression|generic_application` expression/type node；沿相同node tag和field order递归内嵌children，省略NodeByteLength、HirNodeId、OriginId，禁止capture、TemplateId、TypeId、ConstantId、SymbolId、TemplateSemanticDigest、NormalizedHirSemanticDigest、SourceFileContentDigest、source range、statement/control/declaration node，不新增未分配numeric tag的inline node。identifier保持原field位置并唯一编码`IdentifierUtf8 : Bytes, IdentifierRoleTag : U`，其中bytes必须nonempty valid UTF-8 NFC；literal保持`LiteralKindTag`在前并把ConstantId替换为完整ConstantSemanticProjection；resolved_reference保持`ReferenceKindTag`在前并把SymbolId替换为下述tagged projection；type_expression保持`TypeExpressionKindTag`在前，把ResolvedType替换为`Opt<GenericDeclarationSignatureTypeProjection>`后再编码Operands，root为dependent_expression时不得通过该optional回指自身；普通scalar/operator/kind字段保持原tag，children递归内联而不编码HirNodeId。`TaggedGenericSignatureArgumentProjection`闭集为：1 `closed_value`/完整ConstantSemanticProjection；2 `closed_type`/GenericDeclarationSignatureTypeProjection；3 `closed_declaration`/SymbolKey；4 `parameter_ref`/`ParameterOrdinal : U, ExpectedKind : Enum<GenericParameterKindTag>`；5 `pack_ref`/`ParameterOrdinal : U, ElementUseKind : Enum<GenericSignaturePackUseKindTag>`；6 `self_declaration`/empty。resolved reference的external leaf使用该projection；dependent_reference与generic_application仍按原field order递归内嵌qualifier/identifier/argument children，pack occurrence唯一用tag 5承载且不另建pack_expansion node。tag 3必须唯一解析到当前artifact、registry或同provider public boundary中的Symbol记录而不是自由D32；tag 4只允许expected kind为`value|type|declaration`并逐项匹配ParameterKinds；tag 5匹配pack及所在splice context；tag 6只允许解析到当前marker source declaration，self type/receiver必须使用它而不能嵌入当前SymbolKey形成`SymbolKey -> marker surface -> SymbolKey`回边。dependent name保留规范lookup tag与NFC identifier/qualifier tree，已解析alias不能保留另一份spelling。

`GenericDeclarationSignatureSurfaceVector` required顺序为`DeclarationSignatureProjectionTag : U = 2, ParameterKinds : Vec<GenericParameterKindTag>, SurfaceCount : U, TaggedSurfaces`，vector必须nonempty，pack至多一项且只能最后；SurfaceCount固定1。它复用下述surface tag和non-type field order，但每个type slot改用`GenericDeclarationSignatureTypeProjection`：ProducedSymbolKind=`type`只接受`type_head`，`function|stable_entry`只接受`function_signature`，`global`只接受`global_signature`；其他surface tag/count拒绝。`GenericDeclarationProvenancePayload` required顺序为`ProducedSymbolKind : Enum<SymbolKindTag>, ParameterKinds : Vec<GenericParameterKindTag>, DeclarationSignatureSurfaces : GenericDeclarationSignatureSurfaceVector`；两个ParameterKinds逐项相等。ProducedSymbolKind只允许`type|function|stable_entry|global`并逐值等于当前parent Symbol.SymbolKind；当前Symbol必须source_backed、ClosedGenericArguments empty。marker-present Staged open entity从一开始就以该generic vector作为其唯一OrderedDeclarationSignatureSurfaces并按§9.2重算SignatureDigest，形成Closed时只是冻结同一vector，不得再从普通closed projection计算第二份；SourceDeclarationKey末尾StableDeclarationStep.DeclarationSignatureDigest必须逐字节等于重算值。Staged/closeAndVerify还必须从准确source declaration、generic binder顺序和仍存在的open entity重建整个vector并逐字段相等；独立verifyClosed exact-decode retained vector并重算SignatureDigest/SymbolKey，按§2.1的provenance层级不声称恢复已删除source/template。

`GenericMarkerSymbolKeyDependencyGraph`的node是从任一marker-present Symbol的完整SymbolKey preimage可达、且属于当前artifact或其ProviderSealedDeclarationIdentityClosure的全部Symbol；edge `S -> T`在S的DeclarationIdentity、ordinary/generic signature surface、GenericDeclarationProvenance、ClosedGenericArguments以及它们递归type/constant projection中每出现一个T.SymbolKey semantic leaf时建立，`self_declaration` sentinel不建立edge，registry builtin与已经独立验证并由ActiveModuleGraph pin住的dependency public boundary是terminal。每个D32 leaf必须先唯一解析到上述node或terminal，不能以未解析hash绕过图验证。revision 1要求该图为DAG；verifier先用claimed references检测环，再按dependency-first递归/memo重算每个SignatureDigest与SymbolKey，ready node之间无需identity tie-break且遍历顺序不进入任何preimage，全部key完成后才执行普通canonical record ordering。直接或间接回到当前/其他marker的环一律拒绝；例如A<T>签名引用B<T>且B<T>签名再引用A<T>不能靠§2.2 SCC处理，因为这里的边是SymbolKey D32 preimage而不是可重定位Ref edge。未来若支持这种互递归必须新增无哈希anchor/SCC identity并提升semantics revision。

marker present的source-backed Symbol在Staged可以拥有对应open generic entity和Template；形成Closed时这些open entity、dependent type、Template和body必须全部删除，只保留该Symbol identity snapshot。Closed中它固定Visibility=private、没有Type/Function/Global/RuntimeDeclaration owner，只能被同CanonicalModuleIdentity的generated generic_instance owner_symbol或InstanceIdentityPayload.GenericDeclarationSymbol引用；provider artifact拥有本地marker，consumer只能通过下述ProviderSealedDeclarationIdentityClosure逐字段镜像它。marker不能参与name lookup、Core operand/callee、constant/relocation、runtime/reflection/registration、Exports或direct import root。每个marker必须至少被一个保留instance/provenance payload引用，未引用marker必须删除。marker absent的Symbol继续受普通entity closure约束；该窄例外不是保留任意裸Symbol的授权。

`SymbolKindTag=reserved_constant` 在revision 1永远非法且decoder拒绝；它只保留历史tag，不对应entity record。命名常量统一编码为`SymbolKind=global`的唯一Global definition，固定`Storage=static`、`Mutability=constant`、`Initializer` present、`InitializerFunction`/`FinalizerFunction` absent、`InitializationPolicy=none`。因此常量值与类型都由Global record闭合，不存在缺payload的第二种`SymbolKind=constant`或`NamedConstant` record。Visibility=`export`当且仅当top-level entity属于CanonicalActiveDependencyInterfaceProjection.Exports，或nested selector作为其owner的public member projection导出；private/module selector仍可内嵌在完整TypeSemanticIdentity/LayoutDigest但不可被下游name lookup。`private`只允许当前module内部引用，`module`允许同一canonical module的其他artifact partition引用，active dependency跨module import只能引用export；非公开provider引用例外只有按规定field path到达并由已绑定provider artifact验证的nominal sealed implementation closure、ProviderSealedDeclarationIdentityClosure与ProviderSealedRuntimeClosure。version_local_body、continuation_local、global lifecycle helper及其physical dispatch/version thunk固定private；runtime可公开调用地址只能是LogicalStableCallable或明确extern/AbiExport record，version-local implementation绝不进入Exports。decorator在revision 1只能private/module且只能由同module staging application使用；Closed中若因provenance保留则固定private且不能作为runtime裸entry。普通`SymbolKind=function` definition同样只能private/module；跨module可执行Ink API必须发布为stable_entry logical declaration或明确Ink extern。Visibility不进入SymbolKey，但其变化进入Symbol S projection和active interface digest。

`DeclarationIdentityTag`闭集及payload为：1 `source_backed`/`SourceDeclarationKey : Bytes`；2 `generated`/`GeneratedDeclarationIdentityPayload`；3 `registry_builtin`/`RegistryNamespace : Str, RegistryEntryTag : U, RuntimeAbiRevision : U, TargetAbiTag : Opt<D32>`。GeneratedDeclarationIdentityPayload required顺序为`GeneratorKind : Enum<GeneratorKindTag>, Parent : TaggedGeneratorParent, GeneratedRole : Enum<GeneratedRoleTag>, SemanticOutputOrdinal : U`；parent tag为1 `owner_symbol`/`Ref<Symbol>`、2 `source_anchor`/`SourceBackedAnchorIdentityPayload`、3 `module_root`/`ModuleContentDigest : D32`、4 `versioned_owner`/`LogicalStableSymbol : Ref<Symbol(stable_entry)>, ModuleVersionContextKey : D32`、5 `module_version_root`/`ModuleVersionContextKey : D32`、6 `expansion_context`/`ExpansionContextIdentityPayload`、7 `versioned_entity_owner`/`OwnerSymbol : Ref<Symbol>, ModuleVersionContextKey : D32`。GeneratorKind numeric tag闭集为1 `generic_instance`、2 `decorator_expansion`、3 `abi_bridge`、4 `stable_adapter`、5 `reserved_dispatch_thunk`、6 `special_member`、7 `lowering_helper`、8 `version_local_body`、9 `runtime_metadata`；tag 5在revision 1永远拒绝，不存在canonical text spelling。GeneratedRole闭集为1 `type`、2 `function`、3 `global`、4 `runtime_declaration`、5 `selector`，不能只做宽泛“相容”判定：`type -> SymbolKind=type`，`global -> global`，`runtime_declaration -> runtime`，`selector -> base|field|enum_variant|interface_member`；`function` 还必须由下表 GeneratorKind 唯一收窄为 `function|decorator|stable_entry`。`LogicalStableCallable(S)`当且仅当S.SymbolKind=stable_entry，且S是source_backed并且GenericDeclarationProvenance absent；或S是generated `stable_adapter|special_member`、使用下述context-independent parent并最终锚定同module source-backed declaration；或S是generated `decorator_expansion`、role=function且其expansion lineage最终锚定同module source-backed decorator application；或S是generated `generic_instance`、role=function且其parent具有ProducedSymbolKind=stable_entry的GenericDeclarationProvenance。其他generated/builtin symbol不满足。generated owner graph与expansion-context ancestry整体必须是DAG并按dependency-first次序重算；versioned_owner/module_version_root/versioned_entity_owner中的ModuleVersionContextKey必须等于当前Manifest/input重算值，versioned_entity_owner.OwnerSymbol还必须属于当前module且自身identity不含ModuleVersionContextKey。空Bytes、伪造SourceDeclarationKey或仅保存CanonicalWorkKey/D32均非法。

`SemanticOutputOrdinal`不是producer可自由分配的“唯一编号”，而由GeneratorKind的闭合矩阵重算；不适用的parent/role组合与不等于下列导出值的ordinal都拒绝：

| GeneratorKind | required parent/role | SemanticOutputOrdinal的唯一推导 |
| --- | --- | --- |
| `generic_instance` | `owner_symbol`指向ClosedGenericArguments empty且GenericDeclarationProvenance present的source-backed parent snapshot；role由parent.ProducedSymbolKind唯一映射为type/function/global，当前Symbol.SymbolKind逐值等于ProducedSymbolKind、GenericDeclarationProvenance absent且ClosedGenericArguments non-empty；arguments顶层tag逐项匹配ParameterKinds，pack参数用唯一pack argument承载 | 固定0；不同实例由Symbol.ClosedGenericArguments区分，完全相同`(parent, arguments, role)`只允许一个symbol |
| `decorator_expansion` | root使用`expansion_context`；nested使用`owner_symbol`直接指向同lineage的generated decorator_expansion parent；role与generated output entity精确对应，function role只允许`SymbolKind=function|decorator|stable_entry` | root固定0，每个root各占一个连续ParentSemanticOutputOrdinal event且一个event只产生一个declaration；nested ordinal恰为parent的direct generated-child canonical semantic-output index，从0连续且不按role/presence压缩。nested parent chain必须唯一且最终到达root expansion_context；`ExpansionSite.NormalizedHirStructuralNodePath`只定位source HIR site，不能代替该generated owner chain |
| `abi_bridge` | direct bridge用`owner_symbol`指向唯一AbiImport或AbiExport，indirect InkToExternal bridge用`module_root`；role=`runtime_declaration`且`SymbolKind=runtime` | 固定0；indirect bridge的完整C/Ink type、TargetAbiTag与policy已进入其runtime SignatureDigest，完全相同surface必须intern为一个symbol；direct owner record必须反向唯一引用并逐字段匹配 |
| `stable_adapter` | `owner_symbol`指向拥有该adapter的nominal class；role=`function`，`SymbolKind=stable_entry` | 先列该class全部virtual slots（SlotOrdinal顺序），再按InterfaceImplementation SourceOrdinal、Binding MemberOrdinal列全部interface binding slots，最后在HasPrimaryVptr=true时列唯一dynamic-destroy slot；不按AdapterSymbol presence压缩。ordinal等于唯一owning slot在该完整序列中的index，且普通slot.AdapterSymbol或DynamicDestroyEntry对应DispatchSlot.TargetFunction反向恰等于当前symbol |
| `reserved_dispatch_thunk` | 无合法parent/role/SymbolKind | revision 1无条件拒绝；DispatchSlot/InterfaceDispatchTable已直接保存concrete stable target，ABI adjustment唯一使用`stable_adapter`，不存在第二个Core dispatch-thunk entity |
| `special_member` | `owner_symbol`指向nominal class；role=`function`，`SymbolKind=stable_entry` | `DefaultInitializer`固定0，`Destructor`固定1；对应NominalSemanticProperties字段必须反向唯一引用当前symbol，absent role不要求ordinal连续 |
| `lowering_helper` | `versioned_entity_owner`指向唯一Global；role=`function`，`SymbolKind=function` | `InitializerFunction`固定0，`FinalizerFunction`固定1；Global字段必须反向唯一引用当前symbol，absent role不要求ordinal连续；revision 1不允许无entity owner的module_version_root lowering_helper |
| `version_local_body` | `versioned_owner`指向LogicalStableCallable；role=`function`，`SymbolKind=function` | 固定0，且当前module唯一StableEntry RuntimeDeclaration必须反向引用它 |
| `runtime_metadata` | `owner_symbol`指向KindPayload中的直接owner；role=`runtime_declaration`且`SymbolKind=runtime` | 固定编码`16 * SubjectOrdinal + RuntimeMetadataRoleTag`：role tag 1为virtual DispatchSlot且SubjectOrdinal=SlotOrdinal，2为dynamic_destroy且SubjectOrdinal=0，3为InterfaceDispatchTable且SubjectOrdinal=InterfaceImplementation.SourceOrdinal，4为ReflectionDescriptor且SubjectOrdinal是同SubjectSymbol descriptor按`ReflectionDescriptorIdentityTuple`严格排序后的index，5为RuntimeGlobal且SubjectOrdinal=0；对应owner surface必须反向唯一确定当前RuntimeDeclaration，存在显式symbol字段时还必须逐ID相等，ReflectionDescriptor则由唯一index/cardinality完成反向验证；其他RuntimeRecordKind不得使用此generator |

source_anchor与module_version_root parent tag在revision 1保留给已注册未来generator schema，本表九种numeric kind均不能以它们绕过对应行；registry-provided builtin仍只使用registry_builtin DeclarationIdentity而不是generated。writer在分配SymbolKey前先从owner records、closed arguments与expansion/order context建立上述subject graph和序列；`closeAndVerify`还必须从VerifiedStaged expansion trace重放decorator root event与direct-child ordinal，artifact-only `verifyClosed`则按07的层级边界验证保留parent DAG、lineage、唯一性和连续性，不伪称能恢复已删除template。禁止按hash-map insertion、worker完成、fixed-point round、已有SymbolKey或待验证ordinal排序。所有owner的SignatureDigest/type_head/runtime_signature必须按§9.2排除被其slot引用的adapter/body/initializer/finalizer/metadata Symbol，slot ordinal推导也禁止读取child identity，避免parent-child hash回边。

ClosedGenericArguments的表示矩阵同样是闭合的：`source_backed` Symbol必须empty，只有source-backed generic parent snapshot允许GenericDeclarationProvenance present；`generated generic_instance`必须marker absent、arguments non-empty并逐项等于对应InstanceIdentityPayload.Arguments；其他generated kind固定marker absent且arguments empty；registry_builtin固定marker absent，arguments数量/种类逐项等于registry entry声明的generic arity。因而闭合实例只有generic_instance一种表示，不能同时编码为“source_backed declaration + arguments”；泛型stable function实例仍通过上段LogicalStableCallable规则获得自己的bodyless stable declaration、StableEntry mapping与version-local body。

outer module绑定固定为：source_backed Symbol.CanonicalModuleIdentity等于SourceDeclarationKey.StableSourceFileIdentity.CanonicalModuleIdentity；generated owner/source-anchor/expansion-context使用parent module，module_root/module_version_root使用当前Manifest module identity，versioned_owner要求LogicalStableSymbol module与当前Manifest module相等，versioned_entity_owner要求OwnerSymbol module与当前Manifest module相等，三种versioned parent的ModuleVersionContextKey都等于当前重算值；registry_builtin等于registry entry登记的canonical builtin module，RuntimeAbiRevision与TargetAbiTag presence/value也逐项匹配entry。`StructuralPath`不进入SymbolKey，只是N display并由identity唯一重算：source_backed取StableDeclarationPath的NFC lexical name chain，anonymous segment打印`$anon.<DeclarationKindTag>.<AnonymousRoleTag>.<AnonymousOrdinal>`；generated以owner/source-anchor/expansion-context/module-root/module-version-root/versioned-entity-owner的parent display为基，versioned_owner以LogicalStableSymbol display为基，再追加`$gen.<GeneratorKind>.<GeneratedRole>.<SemanticOutputOrdinal>`；registry builtin为`$builtin.<RegistryNamespace>.<RegistryEntryTag>`。每个segment使用§15.1 path-segment的唯一bare/quoted escaping，任何不一致拒绝。source-backed identity才适用§2.1的stable file/declaration重放；generated/builtin Symbol通过各自完整preimage在Closed独立验证。

`SignatureTypeProjection` 是不读取nominal definition body的闭合类型投影：builtin编码tag；pointer/reference/slice/array/tuple/function等结构类型递归编码kind与组成投影；nominal/runtime named type只编码owner SymbolKey与closed type arguments；dependent/meta type与anonymous structural recursion非法。普通`DeclarationSignatureSurfaceVector` required顺序为`DeclarationSignatureProjectionTag : U = 1, SurfaceCount : U, TaggedSurfaces`；marker专用`GenericDeclarationSignatureSurfaceVector`以tag 2开头并使用上文schema，两种encoding互斥且不能按偶然相同payload bytes互换。二者共同实现`OrderedDeclarationSignatureSurfaces`，surface tag闭集为：1 `type_head`；2 `function_signature`，普通vector编码使用SignatureTypeProjection替换type refs的FunctionSignaturePayload，generic vector逐type slot使用GenericDeclarationSignatureTypeProjection；3 `global_signature`，普通vector编码Type的SignatureTypeProjection、Storage、Mutability，generic vector替换为generic type projection；4 `runtime_signature`，编码RuntimeRecordKind与不含owner Symbol、linked target symbols、AbiDigest、Origin的ABI surface；5 `nominal_selector`，编码OwnerTypeSymbolKey、SelectorSymbolKindTag、SourceOrdinal，其中SelectorSymbolKindTag直接使用SymbolKindTag的`base|field|enum_variant|interface_member`四值子集，其他值拒绝。type_head的exact payload为：nominal_class编码kind、representation及NominalSemanticProperties四个声明Bool，不编码NominalCompleteness、base/field/special-member function/dispatch/interface bindings/layout/origin；nominal_enum编码kind、representation及source-declared explicit DiscriminantType projection或absent，不编码completeness/variants/layout；nominal_interface只编码`NominalTypeKindTag=nominal_interface`，generic/closed差异已由外层DeclarationSignatureProjectionTag 2/1唯一表达，不编码第二个surface字段，也不编码completeness/ancestors/members；runtime named type编码kind与registry name/type arguments。surface按tag再按完整canonical bytes严格递增且无重复。每个Symbol的`SignatureDigest`唯一重算为`H("ink.declaration-signature.v0", SymbolKindTag, OrderedDeclarationSignatureSurfaces)`：GenericDeclarationProvenance present的Staged open entity与Closed provenance-only Symbol都从marker内同一tag-2 DeclarationSignatureSurfaces取vector，Closed不要求entity owner；其余普通type/function/global分别从唯一owner取得tag-1 vector及恰有surface 1/2/3，decorator与Ink external function使用2，base/field/enum_variant/interface_member恰有5，`stable_entry`无论owner artifact同时具有StableEntry RuntimeDeclaration还是consumer只具有bodyless import Function都固定恰有surface 2，per-version mapping/runtime surface不进入逻辑SymbolKey，其他runtime/external symbol按其唯一owner编码surface 4。revision 1拒绝CallableKind=runtime_thunk，因此不存在Function+RuntimeDeclaration双surface。该投影不把同一owner SymbolKey回灌到自己的preimage，generic self leaf必须使用专用sentinel；同一forward/import declaration变成definition不会改变SymbolKey，完整definition、StableEntry mapping与runtime ABI变化仍由TypeSemanticIdentity、LayoutDigest、Function/Runtime digest和active dependency interface验证。

entity owner/cardinality矩阵是`record_payload`与`verify_staged/closed` callback的required规则：每个nominal Type的owner必须是`SymbolKind=type`，同一type symbol恰由一个nominal Type record拥有；每个ClassBaseDeclaration.SelectorSymbol、ClassFieldDeclaration.SelectorSymbol、EnumVariantDeclaration.SelectorSymbol与InterfaceMemberDeclaration.SelectorSymbol必须分别是`base`、`field`、`enum_variant`与`interface_member`，恰由一个匹配kind的nominal Type member vector拥有一次，且不得拥有Function、Global、RuntimeDeclaration或第二个member slot。selector的nominal_selector signature surface必须使用该owner Type.Symbol、该member vector中的SourceOrdinal和准确selector kind；同一owner/kind/ordinal或同一selector重复都拒绝。每个Global.Symbol必须是`global`，同一global symbol恰有一个Global record；每个Function.Symbol必须是`function`、`decorator`、`stable_entry`或`external`，并分别与ordinary callable、Staged decorator definition或下述Closed provenance declaration、`EntryIdentity=stable`、Ink ABI的`EntryIdentity=extern`相容，同一symbol恰有至多一个Function record；CallableKind=runtime_thunk和`SymbolKind=runtime` Function在revision 1拒绝，C import也不是Function而是AbiImport RuntimeDeclaration。每个RuntimeDeclaration.Symbol必须是`runtime`，唯独StableEntry允许`stable_entry`、AbiExport允许`external`；同一symbol恰有一个RuntimeDeclaration record，不能靠不同RuntimeRecordKind复用。当前module拥有的LogicalStableCallable是唯一跨Function/Runtime两表共享symbol的组合：它必须同时有恰一bodyless Function declaration与恰一StableEntry RuntimeDeclaration；后者.LogicalSymbol等于自身Symbol并指向恰一generated version_local_body definition。stable declaration、version-local body与Runtime record的FunctionTypeId、TargetAbiTag、FunctionAbiDigest、BehaviorContractDigest逐项一致；version body的ModuleVersionContextKey parent使不同semantic module context拥有不同SymbolKey并可在旧pin存活时并存，K-only variant则复用该SymbolKey并以ArtifactVariantKey/LoadedVersionKey隔离物理产物和version owner。active dependency所属stable symbol在本artifact只允许bodyless import Function projection，逐项匹配dependency Export，不得伪造本地StableEntry/version body；loader经dependency version owner解析mapping。AbiImport/AbiBridge/AbiExport是declarative boundary lowering records，backend从其双signature与policy合成C wrapper，不拥有第二个Function record。除StableEntry组合外，一个symbol不得被不同entity owner种类复用。

definition/declaration闭包固定为：本module中可执行function symbol必须有恰一Function record；FunctionRecordKind=`definition|generated_adapter`必须有body，DeclarationIdentity=`generated`本身不推出BodyPresence，generated LogicalStableCallable仍按bodyless stable declaration与per-version mapping矩阵；`generated_adapter`当且仅当当前Function.Symbol是`version_local_body`且其LogicalStableSymbol是generated `stable_adapter`，其他bodyful function固定`definition`，bodyless declaration/extern固定`declaration`。普通`SymbolKind=function` definition不得Visibility=export；跨module调用只导入bodyless stable_entry或extern projection。global symbol必须有恰一Global record，definition/runtime_owned才可带initializer/finalizer，declaration/imported不得带；当前module拥有的runtime/stable/export owner必须按§9.2矩阵具有相应RuntimeDeclaration，active dependency stable只具有bodyless Function import并由provider mapping解析。除上文严格受限且被实例引用的GenericDeclarationProvenance parent snapshot外，orphan definition symbol、kind不匹配、同表重复owner、缺少required entity或额外entity都拒绝。引用外部symbol仍必须通过当前artifact的import declaration entity闭合，不能只放一个裸Symbol record；ProviderSealedDeclarationIdentityClosure、nominal implementation closure与ProviderSealedRuntimeClosure是三个按规定field path可达的窄例外，均不能独立执行或lookup。

Staged decorator必须是具有body的Function definition并只在ComptimeWorld执行；形成Closed时其body、template与decorator op全部消除。若ModuleRegistration、Origin/Expansion/Callsite provenance或其他保留identity仍引用DecoratorDeclarationSymbol，Closed必须保留同一SymbolKey和signature的provenance-only Function：SymbolKind=`decorator`、Visibility=`private`、RecordKind=`declaration`、BodyPresence=false、EntryIdentity=`continuation_local`、CallingConvention=`ink`、DefaultArguments全absent、StableEffectEnvelope/hash absent且Attributes absent；CallableKind/FunctionKind/receiver/parameters/result、contracts及AbiDigest逐项保留以重算SignatureDigest。它不进入Closed Exports，不是runtime declaration或可执行entry，任何call/const.function/dispatch/reflection/runtime address都不得以它为target；未被任何Closed identity引用的decorator Symbol与Function必须一并删除，不能留下裸Symbol。

每个属于active dependency module的direct import Type、Function、Global或Runtime entity都必须具有唯一DirectImportBinding，并且只在binding.FromModuleOrdinal指定的provider `CanonicalActiveDependencyInterfaceProjection.Exports`中唯一解析；本地按§2.3公开投影算法重建的kind与完整surface bytes必须逐字节相等。每个root无论Exposure为private_import还是reexport，都必须对root.ClosedGenericArguments逐tag执行公开semantic reference验证：type递归SignatureTypeProjection，value递归完整constant projection，declaration解析公开top-level symbol或其export nominal owner的public nested selector，pack递归；builtin、无独立owner的结构Type、literal Constant和nested selector不建立独立binding，遇到nonbuiltin top-level dependency entity则必须具有指向同一FromModuleOrdinal provider的唯一binding。只在当前module把该leaf纳入Exports时要求其唯一binding为reexport；仅服务private_import surface时private_import足够。direct import不得自造或改写layout presence/digest、default arguments、constant initializer、entry identity、effect/behavior contract、ABI/failure policy或runtime protocol；缺少本地typed import entity、只存在裸root Symbol、匹配零项/多项或任一字段不等均拒绝。Function direct import只有bodyless stable_entry declaration或resolved extern两种，普通provider definition没有consumer import形态。为核对一个export nominal的完整TypeSemanticIdentity/LayoutDigest而闭包携带的private/module selector、私有类型及其他sealed implementation node不属于direct import root、不要求单独出现在Exports，也不得被name lookup或独立引用；它们必须来自同一已验证dependency artifact并逐项等于该export definition的递归闭包。

`ProviderSealedDeclarationIdentityClosure`从每个direct import root Symbol的DeclarationIdentity与GenericDeclarationProvenance中的reference field递归展开，但不从root.ClosedGenericArguments起步，也不遍历Function/Type/Global/Runtime entity surface。root的ClosedGenericArguments是公开semantic surface，必须按PublicInterfaceReferenceClosure、registry builtin与普通typed direct-import规则在consumer中重建；因此导入`Foo<Bar>`时，公开Bar若为nonbuiltin named type就必须是同一direct provider Exports中的普通typed import root并拥有自己的DirectImportBinding，绝不能作为无binding sealed Type混入。

sealed traversal的每个reference occurrence按以下互斥优先级分类：①TypeKind=builtin或registry_builtin DeclarationIdentity按当前registry、arity、Runtime/Target ABI与CurrentTargetKey验证后停止，不复制为sealed node；registry builtin自身的实际generic arguments仍按该occurrence所属public/identity模式递归，不能一并跳过；②provider Exports中的Symbol、由该Export拥有的typed entity，或已export nominal owner的public nested selector是public boundary，必须在此停止sealed traversal并分别走同一FromModuleOrdinal的普通typed direct-import binding或nested public owner projection；③仅当既非builtin也非public boundary时，才把provider-private/generated provenance Symbol、无独立公开root的结构Type/Constant及其规定identity reference作为identity-only sealed node继续递归。public boundary判定优先，所以同一node绝不能既以sealed身份免binding又以typed root身份独立使用。类别③node必须来自binding指定的同一provider artifact并逐项等于provider原record；不得从Symbol反向进入其Type/Function/Global/Runtime owner，只有identity payload明文引用的Type/Constant才沿其canonical semantic reference fields继续。generated parent、ExpansionContext/InstanceIdentity、GenericDeclarationProvenance marker及其GenericDeclarationSignatureSurfaceVector中的closed declaration/type/value leaf由此可重算import root的完整SymbolKey，而不是信任一个自由D32；self_declaration sentinel只回指当前marker并立即停止。identity-only node不需要Export或独立DirectImportBinding，且不能成为typed import root、name lookup/Core operand/callee、reflection/registration/relocation或re-export对象；多个import root可以共享同一identity node，但全部路径必须绑定同一provider且只能用于identity重算。任一node来自其他provider、字段被裁剪/改写、脱离至少一个合法identity path、错过应停止的public boundary或被独立使用都拒绝。由此provider预生成并export的closed generic instance可以保留private source-backed generic parent marker，而consumer不能实例化或观察该open declaration。

Runtime direct import采用`ProviderSealedRuntimeClosure`，不能从排除了linked targets的公开surface猜测完整KindPayload：consumer保存的RuntimeDeclaration全部S字段必须与ActiveModuleGraph绑定的唯一provider artifact中同SymbolKey record逐canonical field相等，再重建公开RuntimeAbiSurfaceProjection并核对Export tuple/AbiDigest。从该root的KindPayload linked-symbol字段递归可达、但未进入公开surface的provider-private/module Symbol、Type、Function、Global或RuntimeDeclaration形成sealed closure；每个node都必须解析到同一provider artifact中的唯一原记录，所有S字段逐项相等，且只能由该runtime root的规定field path到达。它们不要求进入Exports，不得参与consumer name lookup、独立Core operand/callee/relocation/reflection/registration、再次re-export或与另一provider拼接；HasImplementationBody、bridge matching与lowering通过已pin provider version owner解析，不复制provider body或StableEntry mapping。任一sealed node从其他consumer root可达、provider不一致、字段被裁剪/改写或脱离provider pin均拒绝。

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
| 10 | `InitializationPolicy` | `Enum<GlobalInitializationPolicyTag>` | S |
| 11 | `InitializationOrdinal` | `Opt<U>` | S |
| 12 | `InitializationDependencies` | `Vec<Ref<Global>>` | S |
| 13 | `Attributes` | `Opt<Ref<AttributeSet>>` | S |
| 14 | `LayoutDigest` | `D32` | S |
| 15 | `Origin` | `Ref<Origin>` | N |

每个Global.Type都必须closed、`SizedObjectType`且`LayoutComplete`，其Type.LayoutDigest必须present，Global.LayoutDigest必须逐字节等于该值；这适用于definition、declaration、imported与runtime_owned，因为四者都描述可形成准确storage的对象。runtime opaque或其他无layout对象只能作为有布局的raw pointer global保存，不能把opaque本体伪装成Global.Type。`Initializer` 与 `InitializerFunction` 最多一个 present；`Initializer`表示loader安装的准确typed logical image值。v0不存在implicit zero/default object：每个actual-storage definition必须恰有`Initializer`或`InitializerFunction`，源码零初始化也必须先lower成准确typed Constant；这避免用全零bits绕过class/enum有效表示、constructor或resource lifecycle。

actual-storage definition的FinalizerFunction presence由type唯一决定：`NeedsDestroy(Type)=true`时必须present且由compiler生成完整`obj.destroy`/dynamic destroy chain，false时必须absent；v0没有与type destructor并列的任意module-finalizer hook。constant image若NeedsDestroy同样进入对应plan并在AlivePrefix逆序执行finalizer。runtime_owned的destroy协议由RuntimeGlobal管理，不使用该字段。

RecordKind、module ownership、Visibility、Linkage与Storage的允许矩阵固定为：`definition`的Symbol.CanonicalModuleIdentity必须等于Manifest module identity，Storage只能`static|thread_local`，Visibility/Linkage必须逐项为`private/internal`、`module/module`或`export/export`；`imported`的symbol必须属于active dependency module并在其Exports，Visibility固定`export`、Linkage=`import`、Storage=`extern`；`declaration`只表示当前module声明的host/external Ink ABI global，CanonicalModuleIdentity等于Manifest，Visibility为`module|export`、Linkage固定`external`、Storage=`extern`；`runtime_owned`的symbol属于当前module，Visibility/Linkage只能`private/internal`或`module/module`、Storage固定`runtime_managed`，且必须由唯一RuntimeDeclaration(RuntimeGlobal)通过GlobalSymbol显式指向。definition恰有一种initializer，FinalizerFunction可选；其他三种record不得携带Initializer、InitializerFunction或FinalizerFunction。任何visibility/linkage不映射、`definition+import`、dependency declaration的第二种表示、extern definition或runtime_managed definition组合都拒绝。

lifecycle policy不能只从function presence猜测：declaration/imported/runtime_owned一律`none`且ordinal absent、dependencies empty；runtime_owned状态只由匹配RuntimeGlobal协议管理。static definition在InitializerFunction或FinalizerFunction present时使用`eager_module`，否则使用`none`并在publish前完成constant image安装；thread_local definition无论是constant image还是dynamic initializer、无论是否有finalizer，都必须使用`eager_thread_activation`。因此显式zero/image-only TLS也各有plan step：activation在该step materialize version/thread-local storage、安装typed image、把对象标为Alive并追加到`AlivePrefix`，即使没有用户函数也不会缺少Alive建立路径。

所有非`none`条目的ordinal必须present，并在各自 `(module version, policy)` 分区内从零连续且唯一。只有 image initializer 而有 finalizer 的static条目同样进入plan：image安装后把该条目登记进`AlivePrefix`，从而保留销毁次序。dependency vector只引用同一module version、同一policy分区的Global，按canonical identity严格递增、无重复，且每项ordinal更小；跨module顺序只由active module DAG表达。verifier在每个分区对dependency DAG运行dependency-first stable Kahn，每步从ready set选择Global canonical identity最小者；编码ordinal必须逐项等于重算次序。它们共同构成可从artifact完整重建的`ModuleInitializationPlan`。static plan在模块版本发布前显式执行；thread-local plan在每个线程进入或切换到该模块版本时显式、急切执行，`mem.global_place`永远不隐式触发初始化。

每个`InitializerFunction`固定为sync、zero logical parameters、logical/result mode均为`void`、calling convention `ink`，且entry恰有一个由module runtime提供的`global_lifecycle(0) : !place<init,T>` owner binding；它只服务当前Global，不得复用。process与thread-local initializer都必须具有positive `nothrow`，且lifecycle entry没有Ink unwind successor；runtime/embedding失败可把module置为Failed并按AlivePrefix逆序cleanup，但不是Ink exception。每个`FinalizerFunction`固定为sync、zero logical parameters、logical/result mode均为`void`、calling convention `ink`、positive `nothrow`，entry恰有一个`global_lifecycle(0) : !place<ro/rw,T>` owner binding，并且只由匹配Global的runtime lifecycle entry进入。两类lifecycle symbol都必须Visibility=private、EntryIdentity=version_local、DeclarationIdentity=generated lowering_helper且parent=`versioned_entity_owner(Global.Symbol, ModuleVersionContextKey)`；initializer/finalizer的SemanticOutputOrdinal分别固定0/1并由该Global字段反向唯一绑定。context key等于当前Manifest与active compilation inputs重算值；禁止stable/extern、其他Global、其他module version或可独立取址entry。activation/finalization始终持有匹配module-version owner，因此V1 cleanup不会在V2发布后跳到V2 body。若实现或foreign boundary违反nothrow，必须进入`rt.fatal {kind = nothrow_violation}`，不得产生outward unwind、跳过到另一个cleanup successor或继续发布。thread-local runtime/embedding failure按成功ordinal逆序清理并把该线程状态置为`Failed`，随后拒绝进入该版本；状态机按每线程独立的`Uninitialized -> Initializing(AlivePrefix) -> Alive -> Finalizing -> Destroyed`执行，失败时从`Initializing`清理后进入终态`Failed`，重入`Initializing`是`invalid_dynamic_state`，thread exit按成功ordinal逆序销毁。

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
| 10 | `Parameters` | `Vec<(Ref<Type>, Enum<ParameterPassingModeTag>)>` | S |
| 11 | `DefaultArguments` | `Vec<Opt<Ref<Constant>>>` | S，但不进入FunctionAbiDigest或SymbolKey |
| 12 | `LogicalResultType` | `Ref<Type>` | S |
| 13 | `ResultPassingMode` | `Enum<ResultPassingModeTag>` | S |
| 14 | `BehaviorContracts` | `Bits<BehaviorContractBit>` | S |
| 15 | `BehaviorContractDigest` | `D32` | S，重算核对 |
| 16 | `StableEffectEnvelope` | `Opt<StableEffectEnvelopePayload>` | S |
| 17 | `StableEffectEnvelopeHash` | `Opt<D32>` | S，重算核对 |
| 18 | `Attributes` | `Opt<Ref<AttributeSet>>` | S |
| 19 | `AbiDigest` | `D32` | S |
| 20 | `FunctionOrigin` | `Ref<Origin>` | N |
| 21 | `BodyPresence` | `Bool` | S |
| 22 | `BodyRegions` | present 时 `Vec<Region>` | S，内部 origin N |

Function type 中的签名字段必须与此处 3—6、8、10、12—13一致，并重复执行§7的Receiver矩阵；entry identity、default arguments、contract、effect envelope和ABI digest不属于普通 function type。DefaultArguments.size必须等于Parameters.size；present只允许对应parameter mode=`value`且T closed/Copyable，constant type准确等于T并能由caller在调用前无失败地物化，其他mode或generated/runtime/destructor function固定全部absent。caller在进入InkIR call前展开default，call payload的ExplicitArgumentCount仍等于完整参数数；default projection进入Function S与active dependency interface，因此value变化会使dependent interface digest失配，但不改变SymbolKey/FunctionAbiDigest。constructor 必须是 `CallableKind=constructor`、`FunctionKind=sync`、`ReceiverKind=initializing_instance`、logical `void`、result mode `void`。destructor_body 必须是 sync、mutable_instance receiver、`Parameters` empty、logical/result mode均为 void、具有 `nothrow`，且不得作为普通 direct/indirect/virtual/interface/reflection target；它只能由 `obj.destroy`/`obj.destroy_dynamic` 的已验证析构链调用。async function logical result T 必须为 void、never，或 closed runtime-representable、Copyable(T) 且 CopyConstructionEnabled(T)，且不含 NoEscape。`EntryIdentity=stable` 必须 present envelope 与 envelope hash且FunctionRecordKind=declaration；其他 entry 两者必须 absent。`EntryIdentity=version_local`若Symbol identity是version_local_body，必须为definition并匹配parent logical stable Function；其他version_local function按当前module entity规则验证。FunctionRecordKind `declaration` 必须 BodyPresence=false，`definition|generated_adapter` 必须 BodyPresence=true且恰有一个 function_body region；tag 4 `reserved_extern_bridge`在所有artifact kind中拒绝。Ink ABI extern函数使用`RecordKind=declaration, EntryIdentity=extern, BodyPresence=false`；C boundary adapter不使用Function record，AbiImport/AbiBridge/AbiExport的declarative payload由backend直接lower，不再引入Function层的extern_bridge。`CanonicalLineageIdentity`对stable entry是其SymbolKey，对generated version_local_body是其versioned_owner.LogicalStableSymbolKey，其他entry是当前SymbolKey；因此stable declaration与每版body共享contract/ABI lineage但version-local callable identity仍互异。`BehaviorContractDigest = H("ink.behavior-contract.v0", CanonicalLineageIdentity, BehaviorContracts)`；verifier 按第 5.3 节适用矩阵检查 bit，不再构造反相的 current bool。

Function record的calling-convention矩阵是闭集：ordinary function、decorator、constructor、destructor_body以及stable/version_local/continuation_local/Ink extern entry固定`ink`；CallableKind tag 4保留名`runtime_thunk`但revision 1所有Function record无条件拒绝，runtime/ABI行为只由专用opcode或declarative RuntimeDeclaration表达。`c`只允许出现在Type(function)的CFunctionType和AbiImport/AbiBridge/AbiExport declarative payload，任何Function record填`calling_convention=c`一律拒绝。

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
TargetDependent : Bool
PdbBoundary : Bool
```

三个 Vec 均按 tag 严格递增且无重复；任何EffectUpperBound中`PdbBoundary=true`都要求`TargetDependent=true`，反向不强制。`BottomEffectUpperBound`唯一编码为MemoryAccess=`none`，三个Any均false且对应vectors empty，全部七个Bool为false。StableEffectEnvelopePayload required 顺序为 `SyncEffects : EffectUpperBoundPayload, AsyncConstructionEffects : EffectUpperBoundPayload, AsyncBodyEffects : EffectUpperBoundPayload, SyncMayUnwind : Bool, ConstructionMayUnwind : Bool, BodyMayFail : Bool`。sync function要求两个async phase恰为Bottom且ConstructionMayUnwind/BodyMayFail=false；async function要求SyncEffects恰为Bottom且SyncMayUnwind=false。irrelevant phase不能携带任意保守bytes，因此同一语义只有一个canonical envelope。这是 lineage 首次兼容基线后固定并参与 hash 的 replacement 上界，不是当前 body summary；三个 exception bit同样是固定upper-bound bits。`StableEffectEnvelopeHash = H("ink.stable-effect-envelope.v0", CanonicalLineageIdentity, StableEffectEnvelopePayload)`。当前 BehaviorContracts 只能相对已发布 predecessor 单调加强；调用点使用 baseline envelope 与 current contract 的交集，exact body summary 必须同时满足两者。`FunctionAbiDigest = H("ink.function-abi.v0", CanonicalLineageIdentity, CallableKind, FunctionKind, ReceiverKind, ReceiverType, CallingConvention, TargetAbiTag, Parameters, LogicalResultType, ResultPassingMode)`。loader 必须重算 envelope hash、BehaviorContractDigest 和 FunctionAbiDigest，验证 exact body 是 envelope 子集、满足 current contract，并通过 active published-lineage metadata验证 contract 未减弱。BehaviorContractDigest及其值必须进入import dependency、`active_dependency_interface_digest`、incremental/cache key和AOT assumption identity。

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
| DispatchSlot | `Enum<DispatchSlotRoleTag>, OwnerSymbolId, SlotOrdinal : U, FunctionTypeId, Opt<TargetFunctionSymbolId>, Opt<EntryIdentityTag>` |
| ReflectionDescriptor | `SubjectSymbolId, DescriptorKindTag : U, Opt<LookupOwnerSymbolId>, LookupName : Str, Opt<ReflectionMemberKindTag>, Opt<FunctionTypeId>, VersionPolicyTag : U` |
| StableEntry | `LogicalSymbolId, VersionLocalSymbolId, FunctionTypeId, TargetAbiTag : D32` |
| AbiBridge | `CFunctionTypeId, InkFunctionTypeId, BoundarySignatureMapPayload, CallingConventionTag, BridgeDirectionTag : U, Opt<AbiImportSymbolId> ExternalTarget, TargetAbiTag : D32, Opt<EffectUpperBoundPayload> TrustedExternalEffects, TaggedBoundaryPolicyPayload` |
| RuntimeGlobal | `GlobalSymbolId, TypeId, GlobalStorageTag, MutabilityTag` |
| AbiExport | `CSymbol : Str, InkTargetSymbolId, CFunctionTypeId, BoundarySignatureMapPayload, BridgeSymbolId, TargetAbiTag : D32, StableAddress : Bool, ExportExceptionPolicyPayload` |
| AbiImport | `CSymbol : Str, CFunctionTypeId, ExpectedInkFunctionTypeId, BoundarySignatureMapPayload, TargetAbiTag : D32, Opt<EffectUpperBoundPayload> TrustedExternalEffects, ForeignFailurePolicyPayload` |
| InterfaceDispatchTable | `ConcreteClassSymbolId, InterfaceTypeId, Vec<InterfaceMemberBinding>` |

`BoundarySignatureMapPayload`的required顺序为`ParameterMapCount : U, ParameterMappings : Vec<BoundaryParameterMapping>, LogicalResultChannel : Opt<CAbiPositionPayload>`；count必须等于vector长度。`BoundaryParameterMapping` required顺序为`InkParameterOrdinal : U, CParameterOrdinal : U, ConversionTag : Enum<BoundaryConversionTag>`，revision 1唯一conversion tag是1 `identity_c_abi`。InkFunctionType必须是sync、CallableKind=function、ReceiverKind=none、无varargs；每个Ink parameter固定mode=value且类型满足其C parameter位置的`CAbiSafe`。mappings按InkParameterOrdinal从0连续，CParameterOrdinal严格递增且各自位置的C parameter type与Ink logical parameter type逐TypeSemanticIdentity相等；从C parameter vector删除status_out_parameter和result_out_parameter两个可选special channel后，剩余位置必须恰为这些mapped参数，不允许padding/hidden/context/receiver或未编码conversion。

Ink logical result为void时LogicalResultChannel必须absent；非void时必须closed、非never、CAbiSafe且channel必须present。`result`要求C direct result type逐TypeSemanticIdentity等于Ink result；`result_out_parameter`要求所指C parameter类型准确为`pointer<rw, InkLogicalResultType>`。status policy的StatusChannel独立使用`status_result`或`status_out_parameter`，后者C parameter类型准确为`pointer<rw, StatusType>`；direct C result至多承载logical result或status之一，所有mapped/special C ordinals互异。删去mapped parameters与两个optional special channels后，C signature不得剩余任何parameter；若direct result未被一个present channel占用则必须为void。AbiImport与其target-bound InkToExternal AbiBridge的完整BoundarySignatureMap逐字段相等；AbiExport与ExternalToInk bridge也逐字段相等。由此backend只能按该显式双signature map lower，不能按type相似度猜receiver、arity、转换或result落点。

`TypePropertyTag`：1 `Copyable`；2 `NeedsDestroy`；3 `AddressOnly`；4 `StableAddress`；5 `NoEscape`；6 `SealedRuntimeStorage`。这里只允许 runtime schema 声明的性质，且 verifier 仍按 type kind 重算；runtime_object 固定具有 AddressOnly 与 SealedRuntimeStorage，固定不具有 Copyable。`DispatchSlotRoleTag`：1 `virtual_method`；2 `dynamic_destroy`；call.virtual只接受前者，obj.destroy_dynamic只接受后者。virtual_method的TargetFunction/EntryIdentity同时absent只表示匹配Type slot的abstract状态，同时present时逐项匹配最终dispatch target；dynamic_destroy二者必须present。`BridgeDirectionTag`：1 `InkToExternal`；2 `ExternalToInk`。`DescriptorKindTag`：1 `type`；2 `interface`；3 `function`；4 `member`；5 `constructor`；6 `dispatch_adapter`。`VersionPolicyTag`：1 `version_local`；2 `stable_compatible`；3 `pinned_snapshot`。`ReflectionDescriptorIdentityTuple`的required顺序固定为`DescriptorKindTag, Opt<LookupOwnerSymbolKey>, LookupName UTF8 bytes, Opt<ReflectionMemberKindTag>, Opt<FunctionTypeSemanticIdentity>`；VersionPolicyTag明确不在identity tuple中。同一SubjectSymbol下该tuple严格递增且无重复，同一tuple只能有一条descriptor，因此不能用不同VersionPolicy制造第二条候选；runtime_metadata ordinal只按此tuple排序。

ReflectionDescriptor的字段矩阵是required验证。LookupName必须non-empty valid UTF-8 NFC，并等于SubjectSymbol.StructuralPath对应的canonical qualified name（顶层lookup）或其末尾logical name segment（member lookup）；不能用display alias或locale folding。`type`要求SubjectSymbol唯一拥有complete nominal_class/nominal_enum Type、LookupOwner和MemberKind absent、FunctionType absent；`interface`要求SubjectSymbol唯一拥有complete nominal_interface Type且后三者同样absent；顶层`function`要求SubjectSymbol拥有准确Function、LookupOwner/MemberKind absent且FunctionType逐TypeSemanticIdentity等于该Function signature；成员`function`要求LookupOwner是其声明owner的complete nominal type、MemberKind=`function`且FunctionType准确；`member`只允许field/base selector或一个class的effective interface implementation，LookupOwner为准确nominal owner，MemberKind分别为`field|base|interface`且FunctionType absent；`constructor`要求SubjectSymbol拥有CallableKind=constructor的准确Function、LookupOwner等于initializing receiver class、MemberKind=`constructor`且FunctionType准确；`dispatch_adapter`要求SubjectSymbol拥有generated stable_adapter Function、LookupOwner/MemberKind absent且FunctionType准确，它不进入任何name lookup index，只能由已选function/constructor snapshot的验证后adapter relation取得。VersionPolicy必须是该descriptor唯一记录中的属性；`pinned_snapshot`只允许通过reflection lookup/version pin路径观察，不能成为未pin裸descriptor。

reflection name index也必须在Closed中反向验证而不是由runtime容器覆盖解决。`reflect.lookup_type|reflect.lookup_interface`分别以`(CanonicalModuleIdentity, LookupName, DescriptorKind)`唯一查找；`reflect.lookup_function`再加入ExpectedFunctionTypeSemanticIdentity，并且只接受LookupOwner absent的function descriptor。`reflect.lookup_member`以owned snapshot的SubjectSymbol、canonical member LookupName和ReflectionMemberKindTag唯一查找LookupOwner等于该snapshot subject的field/base/interface/function/constructor descriptor；零项走missing，一项走found，多项使artifact无效。由于P24没有expected function type，revision 1明确拒绝同一reflected owner下同名同member-kind的重载或别名，即使FunctionType不同；不得按声明顺序、当前版本或返回类型任选其一。每条可索引descriptor必须恰出现在对应唯一index中，dispatch_adapter不得伪装成额外lookup候选。

StableEntry.KindPayload的LogicalSymbolId必须逐ID等于该RuntimeDeclaration.Symbol并满足LogicalStableCallable；VersionLocalSymbolId必须是GeneratorKind=version_local_body、parent.versioned_owner.LogicalStableSymbol等于LogicalSymbolId且parent.ModuleVersionContextKey等于当前重算值的generated function symbol。前者Function record固定bodyless declaration/EntryIdentity=stable，后者固定definition/EntryIdentity=version_local；两者signature/target/ABI/contract逐项相等，只有stable declaration携带StableEffectEnvelope与public defaults，version body携带实际body。每个semantic module context对每个logical stable symbol恰一StableEntry mapping，VersionLocalSymbol也只被一条mapping引用；不同ModuleVersionContextKey保证旧/new semantic body identity不同，K-only variant复用该mapping identity但由不同LoadedVersionKey的runtime version owner/pin和物理映射分开承载。该完整mapping只出现在logical symbol所属module artifact；consumer import不得复制。

`TaggedBoundaryPolicyPayload`的outer tag闭集为1 `foreign_failure`/`ForeignFailurePolicyPayload`、2 `export_exception`/`ExportExceptionPolicyPayload`；InkToExternal AbiBridge必须使用前者，ExternalToInk必须使用后者。`ForeignFailurePolicyPayload`的tag闭集为1 `nothrow`/empty、2 `status_to_exception`/`StatusType : Ref<Type>, StatusChannel : CAbiPositionPayload, SuccessStatus : Ref<Constant>, ForeignErrorType : Ref<Type>, ExceptionFactorySymbol : InkCallableSymbolRef, ExceptionFactoryFunctionType : Ref<Type(function)>, ProtocolSchemaDigest : D32`。StatusType必须是C ABI可表示的闭合integer/enum，SuccessStatus类型准确匹配；所有其他bit-pattern合法status都视为failure。ForeignErrorType必须是registered `foreign_status_error_v0` exception payload type并无损保存准确StatusType值；factory必须HasImplementationBody且为sync Ink、none receiver、positive nothrow、唯一参数为value(StatusType)、logical result为ForeignErrorType且passing mode与该type匹配的准确Function。`ProtocolSchemaDigest = H("ink.abi-foreign-failure.v0", TargetAbiTag, PolicyPayloadExcludingDigest)`并逐record重算。

`ExportExceptionPolicyPayload` 是tagged closed payload：tag 1 `nothrow`/empty；tag 2 `catch_and_status`/`StatusType : Ref<Type>, StatusChannel : CAbiPositionPayload, SuccessStatus : Ref<Constant>, DefaultFailureStatus : Ref<Constant>, Mappings : Vec<ExceptionStatusMapping>, ProtocolSchemaDigest : D32`；tag 3 `catch_and_callback`/`CallbackSymbol : Ref<Symbol>, CallbackFunctionType : Ref<Type(function)>, ExceptionInfoType : Ref<Type>, Mappings : Vec<ExceptionCallbackMapping>, ProtocolSchemaDigest : D32`。`ExceptionStatusMapping` required顺序为`ExceptionType : Ref<Type>, StatusValue : Ref<Constant>`；`ExceptionCallbackMapping`为`ExceptionType : Ref<Type>, StableTypeCode : U`。两种mapping都按ExceptionTypeSemanticIdentity严格递增且无重复，只做exact dynamic exception type匹配；未列出type分别使用DefaultFailureStatus或reserved type code 0，不执行subtype-first歧义选择。status constants类型都必须等于StatusType，SuccessStatus与所有failure值互异，StableTypeCode非零且唯一。nothrow export要求Ink target具有positive nothrow且不产生boundary catch；其他两种policy必须在wrapper内catch-all，绝不让Ink exception越过C边界。

CatchAndCallback protocol固定为一次同步、nothrow C ABI调用：CallbackFunctionType必须恰为`(pointer<ro,ExceptionInfoType>) -> void`且无varargs，CallbackSymbol必须解析到唯一RuntimeDeclaration(AbiImport)，其CFunctionTypeId逐项等于CallbackFunctionType且ForeignFailurePolicy=nothrow；普通Ink stable_entry或`EntryIdentity=extern` Function即使逻辑签名相似也不是C callback，必须先经另一个已验证InkToExternal bridge调用该AbiImport，不能直接填入CallbackSymbol。ExceptionInfoType必须是TargetABI registry的`abi_exception_info_v0` C-repr frozen struct，唯一字段顺序为`StableTypeCode : u64, MessageData : pointer<ro,u8>, MessageLength : ptrsize`。wrapper在catch-all内构造call-duration只读view，message bytes为canonical UTF-8且只在callback返回前有效，callback不得保存该pointer；callback返回后wrapper返回void且不再rethrow。catch_and_callback只允许Ink target与C outer signature的logical result都为void、BoundarySignatureMap.LogicalResultChannel absent且没有result-out parameter；否则policy没有合法failure result，artifact拒绝。StatusType、ExceptionInfoType及callback的每个C-facing位置都满足`CAbiSafe(T, TargetAbiTag, TargetKey, CAbiPositionPayload)`。`ProtocolSchemaDigest = H("ink.abi-export-exception.v0", PolicyTag, TargetAbiTag, PolicyPayloadExcludingDigest)`并逐record重算；因此status mapping、callback identity/type/info layout或target ABI变化都会改变Runtime AbiDigest，不能只保存一个枚举名。

`CAbiPositionPayload` required顺序为`Root : Enum<CAbiPositionTag>, RootOrdinal : U, AggregateFieldPath : Vec<U>`；tag闭集为1 `parameter`、2 `result`、3 `status_result`、4 `status_out_parameter`、5 `callback_parameter`、6 `result_out_parameter`。result/status_result要求RootOrdinal=0，其他tag编码准确zero-based parameter ordinal；递归aggregate验证按source field ordinal追加AggregateFieldPath。catch_and_status与status_to_exception都要求CFunctionType中由payload.StatusChannel指定的唯一status channel恰好承载StatusType：直接返回时使用status_result且C result类型精确等于StatusType；out-parameter时使用status_out_parameter且对应C parameter类型精确等于`pointer<rw,StatusType>`。BoundarySignatureMap的logical result同理：result使用准确T的C result，result_out_parameter使用准确`pointer<rw,T>` parameter。status与logical result channel不能同时占用C result、同一parameter或mapped user parameter，也不能遗漏；所有未被map占用的位置拒绝。CatchAndCallback的callback所在registered symbol位置使用callback_parameter并与CallbackFunctionType准确相等。所有C-facing root与递归field必须满足`CAbiSafe(T, TargetAbiTag, TargetKey, CAbiPositionPayload)`；out-parameter root先验证pointer本身，再以相同root/path规则验证其准确pointee。

AbiBridge的CallingConventionTag固定为`c`；`InkToExternal` bridge与AbiImport供`abi.call|abi.invoke`使用，`ExternalToInk` bridge只供AbiExport wrapper使用。每个AbiBridge/AbiImport同时闭合准确CFunctionType与Expected Ink FunctionType pair。InkToExternal bridge的ExternalTarget present时必须解析到唯一AbiImport并逐项匹配C/Ink type、TargetAbiTag、ForeignFailurePolicy与TrustedExternalEffects，供direct call使用；ExternalTarget absent时只供indirect pointer调用且TrustedExternalEffects必须absent。ExternalToInk bridge的ExternalTarget与TrustedExternalEffects必须absent，policy必须等于AbiExport的ExportExceptionPolicy。AbiImport是C linker symbol的唯一IR identity，不创建伪装成`EntryIdentity=extern`的Ink Function；`call.*`永远不直接以AbiImport为callee。AbiExport的InkTarget/Bridge pair、CFunctionType、TargetAbiTag与policy逐项核对，bridge direction固定ExternalToInk；C outer signature与Ink logical signature分别只存在于declarative Runtime payload，backend合成wrapper而不产生可被Core调用、取址或反射的额外Function entity。

ForeignFailurePolicy=nothrow时`abi.call`和`abi.invoke`都可用，但callee违反约定只能`rt.fatal {kind = nothrow_violation}`；status_to_exception只允许`abi.invoke`。后者在C调用返回后先读取唯一status channel：success时才commit/publish logical result或destination并进入normal edge；failure时不得发布或留下partially initialized result，调用注册factory产生准确ForeignError payload、建立ExceptionBox owner并沿unwind edge传递。factory或exception-box建立过程本身不得形成第二个outward exception；资源失败按registered fatal policy处理。Export catch_and_status同理只在Ink target正常完成后写SuccessStatus并发布logical result；捕获异常时先撤销未发布result，再写映射failure status，status channel与logical result-out channel互不重叠。

AbiExport.InkTarget必须满足HasImplementationBody，FunctionSignature固定为sync、CallableKind=function、ReceiverKind=none且所有parameter为value；async、constructor、destructor_body、decorator、receiver method和runtime thunk都拒绝，因为C wrapper同步返回后不能捕获其延迟failure或保留receiver ownership。StableAddress=true时InkTarget必须为LogicalStableCallable；false时可以是当前module private/module ordinary definition，但其地址不得进入公开stable relocation。ExternalToInk bridge的InkFunctionType、BoundarySignatureMap和policy必须逐项等于AbiExport所指target与payload。

每条RuntimeDeclaration.RuntimeAbiRevision必须逐值等于当前Manifest.runtime_abi_revision；registry_builtin还必须等于registry entry revision，dependency import还必须等于provider原record与provider Manifest。record内自洽AbiDigest不能放行旧/未来revision。OpaqueType.KindPayload的Type必须是唯一`runtime_opaque` Type，且该Type.KindPayload的Symbol逐ID等于本RuntimeDeclaration.Symbol；反向每个runtime_opaque Type恰被一条OpaqueType引用，同一Symbol/Type不得二次绑定。EffectHandler与OpaqueType只能使用匹配runtime registry的registry_builtin identity；DispatchSlot、InterfaceDispatchTable、ReflectionDescriptor与RuntimeGlobal的generated symbol必须使用上表runtime_metadata identity并反向匹配owner，StableEntry、AbiBridge、AbiImport和AbiExport则分别使用各自已规定identity形态。

`ForeignWireContractProjection`的tag闭集为1 `nothrow`/empty、2 `status`/`StatusTypeSemanticIdentity, StatusChannel : CAbiPositionPayload, SuccessStatusSemanticProjection`。AbiImport.nothrow投影tag 1；status_to_exception投影tag 2并排除consumer-local ForeignErrorType、ExceptionFactorySymbol/FunctionType与protocol digest。AbiExport.nothrow投影tag 1；catch_and_status投影tag 2；catch_and_callback对被export函数本身投影tag 1，但要求双方BoundarySignatureMap logical result absent、C result void且import policy=nothrow。cross-policy兼容矩阵只有`export nothrow <-> import nothrow`、`export catch_and_status <-> import status_to_exception且tag-2三字段逐canonical field相等`、`export catch_and_callback <-> import nothrow/void-map`三行，其他组合拒绝；callback的独立AbiImport仍按自己的CLinkIdentity验证。

`CSymbol`必须是nonempty valid UTF-8 NFC、不得含NUL，按原始UTF-8 bytes大小写敏感；target object-format的前导下划线或name decoration只由TargetABI lowering处理，不改写IR identity。`CLinkIdentity = H("ink.c-link-symbol.v0", TargetAbiTag, CSymbol UTF8 bytes)`。同一active module DAG中同一CLinkIdentity至多有一个AbiExport provider；多个AbiImport允许共享identity，但其CFunctionType、BoundarySignatureMap、TargetAbiTag、ForeignWireContractProjection和TrustedExternalEffects必须逐canonical field相等，允许各consumer使用不同但已验证的local error type/factory。每个AbiImport必须唯一解析到该active AbiExport或trusted host-link registry entry并核对C signature/map与上述cross-policy矩阵；host registry也必须提供同schema的ForeignWireContractProjection。两个export、export与host definition并存、同名wire contract不相容imports或未解析identity都在link verification时拒绝。

RuntimeGlobal.GlobalSymbol必须是唯一`GlobalRecordKind=runtime_owned`的global symbol，且Type/Storage/Mutability逐项相等；同一runtime-owned Global恰被一条RuntimeGlobal引用。每条 RuntimeDeclaration 的 `AbiDigest = H("ink.runtime-abi.v0", RuntimeRecordKind, SymbolKey, RuntimeAbiRevision, KindPayload, Attributes)`；digest 字段自身不进入 preimage。`TrustedExternalEffects` absent 时使用第 14.6 节最大保守上界；present 时必须来自编译器/平台注册表而非用户 attribute，并进入 Runtime ABI digest、dependency identity 和 verifier assumption。

在Staged artifact建立前运行的ImportSelectionProfile不增加StageOpcode或Core opcode；其闭集、顺序、budget与effect规则唯一由05 §10.1定义。registry只接受由该profile冻结的ActiveModuleGraph和完整Dependency records；任何serialized plan/template声称重新执行guard、添加candidate/import edge或省略guard tracked read都拒绝。

## 10. Dependency、Template 与 ElaborationPlan record

### 10.1 Dependency

五种 Dependency RecordKind 共享外壳，但`KindIdentityPayload`由RecordKind唯一选择：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `KindIdentityPayload` | 下表对应typed payload | S |
| 2 | `ObservedDigest` | `D32` | S |
| 3 | `HandlerRevision` | `U` | S |
| 4 | `ValidationMode` | `Enum<DependencyValidationTag>` | S |
| 5 | `DisplayIdentity` | `Opt<Str>` | N |

| RecordKind | KindIdentityPayload，按顺序 | 唯一ValidationMode | ObservedDigest ordered preimage |
| --- | --- | --- | --- |
| `FileRead` | `NormalizedLogicalPath : Str, Range : TaggedFileReadRange, SymlinkPolicy : Enum<SymlinkPolicyTag>` | `ExistenceAndContent` | `H("ink.dependency.file.v0", HandlerRevision, KindIdentityPayload, ExistenceTag, FileEntryKindTag, ExactSelectedBytes)` |
| `EnvironmentRead` | `EnvironmentNameProfile : Enum<EnvironmentNameProfileTag>, NormalizedVariableName : Str` | `ExactValue` | `H("ink.dependency.environment.v0", HandlerRevision, KindIdentityPayload, PresenceTag, ExactValueBytes)` |
| `DirectoryRead` | `NormalizedLogicalPath : Str, Recursive : Bool, EntryProjection : Enum<DirectoryProjectionTag>, SymlinkPolicy : Enum<SymlinkPolicyTag>` | `OrderedListing` | `H("ink.dependency.directory.v0", HandlerRevision, KindIdentityPayload, ExistenceTag, CanonicallyOrderedDirectoryEntries)` |
| `ConfigRead` | `NormalizedNamespace : Str, NormalizedKey : Str, ExpectedValueKind : Enum<ConfigValueKindTag>` | `ExactValue` | `H("ink.dependency.config.v0", HandlerRevision, KindIdentityPayload, PresenceTag, ActualValueKindTagOrZero, CanonicalTypedValueBytes)` |
| `ToolResourceRead` | `ToolIdentity : Str, ToolVersion : Str, ResourceKey : Str, Selector : Bytes` | `Content` | `H("ink.dependency.tool-resource.v0", HandlerRevision, KindIdentityPayload, PresenceTag, ExactResourceBytes)` |

`PresenceTag`与`ExistenceTag`使用同一个闭合无符号数值编码：0 `missing`、1 `present`，其他值拒绝；`ActualValueKindTagOrZero`只在`PresenceTag=missing`时为0且typed value bytes固定empty，在present时必须等于非零`ConfigValueKindTag`并编码准确canonical typed value。环境变量、tool resource或config的present-empty与missing因此具有不同preimage；present空目录也以ExistenceTag=1、empty listing与missing目录区分。`FileEntryKindTag`闭集为0 `none`、1 `regular_file`、2 `directory`、3 `symlink_or_reparse_point`、4 `other`；FileRead missing时固定none，present时必须最终解析为regular_file，DirectoryRead entry不得使用none。`TaggedFileReadRangeTag`闭集为1 `whole`/empty、2 `byte_range`/`Offset : U, Length : U`，Length必须大于0且overflow/range越界拒绝；`SymlinkPolicyTag`闭集为1 `reject`、2 `resolve_within_capability_root`，第二种identity中的file path必须是snapshot已解析的规范逻辑路径。DirectoryRead v0要求SymlinkPolicy=`reject`：root path的每个已遍历component都不得是symlink/junction/reparse point；listing可以把这些对象按准确FileEntryKindTag与相对名列出，但Recursive不得跟随，projection 2也不得读取其target content。由此递归访问不越出capability root、不需要host-specific cycle detection，其他Directory symlink policy组合拒绝。`DirectoryProjectionTag`闭集为1 `names_and_kinds`、2 `names_kinds_and_content_digests`；directory entry required顺序为`NormalizedRelativeName : Str, FileEntryKindTag : U, Opt<ContentDigest>`，按normalized name UTF-8 bytes严格递增且无重复。projection 1要求digest absent；projection 2要求regular-file digest present并且唯一等于`H("ink.dependency.directory-entry-content.v0", ExactRegularFileBytes)`，包括empty file的真实domain digest，其他kind必须absent。entry digest不含relative/physical path、mtime、permission、handler对象、symlink target或host file identity；这些也不能替代ExactRegularFileBytes。`CanonicallyOrderedDirectoryEntries`编码`EntryCount : U`后逐项`EntryLength : U + 完整entry bytes`。`ConfigValueKindTag`闭集复用SemanticOptionValueKindTag的八种typed value kind。Presence/Existence与entry kind不单独信任artifact字段，而由当前冻结BuildInputSnapshot重新观察后进入上表digest；missing使用固定empty value/listing bytes，因domain与presence tag不同不会和present-empty碰撞。

`EnvironmentNameProfileTag`闭集为1 `case_sensitive_utf8_nfc`、2 `windows_ascii_case_insensitive`。前者要求nonempty valid UTF-8 NFC且不含NUL或`=`，snapshot按UTF-8 bytes精确查找；后者只允许ASCII identifier `[A-Za-z_][A-Za-z0-9_]*`，artifact统一编码ASCII大写，snapshot按Windows ordinal case-insensitive语义查找并拒绝同时暴露两个case alias。handler registry从冻结host profile唯一选择tag，producer不能自由改写；profile/name共同进入dependency identity，故跨宿主case规则不会复用错误cache。

ToolResourceRead.Selector虽然以Bytes承载，但不是opaque identity。Selector bytes的唯一envelope为`SelectorSchemaTag : U, SelectorSchemaRevision : U, PayloadLength : U, TypedPayload : Bytes[PayloadLength]`；`(ToolIdentity, ToolVersion, HandlerRevision, SelectorSchemaTag, SelectorSchemaRevision)`必须唯一选择受信typed-field decoder。decoder exact-decode TypedPayload、做canonical re-encode并要求逐byte相等且恰好消费，semantic projection编码schema tag/revision与解码后的tagged fields而非raw bytes/PayloadLength。unknown schema、handler不匹配、别名encoding、尾随bytes或超decoder budget都拒绝。

`DependencyValidationTag`：1 `Content`；2 `ExistenceAndContent`；3 `OrderedListing`；4 `ExactValue`。RecordKind与ValidationMode必须恰等于上表，不能由producer自由选择；HandlerRevision必须匹配对应受信handler registry revision。candidate loader以KindIdentityPayload在当前snapshot执行相同规范化访问，按同一domain重算ObservedDigest再比较；无法稳定重放、identity越出capability scope或kind/value不匹配都使candidate失效。network/process/clock/random 在 v0 不具有 Dependency RecordKind、UntrackedObservationDigest或build-instance nonce；任一实际执行的untracked read都使`closeAndVerify`拒绝生成、序列化、装载或hot-reload publish Closed artifact，不能只标noncacheable后仍复用同一ModuleVersionContextKey。

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
| `CaptureSourceTag` | 1 | `constant` |
|  | 2 | `plan_result` |
|  | 3 | `core_value` |
|  | 4 | `type` |
|  | 5 | `declaration` |
|  | 6 | `lexical_binding` |
|  | 7 | `target_config` |
|  | 8 | `instance_argument` |
| `CaptureAccessTag` | 1 | `read` |
|  | 2 | `read_write` |
|  | 3 | `initialize` |

`TemplateRolePath` 是 non-empty `SourceStructuralPath`；每个 step 的 required 字段按顺序为 `OwnerSyntaxKindTag : U, ChildRoleTag : U, ChildOrdinal : U`。三个数值来自SourceDeclarationKey声明的`SourceStructuralSchemaVersion` registry，且Staged中该version必须与`NormalizedHirSchemaVersion`相等。该 path 从 source declaration root 走到模板位置，不能用 source offset、分配顺序或进程内 node identity 代替。

`VisibleBindingProjection` required 顺序固定为 `DeclaringSourceDeclarationKey : Bytes, BindingDefinitionPath : SourceStructuralPath, BindingKind : Enum<BindingKindTag>, Name : Opt<Str>, Type : Ref<Type>, CaptureAccess : Enum<CaptureAccessTag>`。vector 按DeclaringSourceDeclarationKey的canonical bytes、BindingDefinitionPath、BindingKind与Name UTF-8 bytes严格递增且无重复。definition path从其自身declaring declaration root定位parameter/local/pattern/catch/loop/generic binder，因此outer lexical binding不需要伪装成当前template子HIR内节点；Staged verifier以BuildInputSnapshot中该source key解析出的source declaration tree和当前TemplateRolePath做完整lexical-scope重放，Type/Access必须相等。若source input、declaration或binder path不能唯一解析则拒绝；不能只凭当前Template.NormalizedHirPayload或一个D32接受。`LexicalEnvironmentKey`从该完整vector重算。

`Template` required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `TemplateKind` | `Enum<TemplateKindTag>` | S |
| 2 | `RegionKind` | `Enum<RegionKindTag>` | S |
| 3 | `SourceDeclarationKey` | `Bytes` | S |
| 4 | `TemplateRolePath` | `SourceStructuralPath` | S |
| 5 | `SourceFile` | `Ref<SourceFile>` | S，通过 content digest |
| 6 | `SourceRange` | `Start : U, End : U` | N |
| 7 | `NormalizedHirSchemaVersion` | `U` | S |
| 8 | `NormalizedHirPayload` | `Bytes` | S，经`normalized_hir_v1` custom projection |
| 9 | `VisibleBindings` | `Vec<VisibleBindingProjection>` | S |
| 10 | `LexicalEnvironmentKey` | `D32` | N，重算核对 |
| 11 | `Captures` | `Vec<Capture>` | S，capture origin N |
| 12 | `Origin` | `Ref<Origin>` | N |
| 13 | `SemanticDigest` | `D32` | N，重算核对 |

Template.SourceDeclarationKey内嵌的`(CanonicalModuleIdentity, SourceRoleTag, LogicalPath)`必须与Template.SourceFile所引用record逐项完全相等，并且该stable tuple在当前module version的SourceFiles中唯一；Template的revision identity另取该SourceFile.ContentDigest。SourceRange以这条准确record的source byte length验证`Start <= End <= ByteLength`；不得把相同content digest的另一个role/path文件用于range或origin。TemplateRolePath必须从该SourceDeclarationKey定位的stable declaration在当前SourceFile revision中的root按§10.2 transition registry走到当前template位置。

Capture required 顺序固定为 `CaptureKind : Enum<CaptureKindTag>, Source : TaggedCaptureSource, ExpectedType : Ref<Type>, CaptureAccess : Enum<CaptureAccessTag>, Origin : Ref<Origin>`。`TaggedCaptureSource` 的 tag/payload 为：

| CaptureSourceTag | payload，按顺序 |
| --- | --- |
| `constant` | `ConstantId` |
| `plan_result` | `PlanNodeId, ResultIndex : U` |
| `core_value` | `OwnerSymbolId, RegionPath, ValueId` |
| `type` | `TypeId` |
| `declaration` | `SymbolId` |
| `lexical_binding` | `VisibleBindingIndex : U` |
| `target_config` | `TargetQueryTag : U, TargetQueryPayload` |
| `instance_argument` | `InstanceIdentity : InstanceIdentityPayload, ArgumentIndex : U` |

`TargetQueryTag`闭集为1 `target_key`，payload empty；2 `sizeof`，payload `TypeId`；3 `alignof`，payload `TypeId`；4 `layout_digest`，payload `TypeId`；5 `semantic_option`，payload `OptionKey : Str`；6 `target_feature`，payload `FeatureName : Str`。query必须由TargetContext/SemanticOptions registry准确解析；不得编码查询结果、host state或自由bytes。

CaptureKind/source兼容矩阵固定为：`comptime_value`只接受constant/plan_result/instance_argument；`runtime_value|place`只接受core_value且其准确Core type/value category匹配；`type`只接受type；`declaration_handle`只接受declaration；`lexical_declaration`只接受lexical_binding；`target_config`只接受target_config。`VisibleBindingIndex`必须落在当前Template.VisibleBindings；plan_result必须满足producer result index/ExpectedType检查；CoreValue必须满足owner/RegionPath/ValueId、dominance、stage和lifetime规则；所有table引用在semantic projection中替换为canonical identity。capture按NormalizedHir中首次capture_reference的规范遍历顺序编码，禁止重复同一 `(CaptureKind, Source, ExpectedType, CaptureAccess)`。

`NormalizedHirSchemaVersion = 1` 时，`NormalizedHirPayload` 必须按下列闭合子 schema 准确解码；它不是 opaque AST blob：

```text
NormalizedHirPayloadV1 = {
    NodeCount : U,
    RootNodeId : HirNodeId,
    Nodes[NodeCount] : NormalizedHirNode,
}
NormalizedHirNode = {
    NodeByteLength : U,
    NodeTag : NormalizedHirNodeTag,
    Fields : exact fields selected by NodeTag,
    OriginId : U,
}
HirNodeId = U
OptionalHirNodeId = 0 | HirNodeId + 1
OptionalTemplateId = 0 | TemplateId + 1
OptionalTypeId = 0 | TypeId + 1
HirNodeVector = Count : U, Items[Count] : HirNodeId
TemplateIdVector = Count : U, Items[Count] : TemplateId
ControlTargetPath = Count : U, StructuralOrdinals[Count] : U
```

| Tag | `NormalizedHirNodeTag` | `Fields`，按编码顺序 |
| ---: | --- | --- |
| 1 | `identifier` | `IdentifierStringId : U, IdentifierRoleTag : U` |
| 2 | `literal` | `LiteralKindTag : U, ConstantId : U` |
| 3 | `resolved_reference` | `ReferenceKindTag : U, SymbolId : U` |
| 4 | `dependent_reference` | `LookupKindTag : U, Qualifiers : HirNodeVector, Identifier : HirNodeId` |
| 5 | `capture_reference` | `CaptureIndex : U` |
| 6 | `type_expression` | `TypeExpressionKindTag : U, ResolvedType : OptionalTypeId, Operands : HirNodeVector` |
| 7 | `unary_expression` | `UnaryOperatorTag : U, Operand : HirNodeId` |
| 8 | `binary_expression` | `BinaryOperatorTag : U, Left : HirNodeId, Right : HirNodeId` |
| 9 | `call_expression` | `Callee : HirNodeId, Arguments : HirNodeVector` |
| 10 | `member_expression` | `MemberAccessTag : U, Base : HirNodeId, Member : HirNodeId` |
| 11 | `index_expression` | `Base : HirNodeId, Indices : HirNodeVector` |
| 12 | `aggregate_expression` | `AggregateKindTag : U, Elements : HirNodeVector` |
| 13 | `generic_application` | `Generic : HirNodeId, Arguments : HirNodeVector` |
| 14 | `attribute_application` | `Attribute : HirNodeId, Arguments : HirNodeVector, Target : HirNodeId` |
| 15 | `pattern` | `PatternKindTag : U, Binder : OptionalHirNodeId, TypeOrValue : OptionalHirNodeId, Children : HirNodeVector` |
| 16 | `expression_statement` | `Expression : HirNodeId` |
| 17 | `binding_declaration` | `BindingKindTag : U, Name : HirNodeId, TypeExpression : OptionalHirNodeId, Initializer : OptionalHirNodeId, DeclarationStructuralPath : SourceStructuralPath` |
| 18 | `assignment_statement` | `AssignmentOperatorTag : U, Destination : HirNodeId, Source : HirNodeId` |
| 19 | `block` | `RegionKindTag : U, Items : HirNodeVector` |
| 20 | `if_control` | `Condition : HirNodeId, ThenTemplateId : U, ElseTemplateId : OptionalTemplateId, ControlNodeKey : D32` |
| 21 | `match_control` | `Selector : HirNodeId, Arms : MatchArmVector, ControlNodeKey : D32` |
| 22 | `for_control` | `BindingPattern : HirNodeId, Iterable : HirNodeId, BodyTemplateId : U, ControlNodeKey : D32` |
| 23 | `while_control` | `Condition : HirNodeId, BodyTemplateId : U, ControlNodeKey : D32` |
| 24 | `return_statement` | `Value : OptionalHirNodeId, Target : ControlTargetPath` |
| 25 | `break_statement` | `Value : OptionalHirNodeId, Target : ControlTargetPath` |
| 26 | `continue_statement` | `Target : ControlTargetPath` |
| 27 | `throw_statement` | `Value : HirNodeId` |
| 28 | `try_control` | `BodyTemplateId : U, Catches : CatchClauseVector, FinallyTemplateId : OptionalTemplateId` |
| 29 | `defer_statement` | `BodyTemplateId : U` |
| 30 | `declaration_template` | `DeclarationKindTag : U, DeclarationStructuralPath : SourceStructuralPath, TemplateId : U` |
| 31 | `await_expression` | `Task : HirNodeId` |

`await_expression`只允许出现在async function/generic async body的Normalized HIR，保留source-level等待点直到实例化与类型化完成；Task child必须解析为准确`Task<T>`。后续Core lowering按T的Copyable/address-only性质唯一选择`async.await`或`async.await_copy`及destination，Normalized HIR不提前编码第二个result-mode flag。sync template中的await、用unary operator别名编码await或在进入Core后残留该node均拒绝。

`SourceStructuralSchemaVersion = 1` 与 `NormalizedHirSchemaVersion = 1` 共享下列语言语法enum闭集；这些domain与`NormalizedHirNodeTag`、`OwnerSyntaxKindTag`、`ChildRoleTag`都进入`EnumRegistryProjection`、`canonical_graph`及`normalized_hir_v1` verifier input schema，数值0保留为invalid。Closed artifact虽没有NormalizedHirPayload，仍按SourceStructuralSchemaVersion使用Owner/Role/Declaration/Binding等source-identity domain：

| Enum | Tag | Canonical name |
| --- | ---: | --- |
| `OwnerSyntaxKindTag` | 1 | `declaration` |
|  | 2 | `function_body` |
|  | 3 | `generic_declaration_body` |
|  | 4 | `attribute_application` |
|  | 5 | `if_control` |
|  | 6 | `match_control` |
|  | 7 | `match_arm` |
|  | 8 | `for_control` |
|  | 9 | `while_control` |
|  | 10 | `try_control` |
|  | 11 | `catch_clause` |
|  | 12 | `defer_statement` |
|  | 13 | `class_body` |
|  | 14 | `interface_body` |
|  | 15 | `enum_body` |
|  | 16 | `source_file` |
|  | 17 | `block` |
|  | 18 | `binding_declaration` |
|  | 19 | `pattern` |
|  | 20 | `value_expression` |
| `ChildRoleTag` | 1 | `value` |
|  | 2 | `body` |
|  | 3 | `then_template` |
|  | 4 | `else_template` |
|  | 5 | `arm_body` |
|  | 6 | `loop_body` |
|  | 7 | `try_body` |
|  | 8 | `catch_body` |
|  | 9 | `finally_body` |
|  | 10 | `defer_body` |
|  | 11 | `declaration_body` |
|  | 12 | `member_item` |
|  | 13 | `attribute_argument` |
|  | 14 | `generic_argument` |
|  | 15 | `top_level_item` |
|  | 16 | `parameter` |
|  | 17 | `block_item` |
|  | 18 | `pattern_child` |
|  | 19 | `pattern_binder` |
|  | 20 | `catch_parameter` |
|  | 21 | `loop_binding` |
|  | 22 | `generic_parameter` |
|  | 23 | `arm_pattern` |
|  | 24 | `arm_guard` |
| `IdentifierRoleTag` | 1 | `reference` |
|  | 2 | `binding` |
|  | 3 | `member` |
|  | 4 | `type` |
|  | 5 | `attribute` |
|  | 6 | `label` |
|  | 7 | `declaration_name` |
| `LiteralKindTag` | 1 | `bool` |
|  | 2 | `integer` |
|  | 3 | `float` |
|  | 4 | `string` |
|  | 5 | `character` |
|  | 6 | `null` |
|  | 7 | `unit` |
| `ReferenceKindTag` | 1 | `local` |
|  | 2 | `parameter` |
|  | 3 | `global` |
|  | 4 | `function` |
|  | 5 | `type` |
|  | 6 | `generic_parameter` |
|  | 7 | `member` |
|  | 8 | `module` |
|  | 9 | `interface` |
|  | 10 | `enum_variant` |
|  | 11 | `constructor` |
| `LookupKindTag` | 1 | `unqualified_value` |
|  | 2 | `qualified_value` |
|  | 3 | `unqualified_type` |
|  | 4 | `qualified_type` |
|  | 5 | `member` |
|  | 6 | `callable` |
|  | 7 | `module` |
| `TypeExpressionKindTag` | 1 | `named` |
|  | 2 | `dependent_name` |
|  | 3 | `pointer` |
|  | 4 | `reference` |
|  | 5 | `slice` |
|  | 6 | `array` |
|  | 7 | `tuple` |
|  | 8 | `function` |
|  | 9 | `generic_application` |
|  | 10 | `typeof` |
|  | 11 | `inferred` |
| `UnaryOperatorTag` | 1 | `positive` |
|  | 2 | `negative` |
|  | 3 | `logical_not` |
|  | 4 | `bitwise_not` |
|  | 5 | `dereference` |
|  | 6 | `address_of` |
| `BinaryOperatorTag` | 1 | `add` |
|  | 2 | `subtract` |
|  | 3 | `multiply` |
|  | 4 | `divide` |
|  | 5 | `remainder` |
|  | 6 | `shift_left` |
|  | 7 | `shift_right` |
|  | 8 | `bitwise_and` |
|  | 9 | `bitwise_or` |
|  | 10 | `bitwise_xor` |
|  | 11 | `equal` |
|  | 12 | `not_equal` |
|  | 13 | `less` |
|  | 14 | `less_equal` |
|  | 15 | `greater` |
|  | 16 | `greater_equal` |
|  | 17 | `logical_and` |
|  | 18 | `logical_or` |
|  | 19 | `range` |
| `MemberAccessTag` | 1 | `dot` |
|  | 2 | `arrow` |
|  | 3 | `optional_chain` |
|  | 4 | `qualified` |
| `AggregateKindTag` | 1 | `tuple` |
|  | 2 | `array` |
|  | 3 | `struct` |
|  | 4 | `class` |
|  | 5 | `enum_variant` |
| `PatternKindTag` | 1 | `wildcard` |
|  | 2 | `binding` |
|  | 3 | `literal` |
|  | 4 | `tuple` |
|  | 5 | `aggregate` |
|  | 6 | `enum_variant` |
|  | 7 | `type` |
|  | 8 | `range` |
|  | 9 | `or` |
| `BindingKindTag` | 1 | `immutable` |
|  | 2 | `mutable` |
|  | 3 | `constant` |
|  | 4 | `parameter` |
|  | 5 | `catch` |
|  | 6 | `loop` |
| `AssignmentOperatorTag` | 1 | `assign` |
|  | 2 | `add_assign` |
|  | 3 | `subtract_assign` |
|  | 4 | `multiply_assign` |
|  | 5 | `divide_assign` |
|  | 6 | `remainder_assign` |
|  | 7 | `shift_left_assign` |
|  | 8 | `shift_right_assign` |
|  | 9 | `bitwise_and_assign` |
|  | 10 | `bitwise_or_assign` |
|  | 11 | `bitwise_xor_assign` |
| `DeclarationKindTag` | 1 | `function` |
|  | 2 | `class` |
|  | 3 | `interface` |
|  | 4 | `enum` |
|  | 5 | `global` |
|  | 6 | `type_alias` |
|  | 7 | `import` |
|  | 8 | `module` |
|  | 9 | `constructor` |
|  | 10 | `destructor` |
|  | 11 | `method` |
|  | 12 | `field` |
|  | 13 | `local` |
|  | 14 | `namespace` |

`SourceStructuralSchemaVersion=1` 的合法transition registry如下；表中未出现的`(OwnerSyntaxKindTag, ChildRoleTag)`组合一律非法。`Allowed child owner`是step落点的source syntax category；若列出多个，实际DeclarationKind/grammar alternative唯一选择其中一个。`ChildOrdinal`始终在同一parent、同一role的source-backed children中从零连续计数，绝不跨role混排：

| Owner | Role | Allowed child owner | Ordinal来源 |
| --- | --- | --- | --- |
| `source_file` | `top_level_item` | `declaration` | 文件lexical top-level declaration顺序 |
| `declaration` | `declaration_body` | `function_body`、`generic_declaration_body`、`class_body`、`interface_body`、`enum_body`、`block` | DeclarationKind唯一决定，ordinal固定0 |
| `declaration` | `parameter` | `binding_declaration` | parameter source order |
| `declaration` | `generic_parameter` | `binding_declaration` | generic parameter source order |
| `declaration` | `value` | `value_expression` | initializer/default/value role内source order |
| `function_body`、`generic_declaration_body`、`block` | `block_item` | `declaration`、`binding_declaration`、`if_control`、`match_control`、`for_control`、`while_control`、`try_control`、`defer_statement`、`attribute_application` | block lexical item顺序 |
| `binding_declaration` | `value` | `value_expression` | initializer present时ordinal固定0 |
| `binding_declaration` | `pattern_child` | `pattern` | declared pattern present时ordinal固定0 |
| `pattern` | `pattern_child` | `pattern` | pattern child source order |
| `pattern` | `pattern_binder` | `binding_declaration` | binder leaf source order |
| `if_control` | `then_template` | `block`、`value_expression` | ordinal固定0 |
| `if_control` | `else_template` | `block`、`value_expression` | present时ordinal固定0 |
| `match_control` | `arm_body` | `match_arm` | arm source order |
| `match_arm` | `arm_pattern` | `pattern` | ordinal固定0 |
| `match_arm` | `arm_guard` | `value_expression` | present时ordinal固定0 |
| `match_arm` | `body` | `block`、`value_expression` | ordinal固定0 |
| `for_control` | `loop_binding` | `pattern` | ordinal固定0 |
| `for_control`、`while_control` | `loop_body` | `block`、`value_expression` | ordinal固定0 |
| `try_control` | `try_body` | `block` | ordinal固定0 |
| `try_control` | `catch_body` | `catch_clause` | catch source order |
| `try_control` | `finally_body` | `block` | present时ordinal固定0 |
| `catch_clause` | `catch_parameter` | `pattern` | present时ordinal固定0 |
| `catch_clause` | `body` | `block` | ordinal固定0 |
| `defer_statement` | `defer_body` | `block` | ordinal固定0 |
| `class_body`、`interface_body`、`enum_body` | `member_item` | `declaration` | member source order |
| `attribute_application` | `attribute_argument` | `value_expression` | argument source order |

`SourceDeclarationKey.StableDeclarationPath`必须按§2.1的name/kind/signature或anonymous role/ordinal规则从source-file root唯一解析到declaration；它不复用本表的偶然child ordinal。TemplateRolePath从该declaration root继续并落在准确template role；BindingDefinitionPath从其DeclaringSourceDeclarationKey解析出的declaration root继续并落在`binding_declaration`。revision-local两类SourceStructuralPath的root、终点和每一步合法性都是schema的一部分，不能只校验三个整数是否在enum范围内。

```text
MatchArm = { Pattern : HirNodeId, Guard : OptionalHirNodeId, BodyTemplateId : U, SourceOrdinal : U }
MatchArmVector = Count : U, Items[Count] : MatchArm
CatchClause = { Pattern : HirNodeId, BodyTemplateId : U, SourceOrdinal : U }
CatchClauseVector = Count : U, Items[Count] : CatchClause
```

`MatchArm.SourceOrdinal`与`CatchClause.SourceOrdinal`都必须恰等于其所在vector的zero-based index；vector本身已按源码顺序编码，ordinal只是供identity/verifier交叉核对的冗余字段，不能跳号、重复或另行排序。

`NodeByteLength` 只覆盖本 node 在 `NodeTag` 后的 fields 与 `OriginId`，并且 decoder 必须准确消费。`HirNodeId` 从零 dense 编号；child 必须先于 parent，`RootNodeId = NodeCount - 1`，每个非 root node 恰被一个 node field 引用，因此 payload 是一棵无环、无共享、无不可达 node 的规范后序树。所有 `*KindTag`、`*RoleTag` 和 operator tag 必须来自上表；未知 tag、非规范 optional、错误 field 数量/长度、尾随 byte 或超预算输入必须拒绝。实现必须在分配前检查 artifact decoder budget，且 `NodeCount`、node byte 总量、vector item 总量和 nesting depth 都计入预算。`SemanticDigest` 必须按第 2.3 节 `TemplateSemanticDigest` 公式重算：payload bytes先解码为本树，再按 `CanonicalHirTreeSemanticProjection` 排除长度、dense ID 和 origin，并把外部引用改用 canonical semantic identity；不得直接 hash artifact-local bytes。

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

#### 10.3.1 StageOpcode schema

下表是`plan_semantics` callback的完整per-opcode输入；不是说明性别名。Inputs/TemplateRefs/ResultTypes/Sink任一不匹配都拒绝，effect只能来自实际执行的选中template与本表控制语义，不能由PlanNode自由声明。

| Tag | StageOpcode | Eager Inputs | TemplateRefs | Outputs | Sink | Rule | Effect | Usage | Staging section |
| ---: | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `stage.force_value` | 0..*个显式eager operand，类型/顺序由value template入口schema决定；template captures不重复放入Inputs | 恰1个`RegionKind=value` template | `ResultTypes`恰1项，等于Known result、Value sink与所有PlanResult use的ExpectedType | `value` | 完整elaborate并执行template，要求恰1个Known typed result，canonicalize后原子提交 | 选中template的实际ComptimeWorld effects；Residual依赖、trap、未处理异常、缺capability或预算耗尽均失败 | comptime expression、泛型实参及attribute/decorator实参等强制编译期值位置 | 5.2 |
| 2 | `stage.force_block` | 0..*个显式eager operand；template captures延迟到activation解析 | 恰1个statement或declaration-region template | `ResultTypes`空 | 与template RegionKind匹配的`statement`、`top_level_declaration`或member sink | 在ComptimeWorld完整执行实际路径；不得产生residual CFG，control target不得逃出template/内部activation | 选中block实际effects；comptime地址、host handle、临时引用和跨template control transfer不得逃逸 | StatementRegion的`comptime { ... }`，或declaration region中无条件提交静态声明的comptime block | 5.3 |
| 3 | `stage.select_if` | 恰1个表示condition的eager PlanInput，ExpectedType为bool；若为PlanResult，只有该direct producer进入dependency vector，transitive依赖由producer自身闭合；branch captures不在Inputs | then template必有，else template可选，按源码顺序 | value sink时恰1项且then/else结果类型相同；其他sink为空 | 与then/else共同RegionKind匹配的sink | 条件准确求值一次，只elaborate选中source-backed template；缺else只能用于非value sink并产生空输出 | 选择控制及选中template实际effects；未选分支无绑定、类型检查、依赖执行或effect | 结构化comptime if；选中statement body可按world residualize，declaration body提交声明 | 5.4 |
| 4 | `stage.select_match` | 恰1个表示selector的eager PlanInput；若为PlanResult只记录direct producer；arm captures不在Inputs | 每个源码arm恰1个template，非空并按SourceOrdinal严格递增 | value sink时恰1项且所有arm结果类型相同；其他sink为空 | 与所有arm RegionKind匹配的sink | 按源码arm顺序选择首个Known匹配；pattern/guard/bindings必须编译期确定 | 选择控制及选中arm实际effects；未选arm无绑定、类型检查、依赖执行或effect | 结构化comptime match，保持穷尽性和重复pattern的独立语义验证 | 5.5 |
| 5 | `stage.expand_for` | 恰1个表示finite、Known、deterministic iterable的eager PlanInput；若为PlanResult只记录direct producer；body captures不在Inputs | 恰1个body template | `ResultTypes`空 | 与body RegionKind匹配的statement/declaration sink | 按iterable语义顺序逐项以准确元素类型重新elaborate，每轮建立fresh binding与IterationIdentity | 按迭代顺序累积body实际effects与pending outputs；受每轮及总fuel/生成量预算 | 异构tuple、comptime sequence和其他编译期可枚举源的结构化展开 | 5.6 |
| 6 | `stage.expand_while` | `Inputs`空；condition/body的外层环境只通过各自Template.Captures解析，每轮condition结果不得缓存为PlanInput | 恰2个template，顺序为condition、body | `ResultTypes`空 | 与body RegionKind匹配的statement/declaration sink | 每轮重新执行condition并要求Known bool；true时elaborate一轮body，false终止 | 按轮次累积condition/body实际effects；受循环、fuel、memory、effect与输出预算 | 需要ComptimeWorld状态推进、无法预先枚举的结构化comptime while | 5.7 |
| 7 | `stage.instantiate` | 第一个semantic input为当前module唯一source-backed、GenericDeclarationProvenance present的开放GenericDecl handle，随后为全部规范化显式/default/pack实参 | 同module selected generic declaration注册的完整template vector，按TemplateRolePath顺序 | `ResultTypes`恰1项，为闭合instance handle/type的准确meta type并匹配全部PlanResult use | 与实例请求位置匹配的value或declaration sink | 形成并重算InstanceIdentity；相同identity合流；不做推导、偏应用、SFINAE或dependency/generated-open-generic template实例化 | 实例work item实际effects由被elaborate typed Core决定；只在完整验证后原子发布 | 当前module source-backed显式泛型实例化与已唯一选中候选的闭合实例请求；dependency只可导入provider预生成closed instance | 5.8 |

#### 10.3.2 PlanInput、Sink 与 PlanNode record

PlanInput payload：

| PlanInputTag | payload，按顺序 |
| --- | --- |
| Constant | `ConstantId, ExpectedTypeId` |
| PlanResult | `PlanNodeId, ResultIndex : U, ExpectedTypeId` |
| CoreValue | `OwnerSymbolId, RegionPath, ValueId, ExpectedTypeId` |
| TemplateCapture | `TemplateId, CaptureIndex : U, ExpectedTypeId` |
| InstanceArgument | `InstanceIdentity : InstanceIdentityPayload, ArgumentIndex : U, ExpectedTypeId` |

Sink payload：

```text
SinkKindTag
OwnerSymbolId
RegionStructuralPath : RegionPath
SourceBackedAnchor : SourceBackedAnchorIdentityPayload
InsertionPositionTag
```

Value sink 的 owner/path/anchor 使用消费者位置并固定InsertionPosition=`replace`、AnchorRole=`replace`；statement/top-level/class/interface/enum member sink允许before/after/replace/append且AnchorRole必须与InsertionPosition逐名映射。append 以区域source-backed end anchor表示，不能留空；`callsite|declaration` AnchorRole不允许出现在Plan Sink。SourceBackedAnchor.OwnerSymbol与RegionStructuralPath必须分别等于外层OwnerSymbolId与RegionStructuralPath，内嵌key逐项重算；保留外层字段用于快速dispatch但不允许错配。revision 1没有任何StageOpcode接受SinkKind=`module_registration`；该tag只供`DeclarationSink(module_registration)` effect与ct.register_module_item记录类型使用，PlanNode见此sink必须拒绝。

`ParentElaborationContextPayload` required 顺序固定为 `ParentCanonicalWorkKey : Opt<D32>, EnclosingInstance : Opt<InstanceIdentityPayload>, DecoratorApplication : Opt<DecoratorApplicationIdentityPayload>, DynamicControlPath : Vec<TaggedDynamicPathStep>, ParentElaborationContextDigest : D32`。DynamicPathStep准确复用§2.3的闭合结构化payload；最后一个digest不进入自身preimage并按§2.3公式重算。`ParentCanonicalWorkKey=absent`只表示没有Plan parent，不强制instance/decorator/path为空：generic instance或decorator application内部的root work item可以保留其他context；只有初始module root才要求其余optional absent且path empty。present parent key必须唯一解析到当前artifact中的PlanNode；EnclosingInstance必须由匹配`stage.instantiate`输入/输出或闭合instance Symbol的TaggedClosedGenericArguments重算；DecoratorApplication的完整frame chain逐项重算。任一组件无法从当前artifact/template context机械确认即拒绝，不能只因最终D32长度正确而接受。

`PlanNode` required payload：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `StageOpcode` | `Enum<StageOpcodeTag>` | S |
| 2 | `Inputs` | `Vec<TaggedPlanInput>` | S |
| 3 | `TemplateRefs` | `Vec<Ref<Template>>` | S |
| 4 | `ResultTypes` | `Vec<Ref<Type>>` | S |
| 5 | `Sink` | `TaggedSink` | S |
| 6 | `ParentElaborationContext` | `ParentElaborationContextPayload` | S，nested digest N |
| 7 | `RequiredCapabilities` | `Vec<Enum<CapabilityTag>>` | S |
| 8 | `DependencyPlanNodes` | `Vec<Ref<PlanNode>>` | S |
| 9 | `CanonicalWorkKey` | `D32` | S |
| 10 | `Origin` | `Ref<Origin>` | N |

每个PlanResult要求`ResultIndex < producer.ResultTypes.size`且`ExpectedTypeId`恰等于对应ResultTypes项；没有隐式result或从sink反推的未编码类型。DependencyPlanNodes必须与Inputs内eager PlanResult引用推导出的边完全相等并按PlanNode canonical key排序。`WorkKeyDependencyGraph`把每个PlanNode指向全部DependencyPlanNodes、present ParentCanonicalWorkKey指向的parent，以及所有TemplateRefs的SemanticCaptureProjection中直接出现的PlanResult producer；最后一类是从Template records唯一派生的`TemplateKeyDependencies`，不另存vector。这个key union graph整体必须无环，不能只分别检查eager/parent/lazy子图。writer/verifier按dependency-first stable Kahn先重算被引用producer key、TemplateSemanticDigest、context digest再重算CanonicalWorkKey，每步从ready set选择完整work-key preimage最小者；因此eager-parent或template-capture-parent形成的任何交叉hash环都拒绝。执行调度图与key图不同：select/expand只在实际选中arm/iteration后解析该template captures并确定性创建或唤醒child work item；未选template的TemplateKeyDependency只用于形成可重算digest，绝不执行、观察value或产生effect。force_value/force_block/instantiate因template无条件选中，在activation时才把其capture producer加入对应child执行dependency，而不在parent Inputs重复编码。

`CanonicalWorkKey`的Inputs projection不编码artifact-local ID：Constant使用constant canonical semantic projection；PlanResult使用`producer.CanonicalWorkKey, ResultIndex, ExpectedTypeSemanticIdentity`；CoreValue使用`OwnerSymbolKey, RegionPath, canonical ValueId, ExpectedTypeSemanticIdentity`；InstanceArgument使用重算的InstanceIdentity、ArgumentIndex与ExpectedTypeSemanticIdentity。TemplateRefs替换为TemplateSemanticDigest，ResultTypes替换为TypeSemanticIdentity，Sink的owner/type引用替换为canonical identity，DependencyPlanNodes替换为按key排序的DependencyWorkKeys；RequiredCapabilities按numeric tag严格递增且无重复。writer与decoder都从这些preimage重算CanonicalWorkKey，禁止直接hash dense PlanNodeId/TemplateId/TypeId/SymbolId。

PlanInputTag `TemplateCapture`只用于template activation时物化的child work-item input，禁止出现在序列化parent PlanNode的`Inputs`；其TemplateId必须是parent的已选TemplateRef，CaptureIndex必须落在该Template.Captures且ExpectedType逐项相等。resolver按TaggedCaptureSource重建值：plan_result只把准确direct producer加入child dependency vector并先调度/等待该producer，transitive边仍由producer闭合；constant/type/declaration/instance_argument/target_config解析为对应canonical实体或Known值；core_value与lexical_binding在选中activation中做dominance、scope、access、lifetime和Residual规则检查。该child vector按producer CanonicalWorkKey排序、去重且与实际plan_result capture集合相等；遇到self/ancestor cycle拒绝。未选择template既不创建child input，也不观察其source。

### 10.4 ModuleRegistration record

SectionKind 16 是 Staged/Closed 都 required 的 semantic section；其逻辑 collection 名为 `TypedRegistrations`，canonical section/table 名统一为 `ModuleRegistrations`/`module_registrations`，不得再建立同义第二节。section 第一个且唯一一个 RecordKind 1 `RegistrationSummary` payload 为 `RegistrationEncodingRevision : U` (S)、`RegistrationCount : U` (N)、`ModuleRegistrationSetDigest : D32` (N，重算)、`ModuleRegistrationInterfaceDigest : D32` (N，重算)。`RegistrationEncodingRevision` 与两个 digest 必须分别匹配 Manifest fields 28—30；`RegistrationCount` 必须恰等于后续 RecordKind 2 的数量，不与不存在的 Manifest count 比较。其后是按下述 structural order 排序的 RecordKind 2 `ModuleRegistration`：

| 顺序 | 字段 | 编码 | 投影 |
| ---: | --- | --- | --- |
| 1 | `RegistrationIdentity` | `D32` | S，重算核对 |
| 2 | `RecordType` | `Ref<Type>` | S |
| 3 | `Value` | `Ref<Constant>` | S |
| 4 | `ProducerSymbol` | `Ref<Symbol>` | S |
| 5 | `DecoratorApplication` | `DecoratorApplicationIdentityPayload` | S，内嵌key逐层重算 |
| 6 | `DecoratorApplicationOrderPath` | `Vec<U>` | S |
| 7 | `SourceBackedCallsite` | `SourceBackedCallsiteIdentityPayload` | S，内嵌key重算 |
| 8 | `DynamicControlPath` | `Vec<TaggedDynamicPathStep>` | S |
| 9 | `EmissionOrdinal` | `U` | S |
| 10 | `ProtocolSchemaDigest` | `D32` | S，重算核对 |
| 11 | `Origin` | `Ref<Origin>` | N |

`DecoratorApplicationOrderPath` 是 non-empty、odd-length 的 module-global hierarchical `Vec<U>`。root application path 恰为 `[CanonicalSourceTraversalApplicationOrdinal]`；该 root ordinal 从 canonical source-file order，再从文件内 lexical decorator-application traversal 依次从零分配。由 generated output 发现的 child application 在 parent path 后恰追加 `(ParentSemanticOutputOrdinal, ChildLocalApplicationOrdinal)`：前者是 parent transaction 中所有 ordered semantic-output events 共用的从零连续序号，不是 registration-only `EmissionOrdinal`；后者是该 generated output 内 lexical decorator-application traversal 的从零序号。所有 ordinal 只从 normalized source/output order 派生；hash bytes、fixed-point round、线程、worker、work queue、arena/container insertion order 都不得参与，cache replay 必须重现相同 semantic-output 与 registration-arrival 两套 ordinal。decoder 必须拒绝空 path 或偶数长度；Staged verifier 还必须核对 root/child derivation。相同 order path 的 records 必须解析到同一 `DecoratorApplicationKey` 和 `ProducerSymbol`。

`DynamicControlPath`的tag/payload准确复用§2.3的四种结构化`DynamicPathStepTag`，按结构嵌套/执行路径顺序编码，不是opaque hash，也不得含执行round、线程、worker、容器插入或hash iteration顺序。同一call site的递归/重复调用以InvocationOrdinal区分；loop中canonical identity相等的不同元素仍以IterationOrdinal区分。ModuleRegistration.ProducerSymbol必须等于DecoratorApplication current frame、SourceBackedCallsite以及所有适用dynamic call/expansion payload中的producer；SourceBackedCallsite固定指向当前`ct.register_module_item` opcode identity。重复嵌套的SourceDeclarationKey/SourceFileIdentity、EnclosingInstance与DecoratorApplicationKey必须逐项相等，任何“外层一套、内层另一套”的错配都拒绝。identity 固定为：

```text
H("ink.module-registration.v0",
  CanonicalModuleIdentity,
  ModuleContentDigest,
  ProducerSymbol.SymbolKey,
  DecoratorApplication.DecoratorApplicationKey,
  DecoratorApplicationOrderPath,
  SourceBackedCallsite.SourceBackedCallsiteKey,
  DynamicControlPath,
  EmissionOrdinal)
```

identity 不含 value、全局 commit 顺序或调度顺序。`EmissionOrdinal` 是该 active decorator application 中每次实际到达 `ct.register_module_item` 时按实际源码执行顺序分配的 application-wide ordinal，从 0 开始连续递增；它不在每个 callsite/path 内重新从零计数。identity 因包含 `ModuleContentDigest` 而是 module-content-local，不承诺跨 module version 保持不变；跨版本由完整 registration set 与代码原子发布/替换。只有 pending-batch commit/writer 前可以合并 replay duplicate：相同 `RegistrationIdentity` 的候选必须是完整 logical record 逐字段相同，包括 canonical Origin structure（不是只比 S projection 或 raw OriginId）；否则是 determinism error。writer 输出前必须保证 `RegistrationIdentity` 唯一、`(DecoratorApplicationOrderPath, EmissionOrdinal)` 唯一，并对每个 path 检查 ordinal 恰为 `0..N-1`。canonical section 的 decoder 遇到上述任一种重复都直接拒绝，绝不去重或规范化；它同样重做连续性检查。path 比较按 `U` 元素的无符号数值逐元素 lexicographic order，较短的真前缀先于其 child；records 严格按 `(DecoratorApplicationOrderPath, EmissionOrdinal, RegistrationIdentity)` 排序。identity 只作最终 tie-break，禁止用任何 D32 字节序替代 program order。

`StaticRegistrationEncodable(T, Constant)` 是中央派生 verifier predicate：T/constant graph 已 closed、LayoutComplete 且类型精确匹配；不得含 runtime/meta/host handle、reference、NoEscape、active lifetime、普通 cleanup/resource ownership或任意需要用户 ctor/dtor/callback 的字段。v0 允许递归 scalar、unit、enum、tuple、array、已闭合 nominal aggregate、受限 symbol relocation，以及 ConstantKind `string`；String frozen image是 readonly module-owned immutable bytes + length，无释放责任且不运行普通 String destructor。用户自定义 destructor/resource type 默认拒绝，只有未来中央 registry 分配准确、无用户代码/无 release responsibility 的 frozen encoding 才能放行，用户 attribute 不能自行声明。

非null address relocation只允许指向当前 version 的 immutable static data或stable function entry；function pointer 不得指向version_local body，data pointer不得指向mutable/TLS/runtime-owned storage。raw pointer的canonical null使用下述AddressValueTag=null且没有target；它不是指向地址0的symbol relocation。registry static metadata若要进入未来版本必须分配新RelocationKindTag；v0不接受。record 装载、替换或卸载不运行用户 install/remove callback。
`FrozenEncodingDescriptor`由RecordType与RegistrationEncodingRevision唯一递归派生，与具体Value无关，tag/payload闭集为：1 `scalar`/`ScalarEncodingKindTag : U, BitWidth : U, Opt<FloatFormatTag>`；2 `unit`/empty；3 `enum`/`TypeSemanticIdentity, DiscriminantDescriptor, Vec<(VariantSelectorSymbolKey, Opt<FrozenEncodingDescriptor>)>`；4 `tuple`/`Vec<FrozenEncodingDescriptor>`；5 `array`/`ElementCount : U, ElementDescriptor`；6 `nominal_class`/`TypeSemanticIdentity, Opt<BaseDescriptor>, Vec<(FieldSelectorSymbolKey, FrozenEncodingDescriptor)>`；7 `string`/`Utf8BytesAndLengthEncodingRevision : U`；8 `address`/`AddressEncodingKindTag : U, PointeeOrFunctionSignatureProjection, NullAllowed : Bool`。`ScalarEncodingKindTag`闭集为1 `signed_integer`、2 `unsigned_integer`、3 `ptrsize_unsigned`、4 `bool`、5 `float_bits`；float format仅tag 5 present且位宽匹配f16/f32/f64，bool固定BitWidth=1，ptrsize固定等于TargetKey pointer width，其余integer位宽等于准确builtin。`DiscriminantDescriptor`为tagged payload：1 `explicit_value`/`DiscriminantTypeSemanticIdentity, Vec<(VariantSelectorSymbolKey, DiscriminantConstantSemanticProjection)>`，只用于c/fixed_discriminant；2 `source_ordinal`/empty，只用于ink representation并以variant SourceOrdinal的canonical U作为逻辑判别。`BaseDescriptor` required顺序为`BaseSelectorSymbolKey, FrozenEncodingDescriptor`。`AddressEncodingKindTag`闭集为1 `data_pointer`、2 `function_pointer`；前者的PointeeOrFunctionSignatureProjection是`data(ValueAccessTag, AddressSpace=0, SignatureTypeProjection(Pointee))`且NullAllowed=true，后者是`function(SignatureTypeProjection(FunctionType))`且NullAllowed=false。`RelocationKindTag`仍闭集为1 `immutable_static_data`、2 `stable_function`，只出现在non-null value payload并必须分别匹配两种address kind。`Utf8BytesAndLengthEncodingRevision`在v0唯一合法值为1。

descriptor中的vector按type declaration source order，enum/field/base selector使用完整SymbolKey；descriptor只描述logical fields，不含padding/offset或具体constant bytes。对应frozen value bytes也唯一：integer/ptrsize按准确bit width写little-endian two's-complement/unsigned bits，bool写单字节0或1，float写准确format little-endian bits，unit为空；enum先按DiscriminantDescriptor写判别再写present payload的`ByteLength : U + bytes`；tuple/array/class按descriptor顺序逐child写`ByteLength : U + bytes`；string revision 1写`Utf8ByteLength : U + canonical UTF-8 bytes`；address先写`AddressValueTag : U`，tag 0 `null`无payload且只在NullAllowed=true合法，tag 1 `relocation`再写`RelocationKindTag : U, TargetSymbolKey : Bytes, Addend : I`。function relocation addend必须0且target为stable entry，data relocation target必须是匹配pointee的immutable static global且addend在对象内。by-value containment必须是有限DAG，pointer/function pointer在address leaf停止；resource/destructor/runtime/meta/reference/interface/noescape/opaque/target_blob或其他未列type无法派生并由StaticRegistrationEncodable拒绝。`ProtocolSchemaDigest = H("ink.module-registration.protocol.v0", RegistrationTypeSemanticIdentity, FrozenEncodingDescriptor, TargetLayoutDigest, RegistrationEncodingRevision, RuntimeAbiRevision)`，decoder从RecordType重派生descriptor并重算，禁止以自由D32或实现私有shape替代。

`ModuleRegistrationSetDigest = H("ink.module-registration.set.v0", OrderedModuleRegistrationSemanticProjections)`，其中该 structured field 编码为 `Count : U`，再为每条 canonical ordered record 编码 `ProjectionLength : U + 完整 S projection bytes`；projection 包含 identity/order path/value。`RegistrationContextIndependent`在计算该digest和`ModuleVersionContextKey`前递归遍历完整S projection引用的Type/Constant/Symbol、producer/callsite/decorator identity、FrozenEncodingDescriptor selector及relocation target；任何`GeneratedDeclarationIdentityPayload`的parent为`versioned_owner|module_version_root|versioned_entity_owner`，或任一transitive identity/preimage含`ModuleVersionContextKey`，都拒绝。immutable data relocation因此只可指向source-backed、module_root或其他经相同遍历证明context-independent的immutable static global；stable function relocation只指向上述逻辑stable symbol，绝不指向per-version mapping/body。该闭包使`ModuleRegistrationSetDigest -> ModuleVersionContextKey -> generated identity -> ModuleRegistrationSetDigest`不可能成环。`ModuleRegistrationInterfaceDigest = H("ink.module-registration.interface.v0", SortedUniqueRegistrationSchemaTuples)`，其中该 structured field 编码为 `Count : U` 后接 sorted unique fixed-schema tuples，每个 tuple 恰按 `(RegistrationTypeSemanticIdentity : D32, ProtocolSchemaDigest : D32, TargetLayoutDigest : D32, RegistrationEncodingRevision : U, RuntimeAbiRevision : U)` 编码，不含 RegistrationIdentity、order path 或 value。tuple 按字段逐项排序：D32 使用 unsigned byte lexicographic order，U 使用无符号数值 order；只有全部字段相等才视为重复。两个 Manifest digest 都重算；code-only hot reload要求 interface digest 相等，而 identity/value set 可以随新 module version变化并由 runtime与代码版本原子发布/替换。record owner 是当前 artifact module version 的容器语义，不编码可伪造 runtime token。

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

canonical serialized region只允许从其entry沿successor可达的block；canonicalization必须先删除CFG-unreachable block，encoder遇到剩余不可达block直接拒绝。block order固定为entry首先，再按opcode schema successor顺序稳定先序遍历；因此不需要未序列化的“lowering role”或“generated ordinal”，也不存在对称不可达component的tie-break。`cf.unreachable`仍可作为可达block的终止指令。

BlockId 是 region 内 canonical block ordinal，ValueId 是 function/独立 CFG region 内按 canonical region/block 顺序的 block arguments、再按 block 内 operation results 分配的 canonical ordinal；entry receiver/parameters/hidden channels 已是 block arguments，没有独立 parameter ValueId。它们不另存 identity record。semantic projection 对其引用展开目标结构和规范 CFG position，不把偶然 dense number 作为身份。

function entry block argument固定顺序为：receiver（若有，RoleIndex=0）；ordered logical parameters（RoleIndex=parameter index）；GlobalRecord指定的InitializerFunction/FinalizerFunction可各有一个runtime-provided `global_lifecycle`（RoleIndex=0）；sync且result mode=result_destination时一个hidden active-owner `!place<init,T>` result_destination（RoleIndex=0）；async body固定一个runtime-provided `task_self`（RoleIndex=0），其类型为borrowed-authority `!place<rw,!runtime-object<task,T>>`；async body且logical T address-only时再有一个runtime-generated owner `!place<init,T>` task_result_storage（RoleIndex=0）。parameter mode到entry type的映射精确为：`value`使用逻辑T；`object`使用owner-authority、Alive、独立参数对象`!place<rw,T>`；`const_reference|mutable_reference|raw_pointer`使用签名中已经闭合的准确reference/raw-pointer type且都是non-owning。object参数在callee与全部实参成功求值后仍由caller cleanup stack拥有，activation建立时一次性把全部prepared parameter obligation原子handoff给callee并使caller capability失效；在handoff前任一后续实参、dispatch或Task construction失败，caller按逆构造序清理prepared对象。constructor只有initializing receiver，不另加result destination。global_lifecycle的type是匹配global的唯一owner place：initializer为`!place<init,T>`，finalizer为`!place<ro/rw,T>`；只有runtime持有匹配module-version lifecycle capability时建立。task_self sealed、不可escape/copy/address/project，只能作为frame-internal logical capability spill；task_result_storage初始为AllocatedUninitialized、无TransactionId，真正构造结果时fresh `obj.init.begin`，可跨await spill，成功后commit/as_alive/publish，失败若已begin则cleanup+rollback。非entry CFG block argument全部role=phi、RoleIndex=0；nested region entry显式参数用region_argument/source index。role/type/order不匹配一律拒绝。

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

`OperandValueIds`只编码operation body中显式出现的普通SSA operands，按对应renderer从左到右并保留重复；它不包含DestinationValueId，也不重复Successor.EdgeArguments中的`existing_value`。因此`cf.br`没有普通operand，`cf.cond_br`只有condition，`cf.switch`只有key，`eh.match|eh.end_catch`只有active token；call/lookup/await只编码callee/receiver/task与显式arguments。edge existing values只在各Successor中按successor schema顺序、再按edge argument顺序编码，保留跨edge或同edge重复。decoder从canonical text的operation body和edges分别唯一重建两组；任何把edge value额外塞入OperandValueIds、遗漏main operand或改变顺序都拒绝。共同type signature左侧也只列OperandValueIds类型。

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

| Tag | Name | Binary required payload，按顺序 | Canonical operation text payload |
| ---: | --- | --- | --- |
| 1 | `Empty` | 空 | 不打印payload |
| 2 | `Constant` | `ConstantId` | `{constant = #cN}` |
| 3 | `Symbol` | `SymbolId` | `{symbol = @symbol}` |
| 4 | `SymbolAddend` | `SymbolId, Addend : I` | `{symbol = @symbol, addend = I}` |
| 5 | `IntegerPredicate` | `Enum<IntegerPredicateTag>` | `{predicate = name}` |
| 6 | `FloatPredicate` | `Enum<FloatPredicateTag>` | `{predicate = name}` |
| 7 | `FastMath` | `Bits<FastMathFlagBit>` | 空集不打印payload；非空打印`{fast = [name, ...]}`，按bit递增 |
| 8 | `Switch` | `CaseCount : U, repeat (ConstantId, CaseSuccessorOrdinal : U), DefaultSuccessorOrdinal : U` | switch renderer打印`[case #cN -> EDGE, ...] default EDGE`，不打印dictionary |
| 9 | `SelectorSymbol` | `SymbolId` | key由opcode固定，打印`{base = @symbol}`、`{field = @symbol}`、`{variant = @symbol}`、`{slot = @symbol}`或`{method = @symbol}` |
| 10 | `StaticIndex` | `Index : U` | `{index = N}` |
| 11 | `Alignment` | `Alignment : U` | `{alignment = N}` |
| 12 | `RawTransferAlignment` | `DestinationAlignment : U, SourceAlignment : U` | `{dst_alignment = N, src_alignment = N}` |
| 13 | `TrapKind` | `Enum<TrapKindTag>` | `{kind = name}` |
| 14 | `Call` | `CallPayload` | call renderer按§15.5打印callee variant和`{function_type, entry_identity, calling_convention, target_abi_tag, explicit_argument_count}`；optional destination另打印destination clause |
| 15 | `EhMatch` | `HandlerCount : U, repeat (Enum<ExceptionHandlerKindTag>, Opt<ExceptionTypeId>, HandlerSuccessorOrdinal), UnmatchedSuccessorOrdinal` | eh_match renderer打印`[!tN -> EDGE, ..., catch_all -> EDGE] unmatched EDGE`，不打印dictionary/count/ordinal |
| 16 | `EhPayload` | `AsTypeId` | `{as = !tN}` |
| 17 | `ThrowTarget` | `ExceptionTypeSymbolId, ConstructorSymbolId, ArgumentCount : U` | `{exception_type = @symbol, constructor = @symbol, argument_count = N}` |
| 18 | `InterfaceMake` | `InterfaceSymbolId, DestinationPayload` | `{interface = @symbol}`后打印required destination clause |
| 19 | `InterfaceUp` | `AncestorInterfaceSymbolId, DestinationPayload` | `{ancestor = @symbol}`后打印required destination clause |
| 20 | `TryClass` | `TargetClassSymbolId, Enum<TryClassResultFormTag>, Opt<DestinationPayload>` | `{target = @symbol, result_form = name}`后按present打印destination clause |
| 21 | `TryInterface` | `TargetInterfaceSymbolId, DestinationPayload` | `{target = @symbol}`后打印required destination clause |
| 22 | `ReflectLookup` | `CanonicalModuleIdentity : Str` | `{module = "canonical.module"}` |
| 23 | `ReflectLookupFunction` | `CanonicalModuleIdentity : Str, ExpectedFunctionTypeId` | `{module = "canonical.module", expected_function_type = !tN}` |
| 24 | `ReflectLookupMember` | `Enum<ReflectionMemberKindTag>` | `{member_kind = name}` |
| 25 | `ReflectCall` | `ExpectedFunctionTypeId, ReceiverPresent : Bool, ArgumentCount : U, AdapterNothrow : Bool, Opt<DestinationPayload>` | reflect_call renderer打印`%snapshot receiver(none/%vN) args([%vN, ...]) {expected_function_type = !tN, receiver_present = BOOL, argument_count = N, adapter_nothrow = BOOL}`，方括号内argument list整体可空，再按present打印destination clause |
| 26 | `AsyncCall` | `CallPayload`，destination required 且 role=result | call renderer按§15.5打印`callee_kind`、callee variant和CallPayload dictionary，再打印required result destination clause |
| 27 | `Await` | `AwaitedLogicalTypeId` | 不打印dictionary；从task operand及operation channel type唯一重建并核对awaited logical type |
| 28 | `PublishResult` | `Enum<ResultPassingModeTag>, LogicalResultTypeId` | `{result_mode = name}`；logical result type从task/value operand与channel signature唯一重建并核对 |
| 29 | `DecoratorRegion` | `Enum<DecoratorKindTag>, Layer : U, FunctionTypeId` | `{decorator_kind = name, layer = N, function_type = !tN}` |
| 30 | `Continuation` | `NextLayer : U, FunctionTypeId, Opt<DestinationPayload>` | `{next_layer = N, function_type = !tN}`后按present打印destination clause |
| 31 | `TransferPin` | `Enum<PinOwnerKindTag>` | `{owner_kind = name}` |
| 32 | `AbiCall` | `AbiCallPayload` | abi_call renderer按§15.5打印C callee variant和`{c_function_type, bridge, expected_ink_function_type, target_abi_tag, explicit_argument_count}`；optional destination另打印destination clause |
| 33 | `ResultMode` | `Enum<ResultPassingModeTag>, LogicalResultTypeId` | `{result_mode = name}`；logical result type从包围decorator signature唯一重建并核对 |
| 34 | `ModuleRegistration` | `Enum<RegistrationValueFormTag>, SourceBackedCallsite : SourceBackedCallsiteIdentityPayload` | `{value_form = name, source_backed_callsite = {完整结构化字段...}}` |

`TryClassResultFormTag`：1 `raw_pointer`；2 `optional_reference`。`ReflectionMemberKindTag`：1 `field`；2 `function`；3 `constructor`；4 `base`；5 `interface`。`DecoratorKindTag`：1 `sync`；2 `async`。`PinOwnerKindTag`：1 `task`；2 `exception`；3 `snapshot`；4 `frame`。`RegistrationValueFormTag`：1 `value`；2 `object`。所有 enum 的 text spelling 为表中小写 name。

P22/P23的CanonicalModuleIdentity必须是NFC canonical module identity并在当前Manifest冻结的active module DAG中恰有一个node；semantic projection编码inline UTF-8 bytes而非StringId。revision 1没有Module SymbolKind/ModuleSymbolId，lookup不能用任意`@symbol`伪装module或绕过active dependency pin。

`ExceptionPayloadClass(T)`当且仅当T是closed、NominalCompleteness=definition、LayoutComplete的nominal_class，按§7.2规则不是abstract，并且其完整InterfaceImplementationDeclaration传递闭包实现runtime registry唯一builtin marker interface`ink.runtime.exception_payload_v0`；marker本身不含可调用member。`ExceptionCatchInterface(I)`当且仅当I是closed、complete nominal_interface且等于该marker或在DirectAncestors传递闭包中继承该marker。P15中`typed` handler要求ExceptionTypeId present且满足ExceptionPayloadClass或ExceptionCatchInterface；class/interface handler均可按源码顺序重叠，不能重排。`catch_all`要求type absent，至多一个且必须是handler sequence最后一项；typed type不得重复。HandlerSuccessorOrdinal与UnmatchedSuccessorOrdinal必须覆盖Shape successor vector的准确ordinal且handler ordinal互异。unmatched successor始终required，即使存在catch_all；它保留统一outer-propagation shape但在该match动态语义上不会被选择。

P17 `ThrowTarget`的ExceptionTypeSymbolId必须是唯一Type owner symbol，且其Type满足ExceptionPayloadClass；interface、abstract/incomplete class或普通class都不能throw。ConstructorSymbolId必须是InkCallableSymbolRef并满足HasImplementationBody，准确FunctionSignature为CallableKind=constructor、FunctionKind=sync、ReceiverKind=initializing_instance、ReceiverType等于该exception class、logical result/result mode均void、calling convention ink；其ordered parameters与前ArgumentCount个operands按mode/type逐项匹配。`eh.throw`恰有ArgumentCount个operands；`eh.throw_from`恰有ArgumentCount+1个，最后一个必须是当前handler的active `!exception` owner token且不能同时作为constructor argument。constructor可unwind，但其new-payload transaction在传播constructor exception前已完整rollback/deallocate。

P9的selector kind与owner/type关系是opcode schema的一部分：`enum.value|enum.init_variant|enum.is_variant|place.enum_payload`只能引用`SymbolKind=enum_variant`且selector必须由operand/destination准确nominal enum拥有；`place.field`只能引用`field` selector且result pointee/type/access等于owner class对应ClassFieldDeclaration；`place.base|cast.class_up`只能引用`base` selector且source是准确derived class、result是该唯一single concrete BaseType；其他P9 opcode组合不存在。`enum.value`还要求variant无payload，`place.enum_payload`要求variant有payload且result type准确匹配。virtual CallPayload的SlotSymbol必须解析到匹配receiver class的RuntimeDeclaration(DispatchSlot)，interface CallPayload的MethodSymbol必须是准确interface或其ancestor拥有的`interface_member` selector，并与FunctionType逐项相等；type symbol本身不能替代base/field/variant/member selector。

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

CalleeVariantPayload：direct 为 `SymbolId`；indirect 为空且 operand 0 是 function value；virtual 为 `SlotSymbolId` 且 operand 0 是 receiver；interface 为 `MethodSymbolId` 且 operand 0 是 interface place；reflection 为 `ExpectedFunctionTypeId, ReceiverPresent : Bool` 且 operand 0 是 function snapshot；abi/decorator/async continuation 不允许出现在普通 CallPayload。reflection的`ExpectedFunctionTypeId`必须逐项等于外层`FunctionTypeId`，canonical text只打印一次`function_type`并用它重建两处；不相等直接拒绝。其余 operands 是按 FunctionSignature 顺序的 receiver（若未由 variant 占用）和显式参数；`ExplicitArgumentCount` 必须准确消费 operand 尾部。

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

`AbiCalleeKindTag`：1 `direct`；2 `indirect`。indirect 的 operand 0 是 C function pointer，direct 没有该 operand。两种form的CFunctionTypeId、ExpectedInkFunctionTypeId与TargetAbiTag都必须和policy owner逐项相等。direct的DirectSymbolId必须解析到唯一AbiImport；BridgeSymbolId要么等于该AbiImport.Symbol（identity bridge），要么解析到ExternalTarget恰等于DirectSymbolId的InkToExternal AbiBridge，且两条record的types、target、ForeignFailurePolicy与trusted effect逐项相等。indirect没有DirectSymbol，BridgeSymbol必须解析到ExternalTarget absent的InkToExternal AbiBridge，不能使用AbiImport或target-bound bridge。普通Function或`EntryIdentity=extern`不能替代这些runtime records。v0 indirect pointer没有可序列化、不可伪造的external-target provenance，因此固定使用最大保守ExternalEffectSummary；只有direct form能使用匹配受信record中present的较窄上界。ForeignFailurePolicy唯一决定`abi.call`合法性以及`abi.invoke`的normal/unwind shape，call-site自由attribute不能覆盖。

### 12.4 Effect 与 trait 表记法

后续 opcode 表使用完整 registry name 的紧凑组合：`R(x)` = `ReadMemory(x)`，`W(x)` = `WriteMemory(x)`，`A(x)` = `Allocate(x)`，`D(x)` = `Deallocate(x)`，`RT(x)` = `RuntimeEffect(x)`。`CallSummary`、`AsyncConstructionSummary`、`TaskBodyEffect`、`TaskDestroySummary`、`ExceptionCreateSummary`、`ExceptionDestroySummary` 和 `ExternalEffectSummary` 是从已经验证的callee/type/envelope/runtime-object side fact规范推导的effect set，不是可序列化自由flag；表中列出的static effect与其取并集。`CallSummary` 覆盖 callee body、object receiver/parameter 的 normal/outward-unwind cleanup 及析构闭包；`AsyncConstructionSummary` 覆盖 capture/parameter prepare、handoff、frame构造与失败清理。`ExceptionCreateSummary`覆盖exception payload constructor/copy、typed payload write、失败时已建subobject cleanup、record/box deallocation与version-pin处理；`ExceptionDestroySummary`覆盖last-owner payload/cause destructor、typed/exception state write、record/box deallocation与pin/runtime-registry release，非last共享owner路径仍使用同一保守上界。logical result 为 `never` 的 call不产生normal value/destination，其summary必须按实际可能保留 `MayUnwind`、`MayTrap`、`RT(fatal)` 与 `MayDiverge`，不能用其中一个替代其他 completion。async construction 为新 Task ObjectLifetimeGeneration建立 verifier-only `TaskBodyEffect` 和 `TaskDestroySummary`；前者来自实际 async body/envelope，后者来自 captured parameter、result、ExceptionBox owner release和frame cleanup。该事实随准确 Task generation、owner/borrow capability和CFG edge传播，phi取逐字段并集；跨函数边界或其他位置不能证明来源时必须使用 read_write_any、全部runtime/allocate/deallocate kind、MayTrap、MayDiverge、TargetDependent、PdbBoundary 的最大保守上界。`TargetDependent` 只表示结果依赖 TargetContext，若其他 effect 允许仍可 Speculatable；`PdbBoundary` 永远不可推测、复制、CSE或删除。`Pure`不能与memory/control/runtime effect同时作为“无效果”承诺；出现`Pure 或 R(typed)`的schema必须按operand representation唯一推导其中一个。

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
| 262 (`0x0106`) | `const.symbol_addr` | S1/P4 | symbol relocation，result compatible raw pointer | Pure, TargetDependent | SC | ConstantLike, Speculatable, SymbolUser |
| 263 (`0x0107`) | `const.function` | S1/P3 | symbol function signature 精确匹配，取得 stable entry | Pure, TargetDependent | SC | ConstantLike, Speculatable, SymbolUser |
| 513 (`0x0201`) | `cf.br` | S10/P1 | successor role branch；全部 edge args existing_value | Control | SC | Terminator, EdgeProducing, NoFallthrough |
| 514 (`0x0202`) | `cf.cond_br` | S11/P1 | operand 0 bool；successor true/false；其余 operands 只经 edge args 引用 | Control | SC | Terminator, EdgeProducing, NoFallthrough |
| 515 (`0x0203`) | `cf.switch` | S12/P8 | key 为 integer/enum discriminant；case unique 且按 constant canonical order；default 最后 | Control | SC | Terminator, EdgeProducing, NoFallthrough, Variadic |
| 516 (`0x0204`) | `cf.select` | S4/P1 | `(bool,T,T)->T`，T 为同一 scalar 或 unit | Pure | SC | Speculatable |
| 517 (`0x0205`) | `cf.return` | S9/P1 | void/destination result O0；value/unit result O1 且精确匹配；never 禁止 | Control | SC | Terminator, NoFallthrough |
| 518 (`0x0206`) | `cf.unreachable` | S5/P1 | verifier 证明无合法运行时到达路径 | Control | SC | Terminator, NoNormalReturn, NoFallthrough |
| 769 (`0x0301`) | `arith.add` | S3/P1 | `(I,I)->I` 同一整数，mod 2^N | Pure | SC | Speculatable |
| 770 (`0x0302`) | `arith.sub` | S3/P1 | `(I,I)->I` 同一整数，结果为左操作数减右操作数并按mod 2^N截断 | Pure | SC | Speculatable |
| 771 (`0x0303`) | `arith.mul` | S3/P1 | `(I,I)->I` 同一整数，结果为完整乘积的低N位即mod 2^N | Pure | SC | Speculatable |
| 772 (`0x0304`) | `arith.neg` | S2/P1 | `(I)->I` | Pure | SC | Speculatable |
| 773 (`0x0305`) | `arith.and` | S3/P1 | `(I,I)->I` | Pure | SC | Speculatable |
| 774 (`0x0306`) | `arith.or` | S3/P1 | `(I,I)->I` | Pure | SC | Speculatable |
| 775 (`0x0307`) | `arith.xor` | S3/P1 | `(I,I)->I` | Pure | SC | Speculatable |
| 776 (`0x0308`) | `arith.not` | S2/P1 | `(I)->I` | Pure | SC | Speculatable |
| 777 (`0x0309`) | `arith.cmp` | S3/P5 | `(T,T)->bool`；T为同一整数且predicate与signedness相容，或T为同一bool且predicate仅eq/ne | Pure | SC | Speculatable |
| 1025 (`0x0401`) | `pdb.sdiv` | S3/P1 | `(iN,iN)->iN` | TargetDependent, PdbBoundary, MayTrap | SC | — |
| 1026 (`0x0402`) | `pdb.udiv` | S3/P1 | `(uN,uN)->uN` | TargetDependent, PdbBoundary, MayTrap | SC | — |
| 1027 (`0x0403`) | `pdb.srem` | S3/P1 | `(iN,iN)->iN` | TargetDependent, PdbBoundary, MayTrap | SC | — |
| 1028 (`0x0404`) | `pdb.urem` | S3/P1 | `(uN,uN)->uN` | TargetDependent, PdbBoundary, MayTrap | SC | — |
| 1029 (`0x0405`) | `pdb.shl` | S3/P1 | `(I,ptrsize)->I` | TargetDependent, PdbBoundary, MayTrap | SC | — |
| 1030 (`0x0406`) | `pdb.lshr` | S3/P1 | `(uN/ptrsize,ptrsize)->same` | TargetDependent, PdbBoundary, MayTrap | SC | — |
| 1031 (`0x0407`) | `pdb.ashr` | S3/P1 | `(iN,ptrsize)->iN` | TargetDependent, PdbBoundary, MayTrap | SC | — |
| 1032 (`0x0408`) | `pdb.fptosi` | S2/P1 | `(F)->iN` | TargetDependent, PdbBoundary, MayTrap | SC | — |
| 1033 (`0x0409`) | `pdb.fptoui` | S2/P1 | `(F)->uN/ptrsize` | TargetDependent, PdbBoundary, MayTrap | SC | — |

### 13.2 `fp` 与 `cast`

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 1281 (`0x0501`) | `fp.add` | S3/P7 | `(F,F)->F`，fast flags 受 TargetContext 验证 | Pure, TargetDependent | SC | Speculatable |
| 1282 (`0x0502`) | `fp.sub` | S3/P7 | `(F,F)->F` 同一浮点格式，执行左操作数减右操作数并按TargetContext strict模式与已验证fast flags舍入 | Pure, TargetDependent | SC | Speculatable |
| 1283 (`0x0503`) | `fp.mul` | S3/P7 | `(F,F)->F` 同一浮点格式，执行乘法并按TargetContext strict模式与已验证fast flags舍入 | Pure, TargetDependent | SC | Speculatable |
| 1284 (`0x0504`) | `fp.div` | S3/P7 | `(F,F)->F` 同一浮点格式，执行除法并按TargetContext strict模式与已验证fast flags舍入；浮点除零不是Ink trap | Pure, TargetDependent | SC | Speculatable |
| 1285 (`0x0505`) | `fp.neg` | S2/P1 | `(F)->F`，只翻转 sign bit | Pure | SC | Speculatable |
| 1286 (`0x0506`) | `fp.fma` | S4/P7 | `(F,F,F)->F` | Pure, TargetDependent | SC | Speculatable |
| 1287 (`0x0507`) | `fp.cmp` | S3/P6 | `(F,F)->bool` | Pure | SC | Speculatable |
| 1288 (`0x0508`) | `fp.assume_finite` | S2/P1 | `(F)->F`；输入必须已证明 finite | Pure | SC | — |
| 1537 (`0x0601`) | `cast.int` | S2/P1 | integer 到 integer；result type 给出宽度/signedness | Pure | SC | Speculatable |
| 1538 (`0x0602`) | `cast.int_to_float` | S2/P1 | integer 到 float，RNE | Pure | SC | Speculatable |
| 1539 (`0x0603`) | `cast.float` | S2/P1 | float 到 float | Pure, TargetDependent | SC | Speculatable |
| 1540 (`0x0604`) | `cast.bit` | S2/P1 | 等width bitcast；operand/result只允许同宽fixed integer、`ptrsize`或float；明确排除`bool`、raw/function pointer、reference/place、aggregate、runtime handle/object与exception | Pure | SC | Speculatable |
| 1541 (`0x0605`) | `cast.ptr` | S2/P1 | raw pointer/ptrsize 间或 compatible raw pointer | Pure | SC | Speculatable |
| 1542 (`0x0606`) | `cast.ptr_access` | S2/P1 | `ptr<rw,T,A> -> ptr<ro,T,A>` | Pure | SC | Speculatable |
| 1543 (`0x0607`) | `cast.ref_access` | S2/P1 | `ref<rw,T> -> ref<ro,T>` | Pure | SC | Speculatable |

### 13.3 `enum` 与 generation-aware `slice`

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 1793 (`0x0701`) | `enum.value` | S1/P9 | selector 是无 payload variant；result 是 scalar enum | Pure, TargetDependent | SC | Speculatable, SymbolUser |
| 1794 (`0x0702`) | `enum.init_variant` | S6/P9 | operand `!place<init,E>` active；variant 属于 E | W(typed), BeginLifetime, TargetDependent | SC | Lifetime, SymbolUser |
| 1795 (`0x0703`) | `enum.discriminant` | S2/P1 | scalar enum 或 readable enum place -> ptrsize | Pure 或 R(typed), TargetDependent | SC | — |
| 1796 (`0x0704`) | `enum.is_variant` | S2/P9 | scalar enum 或 readable enum place -> bool | Pure 或 R(typed), TargetDependent | SC | SymbolUser |
| 2049 (`0x0801`) | `slice.init` | S8/P1 | `(%dst:!place<init,slice<A,T>>, %first:!place<ro/rw,T>, %length:ptrsize)->()`；length > 0；first 捕获当前 ObjectLifetimeGeneration | R(typed), W(typed), BeginLifetime | SC | Lifetime |
| 2050 (`0x0802`) | `slice.data` | S2/P1 | readable slice place -> compatible raw pointer；显式丢失 generation authority | R(typed) | SC | — |
| 2051 (`0x0803`) | `slice.length` | S2/P1 | readable slice place -> ptrsize | R(typed) | SC | — |
| 2052 (`0x0804`) | `slice.index` | S3/P1 | readable slice place + ptrsize -> element place；按TargetContext sizeof(T)计算地址；结果携带 borrow authority + selected generation | R(typed), MayTrap, TargetDependent | SC | PlaceProducing |
| 2053 (`0x0805`) | `slice.subslice` | S9/P1 | dst init slice、source readable slice、begin/end ptrsize；按TargetContext sizeof(T)计算data；结果保留所选子范围 generation set | R(typed), W(typed), BeginLifetime, MayTrap, TargetDependent | SC | Lifetime |
| 2054 (`0x0806`) | `slice.init_empty` | S6/P1 | `!place<init,slice<A,T>> -> ()`；建立 canonical `(null,0)` 与空 BorrowGenerationSet | W(typed), BeginLifetime | SC | Lifetime |

`BorrowGenerationSet`、ObjectLifetimeGeneration 和 element borrow authority 是 verifier dataflow state，不进入 type/operation binary payload，也不打印为自由 attribute；decoder 后 verifier 必须从 `slice.init`/`init_empty`/`subslice`/`index`/`data` 重新推导。`slice.init` 不接受 raw pointer，空 slice 只能用 `slice.init_empty`。`slice.init`、`slice.init_empty` 和 `slice.subslice` 只填充当前 active owner transaction，不自动 commit；final completer 必须随后显式执行一次 `obj.init.commit`。

### 13.4 `mem`、`place`、`raw` 与 `ptr`

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 2305 (`0x0901`) | `mem.alloca` | S1/P1 | result `!place<init,T>`，T SizedObjectType | A(stack) | SC | PlaceProducing |
| 2306 (`0x0902`) | `mem.global_place` | S1/P3 | symbol global；普通 use 只产生 matching generation 的 borrow-authority place，绝不产生 owner | Pure, TargetDependent | SC | PlaceProducing, SymbolUser |
| 2307 (`0x0903`) | `mem.load` | S2/P1 | readable Alive place of scalar/unit -> T | R(typed) | SC | — |
| 2308 (`0x0904`) | `mem.store` | S7/P1 | `(T,!place<rw,T>)->()`，Alive scalar/unit | W(typed) | SC | — |
| 2309 (`0x0905`) | `mem.load_unaligned` | S2/P11 | readable Alive place -> scalar | R(typed) | SC | — |
| 2310 (`0x0906`) | `mem.store_unaligned` | S7/P11 | value + writable Alive place | W(typed) | SC | — |
| 2561 (`0x0A01`) | `place.deref` | S2/P1 | compatible raw pointer -> borrow-authority place；T 非 SealedRuntimeStorage；raw 前置成立时捕获当前 ObjectLifetimeGeneration | Pure | SC | PlaceProducing |
| 2562 (`0x0A02`) | `place.from_ref` | S2/P1 | reference -> same access/type place | Pure | SC | PlaceProducing |
| 2563 (`0x0A03`) | `place.as_alive` | S2/P1 | `!place<init,T>` -> `!place<ro/rw,T>`；当前 path 已有 commit/destination normal Alive proof | Pure | SC | PlaceProducing, CapabilityRebind |
| 2564 (`0x0A04`) | `place.as_uninitialized` | S2/P1 | owner-authority `!place<rw,T>` -> `!place<init,T>`；destroy 后准确 state 为 AllocatedUninitialized，无 live borrow/child capability；readonly 非法 | Pure | SC | PlaceProducing, CapabilityRebind |
| 2565 (`0x0A05`) | `place.field` | S2/P9 | field 属于准确 class；保留 path/access 上限 | Pure, TargetDependent | SC | PlaceProducing, SymbolUser |
| 2566 (`0x0A06`) | `place.base` | S2/P9 | concrete declared base path | Pure, TargetDependent | SC | PlaceProducing, SymbolUser |
| 2567 (`0x0A07`) | `place.tuple_element` | S2/P10 | static tuple index 合法 | Pure, TargetDependent | SC | PlaceProducing |
| 2568 (`0x0A08`) | `place.array_element` | S3/P1 | array place + ptrsize -> element place | Pure, TargetDependent, MayTrap | SC | PlaceProducing |
| 2569 (`0x0A09`) | `place.enum_payload` | S2/P9 | active/partially initialized variant proof | Pure, TargetDependent | SC | PlaceProducing, SymbolUser |
| 2570 (`0x0A0A`) | `place.addr` | S2/P1 | nonsealed place -> compatible raw pointer；丢失 lifecycle authority | Pure | SC | — |
| 2571 (`0x0A0B`) | `place.borrow` | S2/P1 | Alive place -> non-owning reference | Pure | SC | — |
| 2817 (`0x0B01`) | `raw.load` | S2/P11 | compatible raw pointer -> scalar T；访问范围与 active/partial SealedRuntimeStorage 不相交 | R(raw) | SC | — |
| 2818 (`0x0B02`) | `raw.store` | S7/P11 | scalar T + compatible raw pointer；访问范围与 active/partial SealedRuntimeStorage 不相交 | W(raw) | SC | — |
| 2819 (`0x0B03`) | `raw.memcpy` | S8/P12 | dst, src, byte count；未授权重叠非法；两范围均与 active/partial sealed range 不相交 | R(raw), W(raw) | SC | — |
| 2820 (`0x0B04`) | `raw.memmove` | S8/P12 | dst, src, byte count；允许彼此重叠；两范围均与 active/partial sealed range 不相交 | R(raw), W(raw) | SC | — |
| 2821 (`0x0B05`) | `raw.memset` | S8/P11 | dst, u8 byte, byte count；范围与 active/partial sealed range 不相交 | W(raw) | SC | — |
| 3073 (`0x0C01`) | `ptr.offset` | S3/P1 | raw pointer + ptrsize elements -> same pointer type；T sized | Pure, TargetDependent | SC | Speculatable |
| 3074 (`0x0C02`) | `ptr.byte_offset` | S3/P1 | raw pointer + ptrsize bytes -> same pointer type | Pure | SC | Speculatable |
| 3075 (`0x0C03`) | `ptr.cmp` | S3/P5 | compatible address spaces；predicate 仅 eq/ne/ult/ule/ugt/uge | Pure | SC | Speculatable |

`CapabilityRebind` 的共同语义是消费当前 verifier-only `PlaceCapabilityGeneration` 并产生 next generation；地址和 projection path 不变，旧 generation 的任何后续 use 非法。两条 rebind 虽是 `Pure`，也没有 `Speculatable` trait，optimizer 不得复制、CSE、hoist 或 sink。rollback 只终结 active transaction并保留原 init generation；`place.as_uninitialized` 只服务 destroy 后 owner `rw` 到 `init` 的重绑定。

### 13.5 `obj` 与 `call`

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 3329 (`0x0D01`) | `obj.init.begin` | S6/P1 | owner-authority init place；建立 fresh active transaction | BeginLifetime | SC | Lifetime |
| 3330 (`0x0D02`) | `obj.init` | S7/P1 | owner transaction 内 nonsealed init place + scalar/unit value，顺序固定 dst,value；active aggregate parent的direct leaf原子建立Alive/fresh leaf generation并登记path/order，root scalar的generation仍由commit建立 | W(typed), BeginLifetime | SC | Lifetime |
| 3331 (`0x0D03`) | `obj.init.copy` | S7/P1 | owner transaction 内 nonsealed init dst + readable Alive address-only Copyable src；`CopyConstructionEnabled(T)`；原子为每个descendant建立fresh generation/ActiveObjectTree并分别登记InitializedLeafMask/CommittedChildMask，外层仍待commit；source generation 匹配 | R(typed), W(typed), BeginLifetime | SC | Lifetime |
| 3332 (`0x0D04`) | `obj.init.commit` | S6/P1 | owner-authority nonsealed transaction root 完整且无 active child；唯一 commit并建立 fresh ObjectLifetimeGeneration | W(typed), BeginLifetime | SC | Lifetime |
| 3333 (`0x0D05`) | `obj.assign.copy` | S7/P1 | nonsealed writable Alive dst + readable Alive Copyable src；`CopyAssignmentEnabled(T)`；enum只在所有payload递归assign/construct且NeedsDestroy=false时允许分支切换，原子终结旧payload generation tree并建立fresh新tree；authority/generation 均匹配 | R(typed), W(typed)；enum分支可不同时再加EndLifetime, BeginLifetime | SC | — |
| 3334 (`0x0D06`) | `obj.destroy` | S6/P1 | owner-authority `!place<ro/rw,T>` Alive；T nonsealed、静态完整；nominal class从准确NominalSemanticProperties.Destructor（若present）开始并按声明逆序执行唯field/base链，全链nothrow；按析构顺序终结全部descendant generation后终结root generation | W(typed), EndLifetime, CallSummary | SC | Lifetime |
| 3335 (`0x0D07`) | `obj.destroy_dynamic` | S7/P1 | owner-authority nonsealed complete object + matching stable vtable；选择当前兼容析构entry，按动态析构顺序终结全部descendant generation后终结root generation | W(typed), EndLifetime, CallSummary, RT(dynamic_destroy), RT(version_select), MayDiverge | SC | Lifetime, RuntimeSemantic |
| 3585 (`0x0E01`) | `call.direct` | S13/P14 | CalleeKind=direct；ink `ExecutableInkTarget`；nothrow；constructor只用concrete initializing_receiver destination；result form/role 精确匹配 | CallSummary；stable 再加 RT(version_select) | SC | CallLike, DestinationChannel, SymbolUser |
| 3586 (`0x0E02`) | `call.indirect` | S13/P14 | CalleeKind=indirect；function value/signature 精确且provenance只能到达`ExecutableInkTarget`；独立 nothrow proof | CallSummary | SC | CallLike, DestinationChannel |
| 3587 (`0x0E03`) | `call.virtual` | S13/P14 | CalleeKind=virtual；receiver/slot/signature/version table 匹配；slot concrete且final target满足`ExecutableInkTarget`、nothrow | CallSummary, RT(virtual_dispatch) | SC | CallLike, DestinationChannel, SymbolUser |
| 3588 (`0x0E04`) | `call.interface` | S13/P14 | CalleeKind=interface；interface place/method/signature 匹配；binding concrete且final target满足`ExecutableInkTarget`、nothrow | CallSummary, RT(interface_dispatch) | SC | CallLike, DestinationChannel, SymbolUser |
| 3589 (`0x0E05`) | `call.invoke` | S14/P14 | callee kind direct/indirect/virtual/interface/reflection；复用对应`ExecutableInkTarget`/concrete-dispatch规则；恰有 normal/unwind；scalar normal 用 result(0)，address normal 不带 result | CallSummary, MayUnwind, Control；kind-specific RT effect | SC | Terminator, CallLike, InvokeLike, EdgeProducing, DestinationChannel, NoFallthrough |

`obj.init` 的 operand 顺序以本表为 binary source of truth，即 destination 在前、value 在后。任何旧示意若写成相反顺序必须在进入 canonical IR 前规范化。

Call schema 的 SSA/result channel 固定：void 无 result/destination；unit/scalar nothrow call 恰有一个 SSA result；address-only result 使用 destination role=result；constructor 使用 destination role=initializing_receiver；invoke 的 unit/scalar只在 normal edge产生 `result(0)`，address/constructor normal edge不产生 value。所有 destination normal postcondition只建立 Alive proof，successor 用 `place.as_alive` 重绑定 capability。

### 13.6 `eh`、trap 与 fatal

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 3841 (`0x0F01`) | `eh.entry` | S2/P1 | 只在 unwind successor 首部；incoming -> active exception | RT(exception), Control | SC | OwnedHandle |
| 3842 (`0x0F02`) | `eh.match` | S12/P15 | active token；handler source order，catch-all 最后，unmatched required | RT(exception_match), Control | SC | Terminator, EdgeProducing, NoFallthrough |
| 3843 (`0x0F03`) | `eh.payload` | S2/P16 | active token -> readonly nonescaping payload view | R(exception) | SC | — |
| 3844 (`0x0F04`) | `eh.end_catch` | S10/P1 | consume当前ActiveUnwindRecord；唯一normal successor；若引用共享ExceptionBox则只release当前引用，最后一个owner才销毁payload/cause/pin | ExceptionDestroySummary, RT(exception), EndLifetime, Control | SC | Terminator, EdgeProducing, NoFallthrough |
| 3845 (`0x0F05`) | `eh.throw` | S21/P17 | constructor args；新 exception payload/record | ExceptionCreateSummary, A(exception), W(exception), BeginLifetime, RT(exception), MayUnwind, Control | SC | Terminator, NoNormalReturn, NoFallthrough, SymbolUser |
| 3846 (`0x0F06`) | `eh.throw_copy` | S21/P1 | readable Alive source，其T满足ExceptionPayloadClass、Copyable且`CopyConstructionEnabled(T)`；payload按obj.init.copy的完整descendant-generation规则建立 | ExceptionCreateSummary, A(exception), R(typed), W(exception), BeginLifetime, RT(exception), MayUnwind, Control | SC | Terminator, NoNormalReturn, NoFallthrough |
| 3847 (`0x0F07`) | `eh.throw_from` | S21/P17 | 前ArgumentCount个constructor args + 最后active cause；成功转移cause，constructor失败则release旧token后传播新异常 | ExceptionCreateSummary, ExceptionDestroySummary, A(exception), W(exception), BeginLifetime, RT(exception), MayUnwind, Control | SC | Terminator, NoNormalReturn, NoFallthrough, SymbolUser |
| 3848 (`0x0F08`) | `eh.rethrow` | S21/P1 | consume active token，跳过当前 handler list | RT(exception), MayUnwind, Control | SC | Terminator, NoNormalReturn, NoFallthrough |
| 3849 (`0x0F09`) | `eh.resume` | S21/P1 | consume active token，交外层 continuation | RT(exception), MayUnwind, Control | SC | Terminator, NoNormalReturn, NoFallthrough |
| 4097 (`0x1001`) | `rt.trap` | S21/P13 | 立即产生trap completion；不可 catch，不运行语言 cleanup | RT(trap), MayTrap, Control | SC | Terminator, RuntimeSemantic, NoNormalReturn, NoFallthrough |
| 4098 (`0x1002`) | `rt.fatal` | S21/P13 | O0 或 O1 active exception；fail-fast | RT(fatal), MayDiverge, Control | SC | Terminator, RuntimeSemantic, NoNormalReturn, NoFallthrough |

active exception token 的合法终点恰为 `eh.end_catch`、`eh.rethrow`、`eh.throw_from`、`eh.resume` 或携带该token的`rt.fatal`；每条动态路径必须且只能选择其中一个。共享ExceptionBox的payload lifetime属于box owner集合，不属于任一单独ActiveUnwindRecord。

## 14. Runtime、decorator 与 ABI OpcodeTag registry

本节与调用/runtime 章节的 v0 canonical list 一一对应。`abi.export` 是 RuntimeDeclaration RecordKind 8，不是 CFG opcode，因而不占 OpcodeTag。

### 14.1 Dynamic cast

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 1601 (`0x0641`) | `cast.class_up` | S2/P9 | class ref -> declared concrete base ref；保持 access/lifetime | Pure, TargetDependent | SC | Speculatable, SymbolUser |
| 1602 (`0x0642`) | `cast.interface_make` | S15/P18 | object ref + destination；constructed type 是普通 address-only interface_reference；唯一 runtime commit | R(runtime_metadata), W(typed), BeginLifetime, TargetDependent, RT(interface_lookup) | SC | DestinationChannel, RuntimeSemantic, SymbolUser |
| 1603 (`0x0643`) | `cast.interface_up` | S15/P19 | readable interface-reference place + destination；ancestor registered；唯一 runtime commit | R(typed), R(runtime_metadata), W(typed), BeginLifetime, TargetDependent | SC | DestinationChannel, RuntimeSemantic, SymbolUser |
| 1604 (`0x0644`) | `cast.try_class` | scalar S2/P20 或 object S15/P20 | source 有效 class/interface view；raw-pointer form 失败为 null；optional-reference form destination 构造 Some/None | R(runtime_metadata), RT(dynamic_cast), TargetDependent；object 再加 W(typed), BeginLifetime | SC | DestinationChannel, RuntimeSemantic, SymbolUser |
| 1605 (`0x0645`) | `cast.try_interface` | S15/P21 | source 有效 class/interface view；ordinary Optional<InterfaceRef> destination 构造 Some/None | R(runtime_metadata), W(typed), BeginLifetime, RT(dynamic_cast), TargetDependent | SC | DestinationChannel, RuntimeSemantic, SymbolUser |

interface reference 与 Optional descriptor 都不是 `runtime_object`；它们按各自普通 type schema 验证 Copyable/address-only 性质。所有 destination variant normal completion 由该 opcode schema 原子执行唯一 commit，caller 随后用 `place.as_alive` 重绑定。

### 14.2 Reflection

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 4609 (`0x1201`) | `reflect.lookup_type` | S17/P22 | String place；found/missing；found 产生 owned type_snapshot | R(runtime_registry), RT(reflection_lookup), RT(version_select), Control | SC | Terminator, EdgeProducing, OwnedHandle, RuntimeSemantic, NoFallthrough |
| 4610 (`0x1202`) | `reflect.lookup_interface` | S17/P22 | String place；found 产生 owned interface_snapshot | R(runtime_registry), RT(reflection_lookup), RT(version_select), Control | SC | Terminator, EdgeProducing, OwnedHandle, RuntimeSemantic, NoFallthrough |
| 4611 (`0x1203`) | `reflect.lookup_function` | S17/P23 | String place + exact expected signature；found 产生 owned function_snapshot | R(runtime_registry), RT(reflection_lookup), RT(version_select), Control | SC | Terminator, EdgeProducing, OwnedHandle, RuntimeSemantic, NoFallthrough |
| 4612 (`0x1204`) | `reflect.lookup_member` | S17/P24 | owner snapshot + String place；found 产生 owner-bounded borrowed member_view | R(reflection_snapshot), RT(reflection_lookup), Control | SC | Terminator, EdgeProducing, BorrowedHandle, RuntimeSemantic, NoFallthrough |
| 4613 (`0x1205`) | `reflect.snapshot.clone` | S2/P1 | owned K_snapshot -> 第二个相同 kind linear owner | R(runtime_registry), W(runtime_registry), RT(version_pin) | SC | OwnedHandle, RuntimeSemantic |
| 4614 (`0x1206`) | `reflect.snapshot.release` | S6/P1 | consume snapshot owner；全部 borrowed view 已结束 | R(runtime_registry), W(runtime_registry), RT(version_pin) | SC | OwnedHandle, RuntimeSemantic |
| 4615 (`0x1207`) | `reflect.call` | S13/P25 | sync pinned snapshot；target nothrow且AdapterNothrow由verifier从exact signature、typed place/generation和pinned layout证明；void/scalar/destination form精确 | CallSummary, RT(reflection_dispatch)；destination 再加 W(typed), BeginLifetime | SC | CallLike, DestinationChannel, RuntimeSemantic |

lookup 的 found transition 原子执行 version select + pin；v0 不注册 `reflect.snapshot.acquire`。`AdapterNothrow` 是必须由 verifier 重算的证明位，不能由 payload 自我授权；任一 adapter 输入检查可能失败时必须为 false，并唯一使用 `call.invoke` 且 `CalleeKind=reflection`，不注册 `reflect.invoke`。Core 不存在可存储或可传递的 `DynamicRef` 类型；reflection operand 使用普通 scalar/reference/raw-pointer或准确typed place，runtime adapter只在调用内部形成不可观察的临时动态描述符。

### 14.3 Async、Task 与 cancellation

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 4353 (`0x1101`) | `async.call` | S15/P26 | async callee；T=void/never或closed runtime-representable Copyable且CopyConstructionEnabled、无NoEscape；`construction_nothrow=true`；required Task<T> destination role=result；schema原子执行唯一Task-specific sealed commit | AsyncConstructionSummary, A(task_frame), W(typed), BeginLifetime, RT(task_construct)；stable 再加 RT(version_select) | SC | CallLike, DestinationChannel, RuntimeSemantic |
| 4354 (`0x1102`) | `async.invoke` | S16/P26 | async callee；T约束同 async.call；Task<T> destination；normal原子执行Task-specific sealed commit，unwind原子rollback；只表示同步construction completion/failure | AsyncConstructionSummary, A(task_frame), W(typed), BeginLifetime, RT(task_construct), MayUnwind, Control；stable 再加 RT(version_select) | SC | Terminator, CallLike, InvokeLike, EdgeProducing, DestinationChannel, RuntimeSemantic, NoFallthrough |
| 4355 (`0x1103`) | `async.await` | S18/P27 | rw Task<T> place；created状态会drive；normal对unit/scalar产生result(0)，void/never无result；never normal successor不可达；unwind exception | TaskBodyEffect, R(task_state), W(task_state), A(exception), RT(async_suspend), MayUnwind, MayDiverge, Control | SC | Terminator, EdgeProducing, MaySuspend, RuntimeSemantic, NoFallthrough |
| 4356 (`0x1104`) | `async.await_copy` | S19/P27 | rw Task<T> + required T destination；created状态会drive；T address-only、Copyable且CopyConstructionEnabled，succeeded时按obj.init.copy的完整descendant-generation规则构造并原子commit；normal不产生edge value | TaskBodyEffect, R(task_state), R(task_result), W(task_state), W(typed), BeginLifetime, A(exception), RT(async_suspend), MayUnwind, MayDiverge, Control | SC | Terminator, EdgeProducing, DestinationChannel, MaySuspend, RuntimeSemantic, NoFallthrough |
| 4357 (`0x1105`) | `async.task.drive_once` | S6/P27 | compiler/runtime generated；created -> pending，执行到 suspend/final | TaskBodyEffect, R(task_state), W(task_state), RT(task_drive), MayTrap, MayDiverge | SC | RuntimeSemantic, MaySuspend |
| 4358 (`0x1106`) | `async.task.publish_success` | S21/P28 | void O1；value/object O2；result mode/type 精确；pending -> succeeded；object operand 是 Task 内部已 Alive ResultStorage | R(task_state), W(task_state), RT(task_publish), Control；nonunit value 再加 W(task_result) | SC | Terminator, RuntimeSemantic, NoFallthrough |
| 4359 (`0x1107`) | `async.task.publish_failure` | S21/P1 | Task place + active exception；consume token，pending -> failed | R(task_state), W(task_state), A(exception_box), RT(task_publish), Control | SC | Terminator, RuntimeSemantic, NoFallthrough |
| 4360 (`0x1108`) | `async.task.destroy` | S6/P27 | owner-authority Task<T> place；非 pending 按TaskDestroySummary清理并使storage uninitialized；pending走schema-defined fatal且不完成 | TaskDestroySummary, R(task_state), W(task_state), W(typed), EndLifetime, D(task_frame), RT(task_destroy), RT(fatal), MayDiverge | SC | Lifetime, RuntimeSemantic |
| 4361 (`0x1109`) | `async.cancel.request` | S6/P27 | rw Task<T>；nothrow、幂等、线程安全 request publish | W(task_state), RT(cancel_request) | SC | RuntimeSemantic |
| 4362 (`0x110A`) | `async.cancel.is_requested` | S2/P27 | ro/rw Task<T> -> bool；不 drive Task | R(task_state), RT(cancel_query) | SC | RuntimeSemantic |
| 4363 (`0x110B`) | `async.continuation_invoke` | S14/P30 | staged async decorator 下一层；同一 Task/frame；normal/unwind；optional destination原样转发；不可逃逸或并发 | TaskBodyEffect, RT(async_suspend), MayDiverge, Control；effective next-layer BehaviorContract允许时才加MayUnwind | S | Terminator, InvokeLike, EdgeProducing, DestinationChannel, MaySuspend, NoFallthrough |

`Task<T>` 的 constructed/operand type必须是 `!runtime-object<task,T>`。Task 的 sealed storage不能进入generic copy、commit、destroy或place/raw memory schema；只有通用owner `obj.init.begin`、capability rebind与本表Task opcode能操作其place。`async.call`/`async.invoke`是唯一Task construction final-completer，normal postcondition原子执行sealed commit并建立fresh ObjectLifetimeGeneration以及TaskBodyEffect/TaskDestroySummary；不得在callee或successor插入generic `obj.init.commit`。v0不注册`async.await_cancel_on_request`、`async.all`或`async.all_cancel_on_error`；它们必须在canonical IR前展开。

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

`call.*` 只接受 calling convention `ink`，`abi.*` 只接受 `c` 和已注册 bridge。`CRepresentationSafe(T, TargetAbiTag, TargetKey)`是与参数位置无关的封闭representation predicate：T必须具有注册C representation，aggregate要求显式repr、无Ink destructor/vptr/NoEscape/部分初始化语义、字段递归representation-safe且layout digest匹配。`CAbiSafe(T, TargetAbiTag, TargetKey, CAbiPositionPayload)`在此基础上再要求TargetABI为准确parameter/result/status/callback root及AggregateFieldPath注册了唯一classification；unit/never、reference、slice、interface reference、runtime object/handle、exception、meta/dependent和未注册address space始终拒绝。repr(C)定义只使用CRepresentationSafe，AbiImport/Export/Bridge的每个实际C signature位置使用CAbiSafe，不能为类型定义伪造root position。缺少受信外部 effect 声明时ExternalEffectSummary是read/write-any、任意allocate/deallocate/runtime effect、MayTrap、MayDiverge、TargetDependent、PdbBoundary 的保守上界。`StorageKind=heap`在v0只可出现在这种已注册runtime/extern effect summary中；Core没有用户可调用的heap/GC/arena owner opcode，moving GC、arena root/barrier/finalization明确不属于revision 1。`abi.export` required record payload已在9.5节定义；它的canonical text位于runtime declaration table，不能出现在block中。

### 14.7 Comptime module registration

| Tag | Opcode | Shape / schema | Type/CFG rule | Effects | Stage | Traits |
| ---: | --- | --- | --- | --- | --- | --- |
| 5377 (`0x1501`) | `ct.register_module_item` | S6/P34 | value form operand 为 scalar/unit T；object form为 `!place<ro,T>`；两者必须生成满足 StaticRegistrationEncodable(T, Constant) 的 frozen constant | DeclarationSink(module_registration)；object form 再加 R(typed) | S | OrderedSemanticEmit |

该 opcode 只能在 ComptimeWorld 的 active decorator-application context 执行；动态 CFG 自然决定是否以及执行几次。`OrderedSemanticEmit` 表示它不可 CSE、复制、推测、删除或越过其他 semantic emit 重排，但 deterministic record emission 本身不使 comptime/module cache noncacheable。完整`SourceBackedCallsiteIdentityPayload`是operation payload，内嵌key由ProducerSymbol、SourceFileContentDigest、SourceDeclarationKey、NormalizedStructuralNodePath和固定`core_opcode(ct.register_module_item)` callee identity重算；DecoratorApplication、order path、DynamicControlPath和application-wide EmissionOrdinal来自已验证执行context。每次执行向当前 pending declaration/registration batch 发射 ModuleRegistration record；batch 验证、ordinal continuity 和 identity collision 检查成功后才按 canonical key提交。它不是 Plan node，不新增 StageOpcodeTag，不可 residualize；ClosedModule 中 opcode 必须消失，但已提交 ModuleRegistrations section 保留。

canonical text 固定为 `ct.register_module_item %v {value_form = value|object, source_backed_callsite = {producer_symbol = @symbol, source_file_content_digest = sha256"...", source_declaration_key = hex"...", normalized_structural_node_path = [...], callee_identity = core_opcode(ct.register_module_item), source_backed_callsite_key = sha256"..."}} : (T) -> () loc(#oN)`；object form 的 T 在 operand type signature 中是 `!place<ro,T>`。嵌套record字段按SourceBackedCallsiteIdentityPayload required顺序完整打印，key不可单独替代preimage。

### 14.8 明确未注册的名称

以下拼写在 revision 1 中没有 OpcodeTag，decoder/printer/verifier 必须拒绝：`reflect.snapshot.acquire`、`reflect.invoke`、`async.await_cancel_on_request`、`async.all`、`async.all_cancel_on_error`、通用 `rt.call`、`runtime.invoke`、`obj.init.abort`、`committed_destination` edge sentinel，以及把 runtime operation 伪装成自由 attribute 的任何别名。

## 15. Canonical text 完整生成语法

本节定义 printer 输出，不定义宽松 parser。双引号内是 ASCII literal，`LF` 是单个 U+000A。文件为 UTF-8、无 BOM、无 CR、无尾随空白，最后恰有一个 LF。缩进恰为每层两个 ASCII space；不输出空行或注释。

### 15.1 Lexical value 与 type-use

```ebnf
digit = "0" | nonzero-digit ;
nonzero-digit = "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
ASCII-letter = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z" | "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" | "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" | "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z" ;
upper-hex-digit = digit | "A" | "B" | "C" | "D" | "E" | "F" ;
printable-ascii-except-quote-backslash = ? one scalar U+0020..U+007E except U+0022 and U+005C ? ;
LF = ? the single scalar U+000A ? ;
decimal = "0" | nonzero-digit, { digit } ;
signed-decimal = "-", nonzero-digit, { digit } ;
bool = "false" | "true" ;
digest = "sha256\"", 64 * upper-hex-digit, "\"" ;
bytes = "hex\"", { upper-hex-digit, upper-hex-digit }, "\"" ;
apint = "apint(", decimal, ",0x", upper-hex-digit, { upper-hex-digit }, ")" ;
float-bits = "floatbits(", ("f16" | "f32" | "f64"), ",0x", upper-hex-digit, { upper-hex-digit }, ")" ;
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
tuple-value = "(", value, ", ", value, { ", ", value }, ")" ;
field-name = bare-segment ;
record-value = "{", [ field-name, " = ", value, { ", ", field-name, " = ", value } ], "}" ;
variant-name = bare-segment ;
variant-value = variant-name, "(", [ value, { ", ", value } ], ")" ;
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
enum-name = bare-segment ;
artifact-kind = "staged" | "closed" ;
table-name = "source_files" | "origins" | "types" | "attributes" | "constants" | "symbols" | "globals" | "functions" | "runtime_declarations" | "module_registrations" | "dependencies" | "normalized_templates" | "elaboration_plan" ;
builtin-short = "i8" | "i16" | "i32" | "i64" | "i128" | "u8" | "u16" | "u32" | "u64" | "u128" | "ptrsize" | "f16" | "f32" | "f64" | "bool" | "void" | "never" | "unit" ;
place-access = "ro" | "rw" | "init" ;
runtime-handle-kind = "type_snapshot" | "interface_snapshot" | "function_snapshot" | "member_view" | "exception_box" | "version_owner" | "version_pin" | "decorator_continuation" | "abi_handle" ;
runtime-object-kind = "task" ;
opcode = bare-segment, ".", bare-segment, { ".", bare-segment } ;
value = decimal | signed-decimal | bool | digest | bytes | apint | float-bits | string | enum-name | symbol-reference | value-ref | block-ref | type-use | attribute-ref | constant-ref | source-ref | origin-ref | dependency-ref | template-ref | plan-ref | registration-ref | optional | vector | tuple-value | record-value | variant-value | dynamic-control-path | normalized-hir-value ;
normalized-hir-value = ? the field-specific hir_v1 production from §15.3 ? ;
dynamic-control-path = "[", [ dynamic-path-step, { ", ", dynamic-path-step } ], "]" ;
dynamic-path-step = "expansion(", record-value, ")"
                  | "call(", record-value, ", ", decimal, ")"
                  | "branch(", record-value, ", ", decimal, ")"
                  | "loop(", record-value, ")" ;

type-use = builtin-short | type-ref | inline-capability ;
inline-capability = "!place<", place-access, ",", type-use, ">"
                  | "!exception"
                  | "!runtime-handle<", runtime-handle-kind, ">"
                  | "!runtime-object<", runtime-object-kind, { ",", type-use }, ">" ;
```

decimal与signed-decimal都使用最短十进制形式；`-0`、前导零、显式`+`和超出字段类型范围的拼写一律拒绝，因此数值0只有`0`一种canonical text。

`indent` 是当前 operation 相对文件开头的准确缩进：function body 顶层 operation 为八个 ASCII space，每深入一个 nested region 再增加两个；它不是输入可任选的 token。`opcode` 的实际可接受集合恰为第 14 节 `OpcodeSchema` 的 canonical name，词法形状匹配但未注册的名称仍必须拒绝。

`enum-name`只可取字段静态enum domain中已注册的canonical lowercase name，不能接受任意bare-segment。`field-name`与`variant-name`同样由静态record/payload schema限定，不能接收自由名称：named payload一律用record-value并按required field顺序完整打印，tagged union一律用variant-value并按该tag的positional payload顺序打印，未命名product只用至少两项的tuple-value；optional必须显式some/none，空record与空variant payload仍分别打印`{}`与`name()`。parser由当前字段的binary schema决定value production，不得依词法猜类型或省略“默认”字段。Template.NormalizedHirPayload单独派发到normalized-hir-value，禁止降级为bytes。

binary中所有type-use位置仍编码`Ref<Type>`；inline-capability只是text spelling，并且必须结构精确匹配TypeTable中唯一record，不能隐式创建或省略record。inline-capability只允许`place`、`exception`、`runtime_handle`、`runtime_object`四个TypeKind；pointer/reference/slice/array/tuple/nominal/function/function-pointer/interface-reference/runtime-opaque/dependent等一律打印`!tN`。角括号内逗号后没有空格，例如`!place<init,!t9>`与`!runtime-object<task,i32>`。`()`只属于operation零channel语法；一个unit operand/result必须打印`unit`。APInt hex digit数量必须恰为`ceil(BitWidth/4)`，FloatBits必须恰好匹配format位宽。03/06中的`ptr<T>`等写法只作schema metavariable，不是canonical type-use。

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
                             ", decorator_application = ", record-value,
                             ", decorator_application_order_path = ", unsigned-vector,
                             ", source_backed_callsite = ", record-value,
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

Header 唯一形式是 `  header {`、31 个四空格缩进 field line、`  }`；field 按 FieldId 顺序，每行 `name = value` 且没有逗号：

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
    direct_import_bindings = [{from_module_ordinal = N, imported_symbol_key = <digest>, exposure = <private_import|reexport>}, ...]
  }
```

### 15.3 Non-function record envelope

除 Function 外，record固定占一行：四空格、record prefix、一个空格、field dictionary、LF。dictionary唯一形式为`{field = value, field = value}`；空dictionary为`{}`。字段名是本章required payload field的exact `lower_snake_case`，顺序与binary required payload完全相同，所有optional/count/digest/origin字段都打印；binary count不单独打印，vector的`[]`已唯一决定count。variant `KindPayload`打印`payload = variant_name(positional_value, ...)`，positional value顺序与对应payload表一致。该schema dictionary规则优先于AttributeSet自身按AttributeKeyTag/UTF-8 key排序的规则；typed opcode/record payload永远按registry field order，不按字段名重新排序。

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

Template的`normalized_hir_payload`不使用通用Bytes/hex spelling，唯一嵌入形式为：

```text
hir_v1 {root = %hN, nodes = [%h0 = NODE_TAG {FIELD_NAME = FIELD_VALUE, ...} origin(#oN), ...]}
```

nodes严格按binary child-before-parent顺序打印并从`%h0`连续编号，root必须是最后一项；`NODE_TAG`与字段名/顺序来自§10.2对应NormalizedHirNodeTag行，optional显式打印`none|some(...)`，vector保持source order，child引用用`%hN`，外部Type/Constant/Symbol/Template引用用各自canonical sigil，StringId直接内联其canonical `string` literal（text没有StringId sigil或Strings table）。binary-only NodeByteLength不打印并由parser重算；OriginId打印但不进入semantic projection。禁止把整个payload写成`hex"..."`、源码片段、JSON或省略unknown node，因而Staged canonical text可逐node审查并唯一恢复NormalizedHirPayloadV1。

### 15.4 FunctionSignature、entry envelope 与 block

Function record 不使用通用一行 envelope。bodyless declaration 的唯一模板为：

```text
    func @symbol {record_kind = declaration, callable_kind = K, function_kind = K, receiver_kind = K, receiver_type = OPT, entry_identity = K, calling_convention = K, target_abi_tag = DIGEST} (SIGNATURE_PARAMETERS) defaults [DEFAULT_ARGUMENTS] -> RESULT_SPEC {behavior_contracts = [...], behavior_contract_digest = DIGEST, stable_effect_envelope = OPT, stable_effect_envelope_hash = OPT, attributes = OPT, abi_digest = DIGEST, origin = #oN} body none
```

definition与generated_adapter必须具有 body，唯一模板为；reserved_extern_bridge永远没有合法text form：

```text
    func @symbol {record_kind = K, callable_kind = K, function_kind = K, receiver_kind = K, receiver_type = OPT, entry_identity = K, calling_convention = K, target_abi_tag = DIGEST} (SIGNATURE_PARAMETERS) defaults [DEFAULT_ARGUMENTS] -> RESULT_SPEC {behavior_contracts = [...], behavior_contract_digest = DIGEST, stable_effect_envelope = OPT, stable_effect_envelope_hash = OPT, attributes = OPT, abi_digest = DIGEST, origin = #oN} body region(function_body, origin(#oN)) {
      ^bb0(ENTRY_BINDINGS) origin(#oN):
        ...
    }
```

logical signature parameter 与 entry binding 是不同层次。前者来自 Function.Parameters，不分配 ValueId、不带 origin；后者只存在于 body 的 binary entry BlockArguments，并保存 physical Core type、role、ValueId 与 origin。两者的形式固定为：

```ebnf
signature-parameter = "parameter(", decimal, ") ", parameter-mode, " ", type-use ;
default-argument = "none" | "some(", constant-ref, ")" ;
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

SIGNATURE_PARAMETERS、DEFAULT_ARGUMENTS 与 ENTRY_BINDINGS 都以`, `分隔并按index/11.2节顺序；前两者长度必须相等，零参数函数分别打印`()`和`defaults []`，每个default显式打印`none`或`some(#cN)`，不得因absent或等于语言默认值而省略。object参数在signature中打印logical T，在entry中打印owner `!place<rw,T>`；其余parameter的entry type必须等于signature映射结果。非entry block固定打印`^bbN(`、逗号分隔的`%vM: phi(0) T origin(#oK)`、`) origin(#oK):`；nested region entry用`region_argument(index)`替代phi。StableEffectEnvelope以一行nested dictionary打印，字段顺序严格为9.4节payload顺序；set-bound的Any=true打印`any`，否则打印tag-sorted list。

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
eh.match %active [!tN -> ^bbN(%active), ..., catch_all -> ^bbC(%active)] unmatched ^bbU(%active)
eh.end_catch %active -> ^bbN(args)
```

case 按 Constant canonical order；每个完整模板之后仍追加共同 type signature 与 `loc`。

CallPayload callee renderer固定为：direct `@symbol`；indirect `%callee`；virtual `%receiver {slot = @symbol}`；interface `%receiver {method = @symbol}`；reflection `%snapshot [receiver(%vN)]`。nothrow call写`opcode callee([args])`；`call.invoke`写`call.invoke callee_kind = kind callee([args])`；async写`async.call|async.invoke callee_kind = kind callee([args])`，其中方括号内argument list整体可空且方括号不是literal text。callee与args后必须打印`{function_type = !tN, entry_identity = K, calling_convention = ink, target_abi_tag = DIGEST, explicit_argument_count = N}`，字段按CallPayload required顺序中尚未由callee spelling表达的顺序；需要destination时紧跟destination clause，invoke再按顺序写` normal EDGE unwind EDGE`。`reflect.call`写`reflect.call %snapshot receiver(none|%vN) args([%vN, ...]) {expected_function_type = !tN, receiver_present = BOOL, argument_count = N, adapter_nothrow = true}`；ABI写`abi.call|abi.invoke c_callee([args]) {c_function_type = !tN, bridge = @symbol, expected_ink_function_type = !tN, target_abi_tag = DIGEST, explicit_argument_count = N}`。所有这些随后追加共同type signature/loc。printer不得省略“可从当前module碰巧推导”的semantic payload字段；parser按打印值重建binary并由verifier核对冗余一致性。

lookup 固定写 `opcode operands {typed payload fields} found EDGE missing EDGE`；await 固定写 `async.await %task normal EDGE unwind EDGE`；await_copy 在 `%task` 后打印 destination再打印 normal/unwind；continuation invoke固定写`opcode {next_layer = N, function_type = !tN}(args)`，continuation capability由包围region隐式提供且绝不打印为`%continuation` operand，随后按present打印result destination及`normal EDGE unwind EDGE`；completion terminator没有 successor。Successor keyword、role、顺序和 edge sentinel 必须与 11.3/OpcodeSchema 完全一致。

有 region operation 把本应结束该行的 LF 改为 ` region(role, origin(#oN)) {` + LF，内部 block/operation增加两空格，结束行是原 operation indent 加 `}` + LF。region role 必须是 schema 要求的 CoreRegionRoleTag。`decorator.region` 是 v0 唯一 HasRegions opcode；空 region 非法。任何 operation—including nested operation—仍在其 header 的 `{` 前打印共同 type signature 和 `loc`。

### 15.6 Canonical generation order

printer 必须先建立 2.2 节 canonical SCC projection，再依次分配 table ID、function/block/value ID，最后只按本节模板串接 bytes。table/record/field、block successor、attribute/set、effect set 和 capability set 的排序规则均来自本 registry；禁止 pretty-printer 根据行宽换行。两个实现若输入 semantic/nonsemantic record 都相同，必须产生逐 byte 相同文本；golden generator 必须覆盖每个 enum、空/非空 table、每个 PayloadSchema、destination 两种 role、三种 edge argument、全部 BlockArgumentRole 和 nested region。

## 16. Registry conformance 与生成产物

机器可读 registry 至少生成：所有 C++ enum、name/tag 双向表、binary field decoder/encoder、canonical text emitter、opcode arity/type/effect/stage/trait table、unknown-tag rejection、semantic projector 和覆盖 golden。构建时必须断言：同 enum/tag 无重复；所有 OpcodeTag 在 namespace 内唯一；每个 opcode 引用已定义 Shape/Payload/Effect/Stage/Trait；03 Core、05 typed-Core `ct.*` 与 06 runtime canonical opcode name 集合和本表完全相等；StageOpcodeTag 恰为 1—7；所有 required record 字段具有 S/K/N；每个 text field 有唯一 renderer。

Section 16 conformance 还必须生成：summary-first/unique 检查、order path non-empty/odd-length 检查、identity 与 path+ordinal uniqueness、per-path ordinal continuity、四类 dynamic path decoder、identity/protocol/set/interface 四个 digest 重算器，以及 set/interface domain/preimage golden；canonical decoder 不得复用 pending-batch replay deduplicator。

加载顺序固定为 bounds-check container/section/record envelope，验证 canonical integer/string/order，按 registry 解码 required payload，解析全部引用，重算 SCC projection/digest/effect/trait/type/lifecycle facts，最后运行 Staged 或 Closed verifier。任一 unknown required tag、payload 未恰好消费、noncanonical order、hash mismatch 或 text/binary schema disagreement均使 artifact 整体无效，不允许部分执行或降级成 opaque operation。
