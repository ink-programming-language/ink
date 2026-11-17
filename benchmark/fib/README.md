# Fibonacci 执行性能基准

各实现使用无记忆化的递归算法 `fib(n) = n if n < 2 else fib(n - 1) + fib(n - 2)`。本目录包含 Ink 汇编、C++、Python、Lua、JavaScript 和 Daslang 实现，以及构建和对比脚本。`fib(32)` 的结果为 `2178309`。

当前 Ink 使用 switch 分派。

## 计时口径

- Ink 只计时 `Interpreter::execute`；其他运行时只计时递归 `fib(n)` 调用。
- 不计入 ASM/脚本解析、代码生成、装载、JIT 编译、预热、结果校验、进程启动、输出和 trace 序列化。
- 每个进程先预热 3 次，再记录 11 次；运行 3 轮，每项共 33 个样本，报告合并样本的中位数。
- Windows 子进程绑定逻辑 CPU 2，使用 High 优先级；每轮轮换运行顺序，所有结果在计时区间之后校验。
- 源码算法一致，原生编译器和 JIT 可以优化递归，因此机器级调用次数不保证相同。

`run.py` 保存所有样本、运行命令和运行时版本，并为 Ink 生成 Chrome Trace 格式的 `*.trace.json`。其中事件为完整的 `Interpreter::execute` 区间，不含解析和装载。

## Windows 构建与运行

先初始化仓库子模块。准备 Python 3.14.4、Node.js 20.20.0，以及 Lua 5.5.1 官方源码；以下假定 Lua 的 `src` 目录位于 `build/fib-benchmark/tools/lua-5.5.1/src`。Daslang 为可选项，使用官方 0.6.5 Windows release 包，以下假定其程序位于 `build/fib-benchmark/tools/daslang-0.6.5/daslang/_sdk/bin/daslang.exe`。这些外部运行时不会由 Ink 的 CMake 自动下载。

在 VS 2022 的 **x64 Native Tools Command Prompt** 中启动 PowerShell，使编译器继承 Windows SDK 和 MSVC 标准库环境，然后从仓库根目录执行。按安装位置调整变量；测量使用的 clang-cl 版本为 19.1.5。

```powershell
git submodule update --init --recursive

$VsRoot = "C:/Program Files/Microsoft Visual Studio/2022/Community"
$Clang = "$VsRoot/VC/Tools/Llvm/x64/bin/clang-cl.exe"
$Ninja = "$VsRoot/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe"
$Build = "build/benchmark/o3-shipping"
$LuaBuild = "build/benchmark/tools/lua"
$LuaSource = (Resolve-Path "build/fib-benchmark/tools/lua-5.5.1/src").Path
$Python = "C:/Python314/python.exe"
$Daslang = "build/fib-benchmark/tools/daslang-0.6.5/daslang/_sdk/bin/daslang.exe"

& $Clang --version
cmake -S . -B $Build -G Ninja "-DCMAKE_MAKE_PROGRAM=$Ninja" "-DCMAKE_C_COMPILER=$Clang" "-DCMAKE_CXX_COMPILER=$Clang" -DCMAKE_BUILD_TYPE=Release -DINK_BUILD_MODE=shipping -DINK_BUILD_VM_BENCHMARKS=ON -DBUILD_TESTING=OFF "-DCMAKE_C_FLAGS_RELEASE=/clang:-O3 /DNDEBUG" "-DCMAKE_CXX_FLAGS_RELEASE=/clang:-O3 /DNDEBUG"
cmake --build $Build --target ink_fib_benchmark cpp_fib_benchmark --parallel 8

cmake -S benchmark/fib/lua -B $LuaBuild -G Ninja "-DCMAKE_MAKE_PROGRAM=$Ninja" "-DCMAKE_C_COMPILER=$Clang" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_C_FLAGS_RELEASE=/clang:-O3 /DNDEBUG" "-DLUA_SOURCE_DIR=$LuaSource"
cmake --build $LuaBuild --target lua benchmark_clock --parallel 8

$env:PYTHON_JIT = "0"
& $Python benchmark/fib/run.py --ink "$Build/benchmark/fib/ink_fib_benchmark.exe" --cpp "$Build/benchmark/fib/cpp_fib_benchmark.exe" --lua "$LuaBuild/bin/lua.exe" --lua-clock "$LuaBuild/bin/benchmark_clock.dll" --python $Python --node node --daslang $Daslang --daslang-mode both --n 32 --rounds 3 --samples 11 --warmup 3 --cpu 2 --priority high --output build/benchmark/fib/results.json
```

`INK_BUILD_MODE=shipping` 关闭 Ink 的 `check`，与 CMake 的 `Release` 配置独立。clang-cl 的 Release 默认优化参数不等于 O3，因此命令显式设置 `/clang:-O3`；Ink 自有 C++ 目标另由项目配置关闭异常。Lua 必须同时构建 `benchmark_clock` 模块，Windows 下通过 `QueryPerformanceCounter` 测量经过时间，不使用 `os.clock()` 的进程 CPU 时间。

脚本默认同时运行 JavaScript 默认模式和 `node --jitless`，两者使用同一份 `fib.js` 和相同参数，无需额外开关。默认模式允许 V8 JIT 优化；`--jitless` 用于测量禁用 JIT 的解释执行路径。

不安装 Daslang 时，删去运行命令中的 `--daslang $Daslang --daslang-mode both`。安装后，`--daslang-mode interpreter`、`jit`、`both` 分别选择解释器、LLVM JIT 或两者；默认只运行解释器。JIT 模式使用 `-jit -jit-no-cache`，编译在脚本计时开始前完成。

## 测量结果（2026-11-17）

以下结果使用当前 switch 实现及上述配置测量。

- 环境：Intel Core i9-13900K，Windows 11 22631，逻辑 CPU 2，High 优先级。
- 工作量：`n=32`，每轮预热 3 次、采样 11 次，共 3 轮；各项均为 33 个执行样本的中位数。
- Ink、C++ 和 Lua：clang-cl 19.1.5，O3；Ink 为 Shipping、check 关闭。
- Python 3.14.4 的 JIT 未启用；JavaScript 使用 Node.js 20.20.0 / V8 11.3.244.8-node.33，分别测量默认模式和 `--jitless`。
- Daslang 0.6.5 使用官方 Windows release 包，宿主编译器优化参数未知；不能据此声称与本地 C++ 使用了相同的 O3 参数。解释器和 LLVM JIT 分开列出。

| 实现 | 执行模式 | 中位耗时（ms） |
|---|---|---:|
| Ink switch | 当前保留的分派实现，O3 / Shipping / check 关闭 | 65.9266 |
| C++ | 原生编译，O3 | 4.5326 |
| Python 3.14.4 | CPython 解释器，JIT 未启用 | 211.3313 |
| Lua 5.5.1 | 解释器，宿主 O3 构建 | 101.9600 |
| JavaScript 默认模式 | Node.js / V8，允许 JIT 优化 | 15.0292 |
| JavaScript `--jitless` | Node.js / V8，JIT 禁用 | 136.0454 |
| Daslang 0.6.5 | 解释器 | 37.9089 |
| Daslang 0.6.5 | LLVM JIT，编译时间不计入 | 4.6521 |

该结果只描述此递归 Fibonacci 工作量和上述工具链，不能推广为各语言的整体性能排名。运行命令的 `--output` 路径保存原始样本、完整命令和版本信息；上例为 `build/benchmark/fib/results.json`。
