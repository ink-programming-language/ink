# ink

当前按 [`docs/Ink-Lexical-Rules.md`](docs/Ink-Lexical-Rules.md)、[`docs/Ink-grammar-Rules.bnf`](docs/Ink-grammar-Rules.bnf) 和 [`docs/Ink-Parser-Design.md`](docs/Ink-Parser-Design.md) 构建前端，包含 Core、CLI 支持库、tokenizer、parser、semantic 对象模型、`ink-tokenize`、`ink-parse` 及其测试。parser 已替换为 ASTKind 单继承与 Visitor 架构，使用 C++20。semantic 已接入构建，当前提供名称池、类型、常量、声明和调用指令的基础存储，尚不执行名称解析或源码语义分析。IR、后端和执行器尚未接入新的前端接口，不参与构建。词法、语法和语义模型测试分别位于 `src/testcase/tokenizer`、`src/testcase/parser` 与 `src/testcase/semantic`。

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

## Semantic 对象模型

- 对象模型头文件位于 `src/include/ink/semantic/model`，实现位于 `src/lib/semantic/model`；声明基类及其派生类放在 `model/decl` 子目录，类型基类、派生类及类型注册表放在 `model/type` 子目录，按需包含对应的独立头文件。
- `src/include/ink/semantic/model/Values.def` 集中定义 `ValueKind`，枚举项与 C++ 类同名，如 `IntegerType`、`FunctionType`、`ExprValue` 和 `CallInstruction`；类型、常量和指令条目分别生成 `Type::classof()`、`Constant::classof()`、`Instruction::classof()`。元类型、void、bool 的实际对象均为 `BuiltinType`，由 `TypeKind` 继续区分；仅作为中间基类的 `Type`、`UserDefinedType`、`Constant`、`Instruction` 不单独占用值种类。
- `src/include/ink/semantic/model/type/Types.def` 用 `INK_SEMANTIC_TYPE(Name, Base)` 集中登记类型种类，`Base` 配置为 `BuiltinType` 或 `UserDefinedType`；`TypeKind` 与两个基类的 `classof()` 分类判断均由该表生成。
- `src/include/ink/semantic/model/instruction` 保存抽象指令基类 `Instruction` 及具体指令；当前 `CallInstruction` 使用同名 `ValueKind`，其 `type()` 为调用签名的返回类型。
- `SemanticContext` 借用 `core::CompilationContext`，拥有 `NamePool`、`ConstantPool`、结构类型、名义类型、表达式值、函数值、调用指令、普通变量绑定和泛型声明；对象地址在上下文存活期间保持稳定。后续语义 Session 可以组合这份存储。
- `constantPool()` 返回当前上下文唯一的常量池，负责 bool、任意位宽整数、`StringConst` 和 `FloatConst` 的创建、所有权与去重；`SemanticContext` 上的常量工厂转发到同一池。规范类型身份和完整 payload 决定常量身份，哈希命中后仍精确比较，类型来自其他上下文或 payload 与类型不匹配时返回空指针且不插入。`owns()` 检查具体对象的池归属，`size()` 包含创建时已有的 false、true 两个常量；池只由所属上下文创建，不可复制、移动或清空。
- `getStringConst(SliceType, Payload)` 要求只读 `u8` 切片类型，复制并驻留调用方已经验证、解码的 UTF-8 字节，支持空串和内嵌 NUL，不重复解码转义或执行 Unicode 规范化；相同解码字节只保存一个常量。`getFloatConst(FloatType, FloatBits)` 使用项目自有的 IEEE binary16/32/64 位表示，要求位宽与类型完全一致且没有多余高位；按位去重，区分正负零、NaN 符号和 payload，不隐式转换或舍入。常量及字符串视图在上下文存活期间保持有效；源码字面量的语义分析与 IR lowering 尚未接入。
- `Name` 只有一个 32 位索引，默认无效。`namePool().intern(Text)` 为相同字节串复用索引，`find(Text)` 不插入，`text(Name)` 返回池拥有的稳定视图。名称只在所属池内比较；调用方负责携带池或上下文，不能将一个池的索引交给另一个池解释。池不执行词法验证或 Unicode 规范化，前端名称应来自已经验证的 token。
- `Type`、`Constant`、`ExprValue`、`Function` 和 `Instruction` 都继承 `Value`，`Decl` 与 `Variable` 独立于该层次。`Type` 分为 `BuiltinType` 与 `UserDefinedType`：前者包括元类型、void、bool、整数、IEEE 16/32/64 位浮点、定长数组、切片、指针、引用和函数类型；后者包括以独立对象身份区分的 `ClassType`、`EnumType`、`InterfaceType`。类型值的类型为元类型，元类型的类型为自身。当前常量包括 bool、整数、字符串和浮点；整数 payload 使用自有 `IntegerBits`（位宽与低位字在前的 `uint64_t` 字数组），浮点使用自有 `FloatBits`（IEEE 位宽与 `uint64_t` 原始位模式），字符串拥有完整的解码字节。常量池拒绝无效位宽、错误字数和多余高位，不隐式截断或扩展。
- semantic 的公共接口、实现和测试使用项目自有模型与标准库，`ink_semantic` 仅依赖 `ink::core`。名称池采用拥有字符串键的标准容器，哈希仅用于内存查找，分类通过 `classof()` 完成；LLVM IR 类型转换属于后端适配层。
- `createExprValue(ValueType, Expression, Source)` 为已检查表达式记录结果类型、只读 AST 来源和文件身份，`context()` 返回所属的模型上下文。工厂要求结果类型属于当前上下文，由调用方负责表达式检查；创建不会执行表达式，也不会按 AST 节点合并表达式值。表达式 AST 必须保持存活，后续分析器可按独立的 `ExprValue` 身份关联绑定、转换和实例结果。
- `getArrayType(ElementType, ElementCount)` 按元素类型和 64 位定长长度复用数组类型；多维数组通过嵌套构造。`getSliceType()`、`getPointerType()`、`getReferenceType()` 按目标类型与 `AccessKind::ReadOnly/ReadWrite` 复用类型。访问权限表示能否通过该值修改目标，与变量绑定本身的可变性分开；引用类型也与表达式的值/位置类别分开。这些类型构造器本身属于 builtin，即使目标是用户类。
- `getFunctionType(ReturnType, ParameterTypes)` 按返回类型和有序形参类型列表复用固定参数签名，支持零参数与 void 返回值；`FunctionType` 拥有列表存储，所有组成类型必须属于当前上下文，空形参指针被拒绝。参数名称、默认值、参数包和泛型绑定由后续分析负责。
- `Decl` 只保存名称和借用的只读 AST，不保存上下文、种类、重复的源码信息、已解析类型或初始化值。当前只有泛型函数和泛型类创建语义声明：`createFunctionDecl(Name, AST)` 与 `createClassDecl(Name, AST)` 要求 AST 含有泛型参数，普通定义返回空指针；派生类不增加字段，`ast()` 提供对应的具体 AST，`classof()` 从 AST 分类。语义层暂不提供 `VarDecl`。泛型定义、实例结果和执行时状态分别保存，AST 所属 `ParsedUnit` 必须比语义上下文活得更久。
- `createFunction(Name, Signature)` 创建具有独立身份和本地 `FunctionType` 的 `Function` 值，供普通函数或已闭合的泛型实例使用。`createCallInstruction(Callee, Arguments)` 直接调用 `Function`，或间接调用其他类型为 `FunctionType` 的值；泛型 `FunctionDecl` 需要先实例化，不能直接作为调用目标。实参须已完成排序和转换，数量和逐项类型必须精确匹配。指令拥有实参引用列表，每次调用均创建独立对象；创建不执行函数，不进行重载解析或泛型实例化。
- `createClassType(Name)`、`createEnumType(Name)`、`createInterfaceType(Name)` 直接创建具有独立身份的名义类型，不创建语义 `Decl`。同名类型不合并，分析器负责复用同一普通类型或泛型实例的返回对象；成员、继承、枚举底层类型、布局和实例缓存仍待实现。组合类型工厂仅检查结构约束；元素合法性、数组布局大小和引用使用规则尚未由分析器检查。
- `createVariable(Name, Mutability, Initializer)` 创建独立于 `Decl` 的普通变量绑定；`BindingMutability::Mutable/Immutable` 分别对应 `var/const`。可空的初始化值可以是本上下文的类型、常量、函数或表达式结果。`setType()` 和 `setInitializer()` 各发布一次，允许重复发布同一对象，拒绝冲突及外来对象；两者均存在时类型必须一致。绑定不保存执行时的变量内容，AST 绑定关系由后续语义旁表负责。
- 空名称或名称索引空间耗尽返回无效 `Name`；无效或越界索引查询返回空视图。整数工厂拒绝零位宽，浮点工厂拒绝 16/32/64 以外的位宽；组合类型工厂拒绝外来类型和无效访问权限。命名对象工厂拒绝无效或越界名称，变量绑定工厂另检查可变性和初始化值，泛型声明工厂另检查 AST 的泛型参数。上述检查返回显式状态，不依赖异常。
