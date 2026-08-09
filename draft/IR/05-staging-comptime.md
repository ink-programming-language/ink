# InkIR Staging、`comptime` 与固定点展开

> 状态：已确认设计
>
> 确认日期：2026-08-08
>
> 本文规定 `StagedModule`、`ElaborationPlan`、`NormalizedTemplateTable`、Partial Evaluation、三个执行 world、声明固定点、编译期 capability、可观察效果顺序与缓存政策。普通 Core operation 的执行语义由 [`04-execution-model.md`](./04-execution-model.md) 规定，阶段验证由 [`07-verification-passes.md`](./07-verification-passes.md) 规定。

## 1. 目标与边界

Ink 不建立独立的模板语言或 AST 解释器。普通函数、算术、控制流、内存、调用、异常与生命周期只拥有一套 typed Core InkIR 语义；编译期执行、部分求值和运行时解释通过不同 world 使用同一个执行核心。

同时，Staged IR 不能把尚未选择的源码体伪装成已经完成类型检查的 CFG。以下结构必须延迟 elaboration：

- 未选择的 `comptime if` 或 `comptime match` 分支；
- 依赖开放泛型参数的函数体和声明体；
- 异构 `comptime for` 中需要按每项准确类型重新分析的 body；
- declaration region 中只在选择或展开后才存在的源码声明；
- 尚未请求的泛型函数体实例。

因此本设计采用组合容器，而不是要求 `StagedModule` 中每个对象都是 operation：

```text
StagedModule
|- Typed Core InkIR
|- ElaborationPlan
|- NormalizedTemplateTable
`- committed TypedRegistrations

fixed-point elaboration + partial evaluation
    -> ClosedModule[target]
```

只有 Typed Core InkIR operation 保证已经完成名称解析、类型检查并具有规范类型。`ElaborationPlan` 和 `NormalizedTemplateTable` 是编译阶段数据，不属于 RuntimeWorld 可执行指令集。

## 2. `StagedModule` 的组成

逻辑结构为：

```text
StagedModule = {
    Header,
    TargetKeyAndTargetContextDigest,
    ActiveModuleGraph,
    TypedCoreModule,
    ElaborationPlan,
    NormalizedTemplateTable,
    TypedRegistrations,
    SourceFileTable,
    OriginTable,
    DependencyManifest,
    CapabilityPolicyRevisionAndDigest,
}
```

`TargetContext`、已解析dependency artifacts、`CapabilityPolicy`、comptime handler registry与pass implementation是trusted、immutable、nonserialized execution bindings，不是上述artifact payload。`StagedVerificationContext`/`ClosedVerificationContext`必须按 07 的API把这些对象与Manifest中的key/revision/digest、Section 17 ActiveModuleGraph逐项匹配；Verified token保存不可伪造的只读关联，执行器不再从路径或全局注册表重选同名对象。

其不变量为：

- active module DAG 已经由静态候选 import 选择阶段确定并冻结；
- Typed Core 中每条 operation 的 operand、result、attribute、effect 和 origin 完整；
- 未完成 elaboration 的源码结构只通过稳定 `TemplateId` 出现在模板表中；
- plan node 只能引用本 module、已冻结依赖或规范闭合实例中的稳定对象；
- host pointer、arena address、进程内 `DeclId` 和可写 IR pointer 不得成为 plan 或 template 的持久身份；
- `TargetContext` 在任何 target-dependent 查询或执行前已经确定；
- capability 只授予实际执行到的 effect operation，不按函数名称预授予；
- StagedModule 可以含 meta type、开放声明句柄和 dependent template，但这些内容必须在形成 ClosedModule 前消失。
- `TypedRegistrations` 只包含 fixed-point transaction 已原子提交的强类型记录；仍在执行或验证的 emission 只存在于当前 pending batch，对同轮其他 work item 不可见。artifact 中对应的 canonical section 名为 `ModuleRegistrations`。

只有 `verifyStaged` 成功返回的 `VerifiedStagedModule` 可以交给 fixed-point elaborator。给普通 module 设置一个可修改的 `StageKind` 字段不能替代该能力边界。

## 3. `NormalizedTemplateTable`

### 3.1 模板不是 CST 副本

`NormalizedTemplateTable` 保存重新 elaboration 所需的最小、source-backed 语义模板。它不复制 full-fidelity CST 的 Trivia，也不保存可由用户修改的 AST 或 IR Builder。

每个模板按 10 中央注册表的 required 顺序恰好记录：

```text
NormalizedTemplate = {
    TemplateKind,
    RegionKind,
    SourceDeclarationKey,
    TemplateRolePath,
    SourceFile,
    SourceRange,
    NormalizedHirSchemaVersion,
    NormalizedHirPayload,
    VisibleBindings[],
    LexicalEnvironmentKey,
    Captures[],
    Origin,
    SemanticDigest,
}
```

`TemplateId` 只是 `NormalizedTemplateTable` 的 dense table id，不是上述 record payload。`SourceDeclarationKey` 的 canonical module/source-role/logical-path stable tuple 必须与 `SourceFile` 逐项相等，当前 module version 中该 tuple 唯一；具体 revision 由 `SourceFile.ContentDigest` 绑定。`SourceRange` 必须满足 `Start <= End <= SourceFile.ByteLength`，`NormalizedHirSchemaVersion` 必须匹配 structural registry；不得用另一个同 content digest 文件、artifact-local root id 或旧拼写 `ExplicitCaptures` 替代这些字段。

v0 `TemplateKind` 的闭集为以下八种，numeric tag 以 10 §10.2 为准：

```text
ValueExpression
StatementBlock
TopLevelItems
ClassMemberItems
InterfaceMemberItems
EnumMemberItems
FunctionBody
GenericDeclarationBody
```

`RegionKind` 使用统一集合：

```text
Value
Statement
TopLevel
ClassMember
InterfaceMember
EnumMember
ModuleRegistration
```

其中 `ModuleRegistration` 的 schema 名称为 `module_registration`，只用于承载 decorator module-registration body；它不能退化成普通 `TopLevel` declaration sink。

模板必须已经通过 Tokenizer、Parser 和语法恢复检查。未选择模板仍须词法和语法正确，但不得仅为建立模板而执行依赖名称绑定、成员查找或类型检查。

### 3.2 Normalized HIR 子 schema

`NormalizedHirPayload` 不是可以由实现任意解释的 AST blob。`NormalizedHirSchemaVersion = 1` 时，它必须按以下逻辑子 schema 解码；二进制层仍由 [`10-schema-registry.md`](./10-schema-registry.md) 的 `Template.NormalizedHirPayload : Bytes` 承载，但该字段的每个 byte 都受本节约束：

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
HirNodeVector = Count : U, Items[Count] : HirNodeId
ControlTargetPath = Count : U, StructuralOrdinals[Count] : U
```

`NodeByteLength` 只覆盖本 node 在 `NodeTag` 后的 fields 与 `OriginId`，使 decoder 能在预算内检查准确消费；它不允许跳过未知 tag。`HirNodeId` 是从零开始的 dense index。每个 child node 必须先于 parent，`RootNodeId` 必须是最后一个 node，每个非 root node 必须恰好被一个 node field 引用；因此 v1 payload 是一棵规范后序树，不允许环、共享子树或不可达 node。重复源码片段必须重复编码，不能靠进程内 uniquing 改变语义字节。

v1 的 `NormalizedHirNodeTag` 是闭集：

| Tag | 名称 | `Fields`，按编码顺序 |
| --- | --- | --- |
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

辅助 record 也属于该闭合子 schema：

```text
OptionalTypeId = 0 | TypeId + 1
MatchArm = {
    Pattern : HirNodeId,
    Guard : OptionalHirNodeId,
    BodyTemplateId : U,
    SourceOrdinal : U,
}
MatchArmVector = Count : U, Items[Count] : MatchArm
CatchClause = {
    Pattern : HirNodeId,
    BodyTemplateId : U,
    SourceOrdinal : U,
}
CatchClauseVector = Count : U, Items[Count] : CatchClause
```

`MatchArm.SourceOrdinal`和`CatchClause.SourceOrdinal`必须逐项等于所在vector的zero-based index；它们是源码顺序的冗余完整性编码，不允许跳号、重复或用另一种排序重编号。

所有 `*KindTag`、`*RoleTag` 和 operator tag 都必须使用10 §10.2为`NormalizedHirSchemaVersion = 1`注册的闭合数值表；`TemplateRolePath`的`OwnerSyntaxKindTag`/`ChildRoleTag`也使用同一中央表。实现不得把枚举名、localized spelling、C++ enum ordinal 或 parser 私有数值直接写入 payload。v1 遇到未知 node/tag、非规范 optional、错误 field 数量、错误 node 长度或尾随 byte 必须拒绝，不能保留为 opaque extension。

Normalized HIR 可以包含：

- 已经确定的语法种类、操作符和控制结构；
- 真实 Identifier Token 的规范拼写和 source origin；
- 已经能够非依赖地解析的声明引用；
- 尚待实例化解析的 dependent name、dependent type expression 和 generic argument expression；
- 对外层已知值、运行时 value/place 和声明句柄的显式 capture；
- declaration region 中静态写出的普通声明模板。

它不得包含：

- 由字符串或反射名称构造的新 Identifier；
- 源码中不存在的 import、字段、函数或枚举分支；
- 指向当前 C++ AST、scope、symbol table 或 arena 的裸指针；
- 可以任意创建 operation、block 或声明的用户可见 builder；
- 假装已经解析成功的 dependent member、overload 或 layout；
- 只靠本轮 worklist 顺序才能解释的临时身份。

非依赖代码仍应在最早可行阶段完成绑定和类型检查。只有真正依赖尚未知编译期输入的部分保留为 dependent template；实现不得把所有泛型体无条件退化为无类型 AST。

每个 `identifier` 必须引用 Strings section 中 NFC 规范化、由真实源码 Identifier token 建立的 spelling；合成的展示文本不能成为 identifier。每个 `resolved_reference` 必须指向当前 artifact 中稳定、可验证的 symbol record；每个 `capture_reference` 的 index 必须落在 `ExplicitCaptures` 内，并与其期望语义类型和 access mode 一致。外部 `TemplateId`、`TypeId`、`ConstantId`、`StringId`、`SymbolId` 和 `OriginId` 引用都必须先做 bounds/type/category 检查。HIR 中的 `TemplateId` 只能指向 `TemplateRolePath` 的严格 source-backed descendant；所有 template-reference edge 必须形成 DAG，禁止 self-reference、ancestor edge 和仅靠解码顺序打破的 digest cycle。

decoder 在分配或递归前必须同时检查以下独立预算：`NormalizedHirPayload` 总 byte 数、node 数、单 node byte 数、树深度、单 vector arity、总 child edge 数、identifier/structural-path 总 byte 数、template/type/constant/symbol/origin 引用数以及解码后总内存。每个 `Count` 必须先与剩余 byte 和对应上限交叉检查；`NodeByteLength`、长度加法和 `Id + 1` 必须使用 checked arithmetic。超限是 artifact/资源诊断，不能截断、延迟到 elaboration 或当作 cache miss。

Normalized HIR 的摘要投影为：

```text
NormalizedHirSemanticDigest = H(
    "ink.normalized-hir.v1",
    NormalizedHirSchemaVersion,
    canonical node tree with every OriginId omitted,
    referenced strings/constants/types/symbols/templates by canonical semantic identity)
```

摘要覆盖 node tag、所有其他 fields、vector 顺序、optional presence、结构路径和语言语法 registry revision；不覆盖 `NodeByteLength`、dense `HirNodeId` 数字、artifact-local table index 或 source range。计算时按规范树结构递归投影，不能直接 hash artifact-local payload bytes。`NormalizedTemplate.SemanticDigest` 必须进一步覆盖 `TemplateKind`、`RegionKind`、`SourceDeclarationKey`、`TemplateRolePath`、`SourceFileContentDigest`、`NormalizedHirSemanticDigest`、`LexicalEnvironmentKey` 以及按序投影的 captures；capture 的 `OriginId` 同样不进入语义摘要。origin/debug table 虽不进入摘要，仍必须绑定同一 module content digest，并在 cache materialization 时验证。exact domain 与 ordered preimage 以 10 §2.3 为准。

### 3.3 显式 capture

模板对外层环境的依赖必须显式编码为 capture：

```text
TemplateCapture = {
    CaptureKind,
    TaggedCaptureSource,
    ExpectedSemanticType,
    AccessMode,
    OriginId,
}
```

`CaptureKind` 可以是 compile-time value、runtime SSA value、place、type、declaration handle、lexical declaration 或 target/config query；`TaggedCaptureSource` 必须是 10 §10.2 的闭合 constant/PlanResult/CoreValue/type/declaration/lexical-binding/target-config/instance-argument variant之一，而不是 opaque key。PlanResult source携带producer和result index，CoreValue携带owner/RegionPath/ValueId，lexical binding携带VisibleBindings index，因此选中后可以机械重建值与lazy dependency。capture 顺序按照模板中建立的规范首次使用顺序固定；序列化和 hash 不得依赖 map 迭代顺序。

运行时 SSA value 或 place 可以被 `stage.select_if`、`stage.select_match`、`stage.expand_for` 或 `stage.expand_while` 实际选中的 body 捕获，因为该 body 可以残留普通运行时代码。相同值进入 `stage.force_value` 或 `stage.force_block` 时若成为必经依赖，则强制求值失败。

### 3.4 模板身份

`TemplateRolePath` 是 declaration 内稳定、不可为空的结构角色路径：

```text
TemplateRolePath = StepCount : U (must be greater than zero), Steps[StepCount] : TemplateRoleStep
TemplateRoleStep = {
    OwnerSyntaxKindTag : U,
    ChildRoleTag : U,
    ChildOrdinal : U,
}
```

`OwnerSyntaxKindTag` 和 `ChildRoleTag` 来自 `SourceDeclarationKey.SourceStructuralSchemaVersion` 对应的语法 registry；Staged中该version必须等于`NormalizedHirSchemaVersion`。`ChildOrdinal` 只在同一 owner、同一 role 的 source-backed children 内计数。路径不得使用 source offset、AST 地址、parser arena ID、当前 `TemplateId` 或遍历 map 的顺序。decoder 必须验证它准确解析到 `SourceDeclarationKey` 内当前模板的语法角色，且同一 declaration 内没有重复路径。

`TemplateId` 只是当前 artifact 内的紧凑引用。跨进程缓存中的规范模板身份为：

```text
canonical module identity
+ module content digest
+ source declaration structural path
+ template role/path inside that declaration
+ normalized template semantic digest
```

物理绝对路径、当前 `TemplateId` 数字、指针地址和 fixed-point round 不进入该身份。

## 4. `ElaborationPlan`

### 4.1 Plan node 模型

`ElaborationPlan` 是具有显式依赖边的有向图。每个 node 的 required payload 精确引用 10 §10.3；摘要如下，不得再缩减为第三套 schema：

```text
PlanNode = {
    PlanNodeId,
    StageOpcode,
    Inputs[],
    TemplateRefs[],
    ResultTypes[],
    Sink {Kind, OwnerSymbol, RegionStructuralPath, SourceBackedAnchor : SourceBackedAnchorIdentityPayload, InsertionPosition},
    ParentElaborationContext {ParentCanonicalWorkKey?, EnclosingInstance : InstanceIdentityPayload?, DecoratorApplication : DecoratorApplicationIdentityPayload?, DynamicControlPath, Digest},
    RequiredCapabilities[],
    DependencyPlanNodes[],
    CanonicalWorkKey,
    OriginId,
}
```

plan dependency 必须无非法环。无环检查使用10 §10.3的统一`WorkKeyDependencyGraph`：`DependencyPlanNodes`中的eager PlanResult producer、present `ParentCanonicalWorkKey` parent，以及所有`TemplateRefs.SemanticCaptureProjection`中直接出现的PlanResult producer三类边取并集后整体做dependency-first stable Kahn，不能分别无环却在交叉后形成hash环。最后一类`TemplateKeyDependencies`只形成可重算的key dependency；未选template绝不因此执行、观察值或产生effect，只有template实际activation后才把对应capture producer加入child执行依赖。普通函数递归不表现为 plan graph 环；泛型实例递归通过实例状态机和 provisional declaration 处理。

`stage.*` 名称是 plan schema 的 opcode，不是可以出现在 Closed CFG 中的 RuntimeWorld operation。selector、普通函数调用和 body 中已经 elaboration 的代码仍由 typed Core operation 执行。

### 4.2 `SinkKind`

输出 sink 使用以下闭合集合：

```text
Value
Statement
TopLevelDeclaration
ClassMember
InterfaceMember
EnumMember
ModuleRegistration
```

sink 只接收对应区域允许的输出：

- Value 接收一个 typed value；
- Statement 接收正常 lowering 后的 statement CFG fragment；
- TopLevelDeclaration 接收顶层声明，不接收普通表达式语句；
- ClassMember 接收类字段、函数和嵌套类型；
- InterfaceMember 接收接口区域允许的成员；
- EnumMember 接收枚举分支或该区域未来明确允许的成员。
- ModuleRegistration 接收一个已验证的 immutable typed registration record；它不向 CFG、声明作用域或用户 module hook 插入代码。

shape 通常已经由 Parser 的 `RegionKind` 保证，但 Staged verifier 必须再次检查，防止内部构造错误绕过区域边界。

### 4.3 Plan input、依赖边和插入锚点

`Inputs[]` 不是无类型 variant，也不能保存进程内 IR pointer。v0 使用以下闭合集合：

```text
PlanInput =
    Constant(ConstantId)
  | PlanResult(PlanNodeId, ResultIndex)
  | CoreValue(OwnerSymbol, RegionPath, ValueId)
  | TemplateCapture(TemplateId, CaptureIndex)
  | InstanceArgument(InstanceIdentityPayload, ArgumentIndex)
```

每个 input 同时记录准确 semantic type。`PlanResult` 自动形成从 producer 到 consumer 的 dependency edge，producer 必须唯一且 result index 合法。`CoreValue` 的 `%vN` 只在 owner function/region 内有意义，因而序列化时必须带 `OwnerSymbol + RegionPath`；只保存裸 `ValueId` 不合法。该引用还必须满足 Typed Core 的 dominance、place lifetime 和 stage-capture 规则。

Known selector/condition/iterable、`stage.force_value` 和 `stage.force_block` 的必经输入不得是 Residual `CoreValue`。`stage.select_if`、`stage.select_match`、`stage.expand_for` 与 `stage.expand_while` 实际选中或执行后允许 residual code 的 body 可以通过显式 `TemplateCapture` 捕获运行时 value/place；这不使 selector、condition 或 iterable 本身变为 runtime 值。

序列化 parent PlanNode 的 `Inputs` 禁止 `TemplateCapture` variant；它只在某个 TemplateRef 实际选中并建立 child work item 时由 resolver 按 `(TemplateId, CaptureIndex)` 物化。若 TaggedCaptureSource 是 PlanResult，准确 producer/index 只进入 child 的 direct dependency vector并先完成该producer；未选template不解析source、不建边、不调度producer也不触发effect。其他source按10 §10.2重建为canonical实体、Known值或经scope/dominance/lifetime验证的Residual CoreValue/place。

Statement 或 declaration sink 的插入位置使用：

```text
InsertionAnchor = {
    OwnerSymbol,
    RegionStructuralPath,
    SourceBackedAnchorIdentityPayload,
    Position,                 // before, after, replace 或 append
}
```

decoder/verifier 可以在进程内为已解析的 sink 派生 canonical `BlockId/OperationOrdinal` 快速索引，并必须确认其解析到同一结构化SourceBackedAnchor及重算key；该payload内的OwnerSymbol/RegionStructuralPath还必须与InsertionAnchor外层字段相等。快速索引不属于PlanNode required payload、不得序列化或进入digest。work key和跨进程cache identity使用完整source-backed anchor preimage，不使用优化前对象地址、临时block指针或当前容器下标。

## 5. Stage opcode

### 5.1 总表

v0 的 plan opcode 为：

| Opcode | 强制 Known 的部分 | 输出行为 |
| --- | --- | --- |
| `stage.force_value` | 整个值表达式 | 向 Value sink 提交一个 Known 结果 |
| `stage.force_block` | 整个 statement block | 完全在 ComptimeWorld 执行，不产生 residual CFG |
| `stage.select_if` | 条件 | 只 elaboration 选中的模板 |
| `stage.select_match` | 被匹配值与 arm 选择 | 只 elaboration 第一个匹配模板 |
| `stage.expand_for` | 迭代源、长度与每项身份 | 按源顺序逐轮重新 elaboration body |
| `stage.expand_while` | 每轮条件与展开决定 | 按轮次重复 elaboration body |
| `stage.instantiate` | 泛型声明与全部编译期实参 | 请求或复用一个闭合实例 |

这些 node 没有用户可取得的函数地址，也不形成运行时函数类型或调用约定。

`register_module_item` 不在上表中伪装成无条件 plan node。它的执行取决于普通 comptime CFG、函数调用、分支和循环是否真正到达，因此使用 5.9 节的 Staged-only typed Core operation；结构化 stage node 仍只负责选择或展开承载该 operation 的代码。

### 5.2 `stage.force_value`

`stage.force_value` 对应 `comptime expression` 以及泛型实参、attribute/decorator 实参等天然要求编译期值的位置。

执行过程为：

```text
elaborate selected value template
-> execute in ComptimeWorld
-> require one Known typed result
-> canonicalize result
-> submit to Value sink
```

要求：

- 结果数量必须准确为一；`void` 位置由其消费者另行规定，不能伪造 SSA `void` 值；
- 必经路径不得读取 Residual value/place；
- 结果可以是 `type`、声明句柄、编译期 tuple 或其他 meta value，只要消费位置允许；
- 若结果最终进入 runtime，必须能够转换为普通可表示常量，且不能含 host/comptime-only 句柄；
- trap、未处理异常、缺少 capability、预算耗尽和 Residual 依赖都使强制求值失败。

示意：

```text
#p0 = plan_node {stage_opcode = stage.force_value, inputs = [], template_refs = [#tmpl0], result_types = [i64], sink = value(@owner, [0], sha256"0000000000000000000000000000000000000000000000000000000000000000", replace), parent_elaboration_context = sha256"0000000000000000000000000000000000000000000000000000000000000000", required_capabilities = [], dependency_plan_nodes = [], canonical_work_key = sha256"0000000000000000000000000000000000000000000000000000000000000000", origin = #o7}
```

该文本使用PlanNode的canonical record envelope；digest与sink positional values仅为格式示例，真实artifact必须按10的公式重算。它不是一条`%v = ...` Core instruction。

### 5.3 `stage.force_block`

`stage.force_block` 对应 StatementRegion 的 `comptime { ... }`。block 中的绑定、赋值、普通调用、`try`、`defer` 和 `throw` 继承 ComptimeWorld；嵌套函数 activation 内部的 `return` 以及模板内部循环的 `break`/`continue` 仍按其正常控制目标执行。

要求：

- 整个实际执行路径必须在编译期完成；
- 不允许输出 residual runtime operation；
- 局部对象、虚拟目标内存和 cleanup 必须按普通语言语义执行；
- 离开 block 后仍被使用的结果必须是可合法逃逸的 Known value；
- comptime allocation 地址、host handle 和临时引用不得逃逸；
- `return` 不得以包围该模板的源码函数为 target，`break`/`continue` 不得以模板外的循环或 label 为 target；`ControlTargetPath` 必须完全解析在当前模板或其内部新建的函数 activation 内；
- 任何试图从模板逃逸的 `return`、`break` 或 `continue` 都是 staging 诊断，不能把 surrounding runtime activation 当作 ComptimeWorld continuation，也不能隐式 residualize 该 control transfer；
- 未执行路径不触发 capability 检查或效果。

declaration region 中表面相同的 `comptime { declarations }` 不是“执行每个函数体”。它无条件选择其中静态写出的声明并提交给当前 declaration sink；每个声明自己的 initializer、函数体和阶段要求仍按正常规则 elaboration。

### 5.4 `stage.select_if`

`stage.select_if` 要求条件得到 Known `bool`。只有选中的 source-backed template 进入后续名称绑定、类型检查和 lowering；未选分支除已经完成的词法、语法检查外不产生语义诊断、效果或 IR。

选中 body 不自动变成完全编译期 block。Statement sink 中依赖运行时输入的普通代码可以由 ResidualizeWorld 生成 residual CFG；declaration sink 则提交选中的源码声明模板。

缺少 `else` 等价于选中空模板，不产生隐式 unit 或 `void` value。

### 5.5 `stage.select_match`

`stage.select_match` 先将被匹配表达式准确求值一次，然后按源码 arm 顺序选择第一个匹配项。被匹配值、判别和 pattern 绑定必须在编译期确定。

每个 arm template 保持准确 RegionKind。未选 arm 不执行绑定、类型检查、effect 或 layout 查询。穷尽性、重复 pattern 和普通语法规则在其对应语义阶段验证，不能由 plan 的默认 successor 静默掩盖。

### 5.6 `stage.expand_for`

`stage.expand_for` 要求迭代源、长度、顺序和每项值在编译期已知。异构 tuple 或运行时参数包的每轮 body 使用该位置准确的元素类型重新 elaboration：

```text
iteration 0: Element : type
iteration 1: Element : ptrsize
iteration 2: Element : GenericTypeDecl
```

它不是先在一个擦除公共类型下检查一次再复制 CFG。

展开规则为：

- 按迭代源语义顺序处理；
- 每轮建立新的 lexical binding 和 expansion origin；
- Statement sink 按轮次顺序拼接 residual CFG；
- declaration sink 为每轮产生独立暂定声明；
- 某轮失败使整个 work item 失败，不能静默丢弃该轮；
- 空迭代源执行零轮；
- 每轮和总生成量都计入资源预算。

### 5.7 `stage.expand_while`

`stage.expand_while` 在每轮开始前于 ComptimeWorld 求值条件。条件必须是 Known `bool`；`false` 终止，`true` elaboration 一轮 body，然后重新求值条件。

body 可以更新 comptime local state，也可以在 Statement sink 中产生 residual runtime code。每轮条件、状态变化、body 效果和输出顺序都遵守源码抽象执行顺序。

该 node 必须同时受以下预算限制：

- condition evaluation fuel；
- 最大迭代次数；
- 最大生成声明数量；
- 最大 residual operation 数量；
- 最大 comptime memory；
- 最大 effect 次数。

达到预算是编译诊断，不等价于条件为 `false`。

### 5.8 `stage.instantiate`

`stage.instantiate` 接收唯一开放泛型声明和全部规范化编译期实参。绑定顺序为：

```text
bind explicit positional arguments
-> evaluate omitted trailing defaults left to right
-> bind final parameter pack
-> canonicalize every argument
-> form InstanceIdentity
-> request or reuse the instance
```

它不执行泛型实参推导、偏应用或 SFINAE。候选声明头必须先按既有重载规则闭合；只有唯一选中候选的函数体进入实例化。

请求相同 `InstanceIdentity` 时复用同一个进行中或已完成实例。相同机器码不能使两个语义身份不同的闭合类型相等。

revision 1不提供跨module template/code staging interface：`stage.instantiate`的开放GenericDeclarationSymbol必须是当前Manifest module的source-backed declaration并携带10 §9.2的GenericDeclarationProvenance marker，其全部Template也必须属于当前module；generated open generic与active dependency open generic都不能作为candidate。dependency只能export provider已生成并验证的closed type/stable function instance，consumer把它作为普通typed direct import使用，不再执行stage.instantiate。decorator application同样要求DecoratorDeclarationSymbol属于当前module；dependency decorator、其body/template或“仅公开signature”的decorator都不能在consumer执行。未来若开放generated/cross-module template能力，必须新增显式ComptimeExecutionExports及其完整body/template/dependency/capability closure，不能复用Runtime Exports猜测。

### 5.9 `ct.register_module_item`

源码 compiler builtin `register_module_item(value)` lowering 为显式 Staged-only typed Core operation `ct.register_module_item`，不是 RuntimeWorld call、用户可取地址函数或隐式编译器回调。它只允许在 active decorator application 的 `ParentElaborationContext` 内由 ComptimeWorld 真正执行；普通 comptime CFG 未到达该 operation 时不产生记录。该 operation 不可 residualize，形成 ClosedModule 前必须被执行并消除，或因所在模板未选择而随模板消失。

operation 有无结果的两个 operand schema：单标量或 unit 直接传 `T`；address-only 值传只读 `!place<ro,T>`。opcode payload还保存不可由用户指定且内嵌key重算的完整`SourceBackedCallsiteIdentityPayload`。执行器要求 operand 是完整 Known 值，把它按准确逻辑类型 canonicalize 到 ConstantTable，然后向当前 work item 的 pending `ModuleRegistration` sink 提交恰好一条记录；它不把 place、ComptimeWorld allocation 地址或宿主对象保存进记录。

提交记录至少固定：

```text
ModuleRegistrationRecord = {
    RegistrationIdentity,
    RegistrationType,
    RegistrationValueConstant,
    ProducerSymbol,
    DecoratorApplicationIdentityPayload,
    DecoratorApplicationOrderPath,
    SourceBackedCallsiteIdentityPayload,
    DynamicControlPath,
    EmissionOrdinal,
    ProtocolSchemaDigest,
    OriginId,
}
```

`ProducerSymbol` 是本次 decorator application 的最终声明身份，不是 decorator 定义函数本身；注册值中的 `function.entry` 必须解析到最终装饰后的 stable entry。`DynamicControlPath` 是下列结构化 tagged step 序列，不是 opaque hash：

```text
Expansion(ExpansionContextIdentityPayload)
Call(CallSiteIdentityPayload, InvocationOrdinal)
Branch(ControlNodeIdentityPayload, ArmOrdinal)
Loop(IterationIdentityPayload)
```

call/recursion 在同一 site 的多次动态进入由当前 work item 源码执行顺序的 `InvocationOrdinal` 区分；相等 loop element仍由 `IterationOrdinal` 区分。`DecoratorApplicationOrderPath` 是非空奇数长度 `Vec<U>`：root application 是按 [08](./08-text-binary-format.md) 的唯一 source-file 顺序和文件内 lexical decorator-application traversal 得到的 `[RootApplicationOrdinal]`；生成的 child 是 `ParentPath ++ [ParentSemanticOutputOrdinal, ChildLocalApplicationOrdinal]`。`ParentSemanticOutputOrdinal` 统一计数 parent transaction 的全部 ordered semantic-output event，不是 registration-only 计数；路径按无符号数字典序比较，短前缀先于其 child。`EmissionOrdinal` 则是同一 active application 内每次实际到达 `ct.register_module_item` 时从零连续递增的 application-wide 序号。batch 按 `(DecoratorApplicationOrderPath, EmissionOrdinal, RegistrationIdentity)` 排序；不得把 D32 hash字节序冒充程序顺序。线程、fixed-point round、work queue 和容器插入顺序都不得进入，cache replay必须重现相同路径和两套 ordinal。

语义身份为：

```text
RegistrationIdentity = H(
    "ink.module-registration.v0",
    CanonicalModuleIdentity,
    ModuleContentDigest,
    ProducerSymbol.SymbolKey,
    DecoratorApplicationIdentityPayload.DecoratorApplicationKey,
    DecoratorApplicationOrderPath,
    SourceBackedCallsiteIdentityPayload.SourceBackedCallsiteKey,
    DynamicControlPath,
    EmissionOrdinal)
```

相同 artifact/work item 内相同 identity 的重放只有在完整 logical record（包括 canonical Origin structure）逐字段相同时才能由 pending-batch commit 在writer之前去重；任何差异都是 determinism/compiler error。canonical section本身不得含重复identity或重复 `(DecoratorApplicationOrderPath, EmissionOrdinal)`，decoder不负责规范化。identity 是 module-content/version-local：value变化即使不作为独立 identity字段，也通常会经 `ModuleContentDigest` 产生新 identity。跨版本替换依靠不可伪造 version owner 下整套 registration set 的原子 publish/retire，不承诺同一 lexical site 保持相同 RegistrationIdentity。

`StaticRegistrationEncodable(T, Constant)` 是中央 verifier 派生谓词，而不是用户可随意声明的 attribute。它要求 closed、`LayoutComplete` 的准确 typed constant 具有只读 frozen module-image encoding，不需要在安装或撤销时运行用户 constructor、destructor 或 rollback。v0 闭包允许 scalar/unit、受支持 enum/tuple/array/nominal aggregate、受控 symbol relocation，以及中央 registry 明确支持的 built-in String frozen encoding；String 的 module-owned immutable bytes/length 表示必须布局合法且没有独立释放责任。用户 destructor、资源字段、runtime/meta/host handle、runtime object/opaque object、place、exception、checked reference/interface/no-escape 值和宿主/comptime pointer 默认拒绝，除非中央 registry 明确给出无用户代码、无释放责任的准确 frozen encoding。address space 0 raw pointer可以是canonical null；非null raw pointer与全部function pointer只能由获准symbol relocation产生。函数目标必须是stable entry，其他static relocation的存活期必须覆盖所属module version，version-local body裸地址非法。每个 relocation target identity、`FrozenEncodingDescriptor` selector 及它们递归可达的 Type/Constant/Symbol/producer/callsite/decorator identity 还必须通过 10 的 `RegistrationContextIndependent` 谓词；任一 `versioned_owner|module_version_root|versioned_entity_owner` parent 或 transitive `ModuleVersionContextKey` 都使整条 registration 非法，不得只以“存活期足够长”放行。

该 operation 的 effect 是 `DeclarationSink(module_registration)`；address-only variant 另有 `ReadMemory(typed)`，schema trait 为 `OrderedSemanticEmit`。它不可删除、CSE、复制、推测或跨另一 semantic emit 重排，但该 trait本身不使结果不可缓存。内部 module semantic sink authority 只在上述 decorator context 授予，不是可配置的 host 写权限。记录 emission 是可缓存、可事务重放的 semantic output，不等同于 `build.log`、`fs.write` 或用户可观察宿主写；operand 求值实际发生的其他效果仍按其自身依赖与 cacheability 分类。

## 6. Partial value 与 Residualization

### 6.1 值域

ResidualizeWorld 使用：

```text
PartialValue =
    Known(CtValue)
  | Residual(ValueId)
  | Aggregate(PartialValue...)
```

本文中的 `Residual(ValueId)` 对应既有语言议题所称的 `Runtime(ir_value)`。名称变化只强调该值已经或将要在 residual Core InkIR 中存在，不改变语义。

`Error` 和 `Unreachable` 是执行状态，不是可以被普通 operation 消费、存储或序列化的值。InkIR 不提供用任意值继续执行的 `undef` 或 `poison`。

`Aggregate(PartialValue...)` 是 partial evaluator 内部的结构化抽象值，不是 Core InkIR aggregate SSA value。它在进入 residual IR 时必须按类型规则拆成合法标量，或直接物化到最终 place/result destination；address-only、noncopyable 或需要稳定地址的聚合绝不能因为 partial evaluation 而获得普通 SSA 表示。

`CtValue` 至少可以表示：

- 准确 bit width 和 bit pattern 的整数；
- `bool`；
- 准确 format、bits 和 TargetContext float mode 的浮点；
- 普通 runtime-representable aggregate；
- comptime sequence 和异构 tuple；
- `type`、`GenericDecl`、`FunctionDecl`、Identifier 和结构化反射结果；
- ComptimeWorld 虚拟目标内存中的受控 place/reference；
- capability handler 返回、但被明确标记为不可残留的 host resource value。

### 6.2 普通 operation 的规则

每个 Core opcode schema 必须定义 residualization rule。一般规则为：

- 全部 operand Known、operation 可在当前 world 执行且不会错误移动运行时效果时，计算 Known 结果；
- 任一必要 operand Residual 时，生成同语义 typed Core operation；
- aggregate 可以逐元素混合 Known 和 Residual，只有最终消费要求完整物化时才整体 materialize；
- target-dependent operation 只有在 TargetContext 完整时才能折叠；
- MayTrap、MayUnwind、PdbBoundary 和其他不可消去 effect operation 不能仅因结果未使用而提前执行、删除或推测；`TargetDependent` 本身只约束 TargetContext identity，不单独禁止合法推测；
- 编译期专用值不能作为 residual operand 写入 Closed IR。

示例：

```text
Known(i32 10) + Known(i32 20) -> Known(i32 30)
Residual(%v0) + Known(i32 10) -> residual `arith.add %v0, const.int 10`
```

### 6.3 控制流

普通 `cf.cond_br` 的条件 Known 时只 residualize 被选择 successor。条件 Residual 时保留 runtime branch，并分别 residualize 两条可达路径；不得执行任一分支中的 RuntimeEffect。

“Residual control” 包括 selector、循环继续条件、异常 successor、间接调用目标或其他控制决定含 Residual 的所有区域。在其支配的任何可能路径上，编译器不得执行、复制、延迟或推测执行任何 comptime-only operation，也不得发生任何编译期可观察效果；tracked read、declaration sink、module-registration emission、diagnostic/build log 和 host write 都在禁止之列。普通 RuntimeWorld effect 可以作为已经类型检查的 Core operation 原样 residualize，但绝不能因另一 operand Known 而在编译期发生。若源码语义要求在该 Residual control 下执行 comptime-only operation 或编译期可观察效果，当前 staging 请求必须报错，而不是选择一条路径、执行两条路径或把效果丢弃。

`stage.select_if` 与普通 `cf.cond_br` 不可混同：前者的 selector 必须 Known，而且未选 template 不进入语义分析；后者是已经完成类型检查的普通运行时 CFG。

普通循环的条件 Residual 时保留 CFG 回边。只有 `stage.expand_for/while` 可以按编译期已知迭代结构复制模板或 residual body。

### 6.4 调用与副作用阶段

同一个普通函数可以：

- 在 ComptimeWorld 完整执行；
- 在 ResidualizeWorld 以混合 Known/Residual 输入部分求值；
- 在 RuntimeWorld 解释执行 Closed body。

函数没有永久的 `comptime` bit。能否编译期执行由本次参数、实际路径、TargetContext、capability 和 handler 决定。

普通运行时效果不得仅因参数 Known 被移动到编译期。例如普通函数中的 `stdio.write` 默认保留为 runtime effect；只有源码把对应调用放入强制 comptime 上下文并且 capability policy 允许时，ComptimeWorld 才执行它。

## 7. 一个执行核心、三个 World

### 7.1 共享执行核心

执行核心共同负责：

- operation schema 与控制流；
- activation 和调用栈；
- 目标整数、浮点、指针和内存语义；
- normal/unwind、trap/fatal 与 cleanup；
- 资源预算和取消检查；
- source-order effect dispatch；
- origin context 与编译期调用栈。

world 只替换 value domain、memory implementation、effect handler、capability 和 residual emitter，不能重写 `arith.add`、`mem.load`、`call.direct` 等 operation 的语言含义。

### 7.2 `ComptimeWorld`

`ComptimeWorld`：

- 只接受 concrete/Known 输入；
- 支持 meta value、编译期 sequence、declaration handle 和反射结果；
- 使用遵守 TargetContext 的虚拟目标内存，不暴露宿主 C++ 地址；
- 在实际到达 effect operation 时检查 capability；
- 不生成 residual operation；
- 遇到运行时未知依赖、无 handler effect 或不可残留 host value 时产生结构化诊断；
- 在强制边界内执行普通调用、递归、循环、异常和生命周期语义。

### 7.3 `ResidualizeWorld`

`ResidualizeWorld`：

- 使用 Known/Residual partial value；
- 执行允许且阶段正确的 Known 纯计算；
- 为未知或必须留在运行时的计算生成 typed Core InkIR；
- 保持 runtime effect、异常、trap、PDB 和 source order；
- 可以调用 ComptimeWorld 完成强制 selector 或 meta operation；
- 不允许 host/comptime-only value 进入 residual module。

ResidualizeWorld 的输出仍然是未验证 Core module。只有 fixed point 完成、全部 stage 内容消除并通过 Closed verifier 后才形成 `VerifiedClosedModule<TargetKey>`。

### 7.4 `RuntimeWorld`

`RuntimeWorld`：

- 只接收 `VerifiedClosedModule<TargetKey>`；
- 不认识 ElaborationPlan、NormalizedTemplateTable、Known/Residual 或 meta value；
- 使用 TargetWorld 内存和 runtime effect handler；
- 不能请求新泛型实例、生成声明或修改待编译 module；
- 与 LLVM AOT 在相同 TargetKey 和外部 effect trace 下保持语言可观察等价。

## 8. Effect 与 Capability

### 8.1 Effect 在实际操作处检查

effect 属性由 opcode schema 推导。调用者不得根据函数名称、源码属性或缓存 summary 猜测整个调用是否可在编译期执行。

v0 的可配置 `CapabilityTag` 对应的典型 policy key 为：

```text
build.log
fs.read
fs.write
environment.read
network.request
clock.now
random.read
process.spawn
```

`target.extern_call` 和 `inline_assembly` 不是 v0 `CapabilityTag`，也不会进入 capability context、policy digest 的授权集合或 handler dispatch。ComptimeWorld 遇到目标 extern call 或 inline assembly 一律拒绝；配置文件、命令行或插件不能授权它们。未来若要支持，必须通过新的 IR semantics/schema revision 定义可验证 ABI、target 状态、效果和 cache 规则，不能复用未知 capability 字符串绕过本条。

target triple、layout、PDB 和语言内建反射查询由 TargetContext 或编译器语义 handler 提供，不读取宿主平台偶然状态。

未选中的分支不检查 capability。执行到无权限、无 handler 或实参不满足 handler 要求的 effect 时，在该 operation origin 报错，并附完整 comptime 调用栈和实例化链。

### 8.2 默认 capability policy

v0 默认允许：

- Pure operation；
- comptime local state 和虚拟目标内存；
- TargetContext 查询；
- 编译器内部类型、声明和结构反射；
- 显式授权并记录依赖的只读输入。

v0 默认拒绝以下已注册 capability：

- 网络；
- 进程创建；
- 未跟踪环境读取；
- 时钟和随机数；
- 没有显式构建权限的文件写入。

构建配置只能显式增加 schema registry 已注册的 `CapabilityTag`，且 policy 本身及 handler revision 必须进入 cache identity。授权不表示结果可缓存；它尤其不能授权 `target.extern_call` 或 `inline_assembly`。

### 8.3 外部资源不得逃逸

host file handle、socket、process handle、host pointer、ComptimeWorld allocation address 和其他只在编译器进程中有意义的值：

- 不能进入 Closed constant；
- 不能成为运行时 global、field、parameter 或 return；
- 不能跨 `extern` ABI；
- 不能作为可在另一进程恢复的编译期实参；
- 不能通过 aggregate 或 tuple 间接逃逸。

允许 handler 返回的普通数据必须转换为有准确 Ink 类型的 CtValue，并记录其依赖或不可复现性。

## 9. 可观察效果、确定性与缓存

### 9.1 `BuildInputSnapshot` 与输出隔离

每次 Staged build 在进入 fixed point 前必须冻结一个只读 `BuildInputSnapshot`：

```text
BuildInputSnapshot = {
    SnapshotIdentity,
    SnapshotDigest,
    CanonicalModuleInputs,
    TrackedFileInputs,
    TrackedConfigInputs,
    TrackedEnvironmentInputs,
    TrackedDirectoryInputs,
    HandlerRevisionDigest,
}
```

snapshot entry 使用规范 resource identity、存在性/类型、content 或 listing bytes、metadata policy 和 digest 表示；不能只保存稍后重新打开 live host resource 的路径。允许实现惰性 materialize 大文件，但第一次读取前必须取得稳定 snapshot token，读取前后验证同一版本，并把准确 bytes/digest 固定到当前 snapshot；无法证明稳定时读取失败，不能接受竞态结果。

所有可缓存 tracked-read handler 只能查询当前 `BuildInputSnapshot`，并把 snapshot entry identity、observed digest 和读取范围写入 `DependencyManifest`。handler 不得绕过 snapshot 重新读取 live filesystem、process environment、config store 或目录；untracked read 即使已获 capability 也不可缓存，并仍受 ordered-lane 规则约束。revision 1没有`UntrackedObservationDigest`或build-instance nonce；因此任一work item实际执行network/process/clock/random或其他untracked read后，整次module elaboration只能用于当前编译器进程内的诊断/一次性试算，`closeAndVerify`必须拒绝生成、序列化、装载或hot-reload publish Closed artifact。仅“跳过FinalModuleKey缓存”不足以避免两个不同观察共享同一ModuleVersionContextKey。

host write 只能写入与 `BuildInputSnapshot` 隔离的 `BuildOutputNamespace`。当前 build 任一轮已计划、正在写或已写的 output identity 都不得作为 tracked/untracked read 的来源；若 read identity 与当前 build 的 output identity 规范化后 alias，则 staging 诊断并终止该 work item。该 output 不会在后续 fixed-point round 注入当前 snapshot，因而不存在同一 build 的 read-your-write；只有一个独立的后续 build 可以把已发布输出重新冻结为新的输入。临时文件、symlink、大小写折叠和 `..` 不能绕过 identity alias 检查。

`SnapshotIdentity` 和 `SnapshotDigest` 绑定本次 work-item execution record，但不作为把所有外部输入粗粒度塞入 base cache key 的替代品。候选 cache entry 的 `DependencyManifest` 必须针对当前 snapshot 逐项复验，只有实际 tracked-read 的 entry identity、observed digest 和读取范围参与该结果的依赖投影；未读取 snapshot entry 的变化不应强制 miss。host output path、临时 staging path 和写入完成时间不进入输入 snapshot identity。

### 9.2 Cacheability 分类

编译期执行按实际走到的 effect 分类：

| 实际效果 | v0 缓存政策 |
| --- | --- |
| 纯计算、TargetContext 查询 | 可缓存 |
| `fs.read`、tracked config/environment read | 记录 DependencyManifest 后可缓存 |
| declaration selection/expansion | 可缓存；它不是宿主写效果 |
| typed module registration emission | 可缓存；record 作为 pending semantic batch output 重放 |
| residual runtime effect | 可缓存；该效果没有在编译期发生 |
| `build.log`、主动 warning 或其他可观察编译输出 | 本次 work item 不可缓存 |
| `fs.write` | 本次 work item 不可缓存 |
| network、process、clock、random | 默认拒绝；即使授权也不可缓存 |
| target extern call、inline assembly execution | v0 非法；没有 capability、handler 或 cacheability 分类 |

cacheability 是本次动态执行路径的属性，不是函数声明的永久属性。未执行分支中的写 effect 不会使当前结果不可缓存。

registration emission有两个不同缓存层。generic/instance IR cache只保存仍含 `ct.register_module_item` 的 typed Core，并在每个新的动态application/call context中重新真实执行该 operation；它不得把某次执行产生的具体record当作实例本身的一部分。只有 work-item result cache 才能重放pending registration output，而且其key必须在 `InstanceCacheKey` 之外准确包含 `DecoratorApplicationKey`、`DecoratorApplicationOrderPath`、当前 `DynamicControlPath`/invocation prefix、起始application-wide `EmissionOrdinal` 和起始 `ParentSemanticOutputOrdinal`。重放必须恢复同一批record并推进两个cursor；上下文任一项不同就必须重新执行，不能吞掉另一callsite、递归帧或loop iteration的emission。普通函数memo若不采用这一context-complete replay key，就必须把`OrderedSemanticEmit`视为不可跳过边界。

一个 work item 一旦实际执行不可重放效果，其本次 partial-evaluation 结果、生成声明批次和闭合实例结果均不得作为可复用 cache entry 发布。普通进程内 memoization 只能在不会跳过该效果的同一次动态执行内部使用。

由于 v0 的跨进程 artifact cache 采用 module-content-hash 粗粒度，只要该 module 任一实际执行的 work item 产生可观察写效果，本次最终 Staged/Closed module artifact 就不得发布；其他纯 work item 的实现内 memo 不把 module 重新变成可缓存。

v0 不记录或重放 effect transcript。cache hit 因而永远不会吞掉、重复或重新排列一次可观察写效果。

### 9.3 Build log 通道

编译期 `stdio.write` 一类显式输出由 ComptimeWorld 映射到 `build.log` capability。它写入编译器的 build-log/diagnostic side channel；CLI Consumer 最终输出到 stderr 或显式日志文件，不得污染 `--emit=ink-ir`、LLVM IR 或其他主结果使用的 stdout。

build log 保持调用内源码顺序，实际执行后使 work item 不可缓存。实现不能因为命中旧 cache 而省略本次应发生的 log。

### 9.4 规范 work-item 顺序

效果和提交采用以下规范顺序：

```text
canonical topological module order
-> fixed-point round
-> canonical ExpansionIdentity or InstanceIdentity
-> plan node canonical order
-> dynamic execution source order
```

canonical module order对10 Section 17的`module -> dependency`边使用dependency-first稳定Kahn：每一步从当前所有零出度module中选择规范`(CanonicalModuleIdentity UTF-8 bytes, ModuleContentDigest bytes)`最小者，追加后删除所有指向它的edge；无ready node表示有环。fixed-point round只参与调度，不进入语义声明身份。ExpansionIdentity按10 §2.3规定的结构化payload逐字段比较，InstanceIdentity按其canonical preimage比较；两者都禁止用摘要字节、worker完成顺序或容器顺序替代。plan node按CanonicalWorkKey的完整preimage排序，digest只用于完整性确认。

调度前必须根据 plan、模板、可达 Core opcode、已解析 callee effect summary 和 capability handler schema 计算每个 work item 的保守 `EffectUpperBound`。只要 upper bound 含任何可能的 host I/O、编译期可观察效果、untracked read 或 unknown/indirect callee，该 work item 就必须从开始执行起进入 ordered lane，即使实际动态路径最终没有到达该 effect；不得先在并行 worker 执行，再根据实际 trace 补排顺序。只有被证明为 Pure、TargetContext-only 或仅从 `BuildInputSnapshot` tracked-read 的 work item 可以并行计算。effect summary 不完整、递归尚未收敛或 handler 未声明精确 upper bound 时按可能 host I/O 处理。

实际动态路径仍决定 9.2 节的 cacheability；保守进入 ordered lane 本身不会使纯结果不可缓存。以下内容必须通过 ordered commit sequencer 按上述顺序发布：

- 生成声明事务；
- generic instance 可见性；
- 结构化诊断和 build log；
- 可观察 host write effect；
- 最终 Core IR 表项。

upper bound 可能执行可观察 host effect 的 work item 必须在其规范顺序槽中串行运行。外部效果一旦执行，后续源码或验证失败不保证回滚；事务性只保证编译器声明图和 cache 不会部分提交。

## 10. 固定点展开

### 10.1 前置阶段

普通 Staged fixed point 开始前必须已经完成：

```text
freeze BuildInputSnapshot
-> parse source files needed for import selection
-> collect finite static candidate import sites
-> execute restricted import guards
-> normalize active module paths
-> form and verify active module DAG
-> freeze ActiveModuleGraph
```

冻结结果必须按 10 Section 17 的`module -> dependency`边与零出度ready-set stable-Kahn算法序列化，不得使用零入度导致反向初始化/链接顺序。后续 plan、template、reflection 和 capability handler 都不得增加 import 或 module dependency。试图这样做是 Staged verifier 错误，而不是启动第二次 module discovery。

`ImportSelectionProfile`是建立StagedModule之前唯一允许执行import guard的闭合子语言。candidate import sites按10 §2.1规范source-file order与文件内lexical structural path严格排序并恰好执行一次；表达式只允许bool、定宽integer、NFC string/enum常量、纯比较/布尔组合、已冻结SemanticOptions值，以及TargetContext中`target_triple_component|cpu_name|required_feature_present`三类标量查询。它禁止sizeof/alignof/layout/PDB/runtime ABI storage查询、用户/extern/decorator/generic函数调用、reflection、循环/递归、声明/registration sink、allocation与pointer/address观察。

guard的外部读取只允许当前BuildInputSnapshot上的FileRead、DirectoryRead、EnvironmentRead、ConfigRead或ToolResourceRead typed tracked handler，并必须把每次实际读取的完整Dependency S projection合并进最终DependencyManifest；读取不能改变candidate site集合或产生新import路径，只能在预收集有限候选中返回bool。fs/network/process/clock/random/build-log等写或untracked effect、live-host绕过snapshot、missing capability、budget耗尽和未处理异常都使import selection失败且不产生Staged artifact。evaluation fuel/stack/byte budget是LanguageRevision登记的固定profile常量，只决定受控失败，不进入成功语义；不得按worker、wall clock或host环境改变。所有guard完成后才按结果形成、验证并冻结ActiveModuleGraph，后续staging不得重开profile。

### 10.2 Work item

fixed-point work item 至少包括：

```text
GenericInstanceWork(InstanceIdentity)
RegionSelectionWork(ExpansionIdentity, PlanNodeId)
RegionExpansionWork(ExpansionIdentity, PlanNodeId)
GeneratedDeclarationValidationWork(PendingBatchId)
ModuleRegistrationValidationWork(PendingBatchId)
LayoutCompletionWork(TypeIdentity)
```

每个 work item 具有稳定 key、origin、资源账户、capability context、dependency recorder 和 cacheability 状态。线程 ID、入队时间和完成顺序不进入 key。

### 10.3 每轮算法

每轮执行：

```text
1. retain the immutable build-wide BuildInputSnapshot and freeze current declaration/type/instance snapshot
2. collect every request visible at round start
3. deduplicate by canonical work-item key
4. sort by canonical order
5. compute conservative EffectUpperBound, execute proven pure/snapshot-read-only items in parallel, and execute every potentially host-I/O/observable item on the ordered lane
6. place generated declarations, types, Core IR and module registration records in pending transactions
7. bind names and check access against the round snapshot plus the batch's own declared scope rules
8. type-check, resolve layout dependencies, verify generated Core IR and validate every frozen registration constant/relocation
9. atomically commit every successful batch in canonical order
10. expose committed results only to the next round
11. enqueue newly requested instances and plan nodes for the next round
12. stop when no mandatory request remains
```

同一轮不得通过反射观察另一 work item 尚未提交的声明或 registration record。pending batch 失败时不得留下部分 symbol、type、instance、registration、diagnostic cache 或 Closed IR。

同一轮以及同一 build 的后续轮都不得通过 tracked read、untracked read、反射或 handler 观察当前 build 的 host output；所有输入查询仍绑定第 9.1 节冻结的 `BuildInputSnapshot`。这条隔离独立于 declaration snapshot 的逐轮可见性。

外部效果与声明事务边界不同：已经在规范 effect lane 上实际发生的 build log 或宿主写入不会因为第 8 步失败而回滚，但相应 work item 永远不可缓存。

### 10.4 实例状态

实例状态机为：

```text
Absent
-> Queued
-> HeaderProvisional
-> Elaborating
-> VerifiedPending
-> Committed

any non-committed state -> Failed
```

请求一个具有完整 provisional header 的相同进行中函数实例可以复用该 header，以支持普通递归。下列情况仍是阶段环错误：

- 在类型自身布局完成前按值查询其完整布局；
- 反射要求观察尚未提交的最终成员集合；
- 默认泛型实参循环依赖自身结果；
- 实例 key 严格增长且无法收敛；
- provisional declaration 被当作已验证 `GenericDecl` 句柄逃逸。

### 10.5 Identity

泛型实例的语义身份为：

```text
InstanceIdentityPayload = {
    GenericDeclarationSymbol,  // source-backed GenericDeclarationProvenance parent snapshot
    CanonicalClosedGenericArguments : Vec<TaggedClosedGenericArgument>,
    TargetKey,
    InstanceIdentityDigest = H("ink.instance-identity.v0", GenericDeclarationSymbolKey, CanonicalClosedGenericArguments, TargetKey),
}
```

生成声明的展开上下文身份精确复用 10 §2.3 的结构化 schema：

```text
ExpansionIdentity =
    SourceDeclarationKey : canonical structured bytes
  + ExpansionSiteIdentityPayload {SourceFileContentDigest, SourceDeclarationKey, TemplateRolePath, NormalizedHirStructuralNodePath, ExpansionRoleTag}
  + optional EnclosingInstance : InstanceIdentityPayload
  + CanonicalControlPath : Vec<TaggedCanonicalExpansionControlStep>
  + TargetKey
  + recomputed ExpansionIdentityDigest
```

上述`InstanceIdentityPayload.TargetKey`、`ExpansionIdentity.TargetKey`以及它们经parent/decorator/dynamic-control/registration context递归携带的每一份副本，都必须逐字节等于当前Manifest.target_key；不能嵌入另一目标的自洽identity再仅重算末尾digest。dependency预生成identity只能在provider Manifest.target_key与consumer当前TargetKey相同且DirectImportBinding固定该provider时镜像。

`CanonicalControlPath` 的 branch step携带`ControlNodeIdentityPayload + ArmOrdinal`；loop step携带下列结构化`IterationIdentityPayload`：

```text
ControlNodeIdentityPayload
+ LoopKindTag(for | while)
+ zero-based semantic iteration ordinal
+ optional TaggedClosedGenericArgument iterated element
```

`for` 的iterated element必须present并使用准确TaggedClosedGenericArgument canonical projection；`while`固定absent并在digest中映射为empty bytes，轮次只由ordinal区分。ordinal也用来区分相等元素产生的两次真实展开，例如 `(i32, i32)`；两次展开随后仍可能按普通声明规则形成重定义诊断。ordinal 是稳定迭代序列中的位置，不是 worker 完成序号。

`CanonicalControlPath` 使用 source-backed control node、numeric arm/branch identity和嵌套路径；ExpansionIdentity按结构化字段排序，摘要只重算核对。不得使用 fixed-point round、本轮第几个结果、digest字节序或对象地址。

`ExpansionIdentity`是work/cache provenance，不替代最终Symbol.DeclarationIdentity。decorator产生的每个root declaration各占一个连续ordered semantic-output event，并使用10 §9.2的generated `decorator_expansion`、parent=`expansion_context(ExpansionContextIdentityPayload)`、`SemanticOutputOrdinal=0`；nested declaration改用`owner_symbol`直接指向同一expansion lineage中的generated lexical parent，ordinal恰为该parent的direct generated-child canonical semantic-output index。这条parent chain必须最终到达root expansion_context，因而nested declaration在generated owner之间移动一定改变identity，不得把仅能定位源码HIR site的`ExpansionSite.NormalizedHirStructuralNodePath`当作generated output tree path。source-backed generic declaration的ClosedGenericArguments固定empty并携带从source binder顺序重建的GenericDeclarationProvenance marker；删除open entity/dependent Type/Template之前，closeAndVerify必须冻结并逐字段核对tag-2 `GenericDeclarationSignatureSurfaceVector`，其binder-relative parameter/pack、inline dependent-expression和self sentinel使Closed仍可按`ink.declaration-signature.v0`独立重算SignatureDigest，而不是只比较两个相同D32或保留当前SymbolKey回边。闭合instance唯一使用generated `generic_instance`、该parent snapshot的owner_symbol、与ParameterKinds逐项匹配的non-empty arguments、marker absent和固定ordinal 0，相同owner/arguments/role必须intern，不能靠source-backed+arguments或另一个ordinal重复。ProducedSymbolKind=stable_entry的泛型function实例本身是logical stable symbol并拥有独立StableEntry/version-local body mapping；parent marker自身不是LogicalStableCallable或可执行entity。形成Closed时open generic entity、dependent types和Template删除，只保留至少被一个instance引用且不能lookup/call/export的private parent snapshot。所有其他generated kind按10 §9.2的owner-slot矩阵反向重算，任何发现顺序或worker编号都非法。

5.9 节的 `RegistrationIdentity` 使用同一套 canonical application/control/iteration ordinal规则。`DecoratorApplicationOrderPath` 来自规范源码/生成输出层次；application-wide `EmissionOrdinal` 来自每次实际到达 registration emission 的确定性 execution trace。二者都不是全局调度计数器，也不能因缓存重放、线程数、fixed-point round或提交次序改变。

### 10.6 收敛与资源预算

固定点终止必须满足：

- 没有未完成 mandatory plan node；
- 没有未决 generic instance；
- 没有等待提交的声明批次；
- 没有等待验证或提交的 module registration record；
- 没有 unresolved dependent type/name/layout；
- 所有 runtime-visible 内容均可进入 Closed IR。

实现必须限制：

- 总执行 fuel；
- 调用深度和实例化栈深度；
- 单循环和总循环迭代数；
- 生成声明、类型、symbol、block 和 operation 数量；
- comptime virtual memory；
- 外部 dependency 数量和读取字节数；
- diagnostic、build-log 和 origin 链大小；
- wall-clock 取消检查点。

预算耗尽或用户取消不提交当前 transaction，也不发布 cache entry。预算耗尽是带调用栈、实例栈和主要资源计数的编译诊断，不是 UB、普通异常或隐式 residualization。

## 11. Cache identity

语义 `InstanceIdentity` 不等于跨进程 `InstanceCacheKey`。内部缓存必须额外包含：

```text
InstanceCacheKey =
    CacheNamespace
  + CompilerBuildId
  + LanguageRevision
  + IrSemanticsRevision
  + TextSyntaxVersion
  + BinaryContainerVersion
  + SchemaRegistryDigest
  + RegistrationEncodingRevision
  + GenericDeclarationCacheKey
  + CanonicalClosedGenericArguments : Vec<TaggedClosedGenericArgument>
  + TargetKey
  + TargetContextDigest
  + CanonicalRequiredFeatureSet
  + SemanticOptionsDigest
  + ActiveModuleDagDigest
  + ActiveDependencyInterfaceDigest
  + CapabilityPolicyRevision
  + CapabilityPolicyDigest
  + ComptimeHandlerRevision
  + HandlerRevisionDigest
  + PassPipelineRevision
  + PassPipelineDigest
  + InstanceDependencyManifestDigest
```

revision 标识对应 schema/语义规则版本，digest 标识本次构建规范化后的实际配置内容，两者不能互相替代。`CapabilityPolicyDigest` 覆盖准确授权集合、作用域与参数；`HandlerRevisionDigest` 覆盖已注册 handler identity、各自 revision 和会改变结果的规范配置；`PassPipelineDigest` 覆盖实际 pass 序列及其语义选项。相同 compiler build 下切换target context/required feature、active module DAG、授权、handler 配置或 pass pipeline 必须 cache miss；不能只靠active dependency的export interface掩盖provider集合、边或root变化。

`InstanceDependencyManifest` 是该实例求值实际观察到的tracked Dependency record的canonical ordered subset，并递归并入被缓存callee返回结果所声明的subset；它使用10 §10.1的完整Dependency S projection，以完整canonical bytes严格递增、无重复，编码为`Count : U`后逐项`ProjectionLength : U + ProjectionBytes`。`InstanceDependencyManifestDigest = H("ink.instance-dependency-manifest.v0", InstanceDependencyManifest)`。首次执行尚不知道该digest时，只能用上述key去掉最后一项所得的`InstanceCacheLookupPrefix`枚举候选；每个候选必须先按其保存的manifest逐项在当前BuildInputSnapshot重放验证，再重算digest并与完整key相等，之后才可命中。空subset仍编码`Count=0`并具有真实digest；不得使用全零哨兵、模块级DependencyManifestDigest替代、只记录direct reads或在命中后才验证。

`GenericDeclarationCacheKey` 使用规范 module identity、完整 module content digest 和 declaration structural path。v0 采用 module 级粗粒度失效：module 任意内容变化可以使其全部 Staged IR 和实例 cache miss；不承诺无关编辑后保持命中。

module artifact的全部tracked external input仍通过 [`08-text-binary-format.md`](./08-text-binary-format.md) 定义的完整DependencyManifest闭合；instance候选则按上段更精确的per-entry subset在返回缓存结果前复验。物理 checkout 路径、当前进程 ID、arena ID、诊断展示路径和 origin 打印编号不得进入语义 key。

origin/debug table 默认不进入 semantic digest，但必须绑定准确 module/source content digest。去掉 origin 不得使来自另一份源码内容的 cache entry 错误映射到当前文件。

## 12. 诊断与 Origin

Staging producer 使用 Core 公共诊断模型提交 `DiagnosticKind`、主 `SourceFileId + SourceRange`、类型化参数和结构化 related origin。它不得自行拼接最终消息。

至少必须区分：

- 强制 value/block 依赖 Residual；
- selector、match 值、迭代源或 while condition 不是 Known；
- 实际执行到无 capability 或无 handler effect；
- meta/host value 尝试 residualize 或逃逸；
- 某轮异构展开 body 不合法；
- 生成声明重定义；
- 进行中实例、布局或反射阶段环；
- fixed point 不收敛；
- 资源预算耗尽或取消；
- cache dependency 失效与 cache corruption。

每个生成 symbol 和 operation 的 origin 至少保留：

- 真实源码模板范围；
- 控制结构和选中 arm；
- 每层 iteration identity；
- 闭合 generic instance；
- request/call site；
- 当前 comptime execution frame。

未选择模板不产生语义诊断；它已有的词法和语法诊断仍由前置阶段正常报告。

## 13. 形成 ClosedModule

fixed point 收敛后，close pass：

1. 只保留已经提交的 type、symbol、global、function、runtime declaration 和 module registration record；
2. 将所有可 runtime-representable Known value 物化为规范 constant 或普通 initialization IR；
3. 删除 ElaborationPlan、NormalizedTemplateTable、`ct.register_module_item`、编译期 sequence、meta value 和 host capability；已提交 registration record 作为 Closed module entity 保留；
4. 确认 active module DAG、TargetKey、layout 和 runtime ABI revision；
5. 拒绝任何 Residual 强制边界、未决 name/type/layout、open generic entity/template 或 stage node；仅允许10 §9.2规定且被closed instance引用的private GenericDeclarationProvenance Symbol snapshot；
6. canonicalize Core tables 和 origin；
7. 运行完整 Closed verifier；
8. 只在成功后构造 `VerifiedClosedModule<TargetKey>`。

关闭过程必须构造新的不可变 artifact 或使旧验证能力失效；不得原地把 `StageKind` 改成 Closed 后继续复用旧 verifier token。

RuntimeWorld、TargetABI 和 LLVM backend 只能接收上述验证结果。LLVM 不理解 Known/Residual、deferred template、declaration sink、capability 或 fixed-point transaction；它只接收验证后的 readonly registration data/relocation descriptor，并不执行用户 install/remove hook。

## 14. 最小实现顺序

首个纵切片按以下顺序实现：

1. `stage.force_value`；
2. Known integer/bool、pure `const.*`、`arith.*`、`cf.*` 与 `call.direct`；
3. ComptimeWorld fuel、调用栈和结构化诊断；
4. C0 `comptime expression` 完整求值；
5. ResidualizeWorld 的 Known/Residual 二元运算和普通调用；
6. `stage.select_if`；
7. 显式值泛型与 `stage.instantiate`；
8. fixed-point transaction、实例 cache 和 dependency manifest；
9. `stage.force_block`、`stage.select_match`、`stage.expand_for/while`；
10. declaration sink、meta value、反射和完整泛型声明展开；
11. `ct.register_module_item`、frozen registration encoding、版本化安装/撤销与原子 hot-reload publish。

在实现 declaration sink 和异构循环前，不能用提前类型检查全部分支的临时方案代替 deferred template。

## 15. 确认结论

InkIR 使用组合式 `StagedModule`：已 elaboration 的代码始终是 typed Core InkIR，尚未选择或仍 dependent 的源码体保存在自包含、source-backed 的 `NormalizedTemplateTable` 中，由 `ElaborationPlan` 的 `stage.force_value`、`stage.force_block`、`stage.select_if`、`stage.select_match`、`stage.expand_for`、`stage.expand_while` 和 `stage.instantiate` 驱动。ComptimeWorld、ResidualizeWorld 与 RuntimeWorld 共享普通 operation 语义；Known/Residual 只属于部分求值器，不能进入 Closed 类型系统。`ct.register_module_item` 作为真正受 typed CFG 到达性控制的 Staged-only operation，向同一 fixed-point transaction 提交 frozen typed registration record；operation 关闭前消失，记录作为版本拥有的 Closed module entity 保留。声明固定点按稳定快照、pending transaction 和下一轮可见性运行，所有可观察编译期效果按规范 work-item 顺序执行并使该 work item 不可缓存；纯计算、受跟踪只读效果和可事务重放的 registration emission 可以缓存，不做 host effect replay。固定点收敛后必须完全消除plan、template、meta value、开放泛型entity与host resource；仅可保留被closed instance引用的非执行GenericDeclarationProvenance Symbol snapshot，再由Closed verifier建立唯一可供解释器和LLVM backend使用的能力对象。
