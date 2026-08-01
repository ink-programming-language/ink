# 议题 14：浮点运行环境与 subnormal

> 状态：已确认
> 确认日期：2026-08-01

本议题补充议题 12 的默认严格浮点语义。

## 1. 固定舍入模式

Ink v0 的普通浮点运算固定使用 round-to-nearest、ties-to-even。

```text
rounding mode = nearest, ties to even
```

程序不能在运行时修改 Ink 浮点运算的舍入模式。Ink v0 不提供读取或写入动态浮点舍入模式的语言 API。

编译器、运行库和平台 ABI 必须保证 Ink 代码在规定的舍入环境下执行。交叉编译使用目标平台规则，不能使用宿主浮点环境决定结果。

## 2. 浮点异常标志不可观察

IEEE 754 的 inexact、underflow、overflow、divide-by-zero 和 invalid 等浮点状态标志不属于 Ink v0 的可观察程序状态。

编译器不需要为了保存或恢复这些标志而在每次普通浮点运算周围插入额外代码。

Ink v0 不提供读取、清除或测试这些浮点异常标志的语言 API。以后如果增加浮点环境 API，必须作为独立语义模式设计，不能追溯改变现有普通浮点表达式。

## 3. 浮点异常不 trap

普通浮点运算不启用硬件浮点异常 trap。

例如：

```ink
let positive = 1.0f64 / 0.0f64; // +Infinity
let negative = -1.0f64 / 0.0f64; // -Infinity
let invalid = 0.0f64 / 0.0f64;  // NaN
```

这些结果遵守严格浮点语义，不是整数除法 PDB，也不需要编译器插入除零检查。

## 4. 严格模式保留 subnormal

普通严格浮点运算必须保留 subnormal，并使用 IEEE 754 gradual underflow 语义。

编译器不能在默认模式下静默启用：

- flush-to-zero（FTZ）；
- denormals-are-zero（DAZ）；
- 其他把 subnormal 输入或结果直接改成零的模式。

严格模式的结果不能因为优化级别、debug/release 构建或链接方式不同而改变为 flush-to-zero 结果。

## 5. `[fast_math]` 与 flush-to-zero

显式 `[fast_math]` 范围可以按照后续确定的 fast-math 契约允许 flush-to-zero 和 denormals-are-zero。

是否启用由源码语义和目标降低共同决定，不能由普通 `-O` 优化级别偷偷启用。

## 6. 目标支持要求

目标只有在能够实现规定语义时，才能声明支持某个严格浮点类型。

如果目标不能用原生硬件保留 subnormal，则实现必须选择以下一种方式：

- 使用符合规定结果的软件实现；
- 不声明支持该严格浮点类型或对应算术操作。

目标不能为了避免软件成本而把严格运算静默改成 PDB 或 flush-to-zero。

对于没有原生 `f16` 算术的目标，使用更宽浮点指令再舍入或使用软件帮助函数时，必须保证每个 `f16` 语言运算产生与严格 `f16` 操作相同的规定结果。无法保证时不得宣称完整支持。

## 7. 与外部代码的边界

Ink 目标 ABI 必须规定调用边界上的浮点环境责任。

普通 Ink 函数可以假设入口处满足 Ink 固定浮点环境。会修改硬件舍入模式、异常 trap 或 subnormal 模式的外部函数，必须在返回 Ink 代码前恢复 ABI 要求的环境。

FFI 包装器和手写汇编负责遵守该约定。违反约定属于调用边界契约错误。

## 8. LLVM 降低

在目标浮点环境满足上述约束时，严格浮点运算可以使用 LLVM 默认浮点指令：

- 默认 round-to-nearest、ties-to-even；
- 默认保留 subnormal；
- 浮点异常 trap 关闭；
- 浮点状态标志不可观察。

因此主流合规目标不需要在每次浮点运算周围生成环境保存、恢复或检查代码。

编译器必须正确设置目标的 denormal mode、函数属性和 ABI 初始化，不能只依赖进程外部偶然留下的硬件控制寄存器状态。
