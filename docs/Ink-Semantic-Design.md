# Ink Semantic 模块设计

基于 AST 的语义分析、泛型实例化与 comptime 执行

日期：2026 年 9 月 23 日。状态：基础对象模型已实现，其余为待实现的架构设计。

本文采用本次确定的方向：**泛型实例化和 comptime 都在 AST 层完成，完成后的运行时语义再 lowering 为 Closed InkIR**。除第 1.3 节列出的基础对象模型外，下面的类是建议实现结构，不表示仓库已经实现这些接口。语言语法继续以 [Ink-grammar-Rules.bnf](Ink-grammar-Rules.bnf) 为准；本文不增加新的泛型、反射或声明生成语法。

## 1 当前基础与目标边界

### 1.1 当前仓库事实

| 现有设施 | 当前状态及设计影响 |
| --- | --- |
| [parser.h](../src/include/ink/parser/parser.h) 中的 `ParsedUnit` | 持有 `TokenBuffer`、`ASTContext`、`ModuleAST` 和恢复记录；semantic 必须保留其生命周期 |
| [ast.h](../src/include/ink/parser/ast.h) | 具体继承节点通过指针连接；已有 `TypeSyntax`、`GenericApplyExpr`、`ComptimeExpr`、`ComptimeStmt`、`FunctionDecl` 等 |
| [ast_context.h](../src/include/ink/parser/ast_context.h) | Arena 管理稳定地址的节点和数组；semantic 不接管单个节点的释放 |
| [ASTNodes.def](../src/include/ink/parser/ASTNodes.def) | 节点种类有稳定显式编号；种类编号不是某个声明或实例的身份 |
| [core/context.h](../src/include/ink/core/context.h) | 已有 `CompilationContext`、`FrontendContext`、源码管理、诊断及目标信息，直接复用 |
| [semantic/CMakeLists.txt](../src/lib/semantic/CMakeLists.txt) 与 [lib/CMakeLists.txt](../src/lib/CMakeLists.txt) | semantic 对象模型已接入构建；名称解析和语义分析器尚未实现 |
| [source_module_compiler.h](../src/include/ink/ir/compilation/source_module_compiler.h) 及其 [实现](../src/lib/ir/compilation/source_module_compiler.cpp) | 现有 `ir::SourceModuleCompiler` 默认读取 `.ir` 并反序列化，不是 `.ink` 源码编译器 |
| [inkc/main.cpp](../src/tools/inkc/main.cpp) | 当前仅处理命令行参数；以下流程仍需接入驱动 |

### 1.2 目标流水线

```text
.ink 源码
  → tokenizer / parser
  → ParsedUnit：只读 AST + TokenBuffer + 源码
  → SemanticDriver
      ↔ 声明登记 / 名称解析 / 类型与调用检查
      ↔ GenericInstantiator：AST + 泛型绑定 → 实例语义结果
      ↔ ComptimeEvaluator：AST + 语义结果 + 调用帧 → 编译期结果
      → CheckedBody：已确定的绑定、类型、转换、活动结构和清理计划
  → SemanticVerifier
  → RuntimeLowerer
  → Closed InkIR[target] → IR verifier
      ├─ 运行时解释器
      └─ LLVM lowering → 目标文件 → 链接

模块语义产物：声明接口 + 所需 AST + 定义环境 + 常量 + 依赖信息
运行时代码产物：已闭合的普通函数、泛型实例和全局初始化代码
```

语义分析、实例化和编译期执行相互按需请求结果，并非必须先完整检查所有函数体，再统一执行 comptime。例如 `TypeSyntax` 包装的表达式可能调用一个编译期函数；这个函数又可能使用某个泛型实例。

这条路径不要求先建立 TemplateIR 或 Staged InkIR。`CheckedBody` 是 AST 的语义旁表及少量展开记录，不另建一棵逐节点复制的 Typed AST，也不是交给后端执行的指令集。运行时解释器仍只接收 Closed InkIR。

### 1.3 已实现的对象模型

当前公共头位于 `src/include/ink/semantic`，由 `SemanticContext` 借用 Core 编译上下文并统一拥有模型对象；完整 `SemanticSession` 后续组合这份存储。

- `model/Values.def` 生成 `ValueKind`，以 C++ 类名标识实际对象，如 `IntegerType`、`FunctionType`、`ExprValue`、`CallInstruction`；类型、常量和指令条目分别生成 `Type::classof()`、`Constant::classof()`、`Instruction::classof()`，具体类的 `classof()` 直接检查同名值种类。元类型、void、bool 共用实际类 `BuiltinType`，通过 `TypeKind` 区分。`Type`、`UserDefinedType`、`Constant`、`Instruction` 仅作为中间基类，不单独占用值种类。
- `model/type/Types.def` 是类型种类与基类分类的注册表，每条记录为 `INK_SEMANTIC_TYPE(Name, Base)`，`Base` 为 `BuiltinType` 或 `UserDefinedType`。`TypeKind` 和两个基类的 `classof()` 从同一张表生成；具体类型类体与构造定义在 `model/type` 的独立头文件中，其继承关系应与注册表一致。
- `Name` 是一个 32 位池内索引，`NamePool` 为同名字节串只保存一份内容；池扩容保持名称与字符串视图稳定。空输入和索引耗尽返回无效名称。名称相等和哈希仅在同一池内有意义；索引本身不携带池身份，无法检测恰好落在另一池有效范围内的外来索引。词法验证与 NFC 处理仍由 tokenizer 负责。
- `ConstantPool` 由 `SemanticContext` 独占，通过 `constantPool()` 访问；当前驻留 bool、任意位宽整数、`StringConst` 和 `FloatConst`，`SemanticContext` 的常量工厂转发到同一池。池在基础类型创建后初始化，并在类型存储销毁前释放；false、true 预先创建，其他常量按规范类型身份与完整 payload 先查找、未命中才分配，哈希碰撞后继续精确比较。外来类型和 payload 与类型不匹配的请求返回空指针且不改变池。`size()` 包含两个 bool 常量，`owns()` 检查具体对象归属，池与常量地址在上下文生命周期内保持稳定。聚合常量、源码字面量语义分析及 IR lowering 仍待后续实现。
- `StringConst` 的类型固定为本上下文的只读 `u8` 切片；`getStringConst(SliceType, Payload)` 复制调用方已验证、解码的 UTF-8 字节，以完整字节序列比较，支持空串、内嵌 NUL 和非 ASCII 内容，不做转义解码或 Unicode 规范化。`value()` 返回池拥有的稳定 `std::string_view`，长度不包含额外终止符。不同源码位置或不同转义拼写只要解码内容相同就复用常量；AST 节点仍独立保存来源。
- `IntegerConstant` 保存自有 `IntegerBits`，由位宽与低位字在前的 `uint64_t` 字数组组成；符号性由 `IntegerType` 决定，负数使用二进制补码。单字构造允许显式零扩展到宽于 64 位的表示，多字构造复制全部输入；`valid()` 检查非零位宽、精确字数与最后一个字的未用高位，常量池拒绝无效表示及类型宽度不匹配，不静默丢弃或补齐输入字。
- `FloatConst` 保存自有 `FloatBits`，由 IEEE binary16/32/64 位宽与 `uint64_t` 原始位模式组成；`valid()` 拒绝不支持的位宽及编码之外的高位。`getFloatConst(FloatType, Payload)` 验证表示有效且位宽与类型匹配，再按完整位模式驻留，区分正负零、无穷、NaN 符号、静默/信号位与 payload。输入必须已经采用对应 IEEE 格式编码，不通过宿主浮点类型转换。十进制字面量解析、浮点运算、格式转换、舍入和溢出诊断由后续语义分析负责；池只保存位表示。
- `Value` 是语义值基类，当前分支为 `Type`、`Constant`、`ExprValue`、`Function` 和抽象指令基类 `Instruction`。`Type` 下分 `BuiltinType` 与 `UserDefinedType`：builtin 提供元类型、void、bool、整数、IEEE binary16/32/64 浮点、定长数组、切片、指针、引用和函数类型；user-defined 提供 `ClassType`、`EnumType` 与 `InterfaceType`。所有类型值的类型是元类型，元类型的类型为自身。结构类型和常量在上下文内规范化；名义类型拥有独立身份，由分析器复用同一类型或实例的对象。不同上下文的对象不可直接混用。semantic 的接口、实现与测试使用项目自有模型及标准库，`ink_semantic` 仅依赖 `ink::core`；LLVM IR 类型转换限定在后端适配层。
- `ExprValue`（`ValueKind::ExprValue`）保存已检查表达式的结果类型、借用的只读 AST 表达式以及文件身份；`context()` 返回所属的 `SemanticContext` 模型存储。`createExprValue()` 校验结果类型归属，由调用方负责表达式检查，不执行表达式，也不标记为仅能在运行时执行。同一 AST 的每次创建均得到独立身份，避免将不同语义分析或实例的结果按 AST 指针错误合并。定义环境、泛型替换环境与 `SemanticContextId` 尚未实现；后续语义旁表须按表达式值身份关联相应的绑定、转换和实例上下文，模型存储上下文不能替代这些环境。
- `ArrayType` 的规范键为元素类型和 64 位长度，多维数组通过嵌套 `ArrayType` 表示；`SliceType` 表示具有运行期长度的视图，不拥有动态容器的分配策略。`PointerType`、`ReferenceType` 和 `SliceType` 的规范键都包括目标类型和 `AccessKind`，且三种类型使用不同存储。访问权限与 `Variable::isMutable()` 分别描述间接访问和绑定可变性，`ReferenceType` 不替代表达式的值/位置类别。即使元素或目标是用户定义类型，这些语言内建类型构造器仍归 `BuiltinType`。
- `FunctionType` 保存已确定的固定参数签名，`getFunctionType(ReturnType, ParameterTypes)` 按返回类型和有序形参类型身份驻留，哈希命中后仍精确比较完整签名。返回和形参类型必须属于当前上下文，形参不能为空；形参列表复制为类型自身拥有的存储，支持零参数、void 返回值、名义类型及嵌套函数类型。参数名、默认值、参数包和泛型绑定不放入这份签名，参数类型的语言合法性仍由分析器检查。
- `Decl` 只保存名称和借用的只读 AST，派生类不增加字段；不保存上下文、种类、重复的源码位置、类型、签名或初始化值。当前只有泛型函数和泛型类生成语义 `Decl`：`createFunctionDecl(Name, AST)` 与 `createClassDecl(Name, AST)` 检查 AST 含有泛型参数，普通函数和普通类返回空指针。`FunctionDecl::ast()`、`ClassDecl::ast()` 返回具体 AST，`classof()` 根据 AST 分类；参数、函数体、类成员及范围从 AST 读取。语义层暂不提供 `VarDecl`。模板保持不变，类型检查结果、定义环境和实例状态由独立对象或旁表保存。
- `Function` 是具有已确定 `FunctionType` 和独立身份的函数值，由 `createFunction(Name, Signature)` 创建，供普通函数和闭合泛型实例使用。`CallInstruction` 保存函数类型的 `Value` 引用及实参引用列表；`directCallee()` 识别 `Function`，其他函数值由 `indirectCallee()` 返回，`callee()` 提供统一入口。`functionType()` 返回签名，`type()` 返回签名的返回类型。工厂拒绝非函数目标、外来上下文对象、空实参及数量或类型不匹配。泛型 `FunctionDecl` 不能直接调用，须先实例化得到闭合函数值。每次调用独立分配，列表和对象地址在扩容后保持稳定；创建不执行函数，也不进行重载选择、参数转换或泛型实例化。
- `createClassType(Name)`、`createEnumType(Name)` 和 `createInterfaceType(Name)` 直接创建具体名义类型，不经过语义 `Decl`。对象地址代表类型身份，同名对象不合并；分析器负责按普通类型或泛型实例身份复用已经创建的对象，避免仅按名字或共享 AST 合并不同实例。成员、基类、枚举底层类型、接口约束、布局和实例缓存仍待实现。组合类型工厂验证上下文归属与访问权限，不在存储层决定数组元素合法性、大小限制、引用折叠或可空性等语言规则。
- `Variable` 保存普通变量绑定的名称、上下文、可变性、已解析类型和初始化值，独立于 `Decl` 与 `Value`。`createVariable(Name, Mutability, Initializer)` 检查名称、`BindingMutability` 和初始化值归属；`Mutable/Immutable` 分别对应 `var/const`。`setType()` 与 `setInitializer()` 各发布一次，允许重复发布同一对象，拒绝冲突及外来对象；两者都有值时类型必须一致。初始化值可以是本地类型、常量、函数或表达式结果。每个绑定具有独立身份，不保存 AST 或执行时的变量内容；源码绑定关系由后续分析器旁表记录。
- 泛型定义和表达式值借用的 AST 所属 `ParsedUnit` 必须保持存活，当前上下文不拥有或复制 AST。源码中的普通 `parser::VarDecl`、`parser::FunctionDecl` 和 `parser::ClassDecl` 是语法节点，不意味着创建同名语义 `Decl`。执行期变量槽位、编译期结果及语义分析状态分别保存，不能写回共享泛型 AST。

这一阶段不提供 `Scope`、`LookupResult`、`Binding`、`ExprInfo`、名称解析、comptime 或 IR lowering。下文的 ID、类型独立存储门面、扩展常量种类和会话结构仍为后续接口规划；当前类型、常量和声明引用使用上下文内稳定指针，不能直接持久化。

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
| `EvalFrame` | 一次实际 comptime 调用的参数槽、局部对象、临时值和清理状态；决定这一次执行读到什么值 |

同一函数 AST 可以对应多个实例；同一实例又可以有多次调用。`F::[i32]` 与 `F::[i64]` 的类型检查结果分开保存，而同一个 `F::[i32]` 分别以 `1`、`2` 调用时，只需分开调用帧，不因普通实参值不同而创建新的泛型实例。

如果某个编译期绑定确实改变成员集合、类型或语句展开，它必须进入新的替换或展开上下文。不能把这类变化隐藏在可变调用帧中，却继续复用旧的类型检查缓存。

当类型计算或泛型应用读取某个帧内值时，先将这次使用需要的值冻结成不可变绑定，再创建相应语义上下文。调用帧本身不进入持久缓存键；之后修改局部变量，也不能改变已经发布的实例或类型。

## 3 类结构与具体职责

以下类位于建议的 `ink::semantic` 命名空间。表中的入口描述逻辑接口，具体返回类型统一遵守第 8 节的显式失败契约。

### 3.1 会话、模块和调度

| 类 | 具体职责 | 主要入口或输出 |
| --- | --- | --- |
| `SemanticSession` | 统一拥有语义存储、模块和缓存；借用 Core 的目标、源码及诊断设施；保证句柄和 AST 生命周期 | `getModule()`、`types()`、`constants()`、`queries()` |
| `SemanticModule` | 保存某一模块的 AST 单元、导入/导出索引、模块作用域、定义环境、全局初始化结果和函数体结果 | 声明索引、`DefinitionEnvironment`、`CheckedBody` |
| `SemanticDriver` | 语义层入口；协调模块分析、必需实例、comptime 请求和最终验证；不把所有节点处理逻辑堆在此类 | `analyzeModule()`、`completeModule()` → `SemanticResult` |
| `SemanticModuleLoader` | 按模块身份加载源码或语义产物；维护加载中、接口可用、完成、失败状态；按需加载函数体 | `loadModule()`、`loadBody()` |
| `SemanticQueries` | 统一缓存声明头、类型、函数体、布局等确定性查询；登记依赖、检测环、区分合法递归与错误循环 | `resolveDecl()`、`resolveType()`、`checkBody()`、`layoutOf()` |

语义层可以请求 IR 生成，但不得借用已有 `ir::CompilationSession` 存放 AST 泛型状态。后者面向 `ir::Module`；接入完整编译器时由驱动协调语义 Session 与 IR 上下文，避免 `semantic → execution → semantic` 的模块依赖环。

### 3.2 基础存储

| 类 | 具体职责 | 保存的关键数据 |
| --- | --- | --- |
| `DeclStore` | 仅为泛型函数和泛型类保存不可变定义；分析及实例状态放在独立旁表 | 名称、借用的只读 AST；普通对象由对应模型存储拥有 |
| `ScopeStore` | 保存模块、类型、函数、块作用域及其名称索引；记录声明的可见条件和导入来源 | `ScopeInfo`、`BindingInfo`、重载集合、可见性边界 |
| `TypeContext` | 规范化内建、名义、复合、函数及元类型；处理类型相等、完整性和目标布局查询 | `TypeId`、类型结构、名义声明身份、布局状态 |
| `ConstantPool` | 驻留不可变的标量、聚合、类型值及允许持久化的符号常量，提供规范相等和哈希 | `ConstValueId`、类型、规范值；不保存可变局部对象 |
| `SemanticInfo` | 按语义上下文保存 AST 的绑定、表达式性质、调用/转换计划和函数体结果 | `ExprInfo`、`CallPlan`、`ConversionPlan`、`CheckedBody` |

`TypeContext` 不应直接等同于现有 `ir::Type`：语义层还要表示元类型、未完成名义类型和依赖类型。`RuntimeLowerer` 在闭合后建立 `semantic::TypeId → ir::Type` 的映射；目标布局规则只能有一个权威实现，后续应复用或重构现有 IR 布局设施。

`ConstantPool` 和 `EvalMemory` 也不能合并。前者内容不可变、可比较并可进入缓存键，后者有对象身份、别名、初始化和销毁过程。

### 3.3 名称、类型与普通语义检查

| 类 | 具体职责 | 主要入口或输出 |
| --- | --- | --- |
| `DeclCollector` | 在允许的作用域登记源码名称和绑定；只有泛型函数及泛型类创建语义 `Decl`，记录尚未激活的区域 | `collectScope()`、`registerDeclaration()` |
| `NameResolver` | 执行词法、模块、成员及导入名称查找，检查访问权限；返回声明、绑定或重载集合 | `lookupName()`、`lookupMember()` → `LookupResult` |
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

`ExpansionBuilder` 可以通过多个 `NodeRef + SemanticContextId` 表示同一语句在不同静态展开中的出现，无须复制整棵树。它提供承载结构变化的机制；`comptime for` 是否生成运行时语句、如何暴露生成的声明，仍由语言规则决定，不能从 AST 包装节点自行推导。

### 3.5 AST 编译期执行

| 类或记录 | 具体职责 | 主要入口或内容 |
| --- | --- | --- |
| `ComptimeEvaluator` | 解释已绑定并检查过的 AST 活动路径，执行表达式、语句和函数调用，按需请求语义查询 | `evaluateExpr()`、`evaluatePlace()`、`executeStmt()`、`callFunction()` |
| `EvalContext` | 一次求值请求的状态集合；关联语义上下文、受控内存、调用栈、预算和诊断轨迹 | 求值模式、目标、请求位置、调用帧栈 |
| `EvalFrame` | 一次函数调用或模块求值的活动记录；将 `BindingId` 映射到参数、局部对象和临时槽 | 所属函数/实例、局部槽、返回目标、清理栈 |
| `EvalMemory` | 管理编译期对象及引用，验证初始化、越界、别名、生命周期和目标布局 | `allocate()`、`load()`、`store()`、`destroy()` |
| `EvalBudget` | 限制步骤、递归、对象数量、分配字节和结构展开数量，支持取消 | 显式预算状态；贯穿嵌套求值与实例化请求 |
| `BuiltinRegistry` | 登记语言内建的签名、编译期实现和运行时表示，确保两种执行方式使用一致的操作契约 | 内建操作描述、允许的编译期能力 |

`EvalContext`、`EvalFrame` 和 `EvalBudget` 可以先实现为小型状态记录；无需为每个字段建立抽象接口。`BuiltinRegistry` 只在需要内建操作时扩展，不把任意宿主函数调用作为默认编译期能力。

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

`ComptimeEvaluator` 解释原 AST，并通过 `SemanticQueries` 取得当前活动节点的语义结论。遇到还未检查的活动节点时，先完成所需的绑定、类型、调用及转换检查，再执行；不能通过一次普通 `ASTWalker` 遍历来求值。

| 场景 | 行为 |
| --- | --- |
| `ComptimeExpr`、必须确定的泛型实参、类型表达式 | 使用 `RequireConstant`；运行时依赖、非法操作和无法持久化的结果显式失败 |
| 编译期执行某个语句或普通函数 | 使用独立 `EvalContext`/`EvalFrame` 执行其活动 AST 路径；普通函数不必再复制为专用 comptime 函数类 |
| 普通运行时表达式 | 由分析器建立运行时语义结果，保留给 lowering；不要求交给求值器先尝试执行 |
| 尚未绑定的泛型上下文 | 保留 `Dependent` 结果；在必须闭合的位置仍未解决时报告错误 |
| 可选常量折叠 | 可在以后增加；只能对允许折叠的操作执行，不能吞掉错误或重复副作用 |

第一版不必实现任意 `Known + Runtime` 的部分求值器。显式泛型替换、强制编译期求值和按规则选择活动结构已经能在 AST 上完成；一般运行时函数可以整体保留到 IR。

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

编译期局部变量、参数对象与临时对象由 `EvalMemory` 管理，不能直接使用宿主 C++ 对象地址作为 Ink 指针。整数宽度、浮点行为、布局和指针语义依据目标及语言规则实现，不能借用宿主未定义行为完成计算。

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

可变编译期内存默认属于当前求值请求，不隐式跨请求共享。模块级编译期初始化结果应显式冻结并进入模块产物。普通运行时全局变量的当前值不能作为编译期输入；文件、环境、宿主调用等能力若以后开放，需要在 `BuiltinRegistry` 中声明并纳入依赖跟踪。

## 7 跨模块泛型与编译产物

### 7.1 A 定义泛型，B 使用实例

设 A 定义 `F1`，B 使用 `A.F1::[i32]`。建议采用以下逻辑产物；`A.inkmod` 是语义产物的暂定文件名，不在本文冻结二进制格式。

| A 的产物 | 保存的内容 | B 如何使用 |
| --- | --- | --- |
| `A.inkmod` | 导出索引、声明身份、参数和签名、需要延迟分析的 AST、定义环境、编译期支持体、类型/常量和依赖指纹 | 找到 `F1`，恢复其定义环境，绑定 `i32`，在 AST 上完成实例化及 comptime |
| `A.obj` 或相应目标代码 | A 中需要运行时定义的普通函数、已选择生成的闭合实例、全局数据和模块初始化 | 与 B 生成的代码链接 |

B 的 `InstanceStore` 保存这个实例，AST 可以仍由加载后的 A 模块持有。实例语义结果属于使用它的编译会话，不回写 A 的共享定义，也不修改磁盘中的 `A.inkmod`。

本方案采用使用方生成其所需实例。B/C 对相同实例的符号身份必须一致，链接时通过目标平台支持的重复定义合并机制处理，或由编译驱动统一安排唯一生成者；不能依赖每个模块随意分配的 `InstanceId` 拼接链接名。

### 7.2 模块存档的必要内容

`CompiledSemanticModule` 至少包含：

1. 模块身份、存档及语义版本、目标约束、编译配置和依赖指纹。
2. 导出与可被定义环境引用的声明索引、可见性、泛型参数、默认实参及签名所需信息。
3. 泛型体、必须在导入方求值的表达式，以及它们可达的 comptime 支持函数体；不必保存所有无关函数体。
4. 定义作用域、导入绑定、稳定声明引用和依赖名称查找配方，含所需私有依赖。
5. AST 节点字段、嵌入记录、源码定位和字面量所需的 Token/payload；规范类型、常量及模块级编译期结果。
6. 按声明/函数组织的块索引，支持接口先加载、函数体按需加载。

当前 `LiteralExpr` 保存 `TokenId`，名称还可能引用源码存储。只写 AST 子节点关系无法恢复可执行语义。存档可保留所需源码与 Token/payload，或者在存储格式中显式编码等价字面量和名称数据；两种方案都必须支持诊断定位。

已经完成的绑定旁表可以作为存档内容，但必须使用可重定位身份；每个实例的类型、重载选择和求值帧不能当作模板定义的唯一答案保存。A 的模块级已冻结结果在导入时读取；依赖 B 本次泛型实参的结果在 B 实例化时计算。

### 7.3 私有依赖与诊断

若 `F1` 使用 A 的私有 `Helper`，内部引用保持 `GlobalDeclRef(A, Helper)`，B 中的同名符号不能改变它。接口可见性和为了实例化而保存实现信息是不同问题：保存私有支持体并不使其变成可供 B 源码直接查找的公开名字。

若 `Helper` 在 comptime 被调用，需要保存其 AST 及传递依赖；若只在运行时调用，可由 A 的目标文件提供定义，但必须有能从 B 生成实例引用的链接符号，不能错误地仅保留为 A 目标文件的局部符号。

错误应同时指向 A 中失败操作、B 中实例化请求及完整泛型/求值调用链。A 的泛型定义可用，不代表每组实参都合法；实例化失败属于具体实例的诊断。

### 7.4 存档验证

Reader 必须检查版本、长度、分配预算、种类编号、必需子节点类别、引用范围、数组长度、源码范围和 Token/payload 引用。节点与 `Parameter`、`Argument`、属性等嵌入记录均须显式编码；`forEachChild()` 不能替代字段序列化。

加载模块时先验证头部、接口、身份映射和块索引，再发布可查询接口；按需加载某个 AST 块时，必须完成该块的字段、引用及结构验证后才发布函数体。资源预算覆盖所有已加载块，不能通过多次局部加载绕过总量限制。

节点指针、`std::span` 和 `string_view` 的内存布局均不是文件格式。稳定 ASTKind 编号帮助识别节点种类，不自动提供字段格式兼容性。

## 8 错误、诊断与闭合验证

### 8.1 显式结果

统一定义 `SemanticResult<T>` 或等价结果类型，区分成功、语言错误、资源限制和取消。查询的“仍依赖泛型参数”属于显式分析结果；名称不存在、模块不存在、候选不适用也应有自己的可判定状态，不能全部压成空指针。

`SemanticDiagnosticEmitter` 只适配 Core 的 `Diagnostic`、`DiagnosticKind`、`SourceId` 和 `SourceRange`。新诊断种类加入 Core 的统一定义，不在 semantic 重建公共诊断容器。失败缓存保存稳定原因；每次使用时按当前请求补充实例化轨迹，避免重复输出过时的调用位置。

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

头文件放在 `src/include/ink/semantic`，实现放在 `src/lib/semantic`。按职责分组即可，不必给每个小记录单独建立文件。

| 文件组 | 主要内容 |
| --- | --- |
| `semantic.h`、`semantic_result.h`、`semantic_ids.h` | 对外入口、结果状态和强类型身份 |
| `semantic_session.h`、`semantic_module.h`、`semantic_driver.h` | 会话、模块、调度入口 |
| `decl_store.h`、`scope_store.h`、`type_context.h`、`constant_pool.h` | 基础存储 |
| `semantic_info.h`、`semantic_queries.h` | 语义上下文、旁表、各类计划和查询缓存 |
| `decl_analyzer.h`、`name_resolver.h`、`type_resolver.h` | 声明收集/完成、名称和类型解析 |
| `expr_analyzer.h`、`stmt_analyzer.h`、`call_resolver.h` | 表达式、语句和调用分析 |
| `conversion_checker.h`、`pattern_analyzer.h` | 转换、初始化、模式分析 |
| `generic_binder.h`、`generic_instantiator.h`、`instance_store.h` | 泛型绑定、实例完成和缓存 |
| `expansion_builder.h` | 编译期结构展开及来源映射 |
| `comptime_evaluator.h`、`eval_context.h`、`eval_memory.h`、`builtin_registry.h` | AST 求值、帧/值/位置/预算、受控内存及内建操作 |
| `flow_analyzer.h`、`cleanup_planner.h` | 控制流分析和清理计划 |
| `semantic_verifier.h`、`runtime_lowerer.h` | 闭合验证和 IR 生成 |
| `semantic_module_loader.h`、`semantic_module_io.h` | 模块加载和存档读写 |
| `semantic_diagnostic_emitter.h` | 语义诊断及实例化/调用轨迹 |

依赖方向为 `semantic → core / tokenizer / parser`，IR lowering 适配部分另依赖 `ir`。Core 和 Parser 不反向依赖 semantic，comptime 求值不依赖运行时 ExecutionEngine 或 LLVM。是否把 lowering 和模块存档拆为独立 target 可在接入时决定，不影响上述类边界。

semantic 对象模型已使用显式源文件列表接入构建，采用与 Parser 公共头兼容的 C++20，并保持目标级禁用异常；测试加入统一的 `ink_tests`。后续新增分析器实现时继续维护目标源文件列表。

### 9.2 实现阶段

| 阶段 | 实现内容 | 应达到的结果 |
| --- | --- | --- |
| 1. 普通语义纵切片 | Session/Module、身份与基础存储、声明/名称/类型/表达式/语句分析、最小查询状态、Verifier | 普通函数形成可验证的 `CheckedBody`，有确定性语义 dump |
| 2. 最小 AST comptime | 求值器、帧、值/位置、受控内存、预算和必要的控制流/清理 | 同一个普通函数可在独立编译期调用中返回不同结果；类型位置可请求求值 |
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
| AST comptime 与闭合 IR 的普通运行时执行 | 对已支持操作得到一致的数值、控制流和清理结果；覆盖调用与构造的区别 |
| 完整源码编译 | 从 `.ink` 经过语义、实例化/求值、闭合 IR 验证到运行时结果，而非只测孤立分析器 |
