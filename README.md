# ink

当前按 [`docs/Ink-Lexical-Rules.md`](docs/Ink-Lexical-Rules.md)、[`docs/Ink-grammar-Rules.bnf`](docs/Ink-grammar-Rules.bnf) 和 [`docs/Ink-Parser-Design.md`](docs/Ink-Parser-Design.md) 构建前端，包含 Core、CLI 支持库、tokenizer、parser、独立 IR 对象模型、execution 执行模块及 semantic 分析器、`ink-tokenize`、`ink-parse`、`inkc` 和测试。`semantic::Analyzer::analyze` 当前支持普通函数签名、直线函数体、定参调用、整数/字符串字面量、return 和整数/bool 编译期求值；`NameResolver` 提供词法作用域、名字绑定和函数重载候选集合。原 semantic/model 已迁入新 IR 模块，execution 支持编译期求值及直线 IR 函数执行，`inkc --interpret` 提供源码解释执行入口；旧 backend 和独立 interpreter 工具仍未接入当前构建。测试按模块位于 `src/testcase`。

Core 的 `PANIC(Message)` 宏通过独立的 spdlog stderr logger 输出消息和调用位置、同步刷新后调用 `abort()`，不依赖全局日志开关。`DiagnosticEngine::report` 遇到 ICE 会立即输出诊断编号和格式化消息并 panic，先于消费者分发；普通用户错误仍正常分发并返回。无效 AST 归档、资源上限等现有 ICE 同样遵循此规则。

## 构建

Core 配置集中定义在 `src/include/ink/core/config.def`，每项包含枚举名、环境变量名和字符串默认值，并生成 `ink::core::ConfigKind` 枚举及以枚举为键的配置映射。`ink::core::ConfigManager::get<ink::core::ConfigKind::SemanticBlockDepthLimit>()` 直接返回 `std::string`，每次读取对应环境变量，未设置时返回定义中的默认值；配置项通过枚举模板参数选择，找不到枚举对应的配置时报告 `INK-C0001` 并立即 panic。`getSize<ink::core::ConfigKind::SemanticBlockDepthLimit>()` 直接返回 `std::size_t`，读取非负十进制整数，空值、非法格式或溢出的环境变量会回退到默认值；默认值也无法解析时报告 `INK-C0002` 并立即 panic。`INK_SEMANTIC_BLOCK_DEPTH_LIMIT` 控制语义分析的块嵌套上限，默认 `256`，`0` 表示不允许任何块；例如 PowerShell 中执行 `$env:INK_SEMANTIC_BLOCK_DEPTH_LIMIT = "128"` 可覆盖该上限。

其他资源限制也由 `config.def` 提供默认值，字节预算的环境变量使用十进制字节数：

| 环境变量 | 默认值 | 用途 |
| --- | --- | --- |
| `INK_SEMANTIC_TYPE_DEPTH_LIMIT` | `256` | 语义类型递归深度 |
| `INK_SEMANTIC_EXPRESSION_DEPTH_LIMIT` | `256` | 语义表达式递归深度 |
| `INK_EXECUTION_MAX_STEPS` | `100000` | 编译期执行步骤预算 |
| `INK_EXECUTION_MAX_OBJECTS` | `16384` | 执行期间累计存储分配预算（Cell/Buffer） |
| `INK_EXECUTION_MAX_CALL_DEPTH` | `256` | 编译期活动调用深度 |
| `INK_EXECUTION_MAX_EVALUATION_DEPTH` | `64` | 语义求值和 IR 调用共享的求值嵌套 |
| `INK_PARSER_MAX_NESTING_DEPTH` | `128` | Parser 嵌套深度 |
| `INK_PARSER_MAX_DIAGNOSTICS` | `100` | Parser 用户诊断数量 |
| `INK_PARSER_MAX_WORK` | `10000000` | Parser 工作量 |
| `INK_PARSER_MAX_ALLOCATION_BYTES` | `67108864`（64 MiB） | Parser AST 分配预算 |
| `INK_AST_ARCHIVE_MAX_BYTES` | `268435456`（256 MiB） | AST 归档大小 |
| `INK_AST_ARCHIVE_MAX_SOURCE_BYTES` | `67108864`（64 MiB） | 归档源码大小 |
| `INK_AST_ARCHIVE_MAX_NODES` | `1000000` | 归档 AST 节点数 |
| `INK_AST_ARCHIVE_MAX_TOKENS` | `2000000` | 归档 Token 数 |
| `INK_AST_ARCHIVE_MAX_ARRAY_ELEMENTS` | `1000000` | 归档数组元素数 |
| `INK_AST_ARCHIVE_MAX_ALLOCATION_BYTES` | `268435456`（256 MiB） | 归档解码分配预算 |

`ParseLimits`、`ASTArchiveLimits` 和 `ExecutionLimits` 在构造时通过 `ConfigManager` 读取配置，调用方可以继续显式设置各字段，优先级为调用方显式值、有效环境变量、`config.def` 默认值。之后修改环境变量只影响新建的 Limits；解析、序列化、反序列化和构造执行引擎省略 Limits 参数时会读取当前配置。Parser 嵌套深度仍受内部 `512` 层硬上限约束。语义类型递归上限在每次 `Analyzer::analyze()` 开始时读取，深度从 `0` 计数，达到上限即报告 ICE 并 panic；`0` 会拒绝任何类型分析。

初始化固定版本的第三方依赖：

```powershell
git submodule update --init --recursive
```

使用 Visual Studio 2022 生成、编译 LLVM/Clang 依赖和 Ink，并运行全部测试：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

自有库和工具通过 `GLOB_RECURSE CONFIGURE_DEPENDS` 递归收集各自目标目录中的 `*.cpp`，测试按模块递归收集 `*_test.cpp`；新增或删除匹配文件会在下次构建时自动触发 CMake 重新配置，无需修改源文件清单。Visual Studio 下增删源码后使用上面的默认构建入口，避免直接构建单个目标时 MSBuild 在当前轮次保留该目标的旧源文件列表。

`ink_tests` 是统一的 GoogleTest 入口，包含 Core、tokenizer、parser/AST、IR、execution 和 semantic 的单元测试；在 CLion 中运行同名配置即可执行这些用例。构建该目标同时构建 `ink-tokenize`、`ink-parse` 和 `inkc`，CTest 再运行额外的 CLI 进程测试和源码执行测试。

LLVM 的构建树包会生成到 `build/third_party/llvm/lib/cmake/llvm`，主工程通过 `LLVM_DIR` 和 `find_package(LLVM CONFIG)` 导入。`utf8proc 2.9.0` 固定使用 Unicode 15.1，与当前词法规则的 Unicode 版本一致。命令行工具统一使用仓库自有的 `ink::cli::Application` 解析参数，并通过 `ParseResult` 显式返回解析和定义错误，不使用 C++ 异常作为控制流。spdlog 1.17.0 负责 Ink 的统一文本输出和运行时日志。

Ink 所有工具共用的参数拼写、输入输出、诊断和退出码规则见 [`docs/command-line.md`](docs/command-line.md)。

基于 [`docs/grammar.bnf`](docs/grammar.bnf) 交互确认的语义分析、泛型、comptime 和新 IR 设计规则见 [`docs/IR.md`](docs/IR.md)。该文档记录设计约定，不表示相关编译管线已经实现。

Ink 跨 module 函数、成员函数、闭合实例、全局变量、Imported 符号和 `extern "C"` 的链接名称规则见 [`docs/name-mangling.md`](docs/name-mangling.md)。

## inkc 解释执行

`inkc --interpret --entry NAME -i FILE` 读取源码，经 tokenizer、parser、semantic 生成 IR 后执行指定入口；`--entry` 默认 `main`，`--input` 是 `-i` 的别名，`FILE` 为 `-` 时读取标准输入。入口须为零参数、具有 IR 函数体的普通 Ink 函数，返回 `void` 或 `i32`：void 退出为 0，i32 作为进程退出码。诊断写入 stderr，源程序的 stdout 输出直接保留，不打印 IR。当前仅支持解释模式；`-oir` 保留参数识别，与 `--interpret` 冲突，未指定解释模式时明确报告尚未实现。

构建后在仓库根目录运行平台对应的 hello world 源码。Windows x64 / PowerShell：

```powershell
.\build\src\tools\inkc\Release\inkc.exe --interpret -i src/testcase/execution/programs/hello_world.windows.ink
ctest --test-dir build -C Release -R '^(ExecutionSourceTest[.]HelloWorld|InkcProcessTest)$' --output-on-failure
```

Linux x86-64 / 单配置构建：

```sh
./build/src/tools/inkc/inkc --interpret -i src/testcase/execution/programs/hello_world.linux.ink
ctest --test-dir build -R '^(ExecutionSourceTest[.]HelloWorld|InkcProcessTest)$' --output-on-failure
```

两份程序分别按 Windows `_write` 和 Linux `write` 的 ABI 声明外部函数，main 写入 `hello, world\n` 后返回 0。`ExecutionSourceTest.HelloWorld` 启动真实 `inkc` 进程，将 stdout 直接写入临时文件，逐字节检查 Windows CRLF / Linux LF、空 stderr 和退出码 0；`InkcProcessTest` 检查入口选择、调用错误及返回结果。完整参数与退出规则见 [命令行接口](docs/command-line.md)。

## Tokenizer 接口

- `tokenize(FrontendContext &, std::string)` 或 `tokenizeSource(FrontendContext &, SourceId)` 返回持有源码和解码值的 `TokenizedBuffer`。先检查 `succeeded()`；成功结果以唯一的 `END_OF_FILE` 结束。全局 UTF-8／NUL／BOM 校验失败不输出 token，其他词法错误保留此前完成的 token，不追加 EOF。
- `TokenKind` 为扁平枚举，C++ 枚举项按仓库约定使用 UpperCamelCase，`tokenKindName()` 返回文档中的大写名称。空白、注释和错误不占 token 种类。标识符的 NFC 名称、字符串的解码 UTF-8 值和字符的 Unicode 标量值分别保存在 payload 中。
- Core 的 `SourceLocation` 使用 32 位不透明编码，默认无效，`fromByteOffset(0)` 表示有效的文件起点。`SourceRange` 始终为原始字节的半开区间，可通过 `getBegin()`／`getEnd()` 获取位置。文件身份由源码 buffer 或诊断的 `Source` 保存；不同文件的位置不可直接比较。超过 `0xFFFFFFFE` 字节的输入显式报 `SourceTooLarge`。
- Unicode 正式名称复用 LLVM 的公开名称查询接口；完整别名由官方 Unicode 15.1 `NameAliases.txt` 生成。更新方式见 `cmake/generate_unicode_aliases.py`，生成表记录原始数据的 SHA-256，XID 和 NFC 数据不复制到本项目。

## Parser 接口

- `ink::parser::parse(FrontendContext &, TokenizedBuffer, ParseLimits)` 返回 `ParseResult`。`Unit` 持有不可变 TokenBuffer、ASTContext、ModuleAST 和恢复记录；节点和源码引用在 Unit 销毁前有效。`succeeded()` 同时检查词法结果、当前调用的语法错误和解析状态。
- `Completed` 表示扫描结束，错误输入仍能返回恢复后的 AST。`Cancelled` 明确标识取消时的部分结果。ParseLimits 控制嵌套、诊断、工作量与 AST 分配；嵌套、工作量或分配预算耗尽会输出 ICE 并立即 panic，不再返回部分 AST。ICE 不受诊断数量上限或诊断事务影响。
- `ASTVisitor`／`ConstASTVisitor` 只分派当前节点，`StrictExprVisitor` 要求覆盖全部表达式。`ASTWalker` 使用显式栈按源码顺序遍历，支持跳过子节点和提前停止。`verifyAST` 校验结构契约；`dumpAST` 返回字符串，不直接产生进程输出。
- `ink-parse INPUT` 或通过标准输入运行 `ink-parse -` 可查看结构与恢复记录。正常退出为 0，词法或语法错误为 1，输入读取失败为 2。
- 测试包含文法家族、恢复边界、Arena 析构与回滚、Visitor、源码生命周期、长链、资源预算、Token 边界扰动及确定性随机输入。可运行 `cmake --build build --config Release --target run_all_tests` 执行完整回归。

## Semantic 分析接口

`Analyzer` 的头文件和实现放在 semantic 的 `analyzer` 子目录；名字解析放在 `name_resolve` 子目录，拆分为 `binding.h`、`scope.h`、`name_resolver.h` 和 `name_resolver.cpp`，三个类型均位于 `ink::semantic` 命名空间。

`Analyzer::analyze(SemanticContext &, const parser::ParseResult &, std::string_view ModuleName)` 为成员函数，当前支持空模块、块作用域和普通定参函数的声明/定义，包括 `extern "C"`、基础标量/指针/引用签名、形参绑定和函数体遍历；用户错误报告诊断并返回 `nullptr`，未支持的语义报告 ICE 并终止。`SemanticContext::scopeStore()` 持有作用域、绑定及成员/定义作用域索引；每次分析由 `AnalysisState` 拥有独立 `NameResolver`，通过 `enterScope()` 和 `exitScope()` 管理当前位置，并可从已有 `Scope` 恢复查找。resolver 销毁后绑定仍然保留，支持 `Value *` 和泛型 `Decl *`；`lookup()` 查找当前及父作用域，`lookupLocal()` 仅查当前作用域。`enterScope(Owner)` 为实体创建成员作用域，`lookupMember(Owner, Name)` 查找该实体的直接成员，别名共享同一实体的成员绑定。普通函数支持按参数类型形成重载集合及选择、定参调用、显式 return、直线返回路径检查和 void 隐式返回；C 调用中的字符串常量可通过独立可写副本传给 *u8。分支控制流、泛型源码、C 变参及重复声明合并仍待实现。接口、生命周期和具体支持范围见 [语义分析接口](docs/Ink-Semantic-Analysis.md)。

## Execution 执行接口

execution 的公共头与实现分别位于 `src/include/ink/execution` 和 `src/lib/execution`，按相同的功能目录组织：

| 子目录 | 职责 |
| --- | --- |
| `engine/` | 执行引擎、帧、IR 解释、内存访问与运算调度 |
| `memory/` | Heap、Cell/Buffer 存储、弱存储身份与指针载荷 |
| `value/` | 值基类、值引用与结果、七种不可变值子类、精确位宽整数运算 |
| `support/` | 执行对象基类、公共状态与结果、状态名称转换 |
| `ffi/` | 外部调用、参数封送、ABI 类型映射、宿主符号查找与缓存 |

七种值子类各有独立的 `value/execution_*_value.h` 和 `.cpp`，`ExecutionValueRef`、`ExecutionValueResult` 仍与基类位于 `value/execution_value.h`。测试位于 `src/testcase/execution` 的 `engine/`、`value/`、`ffi/` 和 `cli/`；源码样例保留在 `programs/`。

`ink::execution` 模块提供 `ExecutionEngine`、模块/分析/调用帧和可变对象，并已接入 Analyzer。支持整数/bool 编译期变量、表达式和块、赋值、函数调用、短路及编译期条件/循环；模块按源码顺序处理，表达式按实际控制流直接求值，变量修改只影响后续求值。初始化结果存入对象，后续读取取得对象当前值；每轮循环和每次实际调用使用独立帧执行。运行时局部变量可用编译期结果初始化，但仍不能在编译期读取。接口与边界见 [语义分析接口](docs/Ink-Semantic-Analysis.md) 和 [执行设计](docs/Ink-Semantic-Design.md#65-当前最小执行模块的边界)。

[`ExecutionObject`](src/include/ink/execution/support/execution_object.h) 是执行对象基类；抽象的 [`ExecutionValue`](src/include/ink/execution/value/execution_value.h) 派生出不可变的 void、bool、整数、浮点位模式、字符串、指针和函数值。`ExecutionValueRef` 通过 RAII 引用计数共享只读载荷，复制结果不复制整数和字符串内容；标量和字符串结果可以在创建它们的引擎销毁后继续存在，但所借用的 IR 类型仍须有效，函数值也要求引用的 IR 函数保持有效。这套机制不使用 GC。

[`ExecutionHeap`](src/include/ink/execution/memory/execution_heap.h) 统一创建值和存储，并唯一拥有 `ExecutionCell` 与 `ExecutionBuffer`；Cell 保存可替换的值引用，Buffer 保存固定字节数组。帧结束或显式 `release()` 真正释放对应存储。`ExecutionPlace` 和缓冲区指针使用弱 `ExecutionStorageRef`，不延长存储寿命；堆控制块身份和槽位代次共同阻止旧句柄访问重建堆或复用槽位中的新对象。槽位可以重用，累计存储分配预算不会随释放返还。

`ExecutionEngine::execute(Function, span<const ExecutionValueRef>)` 从普通 IR 函数入口执行 `Alloca`、`Store`、`Load`、整数 `Add`、`CString`、`Call` 和 `Return`；块内函数声明由 `makeFunctionValue()` 构造函数值，实际调用由 `Call` 指令执行。调用帧保存参数和 SSA 值引用，记录局部存储的生命周期，同一调用结果被多次读取不会重复执行副作用，新调用建立独立帧。普通 if/循环 IR 尚未实现。所有非泛型函数在定义处生成 IR 函数体，编译期调用和普通执行统一使用 `execute()`，返回独立的执行值；语义层仅在编译期结果边界转换为可表示的常量。`allocate()`、`load()`、`store()` 负责常量边界的转换，共用 `allocateValue()`、`loadValue()`、`storeValue()` 的存储实现，运行中间值不驻留到常量池。执行对象的 C++ 继承体系不新增 Ink class 实例或聚合值的执行语义。

整数运算使用项目自有的 `ExecutionInteger`，execution 不使用 `llvm::APInt`，引擎和帧直接持有各自状态。`comptime func` 与普通函数一样在定义处检查并生成 IR，未调用的函数也检查函数体；运行时使用这类函数会报告用户诊断。函数体能力统一受当前 IR 限制，尚未支持的参数相关分支、循环和非加法运算显式诊断。显式 `comptime` 表达式、块和静态循环仍由语义层在生成 IR 时求值或展开；泛型延迟实例化另行实现。

编译期和 IR 中的 `extern "C"` 调用通过系统 API 按声明中的原名查找当前进程符号，再根据 IR 签名使用现有 libffi 调用，不限制函数名。`ffi/native_symbol.cpp` 负责平台符号解析，`ffi/ffi_type.cpp` 负责类型映射，`ffi/ffi_argument.cpp` 负责 `ExecutionValueRef` 到原生参数的转换，`ffi/ffi_call.cpp` 负责调用编排和返回值处理；外部调用入口显式接收调用者的 `ExecutionHeap`。FFI 支持 bool、8/16/32/64 位整数、f32/f64、指针参数与返回值和 void 返回；聚合、变参及 f16 尚不支持。带类型的 `ExecutionPlace` 存储不能直接当作宿主缓冲区。缓冲区指针只保留弱存储身份和偏移，存储释放后 `status()` 返回 `ExpiredPlace`；原生不透明地址可继续传给 FFI，执行引擎不任意解引用。指针和函数结果不能冻结到 IR 常量。

每个 `ExecutionEngine` 持有独立的 `NativeSymbolCache`，按精确符号名保存首次成功解析的地址，供编译期和 IR 外部调用共用；查找失败不会缓存。缓存拥有名称字符串，不持有 DLL／共享库的加载引用，调用方必须保证相关模块在缓存和返回地址的使用期间保持加载。如需卸载模块或重新绑定同名符号，应在相关调用和地址使用结束后、卸载前调用 `clearNativeSymbolCache()`；下一次调用会重新解析。

`CString` 缓冲区在执行该指令的函数帧结束时释放，FFI 返回其内部地址不延长该生命周期。直接传给 FFI 的字符串另建可写临时副本；未由返回值引用的副本在调用结束时释放，返回其内部或尾后地址时则明确将清理责任移交给调用者的 Heap，直到显式释放或 Heap 销毁。返回指针仍不拥有缓冲区，已有帧缓冲区也不会因此提升生命周期。测试覆盖“源码 → tokenizer/parser AST → semantic IR → `execute(Entry)` → `_write`/`write` → 管道字节与返回值断言”，并检查重复读取调用结果只写入一次、嵌套调用及指针逃逸。Windows 源码直接声明 `_write`，Linux 声明 `write`。声明示例和当前类型边界见 [编译期外部调用](docs/Ink-Semantic-Analysis.md#编译期外部调用)。

## IR 对象模型

公共类型统一位于 `ink::ir`，包括 Value、类型与常量及其池、函数、基本块、指令、Name/NamePool、Decl 及泛型 AST 关联，以及 IRBuilder。Analyzer、名称解析和作用域保留在 `ink::semantic`。

这套模型提供函数、调用、内存、加法和返回节点的构造接口，execution 已接入这些直线 IR 指令。完整源码语义分析、控制流解释、后端及完整图验证器仍待实现。当前分析能力与后续缺口见 [语义分析接口](docs/Ink-Semantic-Analysis.md)。

- `IRBuilder::createFunction()` 根据签名创建 `FunctionParameter`，通过 `parameters()` 访问；形参通过 `outer()` 关联函数，`function()` 从该父节点取得所属函数，不再重复保存 Owner；同时保存 `Name ParameterName`（通过 `name()` 访问）、零起始索引、值类型和 `ParameterKind`（Positional、Named、Variadic），通过 `parameterKind()` 查询。`IRBuilder::createFunction()` 的可选种类列表必须与签名槽位数量一致，省略时全部为 Positional；第四个可选参数 `ParameterNames` 按签名顺序提供名称，省略时参数匿名，由调用方驻留并填写形参名；种类是绑定元数据，不改变规范化运行时签名或开启变参展开。`createAddInstruction()` 接受同型整数操作数，定义按位宽回绕的加法；`IRBuilder::createDetachedReturnInstruction(ReturnedValue)` 创建未挂接的 void 类型终结节点，只校验操作数归属和非 void 类型；`IRBuilder::appendValue()` 校验目标块属于函数且返回值匹配该函数签名。返回指令不保存 Owner，`function()` 沿 outer → BasicBlock → Function 查询，未挂接时返回空指针。

- 对象模型头文件位于 `src/include/ink/ir`，实现位于 `src/lib/ir`；声明基类及其派生类放在 `ir/decl` 子目录，函数相关的 `Function`、`FunctionType`、`BasicBlock` 放在 `ir/function` 子目录，模块值 `Module` 放在 `ir/module` 子目录，`Name` 和 `NamePool` 放在 `ir/name` 子目录，其余类型及类型注册表放在 `ir/type` 子目录，按需包含对应的独立头文件。
- `src/include/ink/ir/coredefines.h` 集中定义 `ValueKind`、`TypeKind`、`ParameterKind`、`CallingConvention`、`LanguageLinkage`、`AccessKind` 和 `VisibilityKind`，仅依赖 `<cstdint>` 与枚举注册表，可独立包含。`VisibilityKind::Public/Private` 表示声明或成员的可见性，具体访问检查尚未接入。
- `Function` 保存独立的调用约定 `CallingConvention::C/Fast/Cold` 和语言链接规则 `LanguageLinkage::Ink/C`，分别通过 `callingConvention()`、`languageLinkage()` 查询。`IRBuilder::createFunction()` 的第五、六个可选参数设置它们，默认是目标平台 C 调用约定与 Ink 语言链接；无效枚举值返回空指针。两者不参与 `FunctionType` 的规范化身份，也不决定函数是否有定义；添加函数体会保留这些属性。Parser 已支持 `extern "名称" func f(): void;` 及带函数体的形式，并在 AST 中保留任意链接字符串；Analyzer 将普通函数和 `extern "C"` 函数分别映射为 Ink 与 C 语言链接，其他链接字符串报告不支持。符号命名和后端 ABI lowering 尚未接入。语言链接不表示 external/internal 符号链接性或导出可见性。
- `src/include/ink/ir/Values.def` 集中定义 `ValueKind`，枚举项与 C++ 类同名，如 `IntegerType`、`FunctionType` 和 `CallInstruction`；类型和常量条目分别生成 `Type::classof()`、`Constant::classof()`。元类型、void、bool、label、module 的实际对象均为 `BuiltinType`，由 `TypeKind` 继续区分；仅作为中间基类的 `Type`、`UserDefinedType`、`Constant` 不单独占用值种类。
- `src/include/ink/ir/type/Types.def` 用 `INK_IR_TYPE(Name, Base)` 集中登记类型种类，`Base` 配置为 `BuiltinType` 或 `UserDefinedType`；`TypeKind` 与两个基类的 `classof()` 分类判断均由该表生成。
- `src/include/ink/ir/instruction` 保存直接继承 `Value` 的 `CallInstruction`、`AllocaInstruction`、`LoadInstruction` 和 `StoreInstruction`，每种指令有自己的头文件，使用同名 `ValueKind` 分类，由 `IRBuilder` 创建，成功插入后由所属基本块拥有。
- `CStringInstruction` 表示把字符串常量复制到目标程序的独立可写 `u8` 缓冲区，包含终止 NUL，存活到所属函数返回。`createDetachedCStringInstruction()` 和 `createCStringInstruction()` 拒绝外来常量及内嵌 NUL；该节点只能插入函数块。execution 使用所属调用帧管理的 `ExecutionBuffer` 实现复制和失效，不把宿主 `const char *` 地址写入 IR；目标代码的分配与复制仍需后端 lowering。
- `IRContext` 位于 `src/include/ink/ir/context.h`，借用 `core::CompilationContext`，拥有 `NamePool`、`TypePool`、`ConstantPool` 和根模块；声明树由各 Module 拥有；函数、基本块和指令由所属 IR 父节点拥有。共享对象在上下文存活期间保持地址稳定。`SemanticContext` 位于 `src/include/ink/semantic/context.h`，组合 `IRContext` 与 `ScopeStore`，通过 `irContext()` 访问独立 IR 存储，并转发常用池访问器。程序化 IR 构建可以直接使用 `IRBuilder(IRContext &)`，无需语义分析器。
- `typePool()` 返回当前上下文唯一的类型池，位于 `ir/type/type_pool.h`；所有内建类型、结构类型、函数类型和名义类型的创建与所有权均由池负责，类型获取统一通过 `Context.typePool().getType<TypeKind::...>(...)`，`IRBuilder` 上的名义类型创建工厂转发到同一池。结构类型按完整结构去重，名义类型每次创建独立身份；类型对象的 `context()` 指向所属 `IRContext`。池只由上下文创建，不可复制、移动或清空；元类型先于其他类型初始化，模块、常量和声明先于类型池销毁，名称池最后销毁。声明由 Module 的独立声明树管理，AST 仍由 ParsedUnit 拥有，不放入类型池。
- `constantPool()` 返回当前上下文唯一的常量池，负责 bool、任意位宽整数、`StringConstant` 和 `FloatConstant` 的创建、所有权与去重；常量获取统一通过 `Context.constantPool().getXXXConstant(...)`，Context 不提供转发接口。规范类型身份和完整 payload 决定常量身份，哈希命中后仍精确比较，类型来自其他上下文或 payload 与类型不匹配时返回空指针且不插入。`owns()` 检查具体对象的池归属，`size()` 包含创建时已有的 false、true 两个常量；池只由所属上下文创建，不可复制、移动或清空。
- `ConstantPool::getStringConstant(SliceType, Payload)` 要求只读 `u8` 切片类型，复制并驻留调用方已经验证、解码的 UTF-8 字节，支持空串和内嵌 NUL，不重复解码转义或执行 Unicode 规范化；相同解码字节只保存一个常量。字符串存储以额外 NUL 结尾，`value()` 的长度不含该终止符，`nullTerminatedValue()` 提供包含终止符的完整存储视图；`tryGetCString()` 对无内嵌 NUL 的内容零拷贝返回 `const char *`，否则返回 `nullptr`，避免静默截断。该接口是宿主常量访问；源码字符串常量在 C 调用实参位置通过 CStringInstruction 创建独立可写副本，存活到调用者函数返回，后端 lowering 尚未接入。`ConstantPool::getFloatConstant(FloatType, FloatBits)` 使用项目自有的 IEEE binary16/32/64 位表示，要求位宽与类型完全一致且没有多余高位；按位去重，区分正负零、NaN 符号和 payload，不隐式转换或舍入。常量及字符串视图在上下文存活期间保持有效；整数和字符串源码字面量已接入语义分析，浮点字面量及后端 lowering 仍待实现。
- `Name` 只有一个 32 位索引，默认无效。`namePool().intern(Text)` 为相同字节串复用索引，`find(Text)` 不插入，`text(Name)` 返回池拥有的稳定视图。名称只在所属池内比较；调用方负责携带池或上下文，不能将一个池的索引交给另一个池解释。池不执行词法验证或 Unicode 规范化，前端名称应来自已经验证的 token。
- `Type`、`Constant`、`Function`、`BasicBlock`、`Module`、`CallInstruction`、`AllocaInstruction`、`LoadInstruction` 和 `StoreInstruction` 都继承 `Value`，`Decl` 独立于该层次。`Type` 分为 `BuiltinType` 与 `UserDefinedType`：前者包括元类型、void、bool、整数、IEEE 16/32/64 位浮点、定长数组、切片、指针、引用和函数类型；后者包括以独立对象身份区分的 `ClassType`、`EnumType`、`InterfaceType`。类型值的类型为元类型，元类型的类型为自身。当前常量包括 bool、整数、字符串和浮点；整数 payload 使用自有 `IntegerBits`（位宽与低位字在前的 `uint64_t` 字数组），浮点使用自有 `FloatBits`（IEEE 位宽与 `uint64_t` 原始位模式），字符串拥有完整的解码字节。常量池拒绝无效位宽、错误字数和多余高位，不隐式截断或扩展。
- `Value` 统一保存所属 `IRContext` 和自身类型 `const Type &ValueType`，派生类通过继承的 `context()`、非虚 `type()` 查询。类型在构造时确定，派生类不重复保存自身类型，也不沿操作数链递归推导；元类型的自身类型指向自己。`Function::functionType()` 提供函数签名访问，数组元素类型、函数返回类型等类型结构成员仍由具体类型保存。
- `Value::outer()` 返回非拥有的结构父节点指针；父节点通过 `unique_ptr` 拥有子节点。IRContext 拥有共享类型、常量和根 Module，SemanticContext 另持有作用域存储，Module 分别拥有声明树根和 IR 入口块，Function 拥有参数和函数块，BasicBlock 拥有块内节点。未挂载节点由调用方的 `unique_ptr` 拥有；调用目标、实参、类型和解析器绑定均为借用引用，不参与所有权。Context 必须比所有借用它的节点活得更久，节点析构通过 IR 的 LifetimeObserver 通知 ScopeStore 清理名字绑定；删除节点前调用方仍须处理操作数和 Builder 插入点；目前没有 use-def 自动修复。
- `Value`、`Type`、`BuiltinType`、`UserDefinedType`、`Constant` 和 `Decl` 的基类构造函数使用 `protected`，字段保持 `private`；派生类无需逐个列入基类友元名单。函数、参数、基本块、模块、声明和指令的私有构造函数授权 `IRBuilder`，具体类型和常量分别授权 `TypePool`、`ConstantPool`。IRBuilder 负责节点创建、所有权转移和父子关系维护，并独占 Value、BasicBlock 等节点的结构写权限；Context 只持有共享存储和根模块。
- IR 的公共接口、实现和测试使用项目自有模型与标准库，独立目标 `ink_ir`（`ink::ir`）依赖 `ink::core` 与 `ink::parser`，因为 Decl 保留 AST 关联；`ink_semantic` 单向依赖 `ink::ir`。名称池采用拥有字符串键的标准容器，哈希仅用于内存查找，分类通过 `classof()` 完成；LLVM IR 类型转换属于后端适配层。
- `TypePool` 使用受约束的 `TypePool::getType<TypeKind::...>(...)` 模板重载获取类型。Meta、Void、Bool、Label、Module 无参数，返回 `const BuiltinType &` 并保留 `const noexcept`；参数化类型返回对应的具体类型指针，运行时参数无效时返回空指针。错误的种类与参数组合在编译期拒绝；名义类型仍通过 `IRBuilder::createClassType`、`IRBuilder::createEnumType`、`IRBuilder::createInterfaceType` 创建独立身份。
- `TypePool::getType<TypeKind::Array>(ElementType, ElementCount)` 按元素类型和 64 位定长长度复用数组类型；多维数组通过嵌套构造。`TypePool::getType<TypeKind::Slice>(TargetType, Access)`、`TypePool::getType<TypeKind::Pointer>(TargetType, Access)`、`TypePool::getType<TypeKind::Reference>(TargetType, Access)` 按目标类型与 `AccessKind::ReadOnly/ReadWrite` 复用类型。访问权限表示能否通过该值修改目标，与变量绑定本身的可变性分开；引用类型也与表达式的值/位置类别分开。这些类型构造器本身属于 builtin，即使目标是用户类。
- `TypePool::getType<TypeKind::Function>(ReturnType, ParameterTypes)` 按返回类型和有序形参类型列表复用固定参数签名，支持零参数与 void 返回值；`FunctionType` 拥有列表存储，所有组成类型必须属于当前上下文，空形参指针被拒绝。参数名称、默认值、参数包和泛型绑定由后续分析负责。
- `Decl` 是 Module 拥有的独立声明树节点，保存名称、借用的只读 AST、所属模块和非拥有的父声明指针；`module()`、`parent()` 查询归属，模块声明根的父声明为空。节点使用 `std::vector<std::unique_ptr<Decl>> Children` 拥有子声明，`children()` 返回只读容器，元素通过 `.get()` 借用。`IRBuilder::createModuleDecl(Module &, ModuleAST)` 为本地模块设置唯一声明根并使用模块名称，重复设置或外来模块返回空指针；`createFunctionDecl(Decl &Parent, Name, AST)`、`createClassDecl(Decl &Parent, Name, AST)` 创建后立即挂入本地父声明，要求有效名称和泛型 AST，失败不改变树。派生类不增加字段，基类 `ast()` 返回 `ASTNodeBase`，派生类返回具体 AST，`classof()` 从 AST 分类。声明不属于 `Value`，不保存已解析类型、初始化值或实例状态，IR 声明树暂不提供 `VarDecl`。AST 所属 `ParsedUnit` 必须比声明所属 Module 活得更久。
- `IRBuilder` 提供名义类型、声明、模块、函数、基本块及指令的 `createXXX` 入口；`IRContext` 持有共享存储、Pool 访问器和根模块查询，`SemanticContext::irContext()` 提供语义分析所用的 IR 上下文；所有权编辑及插入点校验统一由 IRBuilder 负责。函数和无参基本块工厂返回未挂载的 `unique_ptr`，根模块交给 Context 持有，声明根交给 Module 持有，子声明交给父声明持有，名义类型交给 TypePool 持有；这些创建操作均不使用或改变指令插入点。
- `ink::ir::IRBuilder`（`ink/ir/ir_builder.h`）的 `createCallInstruction`、`createAllocaInstruction`、`createLoadInstruction`、`createStoreInstruction`、`createAddInstruction` 和 `createReturnInstruction` 按插入点创建指令，成功后将所有权移交给 BasicBlock 并返回借用指针。先调用 `setInsertPoint(Block)` 选择块末尾，或用 `setInsertPoint(Before)` / `setInsertPoint(Block, Before)` 选择锚点；连续创建保持调用顺序，Alloca 不自动移动到入口块。对应的 `createDetachedXXXInstruction` 工厂返回未挂载的 `unique_ptr`，忽略且不改变插入点，由调用方通过 IRBuilder 的显式编辑接口转移所有权。
- `IRBuilder::saveInsertPoint()` / `restoreInsertPoint()` 保存和恢复块及锚点指针，`InsertPointGuard` 通过 RAII 恢复临时切换前的位置；没有插入点时也可保存和恢复。多个 Builder 的位置相互独立，vector 扩容和所有权转移不改变节点地址。设置或恢复外来块、已摘除或属于其他块的锚点时返回 false 并清空位置，守卫恢复失效位置时同样清空。没有有效位置、操作数不合法或向 return 后插入时，自动插入指令工厂返回空指针且不插入节点。恢复已结束块的末尾位置本身允许，但后续创建会拒绝。保存的位置不延长节点寿命，销毁目标块或锚点前应清理相关位置和守卫；不支持并发修改同一上下文。
- `IRBuilder::createFunction(Name, Signature)` 返回拥有独立身份和本地 `FunctionType` 的 `std::unique_ptr<Function>`，初始未挂载，供普通函数或已闭合的泛型实例使用。`IRBuilder::createCallInstruction(Callee, Arguments)` 直接调用 `Function`，或间接调用其他类型为 `FunctionType` 的值；泛型 `FunctionDecl` 需要先实例化，不能直接作为调用目标。实参须已完成排序和转换，数量和逐项类型必须精确匹配。指令拥有实参引用列表，每次调用均创建独立对象；创建不执行函数，不进行重载解析或泛型实例化。
- `Function` 使用 `std::vector<std::unique_ptr<BasicBlock>>` 拥有函数块，使用 `std::vector<std::unique_ptr<FunctionParameter>>` 拥有参数；`blocks()`、`parameters()` 返回只读容器，元素通过 `.get()` 借用。空块列表表示没有函数体，`entryBlock()` 返回首块的借用指针。`IRBuilder::createBasicBlock(Function &)` 创建并直接交给函数拥有，设置 `Outer` 后返回借用指针；首块自动成为入口。`IRBuilder::createFunctionBody(Function &)` 仅创建首块，已有函数体或外来函数返回空指针。函数销毁时先销毁函数块，再销毁参数；列表顺序不代表执行顺序，块内指令通过 `IRBuilder` 构建。源码直线返回检查已实现；跳转节点、控制流连接和完整返回路径检查仍待实现。
- `BasicBlock` 使用 `std::vector<std::unique_ptr<Value>>` 拥有有序子节点，`values()` 返回只读容器。`IRBuilder::createBasicBlock()` 返回未挂载的 `std::unique_ptr<BasicBlock>`。`IRBuilder::insertValue(Block, std::move(Child), Before)` 在指定直接子节点前转移所有权，Before 为空时追加；`IRBuilder::appendValue(Block, std::move(Child))` 是末尾追加入口。这些显式编辑接口使用参数指定的块和锚点，不读取或改变 Builder 当前的插入点。成功清空调用方的 owner 并设置 `Outer`，失败保留 owner 与原块内容；拒绝空 owner、外来对象、非法锚点、已有父节点、循环包含、类型和常量。return 只能位于所属函数块的末尾且匹配返回签名，已有 return 后不能追加节点。`IRBuilder::removeValue(Block, Child)` 摘除直接子节点、清空其 `Outer` 并返回 `std::unique_ptr<Value>`；失败返回空 owner。`IRBuilder::eraseValue(Block, Child)` 则立即销毁该子树。摘除后的节点可重新挂载，父节点销毁会递归释放仍属于它的子树。
- `Module` 使用 `std::unique_ptr<ModuleDecl>` 和 `std::unique_ptr<BasicBlock>` 分别拥有独立的声明树与 IR 入口块，销毁时先释放 IR，再释放声明树。`declarationRoot()` 返回借用指针，尚未创建声明根的程序化模块返回空指针。`IRBuilder::createModule(Name)` 校验名称后创建 Context 拥有的根模块，返回借用指针；`modules()` 提供根模块的只读拥有容器，`entryBlock()` 借用入口块，入口块的 `Outer` 指向 Module。`IRBuilder::removeModule(Module)` 从根列表摘除并返回 `unique_ptr<Module>`，随后可交给某个 BasicBlock 实现嵌套；`IRBuilder::appendModule(std::move(Owner))` 将未挂载 Module 重新交给 Context，兼容 `IRBuilder::removeValue()` 返回的基类 owner，失败不消费所有权。`IRBuilder::eraseModule(Module)` 销毁根模块及其两棵子树；模块所有权转移不改变声明归属或节点地址。模块仍使用 `TypePool::getType<TypeKind::Module>()`，与借用文件 AST 的 ModuleDecl 分开。
- `IRBuilder::createClassType(Name)`、`IRBuilder::createEnumType(Name)`、`IRBuilder::createInterfaceType(Name)` 直接创建具有独立身份的名义类型，不创建语义 `Decl`。同名类型不合并，分析器负责复用同一普通类型或泛型实例的返回对象；成员、继承、枚举底层类型、布局和实例缓存仍待实现。组合类型工厂仅检查结构约束；元素合法性、数组布局大小和引用使用规则尚未由分析器检查。
- `createAllocaInstruction(AllocatedType)` 创建单对象、未初始化的分配指令，结果为对应的可读写指针；数组通过 `ArrayType` 表示。当前支持 bool、整数、浮点、指针、引用、切片及这些类型组成的定长数组，拒绝外来类型、元类型、void、label、module、原始函数签名和布局未完成的名义类型。`createLoadInstruction(Address)` 从指针读取并产生元素类型的值，接受只读或可读写指针；`createStoreInstruction(Address, StoredValue)` 要求可读写指针、同一上下文和完全一致的元素类型，结果类型为 void。对象存活期间地址在容器扩容和所有权转移后保持稳定，操作数仅被借用，每次创建均有独立身份，调用者通过 `IRBuilder` 的插入点安排执行顺序；构建节点不会执行内存操作，也不检查初始化、支配关系或实际地址有效性。
- 执行模型不再保存源码变量绑定对象。源码变量和形参与模型对象的绑定、const 分析仍待实现。初始化和赋值由各自位置的 `StoreInstruction` 表示，读取使用 `LoadInstruction`；`StoreInstruction` 在构造时将自身类型设置为上下文唯一的 void 类型，由 `Value::type()` 返回。
- 空名称或名称索引空间耗尽返回无效 `Name`；无效或越界索引查询返回空视图。整数工厂拒绝零位宽，浮点工厂拒绝 16/32/64 以外的位宽；组合类型工厂拒绝外来类型和无效访问权限。命名对象工厂拒绝无效或越界名称，内存指令工厂另检查地址、访问权限和操作数类型，泛型声明工厂另检查 AST 的泛型参数。上述检查返回显式状态，不依赖异常。
