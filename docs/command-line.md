# Ink 命令行界面规范

> 状态：v0 基础契约已确认
>
> 确认日期：2026-08-17

## 1. 范围与目标

Ink 的 Tokenizer、Parser、完整编译器以及未来其他官方命令行工具共用本规范定义的参数词法、公共选项、路径编码、标准流和退出状态规则。

“共用”表示同一概念始终使用同一名称和语义，而不是让每个阶段接受所有选项。Tokenizer 和 Parser 不得接受 `--target` 一类无意义参数后静默忽略；完整编译器也不得重新定义 `-`、`--`、诊断格式或退出码。

所有工具必须复用 `ink::cli::Application` 完成参数解析。Tokenizer、Parser、Core 和其他语言阶段库本身不得依赖命令行解析实现，也不得复制参数解析和退出码映射。

本文件同时记录已经实现的 v0 基础契约和为完整驱动预先确定的扩展规则。尚未实现的选项必须被拒绝，不能先提供不生效的空壳参数。

### 解析实现

v0 使用仓库自有的 `ink::cli::Application` 实现 option、operand、帮助、校验和退出码映射。解析与定义错误均通过 `ParseResult` 显式返回，不使用 C++ 异常，也不依赖以异常作为解析协议的第三方命令行库。

`ink::cli` 统一固定严格拼写、重复策略、UTF-8、标准流和退出码。Windows 入口通过 Win32 宽字符命令行重建 UTF-8 argv，POSIX 入口校验传入 argv；语言阶段库只接收解析后的值，不链接 CLI 实现。这样可以让公共命令行契约独立于第三方默认行为，并保证所有自有目标在关闭 C++ 异常后仍使用同一套解析路径。

## 2. 工具与编译阶段

用户可见的独立阶段工具使用连字符命名；CMake target 可以继续使用下划线：

```text
ink-tokenize [OPTIONS] [--] [INPUT]
ink-parse    [OPTIONS] [--] [INPUT]
ink          [OPTIONS] [--] INPUT...
```

完整编译驱动中的 Tokenizer、Parser、语义分析和代码生成是同一流水线的阶段，不设计成子命令。驱动使用 `--emit` 选择最终需要的阶段结果：

```text
ink --emit=tokens source.ink
ink --emit=cst source.ink
ink --emit=ink-ir source.ink
ink --emit=llvm-ir source.ink
ink --emit=asm source.ink
ink --emit=obj source.ink
ink --emit=exe source.ink
```

只有 `build`、`run`、`test`、`fmt`、`doc`、`package` 和 `lsp` 一类独立工作流才可以在未来成为单独命令或子命令；它们不改变本规范的公共参数规则。

v0 的 `ink::cli::Application` 只承载一个叶命令；Tokenizer、Parser 和完整编译驱动都属于叶命令。未来若引入带子命令的工作流，必须先让共享解析层按活动子命令解析 option 上下文并补齐一致性测试，不能由单个工具自行引入另一套默认行为。

独立阶段工具和完整驱动必须调用同一份阶段 option 注册代码，不能维护近似但不同的别名、默认值或验证逻辑。

## 3. 参数拼写

参数采用平台无关的 GNU 风格：

- 长选项使用小写 ASCII kebab-case，例如 `--diagnostic-format`。
- 常规短选项名恰好是一个 ASCII 字母或数字。为兼容已经公开的工具界面，共享层也允许显式注册 `-oir` 一类小写 ASCII 多字符单横线名称；它们必须完整匹配，不能作为前缀缩写，新增公共选项仍优先使用 `--long-name`。
- 选项名和枚举值区分大小写，不把下划线视为连字符，也不接受前缀缩写。
- `/option` 不作为选项拼写；若当前位置允许 operand，它按普通路径处理。`-long-option` 只有在作为完整单横线名称显式注册时才直接匹配；否则，如果首个已注册短选项需要值，它仍可按短选项附加值语法解释，例如只注册 `-o` 时，`-output` 等价于 `-o utput`。
- 长选项值同时接受 `--name=value` 和 `--name value`；文档使用 `--name=value` 作为规范拼写。
- option 值不能为空；`--name=` 是调用错误，不能继续从下一 token 补值。
- 非 positional option 必须是无值 flag，或每次出现恰好接收一个非空 argv 值；v0 不使用可选值、单次多值、分隔符拆值或隐式追加值，列表必须通过重复 `Append` option 累积。
- 短选项值使用 `-o FILE` 作为规范拼写；具体选项可以同时接受 `-oFILE`。
- 除单独的标准流标记 `-` 外，值本身以 `-` 开头时不得作为分离 token 传递，必须使用 `--name=-value` 或该短选项定义的附加值形式；其他分离的 dash-leading token 都先按 option 解析。
- 无值的短 flag 可以组合；带值的短选项只能位于组合末尾，剩余字符是它的值。
- Boolean 使用无值 flag；不接受 `--flag=true`。Boolean 始终是 `Single`，默认开启的能力使用明确的 `--no-name` 关闭。
- 单独的 `--` 终止选项识别，其后所有参数均为 operand。
- 参数可以出现在 operand 前后；除非经过 `--`，以 `-` 开头的未知 token 按选项错误处理。
- 程序不展开 glob、`~`、`$NAME` 或 `%NAME%`；这些只由调用 shell 处理。

未知选项、缺失值、非法枚举、越界整数、意外 operand 和冲突选项都必须失败，不能忽略、自动更正或猜测。

## 4. 短选项分配

短选项只授予高频且能够长期稳定的功能：

| 短选项 | 长选项 | 语义 |
| --- | --- | --- |
| `-h` | `--help` | 显示当前工具或命令的帮助 |
| `-V` | `--version` | 显示版本；`-v` 不得用作版本 |
| `-o FILE` | `--output=FILE` | 指定唯一主输出 |
| `-v` | `--verbose` | 增加进度信息，不改变主输出 |
| `-q` | `--quiet` | 抑制非诊断进度信息 |
| `-j N` | `--jobs=N` | 限制并行作业数 |

为编译器惯例保留 `-O`、`-g`、`-I`、`-L`、`-D` 和 `-W` 选项族。阶段工具不得为临时功能占用这些名称，也不得随意分配 `-t`、`-p` 一类含义不稳定的短选项。

## 5. 重复、累加与覆盖

每个 option 必须在定义时选择一种重复策略：

- `Single`：默认策略，最多出现一次；重复时即使值相同也返回调用错误。
- `Append`：列表项按展开后的参数顺序累加，例如未来的 `-I`、`-L` 和多种 `--emit`。
- `LastWins`：只用于明确具有顺序覆盖语义的单一带值 option，例如优化级别；最后一个匹配项生效。

默认策略是 `Single`；正式实现需要非默认策略时必须调用 `Option::repeatPolicy` 选择 `Append` 或 `LastWins`。共享层只接受这三种最终语义，其他枚举值属于无效命令行定义并显式返回 internal error。

`-o a --output=b` 是同一个 `Single` option 的重复。直接参数和响应文件展开后的参数属于同一个序列，应用相同规则。工具不得根据某个 C++ 绑定类型暗中改变重复策略。

真正互斥的模式优先设计成一个枚举选项。若必须提供 `--foo` 和 `--no-foo`，两者同时出现属于冲突，不能产生两个互不相干的状态。

`-W` 一类 keyed policy family 必须先作为 `Append` ordered events 保留，再由 family resolver 对同一 warning key 应用后者覆盖；不能把整个 warning 序列定义成 `LastWins`。

## 6. 输入 operand

单源码流阶段工具准确接受零个或一个 `INPUT`：

```text
省略 INPUT    从 stdin 读取
INPUT 为 "-" 从 stdin 读取
其他 INPUT    作为物理源码文件路径
```

名为 `-` 的真实文件使用 `./-`、绝对路径或其他不等于单独 `-` 的路径。以 `-` 开头的文件名使用 `--`：

```text
ink-tokenize -- -generated.ink
```

完整编译驱动不会因省略输入而隐式等待 stdin。它必须收到源码、manifest 或项目上下文；准确 operand 模型由 package manifest 规范继续细化。一次调用最多使用一个 stdin operand，且命令必须具有为 `<stdin>` 建立所需源码身份的能力。

源文件始终按二进制字节流读取。驱动不得执行换行转换、编码替换或 Unicode normalization；原始字节直接交给 Tokenizer，以保持字节跨度和 full-fidelity 契约。

## 7. 路径与 UTF-8

CLI 字符串契约统一为 UTF-8：

- Windows 在解析前从 Win32 宽字符命令行重建 UTF-8 argv，再把 UTF-8 路径无损转换为原生路径。
- POSIX argv 中用于 Ink 选项和路径的字节必须是合法 UTF-8；非法输入属于调用错误。
- 相对 CLI 路径相对于进程启动时的当前工作目录；工具不得因发现 manifest 或响应文件而隐式 `chdir`。
- manifest 内路径可以按 manifest 规范相对 manifest 目录解析，但这不改变 CLI 路径规则。
- CLI 路径的盘符、分隔符、符号链接拼写和绝对位置不进入 Ink module 的规范名称。

所有重定向文本使用 UTF-8。Windows 上的源码 stdin 必须切换到 binary mode，机器输出也不得依赖当前代码页。

## 8. stdout、stderr 与输出文件

标准流职责固定为：

```text
stdin   支持标准输入的命令所读取的源码或数据
stdout  主结果、帮助或版本
stderr  源码诊断、CLI 错误、I/O 错误和显式开启的进度信息
```

默认执行不得向 stdout 写 banner、进度、计时或“成功”消息。token、CST、IR 和其他机器结果必须能够直接通过 pipe 消费。

完整输出模型预留以下拼写：

```text
-o PATH
--output=PATH
--out-dir=DIR
--emit=KIND
--emit=KIND=PATH
```

规则如下：

- `-o` 只在恰好一个主输出时合法。
- 多种输出使用各自的 `--emit=KIND=PATH`，或由 `--out-dir` 派生文件名。
- 输出路径 `-` 表示 stdout；同一调用最多一种输出写 stdout。
- stdout 输出路径可以写作惯用的 `-o -`，也可以写作 `--output=-` 或支持附加值的 `-o-`；只有单独的 `-` 享有这一分离值特例。
- 文本输出可以写终端；二进制输出只有在 stdout 不是终端且用户显式指定时才能写入。
- 多输入与单一 `-o` 产生歧义时直接报错，不猜测文件名。
- 完整编译失败时不得发布部分最终产物；普通文件输出应使用临时文件并在成功后原子替换。

## 9. 公共帮助与版本

每个官方工具必须提供：

```text
-h, --help
-V, --version
```

帮助和版本写 stdout 并返回 `0`。解析帮助和版本时不要求 required operand 或其他 required option 已经出现。

帮助与版本互斥，同一调用同时指定时返回 `2`。UTF-8、未知 option、Boolean 带值、重复和冲突检查仍先执行，因此 `ink-tokenize --unknown --help` 不会用帮助掩盖拼写错误。

版本首行格式固定为：

```text
<program> <version>
```

开发构建可以使用 `development` 作为版本。发布构建必须由统一构建版本源提供值，不能由各工具分别硬编码。后续行可以显示 commit、目标、语言版本和 Unicode 数据版本，但脚本只能依赖首行格式。

帮助按公共选项、输入输出、诊断、目标、代码生成和高级选项分组。human 帮助的换行与排列不是机器接口。

## 10. 诊断选项与 Consumer

所有产生源码诊断的工具最终统一提供：

```text
--diagnostic-format=human|short|json
--color=auto|always|never
```

- `human` 是默认格式，供终端阅读。
- `short` 每条主诊断使用一行，保留路径、行列、severity 和稳定诊断码。
- `json` 使用 UTF-8 JSON Lines，每个顶层诊断一个对象。
- JSON 永远不包含 ANSI；`--diagnostic-format=json --color=always` 是调用错误。
- `auto` 只在 stderr 是可着色终端且 `NO_COLOR` 未设置时启用颜色。
- 显式 `--color` 优先于 `NO_COLOR`，`NO_COLOR` 优先于自动检测。

诊断必须沿既有公共流水线输出：

```text
producer
→ Diagnostic
→ Core DiagnosticFormatter
→ terminal / short / JSON Consumer
```

CLI 层不得重新分配 `INK-T/P/S` 编号、复制消息模板或让不同工具为同一诊断生成不同语义。主输出格式和 `--diagnostic-format` 是正交概念；token/CST JSON 不能与诊断 JSON 混为一个开关。

JSON schema 正式发布前必须标记版本；发布后删除字段或改变字段语义需要提升 schema version。

## 11. 调用错误

共享命令行解析层使用统一 driver 文本报告解析错误，不伪造源码诊断码：

```text
ink-tokenize: error: unknown option '--foo'
Try 'ink-tokenize --help' for more information.
```

响应文件错误和未进入公共 Diagnostic registry 的 I/O 错误沿用相同的 `<program>: error: <message>` 首行，但不附加帮助提示。所有调用错误只写 stderr，并返回 `2`；默认不附带整份帮助，以免淹没真正错误。

## 12. 响应文件

完整编译驱动需要响应文件以绕过平台命令行长度限制。该能力实现后，所有 Ink 工具必须复用同一个展开器：

```text
ink @compile.rsp
```

响应文件规则采用确定性的逐行 argv 模型：

- 文件为 UTF-8，可以接受开头的单个 UTF-8 BOM，支持 LF 和 CRLF。
- 每个物理行是一个完整参数；空行表示一个空参数，不进行 shell quoting、变量或注释展开。
- `@PATH` 在原位置展开；`@@text` 表示字面量 `@text`，`@-` 禁止。
- 顶层响应文件路径相对启动 cwd；嵌套响应文件路径相对包含它的文件。
- 响应文件内其他相对路径仍相对启动 cwd，使参数移入文件前后语义不变。
- 允许嵌套，但最大深度为 16；包含环、缺失文件、非法 UTF-8、NUL 或超限均返回 `2`。
- 展开在 `--` 和 option 解析前完成，展开后再执行重复、冲突、arity 和未知选项检查。

响应文件展开器落地前，工具不得把第三方 config file 或另一种 quoting 规则作为临时公共协议。

## 13. 配置文件与环境变量

v0 不提供通用 CLI 配置文件，也不自动扫描用户目录或当前目录。Package manifest 是显式项目输入，不是命令行解析器的隐式 config file。

不提供 `INKFLAGS` 一类把自由文本隐式注入命令行的环境变量。影响 target、语言版本、条件 import、诊断策略或代码生成的参数必须显式来自 CLI 或 manifest，并进入构建缓存键。

当前唯一采用的展示环境约定是 `NO_COLOR`。未来若增加显式 `--config=PATH`，格式、版本、列表清空和优先级必须先单独规范，不能直接暴露某个解析实现的内部配置格式。

## 14. 退出状态

所有官方工具固定使用：

| 退出码 | 含义 |
| --- | --- |
| `0` | 成功；也用于成功的 help 和 version |
| `1` | 调用有效，但源码、语法、module、语义或代码生成诊断使请求失败 |
| `2` | CLI、响应文件、显式输入输出 I/O、manifest 定位或工具链启动错误 |
| `3` | 已报告的 Internal Compiler Error、无效内部定义或不变量破坏 |
| `4`—`125` | 保留，v0 不主动返回 |

多个失败同时发生时优先级为 `3 > 2 > 1`。warning 默认不改变退出码；warning-as-error 策略产生有效 error 时返回 `1`。外部 linker 等子进程的任意状态不得直接成为 Ink 的公共退出码；原始状态进入诊断，进程状态映射到本表。

`ink::cli::Application` 把用户参数错误和内部 option 定义错误都返回为 `ParseResult`：前者使用 `InvocationError` 和退出码 `2`，后者使用 `InternalError` 和退出码 `3`。所有官方入口显式传播该结果；不得抛出或捕获 C++ 异常，也不得调用以异常表示预期解析失败的接口。

被信号或外部强制终止时保留平台原生状态，不伪装成 `1`。

## 15. 兼容性

进入公开版本后，下列内容属于兼容性契约：

- 工具名、长选项、短别名和枚举值拼写；
- operand arity 与 `-`、`--` 语义；
- stdout 和 stderr 分工；
- 退出码含义；
- 响应文件语法；
- 正式发布的机器输出 schema；
- 稳定诊断码。

新增选项可以向后兼容地追加。删除、改名或改变默认语义必须经历至少一个发布周期的弃用。未知选项不得自动落入 positional 来掩盖兼容问题。

human 帮助排列和诊断自然语言措辞不是机器接口。实验能力使用 `--experimental-...` 命名空间并在帮助中明确不保证兼容；内部测试参数不得进入公开帮助。

## 16. 实现阶段与一致性测试

当前 v0 基础层已经负责：

- `-h`、`--help`、`-V`、`--version`；
- 严格且跨平台一致的 option 拼写；
- `--` 和 UTF-8 argv；
- UTF-8 到原生文件系统路径转换；
- CLI 错误格式和 `0/1/2/3` 退出码；
- 无效内部命令行定义的显式 internal-error 报告和退出码 `3`；
- 单源码工具省略输入或使用 `-` 时读取 binary stdin。

以下能力只有在对应 Consumer 或 driver 真正实现后才可以暴露：

- `--color` 和多种诊断格式；
- `-o`、`--out-dir` 和多产物 `--emit`；
- 响应文件；
- 完整编译驱动和 package/manifest operand。

公共 CLI 层的测试必须覆盖 help/version 的流与退出码、未知和重复 option、大小写/下划线/前缀近似、Windows slash option、`--`、dash-leading operand、UTF-8 参数与路径，以及源码错误、I/O 错误和内部错误的状态映射。每个新工具还必须有少量真实进程测试验证 stdin/stdout/stderr 不互相污染。

## 17. 设计依据

- [POSIX Utility Syntax Guidelines](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html) 提供 `--`、`-` 和 operand 的基础约定。
- [Clang Command Guide](https://clang.llvm.org/docs/CommandGuide/clang.html) 与 [GCC Overall Options](https://gcc.gnu.org/onlinedocs/gcc/Overall-Options.html) 展示编译阶段由 driver option 选择而不是拆成子命令的惯例。
- [rustc command-line arguments](https://doc.rust-lang.org/rustc/command-line-arguments.html) 提供 `--emit`、输出、诊断格式和逐行 UTF-8 响应文件的成熟参考。
- Windows 使用系统宽字符命令行作为 Unicode 来源；共享层负责 UTF-8 转换、参数解析、帮助生成和退出码契约。
