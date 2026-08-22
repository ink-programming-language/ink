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

## 构造函数初始化列表

- 所有带初始化列表的 C++ 构造函数都必须展开为多行，包括类内定义、类外定义、单项初始化、委托构造函数以及带 `noexcept`、`requires` 等说明的构造函数。
- 构造函数头必须完整保留在同一物理行，并在初始化列表前结束；下一行相对构造函数声明缩进 4 个空格，以 `:` 和首个初始化项开头。
- 每个后续初始化项必须单独占一行并与首个初始化项对齐；每个非末项的逗号放在该项行末，末项不加逗号。
- 每个初始化项及其完整初始化表达式必须保持在同一物理行，不得把初始化表达式的实参逐项换行；函数体左花括号另起一行并与构造函数头对齐。

正确：

```cpp
Impl(ExecutionContext &Context, const ir::Module &ModuleValue)
    : Context(Context),
      OwnedProvider(std::make_unique<SingleModuleProvider>(ModuleValue)),
      Provider(OwnedProvider.get()),
      EntryModule(static_cast<SingleModuleProvider &>(*Provider).id()),
      Loader(*Provider, *this, Context.compilationContext().targetContext())
{
}
```

错误：

```cpp
Impl(ExecutionContext &Context, const ir::Module &ModuleValue) : Context(Context), OwnedProvider(std::make_unique<SingleModuleProvider>(ModuleValue)), Provider(OwnedProvider.get()), EntryModule(static_cast<SingleModuleProvider &>(*Provider).id()), Loader(*Provider, *this, Context.compilationContext().targetContext())
{
}

Impl(ExecutionContext &Context, const ir::Module &ModuleValue)
    : Context(Context), OwnedProvider(std::make_unique<SingleModuleProvider>(ModuleValue)), Provider(OwnedProvider.get()), EntryModule(static_cast<SingleModuleProvider &>(*Provider).id()), Loader(*Provider, *this, Context.compilationContext().targetContext())
{
}
```

## 目录约定

- Tokenizer 单元测试固定放在 `src/testcase/tokenizer`，不得使用 `testcast/tokenizer` 或另建同类测试目录。

## Core 公共类型

- 可被 tokenizer、parser 等多个前端阶段复用的源码位置和诊断类型放在 `src/include/ink/core`，实现放在 `src/lib/core`。
- `SourceLocation` 只用于表示源码中的单个位置；包含 `Start` 和 `End` 的半开区间 `[Start, End)` 必须命名为 `SourceRange`，不得把范围命名为 `SourceLocation`。
- `Diagnostic`、`DiagnosticKind` 和诊断名称转换等公共诊断 API 归 Core 模块所有，具体前端模块不得重复定义同类公共容器。

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

## 类访问控制与成员缩进

- `class` 和 `struct` 中，`public:`、`protected:`、`private:` 必须相对类型声明缩进一级。
- 每个访问控制区段内的成员声明、成员定义、友元声明、嵌套类型和其他内容必须相对访问控制标签再缩进一级；成员函数体内部继续按代码块层级递增缩进。
- 没有显式访问控制标签的 `class` 或 `struct` 成员按对应的隐式访问区段处理，其缩进必须与存在访问控制标签时的成员缩进一致。
- 不得把访问控制标签与类型声明或类型体右花括号对齐。本节规则适用于头文件、实现文件和测试代码，不适用于 `src/third_party`。

正确：

```cpp
class ByteConstantId
{
  public:
    constexpr bool valid() const noexcept
    {
      return Value != InvalidId;
    }

  private:
    std::size_t Value = InvalidId;
};
```

错误：

```cpp
class ByteConstantId
{
public:
  constexpr bool valid() const noexcept
  {
    return Value != InvalidId;
  }

private:
  std::size_t Value = InvalidId;
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

## 输出与日志

- 仓库固定使用 `src/third_party/spdlog` 中的 spdlog 1.17.0 作为统一文本输出和日志库，不得引入第二套日志库。
- 本仓库自有 C++ 代码产生的所有进程文本输出和运行时日志都必须经过 spdlog；CLI 的 stdout、stderr 输出统一调用 `ink::cli::writeOutput`，其他日志场景使用 spdlog logger 与对应 sink。
- 禁止直接通过 `std::cout <<`、`std::cerr <<`、`std::clog`、`printf`、`fprintf`、`puts`、`fputs`、`llvm::outs()` 或 `llvm::errs()` 产生进程输出或日志。允许使用 `std::ostringstream` 仅在内存中组装完整消息，再交给 spdlog 输出。

## 异常机制

- 本仓库自有 C++ 代码（包括库、命令行工具和测试）不得使用或依赖 C++ 异常处理。禁止 `try`、`catch`、`throw`（包括无操作数重抛）、函数级 try 块、动态异常说明、`std::exception_ptr`、`std::current_exception`、`std::rethrow_exception`，以及 `EXPECT_THROW`、`ASSERT_THROW`、`EXPECT_ANY_THROW`、`ASSERT_ANY_THROW`、`EXPECT_NO_THROW`、`ASSERT_NO_THROW` 等异常测试宏；也不得使用 Windows SEH、`setjmp`/`longjmp` 等机制绕过本规则。
- 所有可恢复失败必须通过显式返回值、状态或结果类型、错误码或诊断报告；清理和回滚必须通过 RAII 或显式控制流完成，不得依赖异常栈展开。不得调用以抛异常作为预期失败通道的接口。
- `noexcept` 声明不属于被禁止的异常处理机制，但不得用 `noexcept` 把本应显式报告的可恢复失败静默转换为进程终止。宿主内存耗尽、标准库分配失败和同步原语失效等无法可靠恢复的宿主级故障按进程级致命错误处理，不得为捕获这些故障重新启用异常。
- 所有本仓库自有 C++ 编译目标必须在目标级关闭异常；不得通过全局编译选项影响 `src/third_party` 下的外部目标，也不得为单个自有目标重新启用异常。
- `src/third_party` 中未修改的上游实现可以保留自身异常代码，但异常不得越过第三方边界进入本仓库自有代码。本仓库只能调用具有受支持无异常模式或显式错误返回接口的依赖；不满足该条件的依赖必须替换或移除，不得在本仓库代码中用 `catch` 包装。

## Unicode 数据

- Tokenizer 的 `XID_Start` 和 `XID_Continue` 必须直接复用仓库固定版本 Clang 的 `clang/lib/Lex/UnicodeCharSets.h`，不得复制或生成一份本项目自有的 XID 表。
- Clang 的 `XIDContinueRanges` 只包含不属于 `XID_Start` 的继续字符，因此 Ink 的 XID Continue 判定必须同时查询 `XIDStartRanges` 和 `XIDContinueRanges`。
- NFC 校验固定使用 `utf8proc 2.9.0` 提供的 Unicode 15.1 规范化实现，不得重新生成或维护本项目自有的规范分解、组合和规范组合类表。
- LLVM、Clang 和 utf8proc 通过 `src/third_party` 下固定提交的 submodule 引入；除本节明确允许的 `UnicodeCharSets.h` 外，不得继续扩大对 `clang/lib` 私有接口的依赖。
