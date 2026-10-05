# Semantic 测试保留说明

本目录的 17 个原有 `*_test.cpp` 文件保持精简前的完整内容，作为重建 semantic 的行为参考。

`main` 的 semantic 当前只有声明和文件骨架；这些测试仍引用原有 IR、execution 等 API，因此暂不加入 CMake 测试目标，也不计入当前通过的测试数量。恢复实现并迁移相关 API 后，再将对应测试加入构建。

可在 `saved-execution` 分支的 `22c3946c4737a7942fc28ab7cfa6f78e6fa3bd31` 提交查看原有完整实现和测试构建配置。
