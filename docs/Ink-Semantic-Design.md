# Ink Semantic 模块设计

基于 AST 的语义分析、泛型实例化与统一 IR 函数执行

更新：2026 年 10 月 6 日。本文保留分层架构设计；当前已实现显式泛型函数实例化、实例缓存、定义环境快照、跨模块导入和编译期执行，并贯通 IR 归档、字节码与 LLVM AOT。实际支持范围和限制见 [泛型函数](Ink-Generic-Functions.md)。泛型 class、参数包和可重新实例化的模块模板归档仍属后续设计。

本文采用确定的方向：**非泛型函数在定义处检查并生成 IR，编译期调用和普通执行共用该 IR；泛型保留 AST，实例化后再生成具体函数 IR**。当前 [语义分析接口](Ink-Semantic-Analysis.md) 已有模块创建、语句/声明严格分派，以及普通定参函数签名、链接方式、形参作用域和函数体遍历；已支持定参调用、整数/字符串字面量、bool 逻辑与短路、整数及 bool 比较、bool 条件分支、分支返回路径与确定初始化检查及整数/bool 编译期求值。第 1.3 节对象模型、NameResolver 的词法作用域基础和第 6.5 节最小执行模块已实现，其余分析器类及扩展能力仍是建议结构。语言语法以 [Ink-grammar-Rules.bnf](Ink-grammar-Rules.bnf) 为准；本文不增加泛型、反射或声明生成语法。

编译期变量允许修改。**对已经进入的模块或具体函数语义上下文，按源码顺序完成当前语句的必要分析及编译期操作，再处理下一条语句。** 模块级编译期对象持续存活并保持其声明允许的可写性；保存某次求值的不可变结果不等于冻结整个模块。具体执行契约与当前实现边界见第 6 节。

## 1 当前基础与目标边界

### 1.1 当前仓库事实

| 现有设施 | 当前状态及设计影响 |
| --- | --- |
| [parser.h](../src/include/ink/parser/parser.h) 中的 `ParsedUnit` | 持有 token、AST 和恢复记录；泛型 Decl 借用 AST，其所属 ParsedUnit 必须保持存活 |
| [ast.h](../src/include/ink/parser/ast.h) | 具体继承节点通过指针连接；已有 `TypeSyntax`、`GenericApplyExpr`、`FunctionDecl` 等，comptime 复用普通节点的 `isComptime()` 属性 |
| [ast_context.h](../src/include/ink/parser/ast_context.h) | Arena 管理稳定地址的节点和数组；semantic 不接管单个节点的释放 |
| [ASTNodes.def](../src/include/ink/parser/ASTNodes.def) | 节点种类有稳定显式编号；种类编号不是某个声明或实例的身份 |
| [core/context.h](../src/include/ink/core/context.h) | 已有 `CompilationContext`、`FrontendContext`、源码管理、诊断及目标信息，直接复用 |
| [semantic/CMakeLists.txt](../src/lib/semantic/CMakeLists.txt) 与 [lib/CMakeLists.txt](../src/lib/CMakeLists.txt) | 对象模型、NameResolver 和 Analyzer 严格分派骨架已接入构建 |
| [analyzer/analyzer.h](../src/include/ink/semantic/analyzer/analyzer.h) | Analyzer 创建模块并按 AST 宏表分派语句/声明，按源码顺序协调必要的编译期执行；执行失败映射为 Core 的具体 Execution 诊断，按原因区分用户错误与 ICE |
| [execution/engine/execution_engine.h](../src/include/ink/execution/engine/execution_engine.h) | 独立 `ink::execution` 模块，提供执行值、帧、受控存储、带显式分支的 IR 入口执行、共享预算和基础运算；不依赖 semantic |
| [inkc/main.cpp](../src/tools/inkc/main.cpp) | 当前仅处理命令行参数；以下流程仍需接入驱动 |

### 1.2 目标流水线

```text
.ink 源码
  → tokenizer / parser
  → ParsedUnit：只读 AST + TokenBuffer + 源码
  → SemanticDriver
      ↔ 声明登记 / 名称解析 / 类型与调用检查
      ↔ GenericInstantiator：AST + 泛型绑定 → 实例语义结果
      ↔ ComptimeEvaluator：显式 comptime 表达式/语句 → 编译期结果或静态展开
      ↔ ExecutionEngine：具体函数 IR + 实参 → 编译期调用结果
      → CheckedBody：已确定的绑定、类型、转换、活动结构和清理计划
  → SemanticVerifier
  → RuntimeLowerer
  → Closed InkIR[target] → IR verifier
      ├─ 运行时解释器
      └─ LLVM lowering → 目标文件 → 链接

模块语义产物：声明接口 + 泛型 AST/定义环境 + 具体函数 IR + 常量 + 依赖信息
运行时代码产物：已闭合的普通函数、泛型实例和全局初始化代码
```

语义分析、实例化和编译期执行相互按需请求结果，并非必须先完整检查所有函数体，再统一执行 comptime。例如 `TypeSyntax` 包装的表达式可能调用一个编译期函数；这个函数又可能使用某个泛型实例。按需请求必须遵守当前源码处理顺序，不能为了填充查询缓存而提前执行后续语句的编译期副作用。

这条路径不要求先建立 TemplateIR 或 Staged InkIR。`CheckedBody` 是 AST 的语义旁表及少量展开记录，不另建一棵逐节点复制的 Typed AST，也不是交给后端执行的指令集。编译期函数调用和运行时解释器都只执行已生成的具体函数 IR。

### 1.3 已实现的对象模型

当前独立 `ink::ir` 模块由原 `semantic/model` 迁入，提供显式内存、加法、逻辑、比较、调用、分支和返回节点的构造接口；源码分析器已支持基础函数声明和定义。原有 IR/execution 已删除，新建的独立 `ink::execution` 提供显式 comptime 表达式/语句求值所需的基础设施，并通过 `ExecutionValue` 和 `execute()` 统一执行非泛型函数的 IR 及显式分支；下文其他章节仍是分层语义分析方案。当前执行设施的边界见第 6.5 节，完整语义流水线仍按后续阶段逐步接入。

- `IRBuilder::createBranchInstruction(Target)` 和 `createConditionalBranchInstruction(Condition, TrueTarget, FalseTarget)` 生成 void 类型分支，后者要求同一上下文的 bool 条件。对应的 detached 工厂可引用本上下文的函数块或未挂接块；条件分支的两个已挂接目标须属于同一函数。真正插入时，源块和全部目标必须已挂接到同一函数，目标引用不转移所有权。
- `Value::isTerminator()` 识别 Return、Branch 和 ConditionalBranch；`BasicBlock::terminator()` 返回末尾终结节点或空指针。终结指令只能插入函数块末尾，块内至多一个终结指令，其后不可追加节点；普通指令可插在已有终结指令之前。

源码的 bool if/else/else-if 与嵌套分支已通过这些节点生成控制流。普通 if 的两个分支都检查，`comptime if` 仅分析选中分支；每支拥有独立作用域并从同一入口初始化状态开始，汇合时只对继续执行的路径取确定初始化交集。无 else 时包含入口路径，已返回的路径不参与交集或生成汇合跳转；所有路径均返回时，后续语句不可达。运行时 bool 逻辑非与短路、整数及 bool 比较已经接入；源码循环仍未接入。局部变量和短路结果通过显式存储传递，不生成 phi。

`LogicalNotInstruction`、`LogicalAndInstruction` 和 `LogicalOrInstruction` 只接受同一上下文的 bool 操作数；后两者消费已求值的左右值，不改变操作数先前执行的副作用。源码 `&&`、`||` 将左值保存到临时 bool 存储，通过 `ConditionalBranchInstruction` 选择是否进入右侧块，再在汇合块加载结果。普通函数定义处仍检查两侧类型；执行时左侧仅执行一次，右侧仅在需要的路径执行。嵌套短路按生成后的当前块接续控制流。

`IRBuilder::createCompareInstruction(Predicate, Left, Right)` 和对应 detached 工厂生成 bool 结果。`ComparisonPredicate` 包含 `Equal`、`NotEqual`、`Less`、`LessEqual`、`Greater`、`GreaterEqual`；同型整数接受全部六种谓词，bool 仅接受前两种。整数比较保留类型的符号属性和任意 IR 位宽，不通过宿主整数窄化；操作数必须属于同一上下文且类型完全相同。浮点、指针及其他类型比较被显式拒绝。

- `IRBuilder::createFunction()` 根据签名创建 `FunctionParameter`，通过 `parameters()` 访问；形参通过 `outer()` 关联函数，`function()` 从该父节点取得所属函数，不再重复保存 Owner；同时保存 `Name ParameterName`（通过 `name()` 访问）、零起始索引、值类型和 `ParameterKind`（Positional、Named、Variadic），通过 `parameterKind()` 查询。`IRBuilder::createFunction()` 的可选种类列表必须与签名槽位数量一致，省略时全部为 Positional；第四个可选参数 `ParameterNames` 按签名顺序提供名称，省略时参数匿名，由调用方驻留并填写形参名；种类是绑定元数据，不改变规范化运行时签名或开启变参展开。`createAddInstruction()` 接受同型整数并定义按位宽回绕的加法；`IRBuilder::createDetachedReturnInstruction(ReturnedValue)` 创建未挂接的 void 类型终结节点，只校验操作数归属和非 void 类型；`IRBuilder::appendValue()` 校验目标块属于函数且返回值匹配该函数签名。返回指令不保存 Owner，`function()` 沿 outer → BasicBlock → Function 查询，未挂接时返回空指针。工厂不检查整体控制流；源码 if 的返回路径及确定初始化检查已实现，源码循环也支持跳转、返回路径及确定初始化检查。

IR 公共头位于 `src/include/ink/ir`，实现位于 `src/lib/ir`，统一使用 `ink::ir` 命名空间；Value、类型、常量、函数、指令、名称池、Decl/泛型声明及其 AST 关联和 IRBuilder 均归 IR。独立 [`IRContext`](../src/include/ink/ir/context.h) 借用 Core 编译上下文，拥有名称、类型、常量和根模块，各模块拥有声明树及 IR 树。[`SemanticContext`](../src/include/ink/semantic/context.h) 组合 IRContext 和 ScopeStore，通过 `irContext()` 供 Analyzer 构造 IR；Analyzer、名称解析和作用域保留在 semantic。

- `ir/coredefines.h` 集中定义 `ValueKind`、`TypeKind`、`ParameterKind`、`CallingConvention`、`LanguageLinkage`、`FunctionBinding`、`AccessKind` 和 `VisibilityKind`，仅依赖 `<cstdint>` 与枚举注册表，可独立包含。`VisibilityKind::Public/Private` 保存函数的源码可见性：顶层默认 Public，显式 private 和局部函数为 Private。import 绑定和模块成员访问检查目标函数权限，字节码生成保留这些权限；成员类型等尚未实现的声明继续由对应语义阶段报告。
- 源码可见性、ABI 和原生方向分别保存。`import "C"` 使用 `FunctionBinding::Import` 且没有函数体，顶层 `export "C"` 使用 Export 且必须有函数体，`[abi("C")]` 只将 Local 定义设为 C ABI。普通本地定义和仅带 ABI 的定义允许按参数列表重载，原生导入导出名称不允许重载。`private export` 不开放 Ink 模块访问，但允许已纳入同一程序的原生导入按名字和签名匹配；重复导出和签名不匹配分别报告 `INK-S0049`、`INK-S0050`。原生导入不自动加载 Ink 源文件，也不加载新的宿主动态库；当前没有原生 DLL/SO 生成或可传给 C 的本地回调地址。
- `ir/Values.def` 生成 `ValueKind`，以 C++ 类名标识实际对象，如 `IntegerType`、`FunctionType`、`CallInstruction`；类型和常量条目分别生成 `Type::classof()`、`Constant::classof()`，具体类的 `classof()` 直接检查同名值种类。元类型、void、bool、label、module 共用实际类 `BuiltinType`，通过 `TypeKind` 区分。`Type`、`UserDefinedType`、`Constant` 仅作为中间基类，不单独占用值种类。
- `ir/type/Types.def` 是类型种类与基类分类的注册表，每条记录为 `INK_IR_TYPE(Name, Base)`，`Base` 为 `BuiltinType` 或 `UserDefinedType`。`TypeKind` 和两个基类的 `classof()` 从同一张表生成；`FunctionType` 定义在 `ir/function/function_type.h`，其余具体类型类体与构造定义在 `ir/type` 的独立头文件中，其继承关系应与注册表一致。
- `Name` 是一个 32 位池内索引，`NamePool` 为同名字节串只保存一份内容；池扩容保持名称与字符串视图稳定。空输入和索引耗尽返回无效名称。名称相等和哈希仅在同一池内有意义；索引本身不携带池身份，无法检测恰好落在另一池有效范围内的外来索引。词法验证与 NFC 处理仍由 tokenizer 负责。
- `ConstantPool` 由 `IRContext` 独占，通过 `constantPool()` 访问；当前驻留 bool、任意位宽整数、`StringConstant` 和 `FloatConstant`，常量获取统一通过 `Context.constantPool().getXXXConstant(...)`，Context 不提供转发接口。池在基础类型创建后初始化，并在类型存储销毁前释放；false、true 预先创建，其他常量按规范类型身份与完整 payload 先查找、未命中才分配，哈希碰撞后继续精确比较。外来类型和 payload 与类型不匹配的请求返回空指针且不改变池。`size()` 包含两个 bool 常量，`owns()` 检查具体对象归属，池与常量地址在上下文生命周期内保持稳定。整数和字符串源码字面量已接入分析器；聚合常量、浮点字面量及后端 lowering 仍待实现。
- `StringConstant` 的类型固定为本上下文的只读 `u8` 切片；`ConstantPool::getStringConstant(SliceType, Payload)` 复制调用方已验证、解码的 UTF-8 字节，以完整字节序列比较，支持空串、内嵌 NUL 和非 ASCII 内容，不做转义解码或 Unicode 规范化。存储保证在完整 payload 后附加一个 NUL，空串也有终止符。`value()` 返回池拥有的稳定 `std::string_view`，长度和常量身份均不包含额外终止符；`nullTerminatedValue()` 返回包含该终止符的完整存储视图，供后续 lowering 使用。`tryGetCString()` 在 payload 不含 NUL 时零拷贝返回稳定的 `const char *`；payload 中任何位置含有 NUL（包括结尾）时返回 `nullptr`，显式报告无法无损表示为 C 字符串，不截断、不拒绝原 Ink 常量。底层 `std::string` 自带的终止符提供这一存储保证；这些指针是宿主接口，源码字符串常量的 C 调用实参转换已通过 CStringInstruction 表示独立可写、NUL 结尾的副本，生命周期为调用者函数的一次执行；目标存储的后端 lowering 仍待接入。不同源码位置或不同转义拼写只要解码内容相同就复用常量；AST 节点仍独立保存来源。
- `IntegerConstant` 保存自有 `IntegerBits`，由位宽与低位字在前的 `uint64_t` 字数组组成；符号性由 `IntegerType` 决定，负数使用二进制补码。单字构造允许显式零扩展到宽于 64 位的表示，多字构造复制全部输入；`valid()` 检查非零位宽、精确字数与最后一个字的未用高位，常量池拒绝无效表示及类型宽度不匹配，不静默丢弃或补齐输入字。
- `FloatConstant` 保存自有 `FloatBits`，由 IEEE binary16/32/64 位宽与 `uint64_t` 原始位模式组成；`valid()` 拒绝不支持的位宽及编码之外的高位。`ConstantPool::getFloatConstant(FloatType, Payload)` 验证表示有效且位宽与类型匹配，再按完整位模式驻留，区分正负零、无穷、NaN 符号、静默/信号位与 payload。输入必须已经采用对应 IEEE 格式编码，不通过宿主浮点类型转换。十进制字面量解析、浮点运算、格式转换、舍入和溢出诊断由后续语义分析负责；池只保存位表示。
- `Value` 是 IR 值基类，当前分支为 `Type`、`Constant`、`Function`、`BasicBlock`、`Module`、`CallInstruction`、`AllocaInstruction`、`LoadInstruction`、`StoreInstruction`、`FunctionParameter`、`AddInstruction`、`LogicalNotInstruction`、`LogicalAndInstruction`、`LogicalOrInstruction`、`CompareInstruction`、`BranchInstruction`、`ConditionalBranchInstruction` 和 `ReturnInstruction`。`Type` 下分 `BuiltinType` 与 `UserDefinedType`：builtin 提供元类型、void、bool、整数、IEEE binary16/32/64 浮点、定长数组、切片、指针、引用和函数类型；user-defined 提供 `ClassType`、`EnumType` 与 `InterfaceType`。所有类型值的类型是元类型，元类型的类型为自身。结构类型和常量在上下文内规范化；名义类型拥有独立身份，由分析器复用同一类型或实例的对象。不同上下文的对象不可直接混用。IR 的接口、实现与测试使用项目自有模型及标准库，独立目标 `ink_ir`（`ink::ir`）依赖 `ink::core` 与 `ink::parser`，因为 Decl 保留 AST 关联；`ink_semantic` 单向依赖 `ink::ir`；LLVM IR 类型转换限定在后端适配层。
- `Value` 统一保存所属 `IRContext` 和自身类型 `const Type &ValueType`，派生类通过继承的 `context()`、非虚 `type()` 查询。类型在构造时确定，派生类不重复保存自身类型，也不沿操作数链递归推导；元类型的自身类型指向自己。`Function::functionType()` 提供函数签名访问，数组元素类型、函数返回类型等类型结构成员仍由具体类型保存。
- `Value::outer()` 返回非拥有的结构父节点指针；父节点通过 `unique_ptr` 拥有子节点。IRContext 拥有共享类型、常量和根 Module，SemanticContext 另持有作用域存储，Module 分别拥有声明树根和 IR 入口块，Function 拥有参数和函数块，BasicBlock 拥有块内节点。未挂载节点由调用方的 `unique_ptr` 拥有；调用目标、实参、类型和解析器绑定均为借用引用，不参与所有权。Context 必须比所有借用它的节点活得更久，节点析构通过 IR 的 LifetimeObserver 通知 ScopeStore 清理名字绑定；删除节点前调用方仍须处理操作数和 Builder 插入点；目前没有 use-def 自动修复。
- `Value`、`Type`、`BuiltinType`、`UserDefinedType`、`Constant` 和 `Decl` 的基类构造函数使用 `protected`，字段保持 `private`；派生类无需逐个列入基类友元名单。函数、参数、基本块、模块、声明和指令的私有构造函数授权 `IRBuilder`，具体类型和常量分别授权 `TypePool`、`ConstantPool`。IRBuilder 负责节点创建、所有权转移和父子关系维护，并独占 Value、BasicBlock 等节点的结构写权限；Context 只持有共享存储和根模块。
- `ArrayType` 的规范键为元素类型和 64 位长度，多维数组通过嵌套 `ArrayType` 表示；`SliceType` 表示具有运行期长度的视图，不拥有动态容器的分配策略。`PointerType`、`ReferenceType` 和 `SliceType` 的规范键都包括目标类型和 `AccessKind`，且三种类型使用不同存储。访问权限描述间接访问；源码绑定的可变性由语义分析器单独检查，`ReferenceType` 不替代表达式的值/位置类别。即使元素或目标是用户定义类型，这些语言内建类型构造器仍归 `BuiltinType`。
- `IRBuilder` 提供名义类型、声明、模块、函数、基本块及指令的 `createXXX` 入口；`IRContext` 持有共享存储、Pool 访问器和根模块查询，`SemanticContext::irContext()` 提供语义分析所用的 IR 上下文；所有权编辑及插入点校验统一由 IRBuilder 负责。函数和无参基本块工厂返回未挂载的 `unique_ptr`，根模块交给 Context 持有，声明根交给 Module 持有，子声明交给父声明持有，名义类型交给 TypePool 持有；这些创建操作均不使用或改变指令插入点。
- `ink::ir::IRBuilder`（`ink/ir/ir_builder.h`）的 `createCallInstruction`、`createAllocaInstruction`、`createLoadInstruction`、`createStoreInstruction`、`createAddInstruction`、`createLogicalNotInstruction`、`createLogicalAndInstruction`、`createLogicalOrInstruction`、`createCompareInstruction` 和 `createReturnInstruction` 按插入点创建指令，成功后将所有权移交给 BasicBlock 并返回借用指针。先调用 `setInsertPoint(Block)` 选择块末尾，或用 `setInsertPoint(Before)` / `setInsertPoint(Block, Before)` 选择锚点；连续创建保持调用顺序，Alloca 不自动移动到入口块。对应的 `createDetachedXXXInstruction` 工厂返回未挂载的 `unique_ptr`，忽略且不改变插入点，由调用方通过 IRBuilder 的显式编辑接口转移所有权。
- `FunctionType` 保存已确定的固定参数签名，`TypePool::getType<TypeKind::Function>(ReturnType, ParameterTypes)` 按返回类型和有序形参类型身份驻留，哈希命中后仍精确比较完整签名。返回和形参类型必须属于当前上下文，形参不能为空；形参列表复制为类型自身拥有的存储，支持零参数、void 返回值、名义类型及嵌套函数类型。参数名、默认值、参数包和泛型绑定不放入这份签名，参数类型的语言合法性仍由分析器检查。
- `Decl` 是 Module 拥有的独立声明树节点，保存名称、借用的只读 AST、所属模块和非拥有的父声明指针；`module()`、`parent()` 查询归属，模块声明根的父声明为空。节点使用 `std::vector<std::unique_ptr<Decl>> Children` 拥有子声明，`children()` 返回只读容器，元素通过 `.get()` 借用。`IRBuilder::createModuleDecl(Module &, ModuleAST)` 为本地模块设置唯一声明根并使用模块名称，重复设置或外来模块返回空指针；`createFunctionDecl(Decl &Parent, Name, AST)`、`createClassDecl(Decl &Parent, Name, AST)` 创建后立即挂入本地父声明，要求有效名称和泛型 AST，失败不改变树。派生类不增加字段，基类 `ast()` 返回 `ASTNodeBase`，派生类返回具体 AST，`classof()` 从 AST 分类。声明不属于 `Value`，不保存已解析类型、初始化值或实例状态，IR 声明树暂不提供 `VarDecl`。AST 所属 `ParsedUnit` 必须比声明所属 Module 活得更久。
- `Function` 是具有已确定 `FunctionType` 和独立身份的函数值，由 `IRBuilder::createFunction(Name, Signature)` 创建，供普通函数和闭合泛型实例使用。`CallInstruction` 保存函数类型的 `Value` 引用及实参引用列表；`directCallee()` 识别 `Function`，其他函数值由 `indirectCallee()` 返回，`callee()` 提供统一入口。`functionType()` 返回签名，`type()` 返回签名的返回类型。工厂拒绝非函数目标、外来上下文对象、空实参及数量或类型不匹配。泛型 `FunctionDecl` 不能直接调用，须先实例化得到闭合函数值。每次调用独立分配，列表和对象地址在扩容后保持稳定；创建不执行函数，也不进行重载选择、参数转换或泛型实例化。
- `Function` 使用 `std::vector<std::unique_ptr<BasicBlock>>` 拥有函数块，使用 `std::vector<std::unique_ptr<FunctionParameter>>` 拥有参数；`blocks()`、`parameters()` 返回只读容器，元素通过 `.get()` 借用。空块列表表示没有函数体，`entryBlock()` 返回首块的借用指针。`IRBuilder::createBasicBlock(Function &)` 创建并直接交给函数拥有，设置 `Outer` 后返回借用指针；首块自动成为入口。`IRBuilder::createFunctionBody(Function &)` 仅创建首块，已有函数体或外来函数返回空指针。函数销毁时先销毁函数块，再销毁参数；列表顺序不代表执行顺序，块内指令通过 `IRBuilder` 构建。源码 if 的控制流连接、返回路径及确定初始化检查已实现；源码循环也支持跳转、返回路径及确定初始化检查。
- `BasicBlock` 使用 `std::vector<std::unique_ptr<Value>>` 拥有有序子节点，`values()` 返回只读容器。`IRBuilder::createBasicBlock()` 返回未挂载的 `std::unique_ptr<BasicBlock>`。`IRBuilder::insertValue(Block, std::move(Child), Before)` 在指定直接子节点前转移所有权，Before 为空时追加；`IRBuilder::appendValue(Block, std::move(Child))` 是末尾追加入口。这些显式编辑接口使用参数指定的块和锚点，不读取或改变 Builder 当前的插入点。成功清空调用方的 owner 并设置 `Outer`，失败保留 owner 与原块内容；拒绝空 owner、外来对象、非法锚点、已有父节点、循环包含、类型和常量。Return、Branch 和 ConditionalBranch 都是终结指令，只能位于函数块末尾；已有终结指令后不能追加节点，但可在终结指令前插入普通指令。Return 必须匹配所属函数的返回签名，分支目标必须属于同一函数。`IRBuilder::removeValue(Block, Child)` 摘除直接子节点、清空其 `Outer` 并返回 `std::unique_ptr<Value>`；失败返回空 owner。`IRBuilder::eraseValue(Block, Child)` 则立即销毁该子树。摘除后的节点可重新挂载，父节点销毁会递归释放仍属于它的子树。
- `Module` 使用 `std::unique_ptr<ModuleDecl>` 和 `std::unique_ptr<BasicBlock>` 分别拥有独立的声明树与 IR 入口块，销毁时先释放 IR，再释放声明树。`declarationRoot()` 返回借用指针，尚未创建声明根的程序化模块返回空指针。`IRBuilder::createModule(Name)` 校验名称后创建 Context 拥有的根模块，返回借用指针；`modules()` 提供根模块的只读拥有容器，`entryBlock()` 借用入口块，入口块的 `Outer` 指向 Module。`IRBuilder::removeModule(Module)` 从根列表摘除并返回 `unique_ptr<Module>`，随后可交给某个 BasicBlock 实现嵌套；`IRBuilder::appendModule(std::move(Owner))` 将未挂载 Module 重新交给 Context，兼容 `IRBuilder::removeValue()` 返回的基类 owner，失败不消费所有权。`IRBuilder::eraseModule(Module)` 销毁根模块及其两棵子树；模块所有权转移不改变声明归属或节点地址。模块仍使用 `TypePool::getType<TypeKind::Module>()`，与借用文件 AST 的 ModuleDecl 分开。
- `IRBuilder::createClassType(Name)`、`IRBuilder::createEnumType(Name)` 和 `IRBuilder::createInterfaceType(Name)` 直接创建具体名义类型，不经过语义 `Decl`。对象地址代表类型身份，同名对象不合并；分析器负责按普通类型或泛型实例身份复用已经创建的对象，避免仅按名字或共享 AST 合并不同实例。Class 字段通过 defineClassType() 补全，成员方法由语义层登记并降低为带 this 指针的普通函数；共享 TypeLayout 提供对象布局。基类、枚举底层类型、接口约束和泛型实例缓存仍待实现。组合类型工厂验证上下文归属与访问权限，不在存储层决定数组元素合法性、大小限制、引用折叠或可空性等语言规则。
- `createAllocaInstruction(AllocatedType)` 创建单对象、未初始化的分配指令，结果为对应的可读写指针；数组通过 `ArrayType` 表示。当前支持 bool、整数、浮点、指针、引用、切片及这些类型组成的定长数组，拒绝外来类型、元类型、void、label、module、原始函数签名和布局未完成的名义类型。`createLoadInstruction(Address)` 从指针读取并产生元素类型的值，接受只读或可读写指针；`createStoreInstruction(Address, StoredValue)` 要求可读写指针、同一上下文和完全一致的元素类型，结果类型为 void。对象存活期间地址保持稳定，每次创建均有独立身份，普通指令工厂按 IRBuilder 的插入点安排执行顺序，未挂载版本通过 IRBuilder 的显式编辑接口转移所有权；构建节点不会执行内存操作，也不检查初始化、支配关系或实际地址有效性。
- 执行模型不再保存源码变量绑定对象。源码变量和形参与模型对象的绑定、const 分析仍待实现。初始化和赋值由各自位置的 `StoreInstruction` 表示，读取使用 `LoadInstruction`；`StoreInstruction` 在构造时将自身类型设置为上下文唯一的 void 类型，由 `Value::type()` 返回。
- 声明借用的 AST 所属 `ParsedUnit` 必须比声明所属 Module 活得更久，当前上下文和模块不拥有或复制 AST。源码中的普通 `parser::VarDecl`、`parser::FunctionDecl` 和 `parser::ClassDecl` 是语法节点，不意味着创建同名语义 `Decl`。执行期变量槽位、编译期结果及语义分析状态分别保存，不能写回共享泛型 AST。

名字解析代码位于 `name_resolve` 目录，`Scope`、`Binding<T>`、`ScopeStore` 和 `NameResolver` 已拆分为独立类型和头文件。`SemanticContext::scopeStore()` 持久保存作用域、绑定、成员及定义作用域索引；每次分析由 `AnalysisState` 拥有独立 resolver，resolver 可从已有作用域恢复查找，销毁时不释放这些数据。`Binding<T>` 仅支持 `Value *` 和 `Decl *`；两类绑定共享词法名字空间及遮蔽规则，普通函数与泛型函数候选按类别保存。`NameResolver` 提供作用域进入与退出、有类型查找、实体成员作用域、直接成员查找、重载候选及泛型定义首次绑定作用域；详见 [语义分析接口](Ink-Semantic-Analysis.md)。这一阶段已接入基本表达式名字和定参调用分析，尚未提供完整 `LookupResult`、`ExprInfo`、comptime 或后端 lowering。下文的 ID、类型独立存储门面、扩展常量种类和会话结构仍为后续接口规划；当前类型、常量和声明引用使用对象存活期间的稳定指针；类型与常量随 Context 存活，声明随所属 Module 存活，指针不能直接持久化。

## 2 所有权、身份和上下文

### 2.1 生命周期

`SemanticSession` 借用已有 `core::CompilationContext`，持有本次编译的语义模块、声明、作用域、类型、常量、实例和查询缓存。Core 上下文必须比 Session 活得更久。

`SemanticModule` 拥有本模块的 `ParsedUnit` 或反序列化得到的等价 AST 单元，以及该模块的定义环境和语义结果。Session 结束前不卸载仍被实例或语义结果引用的模块。销毁时先释放查询、实例与语义旁表，再释放 AST 单元；诊断中需要延迟输出的数据必须拥有自己的存储或持有有效源码引用。

Parser 发布 AST 后，semantic 只通过只读接口访问。当前 Parser API 仍有可写入口，冻结是 semantic 的使用契约，不应把它描述为现有类型系统已经完全强制的限制。

### 2.2 身份类型

| 类型 | 表达的身份 |
| --- | --- |
| `ModuleId`、`UnitId` | 当前会话中的模块和 AST 单元 |
| `NodeRef` | `UnitId` 加只读节点指针；只在节点所属单元存活期间有效 |
| `DeclId` | 泛型语义定义；普通函数、名义类型和变量使用各自的对象身份 |
| `GlobalDeclRef` | 跨模块的声明身份：模块身份、接口版本及模块内声明身份 |
| `ScopeId`、`BindingId` | 词法作用域和具体名称绑定；同名局部变量必须可区分 |
| `TypeId`、`ConstValueId` | 会话内规范化的类型和不可变常量 |
| `SubstitutionId` | 不可变的泛型参数及必要外层编译期绑定 |
| `InstanceId` | 某个泛型声明在一组规范实参下的实例 |
| `SemanticContextId` | 定义环境、替换环境、所属实例和静态展开上下文的组合 |
| `BodyId`、`ExpansionId` | 一份函数体语义结果和一次编译期结构展开 |
| `ObjectId` | 一次编译期执行中的受控内存对象；不能作为持久化身份 |

这些是强类型句柄或值记录，不必都实现为拥有资源的类。会话内整数 ID 和宿主指针不能直接写入模块文件；持久化时映射为存档局部编号或稳定的 `GlobalDeclRef`。

### 2.3 三种环境必须分开

| 环境 | 内容与用途 |
| --- | --- |
| `DefinitionEnvironment` | 声明定义时的模块、词法作用域、导入绑定、访问权限和必要外层绑定；决定函数体中的名字从哪里查找 |
| `Substitution` | 泛型参数对应的类型、编译期值及参数包；决定这一次实例化的具体语义 |
| 编译期执行环境 | 模块帧保存持续可写的模块对象，函数分析帧保存本次语义处理的编译期局部对象，调用帧保存一次实际 comptime 调用的参数和局部对象；决定当前执行读到什么值 |

同一函数 AST 可以对应多个实例；同一实例又可以有多次调用。`F::[i32]` 与 `F::[i64]` 的类型检查结果分开保存，而同一个 `F::[i32]` 分别以 `1`、`2` 调用时，只需分开调用帧，不因普通实参值不同而创建新的泛型实例。

如果某个编译期绑定确实改变成员集合、类型或语句展开，它必须进入新的替换或展开上下文。不能把这类变化隐藏在可变调用帧中，却继续复用旧的类型检查缓存。

当类型计算或泛型应用读取某个帧内值时，先将这次使用需要的值冻结成不可变绑定，再创建相应语义上下文。冻结的是本次读取的结果，模块变量和局部变量仍按各自声明允许修改。调用帧本身不进入持久缓存键；之后修改变量，也不能改变已经发布的实例或类型。

## 3 类结构与具体职责

以下分析器类位于建议的 `ink::semantic` 命名空间；第 3.5 节的帧、存储、预算及基础运算由独立 `ink::execution` 承担。表中的入口描述逻辑接口，具体返回类型统一遵守第 8 节的显式失败契约。

### 3.1 会话、模块和调度

| 类 | 具体职责 | 主要入口或输出 |
| --- | --- | --- |
| `SemanticSession` | 统一拥有语义存储、模块和缓存；借用 Core 的目标、源码及诊断设施；保证句柄和 AST 生命周期 | `getModule()`、`types()`、`constants()`、`queries()` |
| `SemanticModule` | 保存某一模块的 AST 单元、导入/导出索引、模块作用域、定义环境、持久编译期模块帧、求值结果快照和函数体结果 | 声明索引、`DefinitionEnvironment`、`CheckedBody` |
| `SemanticDriver` | 语义层入口；按源码顺序协调模块分析、必需实例、comptime 请求和最终验证；当前语句处理完成后才推进下一条 | `analyzeModule()`、`completeModule()` → `SemanticResult` |
| `SemanticModuleLoader` | 按模块身份加载源码或语义产物；维护加载中、接口可用、完成、失败状态；按需加载函数体 | `loadModule()`、`loadBody()` |
| `SemanticQueries` | 统一缓存声明头、类型、函数体、布局等确定性查询；登记依赖、检测环、区分合法递归与错误循环 | `resolveDecl()`、`resolveType()`、`checkBody()`、`layoutOf()` |

语义层请求 IR 生成并持有 ExecutionEngine，泛型实例及语义旁表仍归 semantic。ExecutionEngine 复用 `IRContext` 中的类型和常量，但不反向请求 Analyzer；AST 求值与语义查询之间的协调由语义层负责，避免 `semantic → execution → semantic` 的模块依赖环。

### 3.2 基础存储

| 类 | 具体职责 | 保存的关键数据 |
| --- | --- | --- |
| `DeclStore` | 仅为泛型函数和泛型类保存不可变定义；分析及实例状态放在独立旁表 | 名称、借用的只读 AST；普通对象由对应模型存储拥有 |
| `ScopeStore` | 已实现 Context 拥有的作用域、绑定、成员及定义作用域索引；可见条件和导入来源待扩展 | 当前为 `rootScope()`、`memberScope()`、`definitionScope()`；后续扩展可见性边界 |
| `TypeContext` | 规范化内建、名义、复合、函数及元类型；处理类型相等、完整性和目标布局查询 | `TypeId`、类型结构、名义声明身份、布局状态 |
| `ConstantPool` | 驻留不可变的标量、聚合、类型值及允许持久化的符号常量，提供规范相等和哈希 | `ConstValueId`、类型、规范值；不保存可变局部对象 |
| `SemanticInfo` | 按语义上下文保存 AST 的绑定、表达式性质、调用/转换计划和函数体结果 | `ExprInfo`、`CallPlan`、`ConversionPlan`、`CheckedBody` |

当前规范化类型及元类型统一由 `ir::Type` 和 `ir::TypePool` 表示。后续的 `TypeContext` 是语义查询与实例化状态门面，不重新拥有一份相同的类型模型；未完成名义类型和依赖类型的状态需在扩展时明确归属。`RuntimeLowerer` 复用已确定的 IR 类型身份；目标布局规则只能有一个权威实现。

`ConstantPool` 和 `EvalMemory` 也不能合并。前者内容不可变、可比较并可进入缓存键，后者有对象身份、别名、初始化和销毁过程。

### 3.3 名称、类型与普通语义检查

| 类 | 具体职责 | 主要入口或输出 |
| --- | --- | --- |
| `DeclCollector` | 在允许的作用域登记源码名称和绑定；只有泛型函数及泛型类创建语义 `Decl`，记录尚未激活的区域 | `collectScope()`、`registerDeclaration()` |
| `NameResolver` | 已实现词法作用域、实体成员作用域、绑定及重载候选集合；Analyzer 已实现模块函数导入及可见性检查；继承成员及其他访问权限检查待扩展 | 当前为 `bind()`、`lookup()`、`lookupLocal()`、`lookupMember()`；后续扩展 `LookupResult` |
| `DeclAnalyzer` | 检查泛型形参、函数签名、基类/接口、字段、全局初始化和属性；按需完成声明 | `analyzeHeader()`、`completeType()`、`analyzeInitializer()` |
| `TypeResolver` | 将 `TypeSyntax` 包装的表达式解释为类型；必要时请求 comptime；返回具体类型或依赖配方 | `resolveType()` → 具体 `TypeId` 或依赖结果 |
| `ExprAnalyzer` | 检查表达式，确定类型、值类别、可写性、阶段依赖和操作含义；不执行运行时表达式 | `analyzeExpr()` → `ExprInfo` |
| `StmtAnalyzer` | 检查函数体、控制流、局部声明、赋值、`return`、`yield`、`defer`；组织活动语句结构 | `analyzeBody()`、`analyzeStmt()` → `CheckedBody` |
| `CallResolver` | 解析调用目标、普通实参映射和重载选择；区分 `CallExpr` 的函数调用与类型构造；请求候选的泛型签名 | `resolveCall()` → `CallPlan` |
| `ConversionChecker` | 统一判断隐式/显式转换、初始化、拷贝/移动及参数传递是否合法并记录操作 | `checkConversion()`、`checkInitialization()` → `ConversionPlan` |
| `PatternAnalyzer` | 检查绑定与 match 模式，建立绑定，处理解构、守卫和可判定的覆盖/重复情况 | `analyzeBinding()`、`analyzeMatch()` → `PatternPlan` |

`ExprAnalyzer` 与 `StmtAnalyzer` 负责静态结论，`ComptimeEvaluator` 负责实际执行。后者不能自己临时决定名字绑定、重载、数值转换或拷贝规则。名字即使拼写相同，类型名、变量、函数集合和模块引用也必须通过语义结果区分。

`DeclCollector` 不得把整棵 AST 中的声明无条件加入可见表。尤其是模块和类体允许包含语句，处于编译期条件内的声明需要保留激活条件。支持普通声明预登记，也不等于已经确定其前向可见性；可见性规则由 `ScopeStore`/`NameResolver` 的统一策略实施。

匿名函数由 `ExprAnalyzer` 建立独立的函数身份与定义环境，交给 `StmtAnalyzer` 检查函数体，并记录捕获关系。运行时捕获属于闭包存储；只有参与泛型或类型计算的编译期捕获进入替换环境，不能把运行时局部对象地址放进实例键。

### 3.4 泛型实例化

| 类 | 具体职责 | 主要入口或输出 |
| --- | --- | --- |
| `GenericBinder` | 将 `::[...]` 中的位置、命名、展开实参与泛型形参对应；处理默认值、类型和值参数、参数包；规范化结果 | `bindArguments()` → `Substitution`、规范实参序列 |
| `GenericInstantiator` | 在定义环境和替换环境下完成实例签名、成员及函数体；请求语义分析和 comptime；共享原始 AST | `instantiateSignature()`、`instantiateBody()`、`instantiateType()` |
| `InstanceStore` | 以规范实例键去重，保存实例状态、签名、类型或函数体结果；支持递归引用已发布的身份 | `findOrCreate()` → `InstanceId`、`InstanceRecord` |
| `ExpansionBuilder` | 保存已选择的编译期结构、需要展开的语句序列及生成声明；分配独立上下文与来源记录 | `ExpansionId`、带上下文的 `NodeRef` 序列、生成声明记录 |

泛型形参与普通形参沿用当前 AST 的 `Parameter`，实参沿用 `Argument`。两者可复用参数对应算法，但泛型绑定必须得到允许的编译期实参；普通调用实参通常是运行时值。`GenericBinder` 不替代 `CallResolver`，也不自行决定所有重载优先级。

泛型实例化的默认实现是“共享定义 AST，加一份独立语义实例”。只有语言功能确实产生新语法结构时才分配新节点。若使用新 AST，必须由独立 `ASTContext` 拥有，保留来源映射并通过结构验证；不把其他树的节点直接挂入新语法树。现有 `verifyAST` 会拒绝同一树内重复引用的节点。

`ExpansionBuilder` 可以通过多个 `NodeRef + SemanticContextId` 表示同一语句在不同静态展开中的出现，无须复制整棵树。它提供承载结构变化的机制；`comptime for` 是否生成运行时语句、如何暴露生成的声明，仍由语言规则决定，不能仅从 AST 的 comptime 标记自行推导。

### 3.5 编译期求值与 IR 函数执行

| 类或记录 | 具体职责 | 主要入口或内容 |
| --- | --- | --- |
| `ComptimeEvaluator` | 求值显式 comptime 表达式和语句、展开静态结构；函数调用交给 ExecutionEngine 执行具体 IR | `evaluateExpr()`、`evaluatePlace()`、`executeStmt()`、`callFunction()` |
| `EvalContext` | 一次求值请求的状态集合；关联语义上下文、共享模块存储、当前局部/调用帧、预算和诊断轨迹 | 求值模式、目标、请求位置、调用帧栈 |
| `ExecutionEngine` / `ExecutionFrame` | 管理模块、函数分析及词法块中的语义活动记录；按绑定身份保存编译期对象 | `createFrame()`、`allocate()`、`lookup()`、`resolveValue()`、`endFrame()` |
| `ExecutionCompiler` / `ExecutionLinker` / `ExecutionMachine` | 将具体函数 IR 降低为自有执行镜像，按函数 ID 链接，以独立槽位数组和显式 VM 调用栈执行 | `compile()`、`verify()`；统一经 `ExecutionEngine::execute()` 进入 |
| `SemanticValueBridge` / `RuntimeTypeTable` | 在语义边界转换类型、常量、函数与运行时 ID 和载荷；布局、签名和运行时值不借用 IR 对象 | `lowerType()`、`lowerFunction()`、`lowerValue()`、`raiseValue()` |
| `EvalMemory` | 管理编译期对象及引用，验证初始化、越界、别名、生命周期和目标布局 | `allocate()`、`load()`、`store()`、`destroy()` |
| `EvalBudget` | 限制步骤、递归、对象数量、分配字节和结构展开数量，支持取消 | 显式预算状态；贯穿嵌套求值与实例化请求 |
| `BuiltinRegistry` | 登记语言内建的签名、编译期实现和运行时表示，确保两种执行方式使用一致的操作契约 | 内建操作描述、允许的编译期能力 |

`EvalContext`、`EvalFrame` 和 `EvalBudget` 可以先实现为小型状态记录；无需为每个字段建立抽象接口。`BuiltinRegistry` 只在需要内建操作时扩展，不把任意宿主函数调用作为默认编译期能力。

当前语义绑定帧使用 [`ExecutionFrameKind`](../src/include/ink/execution/engine/execution_frame.h)，保留 `Module`、`Analysis`、`Call` 和 `Block` 枚举；具体函数执行使用 `ExecutionMachine` 私有的 `CallFrame`，其参数和中间结果保存在相对槽位中。两条路径通过 [`ExecutionEngine`](../src/include/ink/execution/engine/execution_engine.h) 共享存储与预算，详见 [Ink 执行字节码](Ink-Execution-Bytecode.md)。上表其余 `Eval*` 名称描述后续完整求值器的逻辑职责，不表示这些类都已经存在。

### 3.6 控制流、验证与产物

| 类 | 具体职责 | 主要入口或输出 |
| --- | --- | --- |
| `FlowAnalyzer` | 从活动 AST 结构及语义结果建立函数内控制流，检查必需返回、可达性、初始化及生命周期约束 | `FlowInfo`；按函数/实例保存的分析图 |
| `CleanupPlanner` | 按作用域及退出边建立临时对象、局部对象和 `defer` 的清理计划 | `CleanupPlan`；供 comptime 执行和 IR lowering 使用 |
| `SemanticVerifier` | 检查实例、函数体和运行时可达依赖是否完整闭合；验证旁表、转换及清理计划一致 | 模块产物验证结果、运行时闭合验证结果 |
| `RuntimeLowerer` | 将闭合 AST 语义结果转换为显式类型、调用、存储、分支和清理的 InkIR | `lowerModule()`、`lowerBody()` → `ir::Module` |
| `SemanticModuleWriter` | 输出接口、所需 AST、定义环境、常量和依赖信息，检查导出依赖闭包 | `CompiledSemanticModule` 存档 |
| `SemanticModuleReader` | 验证并恢复模块存档、重建身份映射、按声明块读取 AST；由 Loader 调用 | 已校验模块接口及可按需加载的函数体 |
| `SemanticDiagnosticEmitter` | 为 Core 诊断补上定义、实例化和 comptime 调用轨迹；管理候选检查的诊断缓冲 | `core::Diagnostic` 及关联诊断 |

控制流分析图只服务分析和清理计划，不成为第二套编译期执行语言。`ComptimeEvaluator` 和 `RuntimeLowerer` 共用操作、转换、调用及清理决策，分别负责执行这些决策和编码这些决策。

## 4 语义旁表的数据契约

### 4.1 表达式信息

`ExprInfo` 至少将以下信息分开保存，不能用一个“是否常量”的布尔值代替：

| 维度 | 含义 |
| --- | --- |
| 类型 | 表达式的 `TypeId`；依赖类型使用显式依赖配方 |
| 名称/实体性质 | 普通值、类型值、声明引用、重载集合或模块引用 |
| 值类别 | 值还是可寻址位置；位置还需记录可写性和引用约束 |
| 阶段依赖 | 已知常量、运行时值、依赖尚未绑定的泛型参数 |
| 已解析操作 | 内建操作、选中的函数/构造、成员和转换计划 |
| 副作用 | 读写、调用和清理需求；决定能否安全折叠或缓存 |

`Dependent` 表示待实例化，`Runtime` 表示本次分析不能在编译期获得值，`Error` 表示分析失败；三者不可互换。类型值携带“被表示的 `TypeId`”，这与该表达式本身属于元类型是两件事。

### 4.2 缓存键

语义查询至少以 `NodeRef + SemanticContextId + 查询种类` 为键。受分析模式、期望类型或可见性位置影响的查询还必须包含相应参数，或只缓存与这些参数无关的内在结果。

不能只用 `ASTNodeBase *`，也不能只用 `InstanceId`：同一节点可能位于不同的静态展开上下文；同一调用可能在不同期望类型下选择不同转换。定义环境中的名字集合发生变化时，查询必须通过环境版本或依赖失效机制避免复用过期结果。

语义缓存可以记录 `x + x` 的类型、操作和局部槽位，不能记录某次调用中 `x` 的数值。求值结果缓存是另一层功能，第一版不缓存一般函数调用结果；以后只有在参数、可观察内存、外部依赖和副作用均受控时才能增加。

编译期表达式按源码顺序和实际控制流直接求值。初始化完成后，结果保存到对应对象，后续读取使用对象当前值；每次调用、循环迭代或静态展开重新执行所到达的表达式，详见第 6.1 节。

### 4.3 `CheckedBody` 的内容

一份 `CheckedBody` 保存所属声明/实例、原始体 `NodeRef`、语义上下文、局部绑定、表达式信息、调用和转换计划、活动分支/展开记录、控制流与清理计划，以及完成/失败状态。

编译期常量结果通过旁表替换视图供 lowering 消费，不原地把原 AST 改为字面量。被舍弃的结构仍留在源码 AST 中，但不进入当前实例的运行时活动结构。普通运行时分支的所有可能路径仍需要语义检查，不能因为求值器只访问某条路径就省略运行时检查。

查询状态采用 `NotStarted → InProgress → Complete`，失败显式标记为 `Failed`；取消与预算耗尽另行记录，不能伪装成成功或跨请求永久缓存为用户程序错误。查询依赖图记录“为何请求”，用于环诊断和未来的缓存失效。

## 5 泛型实例化的具体流程

以 `F::[i32, 4](x)` 为示意，泛型应用与随后发生的普通调用是两个步骤。

1. `NameResolver` 找到 `F` 的声明或候选集合，取得各自的 `DefinitionEnvironment`。
2. `GenericBinder` 在使用点环境分析显式泛型实参，要求它们得到允许的编译期结果；按形参映射命名、位置和展开实参。缺省实参在声明定义环境及已绑定形参环境中求值。
3. 将实参转换到形参要求的形式，规范化类型、值和参数包，形成 `Substitution`。依赖外层泛型参数时先返回依赖的泛型应用配方，不能提前宣称实例已经闭合。
4. `InstanceStore` 查找或建立实例记录。`GenericInstantiator` 在定义环境中应用替换，先完成具体签名或名义类型身份，再发布可供递归引用的头部。
5. 对函数候选，`CallResolver` 使用具体签名检查普通实参、转换及重载选择；参数对应关系与实际求值顺序分别记录，不因命名实参的重排而改变求值顺序。
6. 对选中的函数或需要完成的类型，按需检查其 AST 体。涉及类型运算或显式 comptime 时进入 `ComptimeEvaluator`，结果写入该实例的旁表和展开记录。
7. 完成必要的成员、布局、控制流与清理检查，发布 `CheckedBody` 或具体类型结果。失败实例不得进入运行时代码生成队列。
8. `RuntimeLowerer` 只为运行时可达或需要导出的闭合实体生成代码；仅用于 comptime 的函数实例不必生成运行时定义。

实例化成功并不表示函数已被执行；普通实参 `x` 可以是运行时值。comptime 调用同一个实例时，再为普通参数建立新的 `EvalFrame`。

### 5.1 实例身份与缓存

逻辑实例键为：

```text
定义声明 GlobalDeclRef
  + 必要的外层实例/编译期绑定
  + 规范泛型实参序列（类型、带类型的值、参数包）
```

普通调用实参和诊断中的调用位置不属于泛型实例身份。名义类型通过声明/实例身份区分，不能仅凭字段结构合并。参数包的长度、顺序和元素类型必须参与规范化；值参数的相等规则由语言定义，不能通过宿主内存的逐字节比较临时决定。

持久缓存还需要编译器及语义格式版本、完整目标/ABI 配置、编译选项、定义模块和受跟踪依赖指纹。当前 `core::TargetContext` 只有指针宽度、字节序及本机 ABI 兼容标记，不足以单独充当未来完整 AOT 缓存的目标身份。

第一版只需会话内实例缓存。模块存档格式与跨编译持久缓存可以分别实现，不能把“能够读取模块 AST”当成“所有语义缓存均可直接复用”。

### 5.2 递归与完成状态

`InstanceRecord` 分阶段记录状态：

```text
Created → ResolvingSignature → SignatureReady → CheckingBody → Ready
任一必要阶段失败 → Failed
```

普通递归和互递归函数可以引用 `SignatureReady` 的实例，不要求先完成被调用者的函数体。名义类型也可以先发布身份，再按需完成成员和布局。要求完整大小的循环依赖必须报错；只引用一个允许未完成的类型身份，不应被误判为布局环。

函数体局部检查完成不等于整个依赖闭包已经通过。相互递归的实例通过依赖组完成检查，只有该组及其必需依赖都无错误，才允许模块闭合验证成功。

签名求值、常量初始化或布局查询再次请求自身尚未建立的必要结果时，`SemanticQueries` 报告依赖环。运行中的递归 comptime 调用则由独立调用帧和共享预算处理，不能仅因为再次调用同一个 `DeclId` 就报循环。

`F::[N]` 不断请求 `F::[N + 1]` 不会命中同一个缓存键，所以还需要实例深度、数量和工作量预算。嵌套查询不能每次重新获得一份完整预算。

### 5.3 定义环境与候选检查

非依赖名字在相应活动语义路径被分析时绑定到定义环境中的声明；依赖参数的成员查找保留查找配方，在替换后结合实参类型完成。两者都不能把使用模块的同名局部变量当成模板定义内部的名字。

候选检查不应提前完整实例化所有函数体。候选签名所需的编译期计算、临时绑定和诊断必须隔离；失败候选不能向模块发布生成声明、可变编译期状态或永久错误诊断。可以保留与候选选择无关的确定性查询结果。

实参不匹配、依赖尚未满足、替换时出现语言错误是不同结果。是否允许把某类替换错误作为候选淘汰条件，应在重载规则中明确，不能因使用了缓存或延迟实例化便默认获得 SFINAE 行为。

## 6 comptime 的执行契约

### 6.1 求值入口与模式

`ComptimeEvaluator` 在语义分析期间求值显式 comptime 表达式和语句，并通过 `SemanticQueries` 取得所需语义结论。实际函数调用要求已有具体 IR，交给 `ExecutionEngine::execute()`；不得重新解释非泛型函数 AST。非泛型函数即使未调用也在定义处检查，泛型函数在具体实例被选中后检查函数体。

| 场景 | 行为 |
| --- | --- |
| `isComptime()` 为 true 的表达式、必须确定的泛型实参、类型表达式 | 使用 `RequireConstant`；运行时依赖、非法操作和无法持久化的结果显式失败 |
| 编译期执行某个语句或具体函数 | 显式编译期语句使用当前语义帧；函数调用使用已生成 IR 和独立调用帧，名称及常量绑定已在定义处确定 |
| 普通运行时表达式 | 由分析器建立运行时语义结果，保留给 lowering；不要求交给求值器先尝试执行 |
| 尚未绑定的泛型上下文 | 保留 `Dependent` 结果；在必须闭合的位置仍未解决时报告错误 |
| 可选常量折叠 | 可在以后增加；只能对允许折叠的操作执行，不能吞掉错误或重复副作用 |

第一版不必实现任意 `Known + Runtime` 的部分求值器。显式泛型替换、强制编译期求值和按规则选择活动结构已经能在 AST 上完成；非泛型函数统一生成 IR 后执行。

#### 6.1.1 源码顺序与三种帧

模块语义分析从第一条语句依次推进。每条语句先取得必要的名字、类型和操作结论，再完成其中要求的编译期执行、存储更新和结果记录，最后进入下一条语句。进入具体函数体时采用相同的顺序；普通运行时语句只建立运行时语义结果，不因此被提前执行。未绑定的泛型体保留依赖，具体实例进入分析后再遵守上述顺序。

| 帧 | 建立与退出 | 保存的状态 |
| --- | --- | --- |
| 模块帧 `Module` | 开始分析模块时建立，在执行引擎生命周期内保持有效 | 模块级编译期对象；声明允许写入的对象持续可写，后续求值访问同一位置 |
| 函数分析帧 `Analysis` | 开始检查并生成某个具体函数 IR 时建立，处理结束后退出 | 显式局部 `comptime` 对象；普通形参和局部变量生成 IR，不在此帧分配执行存储 |
| 函数调用帧 `ExecutionMachine::CallFrame` | 每次具体函数调用压入 VM 栈，返回或失败时退出 | 本次形参和中间结果的固定槽位、PC、返回槽及局部存储；编译期与普通执行遵守同一规则 |

语义求值的词法块可建立 `Block` 子帧，使编译期局部对象的生命周期随块退出结束。函数 IR 中的名称已绑定到具体对象，实际调用不恢复 AST 名称环境，也不会读取调用者的同名局部变量。退出语义块帧不能销毁其父模块帧中的对象。

例如按顺序处理以下声明：

```ink
comptime var A: int32 = 1;
comptime var X: int32 = A + 1;
comptime
{
    A = 10;
}
comptime var Y: int32 = A + 1;
```

结果为 `A = 10`、`X = 2`、`Y = 11`。读取后驻留到 ConstantPool 的 `2` 是不可变快照；它不使 `A` 变为只读，也不会随 `A` 的后续写入改变。

`var B: int32 = comptime(A + 1)` 仍声明运行时变量。分析运行时函数体时，`B` 只有运行时存储计划，后续 `comptime(B * B)` 必须报告运行时依赖；改为 `comptime var B` 才会在分析帧中建立可读写的编译期对象。实际编译期调用执行已经检查过的 IR；其中的 Alloca 在本次 `Call` 帧中创建局部存储，Load/Store 访问当前调用的值。

#### 6.1.2 初始化结果存储与实际控制流求值

一条声明的同一次初始化只提交一次副作用。以下源码执行后必须为 `Counter = 1`、`X = 1`、`Y = 1`：

```ink
comptime var Counter: int32 = 0;
comptime var X: int32 = ++Counter;
comptime var Y: int32 = Counter;
```

分析 `X` 的初始化表达式时执行一次递增并取得结果 `1`，创建变量存储或生成运行时常量时使用该结果。结果存入 `X` 的对象后，后续读取访问对象当前值，不重新执行 `++Counter`。

语义驱动按源码顺序处理声明，按显式编译期控制流求值表达式。每轮静态循环和展开建立独立帧，重新读取当前对象并执行所到达的表达式。实际函数调用另建 IR 调用帧，按已生成的指令执行副作用；不会再次分析函数 AST。

执行不提供事务回滚。失败前可能已经发生写入；驱动必须终止该次失败的分析，不能把半完成状态当作可用语义结果继续。取消或预算耗尽会停止当前引擎的后续执行，并保留清理入口。若要重新开始，必须重建受影响状态或先有明确的回滚机制，不能在已有副作用上直接重放求值。候选探测同样不能借用正式模块状态执行试探性写入。

### 6.2 值、位置与控制流

| 记录 | 内容 |
| --- | --- |
| `EvalValue` | 标量、聚合、类型值、可调用实体及允许的符号引用；必要时引用 `EvalMemory` 中的对象 |
| `EvalPlace` | `ObjectId`、子对象路径或受检偏移、类型、可写性和生命周期身份 |
| `EvalResult` | 表达式求值成功及结果，或错误、取消、预算耗尽等显式状态 |
| `ExecResult` | 正常继续、返回及返回值、跳出/继续目标、块表达式的 yield 结果，以及失败状态 |

`evaluatePlace()` 负责赋值目标和取地址等场景，不能把所有表达式都先加载为值。`return`、`break`、`continue` 和 `yield` 用显式控制流结果向上传递，由对应函数、循环、switch 或块表达式边界消费；`yield return` 的额外执行协议若尚未支持，必须明确诊断，不可悄悄当普通返回。

逻辑短路、空值合并、条件访问、条件表达式、match 守卫和分支必须按实际执行路径访问子节点。循环每次重新读取帧中的值；不能把第一次迭代的求值结果当成该节点的永久常量。

编译期条件选出的活动结构可以要求实例相关的延迟语义检查；未选结构仍经过 Parser 的语法检查，但不能被通用遍历器强制实例化。哪些非依赖错误要求在模板定义时诊断属于语言检查策略，应明确列为规则，不能由遍历顺序偶然决定。

### 6.3 内存和清理

编译期模块变量、局部变量、参数对象与临时对象由 `ExecutionHeap` 管理。`ExecutionPlace` 记录内部存储身份，Ink 指针指向按共享目标布局分配的数据区，而不是 Cell 管理对象。Heap 唯一拥有 Cell 和 Buffer；对象在所属帧结束或显式释放时销毁。不可变值通过 RAII 引用计数共享，指针值只复制机器地址，不保活目标；实现不使用 GC。原生返回地址可由引擎按类型直接读写，调用方必须保证有效性。指针不能冻结为 IR 常量；整数和浮点计算仍遵守目标及语言定义的位宽规则。

每个对象记录初始化状态、活动生命周期和必要的子对象状态。通过引用读写同一对象必须保持别名关系；已销毁对象、未初始化读取和越界访问需要显式失败。冻结聚合结果时必须遍历其引用，不能只检查最外层对象。

`CleanupPlanner` 给出清理顺序和退出边，`EvalFrame` 记录本次执行中实际完成的初始化及已登记的 `defer`。正常离开、返回、break 和 continue 都执行对应清理；部分初始化失败只能清理已完成部分。因取消或编译错误终止时回收编译器资源，不为“清理”继续执行不受限的用户代码。

语言中的析构/清理通过显式执行实现；宿主 C++ 对象回收采用 RAII。整个机制不使用 C++ 异常、SEH 或 `setjmp`/`longjmp`。

### 6.4 跨越编译期边界的结果

| 结果 | 保存或消费方式 |
| --- | --- |
| 可表示的普通常量 | 深度冻结到 `ConstantPool`，在 lowering 中变为 IR 常量或静态数据 |
| 类型值 | 保存规范 `TypeId`，供类型与泛型操作使用；不作为普通运行时 ABI 值 |
| 声明/结构展开 | 交给 `ExpansionBuilder` 和 `DeclCollector` 登记，保留来源及替换上下文 |
| 指向本次临时编译期对象的引用 | 不允许直接逃逸；只能按明确规则复制内容或转为可持久化符号引用，否则报错 |
| 运行时依赖或尚未闭合的结果 | 在 `RequireConstant` 请求处报告失败，不能悄悄残留为运行时代码 |

模块级可变编译期内存显式属于模块帧，由后续语义求值共享。函数 IR 中已生成的编译期常量保存定义时的值，函数局部内存按其分析或调用身份隔离。初始化完成不冻结整个模块，后续合法编译期写入仍可更新模块对象。跨边界保存的是本次求值所需的结果快照，已经发布的常量、类型或实例不会被后续写入追溯修改。

普通运行时全局变量的当前值不能作为编译期输入。当前原生导入可匹配同一程序中已纳入的导出；未匹配的定参 `import "C"` 调用通过 libffi 进入原生宿主，不限制函数名，外部副作用按源程序的调用顺序执行；外部文件、环境等依赖跟踪仍需后续接入。模块帧尚未接入跨模块加载和存档协议；本节只规定当前分析中的对象与结果边界，不规定多个导入模块之间的全局执行顺序。

### 6.5 当前最小执行模块的边界

[`ExecutionEngine`](../src/include/ink/execution/engine/execution_engine.h)、[`ExecutionFrame`](../src/include/ink/execution/engine/execution_frame.h)、[`ExecutionHeap`](../src/include/ink/execution/memory/execution_heap.h)、[`ExecutionObject`](../src/include/ink/execution/support/execution_object.h)、[`ExecutionValue`](../src/include/ink/execution/value/execution_value.h) 和 [`ExecutionResult`](../src/include/ink/execution/support/execution_result.h) 的公共头位于 `src/include/ink/execution`，实现位于 `src/lib/execution`；两侧按 `bytecode/`、`runtime/`、`bridge/`、`engine/`、`memory/`、`value/`、`support/`、`ffi/` 分目录。`bytecode/` 定义操作码模式、连续指令产物和函数编译器，`engine/execution_machine*.cpp` 承担槽位执行和显式调用栈。七种值子类分别使用 `value/execution_*_value.h` 和 `.cpp`，基类头 `value/execution_value.h` 保留 `ExecutionValueRef` 与 `ExecutionValueResult`；语义绑定帧构造和析构位于 `engine/execution_frame.cpp`，执行状态的 Core 诊断适配位于 `support/execution_diagnostic.cpp`。它们同时服务于显式 comptime 语句求值和统一的函数执行，执行表示、缓存与预算详见 [Ink 执行字节码](Ink-Execution-Bytecode.md)。当前提供以下基础能力：

- `ExecutionObject` 是地址稳定、不可复制或移动的对象基类，`ExecutionValue` 是其抽象值子类；`ExecutionVoidValue`、`ExecutionBoolValue`、`ExecutionIntegerValue`、`ExecutionFloatValue`、`ExecutionStringValue`、`ExecutionPointerValue` 和 `ExecutionFunctionValue` 分别保存一种不可变载荷。`ExecutionValueRef` 通过 RAII 引用计数共享只读值对象，最后一个引用释放时销毁对象，不使用 GC。类型和函数身份仍借用 IR；标量和字符串结果可以越过引擎生命周期，只要其 IR 类型保持有效。普通结果使用 `ExecutionValueResult`，默认失败，成功结果必须有非空值引用；运行中间值不驻留到常量池。
- `ExecutionHeap` 提供语义值与存储入口，Cell/Buffer 由不依赖 IRContext 的 `ExecutionMemoryManager` 独占管理。Cell 以共享 `TypeDesc` 分配原生标量或连续字节区，内部元数据保存类型、可写性和初始化状态；Buffer 保存固定字节数组。内部绑定使用 `ExecutionPlace` 进行清理和语义检查，语言指针使用数据区的裸地址。写入同一位置不修改先前读取的不可变快照。
- 语义模块、分析和词法块使用 `ExecutionFrame` 管理绑定；VM 调用帧管理参数槽、结果槽和存储，并在返回时释放 Cell/CString Buffer。内部 `ExecutionStorageRef` 保存弱控制块、槽位和代次，供分配管理和清理使用。语言指针只包含一个原始地址，不含代次或所有权。显式释放或帧结束后，已有指针成为悬空地址，继续解引用属于未定义行为。累计存储分配预算不因释放而返还。
- 语义层将已求值实参转换为 `ExecutionValueRef`，通过 `execute()` 统一执行函数，并在编译期调用的返回边界冻结为常量；不保存 AST 函数体回调。`allocate()`、`load()`、`store()` 负责语义变量的常量边界转换，分别复用 `allocateValue()`、`loadValue()`、`storeValue()`。`load()` 返回 `ExecutionResult`；`execute()` 返回 `ExecutionValueResult`。常量冻结只支持同一上下文的 bool、整数、浮点和字符串，指针和函数拒绝冻结，void 表示成功且无常量。
- `execute(Function, span<const ExecutionValueRef>)` 通过 `ExecutionCompiler` 按函数将 Alloca、Store、Load、整数 Add、LogicalNot、LogicalAnd、LogicalOr、Compare、CString、Call、Branch、ConditionalBranch 和 Return 降低为连续字节码。`ExecutionMachine` 按相对槽位执行，在显式 VM 栈中压入和退出调用帧；函数内的 Function 声明只产生函数值。整数位宽与符号、操作数槽和分支 PC 在执行前确定，满足地址使用约束的局部存储使用专门的 Local 指令。代码及直接调用点链接按需缓存，`IRContext::revision()` 改变后失效；不会缓存函数结果。普通执行与编译期函数调用共享这一路径，源码循环也通过分支和回边连接控制流。
- 操作码和 `Operands[0..3]` 各项的操作数种类登记在 `bytecode/instruction.def`；VM 分派直接更新 PC、槽位和调用栈，失败通过 `ExecutionStatus` 传播，公开 `execute()` 返回 `ExecutionValueResult`。[`ExecutionInstructionResult`](../src/include/ink/execution/support/execution_instruction_result.h) 与 `support/execution_instruction_result.cpp` 保留为公共兼容类型，当前 VM 不使用它，也不调用旧的逐 IR 指令处理函数。
- `SemanticValueBridge` 将语义类型、常量、函数降低为运行时 ID 和 `RuntimeValue`；`ExecutionLinker` 拥有并链接 `ExecutionImage`，按需编译具体函数。执行函数拥有初始槽与常量字节，共享不含 IR 指针的 `TypeDesc` 表；VM、Cell 和原生调用使用运行时表示，对外语义结果通过桥接层转换。定宽整数内存操作选择 `LoadI*` / `StoreI*`，不逃逸局部变量使用固定帧单元，每次动态分配重置初始化状态并继续计入预算。类型身份与大小/对齐分别保留，间接调用校验完整签名 ID。准备完整的 `ExecutionImage` 可直接构造无桥接层链接器，在源 IR 析构后执行；语义入口仍使用带桥接层的按需编译模式。
- 原生导入优先匹配当前程序已纳入的同名同签名导出，未匹配时通过宿主符号和 FFI 调用；`export "C"` 和 `[abi("C")]` 本地函数执行各自的字节码函数体。VM 使用 `NativeCallCache` 按需准备和缓存 `NativeCallPlan`，其原生地址、libffi 签名、参数及返回值布局不保存 IR 引用；`runtime_ffi_type.cpp`、`runtime_ffi_argument.cpp`、`runtime_ffi_call.cpp` 每次直接封送 `RuntimeValue`。`ffi/external_function.cpp` 与原有 `ffi_type.cpp`、`ffi_argument.cpp`、`ffi_call.cpp` 保留语义值公共接口，入口显式接收调用者的 `ExecutionHeap &` 和 `span<const ExecutionValueRef>`。`ffi/native_symbol.cpp` 封装平台 API，不使用特定函数的 C++ 签名或名称白名单。IR 兼容参数转换拒绝未求值节点，需要执行引擎先取得其结果。
- 编译期表达式由语义层按源码顺序和实际控制流直接求值，通过共享步骤/资源预算和取消状态限制执行。
- 复用 IR 的 bool 和任意位宽整数常量，提供一元运算、整数算术/位运算/比较及 bool 逻辑；运算先由语义层确定操作数类型，不隐式执行类型转换。

函数指令执行后，其结果写入 `ExecutionMachine` 当前调用帧的固定槽位。`ExecutionFrame` 只保存语义绑定和所属存储，没有函数 SSA 结果表；`resolveValue()` 将常量、函数身份或已有语义绑定解析为执行值，不读取 VM 槽位，也不执行指令。同一 Call 的结果被使用两次仍只调用一次，再次执行函数则使用新的 VM 帧及槽位内容。AST 循环和静态展开遵守前述每次实际执行的规则。

FFI 支持 bool、8/16/32/64 位整数、f32/f64、裸指针参数和返回以及 void 返回，拒绝按值聚合、变参和 f16。指针变量与指针字段保存真实机器地址，支持 `T**` 输出参数和 `*Class` 参数；不受创建它的 Heap 身份限制。`ExecutionPointer::status()` 不检查生命周期。VM 可直接访问有效外部内存，非法裸指针访问没有统一诊断保证；调用者负责存储生命周期、类型、对齐、权限和访问范围。

CString 的 Buffer 由 Heap 唯一拥有，创建它的函数帧负责在返回时释放；FFI 返回该 Buffer 内部或尾后地址时保留原始机器地址，不改变其原有释放时机。直接传给 FFI 的字符串执行值在调用者 Heap 中另建独立可写副本，未由返回值引用的临时副本在调用结束时立即释放；若返回值指向该副本内部或尾后地址，调用层显式移交临时清理责任，让 Heap 保留副本直到显式释放或 Heap 销毁，指针自身仍不保活。原生函数仅保存裸指针不会触发提升，已有帧缓冲区的返回别名也不提升生命周期。这不会改写原字符串或 IR 常量。原生地址、缓冲区地址和函数身份均不通过整数常量保存，也不能越过编译期常量冻结边界。

整数运算由项目自有的 `ExecutionInteger` 完成，以明确位宽和低位在前的字数组保存补码位模式，通过 `ir::IntegerBits` 与常量池交换结果，不依赖 `llvm::APInt`。加减乘及一元取负按位宽回绕；有符号最小值除以 `-1` 返回同一位模式，余数为零。除零、负移位或移位量不小于位宽显式失败。有符号右移为算术右移，无符号右移为逻辑右移。逻辑运算接口和 LogicalAnd/LogicalOr 指令消费已经取得的操作数；显式编译期表达式在 AST 求值层短路，普通函数中的 `&&`、`||` 则生成条件分支、临时 bool 存储和汇合块。右操作数只在实际需要的路径上执行，不能用对两边预先求值后的逻辑指令代替短路。

引擎和帧直接持有状态，不使用 Impl/PIMPL。`ExecutionLimits` 的全部默认值通过 `ConfigManager` 从 `config.def` 及相应环境变量读取，并在构造时保存快照；调用方仍可像其他 Limits 一样显式覆盖字段。

该模块不自行遍历 AST，不解析名字、执行重载选择或生成 IR。语义层在定义处检查并生成全部非泛型函数的 IR，随后按需要调用 `execute()`；引擎负责调用帧的创建和退出。显式 `comptime func` 同样拥有 IR 函数体，但禁止在运行时使用。当前统一支持 IR 的参数读取、局部存储、整数加法、bool 逻辑与短路、整数及 bool 比较、bool 条件的 if/else/else-if 与嵌套分支、嵌套调用和返回；源码 while/for、数组 for-in、整数增减和 += 也使用同一 IR 执行路径；其余未接入的算术运算、复合赋值及参数或模块对象写入没有额外 AST 回退路径，按普通函数规则诊断。显式 comptime 表达式、块和静态循环仍在语义分析期间执行或展开；泛型实例按需生成 IR 并使用同一执行入口。Class 使用名义值语义，运行时字段值为独立快照，字段位置保留所属分配与偏移；具体规则及 VM/AOT 共用接口见 [Class 与对象语义](Ink-Classes.md)。其他未支持的语义仍须显式失败，实际源码覆盖范围以分析器实现及测试为准。

[`entry_execution_test.cpp`](../src/testcase/semantic/entry_execution_test.cpp) 覆盖源码经过 tokenizer/parser AST、semantic IR，再执行普通 Entry 的路径：运行时参数和局部对象参与嵌套调用，Windows `_write` 或 Linux `write` 通过 libffi 写入真实管道，并断言 UTF-8 字节及返回值。其他用例检查同一 Call 结果复用不重放副作用、函数再次调用重新执行，以及 FFI 返回缓冲区别名和 CString 逃逸失效；编译期函数调用测试另验证同一 IR 的常量返回边界、定义处检查及实际调用时的副作用。

逻辑和比较在 `src/testcase/execution/programs` 中保存 6 个独立 `main` 程序，逐项打印用例名和 `PASS`/`FAIL`，内部保留 56 项成功结果检查，全部通过时退出 0；操作数副作用输出验证短路、求值顺序和单次执行。8 份类型错误与 3 份必达缺少函数体调用的输入独立放在 `src/testcase/execution/cli/inputs`。[`source_program_tests.cmake`](../src/testcase/execution/cli/source_program_tests.cmake) 统一登记 HelloWorld 和逻辑源码测试，在 Windows/Linux 注册 6 项成功程序和 11 项负向测试，共 17 项 `ExecutionSourceTest.Logical.*` CTest，共用 `source_program_test.cmake` 以默认 `main` 运行真实 CLI，通过 `OUTPUT_FILE` 捕获 stdout 并核对完整输出、退出码和诊断；常规比较归一化 CRLF，HelloWorld 另外用 `STDOUT_HEX` 校验原始字节。IR 单元测试另外覆盖 eager LogicalAnd/LogicalOr、任意位宽比较和归档往返，不以源码短路测试替代指令自身语义。

## 7 跨模块泛型与编译产物

### 7.1 A 定义泛型，B 使用实例

设 A 定义 `F1`，B 使用 `A.F1::[i32]`。建议采用以下逻辑产物；`A.inkmod` 是语义产物的暂定文件名，不在本文冻结二进制格式。

| A 的产物 | 保存的内容 | B 如何使用 |
| --- | --- | --- |
| `A.inkmod` | 导出索引、声明身份、参数和签名、泛型 AST、定义环境、具体编译期支持函数的 IR、类型/常量和依赖指纹 | 找到 `F1`，恢复其定义环境，绑定 `i32`，在 AST 上完成实例化，函数调用执行具体 IR |
| `A.obj` 或相应目标代码 | A 中需要运行时定义的普通函数、已选择生成的闭合实例、全局数据和模块初始化 | 与 B 生成的代码链接 |

B 的 `InstanceStore` 保存这个实例，AST 可以仍由加载后的 A 模块持有。实例语义结果属于使用它的编译会话，不回写 A 的共享定义，也不修改磁盘中的 `A.inkmod`。

本方案采用使用方生成其所需实例。B/C 对相同实例的符号身份必须一致，链接时通过目标平台支持的重复定义合并机制处理，或由编译驱动统一安排唯一生成者；不能依赖每个模块随意分配的 `InstanceId` 拼接链接名。

### 7.2 模块存档的必要内容

`CompiledSemanticModule` 至少包含：

1. 模块身份、存档及语义版本、目标约束、编译配置和依赖指纹。
2. 导出与可被定义环境引用的声明索引、可见性、泛型参数、默认实参及签名所需信息。
3. 泛型 AST、必须在导入方求值的表达式，以及它们可达的具体 comptime 支持函数 IR；不必保存所有无关函数体。
4. 定义作用域、导入绑定、稳定声明引用和依赖名称查找配方，含所需私有依赖。
5. AST 节点字段、嵌入记录、源码定位和字面量所需的 Token/payload；规范类型、常量及显式捕获的编译期结果快照。可变模块对象的跨模块恢复协议仍需另行定义。
6. 按声明/函数组织的块索引，支持接口先加载、函数体按需加载。

当前 `LiteralExpr` 保存 `TokenId`，名称还可能引用源码存储。只写 AST 子节点关系无法恢复可执行语义。存档可保留所需源码与 Token/payload，或者在存储格式中显式编码等价字面量和名称数据；两种方案都必须支持诊断定位。

已经完成的绑定旁表可以作为存档内容，但必须使用可重定位身份；每个实例的类型、重载选择和求值帧不能当作模板定义的唯一答案保存。存档中的不可变结果快照可以恢复读取；这不意味着 A 的模块级变量在初始化后不可修改。依赖 B 本次泛型实参的结果需要在 B 实例化时计算，可变模块状态如何共享或恢复、何时允许跨模块写入及导入间的执行顺序尚未接入，不能由存档加载顺序隐式决定。

### 7.3 私有依赖与诊断

若 `F1` 使用 A 的私有 `Helper`，内部引用保持 `GlobalDeclRef(A, Helper)`，B 中的同名符号不能改变它。接口可见性和为了实例化而保存实现信息是不同问题：保存私有支持体并不使其变成可供 B 源码直接查找的公开名字。

若非泛型 `Helper` 在 comptime 被调用，需要保存其 IR 及传递依赖；若只在运行时调用，可由 A 的目标文件提供定义，但必须有能从 B 生成实例引用的链接符号，不能错误地仅保留为 A 目标文件的局部符号。

错误应同时指向 A 中失败操作、B 中实例化请求及完整泛型/求值调用链。A 的泛型定义可用，不代表每组实参都合法；实例化失败属于具体实例的诊断。

### 7.4 存档验证

Reader 必须检查版本、长度、分配预算、种类编号、必需子节点类别、引用范围、数组长度、源码范围和 Token/payload 引用。节点与 `Parameter`、`Argument`、属性等嵌入记录均须显式编码；`forEachChild()` 不能替代字段序列化。

加载模块时先验证头部、接口、身份映射和块索引，再发布可查询接口；按需加载某个 AST 块时，必须完成该块的字段、引用及结构验证后才发布函数体。资源预算覆盖所有已加载块，不能通过多次局部加载绕过总量限制。

节点指针、`std::span` 和 `string_view` 的内存布局均不是文件格式。稳定 ASTKind 编号帮助识别节点种类，不自动提供字段格式兼容性。

## 8 错误、诊断与闭合验证

### 8.1 显式结果

统一定义 `SemanticResult<T>` 或等价结果类型，区分成功、语言错误、资源限制和取消。查询的“仍依赖泛型参数”属于显式分析结果；名称不存在、模块不存在、候选不适用也应有自己的可判定状态，不能全部压成空指针。

`SemanticDiagnosticEmitter` 只适配 Core 的 `Diagnostic`、`DiagnosticKind`、`SourceId` 和 `SourceRange`。新诊断种类加入 Core 的统一定义，不在 semantic 重建公共诊断容器。失败缓存保存稳定原因；每次使用时按当前请求补充实例化轨迹，避免重复输出过时的调用位置。

当前执行路径使用 `ExecutionStatus` 作为返回与停止控制状态，使用 execution 的 `makeExecutionDiagnostic()` 适配 Core 统一定义的 `Execution*` 诊断；Core 不反向依赖 execution。适配函数只构造诊断，语义或 CLI 调用边界补充源码范围与上下文并报告一次，底层执行不持有诊断引擎。原因文案同时适用于编译期与运行时。`Success` 和 `Cancelled` 不生成诊断，取消不能因缺少诊断被补报为其他错误；资源上限、内部状态违规与未实现操作归 ICE，在报告边界遵循 Core 的 panic 策略。

当前 Core 的关联诊断记录只有范围、没有独立 `SourceId`。实现跨文件调用轨迹时，应补齐 Core 关联位置表达能力，或先发出各自携带 `SourceId` 的独立 note；不能把 A 的范围套到 B 的源码上。

所有可恢复失败通过结果、状态和诊断报告；日志使用现有 spdlog 设施。错误恢复 AST 可以参与编辑器分析，但不允许生成可执行产物。

### 8.2 两种验证目标

`SemanticVerifier` 区分两种合法输出，不能要求模块中的每个泛型模板都已实例化：

| 验证目标 | 必须满足的条件 |
| --- | --- |
| 可导入的语义模块 | 导出接口、定义环境、AST 引用及依赖闭包有效；开放模板允许保留显式依赖配方，但不能伪装为已闭合函数体 |
| 可生成运行时代码的闭合实体集合 | 可达实体的类型、名称、调用、转换、活动结构和清理均确定；必要签名/布局/函数体完整，没有待执行 comptime 或未绑定泛型参数 |

闭合结果不允许携带编译期临时指针、类型元值或尚未决定的生成声明进入运行时 ABI。允许合法的跨模块外部引用，但必须有确定的声明身份、签名和链接约定；闭合不要求把所有外部函数体复制进当前模块。

`RuntimeLowerer` 只消费已经确定的计划，不重新做名称查找、重载选择或泛型实例化。转换后的 IR 还必须通过 IR verifier，再交给运行时解释器和 LLVM 后端；semantic 验证不能代替 IR 结构验证。

## 9 建议的文件组织与实现顺序

### 9.1 文件组织

语义层头文件放在 `src/include/ink/semantic`，实现放在 `src/lib/semantic`；执行设施独立放在 `src/include/ink/execution` 和 `src/lib/execution`。按职责分组即可，不必给每个小记录单独建立文件。

| 文件组 | 主要内容 |
| --- | --- |
| `analyzer/analyzer.h` | 已实现的 Analyzer 流程骨架及语句/声明严格分派 |
| `name_resolve/binding.h`、`name_resolve/scope.h`、`name_resolve/name_resolver.h` | 已实现的名字绑定、作用域及名字解析 |
| `semantic.h`、`semantic_result.h`、`semantic_ids.h` | 对外入口、结果状态和强类型身份 |
| `semantic_session.h`、`semantic_module.h`、`semantic_driver.h` | 会话、模块、调度入口 |
| `decl_store.h`、`scope_store.h`、`type_context.h`、`constant_pool.h` | 基础存储 |
| `semantic_info.h`、`semantic_queries.h` | 语义上下文、旁表、各类计划和查询缓存 |
| `decl_analyzer.h`、`type_resolver.h` | 声明收集/完成和类型解析 |
| `expr_analyzer.h`、`stmt_analyzer.h`、`call_resolver.h` | 表达式、语句和调用分析 |
| `conversion_checker.h`、`pattern_analyzer.h` | 转换、初始化、模式分析 |
| `generic_binder.h`、`generic_instantiator.h`、`instance_store.h` | 泛型绑定、实例完成和缓存 |
| `expansion_builder.h` | 编译期结构展开及来源映射 |
| `comptime_evaluator.h`、`eval_context.h`、`builtin_registry.h` | AST 求值与语义查询协调、控制流及内建操作；建议结构 |
| `execution/bytecode/` | opcode/操作数宏表、执行函数产物、槽位及跳转布局、函数编译与验证 |
| `execution/runtime/` | 运行时类型/签名和函数 ID、自有存储布局、位模式与复杂值载荷 |
| `execution/bridge/` | 语义类型、常量、函数身份与运行时表示的转换 |
| `execution/engine/` | 执行入口、语义绑定帧、代码缓存、显式 VM 调用栈、槽位分派、内存访问及运算调度 |
| `execution/memory/` | 统一堆工厂、唯一拥有的 Cell/Buffer、内部存储句柄及裸指针 |
| `execution/value/` | 抽象值基类、RAII 值引用、执行值结果、七种独立值子类及精确位宽整数运算 |
| `execution/support/` | 执行对象基类、公共状态、保留兼容的指令动作结果、常量结果与存储位置结果、Core 执行诊断适配 |
| `execution/ffi/` | 外部调用入口、ABI 类型映射、参数封送、原生返回值处理、符号与调用计划缓存 |
| `flow_analyzer.h`、`cleanup_planner.h` | 控制流分析和清理计划 |
| `semantic_verifier.h`、`runtime_lowerer.h` | 闭合验证和 IR 生成 |
| `semantic_module_loader.h`、`semantic_module_io.h` | 模块加载和存档读写 |
| `semantic_diagnostic_emitter.h` | 语义诊断及实例化/调用轨迹 |

依赖方向为 `semantic → execution / ir / core / tokenizer / parser`，`execution → ir`；Core、Parser 和 ExecutionEngine 均不反向依赖 semantic。ExecutionInteger 使用项目自有的整数运算实现，execution 不增加直接 LLVM 依赖。是否把 lowering 和模块存档拆为独立 target 可在接入时决定，不影响上述类边界。

semantic 对象模型已使用显式源文件列表接入构建，采用与 Parser 公共头兼容的 C++20，并保持目标级禁用异常；测试加入统一的 `ink_tests`。execution 测试按 `src/testcase/execution/bytecode`、`engine`、`memory`、`value`、`support`、`ffi` 和 `cli` 组织，源码样例仍位于 `src/testcase/execution/programs`。后续新增分析器实现时继续维护目标源文件列表。

### 9.2 实现阶段

| 阶段 | 实现内容 | 应达到的结果 |
| --- | --- | --- |
| 1. 普通语义纵切片 | Session/Module、身份与基础存储、声明/名称/类型/表达式/语句分析、最小查询状态、Verifier | 普通函数形成可验证的 `CheckedBody`，有确定性语义 dump |
| 2. 最小 comptime 与 IR 执行 | 显式语句求值器、IR 调用、帧、受控内存和共享预算 | 同一函数 IR 在独立编译期调用中按实参返回结果；非泛型函数不延迟检查 |
| 3. 泛型纵切片 | Binder、InstanceStore、Instantiator、依赖配方、按需头部/函数体完成 | 同一 AST 产生多个独立实例，能处理泛型与 comptime 相互请求 |
| 4. 闭合 IR 接入 | 完整活动结构、Flow/Cleanup、RuntimeLowerer、IR 验证及后端适配 | 普通函数和泛型实例能从新前端贯通到运行时解释器或 LLVM |
| 5. 跨模块与扩展 | 模块存档、定义环境、按需 AST 加载、私有依赖闭包；再扩展复杂类型/模式/结构生成 | B 可从 A 的语义产物实例化，并正确链接和报告跨文件错误 |

各阶段先实现目标语言子集所需的操作，未支持的语法必须明确报错。不要为了建立所有文件先创建大量无行为空类，也不要把阶段 1～3 的成功当成端到端代码生成已完成。

## 10 验收场景

以下是实施时的行为验收要求，不表示本次已经运行这些测试。

| 场景 | 必须验证的结果 |
| --- | --- |
| 同一泛型分别绑定两种类型 | 共享源码 AST；类型、重载和转换结果隔离；原始 AST dump 不变 |
| 同一实例重复使用 | 身份及已完成语义结果复用；失败、取消和预算状态没有污染其他请求 |
| 同一实例多次 comptime 调用 | 参数/局部对象隔离，得到各次实际参数对应的值 |
| 顺序修改模块级 comptime 变量 | 后续语句读取更新值；之前保存的求值快照保持不变；模块初始化后仍允许合法写入 |
| 多次读取带副作用初始化的对象 | 初始化结果存入对象，后续读取不重新执行初始化表达式 |
| 静态循环及实际 IR 函数调用 | 每轮展开和每次调用使用独立帧；调用执行已生成 IR，不重新解释函数 AST |
| 普通变量由 comptime 表达式初始化 | 分析运行时函数体时仍是运行时对象；后续强制编译期读取失败 |
| 递归/互递归函数与类型 | 合法的签名/类型身份引用成功；必要布局或常量依赖环有完整轨迹 |
| 泛型实参包含命名、默认和参数包 | 对应及规范化正确，默认值使用定义环境，不重复执行显式实参 |
| 泛型参数影响类型表达式或编译期结构 | 依赖在正确实例/展开上下文中解决；不同展开不复用错误旁表 |
| 强制编译期位置读取运行时值 | 在请求点失败，不残留运行时代码 |
| 短路、守卫与编译期条件 | 未执行操作不产生副作用；未选结构不被无条件实例化；运行时可能路径仍受检查 |
| return、break、continue、defer 和部分初始化 | 控制流传递正确，清理恰好一次且顺序正确 |
| 引用别名、未初始化、越界、悬空与逃逸 | 内存语义正确，非法访问和不可持久化结果有明确诊断 |
| 无限循环、递归和不断增长的实例链 | 预算统一生效，失败不留下可用的半成品 |
| A/B 同名符号、私有运行时辅助函数及 comptime 支持体 | 保持 A 的定义绑定；代码可链接；缺失体/依赖有可定位错误 |
| 模块存档往返及损坏输入 | AST、字面量、定义环境和声明身份可恢复；非法版本/引用/长度被拒绝 |
| 编译期与普通执行共享函数 IR | 定义处检查全部非泛型函数；相同 IR 使用相同执行规则，编译期结果额外经过常量边界 |
| 完整源码编译 | 从 `.ink` 经过语义、实例化/求值、闭合 IR 验证到运行时结果，而非只测孤立分析器 |
