# n-body 执行性能基准

`nbody.asm` 在 Ink VM 内实现五天体的初始动量抵消、能量计算和时间步进。算法与初始条件参照 Christoph Bauer 为 [Computer Language Benchmarks Game 编写的标量 n-body 程序](https://benchmarksgame-team.pages.debian.net/benchmarksgame/program/nbody-gcc-1.html)，时间步长固定为 `0.01`。

上游采用 Revised BSD 许可，保留的版权与分发条款见 [LICENSE.benchmarksgame](LICENSE.benchmarksgame)。

所有位置、速度、质量和中间值均采用双精度。汇编使用 `fadd64rrr`、`fsub64rrr`、`fmul64rrr`、`fdiv64rrr`、`fsqrt64rr` 五条浮点指令；浮点寄存器操作数不带偏移。整数 ADD 的加零形式负责原始位模式装入、复制和存回，整数比较分支负责循环。没有通过 Native stub 执行物理计算，也不依赖新增浮点立即数或 SIMD 指令。

## 构建与运行

本目录提供 Ink、C++、C#、Python、Lua、JavaScript 和 Daslang 的标量双精度实现。JavaScript 分别运行默认模式和 `--jitless`。C# 和 Daslang 分别运行解释器及 JIT。

先按 [Fibonacci 构建说明](../fib/README.md#windows-构建与运行) 配置 `$Build`、构建 Lua、准备 Mono 和其余运行时。在同一 PowerShell 会话中从仓库根目录构建并运行 n-body。

```powershell
cmake --build $Build --target ink_nbody_benchmark cpp_nbody_benchmark --parallel 8

& $Mono $Mcs -optimize+ "-out:$CsharpBuild/nbody.exe" benchmark/csharp/Benchmark.cs benchmark/nbody/nbody.cs

& $Python benchmark/nbody/run.py --ink "$Build/benchmark/nbody/ink_nbody_benchmark.exe" --cpp "$Build/benchmark/nbody/cpp_nbody_benchmark.exe" --lua "$LuaBuild/bin/lua.exe" --lua-clock "$LuaBuild/bin/benchmark_clock.dll" --python $Python --node $Node --daslang $Daslang --daslang-mode both --mono $Mono --csharp "$CsharpBuild/nbody.exe" --steps 100000 --rounds 3 --samples 11 --warmup 3 --cpu 2 --priority high --output build/benchmark/nbody/results.json
```

外部运行时不随 CMake 自动下载。Lua 需要现有 `benchmark/fib/lua_clock.c` 构建的模块；脚本通过 `NBODY_BENCHMARK_CLOCK` 指定其位置，Windows 下使用 `QueryPerformanceCounter`。不安装 Daslang 时，删去 `--daslang` 与 `--daslang-mode both`。`--daslang-mode interpreter`、`jit`、`both` 分别选择解释器、LLVM JIT 或两者。Python 子进程固定设置 `PYTHON_JIT=0`。

Ink 命令格式为 `ink_nbody_benchmark FILE.asm [STEPS=100000] [SAMPLES=11] [WARMUP=3] [TRACE.json]`；其他实现接受 `STEPS SAMPLES WARMUP`，Daslang 在 `--` 之后接受这些参数。步数范围为 `0` 至 `100000000`。所有运行器输出包含初末能量、35 个天体状态值和全部执行耗时的单行 JSON。

## 计时与校验口径

- 每次计时都包含相同的完整模拟：恢复五天体初始状态、抵消动量、计算初始能量、执行指定时间步、计算最终能量。Ink 只计时 `Interpreter::execute()`；其他实现计时同等功能的模拟函数。
- 解析、发射、装载、进程启动、参考计算、显式预热、结果校验、输出和 trace 序列化均不计时。Daslang 显式 LLVM JIT 启动编译在计时开始前完成；V8 自行管理分层编译，不能保证所有后续或后台 JIT 活动都发生在计时之外。
- 每个进程显式预热 3 次，再采样 11 次；运行 3 轮，每项共 33 个样本，汇总其中位数。Windows 子进程绑定逻辑 CPU 2，使用 High 优先级；进程顺序逐轮轮换，各项串行运行。
- 显式预热前，各实现还计算一次不计时的参考结果；Ink 的参考结果由宿主 C++ 计算，因此它不是额外的一次 VM 预热。比较脚本另启动一次 C++ 参考进程，为所有运行时提供共同的校验基准。
- 每次预热和采样均在计时后校验两项能量及全部 35 个状态值；跨运行时也比较这些值，要求有限并满足 `abs(actual - expected) <= 1e-12 * max(1, abs(expected))`。任意不匹配都会终止运行，不能只凭能量接近判断状态正确。
- 数据结构和分配采用各语言的自然实现；运行时 GC、边界检查和编译器自动优化属于各实现的成本。这是同一算法的实现比较，并不保证相同的机器指令流。

Ink、C++ 和 Lua 使用 GCC `-O3 -DNDEBUG`。Ink 为 Shipping，`INK_ENABLE_ASSERTIONS=0`。C# 的 Mono 解释器和 JIT 运行同一个程序集，用 `Stopwatch.GetTimestamp()` 计时。Daslang 使用官方预编译运行时。

`run.py` 将所有样本、完整命令、运行时版本、机器信息、源码哈希、数值参考结果和汇总统计保存在 `build/benchmark/nbody/results.json`。同目录的 `execute-round-*.trace.json` 为 Ink 的 Chrome Trace，各事件只覆盖一次测量的 `Interpreter::execute()`。

VM 单元测试复用此汇编，覆盖 `0`、`1`、`10`、`1000` 步及同一解释器上的重复执行，并核对全部状态和能量。

## 模块接口

| 导出 | 含义 |
| --- | --- |
| `main` | 无参数入口；每次执行重新复制固定初始条件、抵消动量，然后模拟指定步数。 |
| `nbody_steps` | 宿主执行前写入的 `u64` 步数。 |
| `nbody_initial_energy` | 抵消初始动量后的 `double` 能量。 |
| `nbody_final_energy` | 模拟完成后的 `double` 能量。 |
| `nbody_bodies` | 五条连续记录，每条依次为 `x, y, z, vx, vy, vz, mass`，各字段八字节，共 280 字节。 |

宿主可在同一已加载镜像上修改步数并再次执行，不需自行恢复天体数组。R0–R9 按现有调用者保存约定使用；函数之间不依赖 GPR 保留值，能量结果通过镜像内的结果槽传递。

## 测量结果（2026-11-18）

- AMD Ryzen 9 9950X3D，Windows 11 26200，逻辑 CPU 2，High 优先级。Ink 使用基于 `dab46447b62a` 的当前工作区代码。
- 工作量为 100,000 步，`dt=0.01`。每轮预热 3 次、采样 11 次，共 3 轮，各项报告 33 个样本的中位数。
- Ink、C++ 和 Lua 5.5.1 使用 GCC 16.1.0 和 `-O3 -DNDEBUG`。Ink 为 Shipping，`INK_ENABLE_ASSERTIONS=0`，使用 switch 分派。
- CPython 3.13.2 设置 `PYTHON_JIT=0`。JavaScript 使用 Node.js 24.13.0 / V8 13.6.233.17-node.37。
- C# 使用 Mono 6.12.0.206 x64。Daslang 使用官方 0.6.5 Windows x64 wheel。

| 实现 | 执行模式 | 中位耗时（ms） |
| --- | --- | ---: |
| Ink | switch 解释器，O3 / Shipping | 74.8610 |
| C++ | 原生编译，O3 | 3.3144 |
| C# | Mono IL 解释器 | 93.2336 |
| C# | Mono JIT | 7.2492 |
| Python | CPython 解释器，JIT 关闭 | 354.2849 |
| Lua | 解释器，宿主 O3 构建 | 95.5465 |
| JavaScript 默认模式 | V8，允许 JIT 优化 | 4.5277 |
| JavaScript `--jitless` | V8，禁用 JIT | 239.6431 |
| Daslang | 解释器 | 91.4571 |
| Daslang | LLVM JIT | 4.0825 |

全部 30 次正式运行的 330 个测量样本通过校验。初始能量为 `-0.16907516382852447`，最终能量为 `-0.1690798593916664`。Ink 的中位耗时约为 C++ 的 22.59 倍、Daslang 解释器的 0.82 倍。Python、Lua 和 JavaScript jitless 的中位耗时分别约为 Ink 的 4.73、1.28 和 3.20 倍。

原始样本、命令、版本、源码哈希和汇总保存在 `build/benchmark/nbody/results.json`，Ink 的三份 execute trace 位于同目录。复现命令见上文；原始数据与构建产物不纳入源码提交。
