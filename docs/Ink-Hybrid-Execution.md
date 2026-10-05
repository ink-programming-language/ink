# AOT 与字节码混合执行

启用热更后，程序仍静态链接原生目标文件和 runtime。可热更函数只有一份原生函数体；编译器在它的入口插入检查。有补丁时进入 VM 执行字节码并返回，没有补丁时继续执行原来的 AOT 指令。业务代码仍调用 `f()`，不需要声明或维护 `fAot()`。

本实现采用函数体替换。补丁是字节码包，不加载 DLL、不生成机器码，也不改写进程代码页。

## 编译和使用

例如 `game.ink`：

```ink
public export "C" func f(Value: i32): i32
{
  return Value + 1;
}
```

编译基线目标文件，并保存对应清单：

```sh
inkc -i game.ink --library --hot-reload --hot-manifest game.base --emit-object game.obj --opt-level 2
```

`--library` 表示由 C/C++ 宿主提供 `main`。不带它时仍按 `--entry` 生成原生入口，入口会先注册混合执行清单；入口函数本身和名为 `main` 的函数不参与热更。

默认对本次编译涉及的所有模块启用入口检查。也可以重复传入 `--hot-module game`，只让指定模块可热更。模块名使用 `--module-root` 决定的规范模块身份，和普通字节码编译保持一致。

修改 `f` 的函数体后编译补丁：

```sh
inkc -i game.ink --emit-patch game.patch --patch-base game.base --patch-function 'game#f'
```

补丁编译仍需可分析的完整源文件及依赖，文件名、模块根目录和 package 身份应与基线一致。只有 `--patch-function` 选中的函数输出字节码，其余函数作为 runtime 引用。可重复选择多个函数；类方法写作 `game#Counter.advance`，同名重载作为一组选中。新加入的私有辅助函数也须显式选择，不能新增同名但签名不同的旧函数。新增公开 API 和修改现有签名需要重新发布 AOT 基线。

在本仓库 CMake 构建中，将原生目标文件加入宿主并链接：

```cmake
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/game.obj" PROPERTIES EXTERNAL_OBJECT TRUE)
add_executable(game_host host.cpp "${CMAKE_CURRENT_BINARY_DIR}/game.obj")
target_link_libraries(game_host PRIVATE ink::hybrid_runtime)
ink_disable_exceptions(game_host)
```

`ink::hybrid_runtime` 是静态链接依赖的聚合目标，包含 VM 和原生错误处理入口。普通 AOT 仍只需 `ink_aot_runtime`。当前生成器把所分析模块合入一个原生对象，并提供一个 `ink_hybrid_module()` 清单入口；不要把多份含同名清单入口的独立生成对象直接链接进同一程序。

宿主调用方式：

```cpp
#include "ink/execution/hybrid/runtime.h"

extern "C" const InkHybridModule *ink_hybrid_module();
extern "C" int32_t f(int32_t Value);

int main()
{
  if (ink_hybrid_register(ink_hybrid_module()) != InkHybridSuccess)
  {
    return 1;
  }
  const auto Before = f(10);
  if (ink_hybrid_apply("game.patch") != InkHybridSuccess)
  {
    // ink_hybrid_last_error() provides the failure message.
    return 2;
  }
  const auto After = f(10);
  if (ink_hybrid_clear() != InkHybridSuccess)
  {
    return 3;
  }
  const auto Restored = f(10);
  return Before == Restored && After != Before ? 0 : 4;
}
```

安装发生在宿主控制的调用边界，例如游戏主线程下一帧开始之前。文件监视、补丁下载和何时调用安装 API 由宿主安排，runtime 不自动监视文件。

## 调用与类型边界

- 原生入口调用 `ink_hybrid_enter` 取得当前补丁，随后选择 VM 或原生函数体；正常返回配对调用 `ink_hybrid_leave`。热更函数标记为 `noinline`，O2 优化仍保留入口检查。
- 编译器为函数生成统一的参数/返回值内存包装，复用反射调用的 lowering。包装是 ABI 适配代码，不是第二份原生业务函数体。
- VM 的每个已绑定调用先查询 runtime。目标有补丁时进入对应解释镜像，否则调用静态注册的原生包装。递归、跨函数调用，以及 AOT 再回调已热更函数都遵循同一规则。旧补丁中的调用不会缓存过期的函数体。
- 基线已有的 C 导入也生成静态调用包装，支持宿主未导出的静态链接函数。补丁新增的 C 导入则必须能由现有进程符号查询找到，且满足当前 FFI 的签名限制。
- 清单记录规范 `_INK2` 函数身份、签名、目标 ABI 和类型布局。跨镜像不直接比较局部 `FunctionId` 或 `RuntimeTypeId`；根据稳定身份匹配后，在目标类型表中解读原生布局字节。
- 当前支持已有执行器能够表示的标量、原始指针、类值和定长数组。跨 AOT/VM 边界的函数值/闭包尚未支持，加载时拒绝涉及它们的签名。这里的“回调”指原生代码再次调用稳定 Ink 函数入口。

## 安装、撤销和版本

基线清单包含由生成的 LLVM 模块计算的 SHA-256 构建标识。补丁必须携带匹配标识和有效字节码对象。安装依次检查目标 ABI、符号和签名、共享类的字段/布局/方法接口、函数是否启用热更，以及全部字节码函数的合法性。所有检查通过后才发布本包中的替换，失败保留已安装版本。

一次补丁可以替换多个函数；随后安装的包只覆盖自己选中的函数。未涉及的函数保留其当前版本。`ink_hybrid_clear()` 清除当前线程的全部替换，恢复原生函数体。补丁镜像由引用计数持有，仍被其他函数版本使用的镜像不会提前释放。

首版 runtime 按线程持有注册表和 VM。调用线程必须自行注册并安装补丁，另一线程继续使用它自己的版本；这不是进程范围的同时切换。安装、撤销以及新增模块注册只能在当前线程没有活动混合调用时进行，否则返回 `InkHybridBusy`。没有执行栈迁移，也不把已运行到一半的 AOT 帧转换成 VM 帧。

## 对象和失败约定

类对象和数组共用原生内存布局，按值参数和返回值通过包装复制表示；这些复制本身不额外调用语言构造/析构函数。语义分析已生成的生命周期指令负责构造和销毁，未执行的 AOT 分支不会再销毁解释分支的对象。VM 调用 AOT 构造函数后，同步接收对象的初始化状态。

原始指针保留地址与别名关系，不自动转移所有权，也不延长被指对象的生存期。宿主和 AOT 代码不得保存指向已经结束的 VM 局部变量的指针。既存类不能通过补丁改变布局，当前也不迁移存量对象或静态状态。

返回到原生侧的字符串字节由线程 runtime 保留，撤销或替换代码镜像不会使它们失效；这些存储受 runtime 字节预算约束，在执行线程退出时释放。宿主不得将这种借用跨越线程 runtime 的生存期。源码尚未提供完整字符串类型声明语法，这条约定同时覆盖 IR 层的字符串桥接。

VM 的调用深度和执行预算在嵌套混合调用中共享，下一次最外层宿主调用可重新开始计步，仍存活的存储继续计入预算。普通编译期执行的累计预算语义不变。

安装 API 使用显式状态码和 `ink_hybrid_last_error()` 报错。补丁执行失败时，原生 `f` 没有额外错误返回槽，生成入口调用 runtime 的致命错误报告并终止进程，不静默回退并重跑原生函数，以免重复已经发生的副作用。此路径不使用 C++ 异常。

## 验证

```sh
cmake --build cmake-build-debug --target ink_tests
ctest --test-dir cmake-build-debug -R '^Hybrid' --output-on-failure
```

测试包含 O0/O2 真实可执行文件中的默认 AOT、VM→AOT→VM 递归、静态 C 导入、类/数组传值与析构、二次更新后的调用解析、撤销、线程隔离、忙状态拒绝、错误构建及不兼容类型拒绝；另有入口检查优化和字符串生命周期的单元测试。
