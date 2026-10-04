# ink

当前按 [`docs/Ink-Lexical-Rules.md`](docs/Ink-Lexical-Rules.md)、[`docs/Ink-grammar-Rules.bnf`](docs/Ink-grammar-Rules.bnf) 和 [`docs/Ink-Parser-Design.md`](docs/Ink-Parser-Design.md) 构建前端，包含 Core、CLI、tokenizer、parser、独立 IR、execution、semantic、LLVM backend 和测试。源码支持普通函数、原生导入导出、bool 分支与短路、整数比较、定长数组，以及值语义 class、字段、`this` 方法和 Python 风格运算符方法。解释执行、字节码及 LLVM AOT 共用语义分析后的 IR 和 Ink 对象布局；`inkc --interpret` 执行源码，`--emit-llvm` 输出 LLVM IR，`--emit-object` 输出宿主目标文件。LLVM lowering 位于 `src/lib/backend/llvm`，测试按模块位于 `src/testcase`。

Class 的复制、默认初始化、访问权限、方法接收者和运算符协议详见 [Class 与对象语义](docs/Ink-Classes.md)。AOT 目标文件需链接 `ink_aot_runtime` 和系统 C 运行库；命令及当前边界见 [命令行接口](docs/command-line.md)。

Core 的 `PANIC(Message)` 宏通过独立的 spdlog stderr logger 输出消息和调用位置、同步刷新后调用 `abort()`，不依赖全局日志开关。`DiagnosticEngine::report` 遇到 ICE 会立即输出诊断编号和格式化消息并 panic，先于消费者分发；普通用户错误仍正常分发并返回。无效 AST 归档、资源上限等现有 ICE 同样遵循此规则。

## 编辑器支持

CLion / IntelliJ IDEA 可导入 [`editors/textmate/Ink.tmbundle`](editors/textmate/Ink.tmbundle) 获得 `.ink` 语法高亮，并安装 Ink Live Templates 展开函数、分支、循环和 `comptime` 代码骨架。安装和使用步骤见 [`editors/README.md`](editors/README.md)，预览样例见 [`example.ink`](editors/textmate/example.ink)。使用这些配置不需要编译 Ink；语义补全和跳转定义尚未提供。

## 构建

Core 配置集中定义在 `src/include/ink/core/config.def`，每项包含枚举名、环境变量名和字符串默认值，并生成 `ink::core::ConfigKind` 枚举及以枚举为键的配置映射。`ink::core::ConfigManager::get<ink::core::ConfigKind::SemanticBlockDepthLimit>()` 直接返回 `std::string`，每次读取对应环境变量，未设置时返回定义中的默认值；配置项通过枚举模板参数选择，找不到枚举对应的配置时报告 `INK-C0001` 并立即 panic。`getSize<ink::core::ConfigKind::SemanticBlockDepthLimit>()` 直接返回 `std::size_t`，读取非负十进制整数，空值、非法格式或溢出的环境变量会回退到默认值；默认值也无法解析时报告 `INK-C0002` 并立即 panic。`INK_SEMANTIC_BLOCK_DEPTH_LIMIT` 控制语义分析的块嵌套上限，默认 `256`，`0` 表示不允许任何块；例如 PowerShell 中执行 `$env:INK_SEMANTIC_BLOCK_DEPTH_LIMIT = "128"` 可覆盖该上限。

其他资源限制也由 `config.def` 提供默认值，字节预算的环境变量使用十进制字节数：

| 环境变量 | 默认值 | 用途 |
| --- | --- | --- |
| `INK_SEMANTIC_TYPE_DEPTH_LIMIT` | `256` | 语义类型递归深度 |
| `INK_SEMANTIC_EXPRESSION_DEPTH_LIMIT` | `256` | 语义表达式递归深度 |
| `INK_EXECUTION_MAX_STEPS` | `100000` | 编译期执行步骤预算 |
| `INK_EXECUTION_MAX_OBJECTS` | `16384` | 执行期间累计存储分配预算（Cell/Buffer） |
| `INK_EXECUTION_MAX_STORAGE_BYTES` | `67108864`（64 MiB） | 执行期间累计可变存储字节预算（存储对象及缓冲区内容） |
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

Ink 跨 module 函数、成员函数、闭合实例、全局变量、Imported 符号和 `import "C"` 的链接名称规则见 [`docs/name-mangling.md`](docs/name-mangling.md)。

## inkc 解释执行

`inkc --interpret -i FILE` 读取源码，经 tokenizer、parser、semantic 生成 IR 后默认执行 `main`；可用 `--entry NAME` 指定其他入口，`--input` 是 `-i` 的别名，`FILE` 为 `-` 时读取标准输入。入口须为零参数、具有 IR 函数体的本地或导出函数，返回 `void` 或 `i32`：void 退出为 0，i32 作为进程退出码。诊断写入 stderr，源程序的 stdout 输出直接保留，不打印 IR。也可使用下面的字节码编译、链接和文件执行模式；`-oir` 仍只保留参数识别，当前未提供 IR 输出模式。

构建后在仓库根目录运行 hello world 或功能测试程序，均使用默认的 `main` 入口。Windows x64 / PowerShell：

```powershell
.\build\src\tools\inkc\Release\inkc.exe --interpret -i src/testcase/execution/programs/hello_world.windows.ink
.\build\src\tools\inkc\Release\inkc.exe --interpret -i src/testcase/execution/programs/logical_truth_tables.ink
ctest --test-dir build -C Release -R '^(ExecutionSourceTest[.].*|InkcProcessTest)$' --output-on-failure
```

Linux x86-64 / 单配置构建：

```sh
./build/src/tools/inkc/inkc --interpret -i src/testcase/execution/programs/hello_world.linux.ink
./build/src/tools/inkc/inkc --interpret -i src/testcase/execution/programs/logical_truth_tables.ink
ctest --test-dir build -R '^(ExecutionSourceTest[.].*|InkcProcessTest)$' --output-on-failure
```

两份程序分别按 Windows `_write` 和 Linux `write` 的 ABI 声明外部函数，main 写入 `hello, world\n` 后返回 0。`ExecutionSourceTest.HelloWorld` 与逻辑源码测试共用 `source_program_test.cmake`，以默认 `main` 启动真实 `inkc`，通过 `OUTPUT_FILE` 捕获 stdout；HelloWorld 额外使用 `STDOUT_HEX` 逐字节检查 Windows CRLF / Linux LF，并断言空 stderr 和退出码 0；`InkcProcessTest` 检查入口选择、调用错误及返回结果。完整参数与退出规则见 [命令行接口](docs/command-line.md)。

`src/testcase/execution/programs` 中的功能测试均可通过默认 `main` 独立运行，覆盖整数算术、普通函数与递归、C 外部调用、逻辑/比较、分支与局部作用域、编译期求值和静态循环。每项先输出用例名，核对结果后输出 `PASS`；失败输出 `FAIL` 并返回 1，全部成功返回 0。CTest 核对完整 stdout，包括副作用的顺序和次数，同时要求退出码 0、空 stderr。[源码测试清单](src/testcase/execution/cli/source_program_tests.cmake) 统一登记这些程序和 `src/testcase/execution/cli/inputs` 中的负向输入，后者分别核对类型、调用、初始化和执行错误诊断。

算术用例区分已实现的执行路径：普通 IR 验证所有 i/u8、16、32、64、128 的加法及回绕；减、乘、除、取模、位运算、移位、复合赋值和增减在显式 `comptime` 表达式或块中验证。C 外调测试使用 Windows/Linux 的宿主 CRT；声明 `size_t` 为 `u64` 的字符串、指针和副作用程序仅在 64 位宿主注册。可用 `ctest --test-dir build -C Release -L arithmetic --output-on-failure` 按标签运行，也可选 `function`、`external`、`logical`、`control_flow` 或 `comptime`。完整覆盖与添加用例的规则见 [源码执行测试](docs/command-line.md#cli-与源码执行测试)。

定长数组使用 `[T; N]` 类型、`[A, B]` 或 `[Value; N]` 构造，支持下标读写、元素取地址、按值传参和返回、编译期数组及边界检查。可将初始化的 `u8` 数组通过 `&Buffer[0]` 传给 Windows `_read` 或 Linux `read`；语法、值语义、ABI 声明和测试入口见 [数组与下标访问](docs/Ink-Arrays.md)。

## 字节码文件与多文件链接

`--emit-bytecode` 将一个源文件的全部普通函数编译成可重定位对象，`--link-bytecode` 合并独立对象并选择入口，`--run-bytecode` 在新进程加载执行最终镜像。输出扩展名可自选，下例使用 `.inkobj` 和 `.inkbc`：

```text
inkc --emit-bytecode math.inkobj --module-root . -i math.ink
inkc --emit-bytecode app.inkobj --module-root . -i app.ink
inkc --link-bytecode math.inkobj --link-bytecode app.inkobj --entry app#main -o program.inkbc
inkc --run-bytecode -i program.inkbc
```

例如 `math.ink` 定义 `func answer(): i32 { return 42; }`，`app.ink` 使用 `from math import answer;` 并定义 `func main(): i32 { return answer(); }`。最后一步返回 42。加载最终镜像不需要源码或输入对象文件，也不重新运行语义分析。

顶层函数默认 `public`，显式 `private` 的函数只在定义文件内可访问；函数内的局部定义始终私有。Ink 模块间的名字访问必须通过 `from math import answer as localAnswer;` 或 `import math as library;` 后调用 `library.answer()`，签名和可见性从目标源码读取。普通 Ink 函数必须有函数体，只有 `import "C" func ...;` 可以只有声明。编译参数和 CMake 不提供可见性或导入映射覆盖。

原生边界使用 `import "C" func ...;` 与 `export "C" func ... { ... }`；`[abi("C")] func ... { ... }` 设置本地定义的 C ABI，允许按参数列表重载。原生导入导出使用声明原名，不能形成同名重载；`public/private` 只控制 Ink 源码访问。`private export` 仍可被当前程序内同名同签名的原生导入匹配，但不能通过 Ink 模块导入访问。原生导入本身不会搜索或加载其他 Ink 源文件，导出定义必须已纳入分析或对象链接。尚未生成原生 DLL/SO，也不提供可传给 C 的本地回调地址；`link` 属性暂不支持，旧 `extern` 语法已移除。

`--module-root` 指定源码搜索根，默认输入文件所在目录；模块身份由相对路径去掉 `.ink` 后用点连接，例如 `package/math.ink` 对应 `package.math`。各对象应使用同一个源码根编译；`from .math import answer;` 支持包内相对导入。生成对象或使用导入时，源码扩展必须是 `.ink`，相对根目录的各路径组件在移除扩展后不能包含字面量点号，例如使用 `pkg/value.ink`，拒绝 `pkg.value.ink`，以保证模块名与路径一一对应。不含导入的单文件 `--interpret` 使用虚拟模块名 `main`，不从文件名推导模块身份，仍接受 `points.windows.ink` 等任意文件名。`--entry module#function` 选择链接入口。当前源码导入支持普通函数、编译期函数及 class；跨模块编译期调用按需分析所依赖的函数体，循环依赖尚未完成的函数体时报告用户错误。泛型实例化仍由前端另行实现；字节码构建 API 已支持闭合泛型实例身份。

v6 文件限定相同宿主目标 ABI，保存代码、常量和符号，不保存运行中的堆、调用栈、宿主指针或原生调用缓存。完整协议、API 和验证规则见 [字节码文件格式与链接](docs/Ink-Bytecode-Format.md)，命令行细节见 [命令行接口](docs/command-line.md)。`ExecutionMultiFileTest` 以 [`execution/multifile`](src/testcase/execution/multifile) 中的多份真实源码为主要集成用例，分别编译各模块、链接、移走源码和对象文件后加载执行，并覆盖导入、可见性和相互调用。`BytecodeProcessTest` 补充 CLI 模式与参数边界；底层归档、验证和链接测试位于 `src/testcase/execution/artifact`。

## Tokenizer 接口

- `tokenize(FrontendContext &, std::string)` 或 `tokenizeSource(FrontendContext &, SourceId)` 返回持有源码和解码值的 `TokenizedBuffer`。先检查 `succeeded()`；成功结果以唯一的 `END_OF_FILE` 结束。全局 UTF-8／NUL／BOM 校验失败不输出 token，其他词法错误保留此前完成的 token，不追加 EOF。
- `TokenKind` 为扁平枚举，C++ 枚举项按仓库约定使用 UpperCamelCase，`tokenKindName()` 返回文档中的大写名称。空白、注释和错误不占 token 种类。标识符的 NFC 名称、字符串的解码 UTF-8 值和字符的 Unicode 标量值分别保存在 payload 中。
- Core 的 `SourceLocation` 使用 32 位不透明编码，默认无效，`fromByteOffset(0)` 表示有效的文件起点。`SourceRange` 始终为原始字节的半开区间，可通过 `getBegin()`／`getEnd()` 获取位置。文件身份由源码 buffer 或诊断的 `Source` 保存；不同文件的位置不可直接比较。超过 `0xFFFFFFFE` 字节的输入显式报 `SourceTooLarge`。
- Unicode 正式名称复用 LLVM 的公开名称查询接口；完整别名由官方 Unicode 15.1 `NameAliases.txt` 生成。更新方式见 `cmake/generate_unicode_aliases.py`，生成表记录原始数据的 SHA-256，XID 和 NFC 数据不复制到本项目。

## Parser 接口

- `ink::parser::parse(FrontendContext &, TokenizedBuffer, ParseLimits)` 返回 `ParseResult`。`Unit` 持有不可变 TokenBuffer、ASTContext、ModuleAST 和恢复记录；节点和源码引用在 Unit 销毁前有效。`succeeded()` 同时检查词法结果、当前调用的语法错误和解析状态。
- `Completed` 表示扫描结束，错误输入仍能返回恢复后的 AST。`Cancelled` 明确标识取消时的部分结果。ParseLimits 控制嵌套、诊断、工作量与 AST 分配；嵌套、工作量或分配预算耗尽会输出 ICE 并立即 panic，不再返回部分 AST。ICE 不受诊断数量上限或诊断事务影响。
- `ASTVisitor`／`ConstASTVisitor` 只分派当前节点，`StrictExprVisitor` 要求覆盖全部表达式。`ASTWalker` 使用显式栈按源码顺序遍历，支持跳过子节点和提前停止。`verifyAST` 校验结构契约；`dumpAST` 返回字符串，不直接产生进程输出。
- AST 二进制快照当前为 V5，文本快照为 `ast 4`；函数保存独立的原生导入导出方向和 ABI 字符串。IR 模块的二进制与文本归档当前均为 V6，字节码对象与镜像容器为 v6（指令集版本 3）。各格式严格检查版本，旧版本不自动迁移。类型描述、VM 与 AOT 宿主反射接口见 [类型描述与反射](docs/Ink-Reflection.md)。
- `ink-parse INPUT` 或通过标准输入运行 `ink-parse -` 可查看结构与恢复记录。正常退出为 0，词法或语法错误为 1，输入读取失败为 2。
- 测试包含文法家族、恢复边界、Arena 析构与回滚、Visitor、源码生命周期、长链、资源预算、Token 边界扰动及确定性随机输入。可运行 `cmake --build build --config Release --target run_all_tests` 执行完整回归。

## Semantic 分析接口

`Analyzer` 的头文件和实现放在 semantic 的 `analyzer` 子目录；名字解析放在 `name_resolve` 子目录，拆分为 `binding.h`、`scope.h`、`name_resolver.h` 和 `name_resolver.cpp`，三个类型均位于 `ink::semantic` 命名空间。

`Analyzer::analyze(SemanticContext &, const parser::ParseResult &, std::string_view ModuleName)` 为成员函数，当前支持空模块、块作用域、普通定参 Ink 函数定义、`import "C"` 声明、`export "C"` 定义和 `[abi("C")]` 本地定义，包括基础标量/指针/引用/定长数组签名、形参绑定和函数体遍历；普通 Ink 无体声明报告用户错误。`analyze()` API 保留单模块源码顺序作为兼容入口；CLI 统一使用 `analyzeModules()`，无导入的单文件也先预声明顶层函数签名，以保持独立编译与作为导入依赖时的结果一致。`analyzeModules()` 先预声明各模块的顶层函数签名，再解析真实源码导入和分析函数体，支持跨文件调用及相互递归；用户错误报告诊断并返回 `nullptr`，未支持的语义报告 ICE 并终止。`SemanticContext::scopeStore()` 持有作用域、绑定及成员/定义作用域索引；每次分析由 `AnalysisState` 拥有独立 `NameResolver`，通过 `enterScope()` 和 `exitScope()` 管理当前位置，并可从已有 `Scope` 恢复查找。resolver 销毁后绑定仍然保留，支持 `Value *` 和泛型 `Decl *`；`lookup()` 查找当前及父作用域，`lookupLocal()` 仅查当前作用域。`enterScope(Owner)` 为实体创建成员作用域，`lookupMember(Owner, Name)` 查找该实体的直接成员，别名共享同一实体的成员绑定。普通函数支持按参数类型形成重载集合及选择、定参调用、bool 条件分支、显式 return、分支返回路径与确定初始化检查和 void 隐式返回；C 调用中的字符串常量可通过独立可写副本传给 *u8。源码循环、泛型源码、C 变参及重复声明合并仍待实现。接口、生命周期和具体支持范围见 [语义分析接口](docs/Ink-Semantic-Analysis.md)。

## Execution 执行接口

execution 的公共头与实现分别位于 `src/include/ink/execution` 和 `src/lib/execution`，按相同的功能目录组织：

| 子目录 | 职责 |
| --- | --- |
| `bytecode/` | 指令注册表、连续执行表示、槽位分配、跳转定位与验证 |
| `runtime/` | 不含 IR 引用的类型/签名 ID、存储布局与值载荷 |
| `bridge/` | 语义类型、常量、函数与运行时表示之间的转换 |
| `engine/` | 执行引擎、语义帧、字节码虚拟机、内存访问与运算调度 |
| `memory/` | Heap、Cell/Buffer 连续存储、内部存储句柄与裸指针 |
| `value/` | 值基类、值引用与结果、七种不可变值子类、精确位宽整数运算 |
| `support/` | 执行对象基类、公共状态与结果、状态名称转换 |
| `ffi/` | 外部调用、参数封送、ABI 类型映射、宿主符号查找与缓存 |

七种值子类各有独立的 `value/execution_*_value.h` 和 `.cpp`，`ExecutionValueRef`、`ExecutionValueResult` 仍与基类位于 `value/execution_value.h`；`ExecutionInstructionResult` 保留为 `support/` 中的独立动作结果类型，字节码虚拟机直接调度自己的指令和调用帧。测试位于 `src/testcase/execution` 的 `bytecode/`、`engine/`、`value/`、`memory/`、`support/`、`ffi/`、`artifact/` 和 `cli/`；单文件源码样例保留在 `programs/`，多模块源码与编译链接清单位于 `multifile/`。

`ink::execution` 模块提供 `ExecutionEngine`、模块/分析/调用帧和可变对象，并已接入 Analyzer。支持整数/bool 编译期变量、表达式和块、赋值、函数调用、短路及编译期条件/循环；单模块 `analyze()` 按源码顺序处理；多模块入口预声明函数签名，编译期调用仍要求实际依赖的函数体已完成分析。表达式按实际控制流直接求值，变量修改只影响后续求值。初始化结果存入对象，后续读取取得对象当前值；每轮循环和每次实际调用使用独立帧执行。运行时局部变量可用编译期结果初始化，但仍不能在编译期读取。接口与边界见 [语义分析接口](docs/Ink-Semantic-Analysis.md) 和 [执行设计](docs/Ink-Semantic-Design.md#65-当前最小执行模块的边界)。

[`ExecutionObject`](src/include/ink/execution/support/execution_object.h) 是执行对象基类；抽象的 [`ExecutionValue`](src/include/ink/execution/value/execution_value.h) 派生出不可变的 void、bool、整数、浮点位模式、字符串、指针和函数值。`ExecutionValueRef` 通过 RAII 引用计数共享只读载荷，复制结果不复制整数和字符串内容；标量和字符串结果可以在创建它们的引擎销毁后继续存在，但所借用的 IR 类型仍须有效，函数值也要求引用的 IR 函数保持有效。这套机制不使用 GC。

[`ExecutionHeap`](src/include/ink/execution/memory/execution_heap.h) 提供值工厂和统一存储入口，所有 `ExecutionCell` 与 `ExecutionBuffer` 的分配、释放和预算由 [`ExecutionMemoryManager`](src/include/ink/execution/memory/execution_memory_manager.h) 管理。可寻址标量、宽整数、指针、数组和 class 使用连续内存，VM 与 AOT 共用大小、对齐、字段偏移和数组步长规则。`load()` 从当前内存生成不可变快照，`store()` 写回原地址，C 写入也由后续 load 读取。Buffer 保存固定字节数组，`allocateBuffer(Size)` 分配零初始化缓冲。帧结束或显式 `release()` 真正释放存储。内部 `ExecutionPlace` 与 `ExecutionStorageRef` 保留代次用于分配清理；Ink 的 `ExecutionPointer` 仅保存一个裸地址，不携带身份或所有权。累计分配次数和字节预算不会随释放返还；预算覆盖存储对象、连续字节缓冲和聚合元数据，不等于整个进程或不可变快照的内存用量。

源码支持对本函数已初始化的可变局部变量取地址 `&Value`，以及 `*Pointer`、`*Pointer = Value` 和 `&*Pointer`；括号不改变可寻址性，指针操作数只求值一次。临时值、const 变量、直接形参和直接 AST 编译期取地址报告诊断；指针形参仍可解引用，const 指针绑定不限制其指向的可变对象。普通函数和编译期调用的函数体均通过相同 IR 内存操作执行，指针本身不能冻结为 IR 常量。当前局部存储的运行时生命周期延续到所属函数返回；地址不包含生命周期信息，访问悬空指针属于未定义行为。示例见 [`address_of.ink`](src/testcase/execution/programs/address_of.ink)。

`ExecutionEngine::execute(Function, span<const ExecutionValueRef>)` 通过 `ExecutionCompiler` 将普通 IR 函数降低为经过验证的 `ExecutableFunction`，再由 `ExecutionMachine` 执行并缓存。现有 `Alloca`、`Store`、`Load`、整数 `Add`、bool 逻辑、`Compare`、`CString`、`Call`、分支和返回均转换为字节码；函数声明记录已确定的函数身份，实际调用仍由调用指令执行。参数和 SSA 值预先分配固定槽号，每次调用有独立槽位及存储生命周期；重复读取调用结果不会重复副作用。分支目标预先转换为 PC，只执行实际选中的路径，缺少终结指令的块显式失败，不依赖块的存储顺序。编译期调用和普通执行统一使用 `execute()`，返回独立的执行值；语义层仅在编译期结果边界转换为可表示的常量。`allocate()`、`load()`、`store()` 负责常量边界的转换，共用 `allocateValue()`、`loadValue()`、`storeValue()` 的存储实现，运行中间值不驻留到常量池。Class 的运行时载荷保存不可变字段快照，字段地址为对象基址加布局偏移，整体赋值保留原地址。

`bytecode/instruction.def` 统一登记操作码与操作数种类，生成枚举和验证元数据。`SemanticValueBridge` 在语义入口转换类型、常量与函数，`ExecutionLinker` 按函数 ID 持有和链接代码；执行镜像拥有 `ConstantData` 字节区、`InitialSlots` 与 `TypeDesc` 表，不保留 IR 指针。VM 在连续指令数组中分派，按槽号直接读写 `RuntimeValue`，以显式调用栈处理 Ink 函数调用。常用整数和 bool 使用内联位模式；整数读写也选择定宽 `LoadI*` / `StoreI*` 操作码。地址仅供本函数直接读写的 Alloca 使用固定帧单元，不逐次创建 Cell；再次执行分配时重置初始化状态并继续收取累计预算。可观察的地址采用裸指针，分配由调用帧负责清理，指针本身不保活目标。`IRContext::revision()` 变化时执行代码和原生调用计划缓存失效。语义求值的 `ExecutionFrame` 只保存词法绑定与存储，公开执行值在桥接边界转回 `ExecutionValueRef`。类型职责、指令布局和扩展方法见 [执行字节码设计](docs/Ink-Execution-Bytecode.md)。

已经准备完整的 `ExecutionImage` 也可直接构造无桥接层的 `ExecutionLinker`，由 VM 按函数 ID 执行；源 IR 析构后，函数调用、存储、CString 与原生调用仍使用镜像自有数据。`execution/artifact` 提供完整对象构建、稳定符号身份、多文件静态链接及 v6 磁盘归档；归档加载后重新建立共享类型域和原生调用缓存。

整数的通用精确位宽运算使用项目自有的 `ExecutionInteger`；字节码的常用位宽加法和比较直接处理内联位模式，保持既有回绕与符号规则，execution 不使用 `llvm::APInt`。`comptime func` 与普通函数一样在定义处检查并生成 IR，未调用的函数也检查函数体；运行时使用这类函数会报告用户诊断。函数体支持 bool 的 `!`、`&&`、`||`，同型整数的六种比较及 bool 的 `==`、`!=`，可用于普通 if 条件；源码循环和其余未接入的算术运算仍显式诊断为未支持。显式 `comptime` 表达式、块和静态循环仍由语义层在生成 IR 时求值或展开；泛型延迟实例化另行实现。

编译期和 IR 中的 `import "C"` 优先匹配当前程序纳入的同名同签名原生导出，否则通过系统 API 查找当前进程符号。`ffi/runtime_ffi_type.cpp`、`runtime_ffi_argument.cpp` 和 `runtime_ffi_call.cpp` 按布局准备并缓存调用计划，直接封送 VM 值。FFI 支持 bool、8/16/32/64 位整数、f32/f64、裸指针参数与返回以及 void 返回；按值聚合、变参及 f16 尚不支持。原生返回指针保留原始地址，执行器可以读写有效的外部内存；`*Class`、宽整数字段的地址和 `T**` 输出参数均可传递。指针槽内存中保存真实机器地址，多个别名指向同一位置。调用方负责目标生命周期、布局、对齐、可写性和访问范围；地址释放后不会自动清空或保证诊断。指针和函数结果不能冻结到 IR 常量。

每个 `ExecutionEngine` 持有独立的 `NativeSymbolCache`，按精确符号名保存首次成功解析的地址，供编译期和 IR 外部调用共用；查找失败不会缓存。缓存拥有名称字符串，不持有 DLL／共享库的加载引用，调用方必须保证相关模块在缓存和返回地址的使用期间保持加载。如需卸载模块或重新绑定同名符号，应在相关调用和地址使用结束后、卸载前调用 `clearNativeSymbolCache()`；下一次调用会重新解析。

`CString` 缓冲区在执行该指令的函数帧结束时释放，FFI 返回其内部地址不延长该生命周期。直接传给 FFI 的字符串另建可写临时副本；未由返回值引用的副本在调用结束时释放，返回其内部或尾后地址时则明确将清理责任移交给调用者的 Heap，直到显式释放或 Heap 销毁。返回指针仍不拥有缓冲区，已有帧缓冲区也不会因此提升生命周期。测试覆盖“源码 → tokenizer/parser AST → semantic IR → `execute(Entry)` → `_write`/`write` → 管道字节与返回值断言”，并检查重复读取调用结果只写入一次、嵌套调用及指针逃逸。Windows 源码直接声明 `_write`，Linux 声明 `write`。声明示例和当前类型边界见 [编译期外部调用](docs/Ink-Semantic-Analysis.md#编译期外部调用)。

## IR 对象模型

公共类型统一位于 `ink::ir`，包括 Value、类型与常量及其池、函数、基本块、指令、Name/NamePool、Decl 及泛型 AST 关联，以及 IRBuilder。Analyzer、名称解析和作用域保留在 `ink::semantic`。

这套模型提供函数、调用、内存、加法、逻辑、比较、分支和返回节点的构造接口，execution 已接入这些 IR 指令并按显式分支执行。完整源码语义分析、后端及完整图验证器仍待实现。当前分析能力与后续缺口见 [语义分析接口](docs/Ink-Semantic-Analysis.md)。

普通 if 的条件严格要求 bool，支持参数、局部变量、函数返回值和常量。两个运行时分支均检查，`comptime if` 只分析选中分支；分支有独立作用域，汇合处仅对仍能继续执行的路径合并确定初始化状态，无 else 时包含入口路径。两支都返回时后续语句不可达，只有一支返回时继续检查另一支的返回及初始化情况。

`!`、`&&`、`||` 只接受 bool；源码 `&&`、`||` 通过条件分支、临时 bool 存储和汇合块短路，左侧只求值一次，右侧仅在需要时执行，但普通函数定义处仍检查两侧类型。`LogicalAndInstruction` 和 `LogicalOrInstruction` 消费已经求值的两个 bool，不负责跳过此前求值。`CompareInstruction` 对完全同型整数支持 `==`、`!=`、`<`、`<=`、`>`、`>=`，保留符号属性并支持任意 IR 整数位宽；bool 仅支持 `==`、`!=`，浮点和指针比较尚未支持。所有逻辑与比较指令的结果均为 bool。

- `IRBuilder::createBranchInstruction(Target)` 和 `createConditionalBranchInstruction(Condition, TrueTarget, FalseTarget)` 在当前块末尾插入 void 类型分支，条件必须是当前上下文中的 bool 值。对应的 `createDetachedBranchInstruction()` 和 `createDetachedConditionalBranchInstruction()` 返回未挂接的 owner，可引用本上下文的函数块或未挂接块；实际插入时，源块和所有目标块必须已属于同一函数。目标只是借用引用，分支不拥有目标块。
- `Value::isTerminator()` 统一识别 Return、Branch 和 ConditionalBranch；`BasicBlock::terminator()` 返回末尾终结节点，没有终结节点时返回空指针。终结指令只能在函数块末尾插入且不能重复，任何节点都不能追加到已有终结指令之后；普通指令仍可插入终结指令之前。

- `IRBuilder::createFunction()` 根据签名创建 `FunctionParameter`，通过 `parameters()` 访问；形参通过 `outer()` 关联函数，`function()` 从该父节点取得所属函数，不再重复保存 Owner；同时保存 `Name ParameterName`（通过 `name()` 访问）、零起始索引、值类型和 `ParameterKind`（Positional、Named、Variadic），通过 `parameterKind()` 查询。`IRBuilder::createFunction()` 的可选种类列表必须与签名槽位数量一致，省略时全部为 Positional；第四个可选参数 `ParameterNames` 按签名顺序提供名称，省略时参数匿名，由调用方驻留并填写形参名；种类是绑定元数据，不改变规范化运行时签名或开启变参展开。`createAddInstruction()` 接受同型整数操作数，定义按位宽回绕的加法；`IRBuilder::createDetachedReturnInstruction(ReturnedValue)` 创建未挂接的 void 类型终结节点，只校验操作数归属和非 void 类型；`IRBuilder::appendValue()` 校验目标块属于函数且返回值匹配该函数签名。返回指令不保存 Owner，`function()` 沿 outer → BasicBlock → Function 查询，未挂接时返回空指针。

- 对象模型头文件位于 `src/include/ink/ir`，实现位于 `src/lib/ir`；声明基类及其派生类放在 `ir/decl` 子目录，函数相关的 `Function`、`FunctionType`、`BasicBlock` 放在 `ir/function` 子目录，模块值 `Module` 放在 `ir/module` 子目录，`Name` 和 `NamePool` 放在 `ir/name` 子目录，其余类型及类型注册表放在 `ir/type` 子目录，按需包含对应的独立头文件。
- `src/include/ink/ir/coredefines.h` 集中定义 `ValueKind`、`TypeKind`、`ParameterKind`、`CallingConvention`、`LanguageLinkage`、`FunctionBinding`、`AccessKind` 和 `VisibilityKind`，仅依赖 `<cstdint>` 与枚举注册表，可独立包含。`VisibilityKind::Public/Private` 已接入函数导入检查和字节码符号输出；顶层函数默认 public，局部函数为 private。
- `Function` 独立保存调用约定 `CallingConvention::C/Fast/Cold`、语言链接 `LanguageLinkage::Ink/C`、原生方向 `FunctionBinding::Local/Import/Export` 和源码可见性 `VisibilityKind::Public/Private`，分别通过 `callingConvention()`、`languageLinkage()`、`binding()`、`visibility()` 查询。`IRBuilder::createFunction()` 的第五、六、七个可选参数分别设置调用约定、语言链接和原生方向，默认为 C、Ink 和 Local；`setFunctionVisibility()` 设置可见性。Parser 在 AST 中保留 import/export 方向和完整 ABI 字符串；Analyzer 目前支持 `"C"`，`[abi("C")]` 仅设置本地定义的 ABI。Import 不允许函数体，Local 和 Export 执行其字节码函数体；源码签名、导出重名及导入导出签名匹配由语义层和链接器检查。这些属性不改变 `FunctionType` 的规范化身份；字节码对象单独保存 ABI 和导出信息，LLVM 后端可生成宿主目标文件；通用回调地址桥接仍待实现。
- `src/include/ink/ir/Values.def` 集中定义 `ValueKind`，枚举项与 C++ 类同名，如 `IntegerType`、`FunctionType` 和 `CallInstruction`；类型和常量条目分别生成 `Type::classof()`、`Constant::classof()`。元类型、void、bool、label、module 的实际对象均为 `BuiltinType`，由 `TypeKind` 继续区分；仅作为中间基类的 `Type`、`UserDefinedType`、`Constant` 不单独占用值种类。
- `src/include/ink/ir/type/Types.def` 用 `INK_IR_TYPE(Name, Base)` 集中登记类型种类，`Base` 配置为 `BuiltinType` 或 `UserDefinedType`；`TypeKind` 与两个基类的 `classof()` 分类判断均由该表生成。
- `src/include/ink/ir/instruction` 保存直接继承 `Value` 的 `CallInstruction`、`AllocaInstruction`、`LoadInstruction`、`StoreInstruction`、`LogicalNotInstruction`、`LogicalAndInstruction`、`LogicalOrInstruction` 和 `CompareInstruction` 等节点，每种指令有自己的头文件，使用同名 `ValueKind` 分类，由 `IRBuilder` 创建，成功插入后由所属基本块拥有。
- `CStringInstruction` 表示把字符串常量复制到目标程序的独立可写 `u8` 缓冲区，包含终止 NUL，存活到所属函数返回。`createDetachedCStringInstruction()` 和 `createCStringInstruction()` 拒绝外来常量及内嵌 NUL；该节点只能插入函数块。execution 使用所属调用帧管理的 `ExecutionBuffer` 实现复制和失效，不把宿主 `const char *` 地址写入 IR；LLVM 后端通过受控运行时分配与复制，保留相同生命周期。
- `IRContext` 位于 `src/include/ink/ir/context.h`，借用 `core::CompilationContext`，拥有 `NamePool`、`TypePool`、`ConstantPool` 和根模块；声明树由各 Module 拥有；函数、基本块和指令由所属 IR 父节点拥有。共享对象在上下文存活期间保持地址稳定。`SemanticContext` 位于 `src/include/ink/semantic/context.h`，组合 `IRContext` 与 `ScopeStore`，通过 `irContext()` 访问独立 IR 存储，并转发常用池访问器。程序化 IR 构建可以直接使用 `IRBuilder(IRContext &)`，无需语义分析器。
- `typePool()` 返回当前上下文唯一的类型池，位于 `ir/type/type_pool.h`；所有内建类型、结构类型、函数类型和名义类型的创建与所有权均由池负责，类型获取统一通过 `Context.typePool().getType<TypeKind::...>(...)`，`IRBuilder` 上的名义类型创建工厂转发到同一池。结构类型按完整结构去重，名义类型每次创建独立身份；类型对象的 `context()` 指向所属 `IRContext`。池只由上下文创建，不可复制、移动或清空；元类型先于其他类型初始化，模块、常量和声明先于类型池销毁，名称池最后销毁。声明由 Module 的独立声明树管理，AST 仍由 ParsedUnit 拥有，不放入类型池。
- `constantPool()` 返回当前上下文唯一的常量池，负责 bool、任意位宽整数、`StringConstant` 和 `FloatConstant` 的创建、所有权与去重；常量获取统一通过 `Context.constantPool().getXXXConstant(...)`，Context 不提供转发接口。规范类型身份和完整 payload 决定常量身份，哈希命中后仍精确比较，类型来自其他上下文或 payload 与类型不匹配时返回空指针且不插入。`owns()` 检查具体对象的池归属，`size()` 包含创建时已有的 false、true 两个常量；池只由所属上下文创建，不可复制、移动或清空。
- `ConstantPool::getStringConstant(SliceType, Payload)` 要求只读 `u8` 切片类型，复制并驻留调用方已经验证、解码的 UTF-8 字节，支持空串和内嵌 NUL，不重复解码转义或执行 Unicode 规范化；相同解码字节只保存一个常量。字符串存储以额外 NUL 结尾，`value()` 的长度不含该终止符，`nullTerminatedValue()` 提供包含终止符的完整存储视图；`tryGetCString()` 对无内嵌 NUL 的内容零拷贝返回 `const char *`，否则返回 `nullptr`，避免静默截断。该接口是宿主常量访问；源码字符串常量在 C 调用实参位置通过 CStringInstruction 创建独立可写副本，存活到调用者函数返回，LLVM 后端也保留独立副本及相同生命周期。`ConstantPool::getFloatConstant(FloatType, FloatBits)` 使用项目自有的 IEEE binary16/32/64 位表示，要求位宽与类型完全一致且没有多余高位；按位去重，区分正负零、NaN 符号和 payload，不隐式转换或舍入。常量及字符串视图在上下文存活期间保持有效；整数和字符串源码字面量已接入语义分析，浮点字面量及后端 lowering 仍待实现。
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
- `ink::ir::IRBuilder`（`ink/ir/ir_builder.h`）的 `createCallInstruction`、`createAllocaInstruction`、`createLoadInstruction`、`createStoreInstruction`、`createAddInstruction`、`createLogicalNotInstruction`、`createLogicalAndInstruction`、`createLogicalOrInstruction`、`createCompareInstruction` 和 `createReturnInstruction` 按插入点创建指令，成功后将所有权移交给 BasicBlock 并返回借用指针。先调用 `setInsertPoint(Block)` 选择块末尾，或用 `setInsertPoint(Before)` / `setInsertPoint(Block, Before)` 选择锚点；连续创建保持调用顺序，Alloca 不自动移动到入口块。对应的 `createDetachedXXXInstruction` 工厂返回未挂载的 `unique_ptr`，忽略且不改变插入点，由调用方通过 IRBuilder 的显式编辑接口转移所有权。
- `IRBuilder::saveInsertPoint()` / `restoreInsertPoint()` 保存和恢复块及锚点指针，`InsertPointGuard` 通过 RAII 恢复临时切换前的位置；没有插入点时也可保存和恢复。多个 Builder 的位置相互独立，vector 扩容和所有权转移不改变节点地址。设置或恢复外来块、已摘除或属于其他块的锚点时返回 false 并清空位置，守卫恢复失效位置时同样清空。没有有效位置、操作数不合法或向终结指令后插入时，自动插入指令工厂返回空指针且不插入节点。恢复已结束块的末尾位置本身允许，但后续创建会拒绝。保存的位置不延长节点寿命，销毁目标块或锚点前应清理相关位置和守卫；不支持并发修改同一上下文。
- `IRBuilder::createFunction(Name, Signature)` 返回拥有独立身份和本地 `FunctionType` 的 `std::unique_ptr<Function>`，初始未挂载，供普通函数或已闭合的泛型实例使用。`IRBuilder::createCallInstruction(Callee, Arguments)` 直接调用 `Function`，或间接调用其他类型为 `FunctionType` 的值；泛型 `FunctionDecl` 需要先实例化，不能直接作为调用目标。实参须已完成排序和转换，数量和逐项类型必须精确匹配。指令拥有实参引用列表，每次调用均创建独立对象；创建不执行函数，不进行重载解析或泛型实例化。
- `Function` 使用 `std::vector<std::unique_ptr<BasicBlock>>` 拥有函数块，使用 `std::vector<std::unique_ptr<FunctionParameter>>` 拥有参数；`blocks()`、`parameters()` 返回只读容器，元素通过 `.get()` 借用。空块列表表示没有函数体，`entryBlock()` 返回首块的借用指针。`IRBuilder::createBasicBlock(Function &)` 创建并直接交给函数拥有，设置 `Outer` 后返回借用指针；首块自动成为入口。`IRBuilder::createFunctionBody(Function &)` 仅创建首块，已有函数体或外来函数返回空指针。函数销毁时先销毁函数块，再销毁参数；列表顺序不代表执行顺序，块内指令通过 `IRBuilder` 构建。源码 if 的控制流连接、返回路径及确定初始化检查已实现；源码循环和更完整的控制流分析仍待实现。
- `BasicBlock` 使用 `std::vector<std::unique_ptr<Value>>` 拥有有序子节点，`values()` 返回只读容器。`IRBuilder::createBasicBlock()` 返回未挂载的 `std::unique_ptr<BasicBlock>`。`IRBuilder::insertValue(Block, std::move(Child), Before)` 在指定直接子节点前转移所有权，Before 为空时追加；`IRBuilder::appendValue(Block, std::move(Child))` 是末尾追加入口。这些显式编辑接口使用参数指定的块和锚点，不读取或改变 Builder 当前的插入点。成功清空调用方的 owner 并设置 `Outer`，失败保留 owner 与原块内容；拒绝空 owner、外来对象、非法锚点、已有父节点、循环包含、类型和常量。Return、Branch 和 ConditionalBranch 都是终结指令，只能位于函数块末尾；已有终结指令后不能追加节点，但可在终结指令前插入普通指令。Return 必须匹配所属函数的返回签名，分支目标必须属于同一函数。`IRBuilder::removeValue(Block, Child)` 摘除直接子节点、清空其 `Outer` 并返回 `std::unique_ptr<Value>`；失败返回空 owner。`IRBuilder::eraseValue(Block, Child)` 则立即销毁该子树。摘除后的节点可重新挂载，父节点销毁会递归释放仍属于它的子树。
- `Module` 使用 `std::unique_ptr<ModuleDecl>` 和 `std::unique_ptr<BasicBlock>` 分别拥有独立的声明树与 IR 入口块，销毁时先释放 IR，再释放声明树。`declarationRoot()` 返回借用指针，尚未创建声明根的程序化模块返回空指针。`IRBuilder::createModule(Name)` 校验名称后创建 Context 拥有的根模块，返回借用指针；`modules()` 提供根模块的只读拥有容器，`entryBlock()` 借用入口块，入口块的 `Outer` 指向 Module。`IRBuilder::removeModule(Module)` 从根列表摘除并返回 `unique_ptr<Module>`，随后可交给某个 BasicBlock 实现嵌套；`IRBuilder::appendModule(std::move(Owner))` 将未挂载 Module 重新交给 Context，兼容 `IRBuilder::removeValue()` 返回的基类 owner，失败不消费所有权。`IRBuilder::eraseModule(Module)` 销毁根模块及其两棵子树；模块所有权转移不改变声明归属或节点地址。模块仍使用 `TypePool::getType<TypeKind::Module>()`，与借用文件 AST 的 ModuleDecl 分开。
- `IRBuilder::createClassType(Name)`、`IRBuilder::createEnumType(Name)`、`IRBuilder::createInterfaceType(Name)` 直接创建具有独立身份的名义类型，不创建语义 `Decl`。同名类型不合并，分析器负责复用同一普通类型或泛型实例的返回对象；Class 成员和布局通过 defineClassType() 补全；继承、枚举底层类型和泛型实例缓存仍待实现。组合类型工厂仅检查结构约束；源码数组由分析器检查元素类型、编译期长度和下标，执行布局由运行时模块负责；完整引用使用规则尚未实现。
- `createAllocaInstruction(AllocatedType)` 创建单对象、未初始化的分配指令，结果为对应的可读写指针；数组通过 `ArrayType` 表示。当前支持 bool、整数、浮点、指针、引用、切片及这些类型组成的定长数组，拒绝外来类型、元类型、void、label、module、原始函数签名和布局未完成的名义类型。`createLoadInstruction(Address)` 从指针读取并产生元素类型的值，接受只读或可读写指针；`createStoreInstruction(Address, StoredValue)` 要求可读写指针、同一上下文和完全一致的元素类型，结果类型为 void。对象存活期间地址在容器扩容和所有权转移后保持稳定，操作数仅被借用，每次创建均有独立身份，调用者通过 `IRBuilder` 的插入点安排执行顺序；构建节点不会执行内存操作，也不检查初始化、支配关系或实际地址有效性。
- 执行模型不再保存源码变量绑定对象。源码变量和形参与模型对象的绑定、const 分析仍待实现。初始化和赋值由各自位置的 `StoreInstruction` 表示，读取使用 `LoadInstruction`；`StoreInstruction` 在构造时将自身类型设置为上下文唯一的 void 类型，由 `Value::type()` 返回。
- 空名称或名称索引空间耗尽返回无效 `Name`；无效或越界索引查询返回空视图。整数工厂拒绝零位宽，浮点工厂拒绝 16/32/64 以外的位宽；组合类型工厂拒绝外来类型和无效访问权限。命名对象工厂拒绝无效或越界名称，内存指令工厂另检查地址、访问权限和操作数类型，泛型声明工厂另检查 AST 的泛型参数。上述检查返回显式状态，不依赖异常。
