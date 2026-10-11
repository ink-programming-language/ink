# ink

`main` 保留完整的 tokenizer、parser、AST、对应测试，以及前端所需的 Core、CLI 和构建依赖，并开始重建 IR 基础类型。Semantic 保留原有头文件、源文件及目录结构、公开分析入口和 analyzer 基于 `ASTNodes.def` 的语句及声明分派；具体语义处理仍报告未实现。`src/testcase/semantic` 中的 17 个原有测试文件完整保留，暂不参与编译和运行；新增的入口与分派回归测试已启用。

精简前的完整主干保存在 [`saved-execution`](https://github.com/ink-programming-language/ink/tree/saved-execution) 分支，快照提交为 `22c3946c4737a7942fc28ab7cfa6f78e6fa3bd31`。原有 semantic 实现、IR、execution、ABI、LLVM backend、inkc 及其测试和文档可在该分支查看。

## 前端规范

- [词法规则](docs/Ink-Lexical-Rules.md)
- [语法规则](docs/Ink-grammar-Rules.bnf)
- [Parser 设计](docs/Ink-Parser-Design.md)
- [AST 序列化](docs/Ink-AST-Serialization.md)
- [命令行接口](docs/command-line.md)
- [Parser 测试与覆盖率](src/testcase/parser/README.md)

## 编辑器支持

[`editors`](editors/README.md) 提供 CLion / IntelliJ IDEA 可导入的 TextMate 语法高亮和 Ink Live Templates。安装步骤见该目录的说明，高亮样例见 [`example.ink`](editors/textmate/example.ink)。

## 构建与测试

初始化固定版本的第三方依赖：

```powershell
git submodule update --init --recursive
```

使用 Visual Studio 2022 构建并运行全部启用的测试：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

使用其他生成器时可按工具链调整生成参数；单配置生成器使用 `-DCMAKE_BUILD_TYPE=Release`，运行 CTest 时可省略 `-C Release`。

`ink_tests` 包含 Core、IR 类型、tokenizer、parser/AST 及 semantic 分析入口的单元测试，构建时同时构建 `ink-tokenize` 和 `ink-parse`。CTest 还运行 tokenizer、parser 的 CLI 进程测试和 parser 对抗输入测试。单独构建并运行全部启用测试可使用 `cmake --build build --config Release --target run_all_tests`。

`Analyzer::analyze(SemanticContext &, const parser::ParseResult &)` 返回 `bool`，校验解析结果及源码归属后，按源码顺序遍历模块和块，通过 `analyzeStmt`、`analyzeDeclStmt` 和 `analyzeDecl` 到达具体节点处理函数。每次调用独立维护块深度，并读取 `INK_SEMANTIC_BLOCK_DEPTH_LIMIT`。空模块和仅含空块的输入可以完成遍历；具体语义及 comptime 求值尚未实现，命中时报告既有的 `SemanticUnsupported` ICE。当前入口不生成 IR，不提供原执行分支的模块返回值或多模块分析能力。

LLVM/Clang 固定版本为 22.1.8；tokenizer 直接使用 Clang 的 `UnicodeCharSets.h`，parser 的 AST 归档使用 LLVM 支持库。NFC 校验使用 utf8proc 2.9.0 的 Unicode 15.1 实现。spdlog 1.17.0 负责所有自有 C++ 文本输出；CLI 统一通过 `ink::cli::writeOutput` 输出。所有自有 C++ 编译目标在目标级关闭异常。

Core 提供源码位置、范围、源码管理、诊断和配置。资源限制在 `src/include/ink/core/config.def` 中定义，parser 与 AST 归档可通过对应的 `INK_PARSER_*`、`INK_AST_ARCHIVE_*` 环境变量配置；semantic 配置与诊断定义保留供后续实现使用。内部编译错误经 spdlog 输出后 panic，普通源码错误通过诊断引擎报告。

Target 目录提供目标平台描述，头文件位于 `src/include/ink/target`。`target::TargetContext`、`target::PointerWidth` 和 `target::ByteOrder` 定义在 `ink/target/target_context.h` 中；`TargetContext` 保存指针宽度、字节序及标量对齐信息，`native()` 返回宿主目标。Core 的 `CompilationContext` 和 IR 的 `TypeContext` 均使用这一描述。

Core 的 `Singleton<T>` 提供按需初始化的单例基类；派生类将构造、析构函数设为私有，并声明 `Singleton<T>` 为友元。`NamePool::instance()` 提供进程共享的名称池，名称句柄和字符串视图在池增长时保持有效。`NAME_None` 与默认构造的 `Name{}` 相等，索引均为 `Name::InvalidIndex`，表示无效句柄；空字符串不会注册，查找未命中的名称返回该无效句柄。名称池不预注册字符串 `"None"`，该字符串可作为普通名称注册。名称池操作需要调用方进行并发同步。

IR 基础库通过 `ink::ir` 链接，类型头文件位于 `src/include/ink/ir/type`，实现位于 `src/lib/ir/type`；指令与基本块的声明和实现分别位于 `src/include/ink/ir/instruction` 和 `src/lib/ir/instruction`；常量使用对应的 `constant` 目录。`Value.def` 统一登记具体类型、泛型类型、BasicBlock、Instruction 和六种具体常量，以及适用时可确定的固定字节大小；Bool 与 BoolConstant 固定为 1 字节，其他种类使用 `std::nullopt` 表示大小取决于具体类型信息或布局不适用。`Value` 是不可复制的通用 IR 抽象基类，提供 `kind()` 和虚析构，不要求存储布局。`Type` 继承 `Value`，独占以字节为单位的纯虚接口 `size()`、`alignment()`；`hasLayout()` 区分具有运行时布局的类型与尚未实例化的模板。`Type::classof()` 识别所有具体及泛型类型，并拒绝基本块、指令、常量、空指针与未知种类。

`TypeContext` 是当前可创建 IR 类型的统一获取入口，声明位于 `src/include/ink/ir/type/type_context.h`。`TypeContext Types(Target)` 保存完整且固定的目标快照，拥有所有返回的类型对象；上下文不可复制或移动，调用方使用借用的 `const` 指针，上下文必须比所有引用其类型的 IR 对象存活更久。具体类型的构造函数均为私有，仅向 `TypeContext` 开放友元访问，原有类型静态 `create()` 与 `BoolType::get()` 已移除。基本类型在上下文初始化时创建，`getBoolType()`、`getInt32Type()`、`getUInt32Type()`、`getInt64Type()`、`getUInt64Type()`、`getFloat32Type()`、`getFloat64Type()` 和 `getStringType()` 始终返回各自固定的对象；`getIntegerType(Width, Signed)` 和 `getFloatType(Width)` 提供位宽查询，不支持的位宽返回 `nullptr`。同一次编译的多个模块可共享一个上下文；不同上下文的类型身份独立，不得混用，相关操作需要调用方进行并发同步。

`getPointerType(Type)`、`getReferenceType(Type)`、`getArrayType(ElementType, Count)` 和 `getFunctionType(ReturnType, Parameters)` 按完整描述复用结构类型。函数参数的顺序、类型、名称及变参标记均参与缓存键，以保留现有元数据语义。空类型、来自其他上下文的类型及无效布局返回 `nullptr`。类、结构体、接口及三种泛型声明的类型定义和私有构造函数保留，暂不提供创建入口。`owns(Type)` 查询类型归属；`src/testcase/ir/type_context_test.cpp` 覆盖类型复用、地址稳定性、目标隔离及跨上下文拒绝。

`BasicBlock` 直接继承 `Value`，声明位于 `src/include/ink/ir/instruction/basic_block.h`，实现位于 `src/lib/ir/instruction/basic_block.cpp`，表示 IR 控制流基本块的独立身份。构造函数默认使用 `core::NAME_None`，通过 `name()`、`hasName()` 和 `setName()` 查询或设置可选名称，并通过 `classof()` 识别基本块。它不提供 `size()`、`alignment()` 或 `hasLayout()`。基本块的函数归属、指令容器、终结指令和控制流边尚未实现；源码的词法块仍由 parser 的 AST 表达。

`Instruction` 是直接继承 `Value` 的抽象指令基类，声明位于 `src/include/ink/ir/instruction/instruction.h`，实现位于 `src/lib/ir/instruction/instruction.cpp`。受保护的构造函数将种类固定为 `ValueKind::Instruction`，派生指令共享该分类，通过 `classof()` 从 `Value` 识别；纯虚析构函数提供类外定义，支持通过 `Value` 或 `Instruction` 安全销毁派生对象。操作码、操作数和结果类型由后续指令模型补充。`BasicBlock` 与 `Instruction` 均直接继承 `Value`，二者均无存储布局接口。

`Constant` 是直接继承 `Value` 的抽象常量基类，通过 `type()` 返回借用的非空类型指针；大小和对齐仍从该类型查询。`BoolConstant`、`IntegerConstant`、`FloatConstant`、`ArrayConstant`、`NullPointerConstant` 和 `StringConstant` 均直接继承 `Constant`，每类各有独立的头文件和实现文件，分别位于 `src/include/ink/ir/constant` 和 `src/lib/ir/constant`。基类和派生类均提供 `classof()`。

`ConstantContext Constants(Types)` 是常量的统一获取入口，声明位于 `src/include/ink/ir/constant/constant_context.h`。它借用 `TypeContext`，拥有并按类型及精确内容复用不可变常量，返回生命周期受常量上下文约束的借用 `const` 指针。类型上下文必须比常量上下文存活更久，所有引用常量的 IR 对象必须先于常量上下文销毁。上下文不可复制或移动，不接受临时类型上下文，操作需要调用方进行并发同步。六种具体常量的构造函数均为私有，仅向 `ConstantContext` 开放友元访问，并由上下文显式传入对应类型；原有静态创建接口已移除。`typeContext()` 返回关联的类型上下文，`owns(Constant)` 查询常量归属；接收显式类型的入口遇到空类型或外部类型时返回 `nullptr`，各入口遇到非法内容时返回 `nullptr`。

`Constants.getBoolConstant(Value)` 获取布尔常量，自动使用关联上下文的布尔类型，`true` 和 `false` 各复用一个对象，通过 `value()` 查询值。布尔常量数组使用同一类型上下文的 `Types.getArrayType(Types.getBoolType(), Count)` 获取元素类型匹配的数组类型。

`Constants.getIntegerConstantFromBits(Type, Bits)` 和 `Constants.getFloatConstantFromBits(Type, Bits)` 获取保存精确位模式的常量，通过 `bits()` 查询；32 位类型拒绝非零高 32 位，64 位类型保留全部位。位模式按数值位次描述，与宿主及目标字节序无关。整数另外提供 `getSignedIntegerConstant(Type, Value)` 和 `getUnsignedIntegerConstant(Type, Value)`，校验类型符号属性和数值范围，有符号负数以二进制补码表示。整数数值及整数、浮点位模式入口保留类型参数，由调用方选择目标位宽。浮点提供 `getFloatConstant(float Value)` 和 `getFloatConstant(double Value)` 数值入口，分别自动使用关联上下文的 32 位和 64 位 `FloatType`；例如 `Constants.getFloatConstant(4.13f)` 和 `Constants.getFloatConstant(4.13)`。数值入口在经编译期检查的 IEEE 754 binary32/binary64 宿主上复制位模式，不做跨精度转换。数值与位模式入口返回同一类型、同一位模式的同一对象；正负零和不同 NaN 载荷分别保留。当前未实现常量求值。

`Constants.getArrayConstant(Type, Elements)` 获取按类型和有序元素列表复用的数组常量；元素数量须与 `ArrayType::elementCount()` 一致，每项须由当前常量上下文拥有且 `type()` 必须与数组元素类型为同一个类型对象。即使共享类型上下文，也拒绝引用其他常量上下文的元素。数组常量拥有元素指针列表，允许空数组、重复元素和嵌套数组，通过 `elements()` 和 `elementCount()` 查询。`Constants.getNullPointerConstant(Type)` 按指针类型复用空指针常量，不存储宿主地址，也不假定目标空指针的内存表示为全零。本次不提供 Class、Struct 或 Interface 常量。

`StringType` 直接继承 `Type`，描述合法 UTF-8 的只读字符串视图，声明和实现分别位于 `src/include/ink/ir/type/string_type.h` 和 `src/lib/ir/type/string_type.cpp`。`Types.getStringType()` 返回所属上下文唯一的字符串类型。布局按数据指针、目标平台无符号字节长度的顺序组成：32 位目标大小为 8、对齐为 4，64 位目标大小为 16、对齐为 8。`maximumByteLength()` 返回长度字段可表示的最大值；类型大小与文本长度无关，运行时视图不拥有所引用的字节，底层数据须保持有效。

`Constants.getStringConstant(Value)` 接收已经解码的 UTF-8 `std::string`，自动使用关联上下文唯一的 `StringType`，按完整字节内容复用常量。常量拥有文本内容，`value()` 返回生命周期受常量约束的 `std::string_view`，上下文增长不会使视图失效；`byteLength()` 返回实际内容字节数。空字符串、嵌入 NUL、作为内容的 U+FEFF 均合法，不增加终止符，不重复处理转义或执行 Unicode 规范化。非法 UTF-8 或长度超过关联上下文字符串类型的目标上限时返回 `nullptr`。校验直接复用仓库固定的 utf8proc 2.9.0，IR 库通过 CMake 私有链接该依赖；逐个 Unicode 标量校验并显式传入可用字节数，不按 NUL 截断。字符串类型可用于数组和函数签名；当前只提供类型与常量描述，静态数据生成及运行时字符串操作留待后续阶段。`src/testcase/ir/constant_context_test.cpp` 覆盖常量复用、地址与视图稳定性、构造权限及跨上下文拒绝。

`BoolType`、`IntegerType`、`FloatType`、`ArrayType`、`ClassType`、`InterfaceType` 和 `StructType` 均直接继承 `Type`，各有独立的头文件和实现文件。Bool 的大小和对齐固定为 1 字节；Integer 保存 32 或 64 位位宽、符号属性和目标对齐；Float 保存 32 或 64 位位宽和目标对齐，64 位浮点也使用 `FloatType`。Array 保存元素类型、元素数量、大小和对齐，创建时处理元素步长填充并检查溢出。Class、Interface 和 Struct 保留大小和对齐字段，暂不提供创建接口，也尚不描述字段、方法和继承关系。`TypeContext` 现有接口遇到不支持的位宽、非法布局或溢出时返回 `nullptr`。`PointerType` 和 `ReferenceType` 分别保存指向或引用的类型，并使用目标指针宽度确定布局；所有关联的类型均属于同一上下文。`src/testcase/ir/type_test.cpp` 覆盖当前可获取类型的属性、布局、无效输入和类型识别，并验证具体类型不能从外部构造。

`Parameter` 是供函数签名和后续泛型复用的独立参数描述类，声明位于 `src/include/ink/ir/parameter.h`，实现位于 `src/lib/ir/parameter.cpp`。`Parameter(ParameterType, Name = core::NAME_None, Variadic = false)` 保存借用的类型指针、可选的 Core 名称句柄和变参标记，通过 `type()`、`name()`、`hasName()` 和 `isVariadic()` 查询；`NAME_None` 表示无名参数。描述对象可复制，类型对象必须比使用该描述的对象存活更久。描述对象允许保存空类型指针，由需要完整类型的使用方进行校验。

`FunctionType` 同样直接继承 `Type`，声明和实现分别位于 `function_type.h` 和 `function_type.cpp`。`Types.getFunctionType(ReturnType, Parameters)` 接收 `std::vector<Parameter>`，参数列表默认为空；`returnType()`、`parameters()` 和 `parameterCount()` 分别提供返回类型、有序参数描述列表和参数数量。它拥有参数描述列表，保留各参数的类型、名称和变参标记，借用返回类型和参数类型，获取时遇到空类型指针、不属于本上下文或不具有运行时布局的返回/参数类型返回 `nullptr`。名称和变参位置等语言规则由后续语义分析校验。当前函数值使用无捕获函数地址表示，大小和对齐均为目标指针字节宽度。`src/testcase/ir/parameter_test.cpp` 和 `src/testcase/ir/function_type_test.cpp` 验证参数信息、签名、列表所有权、空类型校验及目标布局。

`GenericType` 是继承 `Type` 的抽象模板描述基类，拥有声明名称和 `std::vector<Parameter>` 泛型参数列表，提供 `name()`、`genericParameters()` 和 `genericParameterCount()`。`GenericClassType`、`GenericFunctionType` 和 `GenericVarType` 分别描述类模板、函数模板及带泛型参数的变量声明模板，四个类各有独立的头文件和实现文件；具体模板的构造函数保持私有，暂不提供创建接口。泛型函数另外保存普通参数列表和返回类型，提供 `parameters()`、`parameterCount()` 和 `returnType()`；泛型变量通过 `valueType()` 暴露所声明的值类型。语言规则与类型解析留给后续语义分析；参数列表由模板拥有，类型指针仍为借用。

模板尚无实例化后的运行时布局，`GenericType::hasLayout()` 固定返回 `false`，`size()` 的 0 和 `alignment()` 的 1 仅为占位值；它们与具有有效零字节布局的空数组不同。`src/testcase/ir/generic_type_test.cpp` 保留泛型类型层次的编译期检查及空指针分类检查，构造权限检查保留在 `src/testcase/ir/type_test.cpp`；实例相关测试随创建入口暂时移除。目前未实现模板实例化，也未扩展 parser 的泛型变量语法。
