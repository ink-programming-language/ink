# 命令行接口

当前构建包含 `ink-tokenize` 和 `ink-parse`。工具共用 `ink::cli::Application`，显式返回参数解析状态，不使用异常作为控制流。

## 公共规则

- `-h` / `--help` 打印帮助，`-V` / `--version` 打印版本；信息写入 stdout 后退出。
- 参数名按完整拼写匹配，包含空格的路径由 shell 引号包围。
- `--` 结束选项识别，后续内容按位置参数处理。
- 输入文件按原始字节读取；`-` 表示标准输入。Windows 标准输入切换为二进制读取，文件路径通过 UTF-8 转换。
- 工具诊断写入 stderr，正常工具输出写入 stdout，统一经过 `ink::cli::writeOutput`。

## 词法和语法工具

```text
ink-tokenize [INPUT]
ink-parse [INPUT]
```

省略 INPUT 或指定 `-` 时读取 stdin。 `ink-tokenize` 将 token 流写入 stdout，`ink-parse` 将 AST 和恢复结构写入 stdout；源码诊断写入 stderr。

成功退出为 0，词法或语法错误为 1，参数或输入输出错误为 2。源码内部错误使用项目统一的 ICE/panic 机制，可能直接终止进程。

Windows 多配置 Release 构建示例：

```powershell
'func main(): i32 { return 7; }' | .\build\src\tools\tokenizer\Release\ink-tokenize.exe
'func main(): i32 { return 7; }' | .\build\src\tools\parser\Release\ink-parse.exe
```

单配置生成器的工具位于 `build/src/tools/tokenizer` 和 `build/src/tools/parser`，省略配置子目录。

`ParserProcessTest`、`ParserAdversarialProcessTest` 和 `TokenizerProcessTest` 通过 CTest 启动实际工具进程，验证参数、输入输出和诊断行为。
