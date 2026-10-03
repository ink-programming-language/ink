# 命令行接口

当前构建包含 `ink-tokenize`、`ink-parse` 和 `inkc`。工具共用 `ink::cli::Application`，显式返回参数解析状态，不使用异常作为控制流。

## 公共规则

- `-h` / `--help` 打印帮助，`-V` / `--version` 打印版本；信息写入 stdout 后退出。
- 参数名按完整拼写匹配。带值选项使用 `--name VALUE` 或 `--name=VALUE`；包含空格的路径由 shell 引号包围。
- `--` 结束选项识别，后续内容按位置参数处理。`inkc` 的输入使用命名选项 `-i` / `--input`，不接受位置输入文件。
- 输入文件按原始字节读取；`-` 表示标准输入。Windows 标准输入切换为二进制读取，文件路径通过 UTF-8 转换。
- 工具诊断写入 stderr，供人阅读的工具输出使用统一 CLI 输出接口；不会把诊断混入正常 stdout 数据。

## 词法和语法工具

```text
ink-tokenize [INPUT]
ink-parse [INPUT]
```

省略 INPUT 或指定 `-` 时读取 stdin。`ink-tokenize` 将 token 流写入 stdout，`ink-parse` 将 AST 和恢复结构写入 stdout；源码诊断写入 stderr。

成功退出为 0，词法或语法错误为 1，参数或输入输出错误为 2。源码内部错误使用项目统一的 ICE/panic 机制，可能直接终止进程。

## inkc 解释模式

```text
inkc --interpret [--entry NAME] -i FILE
inkc --interpret [--entry NAME] --input FILE
```

| 参数 | 含义 |
| --- | --- |
| `--interpret` | 分析源码并使用 ExecutionEngine 解释执行 IR |
| `--entry NAME` | 模块内入口函数名，默认 `main` |
| `-i FILE` / `--input FILE` | 必填源码文件；`-` 从 stdin 读取 |
| `-oir FILE` | 保留识别的 IR 输出选项，与解释模式冲突；当前未提供 IR 输出模式 |

必须选择解释、字节码编译、字节码链接或字节码运行中的一种模式。同时指定 `--interpret` 和 `-oir` 会作为参数冲突拒绝。

处理流程为源码 → tokenizer → parser AST → semantic IR → 选择入口 → `ExecutionEngine::execute()`。入口必须是模块内可唯一选择的零参数本地或导出函数，具有可执行 IR 函数体，返回 `void` 或有符号 `i32`。原生导入、仅声明而没有函数体的函数及编译期专用函数不能作为入口；程序实参传递尚未接入。

解释器当前执行 Alloca、Store、Load、整数 Add、LogicalNot、LogicalAnd、LogicalOr、Compare、CString、Call、Branch、ConditionalBranch 和 Return，以及块内函数声明。支持 bool 条件的 if/else/else-if 与嵌套分支，条件可来自参数、局部变量、函数返回值或 bool 常量；分支指令只执行选中的路径。普通 if 的两个分支在定义处均进行语义检查，`comptime if` 只分析选中分支。非泛型函数的编译期调用共享同一 IR 执行路径；显式 `comptime` 语句块和静态循环仍可在语义分析期间求值或展开。源码循环、聚合执行及完整后端编译仍未实现。

源码支持 bool 的 `!`、`&&`、`||`；短路通过条件分支和临时 bool 存储实现，左侧只执行一次，右侧仅在需要时执行，普通函数仍在定义处检查两侧类型。IR 的 LogicalAnd/LogicalOr 直接处理已求值的操作数，不负责撤销或跳过先前求值。`==`、`!=`、`<`、`<=`、`>`、`>=` 接受完全同型整数，比较保留符号属性；bool 只接受 `==`、`!=`。IR 比较支持任意整数位宽，源码可使用已支持的 i/u8、16、32、64、128；浮点和指针比较尚未支持。

`inkc` 不打印 AST 或 IR。编译和执行诊断写入 stderr，源程序调用 `_write` / `write` 等原生函数产生的 stdout 保持为程序输出。成功执行 void 入口时退出码为 0，i32 入口的结果作为进程退出码；shell 如何显示该值遵循宿主平台的进程退出约定。

引擎执行失败由 CLI 将 `ExecutionStatus` 转为 Core 的具体 `Execution*` 诊断，补充入口函数上下文，再通过 `DiagnosticEngine` 统一输出；原因文案不含编译期限定。取消只返回失败，不生成错误诊断。参数、输入或入口选择错误退出为 2，普通源码错误、执行用户错误或取消为 1，CLI 显式报告的内部失败为 3；执行资源超限、内部状态违规、未实现操作等 Core ICE 使用既有 panic，可能直接终止进程。程序自身返回非零值也可能产生相同的进程退出码，应结合 stderr 判断。

### 使用示例

在仓库根目录、完成 README 中的构建后，Windows x64 / PowerShell：

```powershell
.\build\src\tools\inkc\Release\inkc.exe --interpret -i src/testcase/execution/programs/hello_world.windows.ink
.\build\src\tools\inkc\Release\inkc.exe --interpret -i src/testcase/execution/programs/logical_truth_tables.ink
'func main(): i32 { return 7; }' | .\build\src\tools\inkc\Release\inkc.exe --interpret --input -
```

Linux x86-64 / 单配置构建：

```sh
./build/src/tools/inkc/inkc --interpret -i src/testcase/execution/programs/hello_world.linux.ink
./build/src/tools/inkc/inkc --interpret -i src/testcase/execution/programs/logical_truth_tables.ink
echo 'func main(): i32 { return 7; }' | ./build/src/tools/inkc/inkc --interpret --input -
```

hello world 源码分别声明平台实际符号及 ABI：Windows 为 `_write(i32, *u8, u32): i32`，Linux 为 `write(i32, *u8, u64): i64`。main 向 stdout 写入 `hello, world\n` 并返回 0。Windows 示例沿用 CRT stdout 的默认文本模式，捕获的换行为 CRLF；Linux 捕获为 LF。解释器不添加输出标题或转换源程序的输出内容。

## inkc 字节码编译、链接与运行

```text
inkc --emit-bytecode OBJECT [--module-root DIRECTORY] -i SOURCE
inkc --link-bytecode OBJECT --link-bytecode OTHER --entry MODULE#NAME -o EXECUTABLE
inkc --run-bytecode -i EXECUTABLE
```

三种模式与 `--interpret` 互斥。`--emit-bytecode` 的值就是输出路径；`--link-bytecode` 可重复指定，每次接收一个对象文件，并要求 `-o` / `--output`。运行模式的 `-i` 是已链接字节码文件路径，不接受源码 stdin。字节码文件的后缀不参与识别，加载器检查文件魔数、版本和 target。

顶层函数的默认可见性为 `public`，`private func ...` 只在定义它的源码文件内可访问。局部函数始终私有，显式声明为 `public` 报源码错误。编译器把这一语义信息写入 IR 和字节码；CLI 不接受 `--export`、`--module-visible` 或 `--import-symbol`，测试清单也不配置导出及导入映射。所有普通函数定义都会编译，包括从未执行的函数，编译期专用函数不作为运行时导出。局部函数身份包含父函数签名与词法位置，父函数重载中的同名局部函数保持独立。

跨文件使用 `from math import answer;`、`from math import answer as localAnswer;` 或 `import math as library;` 后调用 `library.answer()`。直接导入未写别名时使用模块路径末段作为绑定名。导入以目标声明为准，自动获得真实签名和重载集合；私有定义不能被导入或通过模块成员访问。普通 `func answer(): i32;` 会报告 `INK-S0028`，只能在 `import "C"` 下保留无函数体声明。

模块身份由源码相对 `--module-root` 的路径去掉 `.ink` 后将目录分隔符替换为点确定，例如 `package/math.ink` 对应 `package.math`。生成对象或使用导入时，源文件扩展必须为 `.ink`，根目录内的目录名和文件基名不能包含点，以防 `package.math.ink` 与 `package/math.ink` 映射成相同身份；不含导入的单文件解释模式继续允许原有文件名。未指定源码根时使用入口文件的父目录，stdin 使用当前目录。`from .math import answer;` 的一个点表示当前包，更多点向上查找；不允许越过源码根。每个对象必须使用相同源码根独立编译，运行 `--interpret` 也支持该选项和源码导入。当前导入对象限于已实现的函数，泛型实例化尚未接入；闭合泛型符号元数据仍由库级构建 API 接收。

原生接口使用 `private import "C" func abs(Value: i32): i32;` 或 `public export "C" func sum(A: i32, B: i32): i32 { return A + B; }`。导入禁止函数体，导出必须在模块顶层提供函数体；`[abi("C")] func ... { ... }` 仅设置本地定义的 C ABI。`public/private` 独立控制 Ink 源码访问，允许 `private export`，且不会因此允许其他 Ink 模块通过模块导入访问私有函数。原生导出名为声明名，不能形成重载；跨模块重复导出报告 `INK-S0049`。仅设置 `[abi("C")]` 的本地函数仍可按参数列表重载。旧 `extern` 语法已移除，`link` 属性暂不支持。

当前 CLI 输出字节码对象与镜像；本地 C ABI 和导出函数由 VM 执行，存档保留其 ABI、原生符号名与导出标记。尚不生成原生 DLL/SO，也不提供可传给 C 的本地函数地址。原生导入优先按声明原名及完整签名匹配当前程序已纳入的导出，包括 `private export`；同名签名不匹配报告 `INK-S0050`。未匹配的导入继续按原声明名查找宿主已加载符号。原生导入本身不触发 Ink 源码依赖发现，也不加载新的宿主动态库。

链接入口默认查找公开的 `main`，也可通过 `--entry module#function` 或简单的 `module::function` 指定。入口必须是具有函数体的本地或导出函数，没有泛型实参、没有运行时参数、返回 void 或 i32；候选不唯一时拒绝。链接文件记录所选入口；`--run-bytecode` 直接使用文件中的入口。v2 文件要求同宿主 target/ABI，未被同程序导出满足的原生导入符号在实际调用时重新解析。

成功编译和链接退出为 0；参数、归档读取或写入失败退出为 2；源码、链接符号或普通执行失败退出为 1。运行成功时 void 返回 0，i32 作为退出码。已有的执行资源和内部错误继续使用 Core ICE/panic 规则。协议与 API 见 [字节码文件格式](Ink-Bytecode-Format.md)。

`BytecodeProcessTest` 使用独立 `inkc` 进程编译、链接后移走源文件和对象文件，再重复加载执行最终镜像；另验证原生导入重新绑定、私有入口拒绝、歧义模块路径拒绝、旧可见性覆盖参数拒绝、对象不可直接执行及截断文件拒绝。跨模块源码场景由下述 `ExecutionMultiFileTest` 覆盖。

### CLI 与源码执行测试

源码执行测试统一由 [`source_program_tests.cmake`](../src/testcase/execution/cli/source_program_tests.cmake) 的 `add_source_program_test()` 登记 HelloWorld 和功能测试程序；`src/testcase/CMakeLists.txt` 只需 include 这份清单。所有源码测试共用 [`source_program_test.cmake`](../src/testcase/execution/cli/source_program_test.cmake)，参数与入口错误等 CLI 行为另由 [`inkc_process_test.cmake`](../src/testcase/execution/cli/inkc_process_test.cmake) 验证。成功程序保留在 `src/testcase/execution/programs`，故意失败的输入放在 `src/testcase/execution/cli/inputs`。

Windows 多配置构建运行：

```powershell
ctest --test-dir build -C Release -R '^(ExecutionSourceTest[.].*|InkcProcessTest)$' --output-on-failure
```

Linux 单配置构建省略 `-C Release`：

```sh
ctest --test-dir build -R '^(ExecutionSourceTest[.].*|InkcProcessTest)$' --output-on-failure
```

`ExecutionSourceTest.HelloWorld` 从统一清单选择平台对应的已提交源码，共享脚本使用 CMake `execute_process()` 启动真实 `inkc --interpret -i FILE`，默认执行 `main`，通过 `OUTPUT_FILE` 将 stdout 直接写到临时文件。常规输出比较将 CRLF 归一化为 LF；HelloWorld 另外设置 `STDOUT_HEX`，在归一化前验证原始字节，包括 Windows CRLF / Linux LF，同时要求完整 hello world 输出、空 stderr 和退出码 0。

`InkcProcessTest` 检查默认及指定入口、stdin、入口限制、参数冲突、错误诊断与执行返回结果。`ink_tests` 构建依赖 `inkc`、`ink-tokenize` 和 `ink-parse`；这些进程测试由 CTest 调度，不是由 GoogleTest 进程内模拟 CLI。

单文件 `ExecutionSourceTest` 功能测试程序通过 `puts` 打印用例名，再核对结果并打印 `PASS`；失败打印 `FAIL` 并返回 1，全部通过返回 0。副作用标记夹在用例名与结果之间，编译期外调的输出发生在 `main` 之前。CTest 对完整 stdout 逐字核对，要求退出码 0、空 stderr；仅将 CRLF 归一化为 LF，因此结果、顺序、调用次数和多余输出都受到检查。

多文件用例放在 [`src/testcase/execution/multifile`](../src/testcase/execution/multifile)，每个场景一个子目录，包含至少两个真实 `.ink` 文件和一个 `case.cmake`。所有访问权限和依赖都写在 `.ink` 源码中。清单仅使用 `expect_bytecode_result()` 断言执行返回值，或用 `expect_bytecode_source_error()` 断言指定源码模块的错误。例如 `chain/case.cmake` 的三个文件分别编译为独立对象，再链接执行。

[`multifile_program_tests.cmake`](../src/testcase/execution/cli/multifile_program_tests.cmake) 自动将每个目录注册为独立的 `ExecutionMultiFileTest.<目录名>`。共用运行器递归发现并复制源码，逐文件启动编译进程，并分别按原顺序和反向顺序链接。成功场景先解释执行，然后移走源码副本和对象文件，由两个新进程加载镜像，检查预期退出码及空 stdout/stderr；失败场景检查指定文件的具体诊断、退出码 1 和未生成对象文件，其他文件仍必须编译成功。测试日志和中间产物保留在构建目录的 `src/testcase/execution-multifile/<场景>/<本次运行 ID>`。

多文件场景覆盖调用链、跨文件相互递归、跨模块编译期调用、重载、限定模块名和相对导入、跨文件共享指针读写、模块内访问、同名私有函数隔离、原生导入匹配 private 导出及本地 C ABI 调用，以及越权访问、无体 Ink 声明、签名不匹配、歧义调用、重复定义和缺失模块或符号。新增场景只需创建子目录及清单，无需修改运行器；清单必须编译目录中的每个 `.ink` 文件。

```sh
ctest --test-dir cmake-build-debug -R '^ExecutionMultiFileTest[.]' --output-on-failure
```

多配置生成器另加 `-C Debug` 或相应配置名。

逻辑与比较包含以下程序，CTest 名称以 `ExecutionSourceTest.Logical.` 开头：

| 源码 | CTest 名称后缀 | 覆盖内容 |
| --- | --- | --- |
| [`logical_truth_tables.ink`](../src/testcase/execution/programs/logical_truth_tables.ink) | `Main.TruthTables` | `!`、`&&`、`||`、bool 相等和不等的真值表 |
| [`logical_comparisons.ink`](../src/testcase/execution/programs/logical_comparisons.ink) | `Main.Comparisons` | 六种整数比较、有符号与无符号边界、i/u128 和文字定型 |
| [`logical_contexts.ink`](../src/testcase/execution/programs/logical_contexts.ink) | `Main.Contexts` | 优先级、括号、局部赋值、调用实参、嵌套条件和汇合 |
| [`logical_short_circuit.ink`](../src/testcase/execution/programs/logical_short_circuit.ink) | `Main.ShortCircuit` | 跳过右侧调用及嵌套短路 |
| [`logical_comptime.ink`](../src/testcase/execution/programs/logical_comptime.ink) | `Main.Comptime` | 编译期函数、普通函数的编译期调用及直接编译期表达式 |
| [`logical_effects.ink`](../src/testcase/execution/programs/logical_effects.ink) | `Main.Effects` | 通过操作数输出验证求值顺序、短路和结果复用不重复副作用 |
| [`logical_nested_truth_tables.ink`](../src/testcase/execution/programs/logical_nested_truth_tables.ink) | `Main.NestedTruthTables` | 三个 bool 的全部八种组合，每组分别验证优先级、分组取反和短路分支汇合 |

其余功能测试按类别组织，CTest 名称统一以 `ExecutionSourceTest.` 开头：

| 源码 | CTest 名称后缀 | 覆盖内容 |
| --- | --- | --- |
| [`arithmetic_widths.ink`](../src/testcase/execution/programs/arithmetic_widths.ink) | `Arithmetic.Main.Widths` | 全部十种整数类型的加法、正负边界回绕、无符号高位和跨字进位 |
| [`arithmetic_contexts.ink`](../src/testcase/execution/programs/arithmetic_contexts.ink) | `Arithmetic.Main.Contexts` | 括号、正负字面量、类型推导、赋值、作用域和操作数顺序 |
| [`comptime_arithmetic.ink`](../src/testcase/execution/programs/comptime_arithmetic.ink) | `Arithmetic.Main.Comptime` | 四则、取模、符号规则、位运算和优先级、移位、复合赋值、前后增减及宽整数 |
| [`function_calls.ink`](../src/testcase/execution/programs/function_calls.ink) | `Function.Main.Calls` | 零参/多参/嵌套调用、按类型和参数个数重载、bool/u128 传值、嵌套声明和递归 |
| [`function_storage.ink`](../src/testcase/execution/programs/function_storage.ink) | `Function.Main.Storage` | 按值传参、重复/嵌套/递归调用的局部存储隔离、返回值保存和提前返回 |
| [`function_effects.ink`](../src/testcase/execution/programs/function_effects.ink) | `Function.Main.Effects` | 实参顺序、结果复用、丢弃返回值、显式/隐式 void 返回及副作用次数 |
| [`external_scalars.ink`](../src/testcase/execution/programs/external_scalars.ink) | `External.Main.Scalars` | 真实 CRT 的 i32/i64 参数与返回、字符串转整数和嵌套外调 |
| [`external_strings.ink`](../src/testcase/execution/programs/external_strings.ink) | `External.Main.Strings` | 空串、UTF-8 字节数、字符串比较、编译期嵌入 NUL 和按字节读取 |
| [`external_pointers.ink`](../src/testcase/execution/programs/external_pointers.ink) | `External.Main.Pointers` | 返回内部指针、指针复用与别名、独立可写字符串副本和原生修改 |
| [`external_effects.ink`](../src/testcase/execution/programs/external_effects.ink) | `External.Main.Effects` | 原生副作用、三实参顺序、void 外调、零参外调和重复调用 |
| [`external_comptime.ink`](../src/testcase/execution/programs/external_comptime.ink) | `External.Main.Comptime` | 编译期外调、初始化仅执行一次、普通/编译期函数内外调和跳过缺失符号 |
| [`control_flow.ink`](../src/testcase/execution/programs/control_flow.ink) | `ControlFlow.Main.Branches` | if/else-if/嵌套各路径、确定初始化、局部遮蔽、空分支和分支副作用 |
| [`comptime_control_flow.ink`](../src/testcase/execution/programs/comptime_control_flow.ink) | `Comptime.Main.ControlFlow` | 模块值快照、for/while、break/continue、零次和嵌套迭代、静态展开后的运行时分支 |

普通 IR 当前只接入整数加法；其余算术在显式 `comptime` 表达式或块中验证，不表示运行时已支持这些运算。成功程序依赖 Windows/Linux CRT。字符串、指针和外调副作用程序把 C `size_t` 声明为 `u64`，仅在 `CMAKE_SIZEOF_VOID_P EQUAL 8` 时注册。字符串比较只检查返回值的正负或零；随机数测试对比同一种子重置后的序列，不依赖具体 CRT 的随机值。

负向输入独立验证非 bool 逻辑/条件、整数类型和字面量范围、除零/模零、非法移位、参数数量和类型、重载失败、返回错误、非法捕获、重复声明、未初始化读取、作用域泄漏、缺少函数体及外调错误。每项要求明确退出码、完整 stdout 和对应诊断编号；运行时普通分支即使不会被选中，也必须完成语义检查。

可按 `arithmetic`、`function`、`external`、`logical`、`control_flow`、`comptime` 标签筛选，例如 `ctest --test-dir build -C Release -L arithmetic --output-on-failure`；全部源码测试共用 `source` 标签。新增成功用例时，在 `programs` 添加带 `main` 的源码，并在清单中登记非空完整预期输出；失败用例放到 `cli/inputs`，登记精确退出码和目标诊断。共享 runner 无需为单个用例新增分支。
