# 项目协作规则

## 代码排版

- 本仓库自有的所有代码源文件、`CMakeLists.txt` 和 `.cmake` 文件均不设置最大行宽。
- 不得仅因行长或参数数量，把一个完整语句、函数调用、宏调用或 CMake 命令拆成多行。
- 同一调用的名称、参数和右括号应保持在同一物理行，即使该行较长；禁止逐参数换行以及把一条消息拆成多个相邻字符串。
- 只有语法或内容本身要求多行时才允许换行，例如包含多条语句的代码块、多行字符串和预处理器结构。
- 以上规则不适用于 `src/third_party` 下的外部依赖代码。

正确：

```cmake
add_subdirectory("${INK_GOOGLETEST_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/third_party/googletest" EXCLUDE_FROM_ALL)
```

错误：

```cmake
add_subdirectory(
  "${INK_GOOGLETEST_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}/third_party/googletest"
  EXCLUDE_FROM_ALL
)
```
