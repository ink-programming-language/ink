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

启用 `BUILD_TESTING` 时，配置阶段检查 Frontend/Core 的 CMake 目标依赖，CTest 的 `FrontendDependencyBoundaryTest` 检查对应源码和头文件，禁止引入 IR 或 Semantic。源码检查使用轻量文本匹配，覆盖 include 和命名空间引用；可单独运行 `ctest --test-dir build -C Release -R FrontendDependencyBoundaryTest --output-on-failure`。

`Analyzer::analyze(SemanticContext &, const frontend::parser::ParseResult &)` 返回 `bool`，校验解析结果及源码归属后，按源码顺序遍历模块和块，通过 `analyzeStmt`、`analyzeDeclStmt` 和 `analyzeDecl` 到达具体节点处理函数。每次调用独立维护块深度，并读取 `INK_SEMANTIC_BLOCK_DEPTH_LIMIT`。空模块和仅含空块的输入可以完成遍历；具体语义及 comptime 求值尚未实现，命中时报告既有的 `SemanticUnsupported` ICE。当前入口不生成 IR，不提供原执行分支的模块返回值或多模块分析能力。

LLVM/Clang 固定版本为 22.1.8；tokenizer 直接使用 Clang 的 `UnicodeCharSets.h`，parser 的 AST 归档使用 LLVM 支持库。NFC 校验使用 utf8proc 2.9.0 的 Unicode 15.1 实现。spdlog 1.17.0 负责所有自有 C++ 文本输出；CLI 统一通过 `ink::cli::writeOutput` 输出。所有自有 C++ 编译目标在目标级关闭异常。

Core 的公共头文件按职责放在 `src/include/ink/core` 下的 `support`、`source`、`name`、`diagnostic`、`config` 和 `archive` 子目录，实现和测试在各自的 Core 目录中按相同职责组织；构建仍使用单个 `ink::core` 库，公共命名空间仍为 `ink::core`。`diagnostic/diagnostic.h` 定义诊断数据和元数据接口，参数 schema 与 traits、构造器、消费接口、引擎和格式化器分别位于 `diagnostic_traits.h`、`diagnostic_builder.h`、`diagnostic_consumer.h`、`diagnostic_engine.h` 和 `diagnostic_formatter.h`，调用方按需包含。AST 文本归档辅助头仅供 parser 内部使用，位于 `src/lib/frontend/parser/archive_text.h`。

资源限制在 `src/include/ink/core/config/config.def` 中定义，parser 与 AST 归档可通过对应的 `INK_PARSER_*`、`INK_AST_ARCHIVE_*` 环境变量配置；semantic 配置与诊断定义保留供后续实现使用。内部编译错误经 spdlog 输出后 panic，普通源码错误通过诊断引擎报告。

Frontend 通过统一的 `ink::frontend` 静态库提供 tokenizer、parser/AST 和共享上下文。公共头文件位于 `src/include/ink/frontend`，其中 `tokenizer` 和 `parser` 是两个子目录，命名空间分别为 `ink::frontend::tokenizer` 和 `ink::frontend::parser`；实现位于 `src/lib/frontend` 下对应的子目录。Frontend 的公开编译要求为 C++20；tokenizer 测试单独按 C++17 编译，以保留 UTF-8 字面量行为，最终链接同一个 frontend 库。

AST 类声明位于 `src/include/ink/frontend/parser/ast`，每个 class 使用一个 snake_case 头文件，共 80 个类：64 个具体节点、7 个基类和 9 个辅助记录。`ast/types.h` 集中公共别名、`NameToken`、`RestBinding` 和 7 个枚举，`ast/fwd.h` 提供类前向声明，`ast/cast.h` 提供 `isa`、`dyn_cast` 和 `cast`。`src/include/ink/frontend/parser/ast.h` 保留聚合入口，调用方也可直接包含所需节点头文件。命名空间、公开 API 及 `ASTNodes.def` 的节点 ID 保持不变；`ast_context.h`、`ast_kind.h`、`ast_visitor.h` 和 `ast_walker.h` 仍位于 parser 根目录，现有实现仍位于 `src/lib/frontend/parser`。

`frontend::FrontendContext` 直接借用 `core::SourceManager` 和 `core::DiagnosticEngine`，二者必须比上下文存活更久；tokenizer、parser 和 semantic 使用这些服务而不依赖工具层。`SemanticContext` 同样接收这两个服务。现有 `ink-tokenize`、`ink-parse` 和测试各自拥有服务实例，以保持编译会话之间的源码和诊断隔离。

`inkc::CompilationContext` 是工具私有的编译会话资源所有者，定义位于 `src/tools/inkc/compilation_context.h`，拥有目标描述、源码管理器和诊断引擎。库和测试不依赖此类型；当前分支尚未恢复 inkc 可执行入口。

Target 目录提供目标平台描述，头文件位于 `src/include/ink/target`。`target::TargetContext`、`target::PointerWidth` 和 `target::ByteOrder` 定义在 `ink/target/target_context.h` 中；`TargetContext` 保存指针宽度、字节序及标量对齐信息，`native()` 返回宿主目标。inkc 私有的 `CompilationContext` 和 IR 的 `TypeContext` 均使用这一描述。

Core 的 `Singleton<T>` 提供按需初始化的单例基类；派生类将构造、析构函数设为私有，并声明 `Singleton<T>` 为友元。`NamePool::instance()` 提供进程共享的名称池，名称句柄和字符串视图在池增长时保持有效。`NAME_None` 与默认构造的 `Name{}` 相等，索引均为 `Name::InvalidIndex`，表示无效句柄；空字符串不会注册，查找未命中的名称返回该无效句柄。名称池不预注册字符串 `"None"`，该字符串可作为普通名称注册。名称池操作需要调用方进行并发同步。

IR 基础库通过 `ink::ir` 链接，公共头文件与实现分别位于 `src/include/ink/ir` 和 `src/lib/ir`，按职责拆分为 `type`（类型）、`function`（函数与基本块）、`param`（形参值和调用实参）、`instruction`（指令）和 `constant`（常量）目录。`Value.def` 统一登记具体类型、泛型类型、Function、Parameter、BasicBlock、Instruction 和六种具体常量，以及适用时可确定的固定字节大小；Void 固定为 0 字节，Bool 与 BoolConstant 固定为 1 字节，其他种类使用 `std::nullopt` 表示大小取决于具体类型信息或布局不适用。`Value` 是不可复制的通用 IR 抽象基类，提供 `kind()` 和虚析构，不要求存储布局。`Type` 继承 `Value`，独占以字节为单位的纯虚接口 `size()`、`alignment()`；`hasLayout()` 区分具有运行时布局的类型与尚未实例化的模板。`Type::classof()` 识别所有具体及泛型类型，并拒绝函数实体、参数值、基本块、指令、常量、空指针与未知种类。

`TypeContext` 是当前可创建 IR 类型的统一获取入口，声明位于 `src/include/ink/ir/type/type_context.h`。`TypeContext Types(Target)` 保存完整且固定的目标快照，拥有所有返回的类型对象以及唯一的 `ConstantPool`；两者共同创建和销毁，类型与常量具有统一生命周期。上下文不可复制或移动，返回的指针均为借用，外部 IR 对象必须先于上下文销毁。基本类型和结构类型通过 `const` 指针使用；名义类型返回可变指针，用于补充声明信息，调用 `freeze()` 后禁止修改元数据，所有类型的布局始终不可变。具体类型的构造函数均为私有，仅向 `TypeContext` 开放友元访问，原有类型静态 `create()` 与 `BoolType::get()` 已移除。基本类型在上下文初始化时创建，`getVoidType()`、`getNullPointerType()`、`getBoolType()`、`getUInt8Type()`、`getInt32Type()`、`getUInt32Type()`、`getInt64Type()`、`getUInt64Type()`、`getFloat32Type()` 和 `getFloat64Type()` 始终返回各自固定的对象；`getIntegerType(Width, Signed)` 支持无符号 8 位以及有符号、无符号 32 和 64 位整数，`getFloatType(Width)` 支持 32 和 64 位浮点数，不支持的位宽或符号组合返回 `nullptr`。同一次编译的多个模块可共享一个上下文；不同上下文的类型身份独立，不得混用，相关操作需要调用方进行并发同步。

`getPointerType(Type)`、`getReferenceType(Type)`、`getArrayType(ElementType, Count)` 和 `getFunctionType(ReturnType, Parameters)` 按完整描述复用结构类型。函数类型的缓存键由返回类型与有序参数类型列表组成，参数名称保存在函数实体的形参中。空类型、来自其他上下文的类型及无效布局返回 `nullptr`。类、结构体和接口通过对应的 `create*Type()` 创建独立名义类型；三种泛型声明暂不提供创建入口。`owns(Type)` 查询类型归属；`src/testcase/ir/type_context_test.cpp` 覆盖类型复用、地址稳定性、目标隔离及跨上下文拒绝。

`void` 与 `i32`、`bool` 一样按普通标识符识别，Parser 使用 `NameExpr` 表达，内建类型含义由后续名称解析确定。`VoidType` 是具有唯一值的单位类型，拥有有效的零字节布局，`size()` 为 0、`alignment()` 为 1，可用于返回类型、参数、数组元素、引用目标和后续泛型实例。`Types.getPointerType(Types.getVoidType())` 对应源码 `*void`，其 `PointerType::isOpaque()` 返回 `true`；`pointeeType()` 返回 void 标记，不表示未知对象具有零字节布局。其他指针的 `isOpaque()` 返回 `false`，包括指向 `*void` 值的 `**void`，指针自身仍采用目标指针布局。语言约定禁止直接解引用、索引或按元素移动 `*void`，恢复为 `*T` 需要显式转换并满足对齐、对象生命周期等条件。当前实现提供 IR 类型描述；单位值构造、内建名称解析、指针操作及转换检查和 C ABI 映射尚待语义及后端实现，空元组 `()` 尚未定义为 void 的值或别名。

`BasicBlock` 直接继承 `Value`，声明位于 `src/include/ink/ir/function/basic_block.h`，实现位于 `src/lib/ir/function/basic_block.cpp`，表示 IR 控制流基本块的独立身份。构造函数默认使用 `core::NAME_None`，通过 `name()`、`hasName()` 和 `setName()` 查询或设置可选名称，并通过 `classof()` 识别基本块。`function()` 返回所属函数；独立构造的基本块没有父函数，返回 `nullptr`。基本块通过 `std::vector<std::unique_ptr<Instruction>>` 拥有有序指令，`appendInstruction()` 接收并转移指令所有权，空指针返回 `nullptr` 且不改变基本块；`instructions()`、`instructionCount()` 和 `instruction(Index)` 提供查询，索引越界返回 `nullptr`。它不提供 `size()`、`alignment()` 或 `hasLayout()`。终结指令和控制流边尚未实现；源码的词法块仍由 parser 的 AST 表达。

`Instruction` 是直接继承 `Value` 的抽象指令基类，声明位于 `src/include/ink/ir/instruction/instruction.h`，实现位于 `src/lib/ir/instruction/instruction.cpp`。受保护的构造函数将种类固定为 `ValueKind::Instruction`，派生指令共享该分类，通过 `classof()` 从 `Value` 识别；纯虚析构函数提供类外定义，支持通过 `Value` 或 `Instruction` 安全销毁派生对象。操作码、操作数和结果类型由后续指令模型补充。`BasicBlock` 与 `Instruction` 均直接继承 `Value`，二者均无存储布局接口。

`Constant` 是直接继承 `Value` 的抽象常量基类，通过 `type()` 返回借用的非空类型指针；大小和对齐仍从该类型查询。`BoolConstant`、`IntegerConstant`、`FloatConstant`、`ArrayConstant`、`NullPointerConstant` 和 `StringConstant` 均直接继承 `Constant`，每类各有独立的头文件和实现文件，分别位于 `src/include/ink/ir/constant` 和 `src/lib/ir/constant`。基类和派生类均提供 `classof()`。`Constant::canInitialize(DestinationType)` 统一判定初值兼容性：相同类型身份直接接受，当前常量池的唯一空指针常量还可用于同一类型上下文的任意 `PointerType`；不扩展整数、布尔、引用或其他类型的隐式转换，也不允许跨上下文转换。

`auto &Constants = Types.constantPool()` 是常量的统一获取入口，声明位于 `src/include/ink/ir/constant/constant_pool.h`。每个 `TypeContext` 内置唯一的 `ConstantPool`，不能独立构造、销毁、复制或移动；同一类型上下文总是返回同一常量池，不同类型上下文的常量互相隔离。它按类型及精确内容复用不可变常量，返回生命周期受所属 `TypeContext` 约束的借用 `const` 指针；外部 IR 对象必须先于该上下文销毁。`constantPool()` 提供可变与只读引用重载，操作需要调用方进行并发同步。六种具体常量的构造函数均为私有，仅向 `ConstantPool` 开放友元访问，并由上下文显式传入对应类型；原有静态创建接口已移除。`typeContext()` 返回关联的类型上下文，`owns(Constant)` 查询常量归属；接收显式类型的入口遇到空类型或外部类型时返回 `nullptr`，各入口遇到非法内容时返回 `nullptr`。

`Constants.getBoolConstant(Value)` 获取布尔常量，自动使用关联上下文的布尔类型，`true` 和 `false` 各复用一个对象，通过 `value()` 查询值。布尔常量数组使用同一类型上下文的 `Types.getArrayType(Types.getBoolType(), Count)` 获取元素类型匹配的数组类型。

`Constants.getIntegerConstantFromBits(Type, Bits)` 和 `Constants.getFloatConstantFromBits(Type, Bits)` 获取保存精确位模式的常量，通过 `bits()` 查询；8 位和 32 位类型拒绝对应位宽以外的非零高位，64 位类型保留全部位。位模式按数值位次描述，与宿主及目标字节序无关。整数另外提供 `getSignedIntegerConstant(Type, Value)` 和 `getUnsignedIntegerConstant(Type, Value)`，校验类型符号属性和数值范围，有符号负数以二进制补码表示；无符号 8 位整数接受 0 至 255。整数数值及整数、浮点位模式入口保留类型参数，由调用方选择目标位宽。浮点提供 `getFloatConstant(float Value)` 和 `getFloatConstant(double Value)` 数值入口，分别自动使用关联上下文的 32 位和 64 位 `FloatType`；例如 `Constants.getFloatConstant(4.13f)` 和 `Constants.getFloatConstant(4.13)`。数值入口在经编译期检查的 IEEE 754 binary32/binary64 宿主上复制位模式，不做跨精度转换。数值与位模式入口返回同一类型、同一位模式的同一对象；正负零和不同 NaN 载荷分别保留。当前未实现常量求值。

`Constants.getArrayConstant(Type, Elements)` 获取按类型和有序元素列表复用的数组常量；元素数量须与 `ArrayType::elementCount()` 一致，每项须由当前常量池拥有，且通过 `canInitialize()` 与数组元素类型的兼容性校验；指针元素可共同引用池内唯一空指针常量。数组类型仍提供具体元素类型，数组整体及嵌套数组保持类型身份匹配，拒绝其他常量池的元素。数组常量拥有元素指针列表，允许空数组、重复元素和嵌套数组，通过 `elements()` 和 `elementCount()` 查询。`Constants.getNullPointerConstant()` 无参返回池内唯一的 `NullPointerConstant`，支持只读访问且不分配内存，不再按具体 `PointerType` 建立缓存。它的类型固定为 `Types.getNullPointerType()`，可在同一上下文的不同指针类型使用位置共享，由目的字段、变量或数组提供具体指针类型；常量本身的类型保持不变。`NullPointerType` 直接继承 `Type`，分类为 `ValueKind::NullPointer`，名称为 `null_pointer`，不包含 pointee，使用目标指针大小和对齐；`typeContext()` 返回所属上下文。空指针常量不存储宿主地址，也不假定目标空指针的内存表示为全零。本次不提供 Class、Struct 或 Interface 常量。

IR 不提供独立的字符串类型，文本常量的存储使用无符号 8 位整数数组。`StringConstant` 保留为 UTF-8 文本常量的便捷表示，其 `type()` 返回 `Types.getArrayType(Types.getUInt8Type(), ByteLength)`，数组大小等于实际内容字节数，对齐为 1；32 位和 64 位目标采用相同的字节数组布局。运行时字符串视图所需的数据指针、长度和所有权规则由后续语义及运行时设计确定，可通过普通聚合类型和指针表达；当前尚未实现视图构造及其降低。

`Constants.getStringConstant(Value)` 接收已经解码的 UTF-8 `std::string`，自动使用关联上下文的无符号 8 位数组类型，按完整字节内容复用常量；字节数相同的文本共享数组类型，内容不同的文本仍是不同常量。常量拥有文本内容，`value()` 返回生命周期受常量约束的 `std::string_view`，上下文增长不会使视图失效；`byteLength()` 返回实际内容字节数。空字符串使用零元素数组，嵌入 NUL、作为内容的 U+FEFF 均合法，不增加终止符，不重复处理转义或执行 Unicode 规范化。非法 UTF-8、字节数超过目标指针宽度可表示的无符号上限或无法表示对应数组类型时返回 `nullptr`。校验直接复用仓库固定的 utf8proc 2.9.0，IR 库通过 CMake 私有链接该依赖；逐个 Unicode 标量校验并显式传入可用字节数，不按 NUL 截断。文本常量可初始化同一上下文中类型匹配的字节数组，不会自动转换为指针；普通 `ArrayConstant` 仍可表示任意字节，不要求其内容为合法 UTF-8；两种常量使用独立种类和缓存，即使字节内容相同也不承诺跨表示复用对象。静态数据生成及运行时字符串操作留待后续阶段。`src/testcase/ir/constant_pool_test.cpp` 覆盖常量复用、地址与视图稳定性、构造权限及跨上下文拒绝。

`BoolType`、`IntegerType`、`FloatType` 和 `ArrayType` 均直接继承 `Type`，各有独立的头文件和实现文件。Bool 的大小和对齐固定为 1 字节；Integer 保存 8、32 或 64 位位宽、符号属性和目标对齐，其中 8 位仅支持无符号类型，大小和对齐均为 1 字节；Float 保存 32 或 64 位位宽和目标对齐，64 位浮点也使用 `FloatType`。Array 保存元素类型、元素数量、大小和对齐，创建时处理元素步长填充并检查溢出。`ClassType`、`InterfaceType` 和 `StructType` 通过公共抽象基类 `NominalType` 继承 `Type`，统一保存名称、字段列表、方法列表和固定布局，不再存在独立的 Class、Interface、Struct 或 TypeDeclaration 包装。`TypeContext::createClassType(Size, Alignment, Name = core::NAME_None)`、`createInterfaceType(Size, Alignment, Name = core::NAME_None)` 和 `createStructType(Size, Alignment, Name = core::NAME_None)` 根据已确定的布局创建名义类型；对齐必须为非零二次幂，否则返回 `nullptr`。每次成功调用产生独立身份，即使名称或布局相同也不复用；生产者负责让同一个声明的引用复用同一类型对象。类型可用于同一上下文的指针、引用、数组和函数签名。`TypeContext` 现有接口遇到不支持的位宽、非法布局或溢出时返回 `nullptr`。`PointerType` 和 `ReferenceType` 分别保存指向或引用的类型，并使用目标指针宽度确定布局；所有关联的类型均属于同一上下文。`src/testcase/ir/type_test.cpp`、`nominal_type_test.cpp` 和 `nominal_metadata_test.cpp` 覆盖类型属性、布局、无效输入、独立身份、成员信息与类型识别。

`BoolType`、`FloatType`、`IntegerType`、`NullPointerType`、`VoidType` 和 `VariadicType` 的头文件与实现归入 `type/builtin_type`；`ArrayType`、`FunctionType`、`PointerType` 和 `ReferenceType` 归入 `type/structure_type`；`GenericType` 及其派生类型归入 `type/generic_type`；名义类型及其成员元数据归入 `type/nominal_type`。`Type`、`TypeContext`、`Value` 和公共定义保留在 `type` 根目录。

`FunctionType` 同样直接继承 `Type`，声明和实现分别位于 `function_type.h` 和 `function_type.cpp`。`Types.getFunctionType(ReturnType, Parameters)` 接收 `std::vector<const Type *>`，参数列表默认为空；`returnType()`、`parameters()` 和 `parameterCount()` 分别提供返回类型、有序参数类型列表和参数数量。它拥有指针列表，借用返回类型和参数类型，获取时遇到空类型指针、不属于本上下文或不具有运行时布局的返回/参数类型返回 `nullptr`。名称由具体函数的形参保存。当前函数值使用无捕获函数地址表示，大小和对齐均为目标指针字节宽度。`src/testcase/ir/function_type_test.cpp` 验证参数类型、签名、列表所有权、空类型校验及目标布局。

`Function` 是直接继承 `Value` 的函数实体，声明位于 `src/include/ink/ir/function/function.h`，实现位于 `src/lib/ir/function/function.cpp`。函数签名与函数实体分别使用 `ValueKind::FunctionType` 和 `ValueKind::Function` 分类。`Function(Signature, Name = core::NAME_None, SymbolLink = SymbolLinkage::Local, LanguageLink = LanguageLinkage::Ink, Visibility = FunctionVisibility::Public)` 借用非空 `FunctionType`，通过 `functionType()` 查询；签名及其类型上下文必须比函数存活更久，多个函数可以共享同一个签名。函数不可复制或移动，以保持参数值与基本块的父函数地址稳定。构造时按签名顺序自动创建并拥有 `Parameter`，通过 `parameters()`、`parameterCount()` 和 `parameter(Index)` 查询。`createBlock(Name = core::NAME_None)` 创建并拥有基本块，同时设置其父函数；`blocks()`、`blockCount()`、`block(Index)` 和 `entryBlock()` 提供查询，索引越界或没有入口块时返回 `nullptr`，基本块地址在函数存活期间保持稳定。`isDeclaration()` 仅表示当前没有基本块，也涵盖函数定义尚未构建 IR 函数体的阶段，不能据此推断导入状态。

`function.h` 直接定义三个互相独立的枚举：`SymbolLinkage` 的 `Local`、`Import`、`Export` 表达相对于当前模块的符号方向；`LanguageLinkage` 的 `Ink`、`C` 表达语言链接约定；`FunctionVisibility` 的 `Public`、`Private` 表达解析后的 Ink 访问可见性。对应接口为 `symbolLinkage()`、`languageLinkage()`、`visibility()` 及各自的 setter；修改任何一项不会隐式改变其他属性或函数体。IR 默认可见性为 `Public`，与源码省略修饰时的语义一致；AST 的 `DeclarationVisibility::Default` 仍保留源码未显式修饰的事实。`Import + Ink` 的函数声明表示实现位于其他 Ink 模块，`Import + C` 表示外部 C 函数，`Export + C + Private` 可以同时表达原生符号导出和 Ink 访问限制。Parser 使用自身的 `NativeSymbolKind::None/Import/Export` 保存显式原生 import/export，`None` 仅表示源码没有原生说明符，不决定最终 IR 符号方向。Frontend 不依赖 IR 或 Semantic；源码 ABI 字符串的识别、模块导入解析和到 IR 属性的显式映射仍由后续语义分析实现。

`Module` 表示一个 Ink 模块，声明和实现分别位于 `src/include/ink/ir/module/module.h` 与 `src/lib/ir/module/module.cpp`。构造时接收可选的规范模块短名（不包含包路径），通过 `name()` 和 `hasName()` 查询。模块不可复制或移动，拥有 `GlobalVariable` 和模块级 `Function` 的有序列表，并借用 `ClassType`、`InterfaceType` 和 `StructType` 的有序列表；`globalVariables()`、`globalFunctions()`、`classes()`、`interfaces()` 和 `structs()` 返回这些列表。全局变量通过 `createGlobalVariable()` 创建或 `appendGlobalVariable()` 转移所有权；模块级函数对应使用 `createGlobalFunction()` 和 `appendGlobalFunction()`，空指针返回 `nullptr` 且不改变模块；名义类型由 TypeContext 创建，通过 `addClass()`、`addInterface()` 和 `addStruct()` 登记，重复登记同一指针返回 `false`。各列表提供数量查询；`globalVariable(Index)`、`globalFunction(Index)`、`classType(Index)`、`interfaceType(Index)` 和 `structType(Index)` 在越界时返回 `nullptr`。`owns()` 仅检查模块直接拥有的全局变量和函数；`contains()` 另外包含已登记的名义类型，两者均不递归包含类型的方法函数等嵌套值。实体地址在列表增长后保持稳定，借用的类型与常量必须比模块存活更久；Module 销毁不会销毁名义类型及其拥有的方法、函数和函数体。

`PackagePath` 是普通值结构体，声明位于 `src/include/ink/ir/package/package_path.h`，相等比较实现位于 `src/lib/ir/package/package_path.cpp`。唯一字段 `std::vector<core::Name> Components` 按从根到叶的顺序保存绝对包路径，各段名称由调用方预先校验和规范化；空列表表示根包，不表示路径解析失败。路径不包含模块名，例如 `Components = [game, render]` 与模块短名 `material` 组成完整模块标识 `game.render.material`。相对导入应先由语义阶段解析，不在 IR 中保存相对层级。PackagePath 支持默认复制和移动，`operator==` 比较段数及各段名称和顺序，`operator!=` 为其否定；非空路径去掉最后一段可得到父包路径，追加一段可得到子包路径，无需保存父子包对象。`Module(Name, Package = {})` 按值拥有 PackagePath，默认属于根包；`package()` 返回只读引用，`setPackage()` 按值替换。修改或销毁传入的原路径不会影响模块中的副本，修改包路径也不会改变模块短名。完整模块标识由包路径和短名按需组合，不额外保存另一份完整名称。

`Module` 通过可选的 `InitializationFunction` 和 `DeinitializationFunction` 指针记录初始化与清理入口，默认均为 `nullptr`。`initializationFunction()` 和 `deinitializationFunction()` 分别提供可变及只读查询；`setInitializationFunction()` 和 `setDeinitializationFunction()` 接受本模块 `GlobalFunctions` 中签名为 `() -> VoidType` 的函数，成功返回 `true`，传入 `nullptr` 清除入口，校验失败返回 `false` 并保留原入口。设置入口不转移所有权、不新增全局函数，也不要求函数体已经创建。IR 只描述这两个入口，创建或销毁 Module 不执行它们；每个模块实例的调用次数、模块与变量初始化的依赖顺序、初始化完成后的逆序清理，以及初始化失败时仅清理已完成部分，由后续运行时管理，当前尚未实现。

`NominalType` 位于 `ir/type/nominal_type/nominal_type.h`，使用现有 `ValueKind::Class`、`Interface` 和 `Struct` 分类。它按值拥有 `std::vector<Method>`，每个 Method 再独占自己的 Function。`name()`、`hasName()`、`setName()` 管理类型名称。`addMethod(Method &&)` 在校验成功后转移方法及函数的所有权，拒绝空方法、无效记录和函数签名属于外部上下文的记录；方法按登记顺序保存，同名同签名的不同函数也不会自动合并，重载冲突检查由后续语义阶段决定。`replaceMethod(Index, Method &&)` 使用相同校验替换已有方法，成功后释放旧函数及函数体；越界、校验失败或冻结后调用均保留调用方的方法和原列表。已有方法需通过 `std::move` 转移。`methods()`、`methodCount()`、`method(Index)` 提供有序查询，`findMethods(Name)` 按函数当前名称返回匹配的 `const Method *` 列表；函数改名会改变查询结果，方法元素的地址可能在追加时失效，但所拥有的 Function、参数和块的地址不受方法移动影响。`freeze()` 幂等地冻结类型名称、字段列表和方法列表，`isFrozen()` 查询状态，冻结后的修改返回 `false` 且保持原内容；它不冻结函数名称和函数体。布局在创建时确定，补充元数据不会改变已有结构类型的身份或缓存布局。TypeContext 析构时先释放全部名义类型的方法函数，再清空全部字段元数据，然后销毁常量池，最后释放类型；方法析构期间仍可访问任意名义类型的字段初始值、常量及其类型。

`Field` 定义于 `ir/type/nominal_type/field.h`，与 `NominalType`、`ClassType`、`InterfaceType`、`StructType` 和 `Method` 一同归入 `type/nominal_type`，实现文件采用对应目录。`Field(Type, Name = core::NAME_None, Offset = 0, Initializer = nullptr)` 按值保存成员名称、借用的类型指针、相对所属值起点的字节偏移，以及可选的 `const Constant *` 初始值，不继承 Type 或 Value，不拥有运行时存储。`type()`、`name()`、`hasName()`、`offset()`、`initializer()` 和 `hasInitializer()` 查询字段信息；`nullptr` 表示未记录常量初值，不等同于零初始化，显式零值仍通过 Constant 表示。`valid()` 检查名称有效、类型具有运行时布局，且非空初始值通过 `canInitialize(FieldType)` 校验；本地空指针常量可用于任何具体指针字段。`NominalType` 按值拥有字段列表，提供 `fields()`、`fieldCount()`、`field(Index)`、`findFields(Name)`、`addField(Field)` 和 `replaceField(Index, Field)`；字段保持登记顺序，同名记录不会自动合并。添加和替换拒绝无效字段、外部上下文类型、不由所属常量池拥有的初始值、超出固定大小的存储范围以及冻结后的修改，失败不改变原列表；边界校验通过减法避免偏移与大小相加溢出，零大小字段允许位于结尾。偏移不自动计算，也不改变类型已有布局；重叠、打包对齐及名称冲突由后续阶段处理。字段元素的指针可能在追加时失效。

`Method` 独立定义于 `ir/type/nominal_type/method.h`，实现位于 `src/lib/ir/type/nominal_type/method.cpp`，不继承 Type 或 Value。`Method(std::unique_ptr<Function> Callable, VirtualInfo = {})` 接收函数的独占所有权并保存虚方法信息，不再独立保存名称或 `FunctionType`。`function()` 返回借用的函数指针，`name()` 和 `functionType()` 实时从该函数读取，参数与函数体通过 `function()` 访问。Method 不可复制、可无异常移动；析构或被移动赋值覆盖时释放原函数及其拥有的参数和函数体。传入空指针或移出所有权后的 Method 无效，`function()`、`functionType()` 返回 `nullptr`，`name()` 返回 `NAME_None`。`MethodVirtualInfo` 保存 `MethodDispatch::Direct`、`Virtual` 或 `Abstract`、`Override`、`Final` 和可选的零起始虚表槽位 `Slot`；槽位是表项索引而非字节偏移，未设置表示尚未分配。`isVirtual()` 对 Virtual 和 Abstract 均返回 true，另外提供 `isAbstract()`、`isOverride()`、`isFinal()`、`virtualSlot()` 等查询。`valid()` 校验函数存在、当前名称和分派组合：普通方法不能携带 override、final 或槽位，抽象方法不能为 final。所有权链为 TypeContext → ClassType/InterfaceType/StructType → Method → Function，函数签名仍由 TypeContext 拥有。Module 不维护独立导出表；符号身份、字段布局计算、继承关系、虚表槽位分配与生成、虚调用降低、override 层级校验、跨模块加载及序列化由后续阶段补充。

`GlobalVariable` 位于同一 `ir/module` 目录，直接继承 `Value`，分类为 `ValueKind::GlobalVariable`。`GlobalVariable(VariableType, Name = core::NAME_None)` 借用非空类型，通过 `type()` 查询；名称接口与函数一致。可选的常量初值通过 `initializer()`、`hasInitializer()` 和 `setInitializer()` 管理：传入 `nullptr` 清除初值，不满足 `canInitialize(VariableType)` 时返回 `false` 并保留原初值；本地空指针常量可用于具体指针变量，传入该常量与传入 `nullptr` 清除初值是两种不同操作。初值为借用，不改变常量的所有权；没有初值仅表示尚未设置常量初值，不推断导入状态或运行时初始化方式。

`Parameter` 是直接继承 `Value` 的函数形参值，声明位于 `src/include/ink/ir/param/parameter.h`，实现位于 `src/lib/ir/param/parameter.cpp`，分类为 `ValueKind::Parameter`。它只能由 `Function` 创建并拥有，通过 `function()` 和 `index()` 查询所属函数及参数索引；`type()` 按该索引读取函数签名中的类型指针。每个形参独立保存 `core::Name Name`，默认为 `core::NAME_None`，通过 `name()`、`hasName()` 和 `setName()` 查询或设置。多个函数共享签名时各自拥有独立的形参身份和名称，修改名称不会改变函数类型；函数体指令可以引用这些 `Value`。

`Argument` 表示调用点的实参，声明位于 `src/include/ink/ir/param/argument.h`，实现位于 `src/lib/ir/param/argument.cpp`。`Argument(const Value &ArgumentValue, core::Name Name = core::NAME_None)` 保存借用的非空 `const Value *` 和独立的可选实参名称；`value()` 查询被引用值，`name()`、`hasName()` 和 `setName()` 查询或设置名称。实参可以引用常量、指令结果或调用者自身的形参，其名称独立于被引用值的名称。实参包装可复制，不继承 `Value`，也不拥有被引用值；被引用值必须比使用它的实参存活更久。实参不保存形参索引或变参标记，调用中的顺序由实参列表表达。调用指令及实参与形参的匹配校验留待后续实现。

`GenericType` 是继承 `Type` 的抽象模板描述基类，拥有声明名称和 `std::vector<const Type *>` 泛型参数类型列表，提供 `name()`、`genericParameters()` 和 `genericParameterCount()`。`GenericClassType`、`GenericFunctionType` 和 `GenericVarType` 分别描述类模板、函数模板及带泛型参数的变量声明模板，四个类各有独立的头文件和实现文件；具体模板的构造函数保持私有，暂不提供创建接口。泛型函数另外保存普通参数类型列表和返回类型，提供 `parameters()`、`parameterCount()` 和 `returnType()`；泛型变量通过 `valueType()` 暴露所声明的值类型。语言规则与类型解析留给后续语义分析；指针列表由模板拥有，类型对象仍为借用。

模板尚无实例化后的运行时布局，`GenericType::hasLayout()` 固定返回 `false`，`size()` 的 0 和 `alignment()` 的 1 仅为占位值；它们与具有有效零字节布局的空数组不同。`src/testcase/ir/generic_type_test.cpp` 保留泛型类型层次的编译期检查及空指针分类检查，构造权限检查保留在 `src/testcase/ir/type_test.cpp`；实例相关测试随创建入口暂时移除。目前未实现模板实例化，也未扩展 parser 的泛型变量语法。
