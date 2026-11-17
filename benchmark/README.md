# 性能基准

每个用例使用独立子目录，包含各语言实现、构建配置、运行脚本和结果说明。

- [fib](fib/README.md)：递归 Fibonacci，对比 Ink、C++、Python、Lua、JavaScript 和 Daslang。
- [nbody](nbody/README.md)：五天体双精度模拟，对比 Ink、C++、Python、Lua、JavaScript 和 Daslang，校验能量及全部天体状态。

配置主项目时启用 `INK_BUILD_VM_BENCHMARKS=ON`；各用例的具体配置、计时口径及性能结果见对应文档。第三方运行时不随项目自动下载，构建产物和原始采样记录放在 `build/` 下。
