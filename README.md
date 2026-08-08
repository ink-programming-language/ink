# ink

## 构建

初始化固定版本的第三方依赖：

```powershell
git submodule update --init --recursive
```

使用 Visual Studio 2022 生成、编译 LLVM/Clang 依赖和 Ink，并运行全部测试：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target ink_llvm_components ink_tests
ctest --test-dir build -C Release --output-on-failure
```

LLVM 的构建树包会生成到 `build/third_party/llvm/lib/cmake/llvm`，主工程通过 `LLVM_DIR` 和 `find_package(LLVM CONFIG)` 导入。`utf8proc 2.9.0` 固定使用 Unicode 15.1，与 Tokenizer draft 的语言版本一致。
