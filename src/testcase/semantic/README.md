# Semantic 测试保留说明

本目录的 17 个原有 `*_test.cpp` 文件保持精简前的完整内容，作为重建 semantic 的行为参考。

`main` 的 semantic 保留公开的 `Analyzer::analyze` 入口、模块及块遍历和基于 `ASTNodes.def` 的语句及声明分派；具体语义处理仍报告 `SemanticUnsupported` ICE。原有测试引用 IR、execution 等 API，因此暂不加入 CMake 测试目标，也不计入当前通过的测试数量。恢复实现并迁移相关 API 后，再将对应测试加入构建。

新增的 `analyzer_entry_test.cpp` 已加入 `ink_tests`，覆盖公开入口的输入校验、源码归属、嵌套块遍历、具体语句及声明分派、块深度限制和未实现的 comptime 路径。它不依赖已移除的 IR 或 execution。

可在 `saved-execution` 分支的 `22c3946c4737a7942fc28ab7cfa6f78e6fa3bd31` 提交查看原有完整实现和测试构建配置。
