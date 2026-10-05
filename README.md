# ink

`main` 保留完整的 tokenizer、parser、AST、对应测试，以及前端所需的 Core、CLI 和构建依赖。Semantic 保留原有头文件、源文件及目录结构、公开分析入口和 analyzer 基于 `ASTNodes.def` 的语句及声明分派；具体语义处理仍报告未实现。`src/testcase/semantic` 中的 17 个原有测试文件完整保留，暂不参与编译和运行；新增的入口与分派回归测试已启用。

精简前的完整主干保存在 [`saved-execution`](https://github.com/ink-programming-language/ink/tree/saved-execution) 分支，快照提交为 `22c3946c4737a7942fc28ab7cfa6f78e6fa3bd31`。原有 semantic 实现、IR、execution、ABI、LLVM backend、inkc 及其测试和文档可在该分支查看。

## 前端规范

- [词法规则](docs/Ink-Lexical-Rules.md)
- [语法规则](docs/Ink-grammar-Rules.bnf)
- [Parser 设计](docs/Ink-Parser-Design.md)
- [AST 序列化](docs/Ink-AST-Serialization.md)
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

`ink_tests` 包含 Core、tokenizer、parser/AST 及 semantic 分析入口的单元测试，构建时同时构建 `ink-tokenize` 和 `ink-parse`。CTest 还运行 tokenizer、parser 的 CLI 进程测试和 parser 对抗输入测试。单独构建并运行全部启用测试可使用 `cmake --build build --config Release --target run_all_tests`。

`Analyzer::analyze(SemanticContext &, const parser::ParseResult &)` 返回 `bool`，校验解析结果及源码归属后，按源码顺序遍历模块和块，通过 `analyzeStmt`、`analyzeDeclStmt` 和 `analyzeDecl` 到达具体节点处理函数。每次调用独立维护块深度，并读取 `INK_SEMANTIC_BLOCK_DEPTH_LIMIT`。空模块和仅含空块的输入可以完成遍历；具体语义及 comptime 求值尚未实现，命中时报告既有的 `SemanticUnsupported` ICE。当前入口不生成 IR，不提供原执行分支的模块返回值或多模块分析能力。

LLVM/Clang 固定版本为 22.1.8；tokenizer 直接使用 Clang 的 `UnicodeCharSets.h`，parser 的 AST 归档使用 LLVM 支持库。NFC 校验使用 utf8proc 2.9.0 的 Unicode 15.1 实现。spdlog 1.17.0 负责所有自有 C++ 文本输出；CLI 统一通过 `ink::cli::writeOutput` 输出。所有自有 C++ 编译目标在目标级关闭异常。

Core 提供源码位置、范围、源码管理、诊断和配置。资源限制在 `src/include/ink/core/config.def` 中定义，parser 与 AST 归档可通过对应的 `INK_PARSER_*`、`INK_AST_ARCHIVE_*` 环境变量配置；semantic 配置与诊断定义保留供后续实现使用。内部编译错误经 spdlog 输出后 panic，普通源码错误通过诊断引擎报告。
