# 命令行接口

当前构建包含 `inkc`、`ink-tokenize` 和 `ink-parse`，各工具在自身入口中直接使用固定版本 cxxopts 3.3.1 解析 `argc` / `argv`，自行处理输入输出和退出码，不提供公共 CLI 模块。所有工具显式返回错误状态，不使用异常作为控制流。命令行参数和文件路径暂不做 UTF-8 校验或转换。

## 公共规则

- `-h` / `--help` 打印帮助，`-V` / `--version` 打印版本；信息写入 stdout 后退出。
- 参数名按完整拼写匹配，包含空格的路径由 shell 引号包围。
- `--` 结束选项识别，后续内容按位置参数处理；`inkc` 当前不接受位置参数。
- 词法和语法工具按原始字节读取输入；`-` 表示标准输入。Windows 标准输入切换为二进制读取。源码中的 Unicode 处理仍遵循词法规则。
- 工具诊断写入 stderr，正常工具输出写入 stdout；各工具自行配置 spdlog logger 和对应 sink。

## 编译器入口

```text
inkc -i FILE
```

`-i` / `--input` 指定唯一的输入文件路径，必须提供一次，路径不能为空；也支持 `--input=FILE`。包含空格的路径需加引号。

```powershell
.\build\src\tools\inkc\Release\inkc.exe -i .\examples\hello_world_windows.ink
.\build\src\tools\inkc\Release\inkc.exe -i "D:\Ink Sources\example.ink"
```

入口按原始字节读取文件，通过共享的源码管理器和诊断引擎依次调用 tokenizer、parser 和 semantic，将解析结果交给 `Analyzer::analyze` 写入内存中的 `ir::Module`。词法或语法分析失败时不会进入 semantic。`CompilationContext` 拥有编译会话服务，`SemanticContext` 拥有 Module 借用的 IR 池，Module 先于 SemanticContext 销毁。当前不执行源码，也不生成输出文件；`-` 在 `inkc` 中仍是文件路径。

成功时静默退出，退出码为 0；普通源码错误返回 1；参数错误、文件打开或读取失败、诊断输出失败返回 2。源码诊断由 `DiagnosticFormatter` 使用诊断自身的 `SourceId` 查询源码，输出文件名、从 1 开始的行号和字节列号、诊断码、半开字节范围 `[Begin, End)`、相关源码行及 `^`/`~` 范围标记，写入 stderr；有位置的关联 note 同样显示源码。预览将制表符展开到 8 列制表位，并将不可打印字节和非 ASCII 字节显示为 `\xHH`，以保持无效 UTF-8 等输入下的标记位置准确；位置头始终使用原始源码的字节列号。内部编译错误同样附带可用的源码信息，并沿用项目统一的 ICE/panic 机制直接终止进程。当前 semantic 仅支持空模块和仅含空块的遍历，函数、变量等具体语义及 IR 生成尚未实现，合法但尚未支持的节点会报告 `SemanticUnsupported`（`INK-S0012`），不会作为成功编译处理。`-h` / `--help` 和 `-V` / `--version` 可单独使用，且不读取文件。

`inkc` 在目标级定义 `CXXOPTS_NO_EXCEPTIONS` 和 `CXXOPTS_NO_REGEX`。由于上游无异常模式的错误分支会直接退出进程，入口使用字符串值、允许暂存未识别参数，并追加一个空保护参数，将缺值和未知参数交由显式状态检查处理；`-i` 保持需要实参，未设置隐式值。保护参数不成为用户输入，正常错误通过工具自身的 spdlog logger 报告。

## 词法和语法工具

```text
ink-tokenize [INPUT]
ink-parse [INPUT]
```

省略 INPUT 或指定 `-` 时读取 stdin。 `ink-tokenize` 将 token 流写入 stdout，`ink-parse` 将 AST 和恢复结构写入 stdout；源码诊断写入 stderr。

两个工具分别在各自入口中声明 cxxopts 选项并显式校验结果；只接受一个非空输入路径，`--` 后允许以连字符开头的文件名。未知选项、多余输入、重复帮助或版本选项，以及同时请求帮助和版本均返回参数错误。各目标均启用 `CXXOPTS_NO_EXCEPTIONS` 和 `CXXOPTS_NO_REGEX`。

成功退出为 0，词法或语法错误为 1，参数或输入输出错误为 2。源码内部错误使用项目统一的 ICE/panic 机制，可能直接终止进程。

Windows 多配置 Release 构建示例：

```powershell
'func main(): i32 { return 7; }' | .\build\src\tools\tokenizer\Release\ink-tokenize.exe
'func main(): i32 { return 7; }' | .\build\src\tools\parser\Release\ink-parse.exe
```

单配置生成器的工具位于 `build/src/tools/tokenizer` 和 `build/src/tools/parser`，省略配置子目录。

`InkcProcessTest`、`ParserProcessTest`、`ParserAdversarialProcessTest` 和 `TokenizerProcessTest` 通过 CTest 启动实际工具进程，验证参数、输入输出和诊断行为。
