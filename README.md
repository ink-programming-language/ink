# ink

## 构建

初始化固定版本的第三方依赖：

```powershell
git submodule update --init --recursive
```

使用 Visual Studio 2022 生成、编译 LLVM/Clang 依赖和 Ink，并运行全部测试：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target ink_llvm_components ink_tokenize ink_parse inkc ink_tests
ctest --test-dir build -C Release --output-on-failure
```

LLVM 的构建树包会生成到 `build/third_party/llvm/lib/cmake/llvm`，主工程通过 `LLVM_DIR` 和 `find_package(LLVM CONFIG)` 导入。`utf8proc 2.9.0` 固定使用 Unicode 15.1，与 Tokenizer draft 的语言版本一致。CLI11 2.7.2 以 header-only 方式为命令行工具提供参数解析。

Ink 所有工具共用的参数拼写、输入输出、诊断和退出码规则见 [`docs/command-line.md`](docs/command-line.md)。

## 编译与运行

`inkc` 默认解释执行入口文件，也可以生成本机 AOT 可执行文件：

```powershell
inkc program.ink
inkc --interpret program.ink
inkc --aot -o program.exe program.ink
```

构建时 `src/std` 会复制到 `inkc` 可执行文件旁的 `std` 目录。程序使用 `import std.int as integers;` 一类导入时不依赖当前工作目录；开发或自定义部署也可以通过 `--std-path DIR` 显式指定标准库根目录。

当前语言切片也支持在解释执行和 AOT 中调用 `std.io.output` 输出字符串字面量：

```ink
import std.io as io;

func main() -> i32
{
  io.output("Hello, world!\n");
  return 0;
}
```

`std.io.input` 以及把运行期切片传给 `std.io.output` 需要后续的指针和切片语义支持，当前会通过稳定的 module diagnostic 明确拒绝。
