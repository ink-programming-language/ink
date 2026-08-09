# 项目协作规则

## 代码排版

- 本仓库自有的所有代码源文件、`CMakeLists.txt` 和 `.cmake` 文件均不设置最大行宽。
- 不得仅因行长或参数数量，把一个完整语句、函数调用、宏调用或 CMake 命令拆成多行。
- 同一调用的名称、参数和右括号应保持在同一物理行，即使该行较长；禁止逐参数换行以及把一条消息拆成多个相邻字符串。
- 只有语法、内容或本文后续排版规则本身要求多行时才允许换行，例如代码块、大型表格初始化、多行字符串和预处理器结构。
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

## 目录约定

- Tokenizer 单元测试固定放在 `src/testcase/tokenizer`，不得使用 `testcast/tokenizer` 或另建同类测试目录。

## Core 公共类型

- 可被 tokenizer、parser 等多个前端阶段复用的源码位置和诊断类型放在 `src/include/ink/core`，实现放在 `src/lib/core`。
- `SourceLocation` 只用于表示源码中的单个位置；包含 `Start` 和 `End` 的半开区间 `[Start, End)` 必须命名为 `SourceRange`，不得把范围命名为 `SourceLocation`。
- `Diagnostic`、`DiagnosticKind` 和诊断名称转换等公共诊断 API 归 Core 模块所有，具体前端模块不得重复定义同类公共容器。

## 错误处理与诊断

- 用户源码、module、语义、运行时 trap、资源限制和可恢复的代码生成失败必须先表示为结构化诊断，再交给公共 Diagnostic Consumer 输出；适用公共 registry 的源码诊断必须注册稳定的 `DiagnosticKind`，不得只传递一段临时错误字符串。
- CLI 参数、显式输入输出 I/O、manifest 定位和工具链启动等没有源码位置的失败使用公共 driver diagnostic 入口，不得伪造源码位置或稳定诊断码。
- 已验证结构不可能出现的状态、编译器不变量破坏、verifier 之后的非法 IR，以及成功结果缺失必需载荷属于 Internal Compiler Error；官方工具必须调用统一的 `internalCompilerError` 机制，由最外层 `runMain` 边界唯一格式化并映射到退出码 `3`，不得在内部手写 `internal error` 后直接返回。
- 工具和编译阶段不得用 `std::cerr`、`fprintf(stderr, ...)` 或相邻字符串拼接自行排版错误；路径、severity、稳定诊断码、源码范围、note 和 driver 前缀只能由公共 Consumer 生成。向 Consumer 传入 stderr 不属于自行排版。
- 外部 linker、进程或文件系统操作失败本身不等于 ICE；能够归因于用户输入或环境时必须生成对应诊断，只有证明编译器内部不变量已经破坏时才能升级为 ICE。

## 命名风格

- C++ 命名参考 LLVM Coding Standards。
- 类型、枚举类型和枚举项使用 `UpperCamelCase`。
- 变量、函数参数和结构体或类字段使用 `UpperCamelCase`。
- 函数和方法使用 `lowerCamelCase`。
- 禁止使用 `line_starts_` 一类尾随下划线字段；字段与普通变量采用相同的 `UpperCamelCase` 规则。
- 常量不强制使用 `k` 前缀，按其语义使用清晰的 `UpperCamelCase` 名称即可。
- Python 代码遵循 PEP 8，包括函数、变量和参数使用 `snake_case`，类使用 `CapWords`。
- 本节规则不适用于 `src/third_party` 下的外部依赖代码。

## 代码块

- 所有 C++ 复合语句和声明体都使用 Allman 大括号风格，包括 namespace、class、struct、enum、函数体、`if`/`else`、`while`、`for`、`switch`、lambda 和其他带代码块的结构。
- 左花括号必须单独占一行，右花括号必须从新行开始；声明体结尾语法必需的分号和 namespace 结束注释可以跟在右花括号后。即使代码块只有一条语句或为空，也不得把花括号放在声明、控制语句或块内容所在行。
- namespace 内的所有声明和定义必须相对 namespace 本身缩进一级；嵌套 namespace 逐级增加缩进，不使用 LLVM 默认的 namespace 内容顶格风格。
- namespace 左花括号后的第一行和 namespace 右花括号前的最后一行不得留空行；namespace 内部不同声明之间仍按逻辑保留空行。
- 聚合初始化、初始化列表和函数实参中的花括号不是代码块，不要求使用 Allman 换行，但大型表格初始化仍须遵守“数据定义”一节的逐项多行规则。
- 展开代码块不改变“完整函数调用保持在同一物理行”的规则；代码块内的单个完整调用仍不得因行宽或参数数量拆分。

正确：

```cpp
namespace ink::core
{
  struct State
  {
    bool Ready;
  };

  bool isReady()
  {
    return Ready;
  }
} // namespace ink::core

if (Condition)
{
  handleCondition(FirstArgument, SecondArgument, ThirdArgument);
}
else
{
  recover();
}

auto Handler = []()
{
  return 1;
};
```

错误：

```cpp
namespace ink::core
{
struct State
{
  bool Ready;
};
} // namespace ink::core

struct State {
  bool Ready;
};

bool isReady() {
  return Ready;
}

if (Condition) {
  handleCondition(FirstArgument, SecondArgument, ThirdArgument);
} else {
  recover();
}

auto Handler = []() {
  return 1;
};
```

## 数据定义

- 大型 `constexpr` 数据、查找表和其他表格型初始化定义必须展开为多行，每个表项单独一行，并保留尾随逗号以便维护。
- 本规则针对数据初始化内容，不授权把函数调用或宏调用的参数逐项拆行。

正确：

```cpp
constexpr KeywordEntry Keywords[] = {
    {"as", KeywordKind::As},
    {"async", KeywordKind::Async},
    {"await", KeywordKind::Await},
};
```

错误：

```cpp
constexpr KeywordEntry Keywords[] = {{"as", KeywordKind::As}, {"async", KeywordKind::Async}, {"await", KeywordKind::Await}};
```

## 单元测试

- 每个 `TEST` 宏前必须写一条具体注释，说明该测试验证的功能、边界或错误场景；禁止使用“测试 tokenizer”一类无法区分测试意图的泛化注释。

## Unicode 数据

- Tokenizer 的 `XID_Start` 和 `XID_Continue` 必须直接复用仓库固定版本 Clang 的 `clang/lib/Lex/UnicodeCharSets.h`，不得复制或生成一份本项目自有的 XID 表。
- Clang 的 `XIDContinueRanges` 只包含不属于 `XID_Start` 的继续字符，因此 Ink 的 XID Continue 判定必须同时查询 `XIDStartRanges` 和 `XIDContinueRanges`。
- NFC 校验固定使用 `utf8proc 2.9.0` 提供的 Unicode 15.1 规范化实现，不得重新生成或维护本项目自有的规范分解、组合和规范组合类表。
- LLVM、Clang 和 utf8proc 通过 `src/third_party` 下固定提交的 submodule 引入；除本节明确允许的 `UnicodeCharSets.h` 外，不得继续扩大对 `clang/lib` 私有接口的依赖。
