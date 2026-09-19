# ink

当前按 [`docs/Ink-Lexical-Rules.md`](docs/Ink-Lexical-Rules.md) 重建前端，仅构建 Core、CLI 支持库、tokenizer、`ink-tokenize` 和 tokenizer 测试。parser、IR、后端及执行器的源码保留，但在诊断和 token API 重新适配前不参与构建。旧 testcase 已清空，新测试统一放在 `src/testcase/tokenizer`。

## 构建

初始化固定版本的第三方依赖：

```powershell
git submodule update --init --recursive
```

使用 Visual Studio 2022 生成、编译 LLVM/Clang 依赖和 Ink，并运行全部测试：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target ink_tokenize ink_tests
ctest --test-dir build -C Release --output-on-failure
```

LLVM 的构建树包会生成到 `build/third_party/llvm/lib/cmake/llvm`，主工程通过 `LLVM_DIR` 和 `find_package(LLVM CONFIG)` 导入。`utf8proc 2.9.0` 固定使用 Unicode 15.1，与当前词法规则的 Unicode 版本一致。命令行工具统一使用仓库自有的 `ink::cli::Application` 解析参数，并通过 `ParseResult` 显式返回解析和定义错误，不使用 C++ 异常作为控制流。spdlog 1.17.0 负责 Ink 的统一文本输出和运行时日志。

Ink 所有工具共用的参数拼写、输入输出、诊断和退出码规则见 [`docs/command-line.md`](docs/command-line.md)。

基于 [`docs/grammar.bnf`](docs/grammar.bnf) 交互确认的语义分析、泛型、comptime 和新 IR 设计规则见 [`docs/IR.md`](docs/IR.md)。该文档记录设计约定，不表示相关编译管线已经实现。

Ink 跨 module 函数、成员函数、闭合实例、全局变量、Imported 符号和 `extern "C"` 的链接名称规则见 [`docs/name-mangling.md`](docs/name-mangling.md)。

## Tokenizer 接口

- `tokenize(FrontendContext &, std::string)` 或 `tokenizeSource(FrontendContext &, SourceId)` 返回持有源码和解码值的 `TokenizedBuffer`。先检查 `succeeded()`；成功结果以唯一的 `END_OF_FILE` 结束。全局 UTF-8／NUL／BOM 校验失败不输出 token，其他词法错误保留此前完成的 token，不追加 EOF。
- `TokenKind` 为扁平枚举，C++ 枚举项按仓库约定使用 UpperCamelCase，`tokenKindName()` 返回文档中的大写名称。空白、注释和错误不占 token 种类。标识符的 NFC 名称、字符串的解码 UTF-8 值和字符的 Unicode 标量值分别保存在 payload 中。
- Core 的 `SourceLocation` 使用 32 位不透明编码，默认无效，`fromByteOffset(0)` 表示有效的文件起点。`SourceRange` 始终为原始字节的半开区间，可通过 `getBegin()`／`getEnd()` 获取位置。文件身份由源码 buffer 或诊断的 `Source` 保存；不同文件的位置不可直接比较。超过 `0xFFFFFFFE` 字节的输入显式报 `SourceTooLarge`。
- Unicode 正式名称复用 LLVM 的公开名称查询接口；完整别名由官方 Unicode 15.1 `NameAliases.txt` 生成。更新方式见 `cmake/generate_unicode_aliases.py`，生成表记录原始数据的 SHA-256，XID 和 NFC 数据不复制到本项目。
