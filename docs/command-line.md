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

当前只启用解释模式。省略 `--interpret` 会明确报告非解释编译尚未实现；同时指定 `--interpret` 和 `-oir` 会作为参数冲突拒绝。

处理流程为源码 → tokenizer → parser AST → semantic IR → 选择入口 → `ExecutionEngine::execute()`。入口必须是模块内可唯一选择的零参数普通 Ink 函数，具有可执行 IR 函数体，返回 `void` 或有符号 `i32`。C 链接函数、仅声明而没有函数体的函数及编译期专用函数不能作为入口；程序实参传递尚未接入。

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

所有功能测试程序通过 `puts` 打印用例名，再核对结果并打印 `PASS`；失败打印 `FAIL` 并返回 1，全部通过返回 0。副作用标记夹在用例名与结果之间，编译期外调的输出发生在 `main` 之前。CTest 对完整 stdout 逐字核对，要求退出码 0、空 stderr；仅将 CRLF 归一化为 LF，因此结果、顺序、调用次数和多余输出都受到检查。

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
