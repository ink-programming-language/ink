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
`- committed ModuleRegistrationTable

fixed-point elaboration + partial evaluation
    -> ClosedModule[target]
```

只有 Typed Core InkIR operation 保证已经完成名称解析、类型检查并具有规范类型。`ElaborationPlan` 和 `NormalizedTemplateTable` 是编译阶段数据，不属于 RuntimeWorld 可执行指令集。

## 2. `StagedModule` 的组成

逻辑结构为：

```text
StagedModule = {
    Header,
    TargetContext,
    ActiveModuleGraph,
    TypedCoreModule,
    ElaborationPlan,
    NormalizedTemplateTable,
    ModuleRegistrationTable,
    SourceFileTable,
    OriginTable,
    DependencyManifest,
    CapabilityPolicy,
}
```

其不变量为：

- active module DAG 已经由静态候选 import 选择阶段确定并冻结；
- Typed Core 中每条 operation 的 operand、result、attribute、effect 和 origin 完整；
- 未完成 elaboration 的源码结构只通过稳定 `TemplateId` 出现在模板表中；
- plan node 只能引用本 module、已冻结依赖或规范闭合实例中的稳定对象；
- host pointer、arena address、进程内 `DeclId` 和可写 IR pointer 不得成为 plan 或 template 的持久身份；
- `TargetContext` 在任何 target-dependent 查询或执行前已经确定；
- capability 只授予实际执行到的 effect operation，不按函数名称预授予；
- StagedModule 可以含 meta type、开放声明句柄和 dependent template，但这些内容必须在形成 ClosedModule 前消失。
- `ModuleRegistrationTable` 只包含 fixed-point transaction 已原子提交的强类型记录；仍在执行或验证的 emission 只存在于当前 pending batch，对同轮其他 work item 不可见。

只有 `verifyStaged` 成功返回的 `VerifiedStagedModule` 可以交给 fixed-point elaborator。给普通 module 设置一个可修改的 `StageKind` 字段不能替代该能力边界。

## 3. `NormalizedTemplateTable`

### 3.1 模板不是 CST 副本

`NormalizedTemplateTable` 保存重新 elaboration 所需的最小、source-backed 语义模板。它不复制 full-fidelity CST 的 Trivia，也不保存可由用户修改的 AST 或 IR Builder。

每个模板至少记录：

```text
NormalizedTemplate = {
    TemplateId,
    TemplateKind,
    RegionKind,
    SourceDeclarationKey,
    NormalizedHirRoot,
    LexicalEnvironmentKey,
    ExplicitCaptures[],
    OriginId,
    SemanticDigest,
}
```

`TemplateKind` 至少区分：

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
```

模板必须已经通过 Tokenizer、Parser 和语法恢复检查。未选择模板仍须词法和语法正确，但不得仅为建立模板而执行依赖名称绑定、成员查找或类型检查。

### 3.2 Normalized HIR 的允许内容

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

### 3.3 显式 capture

模板对外层环境的依赖必须显式编码为 capture：

```text
TemplateCapture = {
    CaptureKind,
    CanonicalKey,
    ExpectedSemanticType,
    AccessMode,
    OriginId,
}
```

`CaptureKind` 可以是 compile-time value、runtime SSA value、place、type、declaration handle、lexical declaration 或 target/config query。capture 顺序按照模板中建立的规范首次使用顺序固定；序列化和 hash 不得依赖 map 迭代顺序。

运行时 SSA value 或 place 可以被结构化 `comptime if/for` 的选中 body 捕获，因为该 body 可以残留普通运行时代码。相同值进入 `stage.force_value` 或 `stage.force_block` 时若成为必经依赖，则强制求值失败。

### 3.4 模板身份

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

`ElaborationPlan` 是具有显式依赖边的有向图。每个 node 至少记录：

```text
PlanNode = {
    PlanNodeId,
    StageOpcode,
    Inputs[],
    TemplateRefs[],
    SinkKind,
    ParentElaborationContext,
    RequiredCapabilities[],
    OriginId,
}
```

plan dependency 必须无非法环。普通函数递归不表现为 plan graph 环；泛型实例递归通过实例状态机和 provisional declaration 处理。

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
  | InstanceArgument(InstanceIdentity, ArgumentIndex)
```

每个 input 同时记录准确 semantic type。`PlanResult` 自动形成从 producer 到 consumer 的 dependency edge，producer 必须唯一且 result index 合法。`CoreValue` 的 `%vN` 只在 owner function/region 内有意义，因而序列化时必须带 `OwnerSymbol + RegionPath`；只保存裸 `ValueId` 不合法。该引用还必须满足 Typed Core 的 dominance、place lifetime 和 stage-capture 规则。

Known selector、`stage.force_value` 和 `stage.force_block` 的必经输入不得是 Residual `CoreValue`。`stage.select_if`/`stage.expand_for` 选中后允许 residual code 的 body 可以通过显式 `TemplateCapture` 捕获运行时 value/place；这不使 selector 本身变为 runtime 条件。

Statement 或 declaration sink 的插入位置使用：

```text
InsertionAnchor = {
    OwnerSymbol,
    RegionStructuralPath,
    SourceBackedAnchorKey,
    Position,                 // before, after, replace 或 append
}
```

artifact 内可以附带 canonical `BlockId/OperationOrdinal` 作为快速索引，但 verifier 必须确认其解析到同一 `SourceBackedAnchorKey`。work key 和跨进程 cache identity 使用 source-backed anchor，不使用优化前对象地址、临时 block 指针或当前容器下标。

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
#p0 = stage.force_value template #tmpl0 sink Value origin(#o7)
```

该文本是 ElaborationPlan 记录，不是一条 `%v = ...` Core instruction。

### 5.3 `stage.force_block`

`stage.force_block` 对应 StatementRegion 的 `comptime { ... }`。block 中的绑定、赋值、普通调用、`try`、`defer`、`return`、`break`、`continue` 和 `throw` 继承 ComptimeWorld。

要求：

- 整个实际执行路径必须在编译期完成；
- 不允许输出 residual runtime operation；
- 局部对象、虚拟目标内存和 cleanup 必须按普通语言语义执行；
- 离开 block 后仍被使用的结果必须是可合法逃逸的 Known value；
- comptime allocation 地址、host handle 和临时引用不得逃逸；
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

### 5.9 `ct.register_module_item`

源码 compiler builtin `register_module_item(value)` lowering 为显式 Staged-only typed Core operation `ct.register_module_item`，不是 RuntimeWorld call、用户可取地址函数或隐式编译器回调。它只允许在 active decorator application 的 `ParentElaborationContext` 内由 ComptimeWorld 真正执行；普通 comptime CFG 未到达该 operation 时不产生记录。该 operation 不可 residualize，形成 ClosedModule 前必须被执行并消除，或因所在模板未选择而随模板消失。

operation 有无结果的两个 operand schema：单标量或 unit 直接传 `T`；address-only 值传只读 `!place<ro,T>`。opcode payload 还保存不可由用户指定的 `SourceBackedCallsiteKey`。执行器要求 operand 是完整 Known 值，把它按准确逻辑类型 canonicalize 到 ConstantTable，然后向当前 work item 的 pending `ModuleRegistration` sink 提交恰好一条记录；它不把 place、ComptimeWorld allocation 地址或宿主对象保存进记录。

提交记录至少固定：

```text
ModuleRegistrationRecord = {
    RegistrationIdentity,
    RegistrationType,
    RegistrationValueConstant,
    ProducerSymbol,
    DecoratorApplicationKey,
    DecoratorApplicationOrderPath,
    SourceBackedCallsiteKey,
    DynamicControlPath,
    EmissionOrdinal,
    ProtocolSchemaDigest,
    OriginId,
}
```

`ProducerSymbol` 是本次 decorator application 的最终声明身份，不是 decorator 定义函数本身；注册值中的 `function.entry` 必须解析到最终装饰后的 stable entry。`DynamicControlPath` 是下列结构化 tagged step 序列，不是 opaque hash：

```text
Expansion(ExpansionContextKey)
Call(CallSiteKey, InvocationOrdinal)
Branch(ControlNodeKey, ArmOrdinal)
Loop(ControlNodeKey, IterationOrdinal, IterationIdentityDigest)
```

call/recursion 在同一 site 的多次动态进入由当前 work item 源码执行顺序的 `InvocationOrdinal` 区分；相等 loop element仍由 `IterationOrdinal` 区分。`DecoratorApplicationOrderPath` 是非空奇数长度 `Vec<U>`：root application 是按 [08](./08-text-binary-format.md) 的唯一 source-file 顺序和文件内 lexical decorator-application traversal 得到的 `[RootApplicationOrdinal]`；生成的 child 是 `ParentPath ++ [ParentSemanticOutputOrdinal, ChildLocalApplicationOrdinal]`。`ParentSemanticOutputOrdinal` 统一计数 parent transaction 的全部 ordered semantic-output event，不是 registration-only 计数；路径按无符号数字典序比较，短前缀先于其 child。`EmissionOrdinal` 则是同一 active application 内每次实际到达 `ct.register_module_item` 时从零连续递增的 application-wide 序号。batch 按 `(DecoratorApplicationOrderPath, EmissionOrdinal, RegistrationIdentity)` 排序；不得把 D32 hash字节序冒充程序顺序。线程、fixed-point round、work queue 和容器插入顺序都不得进入，cache replay必须重现相同路径和两套 ordinal。

语义身份为：

```text
RegistrationIdentity = H(
    "ink.module-registration.v0",
    CanonicalModuleIdentity,
    ModuleContentDigest,
    ProducerSymbol.SymbolKey,
    DecoratorApplicationKey,
    DecoratorApplicationOrderPath,
    SourceBackedCallsiteKey,
    DynamicControlPath,
    EmissionOrdinal)
```

相同 artifact/work item 内相同 identity 的重放只有在完整 logical record（包括 canonical Origin structure）逐字段相同时才能由 pending-batch commit 在writer之前去重；任何差异都是 determinism/compiler error。canonical section本身不得含重复identity或重复 `(DecoratorApplicationOrderPath, EmissionOrdinal)`，decoder不负责规范化。identity 是 module-content/version-local：value变化即使不作为独立 identity字段，也通常会经 `ModuleContentDigest` 产生新 identity。跨版本替换依靠不可伪造 version owner 下整套 registration set 的原子 publish/retire，不承诺同一 lexical site 保持相同 RegistrationIdentity。

`StaticRegistrationEncodable(T, Constant)` 是中央 verifier 派生谓词，而不是用户可随意声明的 attribute。它要求 closed、`LayoutComplete` 的准确 typed constant 具有只读 frozen module-image encoding，不需要在安装或撤销时运行用户 constructor、destructor 或 rollback。v0 闭包允许 scalar/unit、受支持 enum/tuple/array/nominal aggregate、受控 symbol relocation，以及中央 registry 明确支持的 built-in String frozen encoding；String 的 module-owned immutable bytes/length 表示必须布局合法且没有独立释放责任。用户 destructor、资源字段、runtime/meta/host handle、runtime object/opaque object、place、exception、checked reference/interface/no-escape 值和宿主/comptime pointer 默认拒绝，除非中央 registry 明确给出无用户代码、无释放责任的准确 frozen encoding。非空 raw/function pointer 只能由获准 symbol relocation 产生；函数目标必须是 stable entry，其他 static relocation 的存活期必须覆盖所属 module version，version-local body 裸地址非法。

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
- MayTrap、MayUnwind、TargetPDB 和 effect operation 不能仅因结果未使用而提前执行、删除或推测；
- 编译期专用值不能作为 residual operand 写入 Closed IR。

示例：

```text
Known(i32 10) + Known(i32 20) -> Known(i32 30)
Residual(%v0) + Known(i32 10) -> residual `arith.add %v0, const.int 10`
```

### 6.3 控制流

普通 `cf.cond_br` 的条件 Known 时只 residualize 被选择 successor。条件 Residual 时保留 runtime branch，并分别 residualize 两条可达路径；不得执行任一分支中的 RuntimeEffect。

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

典型 capability key 为：

```text
build.log
fs.read
fs.write
environment.read
network.request
clock.now
random.read
process.spawn
target.extern_call
inline_assembly
```

target triple、layout、PDB 和语言内建反射查询由 TargetContext 或编译器语义 handler 提供，不读取宿主平台偶然状态。

未选中的分支不检查 capability。执行到无权限、无 handler 或实参不满足 handler 要求的 effect 时，在该 operation origin 报错，并附完整 comptime 调用栈和实例化链。

### 8.2 默认 capability policy

v0 默认允许：

- Pure operation；
- comptime local state 和虚拟目标内存；
- TargetContext 查询；
- 编译器内部类型、声明和结构反射；
- 显式授权并记录依赖的只读输入。

v0 默认拒绝：

- 网络；
- 进程创建；
- 目标 extern call；
- inline assembly；
- 未跟踪环境读取；
- 时钟和随机数；
- 没有显式构建权限的文件写入。

构建配置可以显式增加 capability，但 policy 本身及 handler revision 必须进入 cache identity。授权不表示结果可缓存。

### 8.3 外部资源不得逃逸

host file handle、socket、process handle、host pointer、ComptimeWorld allocation address 和其他只在编译器进程中有意义的值：

- 不能进入 Closed constant；
- 不能成为运行时 global、field、parameter 或 return；
- 不能跨 `extern` ABI；
- 不能作为可在另一进程恢复的编译期实参；
- 不能通过 aggregate 或 tuple 间接逃逸。

允许 handler 返回的普通数据必须转换为有准确 Ink 类型的 CtValue，并记录其依赖或不可复现性。

## 9. 可观察效果、确定性与缓存

### 9.1 Cacheability 分类

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
| host/target extern effect、inline assembly execution | 默认拒绝且不可缓存 |

cacheability 是本次动态执行路径的属性，不是函数声明的永久属性。未执行分支中的写 effect 不会使当前结果不可缓存。

registration emission有两个不同缓存层。generic/instance IR cache只保存仍含 `ct.register_module_item` 的 typed Core，并在每个新的动态application/call context中重新真实执行该 operation；它不得把某次执行产生的具体record当作实例本身的一部分。只有 work-item result cache 才能重放pending registration output，而且其key必须在 `InstanceCacheKey` 之外准确包含 `DecoratorApplicationKey`、`DecoratorApplicationOrderPath`、当前 `DynamicControlPath`/invocation prefix、起始application-wide `EmissionOrdinal` 和起始 `ParentSemanticOutputOrdinal`。重放必须恢复同一批record并推进两个cursor；上下文任一项不同就必须重新执行，不能吞掉另一callsite、递归帧或loop iteration的emission。普通函数memo若不采用这一context-complete replay key，就必须把`OrderedSemanticEmit`视为不可跳过边界。

一个 work item 一旦实际执行不可重放效果，其本次 partial-evaluation 结果、生成声明批次和闭合实例结果均不得作为可复用 cache entry 发布。普通进程内 memoization 只能在不会跳过该效果的同一次动态执行内部使用。

由于 v0 的跨进程 artifact cache 采用 module-content-hash 粗粒度，只要该 module 任一实际执行的 work item 产生可观察写效果，本次最终 Staged/Closed module artifact 就不得发布；其他纯 work item 的实现内 memo 不把 module 重新变成可缓存。

v0 不记录或重放 effect transcript。cache hit 因而永远不会吞掉、重复或重新排列一次可观察写效果。

### 9.2 Build log 通道

编译期 `stdio.write` 一类显式输出由 ComptimeWorld 映射到 `build.log` capability。它写入编译器的 build-log/diagnostic side channel；CLI Consumer 最终输出到 stderr 或显式日志文件，不得污染 `--emit=ink-ir`、LLVM IR 或其他主结果使用的 stdout。

build log 保持调用内源码顺序，实际执行后使 work item 不可缓存。实现不能因为命中旧 cache 而省略本次应发生的 log。

### 9.3 规范 work-item 顺序

效果和提交采用以下规范顺序：

```text
canonical topological module order
-> fixed-point round
-> canonical ExpansionIdentity or InstanceIdentity
-> plan node canonical order
-> dynamic execution source order
```

canonical module order 使用稳定 Kahn 拓扑排序：每一步从当前所有零入度 module 中选择规范 module identity 最小者。fixed-point round 只参与调度，不进入语义声明身份。

Pure 和 tracked-read work item 可以并行计算。以下内容必须通过 ordered commit sequencer 按上述顺序发布：

- 生成声明事务；
- generic instance 可见性；
- 结构化诊断和 build log；
- 可观察 host write effect；
- 最终 Core IR 表项。

实际执行可观察 host effect 的 work item 必须在其规范顺序槽中串行运行，不能先在任意 worker 上执行后仅对日志重新排序。外部效果一旦执行，后续源码或验证失败不保证回滚；事务性只保证编译器声明图和 cache 不会部分提交。

## 10. 固定点展开

### 10.1 前置阶段

普通 Staged fixed point 开始前必须已经完成：

```text
parse source files needed for import selection
-> collect finite static candidate import sites
-> execute restricted import guards
-> normalize active module paths
-> form and verify active module DAG
-> freeze ActiveModuleGraph
```

后续 plan、template、reflection 和 capability handler 都不得增加 import 或 module dependency。试图这样做是 Staged verifier 错误，而不是启动第二次 module discovery。

### 10.2 Work item

fixed-point work item 至少包括：

```text
GenericInstanceWork(InstanceIdentity)
RegionSelectionWork(ExpansionContext, PlanNodeId)
RegionExpansionWork(ExpansionContext, PlanNodeId, IterationIdentity)
GeneratedDeclarationValidationWork(PendingBatchId)
ModuleRegistrationValidationWork(PendingBatchId)
LayoutCompletionWork(TypeIdentity)
```

每个 work item 具有稳定 key、origin、资源账户、capability context、dependency recorder 和 cacheability 状态。线程 ID、入队时间和完成顺序不进入 key。

### 10.3 每轮算法

每轮执行：

```text
1. freeze current declaration/type/instance snapshot
2. collect every request visible at round start
3. deduplicate by canonical work-item key
4. sort by canonical order
5. execute pure/read-only items in parallel and effectful items on the ordered lane
6. place generated declarations, types, Core IR and module registration records in pending transactions
7. bind names and check access against the round snapshot plus the batch's own declared scope rules
8. type-check, resolve layout dependencies, verify generated Core IR and validate every frozen registration constant/relocation
9. atomically commit every successful batch in canonical order
10. expose committed results only to the next round
11. enqueue newly requested instances and plan nodes for the next round
12. stop when no mandatory request remains
```

同一轮不得通过反射观察另一 work item 尚未提交的声明或 registration record。pending batch 失败时不得留下部分 symbol、type、instance、registration、diagnostic cache 或 Closed IR。

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
InstanceIdentity =
    GenericDeclarationIdentity
  + CanonicalComptimeArguments
  + TargetKey
```

生成声明的身份为：

```text
ExpansionIdentity =
    SourceDeclarationIdentity
  + EnclosingInstanceIdentity?
  + CanonicalControlPath
  + IterationIdentity*
  + TargetSelectionKey?
```

`IterationIdentity` 为：

```text
ControlNodeIdentity
+ zero-based semantic iteration ordinal
+ canonical element/key/value
```

ordinal 用来区分相等元素产生的两次真实展开，例如 `(i32, i32)`。两次展开随后仍可能按普通声明规则形成重定义诊断。ordinal 是稳定迭代序列中的位置，不是 worker 完成序号。

`CanonicalControlPath` 使用 source-backed control node、arm/branch 身份和嵌套路径；不得使用 fixed-point round、本轮第几个结果或对象地址。

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
  + CanonicalComptimeArguments
  + TargetKey
  + SemanticOptionsDigest
  + ActiveDependencyInterfaceDigest
  + CapabilityPolicyRevision
  + CapabilityPolicyDigest
  + ComptimeHandlerRevision
  + HandlerRevisionDigest
  + PassPipelineRevision
  + PassPipelineDigest
```

revision 标识对应 schema/语义规则版本，digest 标识本次构建规范化后的实际配置内容，两者不能互相替代。`CapabilityPolicyDigest` 覆盖准确授权集合、作用域与参数；`HandlerRevisionDigest` 覆盖已注册 handler identity、各自 revision 和会改变结果的规范配置；`PassPipelineDigest` 覆盖实际 pass 序列及其语义选项。相同 compiler build 下切换授权、handler 配置或 pass pipeline 必须 cache miss。

`GenericDeclarationCacheKey` 使用规范 module identity、完整 module content digest 和 declaration structural path。v0 采用 module 级粗粒度失效：module 任意内容变化可以使其全部 Staged IR 和实例 cache miss；不承诺无关编辑后保持命中。

tracked external input 通过 [`08-text-binary-format.md`](./08-text-binary-format.md) 定义的 DependencyManifest 在候选 entry 命中后复验。物理 checkout 路径、当前进程 ID、arena ID、诊断展示路径和 origin 打印编号不得进入语义 key。

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
5. 拒绝任何 Residual 强制边界、未决 name/type/layout、open generic 或 stage node；
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

InkIR 使用组合式 `StagedModule`：已 elaboration 的代码始终是 typed Core InkIR，尚未选择或仍 dependent 的源码体保存在自包含、source-backed 的 `NormalizedTemplateTable` 中，由 `ElaborationPlan` 的 `stage.force_value`、`stage.force_block`、`stage.select_if`、`stage.select_match`、`stage.expand_for`、`stage.expand_while` 和 `stage.instantiate` 驱动。ComptimeWorld、ResidualizeWorld 与 RuntimeWorld 共享普通 operation 语义；Known/Residual 只属于部分求值器，不能进入 Closed 类型系统。`ct.register_module_item` 作为真正受 typed CFG 到达性控制的 Staged-only operation，向同一 fixed-point transaction 提交 frozen typed registration record；operation 关闭前消失，记录作为版本拥有的 Closed module entity 保留。声明固定点按稳定快照、pending transaction 和下一轮可见性运行，所有可观察编译期效果按规范 work-item 顺序执行并使该 work item 不可缓存；纯计算、受跟踪只读效果和可事务重放的 registration emission 可以缓存，不做 host effect replay。固定点收敛后必须完全消除 plan、template、meta value、开放泛型和 host resource，再由 Closed verifier 建立唯一可供解释器和 LLVM backend 使用的能力对象。
