# 下一阶段实现方案

## 1. 结论

Tokenizer 和 Parser 已经形成可用基线，下一阶段进入语义层和最小端到端纵切片，但不应直接从 CST 生成最终 Closed InkIR，也不应实现独立的 AST 解释器。

推荐流水线为：

```text
CompilationSession / SourceManager
→ CST
→ 紧凑 AST + 语义 side tables
→ 声明收集 / 名称绑定 / 类型检查
→ VerifiedSemanticModule
→ Staged typed InkIR
→ closeAndVerify
→ Closed InkIR
→ RuntimeWorld 参考解释器 / LLVM lowering
```

近期工作对应 [`roadmap/roadmap.md`](../roadmap/roadmap.md) 的 M1、M2 和 M3：先完成编译会话、源码管理与 AST，再完成最小语义分析，最后生成强类型 InkIR 并接入参考解释器。InkIR 的规范边界以 [`draft/IR/README.md`](../draft/IR/README.md) 和 [`draft/IR/10-schema-registry.md`](../draft/IR/10-schema-registry.md) 为准。

## 2. 推荐目录结构

延续当前 `src/include/ink` 与 `src/lib` 的镜像布局：

```text
src/include/ink/
  core/
    source_file_id.h
    source_manager.h
    string_interner.h
    diagnostic.h
    source_range.h

  ast/
    ast.h
    ast_kind.def
    ast_context.h
    cst_lowering.h
    verifier.h
    printer.h

  type/
    type.h
    type_id.h
    type_context.h
    type_kind.def

  sema/
    declaration.h
    symbol.h
    scope.h
    semantic_model.h
    analyzer.h
    verifier.h

  target/
    target_context.h
    target_key.h

  ir/
    ids.h
    module.h
    function.h
    block.h
    operation.h
    value.h
    builder.h
    verifier.h
    printer.h

  frontend/
    compilation_session.h
    compiler.h
    ir_generator.h

  execution/
    runtime_world.h
    interpreter.h

src/lib/
  ast/
    ast.cpp
    cst_lowering.cpp
    verifier.cpp
    printer.cpp

  type/
    type_context.cpp

  sema/
    declaration_collector.cpp
    name_resolver.cpp
    type_checker.cpp
    control_flow_checker.cpp
    semantic_model.cpp
    analyzer.cpp
    verifier.cpp

  target/
    target_context.cpp
    target_key.cpp

  ir/
    module.cpp
    builder.cpp
    verifier.cpp
    printer.cpp

  frontend/
    compilation_session.cpp
    compiler.cpp
    ir_generator.cpp

  execution/
    runtime_world.cpp
    interpreter.cpp
```

测试目录对应增加：

```text
src/testcase/ast/
src/testcase/sema/
src/testcase/ir/
src/testcase/execution/
```

测试继续由 `src/testcase/CMakeLists.txt` 统一发现。每个 `TEST` 前必须保留一条具体注释，说明验证的功能、边界或错误场景。

## 3. 模块依赖边界

推荐依赖方向为：

```text
core
 ├─ tokenizer → parser → ast → sema
 ├─ type ────────────────┤
 ├─ target ──────────────┤
 └─ ir ← frontend lowering
        ↓
     execution

LLVM backend → ir + target + LLVM
```

必须保持以下边界：

- `ast` 不依赖 `sema`、IR 或 LLVM；
- `sema` 不依赖 IR，只输出不可变的 `SemanticModel`；
- `ir` 不依赖 AST、Parser、Sema 或 LLVM；
- `frontend::IrGenerator` 同时依赖 Sema 和 IR，承担唯一的语义层到 IR lowering 边界；
- RuntimeWorld 和 LLVM 后端只接收验证后的 Closed InkIR；
- 不解释 CST 或 AST；普通运行、编译期执行和 AOT 必须共享 InkIR 语义；
- 前端类型系统不得使用 LLVM Type 作为语言类型身份。

## 4. CompilationSession 与 Core 基础设施

`CompilationSession` 集中拥有一次编译的长期上下文：

- `SourceManager`；
- identifier/string interner；
- AST arena；
- canonical `TypeContext`；
- symbol、scope 和 semantic side tables；
- `TargetContext`；
- 诊断集合；
- IR context/module builder。

应增加强类型 ID，避免在模块边界使用可互换的裸 `std::size_t`：

```text
SourceFileId
AstDeclId
AstExprId
AstStmtId
AstPatternId
TypeId
SymbolId
ScopeId
IrFunctionId
IrBlockId
IrValueId
```

ID 只在所属 context 内有效，序列化时再映射为规范 table ordinal 或稳定 identity。不得把 arena 地址、容器迭代顺序或宿主 pointer 用作稳定身份。

`SourceRange` 继续只表示文件内半开字节区间 `[Start, End)`。`SourceFileId` 由所属 AST file、token buffer、diagnostic 或 compilation context 携带；不得把范围改名为 `SourceLocation`。可被 tokenizer、parser、AST 和 Sema 复用的源码与诊断类型继续放在 `ink/core`。

## 5. AST 结构

不建议建立层级很深、依赖虚函数的面向对象 AST。推荐使用 arena、强类型 ID 和紧凑 tagged payload：

```text
AstContext
  Declarations : Vec<Declaration>
  Expressions  : Vec<Expression>
  Statements   : Vec<Statement>
  Patterns     : Vec<Pattern>
```

AST 节点只保存：

- `AstKind`；
- `SourceRange`；
- 必要的子节点 ID；
- 必要的 interned identifier 或 literal token identity；
- 可选 CST origin；
- 错误恢复所需的 `ErrorDecl`、`ErrorExpr`、`ErrorStmt` 或 `ErrorPattern`。

AST 不复制 Trivia，也不直接保存解析后的类型、符号或 IR value。类型、名称解析、常量值和 value category 放入语义 side tables：

```text
SemanticModel
  DeclSymbols[AstDeclId]
  ExprTypes[AstExprId]
  ExprCategories[AstExprId]
  ResolvedNames[AstExprId]
  ConstantValues[AstExprId]
  DeclScopes[AstDeclId]
```

初期只保留“紧凑 AST + side tables”。不要同时建立内容高度重复的 AST、Typed AST 和 HIR。后续 comptime/generic 所需的 source-backed normalized template，应在 staging 阶段按 IR 规范建立，而不是提前复制整棵语法树。

### 5.1 CST lowering

`CstLowering` 接收 `parser::ParsedFile` 和 `SourceFileId`，生成 AST file。它必须：

- 覆盖现有全部 `CstKind`，尚未启用的语义功能仍应安全生成 AST；
- 把 parser error/missing token 结构 lowering 为错误占位节点；
- 保留准确 `SourceRange` 和必要 origin；
- 不执行名称绑定或类型检查；
- 不因错误或尚未实现的语义功能触发断言；
- 提供确定性 AST dump 和 AST verifier。

## 6. TypeContext

类型系统独立于 Sema 和 IR，由 `TypeContext` 统一 canonicalize。相同结构类型必须获得相同 `TypeId`，不能由不同分析路径重复创建。

首批类型只实现：

- `ErrorType`；
- `unit`；
- `bool`；
- `i32`；
- function type。

后续再增加整数族、浮点、tuple、array、slice、pointer/reference、nominal type、runtime type、meta/dependent type。语言类型身份不包含 LLVM Type；目标布局由独立 `TargetContext` 查询。

## 7. 语义分析 pass

语义分析不应实现为一个同时完成全部工作的巨型递归函数。推荐拆分为以下 pass：

### 7.1 DeclarationCollector

- 建立顶层和局部 scope；
- 收集函数、参数和绑定声明；
- 建立 `DeclId -> SymbolId` 映射；
- 检查同一作用域内的重定义；
- 创建重载集合的基础结构；
- 保持源码顺序，不依赖 hash-map iteration 生成诊断或稳定 dump。

### 7.2 SignatureResolver

- 注册 builtin 类型；
- 解析所有函数参数与结果类型；
- 建立 canonical function type；
- 校验默认实参的基本位置规则；
- 在检查任一函数体前完成同一 module 的函数签名，使递归和前向调用可解析。

### 7.3 NameResolver

- 解析表达式名称；
- 实现 lexical scope 与遮蔽；
- 区分 value、type 和 declaration namespace；
- 记录唯一 `ResolvedName` 或稳定的 unresolved/ambiguous 状态；
- 不直接生成 IR。

### 7.4 TypeChecker

- 计算表达式实际类型与期望类型；
- 记录 place/value category、可写性和 constness；
- 验证赋值、调用实参、函数结果和 return；
- 实现首个纵切片所需的最小转换规则；
- 产生 `ErrorType`/`ErrorValue` 后继续分析，避免级联崩溃。

### 7.5 ControlFlowChecker

- 验证所有 required return path；
- 验证 `break`、`continue` 的目标；
- 标记明显不可达结构；
- 为后续 CFG lowering 提供结构化控制信息；
- 首阶段不承担 RAII、异常或 async cleanup 推导。

### 7.6 SemaVerifier

Sema 完成后必须验证：

- 每个非错误表达式都有合法 `TypeId`；
- 每个非错误名称引用都有唯一解析结果；
- place/value category 与操作相容；
- 每个函数签名和 return 一致；
- side table 的索引和 owner 均有效；
- 不含 LLVM 类型、IR value 或宿主地址。

成功后返回不可伪造的 `VerifiedSemanticModule`，IR lowering 只接受该类型。

## 8. InkIR 实现结构

IR 内部采用 table ownership，而不是分散的 owning pointer graph：

```text
IrModule
  TypeTable
  ConstantTable
  SymbolTable
  Globals
  Functions

Function
  Regions
  Blocks
  Operations
  Values
```

每个函数从一开始使用 block-argument SSA。可变局部变量在首个纵切片中使用 place/load/store 表示，后续再增加 mem2reg，不要求第一版完成全部 SSA promotion。

### 8.1 IrBuilder

所有 operation 只能通过 `IrBuilder` 创建，lowering 不得直接修改 operation/value vector。Builder 负责：

- opcode 和 stage legality；
- operand、result、region 和 successor 数量；
- result type；
- destination role；
- successor argument；
- opcode-specific payload；
- effect/trait metadata；
- source origin。

Builder 提供方便构造合法 IR 的 API，但不代替 verifier。每个函数生成后运行 function verifier，整个 module 完成后再运行 module verifier。

### 8.2 IrGenerator

`frontend::IrGenerator` 接收 `VerifiedSemanticModule`，维护下列映射：

```text
SymbolId  → IrSymbolId
TypeId    → IrTypeId
AstDeclId → IrFunctionId / IrGlobalId
AstExprId → IrValueId 或 Place
```

表达式 lowering 应显式区分 `RValue` 与 `Place`，不能靠是否恰好生成 pointer 判断 value category。statement lowering 负责构造 CFG block 和 edge；名称绑定、重载解析和类型推导不得在 IR lowering 中重复执行。

### 8.3 Staged 与 Closed 边界

即使首个纵切片没有 comptime，仍应保留不同的阶段能力类型：

```text
UnverifiedStagedModule
→ verifyStaged
→ VerifiedStagedModule
→ closeAndVerify
→ VerifiedClosedModule
```

第一阶段的 `closeAndVerify` 可以接近恒等转换，但不得用 `bool IsClosed` 代替阶段能力，也不得让解释器或 LLVM 后端接收未验证的通用 module。

## 9. 首个纵切片

首批语义和 IR 功能限制为：

- `i32`、`bool`、`unit`；
- 整数与布尔字面量；
- `var` 和 `const` 局部绑定；
- 普通函数、参数、返回值和直接调用；
- 基本整数算术、比较和布尔逻辑；
- `if`、`while` 和 `return`；
- 最小 trap 或输出 intrinsic。

首批 IR 操作覆盖：

- 常量；
- 整数/布尔运算；
- place 分配、load 和 store；
- direct call；
- branch、conditional branch 和 return；
- trap。

以下功能不进入首个纵切片：

- class、interface、enum；
- pointer、reference、array、slice 和复杂 aggregate；
- RAII、析构和 `defer`；
- exception；
- async/Task；
- reflection、decorator 和 module registration；
- 完整 FFI 与 stable ABI；
- 完整 comptime、generic 和 residualization。

Parser 已支持这些语法，不表示 Sema 必须立即支持。尚未启用的结构应产生稳定的“语义功能未启用”诊断，而不是断言、崩溃或生成不完整 IR。

## 10. IR Schema 生成

在手写大量 opcode switch 前，应把中央 IR registry 建成机器可读的声明源，并由它生成或校验：

- opcode/type/record numeric enum；
- trait、effect 和 stage table；
- renderer metadata；
- verifier dispatch；
- binary codec descriptor；
- 指令参考文档。

推荐使用无第三方依赖的 Python declarative registry 和生成器，不建议让 C++ 构建过程直接解析 Markdown。复杂 type/CFG/semantic 规则仍由手写 verifier callback 实现，但 callback identity、revision 和输入 schema必须由 registry 登记。

在完成机器可读 registry 迁移前，只实现首个纵切片所需的 opcode payload；不得为全部 144 条指令手写互相独立的 enum、printer、codec 和 verifier 定义。现有逐指令参考见 [`draft/IR/11-instruction-reference.md`](../draft/IR/11-instruction-reference.md)。

## 11. RuntimeWorld 参考解释器

IR 最小子集稳定后实现 RuntimeWorld，而不是 AST 解释器。参考解释器至少包含：

- typed runtime value；
- frame 和调用栈；
- block-argument 传递；
- place/local storage；
- 目标整数运算；
- branch、call、return 和 trap；
- step、recursion 和 stack budget。

所有整数溢出、除法、移位和 trap 行为必须读取 Ink/TargetContext 规则，不能直接继承宿主 C++ 的溢出或未定义行为。每个 module 在执行前必须通过 Closed verifier。

## 12. 测试门禁

每个阶段都应建立独立测试，而不是只依赖最终运行结果：

- AST compile-pass 与 malformed CST lowering；
- AST deterministic dump golden；
- Sema compile-pass；
- Sema compile-fail，断言 DiagnosticKind、SourceFileId、SourceRange、Arguments 和 Related；
- Sema side-table verifier；
- IR printer golden；
- IR verifier 对手工构造非法 IR 的拒绝测试；
- RuntimeWorld run test；
- 后续 RuntimeWorld 与 LLVM differential test。

所有 dump 和诊断顺序必须确定，不得依赖 pointer、unordered container iteration、worker completion 或 arena allocation address。

## 13. 推荐实施批次

### 批次 1：编译会话基础

- `SourceFileId`、`SourceManager`；
- string interner；
- typed ID 基础设施；
- `CompilationSession`；
- 多文件诊断归属；
- 对现有 tokenizer/parser API 的最小适配。

完成门槛：同一 source file 在 session 内拥有稳定 ID，diagnostic 能关联文件和文件内 SourceRange，现有 tokenizer/parser 测试全部通过。

### 批次 2：CST 到 AST

- AST arena 和节点 schema；
- 全部现有 `CstKind` 的安全 lowering；
- error placeholder；
- AST verifier；
- deterministic AST printer 和 golden。

完成门槛：合法/错误 CST 都可 lowering，未启用语义结构不会崩溃，相同输入重复 dump 完全一致。

### 批次 3：最小语义分析

- builtin type 与 `TypeContext`；
- declaration collection；
- lexical scope/name resolution；
- function signature；
- expression/statement type checking；
- control-flow return check；
- semantic diagnostics 和 Sema verifier。

完成门槛：首个纵切片的完整单文件程序能够通过语义检查；重定义、未解析名称、不可写 place、实参不匹配和返回类型错误具有稳定诊断。

### 批次 4：IR 核心结构

- IR ID/table/owner 模型；
- first-slice opcode payload；
- builder；
- deterministic printer；
- staged/closed verifier capability。

完成门槛：可用测试直接构造合法 IR；所有非法 operand/type/CFG/stage 组合被 verifier 拒绝。

### 批次 5：Sema 到 IR 纵切片

- function/global symbol lowering；
- expression与statement lowering；
- place/load/store；
- direct call；
- if/while CFG；
- Staged 到 Closed 转换。

完成门槛：首个纵切片程序能从源码稳定生成通过 Closed verifier 的 InkIR。

### 批次 6：RuntimeWorld

- frame、stack、value 和 local storage；
- operation dispatch；
- call/branch/return/trap；
- resource budget；
- run tests。

完成门槛：函数、递归、分支和循环可执行，运行结果符合语言和 TargetContext 规则。

### 批次 7：LLVM 薄竖切

- 只为首个 Closed IR 子集实现 lowering；
- LLVM IR verification；
- object/executable 输出；
- RuntimeWorld 与 LLVM O0 differential test。

## 14. 当前最优先工作

最值得立即开始的是以下两组：

1. `CompilationSession + SourceManager + typed IDs + string interner`；
2. `CST → AST + AST verifier + deterministic dump`。

完成后再实现 DeclarationCollector、NameResolver 和 TypeChecker。IR 基础表、schema generation 与 verifier框架可以提前设计，但不要在缺少 `VerifiedSemanticModule` 前开始横向实现全部指令或让 Parser 直接生成 IR。
