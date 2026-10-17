# 历史 semantic 测试参考

本目录保存从 `src/testcase/semantic` 原样迁出的 15 个旧测试源文件。它们仅用于查阅旧实现的行为场景，未加入任何构建目标，也不计入当前测试数量。当前有效用例见 [semantic 测试说明](../../../src/testcase/semantic/README.md)。

这些文件使用旧版 IR、execution 或 semantic API，不能直接作为当前实现的可运行测试。部分 include 已不存在，例如 `ink/execution/engine/execution_engine.h`、`ink/ir/decl/module_decl.h` 和 `ink/ir/ir_builder.h`；旧版返回 `Module *` 的两参数 `Analyzer::analyze` 调用也与当前接收调用方输出模块、返回 `bool` 的入口不兼容。相对 include 保留原文件内容，仅供参考，不能据此编译本目录。

| 参考文件 | 原行为主题 |
| --- | --- |
| `address_of_test.cpp` | 运行时取地址及存储访问 |
| `analyzer_function_test.cpp` | 函数分析及旧 IR 构造 |
| `analyzer_test.cpp` | 旧分析入口及输入校验 |
| `array_test.cpp` | 数组构造及执行 |
| `class_test.cpp` | 类、成员及生命周期 |
| `comptime_call_test.cpp` | 编译期调用 |
| `comptime_test.cpp` | 编译期求值及诊断 |
| `entry_execution_test.cpp` | 入口函数执行 |
| `external_call_test.cpp` | 外部函数调用 |
| `generic_name_resolver_test.cpp` | 泛型名称解析 |
| `if_test.cpp` | 条件语句 |
| `logical_test.cpp` | 逻辑运算及短路 |
| `loop_test.cpp` | 循环及控制转移 |
| `module_decl_test.cpp` | 模块声明 |
| `module_import_test.cpp` | 模块导入 |

恢复某项行为时，先检查对应场景是否已有活跃测试，再迁移需要的 API 和断言，将可编译、可运行的用例放回 `src/testcase/semantic` 并显式加入其 `CMakeLists.txt`。只有实际接入并运行后，才能计入当前测试覆盖与通过数量。

原有完整实现及构建配置可在 `saved-execution` 分支的 `22c3946c4737a7942fc28ab7cfa6f78e6fa3bd31` 提交查看。
