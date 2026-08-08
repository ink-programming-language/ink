# Parser 覆盖率

Parser 覆盖率是默认关闭的独立构建模式。启用后只为 `ink_parser` 和 `ink_tests` 添加覆盖率编译参数，不改变普通构建；报告只统计 `src/lib/parser` 与 `src/include/ink/parser`。

支持 GCC 的 `gcov`，以及配套安装了 `llvm-profdata` 和 `llvm-cov` 的 GNU 风格 Clang。MSVC 和 `clang-cl` 不支持此模式。Python 3.8 或更高版本用于汇总工具原生数据，不依赖额外 Python 包。

## 配置和运行

推荐使用专门的 Debug 构建目录。以下是 GCC 示例：

```powershell
$env:Path = "C:\msys64\ucrt64\bin;$env:Path"
cmake -S . -B cmake-build-parser-coverage -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe -DINK_ENABLE_PARSER_COVERAGE=ON
cmake --build cmake-build-parser-coverage --target parser_coverage
```

`parser_coverage` target 会构建 `ink_tests`、删除该 target 对应的旧计数文件、运行 focused parser 测试、生成报告并执行阈值门禁。也可以通过 CTest 单独执行同一门禁：

```powershell
ctest --test-dir cmake-build-parser-coverage -R "^ParserCoverage$" --output-on-failure
```

默认 GoogleTest filter 是 `Parser*`，覆盖所有 parser 单元、恢复和鲁棒性测试，但不运行 2,533 个 CodeContests 集成实例。这样本地和 CI 门禁保持快速、确定；需要让语料集参与覆盖率采样时，可在配置阶段添加 `-DINK_PARSER_COVERAGE_INCLUDE_INTEGRATION=ON`。

## 门槛和报告

默认门槛根据 GCC focused 套件的实测结果设置为：

- 行覆盖率 85%；
- 函数覆盖率 85%；
- 分支覆盖率 55%。

可通过 `INK_PARSER_COVERAGE_MIN_LINES`、`INK_PARSER_COVERAGE_MIN_FUNCTIONS` 和 `INK_PARSER_COVERAGE_MIN_BRANCHES` cache 变量调整。任一指标低于门槛，target 和 CTest 均失败。

报告输出到构建目录的 `parser-coverage/<配置名>`，包括：

- `index.html`：浏览器可读的逐文件汇总；
- `parser-coverage.txt`：适合终端和 CI 日志；
- `parser-coverage.json`：适合后续自动化处理。

多配置生成器应在 CTest 命令中传入对应配置，例如 `ctest --test-dir build -C Debug -R "^ParserCoverage$" --output-on-failure`。
