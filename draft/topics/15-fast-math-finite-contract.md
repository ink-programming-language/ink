# 议题 15：fast-math 与有限值契约

> 状态：已确认，2026-08-08 收窄 v0 fast-math 许可；是否允许附着用户定义数值类型仍待定
> 确认日期：2026-08-08

本议题补充议题 12 和议题 14。

## 1. 两种独立属性

Ink 将“允许宽松浮点变换”与“承诺不存在 NaN/Infinity”拆成两个独立属性：

```ink
[fast_math]
func approximate(value: f64) {
    // 允许宽松浮点变换。
}

[assume_finite]
func finite_only(value: f64) {
    // 承诺相关浮点值始终为有限值。
}
```

两个属性可以单独使用，也可以组合使用。

## 2. `[fast_math]`

`[fast_math]` 允许编译器进行可能改变精确浮点结果的变换，包括：

- 浮点重结合；
- 乘加收缩为 FMA；
- 忽略正零与负零的部分差异；
- 按议题 14 允许 flush-to-zero 和 denormals-are-zero；
- 执行依赖这些许可的向量化和强度削减。

Ink v0 的 `[fast_math]` 不允许使用近似倒数替代规定除法，也不允许近似数学函数。以后若增加这些能力，必须通过独立属性、内建操作或库 API 给出明确误差和特殊值契约。

`[fast_math]` 本身不承诺 NaN 或 Infinity 不会出现：

- NaN 和 Infinity 仍是合法输入和值；
- 不能仅因为出现 NaN 或 Infinity 就产生 UB 或 poison；
- 优化器不能仅凭 `[fast_math]` 删除处理 NaN 或 Infinity 的控制流；
- 宽松变换可以改变特殊值的传播路径和 NaN payload，但不能把特殊值的存在当作不可能。

## 3. `[assume_finite]`

`[assume_finite]` 是程序提供给编译器的明确契约：处于该属性作用域内的相关浮点输入和中间结果不得为 NaN、正 Infinity 或负 Infinity。

违反该契约属于 UB。编译器可以：

- 假设浮点参数不是 NaN 或 Infinity；
- 假设受约束浮点操作不会产生 NaN 或 Infinity；
- 删除只处理 NaN 或 Infinity 的分支；
- 在 LLVM IR 中使用与 `nnan`、`ninf` 等价的优化信息。

`[assume_finite]` 不表示：

- 允许浮点重结合；
- 允许自动 FMA 收缩；
- 忽略正负零；
- 允许 flush-to-zero；
- 值一定是非零、非负或 normal。

这些宽松优化仍需要 `[fast_math]` 或其他未来明确属性。

## 4. 组合使用

需要同时授予宽松数值变换和有限值假设时，可以组合：

```ink
[fast_math, assume_finite]
func kernel(a: f32, b: f32, c: f32) -> f32 {
    return a * b + c;
}
```

组合后：

- 编译器拥有 `[fast_math]` 的数值变换许可；
- 编译器拥有 `[assume_finite]` 的有限值假设；
- NaN 或 Infinity 违反显式契约并可导致 UB；
- subnormal 是否保留由 `[fast_math]` 的 flush-to-zero 许可决定。

## 5. 单独使用示例

只允许 v0 宽松优化，但仍需要接受特殊值：

```ink
[fast_math]
func graphics(value: f32) -> f32 {
    return value * 0.5f32;
}
```

只承诺有限值，但仍要求严格运算顺序：

```ink
[assume_finite]
func ordered(a: f64, b: f64, c: f64) -> f64 {
    return (a + b) + c;
}
```

第二个函数仍不能被重新结合成 `a + (b + c)`。

## 6. 诊断

当编译器能够静态证明 `[assume_finite]` 契约被违反时，必须产生 warning，但仍按照普通 UB 契约规则处理程序。

```ink
[assume_finite]
func invalid() -> f64 {
    return f64.infinity; // warning: violates assume_finite contract
}
```

是否允许项目把该 warning 升级为错误，由统一诊断控制机制决定。

## 7. LLVM 映射

`[fast_math]` 只映射到它明确允许的逐操作许可：重结合、收缩和忽略有符号零；FTZ 与 DAZ 通过满足单条 operation 语义的目标 lowering 实现。它不能映射到近似倒数、近似函数，不能仅凭该属性设置会把 NaN 或 Infinity 变成 poison 的 flags。

`[assume_finite]` 可以映射到 `nnan` 和 `ninf`，并向优化器提供相应契约。

编译器不能为了方便而把单独的 `[fast_math]` 直接映射成 LLVM 的完整 `fast` 标记，因为完整 `fast` 包含本议题没有授予的有限值假设。

## 8. 属性作用域

`[fast_math]` 与 `[assume_finite]` 已确认可以附着函数声明，并按照 Parser 议题 31 位于全部函数修饰符之前。它们作用于该函数实现中的相关浮点运算和契约检查。

Ink v0 不提供参数、返回位置、语句块或单个表达式 attribute 语法，因此这两个属性也不能写在这些位置。是否允许把它们附着到用户定义数值类型声明并传播到其操作，仍属于独立的语义设计问题；Parser 可以建立类型声明前的普通 attribute list，但在该规则确认前语义分析必须拒绝这两个具体属性的类型目标。
