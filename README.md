# ink

`main` 保留 Core、目标平台描述、tokenizer、parser、AST、IR 类型、函数与基础指令、VM 对象文件及对应测试。semantic、执行与后端实现已移除；`inkc` 仅检查输入文件的词法和语法，`ink-tokenize` 和 `ink-parse` 分别提供 token 流和 AST 输出。

精简前的完整主干保存在 [`saved-execution`](https://github.com/ink-programming-language/ink/tree/saved-execution) 分支，快照提交为 `22c3946c4737a7942fc28ab7cfa6f78e6fa3bd31`。原有 semantic 实现、IR、execution、ABI、LLVM backend、inkc 及其测试和文档可在该分支查看。

## 前端规范

- [词法规则](docs/Ink-Lexical-Rules.md)
- [语法规则](docs/Ink-grammar-Rules.bnf)
- [Parser 设计](docs/Ink-Parser-Design.md)
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

项目自有 C++ 目标统一要求 C++23，编译器和标准库必须支持显式对象参数（deducing this）和 `std::forward_like`；配置阶段会编译探测这两个特性。语言标准通过目标级 `cxx_std_23` 声明，第三方目标保留各自的标准要求。CMake 会根据工具链选择标准选项；部分 CMake/MSVC 组合将 C++23 映射为 `/std:c++latest`。

`-DINK_BUILD_MODE=debug|development|shipping` 选择 Ink 构建模式，默认 `debug`。CMake 将 `src/include/ink/core/config.in` 配置为构建目录下的 `include/ink/core/config.h`，提供目标平台、指针大小、编译器身份与版本，以及互斥的 `INK_BUILD_DEBUG`、`INK_BUILD_DEVELOPMENT`、`INK_BUILD_SHIPPING` 宏。所有布尔宏均定义为 `0` 或 `1`，使用 `#if` 判断。`ink::config` 将生成头目录传递给各模块；生成文件不写入源码树。

`core_define.h` 的 `INK_ENABLE_ASSERTIONS` 统一控制八个断言宏：`check`、`checkMsgf`、`checkAlways`、`checkAlwaysMsgf`、`ensure`、`ensureMsgf`、`ensureAlways`、`ensureAlwaysMsgf`。check 和 ensure 采用对称的 `xxx`、`xxxMsgf`、`xxxAlways`、`xxxAlwaysMsgf` 命名。Debug 和 Development 开启，Shipping 关闭。失败时先输出并刷新日志，再在附加了调试器时触发断点；继续执行后，check 终止进程，ensure 返回 `false`。普通 ensure 每个调用点仅报告和中断一次，`ensureAlways` 和 `ensureAlwaysMsgf` 每次失败都报告和中断；check 每次失败都会终止，因此它的 Always 版本与对应普通版本行为相同。Shipping 中 check 不求值，ensure 保留条件求值，所有断言消息参数均不求值。`panic` 始终终止。Ink 模式独立于 `CMAKE_BUILD_TYPE`、`--config` 和 `NDEBUG`，不改变编译器优化或调试信息选项；例如 `cmake -S . -B build -DINK_BUILD_MODE=shipping` 配合 `cmake --build build --config Release` 选择 Shipping 行为和 Release 编译选项。多配置生成器的同一构建目录共用所选 Ink 模式，不同模式请使用不同构建目录。

`ink_tests` 包含 Core、tokenizer、parser/AST、IR 类型和 VM 指令及对象文件的单元测试，构建时同时构建 `inkc`、`ink-tokenize` 和 `ink-parse`。CTest 还运行三个工具的 CLI 进程测试和 parser 对抗输入测试。单独构建并运行全部启用测试可使用 `cmake --build build --config Release --target run_all_tests`。

启用 `BUILD_TESTING` 时，配置阶段检查 Frontend/Core 的 CMake 目标依赖，CTest 的 `FrontendDependencyBoundaryTest` 检查对应源码和头文件，禁止引入 IR、VM 或 Semantic。源码检查使用轻量文本匹配，覆盖 include 和命名空间引用；可单独运行 `ctest --test-dir build -C Release -R FrontendDependencyBoundaryTest --output-on-failure`。

LLVM/Clang 固定版本为 22.1.8；tokenizer 直接使用 Clang 的 `UnicodeCharSets.h` 和 LLVM 支持库的 Unicode 接口。NFC 校验使用 utf8proc 2.9.0 的 Unicode 15.1 实现。spdlog 1.17.0 负责所有自有 C++ 文本输出；各命令行工具自行配置输出，并直接使用 cxxopts 3.3.1 解析 `argc` / `argv`，暂不处理命令行参数和路径的 UTF-8 转换。所有自有 C++ 编译目标在目标级关闭异常。

Core 的公共头文件按职责放在 `src/include/ink/core` 下的 `support`、`source`、`name`、`numeric`、`diagnostic` 和 `config` 子目录，实现和测试在各自的 Core 目录中按相同职责组织；构建仍使用单个 `ink::core` 库，公共命名空间仍为 `ink::core`。`diagnostic/diagnostic.h` 定义诊断数据和元数据接口，参数 schema 与 traits、构造器、消费接口、引擎和格式化器分别位于 `diagnostic_traits.h`、`diagnostic_builder.h`、`diagnostic_consumer.h`、`diagnostic_engine.h` 和 `diagnostic_formatter.h`，调用方按需包含。

资源限制在 `src/include/ink/core/config/config.def` 中定义，parser 可通过对应的 `INK_PARSER_*` 环境变量配置。内部编译错误经 spdlog 输出后 panic，普通源码错误通过诊断引擎报告。

Frontend 通过统一的 `ink::frontend` 静态库提供 tokenizer、parser/AST 和共享上下文。公共头文件位于 `src/include/ink/frontend`，其中 `tokenizer` 和 `parser` 是两个子目录，命名空间分别为 `ink::frontend::tokenizer` 和 `ink::frontend::parser`；实现位于 `src/lib/frontend` 下对应的子目录。Frontend 的公开编译要求为 C++23，tokenizer 测试也使用 C++23，通过测试辅助函数将 `char8_t` UTF-8 字面量显式转换成字节字符串，最终链接同一个 frontend 库。

IR 基础类型由独立的 `ink::ir` 库提供，公共头文件位于 `src/include/ink/ir/type`，命名空间为 `ink::ir`。`Type` 继承 `Value`，具体类型本身也是值，其类型统一为 `MetaType`。`MetaType::instance()` 提供唯一的元类型实例，对应语言中的 `type`；元类型的类型是自身，所有 `TypePool` 共享该实例。`TypeSemantics` 的 `Value`、`Reference` 分别表示具体类型的值语义与引用语义，虚函数 `getSemantics()` 默认返回 `Value`，派生类可以覆盖为 `Reference`。`type_kind.h` 声明类型种类与名称查询，`type.def` 统一列出类型种类、类名和名称。`MetaType`、`IntegerType`、`FloatType`、`BoolType`、`VoidType`、`ArrayType`、`StructType`、`ObjectType`、`ReferenceType` 和 `FunctionType` 各自定义在独立的 snake_case 头文件中。`IntegerType` 通过 `Width::Bits8/Bits16/Bits32/Bits64` 与 `Signedness::Signed/Unsigned` 组合表示有符号和无符号整型，`bitWidth()` 与 `isSigned()` 查询位宽和符号属性。`FloatType` 通过 `Width::Bits32/Bits64` 表示两种浮点位宽。`getKind()` 和 `getName()` 提供类型种类与分类名称，所有整型统一为 `Integer/"integer"`，浮点型统一为 `Float/"float"`。`Type::classof(const Value *)` 及各具体类型的 `classof()` 通过 `asType()` 安全取得实际类型对象，再检查具体的 `TypeKind`，不依赖 RTTI。具有元类型的参数仍是参数对象，不会被识别为 `Type`。`StructType` 和 `ObjectType` 分别表示值语义结构体与引用语义对象。数值及布尔类型的非虚 `bitWidth()` 表示逻辑位宽，不表示目标平台存储大小。类型不可复制或经基类析构，应通过具体类型管理生命周期。

IR 的函数、基本块、字段和参数名称统一保存为 `core::Name`，相应构造参数及 `name()` 查询均使用该句柄类型。调用方通过 `core::NamePool::instance().intern()` 注册文本后传入名称，需要文本时调用名称池的 `text()`；可选名称缺省为 `core::NAME_None`。名称池统一拥有文本，IR 对象只复制句柄。同名文本共享同一句柄，名称池增长不改变既有身份。类型分类的固定拼写和字符串常量的字节内容仍使用文本类型。

`Parameter` 继承 `Value`，构造时接受 `ValueRef` 类型值、可选名称句柄和类别，`type()` 与 `getType()` 返回同一个类型值的 `const Value &`，`name()`、`kind()` 分别查询名称与参数类别。`Parameter::Kind` 包含位置参数 `Positional`、命名参数 `Named` 和变参 `Variadic`，默认采用 `Positional`；提供名称不会自动改变参数类别。`T: type` 直接表示为类型为 `MetaType` 的 `Parameter`；普通参数 `x: T` 的类型槽借用这个 `T`，无需单独的类型参数节点。例如先构造 `Parameter T(Types.getMetaType(), TName)`，再构造 `Parameter X(std::cref(T), XName)`。参数的声明作用域及编译期使用限制由调用方管理。

`Parameter` 保留签名描述的复制与赋值能力，直接使用 `Parameter Copy(T)` 会复制描述，新对象的类型仍是 `MetaType`；以 `T` 作为未命名参数的类型时，应使用 `Parameter X(std::cref(T))`、`Parameter X(std::ref(T))` 或 `Parameter X(ValueRef{T})`。每个参数对象的地址是其独立身份，同名参数及复制出的参数不会自动成为同一类型值。调用方须在声明作用域中为被借用的类型参数提供稳定存储；进入类型依赖图后，不能用副本替换原对象，也不能赋值或移动它。函数签名中的参数副本是签名描述，复制不会重绑定指向原始类型参数的引用。

`FunctionType` 保存返回类型值和有序参数列表，`returnType()` 返回 `const Value &`，`parameters()` 返回只读参数视图。函数类型拥有复制的参数描述和名称句柄，借用返回类型值及参数类型值，这些值必须比函数类型存活更久。完整签名的哈希与相等判断包括返回类型值身份、按序参数类型值身份、参数名称句柄和类别，支持空参数列表；名称、类别或被引用参数身份不同的描述不会在池内合并。

`ArrayType` 和 `ReferenceType` 的类型位置统一接受 `ValueRef`，既可以使用 `i32` 等具体 `Type`，也可以使用 `T: type` 这样的参数。`ArrayType::elementType()`、`ReferenceType::referencedType()` 返回所借用类型值的 `const Value &`。数组仍保存 64 位固定长度，支持空数组和嵌套数组；引用的语义为 `Reference`。`ArrayType(T, 4)`、`ReferenceType(T)`、返回 `T` 的函数以及类型为 `T` 的字段保留同一个参数身份。符号类型只表示类型依赖，实际大小、对齐、布局及运算合法性留到类型实参确定后的阶段处理。当前实现不包含源码泛型语义检查、泛型实例化、类型实参替换或相应后端执行。

`RecordType` 是 `StructType` 和 `ObjectType` 的公共基类，没有独立的 `TypeKind`；`RecordType::classof()` 同时识别二者。`addBase()`、`addField()`、`addMethod()` 按声明顺序追加本级基类型、字段和方法，`bases()`、`fields()`、`methods()` 返回只读视图。`addBase()` 接受 `ValueRef`，`bases()` 返回借用类型值指针的视图，因此基类型也可以暂时引用符号类型参数；继承是否合法留给后续语义检查。独立的 `Field`、`Method` 分别定义在 `ink/ir/type/field.h`、`ink/ir/type/method.h` 中。`Field` 保存声明名称句柄，`type()` 返回借用字段类型值的 `const Value &`，`addField()` 同样通过 `ValueRef` 接受具体类型或类型参数。`Method::function()` 借用实际的 `Function`，通过该函数查询签名和基本块，`Method::name()` 直接返回该函数的名称句柄，不重复存储。`addMethod()` 仅接受函数。同签名的不同函数保留各自身份。公共层只保存声明信息，重复成员及重载规则由后续语义检查处理。记录可先创建再补充成员，支持通过 `ReferenceType` 表达自引用。调用方负责保证借用目标在访问期间有效，追加可能使已有成员视图失效。记录保留各自对象身份，暂不由 `TypePool` 管理。

`Function` 和 `BasicBlock` 定义在 `ink/ir/function` 中。`Function` 继承 `Value`，保存名称句柄并拥有有序基本块，通过 `ValueRef` 接受签名，即使调用方只持有 `const Value &` 也可传入；该值语义上仍必须是实际的 `FunctionType`。`getType()` 返回签名类型值，`signature()` 保留 `const FunctionType &` 的结构化查询，二者指向同一类型对象。签名的生命周期由 `TypePool` 或调用方管理，必须覆盖函数的使用期。`createBasicBlock()` 接收名称句柄并创建、拥有基本块，`basicBlocks()` 提供按 const 性传播的遍历视图，`basicBlockCount()` 返回数量。独立构造的基本块没有所属函数，函数创建的基本块通过 `parent()` 查询所属函数，`name()` 返回名称句柄；函数及基本块不可复制或移动。追加基本块保留已有基本块地址和迭代器，基本块随所属函数销毁。

`TypePool` 定义于 `ink/ir/type/type_pool.h`，每个实例管理基础类型和复合类型，`getMetaType()` 返回全局的 `MetaType::instance()`，`getIntegerType()`、`getFloatType()`、`getBoolType()`、`getVoidType()`、`getArrayType()`、`getReferenceType()` 和 `getFunctionType()` 返回稳定的只读引用；同一池内完整参数相同的请求复用同一个类型。数组、引用和函数类型由三个 `std::unordered_set` 直接持有，类型和公开的 `ArrayType::Key`、`FunctionType::Key` 提供 `hash()` 成员，池使用 `ink/core/support/hash.h` 中的统一透明适配器 `core::Hash<T>`，其中 `T` 就是容器的 `key_type`。对象及具有 `hash()` 的查询键优先调用自身的 `hash()`，引用查询按目标类型值的地址计算哈希。相等判断通过各类型的成员 `operator==` 和 `std::equal_to<>` 完成。数组使用元素类型值地址和完整的 64 位长度，引用使用目标类型值地址，函数使用完整的签名描述，未命中时在节点内原地构造。复合类型按所借用类型值的对象身份区分，外部提供的具体类型或参数必须比池中对应的复合类型存活更久；rehash 不使已返回的类型指针或引用失效。向类型位置传入 `isTypeValue()` 为 false 的值，以及使用非法标量枚举参数，均属于 API 使用错误，会触发 `panic`；这些检查不代替源码诊断。池不可复制或移动，并发操作需要调用方同步。

`ConstantPool` 定义于 `ink/ir/constant/constant_pool.h`，借用一个必须比它存活更久的 `TypePool`，管理不可变的 `IntegerConstant`、`FloatConstant` 和 `StringConstant`。三者均直接派生自 `ink/ir/constant/constant.h` 中的公共基类 `Constant`，再由 `Constant` 继承 `Value`，可通过 `const Constant *` 统一访问完整类型；生命周期仍由具体常量及常量池管理。输入为已解析的数字或已解码的字符串内容，池不解析源码拼写。整数常量仅支持 8、16、32、64 位，使用 `std::uint64_t` 保存位模式，常量层不依赖任意位宽的 `core::IntegerBits`。`getInteger()` 接受 `std::uint64_t` 位模式并拒绝超出目标位宽的非零位；`getSignedInteger()` 和 `getUnsignedInteger()` 按目标符号和位宽验证数值范围，不支持的位宽或溢出返回 `nullptr`。`IntegerConstant::bits()` 和 `unsignedValue()` 返回原始无符号位模式，`signedValue()` 按目标位宽的补码解释并符号扩展为 `std::int64_t`。`getFloat(float/double)` 分别创建或获取 f32/f64，`getFloatBits()` 接受原始位模式并拒绝 f32 的非零越界位。数字按类型属性与完整位模式复用，区分浮点正负零和不同 NaN payload。`getString()` 复制字符串字节，支持空串、UTF-8 和嵌入 NUL，类型为不额外添加终止符的 u8 固定数组。常量使用关联类型池中的规范类型，增长不使已有常量引用及字符串视图失效；常量池不可复制或移动，并发操作需要调用方同步。

IR 值基类 `Value` 定义于 `src/include/ink/ir/value/value.h`，通过 `const Value *ValueType` 显式保存借用的类型值地址。每条类型边在构造时就已有效，`getType()` 直接解引用并返回 `const Value &`，可以指向具体类型或类型为元类型的参数。普通 `Type` 的构造直接保存规范 `MetaType` 的地址；`MetaType` 经专用的 `SelfTypedTag` 构造路径令自身的类型边指向自身。元类型单例在类内声明静态成员，在类定义完成后通过 `inline constexpr` 定义完成常量初始化，`MetaType::instance()` 可在常量表达式中使用。`isTypeValue()` 直接比较类型边与该元类型实例的身份，因此普通 `Type` 和 `T: type` 都是类型值，而参数 `x: T` 仍是普通值。虚查询 `asType()` 默认返回 `nullptr`，`Type` 覆盖后返回自身的 `const Type *`，用于区分实际类型对象与具有元类型的参数。被借用的类型值必须比值对象存活更久；派生类负责维持这一借用约定。`Value` 的构造与析构由派生类使用，通用值和类型不提供复制语义；`Parameter` 为复制签名描述提供专门的复制操作。对象应通过具体类型管理生命周期。

IR 指令位于 `ink/ir/instruction`，`Instruction` 继承 `Value`，通过 `getKind()`、`getName()` 查询由 `instruction.def` 统一定义并显式编号的种类和拼写。`AddInstruction` 借用两个同类型的整数或浮点操作数，并从左操作数取得结果类型；`ReturnInstruction` 自身类型为 `VoidType`，可借用返回值，也可表示无值返回。`AllocaInstruction` 描述类型值 `T` 的栈存储，结果类型为 `ReferenceType(T)`，`getAllocatedType()` 返回被分配类型值的 `const Value &`，可以是具体类型或类型参数；固定数组直接使用 `ArrayType`，分配引用类型时仍增加一层引用。其构造函数的类型参数使用 `ValueRef`。该节点只描述分配，实际大小、对齐和栈帧布局留给后续后端。操作数及其类型必须覆盖指令的使用期，类型合法性由调用方负责。

`LoadInstruction` 保存来源引用和显式结果类型值，`getSource()` 返回来源值；`StoreInstruction` 保存目标引用和写入值，`getDestination()`、`getSource()` 分别查询二者，自身类型为 `VoidType`。直接构造只建模，调用方负责类型合法性。Builder 的 `createLoad(Source)` 要求来源为具体的 `ReferenceType(T)`，通过 `asType()` 和 `TypeKind` 识别后推导结果为类型值 `T`，每次只去掉一层引用；`createStore(Destination, Source)` 要求目标为 `ReferenceType(T)`，且写入值的类型与 `T` 是同一个类型值对象。因此存取符号类型的槽位时同样保留参数身份。非引用地址或不匹配的写入类型使创建返回 `nullptr`，不改变块或当前插入点；这些接口不隐式转换类型。

指令构造函数和 Builder 的操作数参数统一使用 `ValueRef`，即 `std::reference_wrapper<const Value>`。调用方可直接传入普通或 const 左值，也可传入已有引用包装；临时值不能隐式转换为 `ValueRef`。操作数查询仍返回只读引用或指针，调用方负责保证被借用值的生命周期。

`InstructionBuilder` 定义于 `ink/ir/instruction/instruction_builder.h`，借用 `TypePool`，通过 `setInsertPoint(BasicBlock &)` 选择块尾，或通过 `setInsertPoint(Instruction &)` 选择已有指令之前的位置；后者遇到游离指令时返回 `false` 并保留当前插入点。连续创建保持调用顺序，插入不会改变既有指令地址和迭代器。`createAdd()`、`createAlloca()`、`createLoad()`、`createStore()`、`createReturn()` 返回由目标 `BasicBlock` 拥有的具体指令指针；未设置或已清除插入点时返回 `nullptr`。`createAlloca(ValueRef)` 接受具体类型或 `T: type` 参数，在关联类型池中取得结果引用类型；传入非类型值时返回 `nullptr`，不改变块或插入点。外部传入的分配类型值须比该池存活更久。`clearInsertPoint()` 清除当前选择，`getInsertBlock()`、`getInsertBefore()` 查询当前插入位置。块通过 `instructions()` 提供传播 const 的双向引用视图，通过 `instructionCount()` 查询数量；指令通过 `parent()` 查询所属块。Builder 销毁不影响已创建的指令，类型池和借用操作数必须覆盖这些指令的使用期，当前插入块及锚点必须在使用插入点期间保持存活。

常量池使用三个 `std::unordered_set` 直接持有常量对象，不额外保存一份 key 或使用 `unique_ptr`。常量及公开的数字查询键 `IntegerConstant::Key` 和 `FloatConstant::Key` 提供 `hash()` 成员，三个池分别使用 `core::Hash<IntegerConstant>`、`core::Hash<FloatConstant>` 和 `core::Hash<StringConstant>`，模板参数均为容器自身的 `key_type`。对象及数字查询键优先调用自身的 `hash()`，`StringConstant::hash(std::string_view)` 静态重载将字符串查询统一为字节视图。相等判断通过各类型的成员 `operator==` 和 `std::equal_to<>` 完成。整数使用位宽、符号属性和完整的 64 位位模式，浮点使用位宽和完整的 64 位位模式，字符串使用 `std::string_view` 的完整字节内容；哈希与相等判断使用相同字段，命中时无需构造常量或复制字符串。三个常量类型的构造函数公开，调用方可直接构造，并负责保证所借用的类型比常量存活更久。常量仍不可复制或移动，池在节点内原地构造常量；rehash 不使已返回的常量指针、引用或字符串视图失效。

VM 指令和对象文件由独立的 `ink::vm` 库提供，仅依赖 `ink::core`，命名空间为 `ink::vm`。公共头文件分别位于 `src/include/ink/vm/instruction` 和 `src/include/ink/vm/object`，实现和测试位于 `src/lib/vm` 和 `src/testcase/vm` 的对应子目录。指令基类 `Instruction` 保存操作码和可选的结果地址，提供名称、位宽和结果地址查询；具体指令使用地址或立即数操作数，通过具体派生类型管理生命周期。IR 的 `Type` 和 `Value` 仍由 `ink::ir` 提供。对象文件 API 负责对象结构验证、二进制读写和显式错误报告，格式详见 [对象文件格式](docs/Ink-Object-Format.md)。

成对的 const／非 const 访问器使用 C++23 显式对象参数合并定义，保持原来的引用、指针和只读节点数组返回类型。值成员直接通过 `Self` 访问；借用引用和指针指向的对象使用 `std::forward_like<decltype(Self) &>` 传播只读属性，并保持右值调用仍返回借用的左值引用或指针。所有借用结果仍受原对象及底层存储的生命周期约束。

AST 类声明位于 `src/include/ink/frontend/parser/ast`，每个 class 使用一个 snake_case 头文件，共 80 个类：64 个具体节点、7 个基类和 9 个辅助记录。`ast/types.h` 集中公共别名、`NameToken`、`RestBinding`、`TypeQualifier` 和 8 个枚举，`ast/fwd.h` 提供类前向声明，`ast/cast.h` 提供 `isa`、`dyn_cast` 和 `cast`。`src/include/ink/frontend/parser/ast.h` 保留聚合入口，调用方也可直接包含所需节点头文件。命名空间、公开 API 及 `ASTNodes.def` 的节点 ID 保持不变；`ast_context.h`、`ast_kind.h`、`ast_visitor.h` 和 `ast_walker.h` 仍位于 parser 根目录，现有实现仍位于 `src/lib/frontend/parser`。

`frontend::FrontendContext` 直接借用 `core::SourceManager` 和 `core::DiagnosticEngine`，二者必须比上下文存活更久；tokenizer 和 parser 使用这些服务而不依赖工具层。现有工具和测试各自拥有服务实例，以保持编译会话之间的源码和诊断隔离。

`inkc::CompilationContext` 是工具私有的编译会话资源所有者，定义位于 `src/tools/inkc/compilation_context.h`，拥有目标描述、源码管理器和诊断引擎。库和测试不依赖此类型。`inkc` 使用固定版本 cxxopts 3.3.1，以无异常模式解析必需的 `-i FILE` / `--input FILE`，并提供帮助和版本选项。入口按原始字节读取源码，依次调用 frontend 的 tokenizer 和 parser；词法与语法检查成功时以状态 0 退出，stdout 保持静默。当前不进行语义分析、生成 IR 或执行源码，不支持 `-S`。词法或语法错误返回 1，参数、文件输入输出或诊断输出错误返回 2，内部编译错误沿用 ICE/panic 机制。详见 [命令行接口](docs/command-line.md)。

Target 目录提供目标平台描述，头文件位于 `src/include/ink/target`。`target::TargetContext`、`target::PointerWidth` 和 `target::ByteOrder` 定义在 `ink/target/target_context.h` 中；`TargetContext` 保存指针宽度、字节序及标量对齐信息，`native()` 返回宿主目标。inkc 私有的 `CompilationContext` 保存这一描述。

Core 的 `Singleton<T>` 提供按需初始化的单例基类；派生类将构造、析构函数设为私有，并声明 `Singleton<T>` 为友元。`NamePool::instance()` 提供进程共享的名称池，名称句柄和字符串视图在池增长时保持有效。`NAME_None` 与默认构造的 `Name{}` 相等，索引均为 `Name::InvalidIndex`，表示无效句柄；空字符串不会注册，查找未命中的名称返回该无效句柄。名称池不预注册字符串 `"None"`，该字符串可作为普通名称注册。名称池操作需要调用方进行并发同步。

`core::IntegerBits` 保存固定宽度的多字位模式，声明位于 `src/include/ink/core/numeric/integer_bits.h`。有效位宽为 1 至 `IntegerBits::MaxBitWidth`（2²³），所有位宽都允许；该上限约束编译器存储资源，不表示后端或外部 ABI 已支持这些类型。`words()` 按数值位次从低到高返回只读的 64 位字，最高字超出位宽的位始终为零，与宿主及目标字节序无关。`fromWords()` 允许补齐高零和忽略额外高零，但拒绝任何非零越界位；`fromUnsigned()`、`fromSigned()` 检查宿主整数是否适合所选位宽，失败返回 `std::nullopt`。`zeroExtend()`、`signExtend()` 和 `truncate()` 显式改变位宽，`negated()` 保留位宽并按补码取负；这些是数值基础操作，没有接入源码隐式转换或常量折叠。`tryGetUInt64()`、`tryGetInt64()` 在不能精确表示时返回 `std::nullopt`，不会丢弃高位。
