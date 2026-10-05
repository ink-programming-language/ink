# Semantic 测试保留说明

本目录的 17 个原有 `*_test.cpp` 文件保持精简前的完整内容，作为重建 semantic 的行为参考。

`main` 的 semantic 保留声明、文件骨架和 analyzer 基于 `ASTNodes.def` 的语句及声明分派，具体处理函数仍返回失败。这些测试仍引用原有 IR、execution 等 API，因此暂不加入 CMake 测试目标，也不计入当前通过的测试数量。恢复实现并迁移相关 API 后，再将对应测试加入构建。

可在 `saved-execution` 分支的 `22c3946c4737a7942fc28ab7cfa6f78e6fa3bd31` 提交查看原有完整实现和测试构建配置。
