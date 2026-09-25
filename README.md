# ink

当前按 [`docs/Ink-Lexical-Rules.md`](docs/Ink-Lexical-Rules.md)、[`docs/Ink-grammar-Rules.bnf`](docs/Ink-grammar-Rules.bnf) 和 [`docs/Ink-Parser-Design.md`](docs/Ink-Parser-Design.md) 构建前端，包含 Core、CLI 支持库、tokenizer、parser、semantic 对象模型及分析入口占位接口、`ink-tokenize`、`ink-parse` 和测试。`semantic::Analyzer::analyze` 当前为空实现，始终返回空指针；`NameResolver` 提供词法作用域、名字绑定和函数重载候选集合。旧 IR 和 execution 目录已删除；旧 backend 和 interpreter 工具仍未接入当前构建。测试分别位于 `src/testcase/tokenizer`、`src/testcase/parser` 与 `src/testcase/semantic`。

## 构建

初始化固定版本的第三方依赖：

```powershell
git submodule update --init --recursive
```

使用 Visual Studio 2022 生成、编译 LLVM/Clang 依赖和 Ink，并运行全部测试：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target ink_tests
ctest --test-dir build -C Release --output-on-failure
```

`ink_tests` 是统一的 GoogleTest 入口，包含 tokenizer、parser 和 AST 的全部单元测试；在 CLion 中运行同名配置即可执行这些用例。构建该目标同时构建 `ink-tokenize` 和 `ink-parse`，CTest 再运行额外的 CLI 进程测试。

LLVM 的构建树包会生成到 `build/third_party/llvm/lib/cmake/llvm`，主工程通过 `LLVM_DIR` 和 `find_package(LLVM CONFIG)` 导入。`utf8proc 2.9.0` 固定使用 Unicode 15.1，与当前词法规则的 Unicode 版本一致。命令行工具统一使用仓库自有的 `ink::cli::Application` 解析参数，并通过 `ParseResult` 显式返回解析和定义错误，不使用 C++ 异常作为控制流。spdlog 1.17.0 负责 Ink 的统一文本输出和运行时日志。

Ink 所有工具共用的参数拼写、输入输出、诊断和退出码规则见 [`docs/command-line.md`](docs/command-line.md)。

基于 [`docs/grammar.bnf`](docs/grammar.bnf) 交互确认的语义分析、泛型、comptime 和新 IR 设计规则见 [`docs/IR.md`](docs/IR.md)。该文档记录设计约定，不表示相关编译管线已经实现。

Ink 跨 module 函数、成员函数、闭合实例、全局变量、Imported 符号和 `extern "C"` 的链接名称规则见 [`docs/name-mangling.md`](docs/name-mangling.md)。

## Tokenizer 接口

- `tokenize(FrontendContext &, std::string)` 或 `tokenizeSource(FrontendContext &, SourceId)` 返回持有源码和解码值的 `TokenizedBuffer`。先检查 `succeeded()`；成功结果以唯一的 `END_OF_FILE` 结束。全局 UTF-8／NUL／BOM 校验失败不输出 token，其他词法错误保留此前完成的 token，不追加 EOF。
- `TokenKind` 为扁平枚举，C++ 枚举项按仓库约定使用 UpperCamelCase，`tokenKindName()` 返回文档中的大写名称。空白、注释和错误不占 token 种类。标识符的 NFC 名称、字符串的解码 UTF-8 值和字符的 Unicode 标量值分别保存在 payload 中。
- Core 的 `SourceLocation` 使用 32 位不透明编码，默认无效，`fromByteOffset(0)` 表示有效的文件起点。`SourceRange` 始终为原始字节的半开区间，可通过 `getBegin()`／`getEnd()` 获取位置。文件身份由源码 buffer 或诊断的 `Source` 保存；不同文件的位置不可直接比较。超过 `0xFFFFFFFE` 字节的输入显式报 `SourceTooLarge`。
- Unicode 正式名称复用 LLVM 的公开名称查询接口；完整别名由官方 Unicode 15.1 `NameAliases.txt` 生成。更新方式见 `cmake/generate_unicode_aliases.py`，生成表记录原始数据的 SHA-256，XID 和 NFC 数据不复制到本项目。

## Parser 接口

- `ink::parser::parse(FrontendContext &, TokenizedBuffer, ParseLimits)` 返回 `ParseResult`。`Unit` 持有不可变 TokenBuffer、ASTContext、ModuleAST 和恢复记录；节点和源码引用在 Unit 销毁前有效。`succeeded()` 同时检查词法结果、当前调用的语法错误和解析状态。
- `Completed` 表示扫描结束，错误输入仍能返回恢复后的 AST。`LimitExceeded` 和 `Cancelled` 明确标识部分结果。ParseLimits 控制嵌套、诊断、工作量与 AST 分配；分配预算触发停止后仍允许构造父节点和最终列表，以返回结构完整的部分树。
- `ASTVisitor`／`ConstASTVisitor` 只分派当前节点，`StrictExprVisitor` 要求覆盖全部表达式。`ASTWalker` 使用显式栈按源码顺序遍历，支持跳过子节点和提前停止。`verifyAST` 校验结构契约；`dumpAST` 返回字符串，不直接产生进程输出。
- `ink-parse INPUT` 或通过标准输入运行 `ink-parse -` 可查看结构与恢复记录。正常退出为 0，词法或语法错误为 1，输入读取失败为 2。
- 测试包含文法家族、恢复边界、Arena 析构与回滚、Visitor、源码生命周期、长链、资源预算、Token 边界扰动及确定性随机输入。可运行 `cmake --build build --config Release --target run_all_tests` 执行完整回归。

## Semantic 分析接口

`Analyzer` 的头文件和实现放在 semantic 的 `analyze` 子目录；名字解析放在 `name_resolve` 子目录，拆分为 `binding.h`、`scope.h`、`name_resolver.h` 和 `name_resolver.cpp`，三个类型均位于 `ink::semantic` 命名空间。

`Analyzer::analyze(SemanticContext &, const parser::ParseResult &, std::string_view ModuleName)` 为成员函数，当前仍返回 `nullptr`。`NameResolver` 通过 `enterScope()` 和 `exitScope()` 管理当前作用域，直接绑定 `Value *`；`lookup()` 查找当前及父作用域，`lookupLocal()` 仅查当前作用域。`enterScope(Owner)` 为实体创建成员作用域，`lookupMember(Owner, Name)` 查找该实体的直接成员，别名共享同一实体的成员绑定。`Function` 支持同名候选集合，普通绑定重名返回冲突。重载选择和 AST 分析尚未接入。接口、生命周期和状态码见 [语义分析接口](docs/Ink-Semantic-Analysis.md)。

## Semantic 对象模型

这套模型提供函数、调用、内存、加法和返回节点的构造接口，源码语义分析、解释器或后端及完整图验证器仍待实现。当前边界与后续缺口见 [可执行 IR 状态](docs/Ink-Executable-IR-Status.md)。

- `createFunction()` 根据签名创建 `FunctionParameter`，通过 `parameters()` 访问；形参通过 `outer()` 关联函数，`function()` 从该父节点取得所属函数，不再重复保存 Owner；同时保存 `Name ParameterName`（通过 `name()` 访问）、零起始索引、值类型和 `ParameterKind`（Positional、Named、Variadic），通过 `parameterKind()` 查询。`createFunction()` 的可选种类列表必须与签名槽位数量一致，省略时全部为 Positional；第四个可选参数 `ParameterNames` 按签名顺序提供名称，省略时参数匿名，由调用方驻留并填写形参名；种类是绑定元数据，不改变规范化运行时签名或开启变参展开。`createAddInstruction()` 接受同型整数操作数，定义按位宽回绕的加法；`createReturnInstruction(ReturnedValue)` 创建未挂接的 void 类型终结节点，只校验操作数归属和非 void 类型；`appendValue()` 校验目标块属于函数且返回值匹配该函数签名。返回指令不保存 Owner，`function()` 沿 outer → BasicBlock → Function 查询，未挂接时返回空指针。

- 对象模型头文件位于 `src/include/ink/semantic/model`，实现位于 `src/lib/semantic/model`；声明基类及其派生类放在 `model/decl` 子目录，函数相关的 `Function`、`FunctionType`、`BasicBlock` 放在 `model/function` 子目录，模块值 `Module` 放在 `model/module` 子目录，`Name` 和 `NamePool` 放在 `model/name` 子目录，其余类型及类型注册表放在 `model/type` 子目录，按需包含对应的独立头文件。
- `src/include/ink/semantic/model/coredefines.h` 集中定义 `ValueKind`、`TypeKind`、`ParameterKind`、`CallingConvention`、`LanguageLinkage`、`AccessKind` 和 `VisibilityKind`，仅依赖 `<cstdint>` 与枚举注册表，可独立包含。`VisibilityKind::Public/Private` 表示声明或成员的可见性，具体访问检查尚未接入。
- `Function` 保存独立的调用约定 `CallingConvention::C/Fast/Cold` 和语言链接规则 `LanguageLinkage::Ink/C`，分别通过 `callingConvention()`、`languageLinkage()` 查询。`createFunction()` 的第五、六个可选参数设置它们，默认是目标平台 C 调用约定与 Ink 语言链接；无效枚举值返回空指针。两者不参与 `FunctionType` 的规范化身份，也不决定函数是否有定义；添加函数体会保留这些属性。当前只记录模型元数据，源码语法、符号命名和后端 ABI lowering 尚未接入；语言链接不表示 external/internal 符号链接性或导出可见性。
- `src/include/ink/semantic/model/Values.def` 集中定义 `ValueKind`，枚举项与 C++ 类同名，如 `IntegerType`、`FunctionType` 和 `CallInstruction`；类型和常量条目分别生成 `Type::classof()`、`Constant::classof()`。元类型、void、bool、label、module 的实际对象均为 `BuiltinType`，由 `TypeKind` 继续区分；仅作为中间基类的 `Type`、`UserDefinedType`、`Constant` 不单独占用值种类。
- `src/include/ink/semantic/model/type/Types.def` 用 `INK_SEMANTIC_TYPE(Name, Base)` 集中登记类型种类，`Base` 配置为 `BuiltinType` 或 `UserDefinedType`；`TypeKind` 与两个基类的 `classof()` 分类判断均由该表生成。
- `src/include/ink/semantic/model/instruction` 保存直接继承 `Value` 的 `CallInstruction`、`AllocaInstruction`、`LoadInstruction` 和 `StoreInstruction`，每种指令有自己的头文件，使用同名 `ValueKind` 分类，由 `SemanticContext` 创建并管理。
- `SemanticContext` 位于 `src/include/ink/semantic/context.h`，实现位于 `src/lib/semantic/context.cpp`；借用 `core::CompilationContext`，拥有 `NamePool`、`ConstantPool`、结构类型、名义类型、函数值、基本块、模块值、调用与内存指令、模块声明和泛型声明；对象地址在上下文存活期间保持稳定。后续语义 Session 可以组合这份存储。
- `constantPool()` 返回当前上下文唯一的常量池，负责 bool、任意位宽整数、`StringConstant` 和 `FloatConstant` 的创建、所有权与去重；`SemanticContext` 上的常量工厂转发到同一池。规范类型身份和完整 payload 决定常量身份，哈希命中后仍精确比较，类型来自其他上下文或 payload 与类型不匹配时返回空指针且不插入。`owns()` 检查具体对象的池归属，`size()` 包含创建时已有的 false、true 两个常量；池只由所属上下文创建，不可复制、移动或清空。
- `getStringConstant(SliceType, Payload)` 要求只读 `u8` 切片类型，复制并驻留调用方已经验证、解码的 UTF-8 字节，支持空串和内嵌 NUL，不重复解码转义或执行 Unicode 规范化；相同解码字节只保存一个常量。`getFloatConstant(FloatType, FloatBits)` 使用项目自有的 IEEE binary16/32/64 位表示，要求位宽与类型完全一致且没有多余高位；按位去重，区分正负零、NaN 符号和 payload，不隐式转换或舍入。常量及字符串视图在上下文存活期间保持有效；源码字面量语义和后端 lowering 仍待实现。
- `Name` 只有一个 32 位索引，默认无效。`namePool().intern(Text)` 为相同字节串复用索引，`find(Text)` 不插入，`text(Name)` 返回池拥有的稳定视图。名称只在所属池内比较；调用方负责携带池或上下文，不能将一个池的索引交给另一个池解释。池不执行词法验证或 Unicode 规范化，前端名称应来自已经验证的 token。
- `Type`、`Constant`、`Function`、`BasicBlock`、`Module`、`CallInstruction`、`AllocaInstruction`、`LoadInstruction` 和 `StoreInstruction` 都继承 `Value`，`Decl` 独立于该层次。`Type` 分为 `BuiltinType` 与 `UserDefinedType`：前者包括元类型、void、bool、整数、IEEE 16/32/64 位浮点、定长数组、切片、指针、引用和函数类型；后者包括以独立对象身份区分的 `ClassType`、`EnumType`、`InterfaceType`。类型值的类型为元类型，元类型的类型为自身。当前常量包括 bool、整数、字符串和浮点；整数 payload 使用自有 `IntegerBits`（位宽与低位字在前的 `uint64_t` 字数组），浮点使用自有 `FloatBits`（IEEE 位宽与 `uint64_t` 原始位模式），字符串拥有完整的解码字节。常量池拒绝无效位宽、错误字数和多余高位，不隐式截断或扩展。
- `Value` 统一保存所属 `SemanticContext` 和自身类型 `const Type &ValueType`，派生类通过继承的 `context()`、非虚 `type()` 查询。类型在构造时确定，派生类不重复保存自身类型，也不沿操作数链递归推导；元类型的自身类型指向自己。`Function::functionType()` 提供函数签名访问，数组元素类型、函数返回类型等类型结构成员仍由具体类型保存。
- `Value::outer()` 返回可空的结构父节点，内存生命周期仍由 `SemanticContext` 管理；顶层模块、未挂接对象、类型和常量没有父节点。父子关系只能通过 `SemanticContext` 的结构编辑接口维护，每个对象最多有一个结构父节点；调用目标、实参等引用不会改变被引用对象的 `Outer`。
- `Value`、`Type`、`BuiltinType`、`UserDefinedType`、`Constant` 和 `Decl` 的基类构造函数使用 `protected`，字段保持 `private`；派生类无需逐个列入基类友元名单。具体模型类保留私有构造函数，仅授权 `SemanticContext`、`ConstantPool` 或 `NamePool` 等创建入口。结构父子关系的写权限集中给 `SemanticContext`，基类不再授权具体容器。
- semantic 的公共接口、实现和测试使用项目自有模型与标准库，`ink_semantic` 对象模型依赖 `ink::core`，保留对 `ink::parser` 的构建依赖。名称池采用拥有字符串键的标准容器，哈希仅用于内存查找，分类通过 `classof()` 完成；LLVM IR 类型转换属于后端适配层。
- `getArrayType(ElementType, ElementCount)` 按元素类型和 64 位定长长度复用数组类型；多维数组通过嵌套构造。`getSliceType()`、`getPointerType()`、`getReferenceType()` 按目标类型与 `AccessKind::ReadOnly/ReadWrite` 复用类型。访问权限表示能否通过该值修改目标，与变量绑定本身的可变性分开；引用类型也与表达式的值/位置类别分开。这些类型构造器本身属于 builtin，即使目标是用户类。
- `getFunctionType(ReturnType, ParameterTypes)` 按返回类型和有序形参类型列表复用固定参数签名，支持零参数与 void 返回值；`FunctionType` 拥有列表存储，所有组成类型必须属于当前上下文，空形参指针被拒绝。参数名称、默认值、参数包和泛型绑定由后续分析负责。
- `Decl` 保存名称、借用的只读 AST 和 `Child` 子声明列表；`Child` 为 `std::vector<const Decl *>`，通过 `child()` 的可写或只读重载访问，由调用方按顺序填充，不拥有子声明。`createModuleDecl(Name, ModuleAST)` 创建模块声明；`createFunctionDecl(Name, AST)` 与 `createClassDecl(Name, AST)` 要求 AST 含有泛型参数，普通定义返回空指针。三个工厂均返回可写声明指针，声明仍由上下文拥有。派生类不增加字段，基类 `ast()` 返回 `ASTNodeBase`，`ModuleDecl`、`FunctionDecl`、`ClassDecl` 的 `ast()` 返回具体 AST，`classof()` 从 AST 分类。`Decl` 不保存上下文、种类、重复的源码信息、已解析类型或初始化值，语义层暂不提供 `VarDecl`。泛型定义、实例结果和执行时状态分别保存，AST 所属 `ParsedUnit` 必须比语义上下文活得更久。
- `createFunction(Name, Signature)` 创建具有独立身份和本地 `FunctionType` 的 `Function` 值，供普通函数或已闭合的泛型实例使用。`createCallInstruction(Callee, Arguments)` 直接调用 `Function`，或间接调用其他类型为 `FunctionType` 的值；泛型 `FunctionDecl` 需要先实例化，不能直接作为调用目标。实参须已完成排序和转换，数量和逐项类型必须精确匹配。指令拥有实参引用列表，每次调用均创建独立对象；创建不执行函数，不进行重载解析或泛型实例化。
- `Function` 以 `std::vector<BasicBlock *> Blocks` 保存函数体，`blocks()` 提供只读列表；空列表表示没有函数体，`hasBody()` 检查列表是否非空。`entryBlock()` 返回首块的可写或只读指针，列表为空时返回空指针，不重复保存入口字段。`SemanticContext::createBasicBlock(Function &)` 按顺序追加新块，并将各块的 `Outer` 设置为同一函数；首块自动成为入口，外来函数返回空指针。`createFunctionBody(Function &)` 保留为只创建首块的便捷接口，已有函数体时返回空指针。外部函数可以保持无函数体，块和函数均由上下文拥有；列表顺序不代表执行顺序，块内通过 `appendValue()` 填充值。源码返回检查、跳转节点、控制流连接和完整返回路径检查仍待实现。
- `BasicBlock` 继承 `Value`，使用 `ValueKind::BasicBlock` 分类；`createBasicBlock()` 创建由上下文拥有的独立基本块并返回可写指针。`type()` 返回本上下文唯一的 `getLabelType()`，其种类为 `TypeKind::Label`。`Values` 为 `std::vector<Value *>`，`values()` 只提供只读列表；`SemanticContext::appendValue(BasicBlock &, Value &)` 按顺序插入并设置子值的 `Outer`，要求两个对象都属于调用上下文，拒绝已有父节点、循环包含、类型和常量；返回指令另外要求目标块属于函数且返回值匹配其签名。`SemanticContext::removeValue(BasicBlock &, Value &)` 移除本上下文的直接子值并清空其 `Outer`，允许随后显式加入另一容器；失败返回 false 且不改变关系。列表不拥有对象的内存。函数和指令工厂返回可写指针以支持插入；类型和常量池仍返回只读对象，可通过调用参数等引用。
- `model/module/module.h` 的 `Module` 直接继承 `Value`，保存名称和 `EntryBlock` 引用。`createModule(Name)` 校验名称后创建独立模块及其空入口块，两者都由 `SemanticContext` 拥有；`entryBlock()` 提供可写和只读访问，入口块的 `Outer` 由上下文在创建模块时设置。`type()` 返回 `getModuleType()`，其种类为 `TypeKind::Module`。模块值与借用文件 AST 的 `ModuleDecl` 分开，函数与局部名称解析、模块运行时执行仍待实现。
- `createClassType(Name)`、`createEnumType(Name)`、`createInterfaceType(Name)` 直接创建具有独立身份的名义类型，不创建语义 `Decl`。同名类型不合并，分析器负责复用同一普通类型或泛型实例的返回对象；成员、继承、枚举底层类型、布局和实例缓存仍待实现。组合类型工厂仅检查结构约束；元素合法性、数组布局大小和引用使用规则尚未由分析器检查。
- `createAllocaInstruction(AllocatedType)` 创建单对象、未初始化的分配指令，结果为对应的可读写指针；数组通过 `ArrayType` 表示。当前支持 bool、整数、浮点、指针、引用、切片及这些类型组成的定长数组，拒绝外来类型、元类型、void、label、module、原始函数签名和布局未完成的名义类型。`createLoadInstruction(Address)` 从指针读取并产生元素类型的值，接受只读或可读写指针；`createStoreInstruction(Address, StoredValue)` 要求可读写指针、同一上下文和完全一致的元素类型，结果类型为 void。对象和操作数地址由上下文保持稳定，每次创建均有独立身份，调用者通过 `appendValue()` 安排执行顺序；构建节点不会执行内存操作，也不检查初始化、支配关系或实际地址有效性。
- 执行模型不再保存源码变量绑定对象。源码变量和形参与模型对象的绑定、const 分析仍待实现。初始化和赋值由各自位置的 `StoreInstruction` 表示，读取使用 `LoadInstruction`；`StoreInstruction` 在构造时将自身类型设置为上下文唯一的 void 类型，由 `Value::type()` 返回。
- 空名称或名称索引空间耗尽返回无效 `Name`；无效或越界索引查询返回空视图。整数工厂拒绝零位宽，浮点工厂拒绝 16/32/64 以外的位宽；组合类型工厂拒绝外来类型和无效访问权限。命名对象工厂拒绝无效或越界名称，内存指令工厂另检查地址、访问权限和操作数类型，泛型声明工厂另检查 AST 的泛型参数。上述检查返回显式状态，不依赖异常。
