# Ink 链接符号名称改编

> 状态：设计规则版本 1，尚未完整接入 LLVM backend 和多模块 AOT 驱动
>
> 更新日期：2026-08-19

## 1. 范围

本文档规定 Ink 函数和全局变量的规范链接身份以及 ASCII linkage name 编码。该编码用于让独立 lowering 的 Ink module 在 LLVM IR、object 和最终链接阶段引用同一个符号，并避免不同 package、module、类型、重载和闭合实例之间发生名称冲突。

本文档只规定链接名称，不规定 class 内存布局、目标平台参数传递、vtable 布局、异常 ABI、异步 frame ABI 或 object 格式细节。LLVM target backend 仍负责目标平台要求的 object-level symbol decoration。

## 2. 阶段边界

开放泛型声明不得进入 LLVM lowering，也没有 linkage name。泛型绑定、实例化和 residualization 必须在形成 Closed InkIR 前完成。

前端为每个需要生成运行时代码的闭合函数建立 `LinkageIdentity`。该身份由规范声明路径、闭合实例键和闭合 Ink 函数签名组成。LLVM backend 只能消费已经形成的身份，不得重新执行泛型推导、绑定或实例化。

概念流水线为：

```text
开放声明
→ 名称绑定与重载解析
→ 编译期参数绑定
→ 实例化与 residualization
→ 闭合函数和 ClosedInstanceKey
→ Closed InkIR
→ linkage name 编码
→ LLVM lowering
```

普通非实例化声明使用空的闭合实例键。不同闭合实例即使具有相同运行时参数和返回类型，也必须具有不同的闭合实例键。例如 `value::<1>()` 和 `value::<2>()` 都可能具有 `i32()` 签名，但不能共享 linker symbol。

## 3. 规范声明路径

Ink 声明路径由以下部分组成：

```text
root package
+ zero or more subpackages
+ module
+ zero or more enclosing nominal types
+ declaration name
```

例如：

```text
game.main.main
game.graphics.window.Player.take_damage
collections.box.Box.get
```

package 和 module 身份遵守文件系统 package/module 规则。Package 版本、物理文件路径、源码根目录、dependency checkout 路径、局部 import 别名和相对 import 的原始拼写不进入声明路径。

所有名称组件必须已经规范化为 NFC，名称比较区分大小写。成员函数的声明路径包含所属 class；static 成员没有隐含 `this`，仍通过声明路径中的 class 与其他声明区分。

匿名 module 没有可用于跨 module 链接的规范身份。除非编译驱动先为其分配规范 module 身份，否则匿名 module 只能产生 module 内部 linkage，不能导出或被其他 Ink module 导入。

## 4. Linkage 分类

运行时代码使用三类 linkage name：

1. Ink linkage：使用本文定义的 `_INK1` 编码，适用于可跨 Ink module 引用的函数和全局变量。
2. C linkage：`extern "C"` 使用明确的外部符号名，不进行 Ink 编码。
3. Module 内部 linkage：编译器生成的 helper、私有常量和不跨 module 引用的实现细节使用 LLVM `internal` 或 `private` linkage，不构成稳定 Ink ABI。

符号可见性和导出策略独立于名称改编。具有 Ink linkage name 不表示该声明自动成为公共导出。

## 5. 基础文法

Ink linkage name 使用 ASCII，并以 ABI 版本前缀开始：

```text
ink-linkage-name = "_INK1" entity-encoding;
```

其中：

- `_INK` 是 Ink 编译器保留前缀。
- `1` 是名称改编 ABI 版本。
- 不兼容地修改编码时必须增加版本，不能在版本 1 中改变已有编码的含义。

实体种类为：

```text
F  普通函数，包括顶层函数、static 成员函数和非 static 成员函数
C  构造函数
D  析构函数
G  全局变量或 static 数据成员
```

普通实例字段不形成独立 linker symbol，因此不进行名称改编。

## 6. 名称组件编码

名称组件先编码为 UTF-8，再转换为大写十六进制：

```text
name-component = "U" utf8-byte-count "_" uppercase-hex-utf8;
```

长度是原始 UTF-8 字节数，不是 Unicode code point 数量，也不是十六进制文本长度。

示例：

```text
game        → U4_67616D65
window      → U6_77696E646F77
Player      → U6_506C61796572
take_damage → U11_74616B655F64616D616765
```

十六进制编码保证最终 linkage name 不依赖 object 格式对 Unicode symbol 的处理方式。Demangler 必须验证长度、十六进制合法性、UTF-8 合法性和 NFC，再恢复源码名称。

## 7. 作用域编码

作用域使用以下结构：

```text
scope = package-path module enclosing-types declaration-name;

package-path     = "K" package-component-count "_" {name-component};
module           = "M" name-component;
enclosing-types  = "O" owner-count "_" {owner-type};
owner-type       = "T" name-component closed-instance-key;
declaration-name = "N" name-component;
```

规范 module 路径的最后一个段编码为 `M`，此前所有 root package 和 subpackage 段编码在 `K` 中。例如 `game.graphics.window` 编码为 package 路径 `game.graphics` 和 module `window`。

嵌套 nominal type 按从外到内的顺序编码。泛型 class 的开放声明不会到达此阶段；`owner-type` 中记录的是已经闭合的类型实例键。

## 8. 闭合实例键

闭合实例键使用：

```text
closed-instance-key = "X" argument-count "_" {closed-argument};
```

普通非实例化声明使用：

```text
X0_
```

`X` 不是 LLVM backend 中的泛型参数列表，而是前端已经计算完成的闭合实例身份。当前定义以下闭合参数编码：

```text
T<type>                  闭合类型参数
V<type>_<canonical-bits> 闭合标量值参数
```

固定宽度整数的 `canonical-bits` 使用相应宽度的二进制补码大写十六进制。例如：

```text
i32 1  → Vi32_00000001
i32 2  → Vi32_00000002
i32 -1 → Vi32_FFFFFFFF
```

版本 1 为 `byte` 和 `i32` 定义稳定的值参数编码：`byte` 恰好使用 2 个大写十六进制数字，`i32` 恰好使用 8 个大写十六进制数字。`bool`、`ptrsize` 和浮点类型的值参数编码尚未定义，不得用于跨 module 的公开闭合实例。

尚未定义稳定编码的编译期值不能进入跨 module 的公开闭合实例。新增闭合参数种类必须使用新的无歧义标签；改变已有编码含义必须增加名称改编 ABI 版本。

## 9. 类型编码

名称改编使用规范 Ink 类型，不使用 lowering 后的 LLVM 类型。特别地，`ptrsize` 始终编码为 `z`，不能在 64 位目标上改写成 `i64`；LLVM opaque pointer 也不能抹除 Ink 指针目标类型和 const 限定后再参与名称改编。

当前基础类型编码为：

```text
v    void
b    bool
y    byte
i32  i32
z    ptrsize
h    f16
f    f32
d    f64
```

复合类型使用前缀编码：

```text
k<type>              const T
p<type>              T*
r<type>              T&
s<type>              T[]
a<length>_<type>     T[N]
t<count>_<types...>  tuple
n<nominal-identity>  class、interface 或 enum 类型
```

`nominal-identity` 复用作用域编码，并在类型名称后追加该类型自身的闭合实例键：

```text
nominal-identity = scope closed-instance-key;
```

其中 `scope` 的 `declaration-name` 表示 nominal type 名称。因此其完整顺序是规范 package、module、enclosing type、类型名称和闭合实例键。具体类型系统增加新种类时，必须先为其分配无歧义的类型编码。

版本 1 的单棵规范类型树最多包含 128 层。最外层类型计为第 1 层；`k`、`p`、`r`、`s` 和 `a` 的子类型、tuple 的元素类型，以及 nominal type 身份内 owner 或类型自身闭合键中的类型继续增加一层。函数或 global 身份中相互独立的参数、结果、值类型和闭合类型参数分别从第 1 层开始计数。Mangler 和 demangler 必须拒绝超过该限制的身份或输入，避免递归类型编码耗尽实现栈。

## 10. 统一函数模型

顶层函数、static 成员函数和非 static 成员函数使用相同的函数编码：

```text
function-encoding = function-kind scope closed-instance-key parameter-list result-type;

function-kind  = "F" | "C" | "D";
parameter-list = "P" parameter-count "_" {type};
result-type    = "Q" type;
```

参数名称、默认实参、源码位置、函数体和不改变调用 ABI 的 attribute 不进入 linkage name。返回类型不参与 Ink 重载选择，但进入 linkage name，使错误的跨 module 返回类型声明得到不同符号，而不是静默链接后形成 ABI 不匹配。

非 static 成员函数在形成 Closed InkIR 时转换为普通闭合函数，并把隐含 `this` 放在参数列表第一个位置：

```text
func Player.take_damage(amount: i32)
→ game.graphics.window.Player.take_damage(Player*, i32) -> void
```

可写成员的第一个参数是所属类型的可写 receiver 指针，只读成员的第一个参数是指向 const 所属类型的 receiver 指针。物理 LLVM 参数可以是 opaque `ptr`，但名称改编必须使用 opaque pointer 擦除前的规范 Ink receiver 类型。

static 成员函数的声明路径仍包含所属 class，但参数列表没有隐含 receiver。构造函数和析构函数同样在 Closed InkIR 中显式携带 codegen 所需的目标对象参数，并分别使用 `C` 和 `D` 实体标签。

## 11. Global 编码

Global 使用：

```text
global-encoding = "G" scope mutability "Y" type;

mutability = "W" | "R";
```

其中：

- `W` 表示 mutable global。
- `R` 表示 immutable global。
- `Y` 后跟 global 的规范值类型。

类型和可变性进入 linkage name，使错误的 imported global 声明不能静默绑定到不兼容定义。Class static 数据成员使用相同编码，其 `enclosing-types` 包含所属 class。

## 12. Imported 符号

Imported 声明必须根据目标身份生成 linkage name，不能使用当前 module 中的本地别名。

例如：

```text
declare import i32 @local_answer(i32) from module library.math, symbol @answer
```

名称改编输入为：

```text
module:     library.math
symbol:     answer
parameters: i32
result:     i32
```

`local_answer` 只用于当前 InkIR module 内的名称绑定和 `FunctionId` 引用，不进入 linkage name。目标 module 中 `library.math.answer(i32) -> i32` 的 definition 使用完全相同的输入，因此产生相同 LLVM symbol；Imported 一侧形成 declaration，目标一侧形成 definition。

跨 module 解析阶段仍必须验证 imported declaration 和目标 definition 的完整签名、可见性及声明种类。不同 linkage name 只是额外的链接安全边界，不能代替语义诊断。

Imported global 遵守相同规则，使用 `Import.Module`、`Import.Symbol`、目标类型和可变性生成目标 global 的名称。

## 13. `extern "C"`

`extern "C"` 完全绕过 Ink 名称改编。以下声明：

```text
declare extern "C" i32 @write(i32, const byte*, ptrsize)
```

使用 LLVM linkage name：

```text
write
```

规则为：

- 不添加 `_INK1` 前缀。
- 不编码 package、module、所属类型、参数或返回类型。
- LLVM target backend 负责目标 object 格式要求的前导下划线等平台 decoration。
- 同一最终链接单元中的相同 C symbol 必须具有兼容签名。
- C linkage 不允许重载、开放泛型或闭合实例分派。
- 非 static 成员函数、构造函数、析构函数、虚函数和异步函数不能直接使用 C linkage。
- 参数和返回类型必须通过单独的 C ABI 合法性检查。

需要向 C 暴露成员行为时，必须提供显式顶层函数或 static thunk，并显式传入对象指针。

## 14. 不进入 linkage name 的信息

以下内容不进入 Ink linkage name：

- Package 版本和依赖来源。
- 物理文件路径和源码位置。
- 参数名称和默认实参。
- 函数体。
- 局部 import 别名。
- 可见性。
- 不改变调用 ABI 的效果和诊断 attribute。
- Class layout hash。
- 编译器构建版本。

布局和其他 ABI 兼容性由 module interface、编译缓存键和链接规划阶段单独验证，不把所有兼容性信息塞入 symbol name。

## 15. 示例

以下示例省略 LLVM target backend 可能施加的 object-level decoration。

### 15.1 顶层函数

```text
game.main.main() -> i32
```

编码为：

```text
_INK1FK1_U4_67616D65MU4_6D61696EO0_NU4_6D61696EX0_P0_Qi32
```

### 15.2 重载函数

```text
math.ops.convert(i32) -> f64
math.ops.convert(f64) -> i32
```

分别编码为：

```text
_INK1FK1_U4_6D617468MU3_6F7073O0_NU7_636F6E76657274X0_P1_i32Qd
_INK1FK1_U4_6D617468MU3_6F7073O0_NU7_636F6E76657274X0_P1_dQi32
```

### 15.3 闭合实例

前端从 `value<N: i32>() -> i32` 产生的两个闭合普通函数：

```text
game.main.value::<1>() -> i32
game.main.value::<2>() -> i32
```

分别编码为：

```text
_INK1FK1_U4_67616D65MU4_6D61696EO0_NU5_76616C7565X1_Vi32_00000001P0_Qi32
_INK1FK1_U4_67616D65MU4_6D61696EO0_NU5_76616C7565X1_Vi32_00000002P0_Qi32
```

LLVM backend 看到的是两个已经闭合的函数和两个不同的 `ClosedInstanceKey`，看不到开放形参 `N`，也不执行实例化。

### 15.4 Global

```text
mutable global game.state.counter: i32
```

编码为：

```text
_INK1GK1_U4_67616D65MU5_7374617465O0_NU7_636F756E746572WYi32
```

### 15.5 Imported function

本地声明：

```text
declare import i32 @local_answer(i32) from module library.math, symbol @answer
```

目标 definition 和 Imported declaration 都使用：

```text
_INK1FK1_U7_6C696272617279MU4_6D617468O0_NU6_616E73776572X0_P1_i32Qi32
```

本地别名 `local_answer` 不出现在名称中。

### 15.6 `extern "C"`

```text
declare extern "C" i32 @write(i32, const byte*, ptrsize)
```

使用未改编的 linkage name：

```text
write
```

## 16. 实现顺序

当前实现按以下顺序接入本规则：

1. 在 Closed InkIR 边界建立结构化 `LinkageIdentity`，不从带点字符串反推 package、module 或 class。
2. 实现共享的名称组件、类型、函数和 global encoder，并提供 demangler 测试。
3. Definition 使用当前规范 module 和声明身份；Imported 使用 `Import.Module` 和 `Import.Symbol` 的目标身份。
4. External 保留原始 C symbol 并设置 C calling convention。
5. Imported function lower 成 LLVM declaration，目标 definition 生成相同 LLVM symbol。
6. Imported global 使用相同的目标身份规则。
7. 多模块 AOT 驱动解析 active module DAG、验证跨 module 签名、lower 各 module 并链接 LLVM module 或 object。
8. 静态 AOT 流程在进入 LLVM backend 前消除运行时 `import`，或者由单独 runtime ABI 明确定义其 lowering；不得静默丢弃该指令。
