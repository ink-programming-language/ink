# ink

`main` 保留 Core、目标平台描述、tokenizer、parser、AST 及对应测试。IR、semantic 及其测试已移除；`inkc` 仅检查输入文件的词法和语法，`ink-tokenize` 和 `ink-parse` 分别提供 token 流和 AST 输出。

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

`ink_tests` 包含 Core、tokenizer 和 parser/AST 的单元测试，构建时同时构建 `inkc`、`ink-tokenize` 和 `ink-parse`。CTest 还运行三个工具的 CLI 进程测试和 parser 对抗输入测试。单独构建并运行全部启用测试可使用 `cmake --build build --config Release --target run_all_tests`。

启用 `BUILD_TESTING` 时，配置阶段检查 Frontend/Core 的 CMake 目标依赖，CTest 的 `FrontendDependencyBoundaryTest` 检查对应源码和头文件，禁止引入 IR 或 Semantic。源码检查使用轻量文本匹配，覆盖 include 和命名空间引用；可单独运行 `ctest --test-dir build -C Release -R FrontendDependencyBoundaryTest --output-on-failure`。

LLVM/Clang 固定版本为 22.1.8；tokenizer 直接使用 Clang 的 `UnicodeCharSets.h` 和 LLVM 支持库的 Unicode 接口。NFC 校验使用 utf8proc 2.9.0 的 Unicode 15.1 实现。spdlog 1.17.0 负责所有自有 C++ 文本输出；各命令行工具自行配置输出，并直接使用 cxxopts 3.3.1 解析 `argc` / `argv`，暂不处理命令行参数和路径的 UTF-8 转换。所有自有 C++ 编译目标在目标级关闭异常。

Core 的公共头文件按职责放在 `src/include/ink/core` 下的 `support`、`source`、`name`、`numeric`、`diagnostic` 和 `config` 子目录，实现和测试在各自的 Core 目录中按相同职责组织；构建仍使用单个 `ink::core` 库，公共命名空间仍为 `ink::core`。`diagnostic/diagnostic.h` 定义诊断数据和元数据接口，参数 schema 与 traits、构造器、消费接口、引擎和格式化器分别位于 `diagnostic_traits.h`、`diagnostic_builder.h`、`diagnostic_consumer.h`、`diagnostic_engine.h` 和 `diagnostic_formatter.h`，调用方按需包含。

资源限制在 `src/include/ink/core/config/config.def` 中定义，parser 可通过对应的 `INK_PARSER_*` 环境变量配置。内部编译错误经 spdlog 输出后 panic，普通源码错误通过诊断引擎报告。

Frontend 通过统一的 `ink::frontend` 静态库提供 tokenizer、parser/AST 和共享上下文。公共头文件位于 `src/include/ink/frontend`，其中 `tokenizer` 和 `parser` 是两个子目录，命名空间分别为 `ink::frontend::tokenizer` 和 `ink::frontend::parser`；实现位于 `src/lib/frontend` 下对应的子目录。Frontend 的公开编译要求为 C++23，tokenizer 测试也使用 C++23，通过测试辅助函数将 `char8_t` UTF-8 字面量显式转换成字节字符串，最终链接同一个 frontend 库。

成对的 const／非 const 访问器使用 C++23 显式对象参数合并定义，保持原来的引用、指针和只读节点数组返回类型。值成员直接通过 `Self` 访问；借用引用和指针指向的对象使用 `std::forward_like<decltype(Self) &>` 传播只读属性，并保持右值调用仍返回借用的左值引用或指针。所有借用结果仍受原对象及底层存储的生命周期约束。

AST 类声明位于 `src/include/ink/frontend/parser/ast`，每个 class 使用一个 snake_case 头文件，共 80 个类：64 个具体节点、7 个基类和 9 个辅助记录。`ast/types.h` 集中公共别名、`NameToken`、`RestBinding`、`TypeQualifier` 和 8 个枚举，`ast/fwd.h` 提供类前向声明，`ast/cast.h` 提供 `isa`、`dyn_cast` 和 `cast`。`src/include/ink/frontend/parser/ast.h` 保留聚合入口，调用方也可直接包含所需节点头文件。命名空间、公开 API 及 `ASTNodes.def` 的节点 ID 保持不变；`ast_context.h`、`ast_kind.h`、`ast_visitor.h` 和 `ast_walker.h` 仍位于 parser 根目录，现有实现仍位于 `src/lib/frontend/parser`。

`frontend::FrontendContext` 直接借用 `core::SourceManager` 和 `core::DiagnosticEngine`，二者必须比上下文存活更久；tokenizer 和 parser 使用这些服务而不依赖工具层。现有工具和测试各自拥有服务实例，以保持编译会话之间的源码和诊断隔离。

`inkc::CompilationContext` 是工具私有的编译会话资源所有者，定义位于 `src/tools/inkc/compilation_context.h`，拥有目标描述、源码管理器和诊断引擎。库和测试不依赖此类型。`inkc` 使用固定版本 cxxopts 3.3.1，以无异常模式解析必需的 `-i FILE` / `--input FILE`，并提供帮助和版本选项。入口按原始字节读取源码，依次调用 frontend 的 tokenizer 和 parser；词法与语法检查成功时以状态 0 退出，stdout 保持静默。当前不进行语义分析、生成 IR 或执行源码，不支持 `-S`。词法或语法错误返回 1，参数、文件输入输出或诊断输出错误返回 2，内部编译错误沿用 ICE/panic 机制。详见 [命令行接口](docs/command-line.md)。

Target 目录提供目标平台描述，头文件位于 `src/include/ink/target`。`target::TargetContext`、`target::PointerWidth` 和 `target::ByteOrder` 定义在 `ink/target/target_context.h` 中；`TargetContext` 保存指针宽度、字节序及标量对齐信息，`native()` 返回宿主目标。inkc 私有的 `CompilationContext` 保存这一描述。

Core 的 `Singleton<T>` 提供按需初始化的单例基类；派生类将构造、析构函数设为私有，并声明 `Singleton<T>` 为友元。`NamePool::instance()` 提供进程共享的名称池，名称句柄和字符串视图在池增长时保持有效。`NAME_None` 与默认构造的 `Name{}` 相等，索引均为 `Name::InvalidIndex`，表示无效句柄；空字符串不会注册，查找未命中的名称返回该无效句柄。名称池不预注册字符串 `"None"`，该字符串可作为普通名称注册。名称池操作需要调用方进行并发同步。

`core::IntegerBits` 保存固定宽度的多字位模式，声明位于 `src/include/ink/core/numeric/integer_bits.h`。有效位宽为 1 至 `IntegerBits::MaxBitWidth`（2²³），所有位宽都允许；该上限约束编译器存储资源，不表示后端或外部 ABI 已支持这些类型。`words()` 按数值位次从低到高返回只读的 64 位字，最高字超出位宽的位始终为零，与宿主及目标字节序无关。`fromWords()` 允许补齐高零和忽略额外高零，但拒绝任何非零越界位；`fromUnsigned()`、`fromSigned()` 检查宿主整数是否适合所选位宽，失败返回 `std::nullopt`。`zeroExtend()`、`signExtend()` 和 `truncate()` 显式改变位宽，`negated()` 保留位宽并按补码取负；这些是数值基础操作，没有接入源码隐式转换或常量折叠。`tryGetUInt64()`、`tryGetInt64()` 在不能精确表示时返回 `std::nullopt`，不会丢弃高位。
