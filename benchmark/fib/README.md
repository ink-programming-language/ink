# Fibonacci 执行性能基准

各实现使用无记忆化的递归算法 `fib(n) = n if n < 2 else fib(n - 1) + fib(n - 2)`。本目录包含 Ink 汇编、C++、C#、Python、Lua、JavaScript 和 Daslang 实现，以及构建和对比脚本。`fib(32)` 的结果为 `2178309`。

当前 Ink 使用 switch 分派。

## 计时口径

- Ink 只计时 `Interpreter::execute`；其他运行时只计时递归 `fib(n)` 调用。
- 不计入 ASM/脚本解析、代码生成、装载、显式预热、结果校验、进程启动、输出和 trace 序列化。Daslang 显式 LLVM JIT 编译在计时前完成；V8 自行管理分层编译，不能保证后续或后台 JIT 活动全部发生在计时之外。
- 每个进程先预热 3 次，再记录 11 次；运行 3 轮，每项共 33 个样本，报告合并样本的中位数。
- Windows 子进程绑定逻辑 CPU 2，使用 High 优先级；每轮轮换运行顺序，所有结果在计时区间之后校验。
- 源码算法一致，原生编译器和 JIT 可以优化递归，因此机器级调用次数不保证相同。

`run.py` 保存所有样本、运行命令和运行时版本，并为 Ink 生成 Chrome Trace 格式的 `*.trace.json`。其中事件为完整的 `Interpreter::execute` 区间，不含解析和装载。

## Windows 构建与运行

先初始化仓库子模块。准备 MSYS2 UCRT64 GCC、Ninja、Python 和 Node.js。将 Lua 5.5.1 源码、Daslang 0.6.5 Windows x64 wheel 和 Mono 6.12.0.206 x64 解包到以下命令使用的目录。[Lua 源码](https://www.lua.org/ftp/) · [Daslang 发布文件](https://github.com/GaijinEntertainment/daScript/releases/expanded_assets/v0.6.5) · [Mono 下载](https://www.mono-project.com/download/stable/)

从仓库根目录运行 PowerShell 命令，并按本机安装位置调整路径。

```powershell
git submodule update --init --recursive

$GccBin = "C:/msys64/ucrt64/bin"
$Ninja = "$env:LOCALAPPDATA/Programs/CLion/bin/ninja/win/x64/ninja.exe"
$Python = (Get-Command python.exe).Source
$Node = (Get-Command node.exe).Source
$env:Path = "$GccBin;$env:Path"
$Build = "build/benchmark/gcc-o3-shipping"
$LuaBuild = "build/benchmark/tools/lua-o3"
$LuaSource = (Resolve-Path "build/benchmark/tools/lua-5.5.1/src").Path
$Daslang = "build/benchmark/tools/daslang-0.6.5/daslang/_sdk/bin/daslang.exe"
$MonoRoot = "build/benchmark/tools/mono-6.12.0.206/Mono"
$Mono = "$MonoRoot/bin/mono.exe"
$Mcs = "$MonoRoot/lib/mono/4.5/mcs.exe"
$CsharpBuild = "build/benchmark/tools/csharp"

cmake -S . -B $Build -G Ninja "-DCMAKE_MAKE_PROGRAM=$Ninja" "-DCMAKE_C_COMPILER=$GccBin/gcc.exe" "-DCMAKE_CXX_COMPILER=$GccBin/g++.exe" -DCMAKE_BUILD_TYPE=Release -DINK_BUILD_MODE=shipping -DINK_BUILD_VM_BENCHMARKS=ON -DBUILD_TESTING=OFF "-DCMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG" "-DCMAKE_CXX_FLAGS_RELEASE=-O3 -DNDEBUG"
cmake --build $Build --target ink_fib_benchmark cpp_fib_benchmark --parallel 8

cmake -S benchmark/fib/lua -B $LuaBuild -G Ninja "-DCMAKE_MAKE_PROGRAM=$Ninja" "-DCMAKE_C_COMPILER=$GccBin/gcc.exe" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG" "-DLUA_SOURCE_DIR=$LuaSource"
cmake --build $LuaBuild --target lua benchmark_clock --parallel 8

New-Item -ItemType Directory -Force -Path $CsharpBuild | Out-Null
& $Mono $Mcs -optimize+ "-out:$CsharpBuild/fib.exe" benchmark/csharp/Benchmark.cs benchmark/fib/fib.cs

$env:PYTHON_JIT = "0"
& $Python benchmark/fib/run.py --ink "$Build/benchmark/fib/ink_fib_benchmark.exe" --cpp "$Build/benchmark/fib/cpp_fib_benchmark.exe" --lua "$LuaBuild/bin/lua.exe" --lua-clock "$LuaBuild/bin/benchmark_clock.dll" --python $Python --node $Node --daslang $Daslang --daslang-mode both --mono $Mono --csharp "$CsharpBuild/fib.exe" --n 32 --rounds 3 --samples 11 --warmup 3 --cpu 2 --priority high --output build/benchmark/fib/results.json
```

`INK_BUILD_MODE=shipping` 使 `INK_ENABLE_ASSERTIONS=0`。Ink、C++ 和 Lua 均显式使用 `-O3 -DNDEBUG`。Lua 的 `benchmark_clock` 模块在 Windows 上通过 `QueryPerformanceCounter` 测量经过时间。

C# 的两种模式使用同一个由 `mcs -optimize+` 生成的程序集。Mono 的 `--interp` 选择 IL 解释器，默认模式使用 JIT。两者都通过 `Stopwatch.GetTimestamp()` 计时。省略 `--mono` 和 `--csharp` 可跳过 C#。

脚本默认同时运行 JavaScript 默认模式和 `node --jitless`，两者使用同一份 `fib.js` 和相同参数，无需额外开关。默认模式允许 V8 JIT 优化；`--jitless` 用于测量禁用 JIT 的解释执行路径。

不安装 Daslang 时，删去运行命令中的 `--daslang $Daslang --daslang-mode both`。安装后，`--daslang-mode interpreter`、`jit`、`both` 分别选择解释器、LLVM JIT 或两者；默认只运行解释器。JIT 模式使用 `-jit -jit-no-cache`，编译在脚本计时开始前完成。

## 测量结果（2026-11-18）

Ink 使用基于 `dab46447b62a` 的当前工作区代码。

- AMD Ryzen 9 9950X3D，Windows 11 26200，逻辑 CPU 2，High 优先级。
- 工作量为 `n=32`。每轮预热 3 次、采样 11 次，共 3 轮，各项报告 33 个样本的中位数。
- Ink、C++ 和 Lua 5.5.1 使用 GCC 16.1.0 和 `-O3 -DNDEBUG`。Ink 为 Shipping，`INK_ENABLE_ASSERTIONS=0`，使用 switch 分派。
- CPython 3.13.2 设置 `PYTHON_JIT=0`。JavaScript 使用 Node.js 24.13.0 / V8 13.6.233.17-node.37。
- C# 使用 Mono 6.12.0.206 x64。Daslang 使用官方 0.6.5 Windows x64 wheel。

| 实现 | 执行模式 | 中位耗时（ms） |
| --- | --- | ---: |
| Ink | switch 解释器，O3 / Shipping | 39.7864 |
| C++ | 原生编译，O3 | 1.4670 |
| C# | Mono IL 解释器 | 77.3476 |
| C# | Mono JIT | 7.6611 |
| Python | CPython 解释器，JIT 未启用 | 145.6409 |
| Lua | 解释器，宿主 O3 构建 | 69.9332 |
| JavaScript 默认模式 | V8，允许 JIT 优化 | 11.9036 |
| JavaScript `--jitless` | V8，禁用 JIT | 85.9807 |
| Daslang | 解释器 | 29.5980 |
| Daslang | LLVM JIT | 3.5071 |

全部 30 次正式运行的 330 个测量样本通过结果校验，`fib(32)` 均为 `2178309`。Ink execute trace 校验通过。

原始样本、完整命令、运行时版本和汇总统计保存于 `build/benchmark/fib/results.json`，Ink 的三份 execute trace 位于同目录。复现命令见上文；原始数据和构建产物不纳入源码提交。
