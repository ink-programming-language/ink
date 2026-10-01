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

解释器当前执行直线 Alloca、Store、Load、整数 Add、CString、Call 和 Return，以及块内函数声明。普通 if/循环 IR、聚合执行及完整后端编译仍未实现；非泛型函数的编译期调用同样执行生成的 IR，受相同能力限制；显式 `comptime` 语句块和静态循环仍可在语义分析期间求值或展开。

`inkc` 不打印 AST 或 IR。编译和执行诊断写入 stderr，源程序调用 `_write` / `write` 等原生函数产生的 stdout 保持为程序输出。成功执行 void 入口时退出码为 0，i32 入口的结果作为进程退出码；shell 如何显示该值遵循宿主平台的进程退出约定。

引擎执行失败由 CLI 将 `ExecutionStatus` 转为 Core 的具体 `Execution*` 诊断，补充入口函数上下文，再通过 `DiagnosticEngine` 统一输出；原因文案不含编译期限定。取消只返回失败，不生成错误诊断。参数、输入或入口选择错误退出为 2，普通源码错误、执行用户错误或取消为 1，CLI 显式报告的内部失败为 3；执行资源超限、内部状态违规、未实现操作等 Core ICE 使用既有 panic，可能直接终止进程。程序自身返回非零值也可能产生相同的进程退出码，应结合 stderr 判断。

### 使用示例

在仓库根目录、完成 README 中的构建后，Windows x64 / PowerShell：

```powershell
.\build\src\tools\inkc\Release\inkc.exe --interpret --entry main -i src/testcase/execution/programs/hello_world.windows.ink
'func main(): i32 { return 7; }' | .\build\src\tools\inkc\Release\inkc.exe --interpret --input -
```

Linux x86-64 / 单配置构建：

```sh
./build/src/tools/inkc/inkc --interpret --entry main -i src/testcase/execution/programs/hello_world.linux.ink
echo 'func main(): i32 { return 7; }' | ./build/src/tools/inkc/inkc --interpret --input -
```

hello world 源码分别声明平台实际符号及 ABI：Windows 为 `_write(i32, *u8, u32): i32`，Linux 为 `write(i32, *u8, u64): i64`。main 向 stdout 写入 `hello, world\n` 并返回 0。Windows 示例沿用 CRT stdout 的默认文本模式，捕获的换行为 CRLF；Linux 捕获为 LF。解释器不添加输出标题或转换源程序的输出内容。

### CLI 与源码执行测试

进程测试脚本位于 [`src/testcase/execution/cli/hello_world_test.cmake`](../src/testcase/execution/cli/hello_world_test.cmake) 和 [`inkc_process_test.cmake`](../src/testcase/execution/cli/inkc_process_test.cmake)，平台源码样例保留在 `src/testcase/execution/programs`。

Windows 多配置构建运行：

```powershell
ctest --test-dir build -C Release -R '^(ExecutionSourceTest[.]HelloWorld|InkcProcessTest)$' --output-on-failure
```

Linux 单配置构建省略 `-C Release`：

```sh
ctest --test-dir build -R '^(ExecutionSourceTest[.]HelloWorld|InkcProcessTest)$' --output-on-failure
```

`ExecutionSourceTest.HelloWorld` 选择平台对应的已提交源码，使用 CMake `execute_process()` 启动真实 `inkc --interpret --entry main -i FILE`，通过 `OUTPUT_FILE` 将 stdout 直接写到临时文件。测试按字节验证完整 hello world 输出、平台换行、空 stderr 和退出码 0，避免 shell 管道的解码与重新编码影响断言。

`InkcProcessTest` 检查默认及指定入口、stdin、入口限制、参数冲突、错误诊断与执行返回结果。`ink_tests` 构建依赖 `inkc`、`ink-tokenize` 和 `ink-parse`；这些进程测试由 CTest 调度，不是由 GoogleTest 进程内模拟 CLI。
